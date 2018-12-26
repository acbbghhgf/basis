#include "qtk_director.h"
#include "wtk/core/wtk_alloc.h"
#include "wtk/core/wtk_os.h"
#include <assert.h>
#include <string.h>
#include "qtk_watcher.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/http/qtk_request.h"
#include "iris.h"
#include "qtk/core/cJSON.h"


typedef struct connection_info connection_info_t;

struct connection_info {
    qtk_hashnode_t node;
    wtk_queue_t wn_q;
    int reg;
    int sock_id;
};


static qtk_sframe_method_t *sf_method = NULL;
static void *sf_data;


static qtk_sframe_msg_t* director_new_request2(int id,
                                              const char *url,
                                              int urllen,
                                              wtk_strbuf_t *body) {
    qtk_sframe_msg_t *m;

    m = sf_method->new(sf_data, QTK_SFRAME_MSG_REQUEST, id);
    sf_method->set_method(m, HTTP_POST);
    sf_method->set_url(m, url, urllen);
    if (body) {
        sf_method->set_body(m, body);
    }
    return m;
}


static qtk_sframe_msg_t* director_new_request(int id,
                                              const char *url,
                                              wtk_strbuf_t *body) {
    return director_new_request2(id, url, strlen(url), body);
}


static qtk_sframe_msg_t* director_new_response(int id, int method, wtk_strbuf_t *body) {
    qtk_sframe_msg_t *m;

    m = sf_method->new(sf_data, QTK_SFRAME_MSG_RESPONSE, id);
    sf_method->set_status(m, 200);
    if (body) {
        sf_method->set_body(m, body);
    }
    return m;
}


static qtk_sframe_msg_t* director_new_cmd(int id, int cmd) {
    qtk_sframe_msg_t *m;

    m = sf_method->new(sf_data, QTK_SFRAME_MSG_CMD, id);
    sf_method->set_cmd(m, cmd, NULL);

    return m;
}


static int director_send_msg(qtk_sframe_msg_t *msg) {
    return sf_method->push(sf_data, msg);
}


static int qtk_redirect_handler(void *data, qtk_sframe_msg_t * msg) {
    connection_info_t *cinfo = data;
    qtk_sframe_msg_t *m;
    wtk_strbuf_t *buf;
    wtk_strbuf_t *body = NULL;
    wtk_string_t *url;
    int meth;

    buf = sf_method->get_body(msg);
    url = sf_method->get_url(msg);
    meth = sf_method->get_method(msg);
    if (buf) {
        body = wtk_strbuf_new(wtk_round(buf->pos, 16), 1);
        wtk_strbuf_push(body, buf->data, buf->pos);
    }
    m = director_new_request2(cinfo->sock_id, url->data, url->len, body);
    if (meth == HTTP_DELETE) {
        sf_method->set_method(m, HTTP_DELETE);
    }
    director_send_msg(m);
    return 0;
}
static int qtk_fetch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg);
static int qtk_watch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg);
static int qtk_unwatch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg);
static int qtk_register_handler(qtk_director_t *director, qtk_sframe_msg_t *msg);
static int qtk_unregister(qtk_director_t *director, connection_info_t *cinfo);
static int qtk_unregister_handler(qtk_director_t *director, qtk_sframe_msg_t *msg);
static qtk_buildin_watcher_t buildin_watchers[] = {
    qtk_buildin_watcher("/_fetch", qtk_fetch_handler, NULL),
    /* for client add/delete watcher */
    qtk_buildin_watcher("/_watch", qtk_watch_handler, NULL),
    qtk_buildin_watcher("/_watchuniq", qtk_watch_handler, NULL),
    qtk_buildin_watcher("/_unwatch", qtk_unwatch_handler, NULL),
    qtk_buildin_watcher("/register", qtk_register_handler, NULL),
    qtk_buildin_watcher("/unregister", qtk_unregister_handler, NULL),
};


static connection_info_t* connection_info_new(wtk_string_t *from, int sock_id) {
    connection_info_t *cinfo;
    cinfo = wtk_calloc(1, sizeof(*cinfo)+from->len+1);
    memcpy(cinfo+1, from->data, from->len);
    wtk_string_set(&cinfo->node.key._str, (char*)(cinfo+1), from->len);
    wtk_queue_init(&cinfo->wn_q);
    cinfo->reg = -1;
    cinfo->sock_id = sock_id;
    return cinfo;
}


static int connection_info_delete(connection_info_t *cinfo, qtk_director_t *director) {
    wtk_free(cinfo);
    return 0;
}


static qtk_watcher_node_t* connection_info_find(connection_info_t *cinfo,
                                                wtk_string_t *topic) {
    wtk_queue_node_t *qn;
    qtk_watcher_node_t *wn = NULL;
    for (qn = cinfo->wn_q.pop; qn; qn = qn->next) {
        wn = data_offset(qn, qtk_watcher_node_t, cnode);
        if (wtk_string_equal(topic, &wn->watcher->node.key._str)) {
            break;
        }
    }
    return wn;
}


static int qtk_parse_topic(wtk_string_t *url, wtk_string_t *topic) {
    if (url->data[0] == '/' && url->data[1] == '_') {
        int i;
        for (i = 1; i < url->len; ++i) {
            if (url->data[i] == '/') break;
        }
        wtk_string_set(topic, url->data, i);
    } else {
        wtk_string_set(topic, url->data, url->len);
    }
    return 0;
}


static qtk_watcher_t* qtk_director_get_watcher(qtk_director_t *director,
                                               wtk_string_t *topic) {
    qtk_watcher_t *watcher;
    watcher = qtk_hashtable_find(director->watchers,
                                 (qtk_hashable_t*)topic);
    return watcher;
}


static connection_info_t* qtk_director_get_cinfo(qtk_director_t *director,
                                                  wtk_string_t *addr) {
    return qtk_hashtable_find(director->conn_info, (qtk_hashable_t*)addr);
}


static connection_info_t* qtk_director_getoradd_cinfo(qtk_director_t *director,
                                                      wtk_string_t *addr,
                                                      int sock_id) {
    connection_info_t *cinfo;

    cinfo = qtk_hashtable_find(director->conn_info,
                               (qtk_hashable_t*)addr);
    if (NULL == cinfo) {
        cinfo = connection_info_new(addr, sock_id);
        qtk_hashtable_add(director->conn_info, cinfo);
        qtk_log_log(director->log, "new connection from %s\r\n", addr->data);
    }

    return cinfo;
}


static qtk_watcher_t* qtk_director_add_watcher(qtk_director_t *director,
                                               wtk_string_t *topic) {
    qtk_watcher_t *watcher;
    watcher = qtk_director_get_watcher(director, topic);
    if (NULL == watcher) {
        watcher = qtk_watcher_new(topic);
        qtk_hashtable_add(director->watchers, watcher);
    }
    return watcher;
}


static qtk_watcher_node_t* qtk_director_bind_watcher(qtk_director_t *director,
                                                     qtk_watcher_t *watcher,
                                                     wtk_string_t *from,
                                                     int sock_id, int unique) {
    connection_info_t *cinfo;
    qtk_watcher_node_t *wn;
    cinfo = qtk_director_getoradd_cinfo(director, from, sock_id);
    wn = qtk_watcher_node_new(watcher, qtk_redirect_handler, cinfo);
    if (unique) {
        wn->unique = 1;
    }
    qtk_watcher_add(watcher, wn);
    wtk_queue_push(&cinfo->wn_q, &wn->cnode);
    return wn;
}


static int qtk_director_del_watcher(qtk_director_t *director, qtk_watcher_t *watcher) {
    qtk_watcher_delete(watcher);
    return 0;
}


static int qtk_director_remove_watcher(qtk_director_t *director,
                                       qtk_watcher_t *watcher) {
    // wtk_debug("unwatch: %.*s\r\n", watcher->node.key._str.len, watcher->node.key._str.data);
    // wtk_log_log(director->log, "unwatch: %.*s\r\n", watcher->node.key._str.len, watcher->node.key._str.data);
    assert(watcher == qtk_hashtable_remove(director->watchers, &watcher->node.key));
    qtk_director_del_watcher(director, watcher);
    return 0;
}


static qtk_watcher_node_t* qtk_director_add_remote_wnode(qtk_director_t *director,
                                                         wtk_string_t *topic,
                                                         const char *from,
                                                         int sock_id,
                                                         int unique) {
    wtk_string_t sf;
    qtk_watcher_t *watcher = qtk_director_add_watcher(director, topic);
    wtk_string_set(&sf, (char*)from, strlen(from));
    return qtk_director_bind_watcher(director, watcher, &sf, sock_id, unique);
}


static int qtk_director_del_remote_wnode(qtk_director_t *director, wtk_string_t *topic,
                                         wtk_string_t *from) {
    qtk_watcher_t *watcher = qtk_director_get_watcher(director, topic);
    connection_info_t *cinfo = qtk_director_get_cinfo(director, from);
    qtk_watcher_node_t *wn = NULL;

    if (NULL == cinfo || NULL == watcher) {
        /*
          /_unwatch is the first package in connection
         */
        return 0;
    }
    /*
      try the faster way to find watcher node
     */
    if (cinfo->wn_q.length > watcher->notifier_q.length) {
        wn = qtk_watcher_find(watcher, (qtk_watch_handler_t)qtk_redirect_handler, cinfo);
        assert(!wn->buildin);
    } else {
        wn = connection_info_find(cinfo, topic);
    }
    assert(wn);
    if (NULL == wn) return -1;
    wtk_queue_remove(&cinfo->wn_q, &wn->cnode);
    qtk_watcher_remove(watcher, wn);
    if (0 == watcher->notifier_q.length) {
        qtk_director_remove_watcher(director, watcher);
    }
    return 0;
}


void topic_parse_to_cache(const char *src, char *dst, int len) {
    while (len--) {
        *dst++ = (*src == '/') ? '_' : *src;
        src++;
    }
}


void topic_parse_from_cache(const char *src, char *dst, int len) {
    while (len--) {
        *dst++ = (*src == '_') ? '/' : *src;
        src++;
    }
}


static int qtk_fetch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    qtk_fetch_push(director->fetch, msg);
    return 1;
}


static int qtk_watch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    int ret = 0;
    wtk_string_t topic;
    wtk_string_t offset;
    wtk_strbuf_t *body;
    wtk_string_t *url;
    cJSON *jp = NULL, *obj;
    qtk_sframe_method_t *method = director->method;
    void *frame = director->iris->s;
    qtk_storage_t *cache = director->cache;
    int sock_id;
    int unique;
    sock_id = method->get_id(msg);
    body = method->get_body(msg);
    if (NULL == body) goto end;
    wtk_strbuf_push_c(body, '\0');
    body->pos--;
    if (!(jp = cJSON_Parse(body->data))) goto end;
    url = method->get_url(msg);
    unique = wtk_str_equal_s(url->data, url->len, "/_watchuniq");
    for (obj = jp->child; obj; obj = obj->next) {
        qtk_sframe_msg_t *m = NULL;
        qtk_watcher_node_t *wn;
        char *address = method->get_remote_addr(frame, msg)->data;
        wtk_string_set(&topic, obj->string, strlen(obj->string));
        /* wtk_debug("watch: [%d]%.*s\r\n", topic.len, topic.len, topic.data); */
        wn = qtk_director_add_remote_wnode(director, &topic, address, sock_id, unique);
        char cache_topic[topic.len];
        topic_parse_to_cache(topic.data, cache_topic, topic.len);
        topic.data = cache_topic;
        if (obj->valuestring) {
            wtk_string_set(&offset, obj->valuestring, strlen(obj->valuestring));
        } else {
            wtk_string_set(&offset, NULL, 0);
        }
        if (wn->unique && qtk_watcher_get_uniq(wn->watcher) != wn) {
            /* dont send message to standby node of unique topic */
            wtk_string_set(&offset, NULL, 0);
        }
        if (wtk_str_start_with_s(offset.data, offset.len, "id=")) {
            int idx = wtk_str_atoi(offset.data+3, offset.len-3);
            void *v = qtk_storage_read_idx(cache, &topic, idx);
            while (1) {
                wtk_strbuf_t *buf;
                if (NULL == m) {
                    m = director_new_request(sock_id, obj->string, NULL);
                    buf = wtk_strbuf_new(1024, 1);
                    method->set_body(m, buf);
                } else {
                    buf = method->get_body(m);
                }
                wtk_strbuf_reset(buf);
                if (qtk_storage_read_pack(cache, v, buf) <= 0) {
                    break;
                }
                if (buf->pos > 0) {
                    director_send_msg(m);
                    m = NULL;
                }
            }
            qtk_storage_release_idx(cache, v);
        } else if (wtk_str_start_with_s(offset.data, offset.len, "byte=")) {
            wtk_strbuf_t *buf;
            if (NULL == m) {
                m = director_new_request(sock_id, obj->string, NULL);
                buf = wtk_strbuf_new(1024, 1);
                method->set_body(m, buf);
            } else {
                buf = method->get_body(m);
            }
            int ofs = wtk_str_atoi(offset.data+5, offset.len-5);
            int st = qtk_storage_read(cache, &topic, ofs, buf);
            if (st >= 0) {
                if (buf->pos > 0) {
                    director_send_msg(m);
                    m = NULL;
                }
                if (0 == st) {
                    if (NULL == m) {
                        m = director_new_request(sock_id, obj->string, NULL);
                    }
                    director_send_msg(m);
                    m = NULL;
                }
            }
        }
        if (m) {
            method->delete(director->iris->s, m);
        }
    }
end:
    if (jp) {
        cJSON_Delete(jp);
    }
    return ret;
}


static int qtk_unwatch_handler(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    int ret = -1;
    wtk_string_t topic, *from;
    wtk_strbuf_t *body;
    cJSON *jp = NULL, *obj;
    qtk_sframe_method_t *method = director->method;

    body = method->get_body(msg);
    if (NULL == body) goto end;
    wtk_strbuf_push_c(body, '\0');
    body->pos--;
    if (!(jp = cJSON_Parse(body->data))) goto end;
    from = method->get_remote_addr(director->iris->s, msg);
    for (obj = jp->child; obj; obj = obj->next) {
        wtk_string_set(&topic, obj->valuestring, strlen(obj->valuestring));
        qtk_director_del_remote_wnode(director, &topic, from);
    }
    ret = 0;
end:
    if (jp) {
        cJSON_Delete(jp);
    }
    return ret;
}


static int qtk_register_handler(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    int ret = -1;
    qtk_sframe_method_t *method = director->method;
    wtk_strbuf_t *body = method->get_body(msg);
    cJSON *jp = NULL;
    wtk_string_t *addr;
    char *p;

    if (NULL == body) goto end;
    wtk_strbuf_push_c(body, '\0');
    body->pos--;
    if (!(jp = cJSON_Parse(body->data)) || jp->type != cJSON_Object) {
        goto end;
    }
    addr = method->get_remote_addr(director->iris->s, msg);
    /*
      add id before forwarding, because the buildin watcher will be call first
     */
    cJSON_AddStringToObject(jp, "id", addr->data);
    p = cJSON_PrintUnformatted(jp);
    if (p) {
        qtk_sframe_msg_t *m;
        connection_info_t *cinfo;
        int plen = strlen(p);
        /*
          response with id and forward
         */
        wtk_strbuf_reset(body);
        wtk_strbuf_push(body, p, plen);
        ret = qtk_storage_reg(director->cache, body);
        if (ret >= 0) {
            int sock_id = method->get_id(msg);
            cinfo = qtk_director_getoradd_cinfo(director, addr, sock_id);
            cinfo->reg = ret;
            body = wtk_strbuf_new(wtk_round(plen, 16), 1);
            wtk_strbuf_push(body, p, plen);
            m = director_new_response(sock_id, 200, body);
            director_send_msg(m);
            ret = 0;
        }
        wtk_free(p);
    }
end:
    if (jp) {
        cJSON_Delete(jp);
    }
    return ret;
}


static int qtk_unregister(qtk_director_t *director, connection_info_t *cinfo) {
    if (cinfo->reg) {
        const char *address = cinfo->node.key._str.data;
        // wtk_debug("unregister\r\n");
        qtk_log_log(director->log, "%s unregister\r\n", address);
        qtk_storage_unreg(director->cache, cinfo->reg, address);
        cinfo->reg = -1;
    }
    return 0;
}


static int qtk_unregister_handler(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    wtk_string_t *addr = director->method->get_remote_addr(director->iris->s, msg);

    if (NULL == addr) {
        /* msg is disconnect notice, qtk_unregister is already called */
        return 0;
    }
    connection_info_t *cinfo = qtk_director_get_cinfo(director, addr);

    if (cinfo) {
        qtk_unregister(director, cinfo);
    }

    return 0;
}


static int qtk_director_request_process(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    int ret = 0;
    qtk_sframe_method_t *method = director->method;
    wtk_string_t topic;
    qtk_watcher_t *watcher;

    if (0 == qtk_parse_topic(method->get_url(msg), &topic)) {
        int meth = method->get_method(msg);
        wtk_strbuf_t *body = method->get_body(msg);
        /* wtk_debug("topic: [%d]%.*s from %s\r\n", topic.len, topic.len, topic.data, */
        /*           method->get_remote_addr(director->iris->s, msg)->data); */
        if (HTTP_PUT == meth) {
            /* if (body) { */
            /*     wtk_debug("save: %.*s\r\n", body->pos, body->data); */
            /* } */
            char tmp[topic.len];
            wtk_string_t cache_topic;
            topic_parse_to_cache(topic.data, tmp, topic.len);
            wtk_string_set(&cache_topic, tmp, topic.len);
            qtk_storage_save(director->cache, &cache_topic, body);
        } else if (HTTP_DELETE == meth) {
            /*
              DELETE TOPIC DATA
             */
            char tmp[topic.len];
            wtk_string_t cache_topic;
            topic_parse_to_cache(topic.data, tmp, topic.len);
            wtk_string_set(&cache_topic, tmp, topic.len);
            qtk_storage_topic_delete(director->cache, &cache_topic);
        }
        watcher = qtk_director_get_watcher(director, &topic);
        if (watcher) {
            ret = qtk_watcher_handle(watcher, msg);
        }
    }

    return ret;
}


static void qtk_director_del_conninfo(qtk_director_t *director,
                                      connection_info_t *cinfo) {
    wtk_queue_node_t *node;

    if (cinfo->reg >= 0) {
        char tmp[128];
        int n;
        qtk_sframe_msg_t *msg;
        qtk_watcher_t *watcher;
        wtk_string_t topic = wtk_string("/unregister");
        const char *address = cinfo->node.key._str.data;
        wtk_strbuf_t *body = wtk_strbuf_new(1024, 1);
        qtk_unregister(director, cinfo);
        n = snprintf(tmp, sizeof(tmp), "{\"id\":\"%s\"}", address);
        wtk_strbuf_push(body, tmp, n);
        msg = director_new_request(cinfo->sock_id, "/unregister", body);
        watcher = qtk_director_get_watcher(director, &topic);
        if (watcher) {
            qtk_watcher_handle(watcher, msg);
        }
        director->method->delete(director->iris->s, msg);
    }

    while ((node = wtk_queue_pop(&cinfo->wn_q))) {
        qtk_watcher_node_t *wn = data_offset(node, qtk_watcher_node_t, cnode);
        qtk_watcher_t *watcher = wn->watcher;
        qtk_watcher_remove(watcher, wn);
        if (0 == watcher->notifier_q.length) {
            qtk_director_remove_watcher(director, watcher);
        }
    }
    connection_info_delete(cinfo, director);
}


static void qtk_director_notice_process(qtk_director_t *director, qtk_sframe_msg_t *msg) {
    int sig = director->method->get_signal(msg);
    wtk_string_t *from;
    connection_info_t *cinfo;

    switch (sig) {
    case QTK_SFRAME_SIGCONNECT:
        from = director->method->get_remote_addr(director->iris->s, msg);
        wtk_debug("accept %s\r\n", from->data);
        qtk_log_log(director->log, "accept connection '%s'", from->data);
        break;
    case QTK_SFRAME_SIGDISCONNECT:
        from = director->method->get_remote_addr(director->iris->s, msg);
        cinfo = qtk_hashtable_remove(director->conn_info, (qtk_hashable_t*)from);
        if (cinfo) {
            // wtk_debug("remove cinfo %s\r\n", from->data);
            qtk_log_log(director->log, "remove connection %s\r\n", from->data);
            qtk_director_del_conninfo(director, cinfo);
        }
    case QTK_SFRAME_SIGECONNECT:
        director->method->push(director->iris->s,
                               director_new_cmd(msg->id, QTK_SFRAME_CMD_CLOSE));
        break;
    default:
        wtk_debug("unknown signal : %d\r\n", sig);
        qtk_log_log(director->log, "unknown signal: %d", sig);
        break;
    }
}


static int qtk_director_route(void *data, wtk_thread_t *t) {
    qtk_director_t *director = data;
    qtk_sframe_method_t *method = director->method;
    qtk_sframe_msg_t *msg;
    qtk_iris_t *iris = director->iris;
    qtk_log_log(director->log, "director thread[%u] is ready", t->ppid);
    while (iris->run) {
        msg = method->pop(iris->s, 1000);
        if (NULL == msg) continue;
        switch(method->get_type(msg)) {
        case QTK_SFRAME_MSG_REQUEST:
            if (1 == qtk_director_request_process(director, msg)) {
                msg = NULL;
            }
            break;
        case QTK_SFRAME_MSG_NOTICE:
            qtk_director_notice_process(director, msg);
            break;
        default:
            break;
        }
        if (msg) {
            method->delete(iris->s, msg);
        }
    }
    qtk_log_log(director->log, "director thread[%u] exit", t->ppid);
    return 0;
}


static int qtk_director_load_buildin_watcher(qtk_director_t *director) {
    int i;
    for (i = 0; i < sizeof(buildin_watchers)/sizeof(buildin_watchers[0]); i++) {
        qtk_watcher_t *watcher;
        watcher = qtk_director_get_watcher(director, &buildin_watchers[i].topic);
        if (NULL == watcher) {
            watcher = qtk_watcher_new(&buildin_watchers[i].topic);
            qtk_hashtable_add(director->watchers, watcher);
        }
        buildin_watchers[i].watcher.watcher = watcher;
        if (NULL == buildin_watchers[i].watcher.data) {
            buildin_watchers[i].watcher.data = director;
        }
        qtk_watcher_add(watcher, &buildin_watchers[i].watcher);
    }
    return 0;
}


qtk_director_t* qtk_director_new(qtk_director_cfg_t *cfg, qtk_log_t *log) {
    qtk_director_t *director = wtk_malloc(sizeof(qtk_director_t));
    director->cfg = cfg;
    director->log = log;
    director->fetch = qtk_fetch_new(director);
    director->cache = qtk_storage_new(&cfg->storage, log);
    director->watchers = qtk_str_growhash_new(128, offsetof(qtk_watcher_t, node));
    director->conn_info = qtk_str_growhash_new(128, offsetof(connection_info_t, node));
    qtk_director_init(director);
    return director;
}


int qtk_director_init(qtk_director_t *director) {
    wtk_thread_init(&director->thread, qtk_director_route, director);
    qtk_director_load_buildin_watcher(director);
    return 0;
}


int qtk_director_delete(qtk_director_t *director) {
    wtk_thread_clean(&director->thread);
    if (director->conn_info) {
        qtk_hashtable_delete2(director->conn_info,
                             (wtk_walk_handler_t)qtk_director_del_conninfo,
                             director);
    }
    if (director->watchers) {
        qtk_hashtable_delete2(director->watchers,
                              (wtk_walk_handler_t)qtk_director_del_watcher,
                              director);
    }
    if (director->cache) {
        qtk_storage_delete(director->cache);
    }
    if (director->fetch) {
        qtk_fetch_delete(director->fetch);
    }
    wtk_free(director);
    return 0;
}


int qtk_director_start(qtk_director_t *director) {
    qtk_log_log0(director->log, "director start\r\n");
    wtk_thread_start(&director->thread);
    qtk_fetch_start(director->fetch);
    qtk_storage_start(director->cache);
    return 0;
}


int qtk_director_stop(qtk_director_t *director) {
    qtk_log_log0(director->log, "director stop\r\n");
    qtk_fetch_stop(director->fetch);
    qtk_storage_stop(director->cache);
    director->run = 0;
    return 0;
}


int qtk_director_join(qtk_director_t *director) {
    qtk_log_log0(director->log, "wait thread exit...\r\n");
    wtk_thread_join(&director->thread);
    return 0;
}


void qtk_director_set_method(qtk_director_t *director,
                             void *frame) {
    director->method = frame;
    sf_method = frame;
    sf_data = frame;
}
