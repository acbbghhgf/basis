#include "qtk_pipe.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"
#include "wtk/os/wtk_fd.h"
#include "pack/qtk_parser.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

static int _read_process(qtk_pipe_t *p, qtk_event_t *e)
{
    char buf[1024];
    int ret = recv(p->s.fd, buf, sizeof(buf), 0);
    if (ret == 0) {
        if (p->close_notifier) p->close_notifier(p->ud, p);
        goto end;
    } else if (ret < 0) {
        if (errno != EINTR && errno != EAGAIN)
            qtk_debug("recv error:%s\n", strerror(errno));
    } else {
        p->ps.f(&p->ps, p, buf, ret);
    }

    if (e->eof || e->reof) {
        if (p->close_notifier) p->close_notifier(p->ud, p);
    }
end:
    return 0;
}

static int _touch_write(qtk_pipe_t *p, qtk_event_handler wh)
{
    p->ev.want_write = NULL == wh ? 0 : 1;
    p->ev.write_handler = wh;
    return qtk_event_mod_event(p->em, p->s.fd, &p->ev);
}

static int _write_process(qtk_pipe_t *p, qtk_event_t *e)
{
    unuse(e);
    int ret = WTK_OK;
    if (p->ws) ret = wtk_fd_flush_send_stack(p->s.fd, p->ws);
    if (WTK_AGAIN != ret) _touch_write(p, NULL);
    return ret == WTK_ERR ? -1 : 0;
}

qtk_pipe_t *qtk_pipe_new()
{
    qtk_pipe_t *p = qtk_calloc(1, sizeof(*p));
    return p;
}

void qtk_pipe_init(qtk_pipe_t *p)
{
    memset(p, 0, sizeof(*p));
    qtk_unsocket_init(&p->s);
    qtk_event_init(&p->ev, QTK_EVENT_READ, (qtk_event_handler)_read_process, NULL, p);
}

void qtk_pipe_clean(qtk_pipe_t *p)
{
    qtk_unsocket_clean(&p->s);
    if (p->ws) wtk_stack_delete(p->ws);
    if (p->pid > 0) kill(p->pid, SIGKILL);
}

void qtk_pipe_delete(qtk_pipe_t *p)
{
    qtk_parser_clean(&p->ps);
    qtk_pipe_clean(p);
    qtk_free(p);
}

void qtk_pipe_set_close_notifier(qtk_pipe_t *p, qtk_pipe_handler f, void *ud)
{
    p->close_notifier = f;
    p->ud = ud;
}

int qtk_pipe_connect(qtk_pipe_t *p, qtk_event_module_t *em, qtk_parser_t *ps,
                     const char *path, size_t len)
{
    p->em = em;
    qtk_parser_clone(&p->ps, ps);
    if (qtk_unsocket_connect(&p->s, path, len) < 0) return -1;
    p->connected = 1;
    wtk_fd_set_nonblock(p->s.fd);
    int ret = qtk_event_add_event(p->em, p->s.fd, &p->ev);
    if (ret) qtk_debug("add event fail\n");
    return ret;
}

int qtk_pipe_send(qtk_pipe_t *p, const char *d, int l)
{
    if (l <= 0) return 0;
    if (p->ws && p->ws->len) {
        wtk_stack_push(p->ws, d, l);
        return 0;
    }
    int ret, writed;
    while (l > 0) {
        ret = wtk_fd_send(p->s.fd, (char *)d, l, &writed);
        if (ret == WTK_ERR || ret == WTK_EOF) return -1;
        l -= writed;
        d += writed;
        if (ret == WTK_AGAIN) {
            if (NULL == p->ws) p->ws = wtk_stack_new(512, 1024*1024, 1);
            wtk_stack_push(p->ws, d, l);
            _touch_write(p, cast(qtk_event_handler, _write_process));
        }
    }
    return 0;
}

int qtk_pipe_send_string(qtk_pipe_t *p, const char *d, int l)
{
    int32_t len = l;
    if (0 != qtk_pipe_send(p, (const char *)&len, sizeof(len))) return -1;
    return 0 == qtk_pipe_send(p, d, l) ? 0 : -1;
}

void qtk_pipe_kill(qtk_pipe_t *p, int sig)
{
    kill(p->pid, sig);
}
