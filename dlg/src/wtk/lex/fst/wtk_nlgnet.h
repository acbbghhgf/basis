#ifndef WTK_SEMDLG_FST_WTK_NLGNET
#define WTK_SEMDLG_FST_WTK_NLGNET
#include "wtk/core/wtk_type.h" 
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_queue2.h"
#include "wtk/lex/nlg/wtk_nlg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nlgnet wtk_nlgnet_t;
typedef struct wtk_nlgnet_arc wtk_nlgnet_arc_t;
typedef struct wtk_nlgnet_state wtk_nlgnet_state_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_string_t *k;
    wtk_string_t *v;
} wtk_nlgnet_arc_attr_t;

struct wtk_nlgnet_state {
    wtk_queue_node_t q_n;
    wtk_string_t *name;
    wtk_string_t *emit;
    char *pre;
    char *post;
    wtk_nlgnet_state_t *eps;
    wtk_nlg_item_t *emit_item;
    wtk_queue_t output_arc_q;	//wtk_nlgnet_arc_t
    wtk_nlgnet_arc_t *other;
};

typedef struct {
    wtk_queue_node_t q_n;
    wtk_nlgnet_state_t *state;
} wtk_nglnet_arc_to_item_t;

struct wtk_nlgnet_arc {
    wtk_queue_node_t arc_n;
    wtk_queue_t attr_q;	//wtk_nlgfst_arc_attr_t
    wtk_queue_t to_q;	//wtk_nglfst_arc_to_item_t
    wtk_nlgnet_state_t *from;
    wtk_string_t *emit_func;
    wtk_nlg_item_t *emit_item;
    wtk_nlgnet_state_t *other;
    short max_round;
    short min_round;
    float min_conf;
    unsigned match_all_slot :1;
    unsigned use_emit :1;		//当前arc匹配，是否激活到达状态的emit输出
    unsigned or:1;				//当前arc条件 attr_q 的匹配关系
    unsigned clean_ctx :1;		//是否reset 当前领域内的semslot
    unsigned skip_fld :1;		//是否跳出当前fld,重新做domain分析;
    unsigned domained :1;		//如果当前domained==0,当前domained为1返回;
    unsigned must_domain :1;		//如果must_domain==1, 如果当前没有domain，返回;
    unsigned playing :1;			//当前是否在音乐模式，除非用户说停止或者音乐播放完毕;
    unsigned fail_re_doamin :1;  //如果不匹配，重新domain,调用semfld;
};

struct wtk_nlgnet {
    wtk_queue_t state_q;
    wtk_heap_t *heap;
    wtk_nlgnet_state_t *root;
    wtk_nlgnet_state_t *end;
};

wtk_nlgnet_t* wtk_nlgnet_new(wtk_rbin2_t *rbin, char *fn);
wtk_nlgnet_t* wtk_nlgnet_new2(wtk_heap_t *heap, char *data, int len);
void wtk_nlgnet_delete(wtk_nlgnet_t *net);
void wtk_nlgnet_print(wtk_nlgnet_t *net);
int wtk_nlgnet_bind(wtk_nlgnet_t *net, wtk_nlg_t *nlg);
wtk_nlgnet_arc_attr_t* wtk_nlgnet_new_arc_attr(wtk_heap_t *heap,
        wtk_string_t *k, wtk_string_t *v);
wtk_nlgnet_state_t* wtk_nlgnet_arc_next(wtk_nlgnet_arc_t *arc);
void wtk_nlgnet_arc_print(wtk_nlgnet_arc_t *arc);
void wtk_nlgnet_state_print(wtk_nlgnet_state_t *state);
wtk_nlgnet_state_t* wtk_nlgnet_get_state(wtk_nlgnet_t *fst, char *nm,
        int nm_bytes, int insert);
#ifdef __cplusplus
}
;
#endif
#endif
