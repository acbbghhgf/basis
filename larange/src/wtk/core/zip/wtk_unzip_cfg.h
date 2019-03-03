#ifndef WTK_CORE_ZIP_WTK_UNZIP_CFG
#define WTK_CORE_ZIP_WTK_UNZIP_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_unzip_cfg wtk_unzip_cfg_t;
struct wtk_unzip_cfg
{
};

int wtk_unzip_cfg_init(wtk_unzip_cfg_t *cfg);
int wtk_unzip_cfg_clean(wtk_unzip_cfg_t *cfg);
int wtk_unzip_cfg_update_local(wtk_unzip_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_unzip_cfg_update(wtk_unzip_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif