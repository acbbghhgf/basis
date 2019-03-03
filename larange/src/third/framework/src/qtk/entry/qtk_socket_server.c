#include <errno.h>
#include <assert.h>
#include "wtk/os/wtk_pipequeue.h"
#include "qtk_socket_server.h"
#include "qtk_socket.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/frame/qtk_frame.h"
#include "qtk/event/qtk_epoll.h"
#include "qtk/http/qtk_http_parser.h"
#include "qtk/entry/qtk_entry.h"


static int _ss_claim_request(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    qtk_request_t *req = qtk_request_new();
    if (req) {
        p->pack = req;
        return 0;
    } else {
        return -1;
    }
}


static int _ss_request_ready(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    qtk_fwd_t *frame = ss->entry->frame;
    qtk_request_t * req = p->pack;
    assert(req->s);
    req->content_length = req->body ? req->body->pos : 0;
    qtk_sframe_msg_t *msg = qtk_fwd_request_msg(frame, req);
    qtk_entry_raise_msg(ss->entry, msg);
    p->pack = NULL;
    return 0;
}


static int _ss_request_unlink(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    if (p->pack) {
        qtk_request_delete(p->pack);
        p->pack = NULL;
    }
    return 0;
}


static int _ss_claim_response(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    qtk_response_t *rep = qtk_response_new();
    if (rep) {
        p->pack = rep;
        return 0;
    } else {
        return -1;
    }
}


static int _ss_response_ready(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    qtk_fwd_t *frame = ss->entry->frame;
    qtk_response_t *rep = p->pack;
    assert(rep->s);
    qtk_sframe_msg_t *msg = qtk_fwd_response_msg(frame, rep);
    qtk_entry_raise_msg(ss->entry, msg);
    p->pack = NULL;
    return 0;
}


static int _ss_response_unlink(qtk_socket_server_t *ss, qtk_http_parser_t *p) {
    if (p->pack) {
        qtk_response_delete(p->pack);
        p->pack = NULL;
    }
    return 0;
}


static int _socket_accept_handle(qtk_socket_listen_t *lis, qtk_event_t *ev) {
    int ret = 0;
    int fd;
    struct sockaddr_in caddr;
    unsigned int len = sizeof(caddr);
    qtk_entry_t *entry = lis->server->entry;
    qtk_socket_t *socket = qtk_entry_get_socket(entry, lis->id);
    if (socket->stale ^ ((intptr_t)ev & 1L)) {
        wtk_debug("stale event\r\n");
        return 0;
    }
    while (1) {
        fd = accept(socket->fd, (struct sockaddr*)&caddr, &len);
        if (fd == INVALID_FD) {
            if (errno != EAGAIN) {
                wtk_debug("accept failed: %s\r\n", strerror(errno));
                ret = -1;
            }
            break;
        } else {
            int id = qtk_entry_alloc_id(entry, 0);
            qtk_socket_t *cli = qtk_entry_get_socket(entry, id);
            qtk_sframe_msg_t *msg = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGCONNECT, id);
            wtk_fd_set_nonblock(fd);
            wtk_fd_set_tcp_client_opt(fd);
            qtk_socket_init(cli, fd, lis->server);
            cli->remote_addr = qtk_socket_ntop(&caddr);
            cli->local_addr = wtk_string_dup_data(socket->local_addr->data,
                                                  socket->local_addr->len);
            cli->state = SOCKET_CONNECTED;
            qtk_entry_raise_msg(entry, msg);
        }
    }
    return ret;
}


static int process_cmd(qtk_socket_server_t *ss, qtk_socket_t *socket, qtk_sframe_msg_t *msg) {
    switch (msg->signal) {
    case QTK_SFRAME_CMD_CLOSE:
        qtk_socket_clean(socket);
        break;
    default:
        break;
    }
    return 0;
}


static int msg_process(qtk_socket_server_t *ss, qtk_event_t *ev) {
    wtk_queue_node_t *n;
    qtk_sframe_msg_t *msg;
    qtk_socket_t *socket;
    int id;
    int type;
    qtk_fwd_t *frame = ss->entry->frame;

    while ((n = wtk_pipequeue_pop(&ss->msg_q))) {
        msg = data_offset(n, qtk_sframe_msg_t, q_n);
        id = msg->id;
        type = msg->type;
        socket = qtk_entry_get_socket(ss->entry, id);
        if (NULL == socket) {
            /* overdue cmd, the socket is already closed */
            wtk_debug("invalid socket %d\r\n", id);
        } else {
            if (QTK_SFRAME_MSG_CMD == type) {
                process_cmd(ss, socket, msg);
            } else if (QTK_SFRAME_MSG_REQUEST == type) {
                if (socket->state != SOCKET_CLOSED) {
                    if (qtk_request_send(msg->info.request, socket)) {
                        assert((msg->signal & QTK_SFRAME_SIG_MASK) == 0);
                        msg->signal |= QTK_SFRAME_SIGESENDBUF;
                    }
                }
            } else if (QTK_SFRAME_MSG_RESPONSE == type) {
                if (socket->state != SOCKET_CLOSED) {
                    if (qtk_response_send(msg->info.response, socket)) {
                        assert((msg->signal & QTK_SFRAME_SIG_MASK) == 0);
                        msg->signal |= QTK_SFRAME_SIGESENDBUF;
                    }
                }
            }
        }
        if (msg->signal & QTK_SFRAME_SIG_MASK) {
            qtk_entry_raise_msg(ss->entry, msg);
        } else {
            frame->method.delete((qtk_sframe_method_t *)frame, msg);
        }
    }

    return 0;
}


static int qtk_socket_server_process(void *data, wtk_thread_t *t) {
    qtk_socket_server_t *ss = data;
    while (ss->run) {
        qtk_event_module_process(ss->em, 100, 0);
    }
    return 0;
}


qtk_socket_server_t *qtk_socket_server_new(qtk_socket_server_cfg_t *cfg, qtk_entry_t *entry) {
    qtk_socket_server_t *ss = wtk_malloc(sizeof(*ss));
    ss->cfg = cfg;
    ss->em = qtk_event_module_new(&cfg->event);
    wtk_pipequeue_init(&ss->msg_q);
    ss->slot_cap = cfg->slot_size;
    ss->slot = wtk_calloc(ss->slot_cap, sizeof(ss->slot[0]));
    ss->alloc_id = 0;
    ss->lis_cap = 100;
    ss->listens = wtk_calloc(ss->lis_cap, sizeof(ss->listens[0]));
    ss->nlis = 0;
    ss->entry = entry;
    qtk_socket_server_init(ss);

    return ss;
}


int qtk_socket_server_init(qtk_socket_server_t *ss) {
    ss->run = 0;
    wtk_thread_init(&ss->thread, qtk_socket_server_process, ss);
    qtk_event_init(&ss->msg_ev, QTK_EVENT_READ,
                   (qtk_event_handler)msg_process,
                   NULL, ss);
    qtk_event_add_event(ss->em, ss->msg_q.pipe_fd[0], &ss->msg_ev);

    return 0;
}


int qtk_socket_server_delete(qtk_socket_server_t *ss) {
    int i;

    wtk_thread_clean(&ss->thread);
    for (i = 0; i < ss->slot_cap; ++i) {
        if (ss->slot[i].state != SOCKET_INVALID) {
            qtk_socket_clean(&ss->slot[i]);
        }
    }
    if (ss->slot) {
        wtk_free(ss->slot);
    }
    if (ss->listens) {
        int i;
        for (i = 0; i < ss->nlis; ++i) {
            wtk_free(ss->listens[i]);
        }
        wtk_free(ss->listens);
    }
    if (ss->em) {
        qtk_event_module_delete(ss->em);
    }
    wtk_pipequeue_clean(&ss->msg_q);
    wtk_free(ss);

    return 0;
}


int qtk_socket_server_start(qtk_socket_server_t *ss) {
    ss->run = 1;
    wtk_thread_start(&ss->thread);
    return 0;
}


int qtk_socket_server_stop(qtk_socket_server_t *ss) {
    ss->run = 0;
    wtk_pipequeue_touch_write(&ss->msg_q);
    wtk_thread_join(&ss->thread);
    return 0;
}


qtk_socket_listen_t* qtk_ss_get_listen(qtk_socket_server_t *ss, int id) {
    int i;
    for (i = 0; i < ss->nlis; ++i) {
        if (ss->listens[i]->id == id) {
            return ss->listens[i];
        }
    }
    return NULL;
}


int qtk_ss_add_listen(qtk_socket_server_t *ss, int id) {
    qtk_socket_listen_t *lis;
    qtk_socket_t *s;
    if (ss->nlis >= ss->lis_cap) {
        wtk_debug("too many listen, %d/%d\r\n", ss->nlis, ss->lis_cap);
        return -1;
    }
    s = qtk_entry_get_socket(ss->entry, id);
    if (NULL == s) return -1;
    lis = wtk_malloc(sizeof(*lis));
    intptr_t ev = (intptr_t)&lis->event;
    lis->id = id;
    lis->server = ss;
    qtk_event_init(&lis->event, QTK_EVENT_READ,
                   (qtk_event_handler)_socket_accept_handle,
                   NULL, lis);
    if (qtk_event_add_event(ss->em, s->fd, (qtk_event_t*)(ev | s->stale))) {
        wtk_free(lis);
        return -1;
    }
    ss->listens[ss->nlis++] = lis;
    return 0;
}


int qtk_ss_remove_listen(qtk_socket_server_t *ss, int id) {
    int i;
    for (i = 0; i < ss->nlis; ++i) {
        if (ss->listens[i]->id == id) {
            qtk_socket_listen_t *lis = ss->listens[i];
            qtk_socket_t *s = qtk_entry_get_socket(ss->entry, id);
            if (s) {
                qtk_event_del_event(ss->em, s->fd);
            }
            wtk_free(lis);
            --ss->nlis;
            if (i < ss->nlis) {
                ss->listens[i] = ss->listens[--ss->nlis];
            }
            return 0;
        }
    }
    return 0;
}


void qtk_ss_add_socket(qtk_socket_server_t *ss, qtk_socket_t *socket) {
    qtk_http_parser_t *parser = qtk_http_parser_new();
    qtk_http_parser_set_handlers(parser, HTTP_REQUEST_PARSER,
                                 (qtk_http_parser_pack_f)_ss_claim_request,
                                 (qtk_http_parser_pack_f)_ss_request_ready,
                                 (qtk_http_parser_pack_f)_ss_request_unlink,
                                 ss);
    qtk_http_parser_set_handlers(parser, HTTP_RESPONSE_PARSER,
                                 (qtk_http_parser_pack_f)_ss_claim_response,
                                 (qtk_http_parser_pack_f)_ss_response_ready,
                                 (qtk_http_parser_pack_f)_ss_response_unlink,
                                 ss);
    socket->parser = (qtk_parser_t*)parser;
    socket->server = ss;
}


int qtk_ss_push_msg(qtk_socket_server_t *ss, qtk_sframe_msg_t *msg) {
    return wtk_pipequeue_push(&ss->msg_q, &msg->q_n);
}
