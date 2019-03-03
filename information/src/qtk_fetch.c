#include "qtk_fetch.h"
#include "qtk_director.h"
#include "qtk/core/qtk_deque.h"


/*
  calculate number of discontinuous slash, exclude the last slash
 */
static int str_slash_num(const char *s) {
    const char *p = strchr(s, '/');
    int n = 0;
    while (*(++p) == '/');
    while (p) {
        n++;
        p = strchr(p, '/');
        if (NULL == p) break;
        while (*(++p) == '/');
    }
    return n;
}


static void str_to_buf(wtk_string_t *str, char *buf) {
    if (str) {
        memcpy(buf, str->data, str->len);
        buf[str->len] = '\0';
    } else {
        buf[0] = '\0';
    }
}


static int _shell_cmd(const char *cmd, wtk_strbuf_t *buf) {
    FILE *fp;
    char tmp[256];
    int n;
    fp = popen(cmd, "r");
    if (fp == NULL) return -1;
    while ((n = fread(tmp, 1, sizeof(tmp), fp)) > 0) {
        wtk_strbuf_push(buf, tmp, n);
    }
    pclose(fp);
    return 0;
}


static int _read_file(const char *dir, const char *fn, int start, int len, wtk_strbuf_t *buf) {
    char tmpbuf[1024];
    if (dir) {
        if (snprintf(tmpbuf, sizeof(tmpbuf), "%s/%s", dir, fn) > 0) {
            fn = tmpbuf;
        } else {
            return -1;
        }
    }
    int fd = open(fn, O_RDONLY);
    if (fd != INVALID_FD) {
        int n;
        if (lseek(fd, start, SEEK_SET) < 0) {
            close(fd);
            return 0;
        }
        if (len >= 0) {
            while ((n = read(fd, tmpbuf, sizeof(tmpbuf))) > 0 && len > 0) {
                if (len < n) n = len;
                len -= n;
                wtk_strbuf_push(buf, tmpbuf, n);
            }
        } else {
            while ((n = read(fd, tmpbuf, sizeof(tmpbuf))) > 0) {
                wtk_strbuf_push(buf, tmpbuf, n);
            }
        }
        close(fd);
        return 0;
    }
    return -1;
}


static int _dir_list(const char *dir, const char *param, wtk_strbuf_t *buf) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ls -t %s/%s_*", dir, param);
    return _shell_cmd(cmd, buf);
}


static int _read_idx(const char *dir, const char *param, wtk_strbuf_t *buf) {
    return _read_file(dir, param, 0, -1, buf);
}


static int qtk_fetch_list(qtk_fetch_t *fetch, char *param, int id) {
    qtk_director_t *director = fetch->director;
    wtk_strbuf_t *res = wtk_strbuf_new(1024, 1);
    qtk_sframe_method_t *method = director->method;
    qtk_sframe_msg_t *msg;
    int paramlen = strlen(param);
    topic_parse_to_cache(param, param, paramlen);
    _dir_list(director->cache->act_dir->data, param, res);
    _dir_list(director->cache->old_dir->data, param, res);
    _dir_list(director->cache->trash_dir->data, param, res);
    msg = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
    method->set_status(msg, 200);

    /* format */
    char *s = res->data;
    char *e;
    char *cpy = res->data;
    while ((e = strstr(s, param))) {
        e += paramlen + 1;
        s = e;
        e = strchr(s, '\n');
        if (NULL == e) break;
        topic_parse_from_cache(s, s, e - s);
        e++;
        memmove(cpy, s, e - s);
        cpy += e - s;
        s = e;
    }
    res->pos = cpy - res->data;
    method->set_body(msg, res);
    method->push(method, msg);

    return 0;
}


static const char* _parse_idx_last_zero(const char *buf, int len) {
    const char *s, *e = buf+len;
    s = e;
    while (s >= buf) {
        s = e - 1;
        /* get a line between s and e */
        while (s > buf && *(--s) != '\n');
        long long id;
        char fn[16];
        int start, length;
        int ret = sscanf(s, "%lld\t%s\t%d\t%d", &id, fn, &start, &length);
        if (ret < 4) return NULL;
        if (0 == length) return s;
        e = s;
    }
    return NULL;
}


static int _read_chunk(const char *dir, const char *fn, int start, int length, wtk_strbuf_t *buf) {
    return _read_file(dir, fn, start, length, buf);
}


static char *_read_data(const char *dir, const char *idx, int len, wtk_strbuf_t *buf) {
    const char *s = idx, *e = idx+len;
    while (s < e) {
        long long id;
        char fn[16];
        int start, length;
        int ret = sscanf(s, "%lld\t%s\t%d\t%d", &id, fn, &start, &length);
        if (ret < 4) return NULL;
        if (length) {
            _read_chunk(dir, fn, start, length, buf);
        }
        while (s < e && *(s++) != '\n');
    }
    return 0;
}


static int qtk_fetch_data(qtk_fetch_t *fetch, char *param, int id) {
    qtk_director_t *director = fetch->director;
    wtk_strbuf_t *buf = wtk_strbuf_new(1024, 1);
    wtk_strbuf_t *res = wtk_strbuf_new(1024, 1);
    qtk_deque_t *dq = qtk_deque_new(1024, 1024*1024, 1);
    qtk_sframe_method_t *method = director->method;
    qtk_sframe_msg_t *msg;
    int paramlen = strlen(param);
    topic_parse_to_cache(param, param, paramlen);
    _read_idx(director->cache->act_dir->data, param, buf);
    _read_idx(director->cache->old_dir->data, param, buf);
    _read_idx(director->cache->trash_dir->data, param, buf);
    const char *e = _parse_idx_last_zero(buf->data, buf->pos);
    if (e) {
        const char *s = _parse_idx_last_zero(buf->data, e - buf->data);
        if (NULL == s) {
            s = buf->data;
        }
        _read_data(director->cache->data_dir->data, s, e - s, res);
    } else {
        /* no chunk found, read last chunk */
        e = buf->data + buf->pos;
        e--;                    /* last is '\n', skip it */
        while (e > buf->data && *(--e) != '\n');
        long long id;
        char fn[16];
        int start, length;
        int ret = sscanf(e, "%lld\t%s\t%d\t%d", &id, fn, &start, &length);
        if (ret == 4) {
            _read_chunk(director->cache->data_dir->data, fn, start, length, res);
        }
    }
    msg = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
    method->set_status(msg, 200);
    /* wtk_debug("%d\r\n", res->pos); */
    method->set_body(msg, res);
    method->push(method, msg);

    if (buf) {
        wtk_strbuf_delete(buf);
    }
    qtk_deque_delete(dq);

    return 0;
}


static int qtk_fetch_req_process(void *data, wtk_thread_t *thread) {
    qtk_fetch_t *fetch = (qtk_fetch_t*)data;
    qtk_director_t *director = fetch->director;
    qtk_sframe_method_t *method = director->method;
    wtk_queue_node_t *node;
    char buf[1024];

    while ((node = wtk_blockqueue_pop(&fetch->req_q, -1, NULL))) {
        qtk_sframe_msg_t *msg = data_offset(node, qtk_sframe_msg_t, q_n);
        str_to_buf(method->get_url(msg), buf);
        int n = str_slash_num(buf);
        int id = method->get_id(msg);
        if (n < 3) {
            /* invalid request */
            qtk_sframe_msg_t *m = method->new(method, QTK_SFRAME_MSG_RESPONSE, id);
            method->set_status(m, 403);
            method->add_body(m, "Invalid Request", 15);
            method->push(method, m);
        } else if (n < 5) {
            /* list topic of device or hook */
            qtk_fetch_list(fetch, strchr(buf+1, '/'), id);
        } else {
            qtk_fetch_data(fetch, strchr(buf+1, '/'), id);
        }
        method->delete(method, msg);
    }
    return 0;
}


qtk_fetch_t* qtk_fetch_new(qtk_director_t *director) {
    qtk_fetch_t* fetch = wtk_malloc(sizeof(*fetch));
    fetch->director = director;
    wtk_thread_init(&fetch->thread, qtk_fetch_req_process, fetch);
    return fetch;
}


int qtk_fetch_delete(qtk_fetch_t *fetch) {
    wtk_thread_clean(&fetch->thread);
    wtk_free(fetch);
    return 0;
}


int qtk_fetch_start(qtk_fetch_t *fetch) {
    fetch->run = 1;
    wtk_thread_start(&fetch->thread);
    return 0;
}


int qtk_fetch_stop(qtk_fetch_t *fetch) {
    fetch->run = 0;
    wtk_blockqueue_wake(&fetch->req_q);
    wtk_thread_join(&fetch->thread);
    return 0;
}


int qtk_fetch_push(qtk_fetch_t *fetch, qtk_sframe_msg_t *msg) {
    return wtk_blockqueue_push(&fetch->req_q, &msg->q_n);
}
