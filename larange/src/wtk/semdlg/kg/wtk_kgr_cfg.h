#ifndef WTK_SEMDLG_KG_WTK_KGR_CFG
#define WTK_SEMDLG_KG_WTK_KGR_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_kgc.h"
#include "wtk_kg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_kgr_cfg wtk_kgr_cfg_t;
struct wtk_kgr_cfg
{
	wtk_kg_t *kg;
	char *nlg_fn;
	char *kg_fn;
	char* inst_dn;
	char* class_dn;
	char *lua_init;
	char *lua_json_map;
	unsigned use_random:1;
};

int wtk_kgr_cfg_init(wtk_kgr_cfg_t *cfg);
int wtk_kgr_cfg_clean(wtk_kgr_cfg_t *cfg);
int wtk_kgr_cfg_update_local(wtk_kgr_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_kgr_cfg_update(wtk_kgr_cfg_t *cfg);
int wtk_kgr_cfg_update2(wtk_kgr_cfg_t *cfg,wtk_source_loader_t *sl);
#ifdef __cplusplus
};
#endif
#endif
