#include "qtk_dlg.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/os/qtk_base.h"
#include "main/qtk_xfer.h"

static qtk_dlg_cfg_t *_get_cfg(wtk_local_cfg_t *lc)
{
    int ret = -1;
    qtk_dlg_cfg_t *cfg = qtk_calloc(1, sizeof(*cfg));
    if (NULL == cfg) return NULL;
    qtk_dlg_cfg_init(cfg);
    ret = qtk_dlg_cfg_update_local(cfg, lc);
    if (ret) goto end;
    ret = qtk_dlg_cfg_update(cfg);
    if (ret) goto end;
    ret = 0;
end:
    if (ret) {
        qtk_debug("get cfg fail\n");
        qtk_free(cfg);
        return NULL;
    }
    return cfg;
}

static void _clean_cfg(qtk_dlg_t *d)
{
    qtk_dlg_cfg_clean(d->cfg);
    qtk_free(d->cfg);
}

qtk_dlg_t *qtk_dlg_new(qtk_sframe_method_t *gate)
{
    qtk_dlg_t *d = qtk_calloc(1, sizeof(qtk_dlg_t));
    if (d == NULL) { return NULL; }
    d->gate = gate;
    wtk_local_cfg_t *lc = gate->get_cfg(gate);
    d->cfg = _get_cfg(lc);
    d->worker = qtk_dlg_worker_new(d, &d->cfg->dw_cfg);
    return d;
}

int qtk_dlg_delete(qtk_dlg_t *d)
{
    qtk_dlg_worker_delete(d->worker);
    _clean_cfg(d);
    qtk_free(d);
    return 0;
}

static void _dlg_raise_proto(qtk_dlg_t *d, qtk_sframe_msg_t *msg)
{
    wtk_strbuf_t *res = NULL;
    qtk_sframe_method_t *method = d->gate;
    res = method->get_body(msg);
    if (NULL == res) { return; }
    qtk_dlg_event_t *ev = qtk_dlg_event_new_proto(res->data, res->pos);
    ev->id = d->gate->get_id(msg);
    wtk_pipequeue_push(&d->worker->input_q, &ev->node);
}

static void _dlg_raise_sig(qtk_dlg_t *d, qtk_sframe_msg_t *msg)
{
    qtk_dlg_event_t *ev = qtk_dlg_event_new_sig(d->gate->get_signal(msg));
    wtk_pipequeue_push(&d->worker->input_q, &ev->node);
}

static int qtk_dlg_handle_msg(qtk_dlg_t *d, qtk_sframe_msg_t *msg)
{
    int sig, id;
    switch(d->gate->get_type(msg)) {
    case QTK_SFRAME_MSG_REQUEST:    // FALLTHROUGH
    case QTK_SFRAME_MSG_RESPONSE:
        _dlg_raise_proto(d, msg);
        break;
    case QTK_SFRAME_MSG_NOTICE:
        sig = d->gate->get_signal(msg);
        id = d->gate->get_id(msg);
        if (sig == QTK_SFRAME_SIGDISCONNECT || sig == QTK_SFRAME_SIGECONNECT)
            qtk_xfer_destroy(d->gate, id);
        if (d->worker->msg_sock_id == id)
            _dlg_raise_sig(d, msg);
        break;
    default:
        qtk_debug("unknown msg type\n");
    }

    return 0;
}

static void *qtk_dlg_route(void *arg)
{
    qtk_sframe_msg_t *msg = NULL;
    qtk_dlg_t *d = arg;
    qtk_sframe_method_t *method = d->gate;
    for (;d->run;) {
        msg = method->pop(method, 100);
        if (msg) {
            qtk_dlg_handle_msg(d, msg);
            method->delete(method, msg);
        }
    }
    return NULL;
}

int qtk_dlg_start(qtk_dlg_t *d)
{
    d->run = 1;
    qtk_dlg_worker_start(d->worker);
    qtk_thread_init(&d->route, qtk_dlg_route, d);
    qtk_thread_start(&d->route);
    return 0;
}

int qtk_dlg_stop(qtk_dlg_t *d)
{
    d->run = 0;
    qtk_thread_join(&d->route);
    qtk_dlg_worker_stop(d->worker);
    return 0;
}

qtk_dlg_event_t *qtk_dlg_event_new_proto(const char *data, int len)
{
    qtk_dlg_event_t *ev = qtk_calloc(1, sizeof(qtk_dlg_event_t) + len);
    if (NULL == ev) return NULL;
    ev->d.data.data = (char *)(ev + 1);
    ev->d.data.len = len;
    memcpy(ev->d.data.data, data, len);
    wtk_queue_node_init(&ev->node);
    ev->is_sig = 0;
    return ev;
}

qtk_dlg_event_t *qtk_dlg_event_new_sig(int sig)
{
    qtk_dlg_event_t *ev = qtk_calloc(1, sizeof(qtk_dlg_event_t));
    if (NULL == ev) return NULL;
    wtk_queue_node_init(&ev->node);
    ev->is_sig = 1;
    ev->d.sig = sig;
    return ev;
}

void qtk_dlg_event_delete(qtk_dlg_event_t *ev)
{
    qtk_free(ev);
}
