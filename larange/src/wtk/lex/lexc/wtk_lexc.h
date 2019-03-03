#ifndef WTK_LEX_LEXC_WTK_LEXC
#define WTK_LEX_LEXC_WTK_LEXC
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/wtk_vpool.h"
#include "wtk_lex_script.h"
#include "wtk/lex/net/wtk_lex_net.h"
#include "wtk_lexc_cfg.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexc wtk_lexc_t;

typedef enum {
    WTK_LEXC_INIT, 
	WTK_LEXC_COMMENT,	//"#"
    WTK_LEXC_COMMENT2,	// "/"
    WTK_LEXC_INCLUDE,
    WTK_LEXC_IMPORT,
    WTK_LEXC_EXPR_NAME,
    WTK_LEXC_OUTPUT_TRANS,
    WTK_LEXC_STRING,
    WTK_LEXC_DEF,
    WTK_LEXC_IF,
    WTK_LEXC_ELSE,
    WTK_LEXC_IF_END,
    WTK_LEXC_CMD_END,
    WTK_LEXC_CMD_DEL,
    WTK_LEXC_CMD_ADD,
    WTK_LEXC_CMD_CPY,
    WTK_LEXC_CMD_MV,
} wtk_lexc_state_t;

typedef struct {
    wtk_lexc_state_t back_state;
    int back_sub_state;
    char quoted_char;
    char hex1;
    unsigned quoted :1;
} wtk_lexc_state_string_env_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue_t *value_q;
    wtk_queue_t *or_q;
    union {
        wtk_lex_expr_t *expr;
        wtk_lex_expr_item_value_t *value;
    } v;
} wtk_lexc_env_stk_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lex_expr_scope_t *scope;
    unsigned yes :1;
} wtk_lexc_scope_stk_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lex_expr_output_trans_filter_t *cur_trans_filter;
    wtk_lex_expr_output_trans_if_t *cur_trans_if;
    int sub_state;
    unsigned output_if_cap :1;
    unsigned output_if_yes :1;
} wtk_lexc_trans_stk_t;

struct wtk_lexc {
    wtk_lexc_cfg_t *cfg;
    wtk_heap_t *heap;
    wtk_rbin2_t *rbin;
    wtk_strbuf_t *tok;
    wtk_strbuf_t *buf;

    wtk_str_hash_t *str_var_hash;
    wtk_string_t *str_key;

    wtk_lexc_state_string_env_t string_env;
    wtk_lexc_state_t state;
    int sub_state;
    wtk_string_t *pwd;
    wtk_string_t *lib;

    wtk_lex_script_t *script;
    wtk_lex_expr_t *cur_expr;
    wtk_lex_expr_output_item_t *cur_output_item;
    wtk_lexc_trans_stk_t *trans_tk;
    wtk_queue_t trans_stk_q;
    //wtk_lex_expr_t *cur_scope;

    wtk_vpool_t* stk_pool;
    wtk_queue_t value_stk_q;

    wtk_vpool_t* scope_pool;
    wtk_queue_t scope_stk_q;

    wtk_lexc_get_expr_f get_expr;
    void* get_expr_ths;
    wtk_string_t *pre_dat;
    unsigned use_pre :1;
};

wtk_lexc_t* wtk_lexc_new(wtk_lexc_cfg_t *cfg);
void wtk_lexc_delete(wtk_lexc_t *l);
void wtk_lexc_reset(wtk_lexc_t *l);
wtk_lex_script_t* wtk_lexc_compile(wtk_lexc_t *l, char *data, int bytes);
wtk_lex_script_t* wtk_lexc_compile_file(wtk_lexc_t *l, char *fn);
wtk_lex_net_t* wtk_lexc_compile_file2(wtk_lexc_t *l, char *fn);
wtk_lex_net_t* wtk_lexc_compile_str(wtk_lexc_t *l, char *data, int bytes);
wtk_lex_script_t* wtk_lexc_compile_source(wtk_lexc_t *l, wtk_source_t *src);
wtk_lex_script_t* wtk_lexc_compile_str_script(wtk_lexc_t *l, char *data,
        int bytes);
void wtk_lexc_set_get_expr(wtk_lexc_t *l, void *ths,
        wtk_lexc_get_expr_f get_expr);
#ifdef __cplusplus
}
;
#endif
#endif
