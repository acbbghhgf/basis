#include "qtk_variant.h"
#include "qtk_cmd.h"
#include "qtk_incub.h"
#include "qtk_fty.h"
#include "qtk_inst_ctl.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"

#include <assert.h>

qtk_variant_t *qtk_variant_new(pid_t pid, const char *name, size_t nl)
{
    qtk_variant_t *v = qtk_calloc(1, sizeof(*v) + nl);
    v->name.data = (char *)(v + 1);
    v->name.len = nl;
    memcpy(v->name.data, name, nl);
    qtk_pipe_init(&v->p);
    v->p.pid = pid;
    return v;
}

void qtk_variant_delete(qtk_variant_t *v)
{
    qtk_pipe_clean(&v->p);
    qtk_free(v);
}

int qtk_variant_write_cmd(qtk_variant_t *v, int cmd, const char *param)
{
    char buf[4096];
    int len = snprintf(buf, sizeof(buf), "%da%s", cmd, param);
    return qtk_pipe_send_string(&v->p, buf, len + 1); // use '\0' terminate string
}

static void _update_cnt(qtk_variant_t *v)
{
    if (v->raw) {
        qtk_ictl_update_cnt_s("raw", -1);
        qtk_ictl_flush_s("raw");
    } else {
        qtk_ictl_update_cnt(v->name.data, v->name.len, -1);
        qtk_ictl_flush(v->name.data, v->name.len);
    }
}

static int _close_notifier(void *ud, qtk_pipe_t *p)
{
    qtk_variant_t *v = data_offset(p, qtk_variant_t, p);
    if (v->zygote)
        qtk_incub_remove(v->name.data, v->name.len);
    else {
        qtk_fty_remove(v);
    }
    qtk_ictl_try_remove(v->name.data, v->name.len);
    _update_cnt(v);
    if (v->used) {
        qtk_pipe_clean(&v->p);
        v->closed = 1;
    } else {
        qtk_variant_delete(v);
    }
    return 0;
}

int qtk_variant_prepare(qtk_variant_t *v, qtk_event_module_t *em, qtk_parser_t *ps,
                        const char *p, size_t l)
{
    if (v->p.connected == 1) return 0;
    qtk_pipe_set_close_notifier(&v->p, _close_notifier, NULL);
    return qtk_pipe_connect(&v->p, em, ps, p, l);
}

void qtk_variant_kill(qtk_variant_t *v, int sig)
{
    qtk_debug("%.*s[%p] killed\n", v->name.len, v->name.data, v);
    qtk_pipe_kill(&v->p, sig);
}

int qtk_variant_exit(qtk_variant_t *v)
{
    qtk_debug("%.*s[%p] exiting\n", v->name.len, v->name.data, v);
    if (qtk_variant_write_cmd(v, EXIT, NULL))
        qtk_variant_kill(v, SIGKILL);
    return 0;
}
