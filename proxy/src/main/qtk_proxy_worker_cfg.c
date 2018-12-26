#include "qtk_proxy_worker_cfg.h"

int qtk_proxy_worker_cfg_init(qtk_proxy_worker_cfg_t *cfg)
{
    cfg->lua = NULL;
    cfg->lua_path = NULL;
    cfg->lua_cpath = NULL;
    return 0;
}

int qtk_proxy_worker_cfg_clean(qtk_proxy_worker_cfg_t *cfg)
{
    return 0;
}

int qtk_proxy_worker_cfg_update(qtk_proxy_worker_cfg_t *cfg)
{
    return 0;
}

int qtk_proxy_worker_cfg_update_local(qtk_proxy_worker_cfg_t *cfg, wtk_local_cfg_t *main)
{
    wtk_string_t *v = NULL;
    wtk_local_cfg_update_cfg_str(main, cfg, lua, v);
    wtk_local_cfg_update_cfg_string(main, cfg, lua_path, v);
    wtk_local_cfg_update_cfg_string(main, cfg, lua_cpath, v);
    wtk_local_cfg_update_cfg_string(main, cfg, lua_ppath, v);
    return 0;
}
