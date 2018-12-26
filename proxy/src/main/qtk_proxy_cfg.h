#ifndef QTK_MAIN_PROXY_CFG_H_
#define QTK_MAIN_PROXY_CFG_H_

#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk_proxy_worker_cfg.h"

typedef struct qtk_proxy_cfg qtk_proxy_cfg_t;

struct qtk_proxy_cfg {
    char *msg_addr;
    char *listen_addr;
    int listen_backlog;
    qtk_proxy_worker_cfg_t worker;
};

int qtk_proxy_cfg_init(qtk_proxy_cfg_t *cfg);
int qtk_proxy_cfg_clean(qtk_proxy_cfg_t *cfg);
int qtk_proxy_cfg_update(qtk_proxy_cfg_t *cfg);
int qtk_proxy_cfg_update_local(qtk_proxy_cfg_t *cfg, wtk_local_cfg_t *lc);

#endif
