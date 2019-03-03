#ifndef WTK_LEX_LEXR_WTK_LEXR
#define WTK_LEX_LEXR_WTK_LEXR
#include "wtk/core/wtk_type.h"
#include "wtk/lex/net/wtk_lex_net.h"
#include "wtk_lexr_cfg.h"
#include "wtk/core/json/wtk_json.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lexr wtk_lexr_t;
typedef struct wtk_lexr_pth wtk_lexr_pth_t;
typedef struct wtk_lexr_align wtk_lexr_align_t;
typedef struct wtk_lexr_pth_item wtk_lexr_pth_item_t;

typedef enum {
    WTK_LEXR_ALIGN_STR,
    WTK_LEXR_ALIGN_JSON,
    WTK_LEXR_ALIGN_PTH,
    WTK_LEXR_ALIGN_NIL,
} wtk_lexr_align_value_type_t;

struct wtk_lexr_align {
    wtk_lexr_align_t *prev;
    wtk_lex_arc_t *arc;
    union {
        wtk_string_t *str;
        wtk_json_item_t *json;
        wtk_lexr_pth_t *pth;
    } v;
    wtk_lexr_align_value_type_t type;
    int wrd_cnt;
    int pos;
};

struct wtk_lexr_pth_item {
    wtk_lexr_pth_item_t *prev;
    wtk_lexr_align_t *align;
};

struct wtk_lexr_pth {
    wtk_lexr_pth_t *prev;
    wtk_lex_arc_t *arc;	//表达式arc，如pathesis expr,非原子表达式;
    wtk_lexr_pth_item_t *pth_item;
    int depth;
};

typedef struct {
    wtk_queue_node_t tok_n;
    wtk_lex_arc_t *arc;
    wtk_lexr_pth_t *pth;
    wtk_treebin_env_t env;
    int match_cnt;
} wtk_lexr_tok_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_json_item_t *ji;
    int match_wrd_cnt;
} wtk_lexr_output_item_t;

typedef struct {
    wtk_string_t *v;
    int match_wrd_cnt;
} wtk_lexr_output_trans_item_t;

typedef int (*wtk_lexr_has_var_f)(void *ths, char *act, int act_bytes,
        char *name, int name_bytes);

typedef wtk_string_t* (*wtk_lexr_get_var_f)(void *ths, char *act, int act_bytes,
        char *name, int name_bytes);

struct wtk_lexr {
    wtk_lexr_cfg_t *cfg;
    wtk_fkv_t *pron_kv;
    wtk_heap_t *heap;
    wtk_array_t *wrds;
    wtk_strbuf_t *buf;
    wtk_json_t *json;
    wtk_lexr_lib_t *lib;
    wtk_wrdvec_t *wrdvec;
    wtk_hmmnr_t *hmmnr;
    wtk_poseg_t *poseg;
    wtk_json_item_t *action;
    wtk_json_item_t *action_list;
    wtk_json_item_t *action_attr;
    wtk_lexr_output_trans_item_t output_trans;
    wtk_queue_t output_q;
    wtk_queue_t output_tok_q;	//wtk_lexr_output_item_t
    wtk_json_t *rec_json;
    wtk_heap_t *rec_heap;
    wtk_queue_t tok_q;
    wtk_queue_t pend_q;
    int wrd_pos;
    wtk_lex_expr_item_t *cur_expr;
    int match_wrds;
    wtk_lex_net_t *input_net;
    wtk_lexr_has_var_f has_var_f;
    void *has_var_ths;
    wtk_lexr_get_var_f get_var_f;
    void *get_var_ths;
    wtk_string_t *input;
    unsigned run :1;
};

wtk_lexr_t* wtk_lexr_new(wtk_lexr_cfg_t *cfg, wtk_rbin2_t *rbin);
void wtk_lexr_delete(wtk_lexr_t *r);
void wtk_lexr_reset(wtk_lexr_t *r);
void wtk_lexr_set_wrdvec(wtk_lexr_t *r, wtk_wrdvec_t *wvec);

void wtk_lexr_set_has_var_function(wtk_lexr_t *r, void *ths,
        wtk_lexr_has_var_f has_var);
void wtk_lexr_set_get_var_function(wtk_lexr_t *r, void *ths,
        wtk_lexr_get_var_f get_var);
int wtk_lexr_process(wtk_lexr_t *r, wtk_lex_net_t *net, char *input,
        int input_len);

void wtk_lexr_pth_print(wtk_lexr_pth_t *pth);
void wtk_lexr_print(wtk_lexr_t *r);
wtk_string_t wtk_lexr_get_result(wtk_lexr_t *l);

int wtk_lexr_start(wtk_lexr_t *r, wtk_lex_net_t *net);
int wtk_lexr_feed_str(wtk_lexr_t *r, wtk_string_t *v);
wtk_lexr_output_item_t* wtk_lexr_pop_output(wtk_lexr_t *r);
#ifdef __cplusplus
}
;
#endif
#endif
