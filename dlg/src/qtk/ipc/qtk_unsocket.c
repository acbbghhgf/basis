#include "qtk_unsocket.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"

#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static int _setaddr(qtk_unsocket_t *s, const char *p, size_t l, int remote)
{
    struct sockaddr_un *un = remote ? &s->remote : &s->local;
    if (l >= sizeof(un->sun_path)) {
        qtk_debug("Path too long %.*s\n", (int)l, p);
        return -1;
    }
    strncpy(un->sun_path, p, l);
    un->sun_path[l] = '\0';
    un->sun_family = AF_UNIX;
    return 0;
}

qtk_unsocket_t *qtk_unsocket_new()
{
    qtk_unsocket_t *s = qtk_calloc(1, sizeof(*s));
    if (NULL == s) return NULL;
    s->fd = -1;
    return s;
}

int qtk_unsocket_init(qtk_unsocket_t *s)
{
    memset(s, 0, sizeof(*s));
    s->fd = -1;
    return 0;
}

qtk_unsocket_t *qtk_unsocket_bind(const char *p, size_t l)
{
    qtk_unsocket_t *s = qtk_unsocket_new();
    if (NULL == s) return NULL;
    if ((s->fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror(__FUNCTION__);
        goto err;
    }
    if (_setaddr(s, p, l, 0)) goto err;
    unlink(s->local.sun_path);
    if (bind(s->fd, (struct sockaddr *)&s->local, SUN_LEN(&s->local)) < 0) {
        perror(__FUNCTION__);
        goto err;
    }
    s->bind = 1;
    return s;
err:
    qtk_unsocket_delete(s);
    return NULL;
}

qtk_unsocket_t *qtk_unsocket_serve(const char *p, size_t l, int backlog)
{
    qtk_unsocket_t *s = qtk_unsocket_bind(p, l);
    if (NULL == s) return NULL;
    if (listen(s->fd, backlog)) {
        perror(__FUNCTION__);
        goto err;
    }
    s->passive = 1;
    return s;
err:
    qtk_unsocket_delete(s);
    return NULL;
}

void qtk_unsocket_clean(qtk_unsocket_t *s)
{
    if (s->fd > 0) close(s->fd);
    s->fd = -1;
    if (strlen(s->local.sun_path) > 0 && s->bind) unlink(s->local.sun_path);
}

void qtk_unsocket_delete(qtk_unsocket_t *s)
{
    qtk_unsocket_clean(s);
    qtk_free(s);
}

int qtk_unsocket_connect(qtk_unsocket_t *s, const char *p, size_t l)
{
    if (_setaddr(s, p, l, 1)) return -1;
    int fd = s->bind ? s->fd : socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd <= 0) return -1;
    s->fd = fd;
    if (connect(s->fd, cast(struct sockaddr *, &s->remote), SUN_LEN(&s->remote)) < 0) {
        close(s->fd);
        return -1;
    }
    return s->fd;
}
