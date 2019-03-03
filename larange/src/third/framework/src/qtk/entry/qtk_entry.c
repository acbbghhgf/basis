#include <netdb.h>
#include "qtk_entry.h"
#include "qtk/http/qtk_request.h"
#include "qtk/http/qtk_response.h"
#include "qtk/http/qtk_http_parser.h"
#include "qtk/entry/qtk_socket_server.h"
#include "qtk/core/qtk_atomic.h"
#include "qtk/entry/qtk_request_pool.h"

/* ----------------------------------- test section ----------------------------- */

#define TIMER_DELAY     2000
#define GET_WORKER(a, n) (((qtk_socket_server_t**)(a)->slot)[n])


int qtk_entry_bytes(qtk_entry_t *entry)
{
    int bytes = sizeof(qtk_entry_t);
    bytes += wtk_heap_bytes(entry->heap);
    bytes += wtk_str_hash_bytes(entry->loc_hash);
    /* workers bytes not implement */
    /* for (i = 0; i < entry->nworker; i++) { */
    /*     bytes += GET_WORKER(entry->workers, i)->bytes; */
    /* } */

    return bytes;
}

/* static int qtk_entry_show_bytes(qtk_entry_t *entry) */
/* { */
/*     int bytes = 0; */
/*     int heap_bytes = 0; */

/*     bytes = qtk_entry_bytes(entry); */
/*     heap_bytes = wtk_heap_bytes(entry->heap); */

/*     printf("************** entry use [%d] *************\n", bytes); */
/*     printf("heap use %d\n", heap_bytes); */
/*     printf("*******************************************\n"); */

/*     return 0; */
/* } */

/* -------------------------------------------------------------------------- */


int qtk_entry_do_statistic(qtk_entry_t *entry, qtk_response_t *rep) {
    int req_used = 0, req_free = 0;
    int ev_used = 0, ev_free = 0;
    int conn_used = 0, conn_free = 0;
    char buf[1024];
    char *p;
    int n;

    qtk_response_add_hdr_s(rep, "content-type",
                           wtk_heap_dup_string_s(rep->heap, "text/plain"));
    /* socket_server statistics not implement */
    /* for (i = 0; i < entry->nworker; i++) { */
    /*     ss = GET_WORKER(entry->workers, i); */
    /*     req_used += receiver->request_hoard->use_length; */
    /*     req_free += receiver->request_hoard->cur_free; */
    /*     ev_used += receiver->event_hoard->use_length; */
    /*     ev_free += receiver->event_hoard->cur_free; */
    /*     conn_used += receiver->nk->con_hoard->use_length; */
    /*     conn_free += receiver->nk->con_hoard->cur_free; */
    /* } */
    n = sprintf(buf, "{\"event\":{\"used\":%d,\"free\":%d},"
                "\"request\":{\"used\":%d,\"free\":%d},"
                "\"connection\":{\"used\":%d,\"free\":%d}}",
                ev_used, ev_free, req_used, req_free, conn_used, conn_free);
    p = wtk_heap_malloc(rep->heap, n);
    memcpy(p, buf, n);
    qtk_response_set_body(rep, p, n);
    return 0;
}


int qtk_entry_init_loc(qtk_entry_t *entry, qtk_loc_meta_data_t *locs, int sz) {
    int i;
    for (i = 0; i < sz; ++i) {
        wtk_str_hash_add(entry->loc_hash, locs[i].key.data, locs[i].key.len, locs+i);
    }
    return 0;
}


int qtk_entry_loc_dispatch(qtk_entry_t *entry, const char *key, int keybytes, qtk_response_t *rep) {
    qtk_loc_meta_data_t *loc_data = wtk_str_hash_find(entry->loc_hash, (char*)key, keybytes);
    if (NULL == loc_data) return -1;
    return loc_data->process(loc_data->udata, rep);
}


static qtk_socket_t* _entry_get_socket(qtk_entry_t *entry, int id) {
    return &entry->slot[id % entry->slot_cap];
}


qtk_socket_t* qtk_entry_get_socket(qtk_entry_t *entry, int id) {
    qtk_socket_t *s = _entry_get_socket(entry, id);
    if (s->state == SOCKET_INVALID) {
        return NULL;
    } else {
        return s;
    }
}


static qtk_socket_server_t* qtk_entry_select_server(qtk_entry_t *entry) {
    qtk_socket_server_t *ss;
    if (entry->nworker == 1) {
        ss = GET_WORKER(entry->workers, 0);
    } else {
        int i = rand() % entry->nworker;
        ss = GET_WORKER(entry->workers, i);
    }
    return ss;
}


static int qtk_entry_socket_cmd_process(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    qtk_socket_t *s = qtk_entry_get_socket(entry, msg->id);
    if (NULL == s) return -1;
    if (s->state != SOCKET_RESERVE) {
        wtk_debug("expect socket state is %d, actual is %d\r\n", SOCKET_RESERVE, s->state);
        return -1;
    }
    switch (msg->signal) {
    case QTK_SFRAME_CMD_OPEN:
    {
        qtk_sframe_connect_param_t *con_param = msg->info.con_param;
        if (NULL == con_param) {
            wtk_debug("connect param is invalid: %p\r\n", con_param);
            return -1;
        }
        int fd = qtk_socket_connect(s,
                                    (struct sockaddr*)&con_param->saddr,
                                    sizeof(con_param->saddr));
        if (fd == INVALID_FD) {
            qtk_sframe_msg_t *msg2;
            wtk_string_t *addr_s = qtk_socket_ntop(&con_param->saddr);
            wtk_log_log(entry->log, "connect to %s failed", addr_s->data);
            wtk_string_delete(addr_s);
            msg2 = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGECONNECT, msg->id);
            qtk_entry_raise_msg(entry, msg2);
        } else {
            if (s->pid > 0) {
                /* use request pool's server as socket's server */
                qtk_request_pool_t *pool = qtk_entry_get_request_pool(entry, s->pid);
                /* if pool is nil, do nothing, the socket will be closed by pool release */
                if (pool) {
                    qtk_socket_init(s, fd, pool->server);
                }
            } else {
                /* select random socket server */
                qtk_socket_init(s, fd, qtk_entry_select_server(entry));
            }
            s->reconnect = con_param->reconnect;
        }
        wtk_free(con_param);
        msg->info.con_param = NULL;
        break;
    }
    case QTK_SFRAME_CMD_LISTEN:
    {
        qtk_sframe_listen_param_t *lis_param = msg->info.lis_param;
        if (NULL == lis_param) return -1;
        if (!qtk_socket_listen(s,
                               (struct sockaddr*)&lis_param->saddr,
                               sizeof(lis_param->saddr),
                               lis_param->backlog)) {
            int i;
            for (i = 0; i < entry->nworker; ++i) {
                qtk_socket_server_t *ss = GET_WORKER(entry->workers, i);
                qtk_ss_add_listen(ss, s->id);
            }
        }
        wtk_free(lis_param);
        msg->info.lis_param = NULL;
        break;
    }
    default:
        break;
    }
    return 0;
}


static int qtk_entry_pool_cmd_process(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    qtk_request_pool_t *pool = qtk_entry_get_request_pool(entry, -msg->id);
    if (NULL == pool) return -1;
    if (msg->signal == QTK_SFRAME_CMD_OPEN_POOL) {
        qtk_sframe_connect_param_t *con_param = msg->info.con_param;
        if (NULL == con_param) return -1;
        /* reconnect reused for nslot */
        int nslot = con_param->reconnect;
        qtk_socket_server_t *ss = qtk_entry_select_server(entry);
        /*
          before qtk_request_pool_init, write pool is thread_safe
          because pool's event is not initialized
        */
        if (qtk_request_pool_init(pool, nslot, ss)) {
            msg = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGDISCONNECT, -pool->id);
            qtk_entry_raise_msg(entry, msg);
        } else {
            /* forward to request pool */
            qtk_request_pool_push(pool, msg);
            return 1;
        }
    } else {
        qtk_request_pool_push(pool, msg);
        return 1;
    }
    return 0;
}


static int qtk_entry_cmd_process(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    if (msg->id >= 0) {
        return qtk_entry_socket_cmd_process(entry, msg);
    } else {
        return qtk_entry_pool_cmd_process(entry, msg);
    }
}


static int qtk_entry_cmd_route(void *data, wtk_thread_t *t) {
    qtk_entry_t *entry = data;
    wtk_queue_node_t *node;
    qtk_sframe_msg_t *msg;

    while (entry->run) {
        node = wtk_blockqueue_pop(&entry->cmd_q, -1, NULL);
        if (NULL == node) continue;
        msg = data_offset(node, qtk_sframe_msg_t, q_n);
        if (msg->type == QTK_SFRAME_MSG_CMD) {
            /* 1 if msg is forwarded */
            if (1 != qtk_entry_cmd_process(entry, msg)) {
                entry->frame->method.delete((qtk_sframe_method_t *)entry->frame, msg);
            }
        }
    }
    return 0;
}


int qtk_entry_alloc_id(qtk_entry_t *entry, int pid) {
    int i;
    int id;
    for (i = 0; i < entry->slot_cap; ++i) {
        id = QTK_ATOM_INC(&entry->alloc_id);
        if (id < 0) {
            id &= 0x7fffffff;
        }
        qtk_socket_t *s = _entry_get_socket(entry, id);
        if (SOCKET_INVALID == s->state) {
            if (QTK_ATOM_CAS(&s->state, SOCKET_INVALID, SOCKET_RESERVE)) {
                s->id = id;
                s->fd = INVALID_FD;
                s->pid = pid;
                return id;
            }
        }
    }
    return -1;
}


static qtk_request_pool_t* _get_request_pool(qtk_entry_t *entry, int id) {
    return &entry->pool_slot[id % entry->pool_cap];
}


qtk_request_pool_t* qtk_entry_get_request_pool(qtk_entry_t *entry, int id) {
    qtk_request_pool_t *pool = _get_request_pool(entry, id);
    if (pool->id == id && pool->state != SOCKET_INVALID) {
        return pool;
    } else {
        return NULL;
    }
}


int qtk_entry_alloc_pool(qtk_entry_t *entry) {
    int i;
    int id;
    for (i = 0; i < entry->pool_cap; ++i) {
        id = QTK_ATOM_INC(&entry->alloc_pid);
        if (id <= 0) {
            id = 1;
        }
        qtk_request_pool_t *pool = _get_request_pool(entry, id);
        if (SOCKET_INVALID == pool->state) {
            if (QTK_ATOM_CAS(&pool->state, SOCKET_INVALID, SOCKET_RESERVE)) {
                pool->id = id;
                return id;
            }
        }
    }
    return -1;
}


qtk_entry_t *qtk_entry_new(qtk_entry_cfg_t *cfg, qtk_fwd_t *frame, wtk_log_t *log)
{
    qtk_entry_t *entry = NULL;

    entry = wtk_malloc(sizeof(qtk_entry_t));
    entry->heap = wtk_heap_new(1024);
    entry->cfg = cfg;
    entry->log = log;
    entry->frame = frame;
    entry->workers = wtk_array_new_h(entry->heap, cfg->worker, sizeof(void*));
    entry->nworker = cfg->worker;
    entry->loc_hash = wtk_str_hash_new(20);
    entry->output_q = &frame->entry_to_uplayer;

    entry->slot_cap = cfg->slot_size;
    entry->slot = wtk_calloc(cfg->slot_size, sizeof(entry->slot[0]));
    entry->alloc_id = 0;

    entry->pool_cap = cfg->request_pool_size;
    entry->pool_slot = wtk_calloc(cfg->request_pool_size, sizeof(entry->pool_slot[0]));
    entry->alloc_pid = 0;

    qtk_entry_init(entry);

    return entry;
}

int qtk_entry_init(qtk_entry_t *entry)
{
    int ret = 0;
    int i = 0;
    qtk_socket_server_t *ss;
    qtk_entry_cfg_t *cfg = entry->cfg;

    for (i = 0; i < entry->slot_cap; i++) {
        entry->slot[i].state = SOCKET_INVALID;
        entry->slot[i].id = -1;
    }

    wtk_blockqueue_init(&entry->cmd_q);
    for(i = 0; i < entry->nworker; i++) {
        ss = qtk_socket_server_new(&cfg->socket_server, entry);
        wtk_array_push2(entry->workers, &ss);
    }
    wtk_thread_init(&entry->cmd_thread, qtk_entry_cmd_route, entry);

    return ret;
}

int qtk_entry_delete(qtk_entry_t *entry)
{
    int i = 0;

    for (i = 0; i < entry->pool_cap; i++) {
        if (entry->pool_slot[i].state != SOCKET_INVALID) {
            qtk_request_pool_clean(&entry->pool_slot[i]);
        }
    }

    for (i = 0; i < entry->slot_cap; i++) {
        if (entry->slot[i].state != SOCKET_INVALID) {
            qtk_socket_clean(&entry->slot[i]);
        }
    }

    wtk_thread_clean(&entry->cmd_thread);

    wtk_blockqueue_clean(&entry->cmd_q);

    for (i = 0; i < entry->nworker; i++) {
        qtk_socket_server_delete(GET_WORKER(entry->workers, i));
    }
    wtk_free(entry->pool_slot);
    wtk_free(entry->slot);
    wtk_array_dispose(entry->workers);

    wtk_heap_delete(entry->heap);

    wtk_str_hash_delete(entry->loc_hash);

    wtk_free(entry);

    return 0;
}

int qtk_entry_start(qtk_entry_t *entry)
{
    int ret = 0;
    int i = 0;

    entry->run = 1;
    wtk_thread_start(&entry->cmd_thread);
    for(i = 0; i < entry->nworker; i++) {
        ret = qtk_socket_server_start(GET_WORKER(entry->workers, i));
    }

    return ret;
}

int qtk_entry_stop(qtk_entry_t *entry)
{
    int i = 0;

    entry->run = 0;
    wtk_blockqueue_wake(&entry->cmd_q);
    wtk_thread_join(&entry->cmd_thread);
    for(i=0; i<entry->nworker; i++) {
        qtk_socket_server_stop(GET_WORKER(entry->workers, i));
    }

    return 0;
}


static int qtk_entry_push_pool_msg(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    if (msg->type == QTK_SFRAME_MSG_CMD) {
        return wtk_blockqueue_push(&entry->cmd_q, &msg->q_n);
    } else {
        int pid = -msg->id;
        qtk_request_pool_t *pool = qtk_entry_get_request_pool(entry, pid);
        return qtk_request_pool_push(pool, msg);
    }
}


int qtk_entry_push_msg(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    if (msg->id < 0) {
        return qtk_entry_push_pool_msg(entry, msg);
    }
    qtk_socket_t *s = qtk_entry_get_socket(entry, msg->id);
    if (NULL == s) {
        wtk_debug("socket[%d] is already closed, msg(%d,%x) is discard\r\n",
                  msg->id, (int)msg->type, (int)msg->signal);
        if (msg->type == QTK_SFRAME_MSG_REQUEST) {
            qtk_request_print(msg->info.request);
        } else if (msg->type == QTK_SFRAME_MSG_RESPONSE) {
            qtk_response_print(msg->info.response);
        }
        entry->frame->method.delete((qtk_sframe_method_t *)entry->frame, msg);
        return -1;
    }
    if (s->server) {
        qtk_ss_push_msg(s->server, msg);
    } else {
        wtk_blockqueue_push(&entry->cmd_q, &msg->q_n);
    }
    return 0;
}


int qtk_entry_raise_msg(qtk_entry_t *entry, qtk_sframe_msg_t *msg) {
    if (msg->id < 0) {
        /* from request_pool, to uplayer */
        return wtk_blockqueue_push(entry->output_q, &msg->q_n);
    }
    qtk_socket_t *s = qtk_entry_get_socket(entry, msg->id);
    if (NULL == s) {
        entry->frame->method.delete(&entry->frame->method, msg);
        return -1;
    }
    if (s->pid) {
        /* from socket, to request_pool */
        qtk_request_pool_t *pool = qtk_entry_get_request_pool(entry, s->pid);
        if (pool) {
            qtk_request_pool_response(pool, msg);
            return 0;
        } else {
            entry->frame->method.delete(&entry->frame->method, msg);
            return -1;
        }
    } else {
        /* from socket, to uplayer */
        return wtk_blockqueue_push(entry->output_q, &msg->q_n);
    }
}
