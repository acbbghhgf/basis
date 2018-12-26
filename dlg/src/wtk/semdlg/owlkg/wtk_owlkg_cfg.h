#ifndef WTK_SEMDLG_OWLKG_WTK_OWLKG_CFG
#define WTK_SEMDLG_OWLKG_WTK_OWLKG_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owlkg_cfg wtk_owlkg_cfg_t;
struct wtk_owlkg_cfg
{
	wtk_array_t *lua;
	char *nlg;
	char *owl;
	char *brain_dn;
};

int wtk_owlkg_cfg_init(wtk_owlkg_cfg_t *cfg);
int wtk_owlkg_cfg_clean(wtk_owlkg_cfg_t *cfg);
int wtk_owlkg_cfg_update_local(wtk_owlkg_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_owlkg_cfg_update(wtk_owlkg_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
