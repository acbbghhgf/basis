#ifndef WTK_LEX_POOL_WTK_LEXPOOL
#define WTK_LEX_POOL_WTK_LEXPOOL
#include "wtk/core/wtk_type.h" 
#include "wtk_lexpool_cfg.h"
#include "wtk/lex/wtk_lex.h"
#include "wtk/core/wtk_vpool.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexpool wtk_lexpool_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lexpool_item_cfg_t *cfg;
    wtk_lex_script_t *script;
    wtk_lex_net_t *net;
} wtk_lexpool_item_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lexr_t *rec;
} wtk_lexpool_rec_t;

struct wtk_lexpool {
    wtk_lexpool_cfg_t *cfg;
    wtk_lexc_t *lexc;
    wtk_lexr_lib_t *lib;
    wtk_str_hash_t *net_map;
    wtk_queue_t item_q;
    wtk_hoard_t rec_hoard;
};

wtk_lexpool_t* wtk_lexpool_new(wtk_lexpool_cfg_t *cfg);
void wtk_lexpool_delete(wtk_lexpool_t *p);
void wtk_lexpool_reset(wtk_lexpool_t *p);
wtk_lexpool_item_t* wtk_lexpool_get_item(wtk_lexpool_t *p, char *data,
        int bytes);
wtk_lexpool_rec_t* wtk_lexpool_pop_rec(wtk_lexpool_t *p);
void wtk_lexpool_push_rec(wtk_lexpool_t *p, wtk_lexpool_rec_t *r);
wtk_string_t* wtk_lexpool_item_get_json_value(wtk_lexpool_item_t *vi,
        wtk_json_item_t *ji);
#ifdef __cplusplus
}
;
#endif
#endif
