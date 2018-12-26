#include "qtk_dlg_cfg.h"

int qtk_dlg_cfg_init(qtk_dlg_cfg_t *cfg)
{
    cfg->msg_server = NULL;
    return qtk_dlg_worker_cfg_init(&cfg->dw_cfg);
}

int qtk_dlg_cfg_clean(qtk_dlg_cfg_t *cfg)
{
    return qtk_dlg_worker_cfg_clean(&cfg->dw_cfg);
}

int qtk_dlg_cfg_update_local(qtk_dlg_cfg_t *cfg, wtk_local_cfg_t *main)
{
    int ret = -1;
    wtk_local_cfg_t *lc = NULL;
    wtk_string_t *v = NULL;
    wtk_local_cfg_update_cfg_str(main, cfg, msg_server, v);
    wtk_local_cfg_update_cfg_b(main, cfg, use_msg, v);
    lc = wtk_local_cfg_find_lc_s(main, "worker");
    if (lc) {
        ret = qtk_dlg_worker_cfg_update_local(&cfg->dw_cfg, lc);
    }

    return ret;
}

int qtk_dlg_cfg_update(qtk_dlg_cfg_t *cfg)
{
    return qtk_dlg_worker_cfg_update(&cfg->dw_cfg);
}
