#ifndef WTK_CORE_TEXT_WTK_TXTPEEK_CFG
#define WTK_CORE_TEXT_WTK_TXTPEEK_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_txtpeek_cfg wtk_txtpeek_cfg_t;
struct wtk_txtpeek_cfg
{
	unsigned merge_num:1;
	unsigned skip_oov:1;
};

int wtk_txtpeek_cfg_init(wtk_txtpeek_cfg_t *cfg);
int wtk_txtpeek_cfg_clean(wtk_txtpeek_cfg_t *cfg);
int wtk_txtpeek_cfg_update_local(wtk_txtpeek_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_txtpeek_cfg_update(wtk_txtpeek_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
