#ifndef WTK_CORE_MATH_WTK_CLSVEC_CFG
#define WTK_CORE_MATH_WTK_CLSVEC_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_clsvec_cfg wtk_clsvec_cfg_t;
struct wtk_clsvec_cfg
{
	int use_idx:1;
};

int wtk_clsvec_cfg_init(wtk_clsvec_cfg_t *cfg);
int wtk_clsvec_cfg_clean(wtk_clsvec_cfg_t *cfg);
int wtk_clsvec_cfg_update_local(wtk_clsvec_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_clsvec_cfg_update(wtk_clsvec_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
