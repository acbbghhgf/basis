#include <assert.h>
#include <netdb.h>
#include <errno.h>
#include <sys/un.h>
#include "qtk_zeus_socket.h"
#include "qtk_zeus.h"
#include "qtk_zsocket_server.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_socket.h"
#include "qtk/core/qtk_deque.h"
#include "qtk/core/qtk_atomic.h"
#include "qtk_zeus_mq.h"
#include "qtk_zeus_server.h"


#define DBG_PRINT_TIME() wtk_debug("%lld\r\n", (long long)time_get_ms())


typedef struct qtk_zsocket qtk_zsocket_t;
typedef struct qtk_zsocket_cmd qtk_zsocket_cmd_t;


enum {
    ZSOCKET_INVALID = 0,
    ZSOCKET_RESERVE,
    ZSOCKET_CONNECTING,
    ZSOCKET_CONNECTED,
    ZSOCKET_LISTEN,
};

struct qtk_zsocket {
    int fd;
    int id;
    int state;
    uint32_t uplayer;
    wtk_stack_t *send_stk;
    qtk_event_t event;
    char label[64];
    qtk_zsocket_server_t *server;
};


struct qtk_zsocket_cmd {
    wtk_queue_node_t node;
    int type;
    uint32_t src;
    int id;
    int ext_sz;
};


static qtk_zsocket_t *qtk_ss_get_socket(qtk_zsocket_server_t *ss, int id);


static qtk_zsocket_t* _ss_get_socket(qtk_zsocket_server_t *ss, int id) {
    return &ss->slot[id % ss->max_fd];
}


static int _ss_alloc_id(qtk_zsocket_server_t *ss) {
    int i;
    int id;
    for (i = 0; i < ss->max_fd; ++i) {
        id = QTK_ATOM_INC(&ss->alloc_id);
        if (id < 0) {
            id &= 0x7fffffff;
        }
        qtk_zsocket_t *s = _ss_get_socket(ss, id);
        if (ZSOCKET_INVALID == s->state) {
            if (QTK_ATOM_CAS(&s->state, ZSOCKET_INVALID, ZSOCKET_RESERVE)) {
                s->id = id;
                s->fd = -1;
                s->server = ss;
                return id;
            }
        }
    }
    return -1;
}


static void _forward_message(int type, int id, uint32_t dst, const char *buf, size_t sz) {
    qtk_zmesg_t zmsg;
    qtk_zsocket_msg_t* msg = wtk_malloc(sizeof(*msg) + sz);
    msg->type = type;
    msg->id = id;
    memcpy(msg+1, buf, sz);
    sz += sizeof(*msg);
    zmsg.source = 0;
    zmsg.session = 0;
    zmsg.data = msg;
    zmsg.sz = sz | ((size_t)ZEUS_PTYPE_SOCKET << ZEUS_TYPE_SHIFT);
    if (qtk_zcontext_push(dst, &zmsg)) {
        wtk_free(msg);
    }
}


static int qtk_socket_init(qtk_zsocket_t *s, uint32_t uplayer) {
    /* s->send_stk = wtk_stack_new(1024, 1024*1024, 1); */
    s->state = ZSOCKET_RESERVE;
    s->uplayer = uplayer;
    return 0;
}


static void qtk_socket_clean(qtk_zsocket_t *s) {
    if (s->send_stk) {
        wtk_stack_delete(s->send_stk);
        s->send_stk = NULL;
    }
    if (s->fd != INVALID_FD) {
        close(s->fd);
        s->fd = INVALID_FD;
    }
    s->state = ZSOCKET_INVALID;
}


static void qtk_socket_set_label(qtk_zsocket_t *s, const char *addr, size_t sz) {
    memset(s->label, 0, sizeof(s->label));
    if (sz >= sizeof(s->label)) {
        addr = addr + sz - sizeof(s->label) + 1;
        memcpy(s->label, addr, sizeof(s->label)-1);
    } else {
        memcpy(s->label, addr, sz);
    }
}


static int _socket_read_handle(qtk_zsocket_t *s, qtk_event_t *ev) {
    while (1) {
        char buf[4096];
        int readed = 0;
        fd_read(s->fd, buf, sizeof(buf), &readed);
        if (readed) {
            _forward_message(ZSOCKET_MSG_DATA, s->id, s->uplayer, buf, readed);
        }
        if (readed < sizeof(buf)) break;
    }
    if (ev->error) {
        wtk_debug("error\r\n");
        _forward_message(ZSOCKET_MSG_ERR, s->id, s->uplayer, NULL, 0);
        qtk_socket_clean(s);
    } else if (ev->eof || ev->reof) {
        wtk_debug("eof\r\n");
        _forward_message(ZSOCKET_MSG_CLOSE, s->id, s->uplayer, NULL, 0);
        qtk_socket_clean(s);
    }
    return 0;
}


static int _socket_write_handle(qtk_zsocket_t *s, qtk_event_t *ev) {
    wtk_fd_state_t state;
    assert(s->send_stk);
    state = wtk_fd_flush_send_stack(s->fd, s->send_stk);
    if (WTK_AGAIN != state) {
        s->event.want_write = 0;
        s->event.write_handler = NULL;
        qtk_event_mod_event(s->server->em, s->fd, &s->event);
    }
    return WTK_ERR == state ? -1 : 0;
}


static int _socket_accept_handle(qtk_zsocket_t *s, qtk_event_t *ev) {
    int fd;
    struct sockaddr_in caddr;
    unsigned int len;
    while (1) {
        fd = accept(s->fd, (struct sockaddr*)&caddr, &len);
        if (fd == -1 && errno == EAGAIN) break;
        if (fd != INVALID_FD) {
            int id = _ss_alloc_id(s->server);
            qtk_zsocket_t *cli = qtk_ss_get_socket(s->server, id);
            const char *addr = inet_ntoa(caddr.sin_addr);
            qtk_socket_set_label(cli, addr, strlen(addr));
            wtk_fd_set_nonblock(fd);
            wtk_fd_set_tcp_client_opt(fd);
            qtk_socket_init(cli, s->uplayer);
            cli->fd = fd;
            qtk_event_init(&cli->event, QTK_EVENT_READ,
                           (qtk_event_handler)_socket_read_handle,
                           NULL, cli);
            qtk_event_add_event(cli->server->em, fd, &cli->event);
            id = htonl(id);
            _forward_message(ZSOCKET_MSG_ACCEPT, s->id, s->uplayer, (char*)&id, sizeof(id));
        }
    }
    return 0;
}


static void _socket_touch_write(qtk_zsocket_t *s) {
    s->event.want_write = 1;
    s->event.write_handler = (qtk_event_handler)_socket_write_handle;
    qtk_event_mod_event(s->server->em, s->fd, &s->event);
}


static int _get_open_mode(int mode) {
    switch (mode) {
    case ZSOCKET_MODE_WRONLY:
        return O_WRONLY;
    case ZSOCKET_MODE_RDONLY:
        return O_RDONLY;
    case ZSOCKET_MODE_RDWR:
        return O_RDWR;
    default:
        assert(0);
    }
}


static int _get_socket_addr(const char* addr, size_t sz, struct sockaddr_in *saddr) {
    int ret;
    int port;
    char tmp[sz+1];
    strncpy(tmp, addr, sz);
    tmp[sz] = '\0';
    char *p = strrchr(tmp, ':');
    if (p) {
        port = atoi(p+1);
        *p = '\0';
    } else {
        port = 80;
    }
    saddr->sin_family = AF_INET;
    saddr->sin_port = htons(port);
    ret = inet_aton(tmp, &saddr->sin_addr) ? 0 : -1;
    return ret;
}


static int qtk_socket_listen(qtk_zsocket_t *s, const char *addr, size_t sz, int backlog) {
    struct sockaddr_in saddr;
    int fd = INVALID_FD;
    int ret = _get_socket_addr(addr, sz, &saddr);
    if (ret) goto end;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_FD) {
        ret = -1;
        const char *es = strerror(errno);
        wtk_debug("socket failed : %s\r\n", es);
        qtk_zerror(NULL, "socket failed : %s", es);
        goto end;
    }
    wtk_fd_set_tcp_client_opt(fd);
    if ((ret = bind(fd, (struct sockaddr*)&saddr, sizeof(saddr)))) {
        const char *es = strerror(errno);
        wtk_debug("bind to \"%.*s\" failed : %s\r\n", (int)sz, addr, es);
        qtk_zerror(NULL, "socket bind failed : %s", es);
        goto end;
    }
    if ((ret = listen(fd, backlog))) {
        const char *es = strerror(errno);
        wtk_debug("listen failed : %s\r\n", es);
        qtk_zerror(NULL, "listen failed : %s", es);
        goto end;
    }
    wtk_fd_set_nonblock(fd);
end:
    if (ret && fd != INVALID_FD) {
        close(fd);
        fd = INVALID_FD;
    }
    if (fd != INVALID_FD) {
        s->fd = fd;
        qtk_event_init(&s->event, QTK_EVENT_READ,
                       (qtk_event_handler)_socket_accept_handle,
                       NULL, s);
        qtk_event_add_event(s->server->em, fd, &s->event);
    }
    return ret;
}


static int qtk_socket_open(qtk_zsocket_t *s, const char *addr, size_t sz, int mode) {
    int fd;
    if (':' == addr[0]) {
        fd = strtol(addr+1, NULL, 10);
    } else if ('.' == addr[0] || '/' == addr[0]) {
        char tmp[sz+1];
        memcpy(tmp, addr, sz);
        tmp[sz] = '\0';
        fd = open(tmp, _get_open_mode(mode) | O_NONBLOCK);
        qtk_socket_set_label(s, addr, sz);
    } else if (sz > 5 && 0 == memcmp(addr, "unix:", 5)) {
        struct sockaddr_un un;
        int addr_len;
        fd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd == INVALID_FD) {
            const char *es = strerror(errno);
            wtk_debug("socket failed : %s\r\n", es);
            qtk_zerror(NULL, "socket failed : %s", es);
            return -1;
        }
        addr += 5;
        sz -= 5;
        un.sun_family = AF_UNIX;
        memcpy(un.sun_path, addr, sz);
        un.sun_path[sz] = '\0';
        addr_len = offsetof(struct sockaddr_un, sun_path) + sz;
        errno = 0;
        if (connect(fd, (struct sockaddr*)&un, addr_len)) {
            const char *es = strerror(errno);
            wtk_debug("connect to %.*s failed : %s\r\n", (int)sz, addr, es);
            wtk_debug("connect to \"%.*s\" failed : %s", (int)sz, addr, es);
            close(fd);
            return -1;
        }
        qtk_socket_set_label(s, addr, sz);
        wtk_fd_set_tcp_client_opt(fd);
        wtk_fd_set_nonblock(fd);
        mode = ZSOCKET_MODE_RDWR;
    } else {
        struct sockaddr_in saddr;
        int ret = _get_socket_addr(addr, sz, &saddr);
        if (ret || wtk_socket_connect((struct sockaddr*)&saddr,
                                      sizeof(saddr), &fd)) {
            return -1;
        }
        qtk_socket_set_label(s, addr, sz);
        wtk_fd_set_nonblock(fd);
        wtk_fd_set_tcp_client_opt(fd);
        mode = ZSOCKET_MODE_RDWR;
    }
    if (fd >= 0) {
        s->fd = fd;
        if (mode != ZSOCKET_MODE_WRONLY) {
            qtk_event_init(&s->event, QTK_EVENT_READ,
                           (qtk_event_handler)_socket_read_handle,
                           NULL, s);
            qtk_event_add_event(s->server->em, fd, &s->event);
        } else {
            qtk_event_init(&s->event, 0, NULL, NULL, s);
        }
        return 0;
    } else {
        return -1;
    }
}


static int qtk_socket_send(qtk_zsocket_t *s, const char *msg, size_t sz) {
    if (s->send_stk && s->send_stk->len > 0) {
        wtk_stack_push(s->send_stk, msg, sz);
        return 0;
    }
    int writed = 0;
    int ret = 0;
    while (sz > 0) {
        ret = fd_write(s->fd, (char*)msg, sz, &writed);
        msg += writed;
        sz -= writed;
        if (ret < 0) {
            if (errno == EAGAIN) {
                if (NULL == s->send_stk) {
                    s->send_stk = wtk_stack_new(1024, 1024*1024, 1);
                }
                wtk_stack_push(s->send_stk, msg, sz);
                _socket_touch_write(s);
                ret = 0;
            } else {
                const char *es = strerror(errno);
                wtk_debug("socket %s\r\n", es);
                qtk_zerror(NULL, "socket[%s] send failed: %s", s->label, es);
            }
            break;
        }
    }
    if (ret > 0) ret = 0;
    return ret;
}


static void qtk_socket_close(qtk_zsocket_t *s) {
    if (s->fd != INVALID_FD) {
        close(s->fd);
        s->fd = INVALID_FD;
    }
    if (s->send_stk) {
        wtk_stack_reset(s->send_stk);
    }
    s->id = -1;
    s->state = ZSOCKET_INVALID;
}


static qtk_zsocket_t *qtk_ss_get_socket(qtk_zsocket_server_t *ss, int id) {
    qtk_zsocket_t *s = _ss_get_socket(ss, id);
    if (s->id == id && s->state != ZSOCKET_INVALID) {
        return s;
    } else {
        return NULL;
    }
}


static int qtk_ss_cmd_process(qtk_zsocket_server_t *ss, qtk_event_t *ev) {
    wtk_queue_node_t *n;
    qtk_zsocket_cmd_t *cmd;
    qtk_zsocket_t *socket;
    int id;
    while ((n = wtk_pipequeue_pop(&ss->cmd_q))) {
        cmd = data_offset(n, qtk_zsocket_cmd_t, node);
        id = cmd->id;
        int type = cmd->type & 0xffff;
        socket = qtk_ss_get_socket(ss, id);
        if (NULL == socket) {
            /* overdue cmd, the socket is already closed */
            if (type != ZSOCKET_CMD_CLOSE) {
                wtk_debug("invalid socket, opcode: %d\r\n", type);
                qtk_zerror(NULL, "operate on invalid socket, code: %d", type);
                _forward_message(ZSOCKET_MSG_ERR, id, cmd->src, NULL, 0);
            }
            wtk_free(cmd);
            continue;
        }
        switch (type) {
        case ZSOCKET_CMD_OPEN:
            assert(cmd->ext_sz);
            qtk_socket_init(socket, cmd->src);
            /* cmd->id multiplex for open mode */
            if (qtk_socket_open(socket, (const char*)(cmd+1), cmd->ext_sz,
                                (cmd->type>>ZSOCKET_MODE_SHIFT)&((1<<ZSOCKET_MODE_BITS)-1))) {
                wtk_debug("open %.*s error\r\n", cmd->ext_sz, (const char*)(cmd+1));
                qtk_zerror(NULL, "open socket %.*s error", cmd->ext_sz, (const char*)(cmd+1));
                _forward_message(ZSOCKET_MSG_ERR, id, cmd->src, NULL, 0);
            } else {
                _forward_message(ZSOCKET_MSG_OPEN, id, cmd->src, NULL, 0);
            }
            break;
        case ZSOCKET_CMD_SEND:
            assert(cmd->ext_sz);
            if (qtk_socket_send(socket, (const char*)(cmd+1), cmd->ext_sz)) {
                wtk_debug("send error\r\n");
                _forward_message(ZSOCKET_MSG_ERR, id, cmd->src, NULL, 0);
            }
            break;
        case ZSOCKET_CMD_CLOSE:
            qtk_socket_close(socket);
            break;
        case ZSOCKET_CMD_LISTEN:
            assert(cmd->ext_sz);
            qtk_socket_init(socket, cmd->src);
            if (qtk_socket_listen(socket, (const char*)(cmd+1), cmd->ext_sz, 32)) {
                wtk_debug("listen error\r\n");
                _forward_message(ZSOCKET_MSG_ERR, id, cmd->src, NULL, 0);
            } else {
                _forward_message(ZSOCKET_MSG_OPEN, id, cmd->src, NULL, 0);
            }
            break;
        }
        wtk_free(cmd);
    }
    return 0;
}


qtk_zsocket_server_t* qtk_zsocket_server_new(int max_fd) {
    qtk_zsocket_server_t *ss = wtk_malloc(sizeof(*ss));
    qtk_event_cfg_t *cfg = wtk_malloc(sizeof(*cfg));
    qtk_event_cfg_init(cfg);
    cfg->use_timer = 0;
    cfg->use_epoll = 1;
    cfg->poll.epoll.event_num = max_fd;
    ss->em = qtk_event_module_new(cfg);
    wtk_pipequeue_init(&ss->cmd_q);
    ss->max_fd = max_fd;
    ss->slot = wtk_calloc(max_fd, sizeof(ss->slot[0]));
    ss->alloc_id = 0;
    qtk_zsocket_server_init(ss);

    return ss;
}


int qtk_zsocket_server_init(qtk_zsocket_server_t* ss) {
    qtk_event_init(&ss->cmd_ev, QTK_EVENT_READ,
                   (qtk_event_handler)qtk_ss_cmd_process,
                   NULL, ss);
    qtk_event_add_event(ss->em, ss->cmd_q.pipe_fd[0], &ss->cmd_ev);
    return 0;
}


int qtk_zsocket_server_delete(qtk_zsocket_server_t* ss) {
    int i;
    if (ss->em) {
        if (ss->em->cfg) {
            wtk_free(ss->em->cfg);
        }
        qtk_event_module_delete(ss->em);
    }
    for (i = 0; i < ss->max_fd; ++i) {
        qtk_socket_clean(&ss->slot[i]);
    }
    if (ss->slot) {
        wtk_free(ss->slot);
    }
    wtk_pipequeue_clean(&ss->cmd_q);
    wtk_free(ss);
    return 0;
}


int qtk_zsocket_server_process(qtk_zsocket_server_t* ss) {
    return qtk_event_process(ss->em, 100, 0);
}


int qtk_ss_cmd_open(qtk_zsocket_server_t *ss, uint32_t src, const char *addr, size_t sz, int mode) {
    int id = _ss_alloc_id(ss);
    qtk_zsocket_cmd_t *cmd = wtk_malloc(sizeof(*cmd)+sz);
    wtk_queue_node_init(&cmd->node);
    cmd->type = ZSOCKET_CMD_OPEN;
    cmd->type |= mode << ZSOCKET_MODE_SHIFT;
    cmd->id = id;
    cmd->src = src;
    cmd->ext_sz = sz;
    memcpy(cmd+1, addr, sz);
    wtk_pipequeue_push(&ss->cmd_q, &cmd->node);
    return id;
}


void qtk_ss_cmd_close(qtk_zsocket_server_t *ss, uint32_t src, int id) {
    qtk_zsocket_cmd_t *cmd = wtk_malloc(sizeof(*cmd));
    wtk_queue_node_init(&cmd->node);
    cmd->type = ZSOCKET_CMD_CLOSE;
    cmd->src = src;
    cmd->id = id;
    wtk_pipequeue_push(&ss->cmd_q, &cmd->node);
}


void qtk_ss_cmd_send(qtk_zsocket_server_t *ss, uint32_t src, int id, const char *msg, size_t sz) {
    qtk_zsocket_cmd_t *cmd = wtk_malloc(sizeof(*cmd)+sz);
    wtk_queue_node_init(&cmd->node);
    cmd->type = ZSOCKET_CMD_SEND;
    cmd->src = src;
    cmd->ext_sz = sz;
    cmd->id = id;
    memcpy(cmd+1, msg, sz);
    wtk_pipequeue_push(&ss->cmd_q, &cmd->node);
}


void qtk_ss_cmd_send_buffer(qtk_zsocket_server_t *ss, uint32_t src, int id, qtk_deque_t *dq) {
    qtk_zsocket_cmd_t *cmd = wtk_malloc(sizeof(*cmd)+dq->len);
    wtk_queue_node_init(&cmd->node);
    cmd->type = ZSOCKET_CMD_SEND;
    cmd->src = src;
    cmd->ext_sz = dq->len;
    cmd->id = id;
    qtk_deque_pop(dq, (char*)(cmd+1), dq->len);
    wtk_pipequeue_push(&ss->cmd_q, &cmd->node);
}


int qtk_ss_cmd_listen(qtk_zsocket_server_t *ss, uint32_t src,
                       const char *addr, size_t sz, int backlog) {
    int id = _ss_alloc_id(ss);
    qtk_zsocket_cmd_t *cmd = wtk_malloc(sizeof(*cmd) + sz);
    wtk_queue_node_init(&cmd->node);
    cmd->type = ZSOCKET_CMD_LISTEN;
    cmd->src = src;
    cmd->id = id;
    cmd->ext_sz = sz;
    memcpy(cmd+1, addr, sz);
    wtk_pipequeue_push(&ss->cmd_q, &cmd->node);
    return id;
}
