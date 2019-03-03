#ifndef WTK_SEMDLG_SEMFLD_WTK_SEMFLD_CFG
#define WTK_SEMDLG_SEMFLD_WTK_SEMFLD_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/nlg/wtk_nlg.h"
#include "wtk/lex/owl/wtk_owl.h"
#include "wtk/semdlg/kg/wtk_kgr.h"
#include "wtk_semslot.h"
#include "wtk_semfi.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfld_cfg wtk_semfld_cfg_t;
struct wtk_semfld_cfg
{
	wtk_string_t name;
	wtk_string_t ename;
	wtk_semfi_cfg_t semfi;
	wtk_kgr_cfg_t kg;
	wtk_semslot_cfg_t slot;
	wtk_owl_tree_t *owl;
	wtk_nlg_t *nlg;
	wtk_nlg_root_t *nlg_main;
	wtk_nlg_root_t *nlg_usr_inform;
	wtk_nlg_root_t *nlg_usr_set;
	wtk_nlg_root_t *nlg_usr_del;
	wtk_nlg_root_t *nlg_sys_ask;
	wtk_nlg_root_t *nlg_sys_answer;
	char *dn;
	char *luafn;
	char *nlgfn;
	char *fstfn;
	char *owlfn;
	char *lua_flush;
	char *lua_env_init;
	char *dat_dn;
	wtk_string_t *lex_pre_dat;
	wtk_string_t nlg_close;
	int max_empty_input_try;
	unsigned use_kg:1;
	unsigned use_dat_env:1;
	unsigned use_nlg2:1;
};

int wtk_semfld_cfg_init(wtk_semfld_cfg_t *cfg);
int wtk_semfld_cfg_clean(wtk_semfld_cfg_t *cfg);
int wtk_semfld_cfg_update_local(wtk_semfld_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_semfld_cfg_update(wtk_semfld_cfg_t *cfg,wtk_lexc_t *lex);
int wtk_semfld_cfg_update2(wtk_semfld_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lex);
#ifdef __cplusplus
};
#endif
#endif
