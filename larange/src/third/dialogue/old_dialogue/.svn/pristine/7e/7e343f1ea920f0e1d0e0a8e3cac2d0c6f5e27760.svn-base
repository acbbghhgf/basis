#ifndef WTK_LEX_LEXR_WTK_LEXR_LIB_CFG
#define WTK_LEX_LEXR_WTK_LEXR_LIB_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexr_lib_cfg wtk_lexr_lib_cfg_t;

typedef struct {
    wtk_string_t *name;
    wtk_string_t *value;
} wtk_lexr_lib_item_t;

struct wtk_lexr_lib_cfg {
    char *tree_fn;
    wtk_str_hash_t *map_hash;
};

int wtk_lexr_lib_cfg_init(wtk_lexr_lib_cfg_t *cfg);
int wtk_lexr_lib_cfg_clean(wtk_lexr_lib_cfg_t *cfg);
int wtk_lexr_lib_cfg_update_local(wtk_lexr_lib_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lexr_lib_cfg_update(wtk_lexr_lib_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
