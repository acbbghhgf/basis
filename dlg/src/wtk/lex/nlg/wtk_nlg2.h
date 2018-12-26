#ifndef WTK_LEX_NLG_WTK_NLG2
#define WTK_LEX_NLG_WTK_NLG2
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_sort.h"
#include "wtk_nlg2_key.h"
#include "wtk_rexpr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlg2 wtk_nlg2_t;

typedef struct {
    wtk_queue_node_t q_n;
    union v {
        wtk_rexpr_t *expr;
        char *lua;
    } v;
    unsigned is_expr :1;
} wtk_nlg2_expr_item_t;

typedef void (*wtk_nlg2_get_lua_gen_f)(void *ths, char *func,
        wtk_nlg2_function_t *f, wtk_strbuf_t *buf);
typedef wtk_string_t (*wtk_nlg2_get_lua_gen2_f)(void *ths, char *func);

typedef struct {
    void *ths;
    wtk_nlg2_get_lua_gen_f lua_gen;
    wtk_nlg2_get_lua_gen2_f lua_gen2;
} wtk_nlg2_gen_env_t;

typedef struct wtk_nlg2_item wtk_nlg2_item_t;

struct wtk_nlg2_item {
    wtk_queue_node_t q_n;
    wtk_nlg2_function_t function;
    char *pre;	//+
    union {
        wtk_string_t *str;	//= used for parser;
        wtk_nlg2_item_t *item;
    } before; //<
    union {
        wtk_string_t *str;	//= used for parser;
        wtk_nlg2_item_t *item;
    } nxt;	//=
    wtk_queue3_t *local_expr_q;
    wtk_queue3_t *glb_expr_q;	//wtk_rexpr_t
};

typedef struct {
    wtk_queue3_t item_q;	//search px(c,b,a)
} wtk_nlg2_slot_t;

struct wtk_nlg2 {
    wtk_str_hash_t *action_hash;
    wtk_strbuf_t *buf;
    wtk_heap_t *rec_heap;
    wtk_nlg2_item_t *global;
};

wtk_nlg2_t* wtk_nlg2_new(int n);
void wtk_nlg2_delete(wtk_nlg2_t *nlg);
void wtk_nlg2_add_item(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item);
wtk_nlg2_item_t* wtk_nlg2_new_item(wtk_nlg2_t *nlg);
void wtk_nlg2_add_local_expr(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_rexpr_t *expr);
void wtk_nlg2_add_global_expr(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_rexpr_t *expr);
void wtk_nlg2_add_global_lua(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item, char *data,
        int len);
wtk_string_t wtk_nlg2_process(wtk_nlg2_t *nlg, char *data, int len,
        wtk_nlg2_gen_env_t *env);
wtk_string_t wtk_nlg2_process_function(wtk_nlg2_t *nlg, wtk_nlg2_function_t *f,
        wtk_nlg2_gen_env_t *env);
wtk_nlg2_item_t* wtk_nlg2_find_item(wtk_nlg2_t *nlg, wtk_nlg2_function_t *f);
void wtk_nlg2_print(wtk_nlg2_t *nlg);
void wtk_nlg2_item_print(wtk_nlg2_item_t *item);
void wtk_nlg2_update(wtk_nlg2_t *nlg);
void wtk_nlg2_item_gen(wtk_nlg2_t *nlg, wtk_nlg2_item_t *item,
        wtk_strbuf_t *buf, wtk_heap_t *heap, wtk_nlg2_function_t *f,
        wtk_nlg2_gen_env_t *env);
#ifdef __cplusplus
}
;
#endif
#endif
