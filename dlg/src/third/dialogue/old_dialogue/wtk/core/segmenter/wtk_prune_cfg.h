#ifndef WTK_FST_REC_WTK_PRUNE_CFG
#define WTK_FST_REC_WTK_PRUNE_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_prune_cfg wtk_prune_cfg_t;
struct wtk_prune_cfg
{
	float min_score;
	float max_score;
	float bin_width;
	float beam;
	int count;
};

int wtk_prune_cfg_init(wtk_prune_cfg_t *cfg);
int wtk_prune_cfg_clean(wtk_prune_cfg_t *cfg);
int wtk_prune_cfg_update_local(wtk_prune_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_prune_cfg_update(wtk_prune_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
