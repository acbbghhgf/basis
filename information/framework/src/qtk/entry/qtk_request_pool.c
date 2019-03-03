#include <assert.h>
#include "qtk_request_pool.h"
#include "qtk/entry/qtk_entry.h"
#include "qtk/entry/qtk_socket_server.h"


static qtk_socket_slot_t* _get_socket(qtk_request_pool_t *pool, int id) {
    int i = 0;
    for (; i < pool->nslot; ++i) {
        if (pool->slots[i].sock_id == id) {
            return &pool->slots[i];
        }
    }
    return NULL;
}


static qtk_socket_slot_t *_new_socket(qtk_request_pool_t *pool) {
    if (NULL == pool->con_param) {
        return NULL;
    }
    if (pool->nslot < pool->slot_cap) {
        int sock_id = qtk_entry_alloc_id(pool->server->entry, pool->id);
        int i = pool->nslot++;
        qtk_sframe_msg_t *msg;
        pool->slots[i].sock_id = sock_id;
        pool->slots[i].state = SOCKET_CONNECTING;
        pool->slots[i].req = NULL;
        msg = qtk_fwd_cmd_msg(pool->server->entry->frame, QTK_SFRAME_CMD_OPEN, sock_id);
        msg->info.con_param = wtk_malloc(sizeof(*pool->con_param));
        *msg->info.con_param = *pool->con_param;
        qtk_entry_push_msg(pool->server->entry, msg);
        return &pool->slots[i];
    }
    return NULL;
}


static qtk_socket_slot_t* _find_free_socket(qtk_request_pool_t *pool) {
    int i = 0;
    for (; i < pool->nslot; ++i) {
        if (SOCKET_CONNECTED == pool->slots[i].state
            && NULL == pool->slots[i].req) {
            return &pool->slots[i];
        }
    }
    return NULL;
}


static qtk_socket_slot_t* _get_free_socket(qtk_request_pool_t *pool) {
    qtk_socket_slot_t *slot = _find_free_socket(pool);
    if (NULL == slot) {
        _new_socket(pool);
    }
    return slot;
}


static qtk_socket_slot_t* _get_free_or_new_socket(qtk_request_pool_t *pool) {
    qtk_socket_slot_t *slot = _find_free_socket(pool);
    if (NULL == slot) {
        slot = _new_socket(pool);
    }
    return slot;
}


static void _del_socket(qtk_request_pool_t *pool, qtk_socket_slot_t *slot) {
    qtk_socket_slot_t *e = pool->slots + pool->nslot - 1;
    qtk_entry_t *entry = pool->server->entry;
    qtk_sframe_msg_t *msg;
    /* request must be release before */
    assert(NULL == slot->req);
    assert(slot >= pool->slots && slot <= e);
    msg = qtk_fwd_cmd_msg(entry->frame, QTK_SFRAME_CMD_CLOSE, slot->sock_id);
    qtk_entry_push_msg(entry, msg);
    if (slot < e) {
        /* move *e to *slot */
        *slot = *e;
    }
    pool->nslot--;
}


/* pop to socket */
static qtk_sframe_msg_t* qtk_request_pool_pop(qtk_request_pool_t *pool) {
    wtk_queue_node_t *node;
    node = wtk_lockqueue_pop((wtk_lockqueue_t*)&pool->req_q);
    return data_offset(node, qtk_sframe_msg_t, q_n);
}


static int qtk_request_pool_open(qtk_request_pool_t *pool, qtk_sframe_connect_param_t *param) {
    if (pool->state != SOCKET_RESERVE) {
        wtk_debug("expect pool state is %d, actual is %d\r\n", SOCKET_RESERVE, pool->state);
        return -1;
    }
    pool->state = SOCKET_CONNECTING;
    /* notify pool if socket disconnect */
    param->reconnect = QTK_SFRAME_NOT_RECONNECT;
    //param->reconnect = QTK_SFRAME_RECONNECT_LAZY;
    pool->remote_addr = qtk_socket_ntop(&param->saddr);
    /* must set con_param because of used in _new_socket,
       if connect failed, reset con_param to NULL */
    pool->con_param = param;
    pool->req_cache = 4096;
    if (_new_socket(pool)) {
        return 0;
    } else {
        return -1;
    }
}


static int qtk_request_pool_reopen(qtk_request_pool_t *pool) {
    /* resolve domain name */
    if (pool->state != SOCKET_WAIT_REC) {
        wtk_debug("expect pool state is %d, actual is %d\r\n",
                  SOCKET_WAIT_REC, pool->state);
        return -1;
    }
    pool->state = SOCKET_CONNECTING;
    if (pool->remote_addr) {
        wtk_string_delete(pool->remote_addr);
    }
    pool->remote_addr = qtk_socket_ntop(&pool->con_param->saddr);
    if (_new_socket(pool)) {
        return 0;
    } else {
        return -1;
    }
}


static int qtk_request_pool_push_reopen(void *data) {
    qtk_request_pool_t *pool = data;
    if (pool->state == SOCKET_WAIT_REC) {
        qtk_sframe_msg_t *msg = qtk_fwd_cmd_msg(pool->server->entry->frame,
                                                QTK_SFRAME_CMD_REOPEN_POOL,
                                                -pool->id);
        qtk_request_pool_push(pool, msg);
    }
    return 0;
}


static void qtk_request_pool_clear_req(qtk_request_pool_t *pool) {
    qtk_sframe_msg_t * msg;
    qtk_entry_t *entry = pool->server->entry;
    while ((msg = qtk_request_pool_pop(pool))) {
        entry->frame->method.delete(&entry->frame->method, msg);
    }
}


/* not thread-safe, must call in socket-server */
static int qtk_request_pool_close(qtk_request_pool_t *pool) {
    int i;
    wtk_debug("left %d, %d\r\n", pool->cmd_q.length, pool->req_q.length);
    qtk_request_pool_clear_req(pool);
    for (i = pool->nslot-1; i >= 0; i--) {
        if (pool->slots[i].req) {
            qtk_request_delete(pool->slots[i].req);
            pool->slots[i].req = NULL;
        }
        _del_socket(pool, pool->slots+i);
    }
    pool->state = SOCKET_CLOSED;
    return 0;
}


static int qtk_request_pool_request(qtk_request_pool_t *pool, qtk_socket_slot_t *slot) {
    qtk_entry_t *entry = pool->server->entry;
    qtk_request_t *req = slot->req;
    qtk_sframe_msg_t *msg;
    if (NULL == req) {
        msg = qtk_request_pool_pop(pool);
        if (msg) {
            req = msg->info.request;
            msg->id = slot->sock_id;
            req->sock_id = slot->sock_id;
            slot->req = req;
        } else {
            return -1;
        }
    } else {
        msg = qtk_fwd_request_msg(entry->frame, req);
    }
    assert(req->sock_id >= 0);
    msg->signal |= QTK_SFRAME_SIG_DNOT_DEL_REQ;
    qtk_ss_push_msg(pool->server, msg);
    return 0;
}


/*
  qlen is length of req_q
  nfree is number of connected socket with req=NULL
  if min(qlen, nfree) > 0:
      pop min(qlen, nfree) requests to free sockets
  else:
      create new socket if possible (in _get_free_socket)
 */
static int qtk_request_pool_pop_more(qtk_request_pool_t *pool) {
    qtk_socket_slot_t *slot;
    while (pool->req_q.length > 0 && (slot = _get_free_socket(pool))) {
        qtk_request_pool_request(pool, slot);
    }
    return 0;
}


static int req_process(qtk_request_pool_t *pool, qtk_event_t *ev) {
    while (wtk_pipequeue_touch_read(&pool->req_q) == 1);
    qtk_request_pool_pop_more(pool);
    return 0;
}


static int cmd_process(qtk_request_pool_t *pool, qtk_event_t *ev) {
    wtk_queue_node_t *node;
    qtk_fwd_t *frame = pool->server->entry->frame;
    while ((node = wtk_pipequeue_pop(&pool->cmd_q))) {
        qtk_sframe_msg_t *msg = data_offset(node, qtk_sframe_msg_t, q_n);
        switch (msg->signal) {
        case QTK_SFRAME_CMD_OPEN_POOL:
            if (!qtk_request_pool_open(pool, msg->info.con_param)) {
                msg->info.con_param = NULL;
            }
            break;
        case QTK_SFRAME_CMD_REOPEN_POOL:
            qtk_request_pool_reopen(pool);
            break;
        case QTK_SFRAME_CMD_CLOSE:
            qtk_request_pool_close(pool);
            qtk_request_pool_clean(pool);
            break;
        default:
            break;
        }
        frame->method.delete(&frame->method, msg);
    }
    return 0;
}


/* thread-safe */
int qtk_request_pool_init(qtk_request_pool_t *pool, int nslot, qtk_socket_server_t *ss) {
    /* detect con_param is valid */
    pool->slots = wtk_calloc(nslot, sizeof(pool->slots[0]));
    pool->nslot = 0;
    pool->slot_cap = nslot;
    wtk_pipequeue_init(&pool->req_q);
    qtk_event_init(&pool->req_ev, QTK_EVENT_READ,
                   (qtk_event_handler)req_process,
                   NULL, pool);
    wtk_pipequeue_init(&pool->cmd_q);
    qtk_event_init(&pool->cmd_ev, QTK_EVENT_READ,
                   (qtk_event_handler)cmd_process,
                   NULL, pool);
    pool->con_param = NULL;
    pool->state = SOCKET_RESERVE;
    pool->remote_addr = NULL;
    pool->server = ss;
    return qtk_event_add_event(ss->em, pool->req_q.pipe_fd[0], &pool->req_ev)
        || qtk_event_add_event(ss->em, pool->cmd_q.pipe_fd[0], &pool->cmd_ev);
}


int qtk_request_pool_clean(qtk_request_pool_t *pool) {
    if (pool->server) {
        qtk_request_pool_clear_req(pool);
        qtk_event_del_event(pool->server->em, pool->cmd_q.pipe_fd[0]);
        qtk_event_del_event(pool->server->em, pool->req_q.pipe_fd[0]);
        pool->server = NULL;
    }
    wtk_pipequeue_clean(&pool->cmd_q);
    wtk_pipequeue_clean(&pool->req_q);
    if (pool->con_param) {
        wtk_free(pool->con_param);
    }
    if (pool->remote_addr) {
        wtk_string_delete(pool->remote_addr);
        pool->remote_addr = NULL;
    }
    wtk_free(pool->slots);
    pool->state = SOCKET_INVALID;
    return 0;
}


/*
  this function is not thread-safe, but socket raise response in the same
  thread as pool
 */
int qtk_request_pool_response(qtk_request_pool_t *pool, qtk_sframe_msg_t *msg) {
    int sock_id = msg->id;
    qtk_socket_slot_t *slot = _get_socket(pool, sock_id);
    qtk_entry_t *entry = pool->server->entry;
    if (msg->type == QTK_SFRAME_MSG_RESPONSE) {
        if (NULL == slot || NULL == slot->req) {
            wtk_debug("unexpect response, slot=%p\r\n", slot);
        } else {
            msg->id = -pool->id;
            msg->info.response->request = slot->req;
            qtk_entry_raise_msg(entry, msg);
            slot->req = NULL;
            qtk_request_pool_request(pool, slot);
            msg = NULL;
        }
    } else if (msg->type == QTK_SFRAME_MSG_NOTICE) {
        if (QTK_SFRAME_SIGCONNECT == msg->signal) {
            wtk_debug("socket %d connected\r\n", sock_id);
            if (pool->state == SOCKET_CONNECTING) {
                pool->state = SOCKET_CONNECTED;
                qtk_sframe_msg_t *m = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGCONNECT, -pool->id);
                qtk_entry_raise_msg(entry, m);
            }
            if (slot->state == SOCKET_CONNECTING) {
                qtk_request_pool_request(pool, slot);
                slot->state = SOCKET_CONNECTED;
            }
        } else if (QTK_SFRAME_SIGDISCONNECT == msg->signal) {
            qtk_request_t *req = slot->req;
            slot->req = NULL;
            _del_socket(pool, slot);
            if (req) {
                qtk_socket_slot_t *free_slot = _get_free_or_new_socket(pool);
                assert(free_slot);
                req->sock_id = free_slot->sock_id;
                free_slot->req = req;
            }
        } else if (QTK_SFRAME_SIGECONNECT == msg->signal) {
            wtk_debug("socket %d cannot connect to %s\r\n", sock_id,
                      entry->frame->method.get_remote_addr(&entry->frame->method, msg)->data);
            if (slot->req) {
                qtk_sframe_msg_t *m = qtk_fwd_request_msg(entry->frame, slot->req);
                m->id = -pool->id;
                m->signal = msg->signal;
                qtk_entry_raise_msg(entry, m);
                slot->req = NULL;
            }
            _del_socket(pool, slot);
            if (pool->state == SOCKET_WAIT_REC) {
                /* do nothing */
            } else {
                if (pool->state != SOCKET_CONNECTING) {
                    wtk_debug("unexpected state %d\r\n", pool->state);
                }
                pool->state = SOCKET_WAIT_REC;
                qtk_fwd_g_add_timer(1000, qtk_request_pool_push_reopen, pool);
            }
        } else {
            wtk_debug("unexpected msg signal %d\r\n", msg->signal);
        }
    } else if (msg->type == QTK_SFRAME_MSG_REQUEST) {
        /* the request is too large, send back */
        assert(msg->signal & QTK_SFRAME_SIGESENDBUF);
        if (NULL == slot || NULL == slot->req) {
            wtk_debug("unexpect response, slot=%p\r\n", slot);
        } else {
            assert(msg->info.request == slot->req);
            msg->id = -pool->id;
            msg->signal &= ~QTK_SFRAME_SIG_DNOT_DEL_REQ;
            qtk_entry_raise_msg(entry, msg);
            slot->req = NULL;
            qtk_request_pool_request(pool, slot);
            msg = NULL;
        }
    } else {
        wtk_debug("unexpected msg type %d\r\n", msg->type);
    }

    if (msg) {
        entry->frame->method.delete(&entry->frame->method, msg);
    }

    return 0;
}


int qtk_request_pool_push(qtk_request_pool_t *pool, qtk_sframe_msg_t *msg) {
    if (msg->type == QTK_SFRAME_MSG_REQUEST) {
        if (pool->req_q.length < pool->req_cache) {
            return wtk_pipequeue_push(&pool->req_q, &msg->q_n);
        }
    } else if (msg->type == QTK_SFRAME_MSG_CMD) {
        return wtk_pipequeue_push(&pool->cmd_q, &msg->q_n);
    } else {
        assert(1);
    }
    return -1;
}
