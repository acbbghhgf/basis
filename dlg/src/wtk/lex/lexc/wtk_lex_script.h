#ifndef WTK_LEX_LEXC_WTK_LEX_SCRIPT
#define WTK_LEX_LEXC_WTK_LEX_SCRIPT
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue3.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk_lex_ner_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lex_script wtk_lex_script_t;
typedef struct wtk_lex_expr_item_value wtk_lex_expr_item_value_t;
typedef struct wtk_lex_expr_item wtk_lex_expr_item_t;
typedef struct wtk_lex_expr wtk_lex_expr_t;

typedef enum {
    WTK_LEX_NORMAL, WTK_LEX_LOWER, WTK_LEX_UPPER,
} wtk_lex_expr_attr_txt_t;

typedef struct {
    short max_count;		//-1, for can while,[0,+=>]
    unsigned char min_count;	//0
    unsigned greedy :1;				//is greedy or not;
} wtk_lex_expr_repeat_t;

typedef struct {
    wtk_string_t *name;
} wtk_lex_expr_item_key_t;

typedef struct {
    wtk_lex_expr_attr_txt_t txt;
    //wtk_lex_expr_repeat_t repeat;
    wtk_string_t *output;
    short min_wrd_count;
    short max_wrd_count;
    unsigned skipws :1;
    unsigned match_more_wrd :1;
    unsigned match_more_var :1;
    unsigned match_start :1;
    unsigned match_end :1;
    unsigned step_search :1;unsigned export:1;
    unsigned check_skip :1;
    unsigned use_seg :1;
} wtk_lex_expr_item_attr_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *like;
    float thresh;
} wtk_lex_like_item_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lexr_ner_item_t *ner;
    float wrd_pen;
    float prune_thresh;
    float conf_thresh;
    unsigned use_search :1;
} wtk_lex_ner_item_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *f;
    wtk_string_t *t;
} wtk_lex_replace_item_t;

typedef struct {
    wtk_lex_expr_attr_txt_t txt;
    wtk_lex_expr_repeat_t repeat;
    short min_wrd_count;
    short max_wrd_count;
    //float like_thresh;
    wtk_string_t *output;
    wtk_queue_t *like;	//wtk_lex_like_item_t
    wtk_queue_t *ner;	//wtk_lex_ner_item_t
    wtk_array_t *pos;	//pos 词性;
    wtk_array_t *not;
    wtk_array_t *pre;
    wtk_array_t *not_pre;
    wtk_array_t *suc;
    wtk_array_t *not_suc;
    wtk_string_t *skip;				//机场=(${.机场:[skip="国际"]}) => ("$1[kv=机场]");
    //wtk_string_t *like;
    wtk_queue3_t *replace;
    float match_weight;
    unsigned chn2num :1;			//航班=${.航班:[upper,skipws,chn2num]};
    unsigned skipws :1;
    unsigned use_py :1;
} wtk_lex_expr_item_value_attr_t;

typedef enum {
    WTK_LEX_VALUE_STRING, WTK_LEX_VALUE_BRACKET, //[]
    WTK_LEX_VALUE_PERIOD,	//.
    WTK_LEX_VALUE_PARENTHESES,	//()
    WTK_LEX_VALUE_RANGE,	//a-z \x00-\x00
    WTK_LEX_VALUE_ESCAPE,	//
    WTK_LEX_VALUE_EXPR,
    WTK_LEX_VALUE_BUILDIN_VAR,
} wtk_lex_expr_item_value_type_t;

typedef struct {
    wtk_queue_t or_q;		//wtk_lex_expr_item_value
    unsigned caret :1;		//^
    //unsigned clean:1;		//@ clean expr: ^@
} wtk_lex_expr_bracket_t;

typedef struct {
    unsigned int from;	//[from,to]
    unsigned int to;
} wtk_lex_expr_range_t;

/*
 #\d  匹配任何十进制数；它相当于类 [0-9]。
 #\D  匹配任何非数字字符；它相当于类 [^0-9]。
 #\s  匹配任何空白字符；它相当于类  [ \t\n\r\f\v]。
 #\S  匹配任何非空白字符；它相当于类 [^ \t\n\r\f\v]。
 #\w  匹配任何字母数字字符；它相当于类 [a-zA-Z0-9_]。
 #\W  匹配任何非字母数字字符；它相当于类 [^a-zA-Z0-9_]。
 #\h  匹配任何汉字;
 #\H  匹配任何非汉字;
 #\e  匹配任何英文[a-zA-Z]+;
 #\E  匹配任何非英文;
 */
typedef enum {
    WTK_RE_ESC_d,    //\d
    WTK_RE_ESC_D,	 //\D
    WTK_RE_ESC_s,	 //\s
    WTK_RE_ESC_S,	 //\S
    WTK_RE_ESC_w,	 //\w
    WTK_RE_ESC_W,	 //\W
    WTK_RE_ESC_h,	//\h
    WTK_RE_ESC_H,	//\H
    WTK_RE_ESC_e,	//\e
    WTK_RE_ESC_E,	//\E
} wtk_lex_expr_escape_t;

typedef struct {
    wtk_queue_t value_q;
    wtk_queue_t or_q;	//wtk_lex_expr_branch_t
    unsigned capture :1;
} wtk_lex_expr_parentheses_t;

typedef struct {
    wtk_string_t *name;
} wtk_lex_expr_var_t;

struct wtk_lex_expr_item_value {
    wtk_queue_node_t q_n;
    wtk_lex_expr_item_value_type_t type;
    wtk_lex_expr_item_value_attr_t attr;
    union {
        wtk_string_t *str;
        wtk_lex_expr_bracket_t *bracket;	//[]
        wtk_lex_expr_range_t *range;
        wtk_lex_expr_escape_t esc;
        wtk_lex_expr_parentheses_t *parentheses;
        wtk_lex_expr_item_t *expr;
        wtk_lex_expr_var_t *var;
        wtk_string_t *buildin_var;
    } v;
};

typedef enum {
    WTK_LEX_OUTPUT_ITEM_NONE = 0,
    WTK_LEX_OUTPUT_ITEM_STR,
    WTK_LEX_OUTPUT_ITEM_VAR,
    WTK_LEX_OUTPUT_ITEM_VARSTR,
} wtk_lex_expr_output_item_type_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lex_expr_output_item_type_t type;
    wtk_string_t *k;
    union {
        wtk_string_t *str;
        struct {
            wtk_string_t *def;
            int cap_index;
        } var;
        struct {
            wtk_string_t *pre;
            wtk_string_t *pst;
            int cap_index;
        } varstr;
    } v;
    float miss_pen;
    wtk_string_t *hook;
    wtk_string_t *post;
} wtk_lex_expr_output_item_t;

typedef enum {
    WTK_LEX_OUTPUT_TRANS_FILTER_NONE = 0,
    WTK_LEX_OUTPUT_TRANS_FILTER_NIL,
    WTK_LEX_OUTPUT_TRANS_FILTER_SKIP,
    WTK_LEX_OUTPUT_TRANS_FILTER_TONUMBER,
    WTK_LEX_OUTPUT_TRANS_FILTER_NUM2CHN,
    WTK_LEX_OUTPUT_TRANS_FILTER_NUM2TEL,
    WTK_LEX_OUTPUT_TRANS_FILTER_UPPER,
    WTK_LEX_OUTPUT_TRANS_FILTER_LOWER,
    WTK_LEX_OUTPUT_TRANS_FILTER_STRING,
} wtk_lex_expr_output_trans_filter_type_t;

typedef enum {
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_LT,
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_GT,
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_EQ,
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_LE,
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_GE,
} wtk_lex_expr_output_trans_if_type_t;

typedef enum {
    WTK_LEX_EXPR_OUTPUT_TRANS_IF_NUMBER, WTK_LEX_EXPR_OUTPUT_TRANS_IF_STR,
} wtk_lex_expr_output_trans_if_value_type_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lex_expr_output_trans_if_type_t if_type;
    wtk_lex_expr_output_trans_if_value_type_t value_type;
    union {
        wtk_string_t *str;
        float f;
    } v;
} wtk_lex_expr_output_trans_if_t;

typedef struct wtk_lex_expr_output_trans_filter wtk_lex_expr_output_trans_filter_t;

typedef struct {
    wtk_lex_expr_output_trans_filter_type_t type;
    wtk_string_t *str;
    wtk_string_t *pre;
    wtk_string_t *suc;
} wtk_lex_expr_output_trans_filter_func_t;

typedef struct {
    union {
        wtk_lex_expr_output_trans_filter_func_t func;
        wtk_lex_expr_output_trans_filter_t *filter;
    } v;
    unsigned leaf :1;
} wtk_lex_expr_output_trans_filter_item_t;

struct wtk_lex_expr_output_trans_filter {
    wtk_queue_node_t q_n;
    wtk_queue_t if_q; //condition queue
    //wtk_lex_expr_output_trans_filter_t *parent;
    wtk_lex_expr_output_trans_filter_item_t yes;
    wtk_lex_expr_output_trans_filter_item_t no;unsigned or:1;
};

typedef enum {
    WTK_LEX_OUTPUT_TRANS_NONE = 10,
    WTK_LEX_OUTPUT_TRANS_STR,
    WTK_LEX_OUTPUT_TRANS_VAR,
} wtk_lex_expr_output_trans_item_type_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_lex_expr_output_trans_item_type_t type;
    wtk_queue_t filter_q;
    union {
        wtk_string_t *str;
        int cap_index;
    } v;
} wtk_lex_expr_output_trans_item_t;

typedef struct {
    wtk_queue_t item_q;	//wtk_lex_expr_output_item_t if use act; if not output is wtk_lex_expr_output_trans_item_t
    wtk_string_t *name;
} wtk_lex_expr_output_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue_t value_q;	///wtk_lex_expr_item_value
} wtk_lex_expr_branch_t;

struct wtk_lex_expr_item {
    wtk_lex_expr_item_key_t key;
    wtk_lex_expr_item_attr_t attr;
    wtk_queue_t or_q;//if or_q length>1 then esist branch;  wtk_lex_expr_branch_t
    wtk_queue_t value_q;		//wtk_lex_expr_item_value_t  我在 ${.POI}
    wtk_lex_expr_output_t output;
    float prob;
};

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *var;
    wtk_string_t *act;
    wtk_string_t *value;unsigned not:1;
} wtk_lex_expr_if_t;

typedef struct {
    wtk_queue_t condition;	//wtk_lex_expr_if_t
    wtk_queue_t yes;	//wtk_lex_expr_t
    wtk_queue_t no;
    unsigned and:1;
}wtk_lex_expr_scope_t;

typedef enum
{
    WTK_LEX_EXPR_ITEM,
    WTK_LEX_EXPR_SCOPE,
    WTK_LEX_EXPR_CMD,
}wtk_lex_expr_type_t;

typedef enum
{
    WTK_LEX_EXPR_CMD_DEL,
    WTK_LEX_EXPR_CMD_ADD,
    WTK_LEX_EXPR_CMD_MV,
    WTK_LEX_EXPR_CMD_CPY,
    WTK_LEX_EXPR_CMD_DEBUG,
    WTK_LEX_EXPR_CMD_RETURN,
}wtk_lex_expr_cmd_type_t;

typedef struct
{
    wtk_lex_expr_cmd_type_t type;
    wtk_string_t *op1_act;
    wtk_string_t *op1_var;
    wtk_string_t *op2_act;
    wtk_string_t *op2_var;
}wtk_lex_expr_cmd_t;

struct wtk_lex_expr
{
    wtk_queue_node_t q_n;
    wtk_lex_expr_type_t type;
    union {
        wtk_lex_expr_item_t *expr;
        wtk_lex_expr_scope_t *scope;
        wtk_lex_expr_cmd_t *cmd;
    }v;
};

typedef struct wtk_lex_script_lib wtk_lex_script_lib_t;

struct wtk_lex_script_lib
{
    wtk_queue_node_t q_n;
    wtk_string_t *name;
    wtk_queue_t expr_q;		//wtk_lex_expr_t export queue;
    wtk_queue_t local_q;//wtk_lex_expr_t local queue;
    //wtk_queue_t fuzzy_q;	//wtk_lex_expr_t fuzzy queue;
    //unsigned lex_export:1;
};

typedef wtk_lex_expr_t* (*wtk_lexc_get_expr_f)(void *ths,wtk_string_t *libname,char *name,int bytes);

struct wtk_lex_script
{
    wtk_queue_t lib_q;			//wtk_lex_script_lib_t queue;
    wtk_lex_script_lib_t *cache_search;
    wtk_lex_script_lib_t *main_lib;
    wtk_heap_t *heap;
    wtk_lexc_get_expr_f get_expr;
    void *get_expr_ths;
    unsigned hide:1;//if hide,lex_net delete,then delete script
    unsigned sort_by_prob:1;
    unsigned use_fast_match:1;
    unsigned use_eng_word:1;
    unsigned use_act:1;
    unsigned use_nbest:1;
};

wtk_lex_script_t* wtk_lex_script_new();
void wtk_lex_script_delete(wtk_lex_script_t *s);

void wtk_lex_script_add_lib(wtk_lex_script_t *s,char *data,int bytes);
wtk_lex_expr_if_t* wtk_lex_script_new_if(wtk_lex_script_t *s,char *data,int bytes,int n);

wtk_lex_expr_t* wtk_lex_script_new_cmd2(wtk_lex_script_t *s);
wtk_lex_expr_t* wtk_lex_script_new_cmd(wtk_lex_script_t *s,wtk_string_t *libname);
void wtk_lex_script_cmd_set_del(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_add(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_value(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_cpy(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_cpy_value(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_mv(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
void wtk_lex_script_cmd_set_mv_value(wtk_lex_script_t *s,wtk_lex_expr_cmd_t *cmd,char *data,int bytes);
wtk_lex_expr_t* wtk_lex_script_new_scope(wtk_lex_script_t *s,wtk_string_t *libname);
wtk_lex_expr_t* wtk_lex_script_new_scope3(wtk_lex_script_t *s);
wtk_lex_expr_t* wtk_lex_script_new_expr2(wtk_lex_script_t *s,char *name,int bytes);
wtk_lex_expr_t* wtk_lex_script_new_expr(wtk_lex_script_t *s,wtk_string_t *libname,char *name,int bytes,int expt);
wtk_lex_expr_item_value_t* wtk_lex_script_expr_add_str(wtk_lex_script_t *s,wtk_lex_expr_t *expr,char *data,int bytes);
wtk_lex_expr_t* wtk_lex_script_get_expr(wtk_lex_script_t *s,wtk_string_t *libname,char *name,int bytes);
wtk_lex_expr_item_value_t* wtk_lex_script_new_value_expr(wtk_lex_script_t *s,wtk_string_t *lib,char *name,int bytes);

wtk_lex_expr_item_value_t* wtk_lex_script_new_value_dot(wtk_lex_script_t *s);
wtk_lex_expr_item_value_t* wtk_lex_script_new_value_str(wtk_lex_script_t *s,char *data,int bytes);
wtk_lex_expr_item_value_t* wtk_lex_script_new_parentheses(wtk_lex_script_t *s);
wtk_lex_expr_branch_t* wtk_lex_expr_script_new_branch(wtk_lex_script_t *s);
wtk_lex_expr_item_value_t* wtk_lex_script_new_bracket(wtk_lex_script_t *s);
wtk_lex_expr_item_value_t* wtk_lex_script_new_esc(wtk_lex_script_t *s,wtk_lex_expr_escape_t esc);
wtk_lex_expr_output_item_t* wtk_lex_script_new_output_item(wtk_lex_script_t *s,char *data,int len);
wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_item(wtk_lex_script_t *s);
wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_str_item(wtk_lex_script_t *s,char *data,int len);
wtk_lex_expr_output_trans_item_t* wtk_lex_script_new_output_trans_cap_item(wtk_lex_script_t *s,int cap);
wtk_lex_expr_output_trans_filter_t* wtk_lex_script_new_trans_filter(wtk_lex_script_t *s);
wtk_lex_expr_output_trans_filter_type_t wtk_lex_expr_output_trans_filter_type_get(char *data,int len);
wtk_lex_expr_output_trans_if_t* wtk_lex_script_new_output_trans_if(wtk_lex_script_t *s);
int wtk_lex_expr_output_trans_filter_process(wtk_lex_expr_output_trans_filter_t *filter,char *data,int len,wtk_strbuf_t *buf);
void wtk_lex_expr_output_trans_filter_item_set_func(wtk_lex_script_t *script,wtk_lex_expr_output_trans_filter_item_t *item,char *data,int len);
void wtk_lex_expr_output_trans_filter_item_set_filter(wtk_lex_expr_output_trans_filter_item_t *item,wtk_lex_expr_output_trans_filter_t *filter);
void wtk_lex_expr_output_trans_if_set_value(wtk_lex_script_t *l,wtk_lex_expr_output_trans_if_t *xif,char *data,int len);

int wtk_lex_expr_item_value_is_atom(wtk_lex_expr_item_value_t *v);
int wtk_lex_expr_item_value_can_be_nil(wtk_lex_expr_item_value_t *v);

void wtk_lex_script_close_parentheses(wtk_lex_script_t *s,wtk_lex_expr_parentheses_t *item);
void wtk_lex_script_close_expr(wtk_lex_script_t *s,wtk_lex_expr_item_t *item);
void wtk_lex_expr_item_print(wtk_lex_expr_item_t *item);
void wtk_lex_expr_item_value_print(wtk_lex_expr_item_value_t *v);
void wtk_lex_script_print_expr(wtk_lex_expr_t *expr);
void wtk_lex_script_print(wtk_lex_script_t *s);

int wtk_lex_expr_item_value_match(wtk_lex_expr_item_value_t *item,wtk_string_t *tok);
int wtk_lex_expr_output_has_redirect(wtk_lex_expr_output_t *out);
int wtk_lex_expr_output_is_alias(wtk_lex_expr_output_t *out);
wtk_string_t* wtk_lex_expr_output_get_alias(wtk_lex_expr_output_t *out);
void wtk_lex_script_update(wtk_lex_script_t *script);

void wtk_lex_expr_item_value_attr_add_like(wtk_lex_expr_item_value_attr_t *attr,wtk_heap_t *heap,char *data,int bytes);
void wtk_lex_expr_item_value_attr_add_like_thresh(wtk_lex_expr_item_value_attr_t *attr,float f);

void wtk_lex_expr_item_value_attr_add_ner(wtk_lex_expr_item_value_attr_t *attr,wtk_heap_t *heap,wtk_lexr_ner_item_t *ner);
void wtk_lex_expr_item_value_attr_add_ner_wrd_pen(wtk_lex_expr_item_value_attr_t *attr,float f);
void wtk_lex_expr_item_value_attr_add_ner_prune_thresh(wtk_lex_expr_item_value_attr_t *attr,float f);
void wtk_lex_expr_item_value_attr_add_ner_conf_thresh(wtk_lex_expr_item_value_attr_t *attr,float f);
void wtk_lex_expr_item_value_attr_add_ner_use_search(wtk_lex_expr_item_value_attr_t *attr,int u);

void wtk_lex_script_expr_add_not_pre(wtk_lex_script_t *script,wtk_lex_expr_item_value_t *value,wtk_array_t *a);
void wtk_lex_script_expr_add_pre(wtk_lex_script_t *script,wtk_lex_expr_item_value_t *value,wtk_array_t *a);
void wtk_lex_script_expr_add_not_suc(wtk_lex_script_t *script,wtk_lex_expr_item_value_t *value,wtk_array_t *a);
void wtk_lex_script_expr_add_suc(wtk_lex_script_t *script,wtk_lex_expr_item_value_t *value,wtk_array_t *a);
void wtk_lex_script_set_output_item_value(wtk_lex_script_t *s,wtk_lex_expr_output_item_t *output,char *k,int k_len);
void wtk_lex_expr_item_value_update_py(wtk_lex_expr_item_value_t *v,int py);
#ifdef __cplusplus
};
#endif
#endif
