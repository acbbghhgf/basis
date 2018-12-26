#ifndef WTK_LEX_NET_WTK_LEX_NET
#define WTK_LEX_NET_WTK_LEX_NET
#include "wtk/core/wtk_type.h" 
#include "wtk/lex/lexc/wtk_lex_script.h"
#include "wtk/core/wtk_queue2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lex_net wtk_lex_net_t;
typedef struct wtk_lex_node wtk_lex_node_t;
typedef struct wtk_lex_arc wtk_lex_arc_t;
typedef struct wtk_lex_net_root wtk_lex_net_root_t;

//typedef struct wtk_lex_capture wtk_lex_capture_t;
//
//struct wtk_lex_capture
//{
//	wtk_lex_capture_t *parent;
//	wtk_lex_expr_item_value_t *v;
//};

struct wtk_lex_node {
    wtk_queue2_t in_q;
    wtk_queue2_t out_q;
    /*
     * WTK_LEX_VALUE_PARENTHESES
     * WTK_LEX_VALUE_EXPR
     */
    //wtk_lex_expr_item_value_t *value;
    wtk_lex_arc_t *complex_input_arc;
    //unsigned touch:1;
};

struct wtk_lex_arc {
    wtk_queue_node_t in_n;
    wtk_queue_node_t out_n;
    wtk_lex_node_t *from;
    wtk_lex_node_t *to;
    /*
     * 	WTK_LEX_VALUE_STRING
     WTK_LEX_VALUE_BRACKET
     WTK_LEX_VALUE_PERIOD
     WTK_LEX_VALUE_RANGE
     WTK_LEX_VALUE_ESCAPE
     WTK_LEX_VALUE_BUILDIN_VAR
     */
    wtk_lex_expr_item_value_t *value;
};

typedef struct {
    wtk_lex_net_root_t **yes;
    wtk_lex_net_root_t **no;
    unsigned int nyes;
    unsigned int nno;
} wtk_lex_net_scope_t;

struct wtk_lex_net_root {
    wtk_lex_expr_t *expr;
    union {
        wtk_lex_node_t *node;
        wtk_lex_net_scope_t *scope;
    } v;
};

struct wtk_lex_net {
    wtk_lex_script_t *script;
    wtk_heap_t *heap;
    wtk_lex_net_root_t **roots;
    unsigned int nroot;
    unsigned int use_poseg :1;
};

wtk_lex_net_t* wtk_lex_net_new(wtk_lex_script_t *script);
void wtk_lex_net_delete(wtk_lex_net_t *net);

void wtk_lex_node_print(wtk_lex_node_t *n);
void wtk_lex_node_print3(wtk_lex_node_t *n);
#ifdef __cplusplus
}
;
#endif
#endif
