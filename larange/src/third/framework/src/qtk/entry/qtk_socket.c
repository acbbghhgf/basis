#include <assert.h>
#include <errno.h>
#include "wtk/os/wtk_socket.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk_socket.h"
#include "qtk/os/qtk_fd.h"
#include "qtk_socket_server.h"
#include "qtk_entry.h"
#include "qtk/frame/qtk_frame.h"
#include "qtk/http/qtk_http_parser.h"


static void qtk_socket_close_parser(qtk_socket_t *s);


static int _socket_disconnected(qtk_socket_t *s) {
    qtk_sframe_msg_t *msg;
    int signal;

    switch (s->state) {
    case SOCKET_CONNECTING:
        signal = QTK_SFRAME_SIGECONNECT;
        break;
    default:
        signal = QTK_SFRAME_SIGDISCONNECT;
        break;
    }
    qtk_socket_close_parser(s);
    msg = qtk_fwd_notice_msg(s->server->entry->frame, signal, s->id);
    qtk_entry_raise_msg(s->server->entry, msg);
    qtk_socket_close(s);
    return 0;
}


static int _socket_read_handle(qtk_socket_t *s, qtk_event_t *ev) {
    if (s->stale ^ ((intptr_t)ev & 1L)) {
        wtk_debug("stale event\r\n");
        return 0;
    }
    ev = (qtk_event_t*)((intptr_t)ev & ~1L);
    while (s->state == SOCKET_CONNECTED) {
        char buf[4096];
        int readed = 0;
        qtk_fd_read(s->fd, buf, sizeof(buf), &readed);
        assert(s->server && s->parser);
        if (readed && s->server && s->parser) {
            //qtk_sframe_method_t *method = &s->server->entry->frame->method;
            s->parser->handler(s->parser, (void*)s, buf, readed);
        }
        if (readed < sizeof(buf)) break;
    }
    if (ev->error || ev->reof || ev->eof) {
        if (s->reconnect > 0 || s->reconnect == QTK_SFRAME_RECONNECT_NOW) {
            if (s->reconnect > 0) s->reconnect--;
            struct sockaddr_in saddr;
            int fd;
            qtk_socket_aton(s->remote_addr->data, &saddr);
            qtk_socket_reset(s);
            fd = qtk_socket_connect(s, (struct sockaddr*)&saddr, sizeof(saddr));
            if (fd == INVALID_FD) {
                _socket_disconnected(s);
            } else {
                s->fd = fd;
                qtk_socket_init_event(s);
            }
        } else if (s->reconnect == QTK_SFRAME_NOT_RECONNECT) {
            _socket_disconnected(s);
        } else if (s->reconnect == QTK_SFRAME_RECONNECT_LAZY) {
            qtk_socket_reset(s);
        } else {
            wtk_debug("unexpected reconnect param : %d\r\n", s->reconnect);
            assert(0);
        }
    }
    return 0;
}


static int _socket_connected(qtk_socket_t *s) {
    qtk_entry_t *entry = s->server->entry;
    qtk_sframe_msg_t *msg = NULL;
    struct sockaddr_in saddr;
    unsigned int len = sizeof(saddr);
    if (getsockname(s->fd, (struct sockaddr*)&saddr, &len)) {
        return -1;
    }
    if (s->local_addr) {
        wtk_string_delete(s->local_addr);
    }
    s->local_addr = qtk_socket_ntop(&saddr);
    s->state = SOCKET_CONNECTED;
    msg = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGCONNECT, s->id);
    qtk_entry_raise_msg(entry, msg);
    return 0;
}


static int _socket_write_handle(qtk_socket_t *s, qtk_event_t *ev) {
    wtk_fd_state_t state;
    if (s->stale ^ ((intptr_t)ev & 1L)) {
        wtk_debug("stale event\r\n");
        return 0;
    }
    if (s->state == SOCKET_CONNECTING) {
        state = WTK_OK;
        _socket_connected(s);
        if (s->send_stk) {
            state = wtk_fd_flush_send_stack(s->fd, s->send_stk);
        }
    } else {
        assert(s->send_stk);
        state = wtk_fd_flush_send_stack(s->fd, s->send_stk);
    }
    if (WTK_AGAIN != state) {
        s->event.want_write = 0;
        s->event.write_handler = NULL;
        qtk_event_mod_event(s->server->em, s->fd, ev);
    }
    return WTK_ERR == state ? -1 : 0;
}


static void _socket_write_continue(qtk_socket_t *s) {
    intptr_t ev = (intptr_t)&s->event;
    s->event.want_write = 1;
    s->event.write_handler = (qtk_event_handler)_socket_write_handle;
    qtk_event_mod_event(s->server->em, s->fd, (qtk_event_t*)(ev | s->stale));
}


/*
  mainly used for reconnect
 */
int qtk_socket_reset(qtk_socket_t *s) {
    if (s->fd != INVALID_FD) {
        if (s->server) {
            qtk_event_del_event(s->server->em, s->fd);
        }
        close(s->fd);
        s->fd = INVALID_FD;
        s->stale = !s->stale;
    }
    if (s->parser) {
        s->parser->reset_handler(s->parser);
    }
    if (s->send_stk) {
        wtk_stack_reset(s->send_stk);
    }
    return 0;
}


static void qtk_socket_close_parser(qtk_socket_t *s) {
    if (s->parser) {
        s->parser->close_handler(s->parser);
        wtk_free(s->parser);
        s->parser = NULL;
    }
}


int qtk_socket_close(qtk_socket_t *s) {
    qtk_socket_close_parser(s);
    if (s->fd != INVALID_FD) {
        if (s->server) {
            qtk_event_del_event(s->server->em, s->fd);
        }
        close(s->fd);
        s->fd = INVALID_FD;
        s->stale = !s->stale;
    }
    if (s->send_stk) {
        wtk_stack_delete(s->send_stk);
        s->send_stk = NULL;
    }
    s->state = SOCKET_CLOSED;
    return 0;
}


int qtk_socket_clean(qtk_socket_t *s) {
    qtk_socket_close(s);
    if (s->remote_addr) {
        wtk_string_delete(s->remote_addr);
        s->remote_addr = NULL;
    }
    if (s->local_addr) {
        wtk_string_delete(s->local_addr);
        s->local_addr = NULL;
    }
    s->state = SOCKET_INVALID;
    s->pid = 0;
    s->server = NULL;
    return 0;
}


void qtk_socket_init_event(qtk_socket_t *s) {
    intptr_t ev = (intptr_t)&s->event;
    if (s->state == SOCKET_CONNECTING) {
        qtk_event_init(&s->event, QTK_EVENT_READ | QTK_EVENT_WRITE,
                       (qtk_event_handler)_socket_read_handle,
                       (qtk_event_handler)_socket_write_handle, s);
    } else if (s->state == SOCKET_CONNECTED || s->state == SOCKET_RESERVE) {
        qtk_event_init(&s->event, QTK_EVENT_READ,
                       (qtk_event_handler)_socket_read_handle,
                       NULL, s);
    } else {
        wtk_debug("bug found. init socket state : %d\r\n", s->state);
    }
    qtk_event_add_event(s->server->em, s->fd, (qtk_event_t*)(ev | s->stale));
}


int qtk_socket_init(qtk_socket_t *s, int fd, qtk_socket_server_t *ss) {
    s->fd = fd;
    s->parser = NULL;
    s->reconnect = QTK_SFRAME_NOT_RECONNECT;
    qtk_ss_add_socket(ss, s);
    qtk_socket_init_event(s);
    return 0;
}


int qtk_socket_listen(qtk_socket_t *s, struct sockaddr *saddr, int sz, int backlog) {
    int fd = INVALID_FD;
    int ret = 0;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_FD) {
        ret = -1;
        goto end;
    }
    wtk_fd_set_tcp_client_opt(fd);
    if ((ret = bind(fd, saddr, sz))) {
        wtk_debug("bind failed : %s\r\n", strerror(errno));
        goto end;
    }
    if ((ret = listen(fd, backlog))) {
        wtk_debug("listen failed : %s\r\n", strerror(errno));
        goto end;
    }
    s->local_addr = qtk_socket_ntop((struct sockaddr_in*)saddr);
    wtk_fd_set_nonblock(fd);
end:
    if (ret && fd != INVALID_FD) {
        close(fd);
        fd = INVALID_FD;
    }
    if (fd != INVALID_FD) {
        s->fd = fd;
        s->server = NULL;
    }
    return ret;
}


int qtk_socket_connect(qtk_socket_t *s, struct sockaddr* saddr, int sz) {
    int fd;

    s->state = SOCKET_CONNECTING;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if (fd == INVALID_FD) goto end;
    wtk_fd_set_nonblock(fd);
    if (connect(fd, saddr, sz) && errno != EINPROGRESS) {
        wtk_debug("connect error : %s\r\n", strerror(errno));
        close(fd);
        fd = INVALID_FD;
        goto end;
    }
    wtk_fd_set_tcp_client_opt(fd);
    if (NULL == s->remote_addr) {
        s->remote_addr = qtk_socket_ntop((struct sockaddr_in*)saddr);
    }
end:
    return fd;
}


static int qtk_socket_reconnect_with_buf(qtk_socket_t *s, const char *buf, int len) {
    struct sockaddr_in saddr;
    qtk_socket_aton(s->remote_addr->data, &saddr);
    s->fd = qtk_socket_connect(s, (struct sockaddr*)&saddr, sizeof(saddr));
    if (s->fd == INVALID_FD) {
        return -1;
    }
    qtk_socket_init_event(s);
    if (NULL == s->send_stk) {
        s->send_stk = wtk_stack_new(4096, 1024 * 1024, 1);
    }
    wtk_stack_push(s->send_stk, buf, len);
    return 0;
}


int qtk_socket_writable_bytes(qtk_socket_t *s) {
    if (s->send_stk) {
        return s->server->cfg->send_buf - s->send_stk->len;
    } else {
        return s->server->cfg->send_buf;
    }
}


int qtk_socket_send(qtk_socket_t *s, const char *buf, int len) {
    int ret = 0;
    int writed = 0;
    int send_buf = s->server->cfg->send_buf;

    if (s->state == SOCKET_CLOSED) return -1;
    if (len > send_buf) {
        wtk_debug("send data too large\r\n");
        return -1;
    }
    if (INVALID_FD == s->fd) {
        if (s->reconnect == QTK_SFRAME_RECONNECT_LAZY) {
            return qtk_socket_reconnect_with_buf(s, buf, len);
        }
        return -1;
    }
    if (s->send_stk && s->send_stk->len) {
        if (len + s->send_stk->len > send_buf) {
            wtk_debug("send buffer is full\r\n");
            return -1;
        } else {
            wtk_stack_push(s->send_stk, buf, len);
            return 0;
        }
    }
    while (len > 0) {
        ret = wtk_fd_send(s->fd, (char*)buf, len, &writed);
        if (WTK_ERR == ret || WTK_EOF == ret) return -1;
        buf += writed;
        len -= writed;
        if (WTK_AGAIN == ret) {
            if (NULL == s->send_stk) {
                s->send_stk = wtk_stack_new(4096, 1024 * 1024, 1);
            }
            wtk_stack_push(s->send_stk, buf, len);
            _socket_write_continue(s);
            return 0;
        }
    }

    return 0;
}


int qtk_socket_send_stack(qtk_socket_t *s, wtk_stack_t *stk) {
    int ret = 0;
    wtk_fd_state_t state;
    int send_buf = s->server->cfg->send_buf;

    if (stk->len > send_buf) {
        wtk_debug("send data too large\r\n");
        return -1;
    }

    if (INVALID_FD == s->fd) {
        if (s->reconnect == QTK_SFRAME_RECONNECT_LAZY) {
            char tmp[128];
            int n = wtk_stack_pop2(stk, tmp, sizeof(tmp));
            /*
              ensure s->send_stk is not empty, and then merge the left data
             */
            if (qtk_socket_reconnect_with_buf(s, tmp, n)) {
                return -1;
            }
        } else {
            return -1;
        }
    }
    if (s->send_stk && s->send_stk->len) {
        if (stk->len + s->send_stk->len > send_buf) {
            wtk_debug("send buffer is full\r\n");
            ret = -1;
        } else {
            wtk_stack_add(s->send_stk, stk);
        }
        goto end;
    }
    state = wtk_fd_flush_send_stack(s->fd, stk);
    if (WTK_AGAIN == state) {
        if (NULL == s->send_stk) {
            s->send_stk = wtk_stack_new(4096, 1024 * 1024, 1);
        }
        wtk_stack_add(s->send_stk, stk);
        _socket_write_continue(s);
    }
end:
    return ret;
}


wtk_string_t *qtk_socket_ntop(struct sockaddr_in* addr) {
    char tmp[128];
    int n;
    wtk_string_t *addr_s;
    const char *p = inet_ntop(AF_INET, &addr->sin_addr, tmp, sizeof(tmp));
    if (NULL == p) {
        wtk_debug("inet_ntop error : %s\r\n", strerror(errno));
        strcpy(tmp, "0.0.0.0:");
        p = tmp;
    }
    n = strlen(p);
    tmp[n++] = ':';
    n += sprintf(tmp + n, "%d", ntohs(addr->sin_port));
    addr_s = wtk_string_dup_data_pad0(tmp, n);
    addr_s->len--;
    return addr_s;
}


int qtk_socket_aton(const char *addr, struct sockaddr_in *saddr) {
    const char* delim = strrchr(addr, ':');
    if (delim) {
        char ip[32];
        int len = delim - addr;
        int port = atoi(delim+1);
        if (port < 0 && port > 65535) {
            wtk_debug("invalid port range\r\n");
            return -1;
        }
        if (len >= sizeof(ip)) {
            wtk_debug("ip addr is too long\r\n");
            return -1;
        }
        memset(ip, 0, sizeof(ip));
        memcpy(ip, addr, len);
        saddr->sin_family = AF_INET;
        if (!inet_aton(ip, &saddr->sin_addr)) {
            return -1;
        }
        saddr->sin_port = htons(port);
    } else {
        return -1;
    }
    return 0;
}
