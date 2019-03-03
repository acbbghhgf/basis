#ifndef _MAIN_QTK_PROXY_WORKER_CFG_H
#define _MAIN_QTK_PROXY_WORKER_CFG_H

#include "wtk/core/cfg/wtk_local_cfg.h"

typedef struct qtk_proxy_worker_cfg qtk_proxy_worker_cfg_t;

struct qtk_proxy_worker_cfg {
    char *lua;
    wtk_string_t *lua_path;
    wtk_string_t *lua_cpath;
    wtk_string_t *lua_ppath;
};

int qtk_proxy_worker_cfg_init(qtk_proxy_worker_cfg_t *cfg);
int qtk_proxy_worker_cfg_clean(qtk_proxy_worker_cfg_t *cfg);
int qtk_proxy_worker_cfg_update(qtk_proxy_worker_cfg_t *cfg);
int qtk_proxy_worker_cfg_update_local(qtk_proxy_worker_cfg_t *cfg, wtk_local_cfg_t *lc);

#endif
