#ifndef WTK_LEX_FST_WTK_NLGFST
#define WTK_LEX_FST_WTK_NLGFST
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/nlg/wtk_nlg.h"
#include "wtk/lex/nlg/wtk_nlg2.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_queue2.h"
#include "wtk_nlgnet.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlgfst wtk_nlgfst_t;
typedef int (*wtk_nlgfst_emit_f)(void *ths, wtk_nlg_item_t *item);

/**
 * return 0 for success, other break;
 */
//typedef int(*wtk_nlgfst_feed_lua_f)(void *ths,char *func);
typedef int (*wtk_nlgfst_act_has_key_f)(void *ths, char *k, int k_bytes);
//typedef wtk_string_t*(*wtk_nlgfst_act_get_value_f)(void *ths,char *k,int k_bytes);
typedef int (*wtk_nlgfst_act_nvalue_f)(void *ths);

typedef struct {
    void *ths;
    wtk_nlg_act_get_lua_f feed_lua;
    wtk_nlgfst_act_has_key_f has_key;
    wtk_nlg_act_get_value_f get_value;
    wtk_nlgfst_act_nvalue_f nvalue;
    wtk_strbuf_t *buf;
} wtk_nlgfst_act_t;

struct wtk_nlgfst {
    wtk_nlg_t *nlg;
    wtk_nlgnet_t *net;
    wtk_heap_t *heap;
    wtk_queue_t input_slot_q;
    wtk_nlgnet_state_t *cur_state;
    int cur_state_round;
    void* emit_ths;
    wtk_nlgfst_emit_f emit_process;
};

wtk_nlgfst_t* wtk_nlgfst_new(wtk_rbin2_t *rbin, char *nlg_fn, char *fst_fn);
wtk_nlgfst_t* wtk_nlgfst_new2(wtk_rbin2_t *rbin, char *nlg_fn, char *fst_fn,
        int use_nlg2);
void wtk_nlgfst_delete(wtk_nlgfst_t *fst);
void wtk_nlgfst_reset(wtk_nlgfst_t *fst);
int wtk_nglfst_can_be_end(wtk_nlgfst_t *fst);
void wtk_nlgfst_feed(wtk_nlgfst_t *fst, char *data, int bytes);
void wtk_nlgfst_set_emit(wtk_nlgfst_t *fst, void *ths, wtk_nlgfst_emit_f emit);
void wtk_nlgfst_set_state(wtk_nlgfst_t *fst, char *state, int state_bytes);
int wtk_nlgfst_feed2(wtk_nlgfst_t *fst, wtk_nlgfst_act_t *act);
#ifdef __cplusplus
}
;
#endif
#endif
