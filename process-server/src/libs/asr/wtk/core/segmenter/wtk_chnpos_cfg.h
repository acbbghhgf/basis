#ifndef WTK_CORE_SEGMENTER_WTK_CHNPOS_CFG
#define WTK_CORE_SEGMENTER_WTK_CHNPOS_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_chnpos_model.h"
#include "wtk_posdict.h"
#include "wtk_prune.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_chnpos_cfg wtk_chnpos_cfg_t;

struct wtk_chnpos_cfg
{
	wtk_prune_cfg_t prune;
	wtk_chnpos_model_t *model;
	char *model_fn;
	unsigned use_prune:1;
};

int wtk_chnpos_cfg_init(wtk_chnpos_cfg_t *cfg);
int wtk_chnpos_cfg_clean(wtk_chnpos_cfg_t *cfg);
int wtk_chnpos_cfg_update_local(wtk_chnpos_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_chnpos_cfg_update(wtk_chnpos_cfg_t *cfg);
int wtk_chnpos_cfg_update2(wtk_chnpos_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
