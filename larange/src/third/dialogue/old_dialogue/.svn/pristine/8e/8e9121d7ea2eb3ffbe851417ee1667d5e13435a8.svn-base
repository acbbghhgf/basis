#ifndef WTK_LEX_NLG_WTK_NLG
#define WTK_LEX_NLG_WTK_NLG
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_str_parser.h"
#include "wtk/core/wtk_str_hash.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#include "wtk/core/wtk_str_parser.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlg wtk_nlg_t;
#define wtk_nlg_get_root_s(nlg,act) wtk_nlg_get_root(nlg,act,sizeof(act)-1,0)

typedef enum {
    WTK_NLG_INIT, WTK_NLG_COMM, WTK_NLG_KEY, WTK_NLG_VALUE,
} wtk_nlg_state_t;

typedef struct {
    wtk_queue_node_t q_n;
    union {
        wtk_string_t *str;
        wtk_string_t *var;
    } v;
    unsigned is_str :1;
} wtk_nlg_value_str_slot_t;

typedef struct {
    wtk_queue_t str_q; //wtk_nlg_value_str_slot_t
} wtk_nlg_value_str_t;

typedef struct {
    union {
        wtk_nlg_value_str_t *str;
        wtk_string_t *var;
    } v;
    unsigned is_str :1;
} wtk_nlg_value_t;

typedef struct {
    wtk_string_t *k;
    wtk_string_t *v;
} wtk_nlg_key_t;

typedef struct wtk_nlg_item wtk_nlg_item_t;

struct wtk_nlg_item {
    wtk_string_t *action;
    wtk_string_t *comment;
    union {
        wtk_string_t *str;		//str is used for parser
        wtk_nlg_item_t *item;
    } nxt;
    wtk_array_t *attr;	//wtk_nlg_key_t*
    wtk_array_t *value;	//wtk_nlg_value_t
    unsigned match_all_sot :1;
};

typedef struct {
    wtk_queue_node_t q_n;
    wtk_nlg_key_t *key;
    wtk_nlg_item_t *item;
    wtk_queue_t attr_q;	//wtk_nlg_node_t
} wtk_nlg_node_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *action;
    wtk_nlg_item_t *item;
    wtk_queue_t attr_q;	//wtk_nlg_node_t
} wtk_nlg_root_t;

typedef wtk_string_t* (*wtk_nlg_act_get_value_f)(void *ths, char *k, int k_len);
typedef wtk_string_t (*wtk_nlg_act_get_lua_f)(void *ths, char *func);

struct wtk_nlg {
    wtk_heap_t *heap;
    wtk_rbin2_t *rbin;
    wtk_nlg_state_t state;
    int sub_state;
    wtk_strbuf_t *buf;
    wtk_nlg_item_t *item;
    wtk_queue_t tree_q;
    wtk_string_parser_t parser;
    wtk_var_parse_t var_parser;
    wtk_nlg_key_t *key;
    wtk_nlg_value_t *value;
    int cnt;
    wtk_string_t *pwd;
    wtk_strbuf_t *tmp;
};

wtk_nlg_t* wtk_nlg_new(char *fn);
wtk_nlg_t* wtk_nlg_new2(wtk_rbin2_t *rbin, char *fn);
void wtk_nlg_delete(wtk_nlg_t *nlg);
void wtk_nlg_print(wtk_nlg_t *nlg);
void wtk_nlg_node_print(wtk_nlg_node_t *node);
void wtk_nlg_root_print(wtk_nlg_root_t *node);
void wtk_nlg_item_print(wtk_nlg_item_t *item);
void wtk_nlg_value_print(wtk_nlg_value_t *v);
wtk_nlg_value_t* wtk_nlg_get_item_value(wtk_nlg_t *nlg, wtk_nlg_item_t *item);

wtk_nlg_root_t* wtk_nlg_get_root(wtk_nlg_t *nlg, char* action, int action_bytes,
        int insert);
wtk_nlg_node_t* wtk_nlg_root_get_node(wtk_nlg_t *nlg, wtk_nlg_root_t *root,
        wtk_string_t *k, wtk_string_t *v, int insert);
wtk_nlg_node_t* wtk_nlg_node_get_child(wtk_nlg_t *nlg, wtk_nlg_node_t *root,
        wtk_string_t *k, wtk_string_t *v, int insert);
wtk_nlg_item_t* wtk_nlg_get(wtk_nlg_t *nlg, wtk_string_t *action, ...);
wtk_nlg_item_t* wtk_nlg_get2(wtk_nlg_t *nlg, char *cmd, int cmd_bytes,
        wtk_str_hash_t *hash);

//sys_answer_tongbei(rel,test)
wtk_nlg_item_t* wtk_nlg_get3(wtk_nlg_t *nlg, char *cmd, int cmd_bytes);

int wtk_nlg_process(wtk_nlg_t *nlg, wtk_nlg_item_t *item, wtk_strbuf_t *buf,
        void *ths, wtk_nlg_act_get_value_f get_value,
        wtk_nlg_act_get_lua_f get_lua);

wtk_nlg_item_t* wtk_nlg_root_get_default_item(wtk_nlg_root_t *root);

wtk_string_t wtk_nlg_process_nlg_str(wtk_nlg_t *nlg, char *data, int bytes);
wtk_string_t wtk_nlg_process_nlg_str2(wtk_nlg_t *nlg, char *data, int bytes,
        void *get_lua_ths, wtk_nlg_act_get_lua_f get_lua);
#ifdef __cplusplus
}
;
#endif
#endif
