#include "wtk/core/cfg/wtk_main_cfg.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk_proxy.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"
#include "os/qtk_thread.h"

static qtk_proxy_cfg_t *_get_cfg(wtk_local_cfg_t *lc)
{
    int ret = -1;
    qtk_proxy_cfg_t *cfg = qtk_calloc(1, sizeof(*cfg));
    if (NULL == cfg) return NULL;
    qtk_proxy_cfg_init(cfg);
    ret = qtk_proxy_cfg_update_local(cfg, lc);
    if (ret) goto end;
    ret = qtk_proxy_cfg_update(cfg);
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

static void _clean_cfg(qtk_proxy_t *p)
{
    qtk_proxy_cfg_clean(p->cfg);
    qtk_free(p->cfg);
}

static int _proxy_init(qtk_proxy_t *p)
{
    return 0;
}

qtk_proxy_t *qtk_proxy_new(void *gate)
{
    wtk_local_cfg_t *lc = ((qtk_sframe_method_t *)gate)->get_cfg(gate);
    qtk_proxy_cfg_t *cfg = _get_cfg(lc);
    if (NULL == cfg) return NULL;
    qtk_proxy_t *p = qtk_calloc(1, sizeof(qtk_proxy_t));
    if (NULL == p) { return NULL; }
    p->gate = gate;
    p->cfg = cfg;
    p->worker = qtk_proxy_worker_new(p, &p->cfg->worker);
    _proxy_init(p);
    return p;
}

int qtk_proxy_delete(qtk_proxy_t *p)
{
    qtk_proxy_worker_delete(p->worker);
    _clean_cfg(p);
    qtk_free(p);
    return 0;
}

int qtk_proxy_start(qtk_proxy_t *p)
{
    p->run = 1;
    qtk_proxy_worker_start(p->worker);
    return 0;
}

int qtk_proxy_stop(qtk_proxy_t *p)
{
    p->run = 0;
    qtk_proxy_worker_stop(p->worker);
    return 0;
}
