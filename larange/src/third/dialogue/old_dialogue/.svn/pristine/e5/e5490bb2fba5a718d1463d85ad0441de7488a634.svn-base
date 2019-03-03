#ifndef WTK_LEX_LEXR_WTK_LEXR_LIB
#define WTK_LEX_LEXR_WTK_LEXR_LIB
#include "wtk/core/wtk_type.h" 
#include "wtk_lexr_lib_cfg.h"
#include "wtk_treebin.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexr_lib wtk_lexr_lib_t;
struct wtk_lexr_lib {
    wtk_lexr_lib_cfg_t *cfg;
    wtk_treebin_t *tree;
};

wtk_lexr_lib_t* wtk_lexr_lib_new(wtk_lexr_lib_cfg_t *cfg, wtk_rbin2_t *rbin);
void wtk_lexr_lib_delete(wtk_lexr_lib_t *l);
wtk_treebin_env_t wtk_lexr_lib_get_env(wtk_lexr_lib_t *l, char *var,
        int var_bytes);
int wtk_lexr_lib_search(wtk_lexr_lib_t *l, wtk_treebin_env_t *env, char *v,
        int v_bytes);
#ifdef __cplusplus
}
;
#endif
#endif
