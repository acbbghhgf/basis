#ifndef WTK_LEX_OWL_WTK_OWL_TREE
#define WTK_LEX_OWL_WTK_OWL_TREE
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_larray.h"
#include "wtk/core/wtk_sort.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_if.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_owl_tree wtk_owl_tree_t;
typedef struct wtk_owl_class wtk_owl_class_t;
typedef struct wtk_owl_inst wtk_owl_inst_t;
#define wtk_owl_tree_expr_is_s(owl,expr) wtk_owl_tree_expr_is(owl,expr,sizeof(expr)-1)
#define wtk_owl_tree_expr_value_s(owl,expr) wtk_owl_tree_expr_value(owl,expr,sizeof(expr)-1)
#define wtk_owl_tree_find_inst_s(owl,s) wtk_owl_tree_find_inst(owl,s,sizeof(s)-1,0)
#define wtk_owl_item_find_item2_s(owl,s) wtk_owl_item_find_item2(owl,s,sizeof(s)-1)
#define wtk_owl_tree_find_attr_s(owl,item,a,insert) wtk_owl_tree_find_attr(owl,item,a,sizeof(a)-1,insert)
#define wtk_owl_item_find_attr_s(item,a) wtk_owl_item_find_attr(item,a,sizeof(a)-1)
#define wtk_owl_item_find_attr_value_s(item,a,idx) wtk_owl_item_find_attr_value(item,a,sizeof(a)-1,idx)

typedef struct wtk_owl_item wtk_owl_item_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *v;
    wtk_if_t *xif;
} wtk_owl_item_attr_v_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue3_t v;	//wtk_owl_item_attr_v_t
    wtk_string_t *k;
} wtk_owl_item_attr_kv_t;

typedef struct {
    wtk_queue3_t q;	//wtk_owl_item_attr_kv_t
} wtk_owl_item_attr_t;

struct wtk_owl_item {
    union {
        wtk_string_t *str;
        wtk_owl_inst_t *inst;
    } k;
    wtk_owl_item_t *parent_item;
    wtk_owl_item_t *class_parent_item;
    wtk_owl_item_attr_t *attr;
    wtk_owl_item_t **items;
    wtk_string_t **v;
    wtk_owl_inst_t **v_inst;
    void *hook;
    int nitem;
    int nv;
    int n_vinst;
    unsigned use_k_inst :1;
};

typedef struct {
    wtk_owl_item_t **items;
    int nitem;
} wtk_owl_prop_t;

struct wtk_owl_class {
    wtk_owl_item_t *prop;
    wtk_owl_item_t *item;
    wtk_owl_class_t *parent;
};

struct wtk_owl_inst {
    wtk_owl_item_t *prop;
    wtk_owl_item_t *item;wtk_owl_class_t *class;
};

typedef struct {
    wtk_string_t **k;
    int nk;
    wtk_string_t *v;
} wtk_owl_expr_t;

typedef enum {
    WTK_OWL_CLASS,
    WTK_OWL_INST,
    WTK_OWL_PROP,
    WTK_OWL_ITEM,
    WTK_OWL_CLASS_ITEM,
    WTK_OWL_CLASS_PARENT,
} wtk_owl_item_type_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_owl_item_type_t type;
    //wtk_string_t *nm;
    wtk_larray_t *a;
    union {
        wtk_owl_item_t *item;
        wtk_owl_item_t *prop;wtk_owl_class_t *class;
        wtk_owl_inst_t *inst;
        wtk_owl_item_t class_item;
        wtk_owl_item_t parent_item;
    } v;
} wtk_owl_state_item_t;

struct wtk_owl_tree {
    wtk_heap_t *heap;
    wtk_str_hash_t *cls_map;
    wtk_str_hash_t *inst_map;
};

wtk_owl_tree_t* wtk_owl_tree_new();
void wtk_owl_tree_delete(wtk_owl_tree_t *owl);

wtk_owl_inst_t* wtk_owl_tree_new_inst(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes);
wtk_owl_item_t* wtk_owl_tree_new_item(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes);
wtk_owl_class_t* wtk_owl_tree_new_class(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes);
wtk_owl_class_t* wtk_owl_tree_find_class(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes, int insert);

wtk_owl_item_attr_kv_t* wtk_owl_tree_find_attr(wtk_owl_tree_t *owl,
        wtk_owl_item_t *item, char *a, int a_bytes, int insert);
wtk_owl_item_attr_kv_t* wtk_owl_tree_find_attr2(wtk_owl_tree_t *owl,
        wtk_owl_item_t *item, char *a, int a_bytes, int insert, int use_parent);

void wtk_owl_tree_add_attr(wtk_owl_tree_t *owl, wtk_owl_item_t *item, char *a,
        int a_bytes, char *v, int v_bytes);
wtk_string_t* wtk_owl_item_attr_kv_get_value(wtk_owl_item_attr_kv_t *kv,
        int idx);

void wtk_owl_tree_set_item_value(wtk_owl_tree_t *owl, wtk_owl_state_item_t *si,
        wtk_owl_item_t *item, char *v, int v_bytes);
void wtk_owl_tree_print(wtk_owl_tree_t *owl);

wtk_owl_inst_t* wtk_owl_tree_find_inst(wtk_owl_tree_t *tree, char *nm,
        int nm_bytes, int insert);
void wtk_owl_tree_update(wtk_owl_tree_t *owl);
void wtk_owl_item_print(wtk_owl_item_t *item);
int wtk_owl_inst_match(wtk_owl_inst_t *inst, char *nm, int nm_bytes);
int wtk_owl_item_match_value(wtk_owl_item_t *item, char *nm, int nm_bytes);

wtk_owl_item_t* wtk_owl_tree_find_item(wtk_owl_tree_t *owl, wtk_string_t **k,
        int nk);
wtk_owl_inst_t* wtk_owl_tree_match_inst(wtk_owl_tree_t *owl, char *nm,
        int nm_bytes);
wtk_owl_item_t* wtk_owl_class_find_prop(wtk_owl_class_t *cls, wtk_string_t *k);
wtk_owl_item_t* wtk_owl_inst_find_prop(wtk_owl_inst_t *inst, wtk_string_t *k);
wtk_owl_item_t* wtk_owl_item_find_item(wtk_owl_item_t *prop, wtk_string_t *k);

/**
 * 搜索item,包含父级class同级路径
 */
wtk_owl_item_t* wtk_owl_item_find_item2(wtk_owl_item_t *prop, char *s, int len);
wtk_string_t* wtk_owl_item_get_str_value(wtk_owl_item_t *item);

/**
 * input: "(爸爸,爸爸,性别)=男"
 * return: 1 or 0 (true or false)
 */
int wtk_owl_tree_expr_is(wtk_owl_tree_t *owl, char *expr, int expr_bytes);

/**
 * input: "(爸爸,爸爸,性别)=?"
 * return value;
 */
wtk_string_t* wtk_owl_tree_expr_value(wtk_owl_tree_t *owl, char *expr,
        int expr_bytes);

void wtk_owl_inst_print(wtk_owl_inst_t *inst);
void wtk_owl_tree_print_inst(wtk_owl_tree_t *owl);
void wtk_owl_class_print(wtk_owl_class_t *cls);
void wtk_owl_item_print_path(wtk_owl_item_t *item, wtk_strbuf_t *buf);
wtk_owl_item_attr_kv_t* wtk_owl_item_find_attr(wtk_owl_item_t *item, char *a,
        int a_bytes);
wtk_string_t* wtk_owl_item_find_attr_value(wtk_owl_item_t *item, char *a,
        int a_bytes, int idx);

/**
 *  find pvt.爸爸 item
 */
wtk_owl_item_t* wtk_owl_item_find_path(wtk_owl_item_t *citem, char *item,
        int item_len);
#ifdef __cplusplus
}
;
#endif
#endif
