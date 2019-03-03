#ifndef WTK_LEX_NLG_WTK_REXPR
#define WTK_LEX_NLG_WTK_REXPR
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/wtk_if.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_rexpr wtk_rexpr_t;

//\d  匹配任何十进制数；它相当于类 [0-9]。
//string
//var
//(你|我)
typedef enum {
    WTK_REXPR_ITEM_D,
    WTK_REXPR_ITEM_STRING,
    WTK_REXPR_ITEM_VAR,
    WTK_REXPR_ITEM_CAP,
} wtk_rexpr_item_type_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue3_t item_q;	//wtk_rexpr_item_t
    //wtk_queue3_t or_q;
} wtk_rexpr_branch_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue3_t or_q;	//wtk_rexpr_branch_t
} wtk_rexpr_cap_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_rexpr_item_type_t type;
    union {
        wtk_string_t *str;
        wtk_string_t *var;
        wtk_rexpr_cap_t *cap;	//wtk_expr_branch_t
    } v;
    short repeat_min;
    short repeat_max;
} wtk_rexpr_item_t;

struct wtk_rexpr {
    //wtk_queue_node_t q_n;
    wtk_if_t *xif;
    wtk_string_t *name;
    wtk_queue3_t item_q;
};

wtk_rexpr_t* wtk_rexpr_new(wtk_heap_t *heap);
wtk_rexpr_item_t* wtk_rexpr_get_last_item(wtk_rexpr_t *expr);
wtk_rexpr_item_t* wtk_rexpr_new_item_string(wtk_heap_t *heap, char *v,
        int v_len);
wtk_rexpr_item_t* wtk_rexpr_new_item_var(wtk_heap_t *heap, char *v, int v_len);
wtk_rexpr_item_t* wtk_rexpr_new_item_cap(wtk_heap_t *heap);
wtk_rexpr_branch_t* wtk_rexpr_branch_new(wtk_heap_t *heap);
void wtk_rexpr_print(wtk_rexpr_t *expr);

typedef wtk_string_t* (*wtk_rexpr_get_var_f)(void *ths, char *k, int k_len);
void wtk_rexpr_gen(wtk_rexpr_t *expr, wtk_strbuf_t *buf, void *ths,
        wtk_rexpr_get_var_f get_var);

void wtk_rexpr_add_depend(wtk_rexpr_t *expr, char *data, int len,
        wtk_heap_t *heap);
void wtk_rexpr_item_print(wtk_rexpr_item_t *item);
#ifdef __cplusplus
}
;
#endif
#endif
