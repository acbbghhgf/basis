#ifndef WTK_LUA_WTK_LUA2_CFG_H_
#define WTK_LUA_WTK_LUA2_CFG_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lua2_cfg wtk_lua2_cfg_t;
struct wtk_lua2_cfg
{
	wtk_array_t *libs;
	wtk_array_t *include_path;
	char *fn;
	char *local_lang;
};

int wtk_lua2_cfg_init(wtk_lua2_cfg_t *cfg);
int wtk_lua2_cfg_clean(wtk_lua2_cfg_t *cfg);
int wtk_lua2_cfg_update_local(wtk_lua2_cfg_t *cfg,wtk_local_cfg_t *lc);
int wtk_lua2_cfg_update(wtk_lua2_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
