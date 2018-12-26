#ifndef WTK_FST_REC_WTK_PRUNE
#define WTK_FST_REC_WTK_PRUNE
#include "wtk/core/wtk_type.h" 
#include "wtk_prune_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_prune wtk_prune_t;
struct wtk_prune
{
	wtk_prune_cfg_t *cfg;
	float max;
	float min;
	int nbin;
	float bin_width_scale;
	float min_x;
	unsigned int *bins;
	unsigned int count;
	float thresh;
};


wtk_prune_t* wtk_prune_new(wtk_prune_cfg_t *cfg);
void wtk_prune_delete(wtk_prune_t *p);
void wtk_prune_reset(wtk_prune_t *p);
void wtk_prune_add(wtk_prune_t *p,float f);
float wtk_prune_get_thresh(wtk_prune_t *p);
float wtk_prune_get_thresh2(wtk_prune_t *p,int count);
void wtk_prune_update_thresh(wtk_prune_t *p);
int wtk_prune_want_prune(wtk_prune_t *p);
void wtk_prune_plus(wtk_prune_t *dst,wtk_prune_t *src);
#ifdef __cplusplus
};
#endif
#endif
