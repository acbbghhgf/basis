#ifndef WTK_FST_STRLIKE_WTK_CHNLIKE_CFG
#define WTK_FST_STRLIKE_WTK_CHNLIKE_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/text/wtk_txtpeek_cfg.h"
#include "wtk/core/wtk_strdict.h"
#include "wtk_strlike_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_chnlike_cfg wtk_chnlike_cfg_t;

struct wtk_chnlike_cfg
{
	wtk_txtpeek_cfg_t txtpeek;
	wtk_strlike_cfg_t strlike;
	wtk_strdict_t *dict;
	int hash_hint;
	char *dict_fn;
	unsigned use_bin:1;
};

int wtk_chnlike_cfg_init(wtk_chnlike_cfg_t *cfg);
int wtk_chnlike_cfg_clean(wtk_chnlike_cfg_t *cfg);
int wtk_chnlike_cfg_update_local(wtk_chnlike_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_chnlike_cfg_update(wtk_chnlike_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
