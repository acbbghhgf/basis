#ifndef WTK_LEX_OWL_WTK_OWL_CFG
#define WTK_LEX_OWL_WTK_OWL_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owl_cfg wtk_owl_cfg_t;
struct wtk_owl_cfg {
	unsigned pass : 1;
};

int wtk_owl_cfg_init(wtk_owl_cfg_t *cfg);
int wtk_owl_cfg_clean(wtk_owl_cfg_t *cfg);
int wtk_owl_cfg_update_local(wtk_owl_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_owl_cfg_update(wtk_owl_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
