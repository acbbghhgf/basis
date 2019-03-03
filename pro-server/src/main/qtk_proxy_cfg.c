#include "qtk_proxy_cfg.h"

int qtk_proxy_cfg_init(qtk_proxy_cfg_t *cfg)
{
    qtk_proxy_worker_cfg_init(&cfg->worker);
    return 0;
}

int qtk_proxy_cfg_clean(qtk_proxy_cfg_t *cfg)
{
    qtk_proxy_worker_cfg_clean(&cfg->worker);
    return 0;
}

int qtk_proxy_cfg_update(qtk_proxy_cfg_t *cfg)
{
    qtk_proxy_worker_cfg_update(&cfg->worker);
    return 0;
}

int qtk_proxy_cfg_update_local(qtk_proxy_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v = NULL;
    wtk_local_cfg_t *lc = NULL;
    wtk_local_cfg_update_cfg_str(main, cfg, msg_addr, v);
    wtk_local_cfg_update_cfg_str(main, cfg, listen_addr, v);
    wtk_local_cfg_update_cfg_i(main, cfg, listen_backlog, v);
    lc = wtk_local_cfg_find_lc_s(main, "worker");
    if (lc) {
        qtk_proxy_worker_cfg_update_local(&cfg->worker, lc);
    }
    return 0;
}
