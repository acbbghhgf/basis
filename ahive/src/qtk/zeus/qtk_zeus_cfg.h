#ifndef QTK_ZEUS_CFG_H
#define QTK_ZEUS_CFG_H
#include "wtk/core/cfg/wtk_local_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_zeus_cfg qtk_zeus_cfg_t;

struct qtk_zeus_cfg {
    char* log_fn;
    char* hw_auth_host;
    int hw_auth_port;
    char* hw_auth_url;
};


int qtk_zeus_cfg_init(qtk_zeus_cfg_t *cfg);
int qtk_zeus_cfg_clean(qtk_zeus_cfg_t *cfg);
int qtk_zeus_cfg_update(qtk_zeus_cfg_t *cfg);
int qtk_zeus_cfg_update_local(qtk_zeus_cfg_t *cfg, wtk_local_cfg_t *lc);


#ifdef __cplusplus
}
#endif


#endif
