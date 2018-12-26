#ifndef WTK_SEMDLG_SEMFST_WTK_SEMFSTR_CFG
#define WTK_SEMDLG_SEMFST_WTK_SEMFSTR_CFG
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/wtk_lex_cfg.h"
#include "wtk/lua/wtk_lua2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semfstr_cfg wtk_semfstr_cfg_t;
struct wtk_semfstr_cfg
{
	char *dn;
	char *lua_main;
	char *lua_save;
	char *lua_ask;
	wtk_array_t *lua;
};

int wtk_semfstr_cfg_init(wtk_semfstr_cfg_t *cfg);
int wtk_semfstr_cfg_clean(wtk_semfstr_cfg_t *cfg);
int wtk_semfstr_cfg_update_local(wtk_semfstr_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_semfstr_cfg_update(wtk_semfstr_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
