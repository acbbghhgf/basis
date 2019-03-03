#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include "qtk_storage.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "wtk/os/wtk_fd.h"
#include "wtk/core/wtk_os.h"
#include "qtk/core/cJSON.h"


const static char reg_fn[] = "_reg";
const static int INDEX_ACT_TIME = 60;                    /* index in act_dir no longer than 1 min */

typedef struct writable_topic writable_topic_t;
typedef struct topic_idx topic_idx_t;
typedef struct topic_results topic_results_t;
typedef long long (*topic_line_test_f)(const char *buf, int data);


struct writable_topic {
    qtk_hashnode_t node;
    wtk_queue_node_t anode;
    wtk_string_t *dir;
    long long id;
    time_t add_time;
};


struct topic_idx {
    wtk_queue_node_t node;
    long long idx;
    char buffer[16];
    int start;
    int len;
};


struct topic_results {
    wtk_heap_t *heap;
    wtk_queue_t result_q;
};


static topic_results_t* topic_results_new() {
    topic_results_t *tr = wtk_malloc(sizeof(*tr));
    tr->heap = wtk_heap_new(1024);
    wtk_queue_init(&tr->result_q);
    return tr;
}


static int topic_results_delete(topic_results_t *tr) {
    wtk_heap_delete(tr->heap);
    wtk_free(tr);
    return 0;
}


static int topic_results_push(topic_results_t *tr, long long idx, const char *buf,
                              int start, int len) {
    topic_idx_t *ti = wtk_heap_malloc(tr->heap, sizeof(*ti));
    ti->idx = idx;
    strncpy(ti->buffer, buf, sizeof(ti->buffer));
    ti->start = start;
    ti->len = len;
    wtk_queue_node_init(&ti->node);
    wtk_queue_push(&tr->result_q, &ti->node);
    return 0;
}


static topic_idx_t* topic_results_pop(topic_results_t *tr) {
    wtk_queue_node_t *n;
    n = wtk_queue_pop(&tr->result_q);
    return data_offset(n, topic_idx_t, node);
}


static long long topic_line_test_offset(const char *buf, int offset) {
    long long idx;
    char tmp[16];
    int start, len;
    int ret;
    ret = sscanf(buf, "%lld\t%s\t%d\t%d", &idx, tmp, &start, &len);
    assert(ret == 4);
    return offset + len;
}


static long long topic_line_test_idx(const char *buf, int idx) {
    long long line_idx;
    int ret;
    ret = sscanf(buf, "%lld", &line_idx);
    assert(ret == 1);
    return line_idx;
}


static writable_topic_t* writable_topic_new(qtk_storage_t *cache,
                                            wtk_string_t *topic) {
    writable_topic_t *wt = wtk_malloc(sizeof(*wt)+topic->len);
    wtk_queue_node_init(&wt->anode);
    wtk_queue_node_init(&wt->node.n);
    wtk_string_set(&wt->node.key._str, (char*)(wt+1), topic->len);
    wt->dir = cache->act_dir;
    wt->id = 0;
    memcpy(wt+1, topic->data, topic->len);
    return wt;
}


static void writable_topic_delete(qtk_storage_t *cache,
                                  writable_topic_t *wt) {
    wtk_free(wt);
}


static char* strchr_s(const char *s, int n, char c) {
    const char *e = s+n;
    while (s < e && *s != c) ++s;
    return s == e ? NULL : (char*)s;
}


static char* strrchr_s(const char *s, int n, char c) {
    const char *e = s+n-1;
    while (s <= e && *e != c) --e;
    return s > e ? NULL : (char*)e;
}


static int qtk_mkdir_p(char *fn) {
    char *stk[128];
    char *p;
    int top = 0;

    p = fn + strlen(fn);
    do {
        p = strrchr_s(fn, p - fn, '/');
        assert(p);
        *p = '\0';
        stk[top++] = p;
    } while (wtk_file_exist(fn));

    *stk[--top] = '/';

    while (top) {
        wtk_mkdir(fn);
        *stk[--top] = '/';
    }

    return 0;
}


static int creat_wfd(const char *fn) {
    int ret = creat(fn, 0600);
    if (ret == -1 && errno == ENOENT) {
        qtk_mkdir_p((char*)fn);
        ret = creat(fn, 0600);
    }
    return ret;
}


static int open_wfd(const char *fn) {
    int ret = open(fn, O_WRONLY | O_APPEND | O_CREAT, 0600);
    if (ret == -1 && errno == ENOENT) {
        qtk_mkdir_p((char*)fn);
        ret = open(fn, O_WRONLY | O_APPEND | O_CREAT, 0600);
    }
    return ret;
}


static int open_rfd(const char *fn) {
    return open(fn, O_RDONLY);
}


/*
  return read bytes, -1 if error
 */
static int read_next_line(int fd, char *buf, int sz) {
    int step = min(sz, 128);
    int readed = 0;
    int ret;

    while (readed < sz) {
        int want = min(sz-readed, step);
        ret = read(fd, buf+readed, want);
        if (ret <= 0) break;
        char *p = strchr_s(buf+readed, ret, '\n');
        if (p) {
            /* got '\n', adjust file pointer */
            lseek(fd, (p-buf)-readed+1-ret, SEEK_CUR);
            readed += p - (buf + readed) + 1;
            ret = readed;
            break;
        } else {
            readed += ret;
        }
    }
    buf[readed] = '\0';

    return ret < 0 ? ret : readed;
}


static int read_line(int fd, int idx, char *buf, int sz) {
    char *p = NULL;
    char *e = NULL;
    off_t cur = 0;
    char tmp[128];
    int ret = 0;
    if (idx > 0) {
        cur = lseek(fd, 0, SEEK_SET);
        while (idx) {
            if (0 == ret) {
                ret = read(fd, tmp, sizeof(tmp));
                if (ret <= 0) {
                    break;
                }
                p = tmp;
            }
            e = strchr_s(p, ret, '\n');
            if (e) {
                --idx;
                cur += e-p+1;
                ret -= e-p+1;
                if (ret) {
                    p = e + 1;
                }
            } else {
                cur += ret;
                ret = 0;
            }
        }
        lseek(fd, cur, SEEK_SET);
    } else if (idx < 0) {
        cur = lseek(fd, 0, SEEK_END);
        while (idx) {
            if (0 == ret) {
                if (cur > sizeof(tmp)) {
                    lseek(fd, cur-sizeof(tmp), SEEK_SET);
                } else {
                    lseek(fd, 0, SEEK_SET);
                }
                ret = read(fd, tmp, sizeof(tmp));
                if (ret <= 0) {
                    cur = 0;
                    ++idx;
                    break;
                }
                p = tmp;
            }
            e = strrchr_s(p, ret, '\n');
            if (e) {
                ++idx;
                cur -= ret-(e-p);
                ret = e-p;
            } else {
                cur -= ret;
                ret = 0;
            }
        }
        lseek(fd, cur+1, SEEK_SET);
    }
    if (0 == idx) {
        return read_next_line(fd, buf, sz);
    } else {
        return -1;
    }
}


int storage_test() {
    qtk_storage_cfg_t cfg;
    qtk_storage_cfg_init(&cfg);
    qtk_storage_t *cache = qtk_storage_new(&cfg, NULL);
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    wtk_strbuf_push_s(buf, "{\"id\":\"xxx\"}");
    qtk_storage_reg(cache, buf);
    qtk_storage_delete(cache);
    wtk_strbuf_delete(buf);
    qtk_storage_cfg_clean(&cfg);
    return 0;
}


int storage_fd_write(int fd, const char *buf, int len) {
    return wtk_fd_write(fd, (char*)buf, len);
}


static void storage_remove_wtopic(qtk_storage_t *cache, writable_topic_t *wt) {
    qtk_hashtable_remove(cache->wtopics, &wt->node.key);
    wtk_queue_remove(&cache->wtopic_q, &wt->anode);
    writable_topic_delete(cache, wt);
}


static writable_topic_t* storage_add_wtopic(qtk_storage_t *cache, wtk_string_t *topic) {
    writable_topic_t *wt;
    wtk_queue_node_t *node;
    time_t curtime = time(NULL);

    /*
      only remove one wtopic, to avoid hashtable grow
     */
    node = wtk_queue_peek(&cache->wtopic_q, 0);
    wt = data_offset(node, writable_topic_t, anode);
    if (wt && wt->add_time < curtime - cache->cfg->active_index_expire_time) {
        char srcfn[1024];
        char dstfn[1024];
        snprintf(dstfn, sizeof(dstfn), "%.*s/%.*s",
                 cache->old_dir->len, cache->old_dir->data,
                 wt->node.key._str.len, wt->node.key._str.data);
        if (wtk_file_exist(dstfn)) {
            /* topic index not in old_dir, remove from act_dir to old_dir */
            snprintf(srcfn, sizeof(srcfn), "%.*s/%.*s",
                     cache->act_dir->len, cache->act_dir->data,
                     wt->node.key._str.len, wt->node.key._str.data);
            if (rename(srcfn, dstfn)) {
                const char *es = strerror(errno);
                wtk_debug("move error: %s\r\n", es);
                qtk_log_log(cache->log, "move error: %s", es);
            }
            storage_remove_wtopic(cache, wt);
        } else {
            /* move wtopic to tail of queue, until index in old_dir is removed */
            wt->add_time = curtime;
            node = wtk_queue_pop(&cache->wtopic_q);
            wtk_queue_push(&cache->wtopic_q, node);
        }
    }
    wt = writable_topic_new(cache, topic);
    wt->add_time = curtime;
    qtk_hashtable_add(cache->wtopics, wt);
    wtk_queue_push(&cache->wtopic_q, &wt->anode);

    return wt;
}


static int write_topic_idx(qtk_storage_t *cache, wtk_string_t *topic,
                           int start, int len) {
    char tmp[1024];
    writable_topic_t *wt = qtk_hashtable_find(cache->wtopics, (qtk_hashable_t*)topic);
    int fd = INVALID_FD;
    int n;
    int ret = -1;

    snprintf(tmp, sizeof(tmp), "%.*s/%.*s",
             cache->act_dir->len, cache->act_dir->data,
             topic->len, topic->data);
    if (wt) {
        if (wt->dir != cache->act_dir) {
            wt->dir = cache->act_dir;
        }
        fd = open_wfd(tmp);
    } else {
        wt = storage_add_wtopic(cache, topic);
        fd = creat_wfd(tmp);
    }
    if (fd == INVALID_FD) {
        wtk_debug("open %s failed\r\n", tmp);
        qtk_log_log(cache->log, "open %s failed: %s", tmp, strerror(errno));
        goto end;
    }
    n = snprintf(tmp, sizeof(tmp), "%lld\t%.*s\t%d\t%d\n", wt->id,
                 cache->data_fn->len, cache->data_fn->data, start, len);
    if (n < 0) {
        goto end;
    }
    ret = storage_fd_write(fd, tmp, n);
    if (ret>= 0) wt->id++;
end:
    if (fd != INVALID_FD) {
        close(fd);
    }
    return ret;
}


static int write_msg_to_buf(qtk_storage_t *cache, const char *data, int len) {
    assert(cache->data_fd);
    if (storage_fd_write(cache->data_fd, (char*)data, len)) {
        cache->data_flen = lseek(cache->data_fd, 0, SEEK_END);
        return -1;
    } else {
        cache->data_flen += len;
        return 0;
    }
}


static int read_msg_from_buf(int fd, off_t start, size_t len, wtk_strbuf_t *buf) {
    int n = 0;
    if (lseek(fd, start, SEEK_SET) == start) {
        wtk_strbuf_push(buf, NULL, len);
        n = 0;
        fd_read(fd, buf->data+buf->pos-len, len, &n);
        if (n < len) {
            buf->pos -= len;
        }
    }
    return n < len ? -1 : n;
}


static int read_topic_line(int *fds, int *nfd, char *buf, int len) {
    int n = 0;
    while (*nfd) {
        n = read_next_line(fds[*nfd-1], buf, len);
        if (n > 0) {
            break;
        } else if (n <= 0) {
            *nfd = *nfd - 1;
            close(fds[*nfd]);
        }
    }
    return n;
}


static int open_topic(qtk_storage_t *cache, wtk_string_t *topic, int *fds,
                      topic_line_test_f test, int data) {
    char tmp[1024];
    int n;
    int fd;
    int nfd = 0;
    int i;
    int ofs;
    wtk_string_t *dirs[3] = {
        cache->trash_dir,
        cache->old_dir,
        cache->act_dir,
    };
    ofs = 0;
    for (i = 0; i < sizeof(dirs)/sizeof(dirs[0]); ++i) {
        off_t cur = 0;
        n = snprintf(tmp, sizeof(tmp), "%.*s/%.*s",
                     dirs[i]->len, dirs[i]->data,
                     topic->len, topic->data);
        if (n < 1) goto err;
        fd = open_rfd(tmp);
        if (INVALID_FD == fd) continue;
        while (0<=n && ofs < data && ofs >= 0) {
            cur = lseek(fd, 0, SEEK_CUR);
            n = read_next_line(fd, tmp, sizeof(tmp)-1);
            if (n <= 0) break;
            tmp[n] = '\0';
            ofs = test(tmp, ofs);
        }
        /* discard all index before read failed */
        if (n <= 0) ofs = 0;
        if (ofs >= data) {
            lseek(fd, cur, SEEK_SET);
            fds[nfd++] = fd;
        }
    }
    return nfd;
err:
    for (i = 0; i < nfd; ++i) {
        close(fds[i]);
    }
    return -1;
}


static int cache_read_data(qtk_storage_t *cache, const char *fn,
                           int start, int len, wtk_strbuf_t *buf) {
    char tmp[128];
    wtk_string_t *cur_fn = cache->data_fn;
    int ret;

    if (strncmp(cur_fn->data, fn, cur_fn->len) || fn[cur_fn->len]) {
        snprintf(tmp, sizeof(tmp), "%.*s/%s", cache->data_dir->len,
                 cache->data_dir->data, fn);
        int fd = open_rfd(tmp);
        if (INVALID_FD == fd) return -1;
        ret = read_msg_from_buf(fd, start, len, buf);
        close(fd);
    } else {
        assert(INVALID_FD != cache->data_rfd);
        ret = read_msg_from_buf(cache->data_rfd, start, len, buf);
    }
    return ret;
}


static int dir_walk_max_num(int *n, char *buf, int len) {
    char *p = strrchr_s(buf, len, '/');
    assert(p);
    p++;
    int i = wtk_str_atoi(p, buf+len-p);
    if (i > *n) {
        *n = i;
    }
    return 0;
}


static int storage_mkdir(const char *dn) {
    if (wtk_mkdir((char*)dn)) {
        wtk_debug("mkdir '%s' failed\r\n", dn);
        return -1;
    }
    return 0;
}


static int storage_mv(const char *src, const char *dst) {
    if (wtk_file_exist(dst)) {
        qtk_mkdir_p((char*)dst);
    }
    return rename(src, dst);
}


static int storage_write_finish(qtk_storage_t *cache, wtk_string_t *topic) {
    char srcfn[1024];
    char dstfn[1024];
    int ret = 0;
    writable_topic_t *wt = qtk_hashtable_find(cache->wtopics, (qtk_hashable_t*)topic);
    if (wt == NULL) return 0;

    snprintf(srcfn, sizeof(srcfn), "%.*s/%.*s",
             cache->old_dir->len, cache->old_dir->data,
             topic->len, topic->data);
    if (wtk_file_exist(srcfn)) {
        /* mv old_dir/topic to trash_dir/topic */
        snprintf(dstfn, sizeof(dstfn), "%.*s/%.*s",
                 cache->trash_dir->len, cache->trash_dir->data,
                 topic->len, topic->data);
        storage_mv(srcfn, dstfn);
    }
    strcpy(dstfn, srcfn);
    snprintf(srcfn, sizeof(srcfn), "%.*s/%.*s",
             cache->act_dir->len, cache->act_dir->data,
             topic->len, topic->data);
    ret = storage_mv(srcfn, dstfn);
    if (0 == ret) {
        wt->dir = cache->old_dir;
    } else {
        wtk_debug("mv %s to %s failed: %s\r\n", srcfn, dstfn, strerror(errno));
        qtk_log_log(cache->log, "move %s to %s failed: %s", srcfn, dstfn, strerror(errno));
        /*
          we have to delete file so that act_dir files limit
         */
        unlink(srcfn);
    }
    storage_remove_wtopic(cache, wt);

    return ret;
}


static int storage_new_data_buf(qtk_storage_t *cache) {
    long long fn;
    char tmp[128];
    int n;
    int wfd;
    int rfd;
    assert(cache->data_fn);
    fn = wtk_str_atoi(cache->data_fn->data, cache->data_fn->len);
    n = snprintf(tmp, sizeof(tmp), "%.*s/%08lld", cache->data_dir->len,
                 cache->data_dir->data, fn - 1);
    fn++;
    unlink(tmp);
    n = snprintf(tmp, sizeof(tmp), "%.*s/%08lld", cache->data_dir->len,
                 cache->data_dir->data, fn);
    wfd = creat_wfd(tmp);
    rfd = open_rfd(tmp);
    if (wfd != INVALID_FD && rfd != INVALID_FD) {
        if (cache->data_fd != INVALID_FD) {
            close(cache->data_fd);
        }
        cache->data_fd = wfd;
        if (cache->data_rfd != INVALID_FD) {
            close(cache->data_rfd);
        }
        cache->data_rfd = rfd;
        if (cache->data_fn) {
            wtk_string_delete(cache->data_fn);
        }
        n = snprintf(tmp, sizeof(tmp), "%08lld", fn);
        cache->data_fn = wtk_string_dup_data(tmp, n);
        cache->data_flen = 0;
        return 0;
    } else {
        return -1;
    }
}


static int storage_delete_process(void *p, wtk_thread_t *t) {
    qtk_storage_t *cache = (qtk_storage_t*)p;
    wtk_queue_node_t *node;
    char cmd[128];

    wtk_debug("delete process\r\n");
    qtk_log_log0(cache->log, "trash process is ready");
    snprintf(cmd, sizeof(cmd), "rm -rf %.*s", cache->delete_dir->len, cache->delete_dir->data);

    while ((node = wtk_blockqueue_pop(&cache->delete_q, -1, NULL))) {
        if (system(cmd)) {
            wtk_debug("remove [%.*s] error\r\n",
                      cache->delete_dir->len,
                      cache->delete_dir->data);
            qtk_log_log(cache->log, "remove [%.*s] error",
                        cache->delete_dir->len,
                        cache->delete_dir->data);
        } else {
            qtk_log_log(cache->log, "remove %.*s",
                        cache->delete_dir->len,
                        cache->delete_dir->data);
        }
        wtk_free(node);
    }

    return 0;
}


static void storage_clean_trash(qtk_storage_t *cache) {
    wtk_queue_node_t *node;
    if (wtk_file_exist(cache->delete_dir->data)) {
        /* delete_dir not exist */
        rename(cache->trash_dir->data, cache->delete_dir->data);
    } else {
        char fn[128];
        do {
            snprintf(fn, sizeof(fn), "%.*s/%d", cache->delete_dir->len,
                     cache->delete_dir->data, rand());
            qtk_mkdir_p(fn);
        } while (rename(cache->trash_dir->data, fn));
    }
    rename(cache->old_dir->data, cache->trash_dir->data);
    mkdir(cache->old_dir->data, 0777);
    node = wtk_malloc(sizeof(*node));
    wtk_queue_node_init(node);
    wtk_blockqueue_push(&cache->delete_q, node);
}


static void storage_load_act_topic(qtk_storage_t *cache) {
    DIR *d = NULL;
    struct dirent *ent;
    time_t curtime;

    d = opendir(cache->act_dir->data);
    if (!d) goto end;
    curtime = time(NULL);
    while ((ent = readdir(d))) {
        if (ent->d_type == DT_REG) {
            wtk_string_t topic;
            wtk_string_set(&topic, ent->d_name, strlen(ent->d_name));
            writable_topic_t *wt = writable_topic_new(cache, &topic);
            wt->add_time = curtime;
            qtk_hashtable_add(cache->wtopics, wt);
            wtk_queue_push(&cache->wtopic_q, &wt->anode);
        }
    }
end:
    if (d) {
        closedir(d);
    }
}


qtk_storage_t* qtk_storage_new(qtk_storage_cfg_t *cfg, qtk_log_t *log) {
    qtk_storage_t *cache = wtk_malloc(sizeof(qtk_storage_t));
    char tmp[64];
    int n;
    int max_fn;
    const char *root_dir = cfg->data_dir;

    cache->cfg = cfg;
    cache->log = log;
    cache->wtopics = qtk_str_growhash_new(cfg->wtopic_hash_size, offsetof(writable_topic_t, node));
    wtk_queue_init(&cache->wtopic_q);
    cache->act_max = 1000;
    wtk_queue_init(&cache->act_topics);
    if (strlen(root_dir) > sizeof(tmp)/2) {
        wtk_debug("root dir [%s] is too long\r\n", root_dir);
        qtk_log_log(cache->log, "root dir [%s] is too long", root_dir);
        goto err;
    }
    wtk_mkdir_p((char*)root_dir, '/', 1);
    n = snprintf(tmp, sizeof(tmp), "%s/act", root_dir);
    if (storage_mkdir(tmp)) {
        qtk_log_log(log, "mkdir '%s' failed", tmp);
        goto err;
    }
    cache->act_dir = wtk_string_dup_data_pad0(tmp, n);
    n = snprintf(tmp, sizeof(tmp), "%s/old", root_dir);
    if (storage_mkdir(tmp)) {
        qtk_log_log(log, "mkdir '%s' failed", tmp);
        goto err;
    }
    cache->old_dir = wtk_string_dup_data_pad0(tmp, n);
    n = snprintf(tmp, sizeof(tmp), "%s/trash", root_dir);
    if (storage_mkdir(tmp)) {
        qtk_log_log(log, "mkdir '%s' failed", tmp);
        goto err;
    }
    cache->trash_dir = wtk_string_dup_data_pad0(tmp, n);
    n = snprintf(tmp, sizeof(tmp), "%s/delete", root_dir);
    cache->delete_dir = wtk_string_dup_data_pad0(tmp, n);
    n = snprintf(tmp, sizeof(tmp), "%s/data", root_dir);
    if (storage_mkdir(tmp)) {
        qtk_log_log(log, "mkdir '%s' failed", tmp);
        goto err;
    }
    cache->data_dir = wtk_string_dup_data_pad0(tmp, n);
    /*
      find newest data file
     */
    max_fn = 0;
    wtk_os_dir_walk(tmp, &max_fn, (wtk_os_dir_walk_notify_f)dir_walk_max_num);
    snprintf(tmp, sizeof(tmp), "%s/data/%08d", root_dir, max_fn);
    cache->data_fd = open_wfd(tmp);
    if (INVALID_FD == cache->data_fd) goto err;
    cache->data_rfd = open_rfd(tmp);
    n = snprintf(tmp, sizeof(tmp), "%08d", max_fn);
    cache->data_fn = wtk_string_dup_data(tmp, n);
    cache->data_flen = lseek(cache->data_fd, 0, SEEK_END);

    /*
      remove register file
     */
    snprintf(tmp, sizeof(tmp), "%s/data/%s", root_dir, reg_fn);
    close(creat_wfd(tmp));
    snprintf(tmp, sizeof(tmp), "%s/act/_register", root_dir);
    remove(tmp);

    storage_load_act_topic(cache);

    wtk_blockqueue_init(&cache->delete_q);
    wtk_thread_init(&cache->delete_thread, storage_delete_process, cache);
    return cache;
err:
    wtk_debug("init cache error\r\n");
    qtk_log_log0(cache->log, "init cache error");
    if (cache) {
        qtk_storage_delete(cache);
    }
    return NULL;
}


int qtk_storage_delete(qtk_storage_t *cache) {
    writable_topic_t *wt = NULL;
    wtk_queue_node_t *node = NULL;
    while ((node = wtk_queue_peek(&cache->wtopic_q, 0))) {
        wt = data_offset(node, writable_topic_t, anode);
        storage_remove_wtopic(cache, wt);
    }
    wtk_blockqueue_clean(&cache->delete_q);
    wtk_thread_clean(&cache->delete_thread);
    qtk_hashtable_delete(cache->wtopics);
    wtk_string_delete(cache->act_dir);
    wtk_string_delete(cache->old_dir);
    wtk_string_delete(cache->trash_dir);
    wtk_string_delete(cache->delete_dir);
    wtk_string_delete(cache->data_dir);
    wtk_string_delete(cache->data_fn);
    wtk_free(cache);
    return 0;
}


int qtk_storage_start(qtk_storage_t *cache) {
    wtk_thread_start(&cache->delete_thread);
    return 0;
}


int qtk_storage_stop(qtk_storage_t *cache) {
    wtk_blockqueue_wake(&cache->delete_q);
    wtk_thread_join(&cache->delete_thread);
    wtk_thread_clean(&cache->delete_thread);
    return 0;
}


int qtk_storage_save(qtk_storage_t *cache, wtk_string_t *topic, wtk_strbuf_t *body) {
    int ret, start;
    char *buf = NULL;
    int len = 0;
    if (wtk_string_equal_s(topic, "_register")) {
        return -1;
    }
    start = cache->data_flen;
    if (start > cache->cfg->data_chunk_size && !storage_new_data_buf(cache)) {
        /*
          remove all index in trash_dir at same time
         */
        storage_clean_trash(cache);
        start = 0;
    }
    if (body) {
        buf = body->data;
        len = body->pos;
    }
    ret = write_msg_to_buf(cache, buf, len);
    if (ret) goto end;
    ret = write_topic_idx(cache, topic, start, len);
    if (0 == len) {
        /*
          move index file from act_dir to old_dir
         */
        storage_write_finish(cache, topic);
    }
end:
    return ret;
}


int qtk_storage_topic_delete(qtk_storage_t *cache, wtk_string_t *topic) {
    return storage_write_finish(cache, topic);
}


int qtk_storage_read(qtk_storage_t *cache, wtk_string_t *topic,
                     int offset, wtk_strbuf_t *buf) {
    int fds[3];
    int nfd;
    int n;
    char tmp[128];
    int start, len;
    char fn[16];
    int eof = 0;
    nfd = open_topic(cache, topic, fds, topic_line_test_offset, offset);
    if (nfd < 0) return -1;
    while (nfd) {
        char *p;
        int ret;
        n = read_topic_line(fds, &nfd, tmp, sizeof(tmp)-1);
        if (n < 0) return -1;
        if (n == 0) continue;
        tmp[n] = '\0';
        p = strchr_s(tmp, n, '\t');
        assert(p);
        ret = sscanf(p+1, "%s\t%d\t%d", fn, &start, &len);
        if (ret < 0) return -1;
        if (len == 0) {
            eof = 1;
        } else if (eof) {
            /*
              discard old data
             */
            eof = 0;
            wtk_strbuf_reset(buf);
        }
        cache_read_data(cache, fn, start, len, buf);
    }

    return eof ? 0 : 1;
}


int qtk_storage_read_chunk(qtk_storage_t *cache, const char *idx_fn, int idx,
                           wtk_strbuf_t *buf) {
    char tmp[128];
    char fn[16];
    int start, len, n, ret;
    int fd = open_rfd(idx_fn);
    n = read_line(fd, idx, tmp, sizeof(tmp));
    char *p = strchr_s(tmp, n, '\t');
    ret = sscanf(p+1, "%s\t%d\t%d", fn, &start, &len);
    if (ret < 0) return -1;
    if (len) {
        cache_read_data(cache, fn, start, len, buf);
    }
    return 0;
}


void* qtk_storage_read_idx(qtk_storage_t *cache, wtk_string_t *topic, int idx) {
    int fds[3];
    int nfd;
    topic_results_t *tr = NULL;
    nfd = open_topic(cache, topic, fds, topic_line_test_idx, idx);
    if (nfd < 0) return NULL;
    tr = topic_results_new();
    while (nfd) {
        long long idx;
        char fn[16];
        int start, len, n;
        char tmp[128];
        int ret;
        n = read_topic_line(fds, &nfd, tmp, sizeof(tmp)-1);
        if (n < 0) continue;
        tmp[n] = '\0';
        ret = sscanf(tmp, "%lld\t%s\t%d\t%d", &idx, fn, &start, &len);
        if (ret < 0) continue;
        topic_results_push(tr, idx, fn, start, len);
    }
    return tr;
}


int qtk_storage_read_pack(qtk_storage_t *cache, void *idx, wtk_strbuf_t *buf) {
    topic_results_t *tr = idx;
    topic_idx_t *ti = topic_results_pop(tr);
    if (ti) {
        return cache_read_data(cache, ti->buffer, ti->start, ti->len, buf) == ti->len ? 1 : -1;
    } else {
        return 0;
    }
}


int qtk_storage_release_idx(qtk_storage_t *cache, void *idx) {
    topic_results_delete(idx);
    return 0;
}


int qtk_storage_reg(qtk_storage_t *cache, wtk_strbuf_t *data) {
    int ret;
    int fd = INVALID_FD;
    char tmp[128];
    int n, id;
    wtk_string_t topic = wtk_string("_register");

    snprintf(tmp, sizeof(tmp), "%.*s/%s", cache->data_dir->len,
             cache->data_dir->data, reg_fn);
    fd = open_wfd(tmp);
    n = lseek(fd, 0, SEEK_END);
    ret = storage_fd_write(fd, data->data, data->pos);
    if (ret) goto end;
    close(fd);
    /*
      write idx into file
     */
    snprintf(tmp, sizeof(tmp), "%.*s/%.*s",
             cache->act_dir->len, cache->act_dir->data,
             topic.len, topic.data);
    fd = open_wfd(tmp);
    if (fd == INVALID_FD) {
        wtk_debug("open %s failed\r\n", tmp);
        qtk_log_log(cache->log, "open %s failed: %s", tmp, strerror(errno));
        ret = -1;
        goto end;
    }
    id = rand();
    n = snprintf(tmp, sizeof(tmp), "%d\t%s\t%d\t%d\n", id, reg_fn, n, data->pos);
    if (n < 0) {
        ret = -1;
        goto end;
    }
    ret = storage_fd_write(fd, tmp, n);
    assert(ret == 0);
end:
    if (INVALID_FD != fd) {
        close(fd);
    }
    return ret < 0 ? ret : id;
}


int qtk_storage_unreg(qtk_storage_t *cache, int id, const char *host) {
    char fn[1024];
    char nfn[1024];
    char tmp[128];
    char data_fn[32];
    int n;
    int fd = INVALID_FD, nfd = INVALID_FD;
    wtk_strbuf_t *buf = NULL;
    int save = 0;
    wtk_string_t topic = wtk_string("_register");

    /*
      open reg index file and create another empty reg index file
     */
    snprintf(fn, sizeof(fn), "%.*s/%.*s",
             cache->act_dir->len, cache->act_dir->data,
             topic.len, topic.data);
    fd = open_rfd(fn);
    snprintf(nfn, sizeof(nfn), "%.*s/%.*s.new",
             cache->act_dir->len, cache->act_dir->data,
             topic.len, topic.data);
    nfd = creat_wfd(nfn);
    if (fd != INVALID_FD && nfd != INVALID_FD) {
        save = 1;
    } else {
        goto end;
    }

    buf = wtk_strbuf_new(1024, 1);
    while (1) {
        int match = 0;
        int cur_id;
        n = read_next_line(fd, tmp, sizeof(tmp)-1);
        if (n < 0) {
            wtk_debug("register index file error\r\n");
            qtk_log_log(cache->log, "register index file error: %s", strerror(errno));
            break;
        }
        if (n == 0) break;      /* eof */
        cur_id = wtk_str_atoi(tmp, n);
        do {
            int idx, start, len, ret;
            cJSON *jp = NULL, *obj;
            if (cur_id != id) break;
            ret = sscanf(tmp, "%d\t%s\t%d\t%d", &idx, data_fn, &start, &len);
            if (ret < 4) break;
            wtk_strbuf_reset(buf);
            ret = cache_read_data(cache, data_fn, start, len, buf);
            if (ret < 0) break;
            wtk_strbuf_push_c(buf, '\0');
            jp = cJSON_Parse(buf->data);
            if (NULL == jp) break;
            obj = cJSON_GetObjectItem(jp, "id");
            if (obj && cJSON_String == obj->type && !strcmp(obj->valuestring, host)) {
                match = 1;
            }
            if (jp) {
                cJSON_Delete(jp);
            }
        } while (0);
        if (!match) {
            if (wtk_fd_write(nfd, tmp, n)) {
                save = 0;
                break;
            }
        }
    }
end:
    if (buf) {
        wtk_strbuf_delete(buf);
    }
    if (fd != INVALID_FD) {
        close(fd);
    }
    if (nfd != INVALID_FD) {
        close(nfd);
    }
    if (save) {
        if (rename(nfn, fn)) {
            wtk_debug("%s -> %s: %s\r\n", nfn, fn, strerror(errno));
            qtk_log_log(cache->log, "move %s to %s failed: %s", nfn, fn, strerror(errno));
        }
        return 0;
    } else {
        return -1;
    }
}
