#ifndef WTK_EXT_USC_WTK_USC_CFG
#define WTK_EXT_USC_WTK_USC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_usc_cfg qtk_usc_cfg_t;
struct qtk_usc_cfg
{
    char *app_key;
    char *secret_key;
};

int qtk_usc_cfg_init(qtk_usc_cfg_t *cfg);
int qtk_usc_cfg_clean(qtk_usc_cfg_t *cfg);
int qtk_usc_cfg_update_local(qtk_usc_cfg_t *cfg,wtk_local_cfg_t *lc);
int qtk_usc_cfg_update(qtk_usc_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
