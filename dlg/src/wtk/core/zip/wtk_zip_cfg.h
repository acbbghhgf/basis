#ifndef WTK_CORE_ZIP_WTK_ZIP_CFG
#define WTK_CORE_ZIP_WTK_ZIP_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_zip_cfg wtk_zip_cfg_t;
struct wtk_zip_cfg
{
	int blk_size;
	int bits;
};

int wtk_zip_cfg_init(wtk_zip_cfg_t *cfg);
int wtk_zip_cfg_clean(wtk_zip_cfg_t *cfg);
int wtk_zip_cfg_update_local(wtk_zip_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_zip_cfg_update(wtk_zip_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
