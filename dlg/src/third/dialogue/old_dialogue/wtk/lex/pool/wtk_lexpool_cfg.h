#ifndef WTK_LEX_POOL_WTK_LEXPOOL_CFG
#define WTK_LEX_POOL_WTK_LEXPOOL_CFG
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/lex/wtk_lex_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexpool_cfg wtk_lexpool_cfg_t;

typedef struct {
    wtk_queue_node_t q_n;
    char *fn;
    wtk_string_t *slot;
    wtk_array_t *name;
} wtk_lexpool_item_cfg_t;

struct wtk_lexpool_cfg {
    wtk_lexc_cfg_t lexc;
    wtk_lexr_cfg_t lexr;
    wtk_queue_t item_q;
    int cache_size;
};

int wtk_lexpool_cfg_init(wtk_lexpool_cfg_t *cfg);
int wtk_lexpool_cfg_clean(wtk_lexpool_cfg_t *cfg);
int wtk_lexpool_cfg_update_local(wtk_lexpool_cfg_t *cfg, wtk_local_cfg_t *lc);
int wtk_lexpool_cfg_update(wtk_lexpool_cfg_t *cfg);
#ifdef __cplusplus
}
;
#endif
#endif
