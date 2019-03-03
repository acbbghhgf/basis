#ifndef WTK_LEX_NLG_WTK_REXPR_PARSER
#define WTK_LEX_NLG_WTK_REXPR_PARSER
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk_rexpr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rexpr_parser wtk_rexpr_parser_t;

typedef enum {
    WTK_REXPR_INIT, WTK_REXPR_NAME, WTK_EXPR_ITEM,
} wtk_rexpr_state_t;

typedef struct {
    wtk_queue_node_t q_n;
    //wtk_rexpr_item_t *item;
    union {
        wtk_queue3_t *item_q;
        wtk_rexpr_cap_t *cap;
    } v;
    unsigned is_cap :1;
} wtk_rexpr_stack_t;

struct wtk_rexpr_parser {
    wtk_heap_t *stk_heap;
    wtk_heap_t *heap;
    wtk_rexpr_t *expr;
    wtk_strbuf_t *buf;
    wtk_rexpr_state_t state;
    wtk_var_parse_t varparser;
    int sub_state;
    wtk_queue_t stk_q;
};

wtk_rexpr_parser_t* wtk_rexpr_parser_new();
void wtk_rexpr_parser_delete(wtk_rexpr_parser_t *expr);
void wtk_rexpr_parser_reset(wtk_rexpr_parser_t *p);
wtk_rexpr_t* wtk_rexpr_parser_process(wtk_rexpr_parser_t *p, wtk_heap_t *heap,
        char *s, int len);
#ifdef __cplusplus
}
;
#endif
#endif
