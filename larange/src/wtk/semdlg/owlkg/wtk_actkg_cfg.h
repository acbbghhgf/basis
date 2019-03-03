#ifdef USE_CRF
#ifndef WTK_SEMDLG_OWLKG_WTK_ACTKG_CFG
#define WTK_SEMDLG_OWLKG_WTK_ACTKG_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_crfact_parser.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk_owlkv.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_actkg_cfg wtk_actkg_cfg_t;
struct wtk_actkg_cfg
{
	wtk_crfact_parser_cfg_t crfact;
	wtk_owlkv_cfg_t owlkv;
	char *vt_lex_fn;
	char *rt_lex_fn;
	char *rrt_lex_fn;
	char *parser_lex_fn;
	char *pa_lex_fn;
	char *fst_fn;
	char *nlg_fn;
	char *lua_usr_get;
	wtk_array_t *lua_fn;
	char *v_expand_fn;
	wtk_lex_net_t *net_vt;
	wtk_lex_net_t *net_rt;
	wtk_lex_net_t *net_rrt;
	wtk_lex_net_t *net_parser;
	wtk_lex_net_t *net_pa;
	int nhist;
};

int wtk_actkg_cfg_init(wtk_actkg_cfg_t *cfg);
int wtk_actkg_cfg_clean(wtk_actkg_cfg_t *cfg);
int wtk_actkg_cfg_update_local(wtk_actkg_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_actkg_cfg_update(wtk_actkg_cfg_t *cfg);
int wtk_actkg_cfg_update_lex(wtk_actkg_cfg_t *cfg,wtk_lexc_t *lex);
#endif
#ifdef __cplusplus
};
#endif
#endif
