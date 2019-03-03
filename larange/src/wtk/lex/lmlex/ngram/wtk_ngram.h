#ifndef WTK_LMLEX_NGRAM_WTK_NGRAM_H_
#define WTK_LMLEX_NGRAM_WTK_NGRAM_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/cfg/wtk_source.h"
#include "wtk/core/wtk_stridx.h"
#include "wtk/core/wtk_queue2.h"
#include "wtk_ngram_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ngram wtk_ngram_t;
typedef struct wtk_ngram_node wtk_ngram_node_t;
typedef struct wtk_ngram_root wtk_ngram_root_t;

#define wtk_ngram_float_t float

typedef enum {
    WTK_NGRAM_NODE_INIT, WTK_NGRAM_NODE_SORT,
} wtk_ngram_node_state_t;

typedef enum {
    WTK_NGRAM_NODE_STR, WTK_NGRAM_NODE_LEXVAR,
} wtk_ngram_node_type_t;

struct wtk_ngram_node {
    wtk_queue_node_t lex_var_n;
    wtk_queue2_t lex_var_q;
    wtk_ngram_node_t *parent;
    wtk_ngram_node_t *next; //next in child;
    union {
        wtk_ngram_node_t *list; //used for load file;
        wtk_ngram_node_t **array; //used for sort;
    } child;
    int n;
    int wrd_idx;
    int ngram;wtk_ngram_float_t prob;wtk_ngram_float_t bow;
    wtk_ngram_node_state_t state;
    wtk_ngram_node_type_t type;
};

struct wtk_ngram_root {
    wtk_queue2_t lex_var_q;
    wtk_ngram_node_t *nodes;
    int n;
};

struct wtk_ngram {
    wtk_ngram_cfg_t *cfg;
    wtk_heap_t *heap;
    wtk_stridx_t *idxs;
    wtk_ngram_root_t *root;
    wtk_ngram_node_t *start;	//<s>
    int order;
    int idx_unk;
    int idx_snts;
    int idx_snte;
    int idx_pau;
};

wtk_ngram_t* wtk_ngram_new(wtk_ngram_cfg_t *cfg);
void wtk_ngram_delete(wtk_ngram_t *n);
//int wtk_ngram_update_wrd_cnt(wtk_ngram_t *n,char *fn);
int wtk_ngram_load(wtk_ngram_t *n, wtk_source_t *src);
int wtk_ngram_load_file(wtk_ngram_t *n, char *fn);
int wtk_ngram_get_idx(wtk_ngram_t *n, char *data, int bytes);
wtk_ngram_node_t* wtk_ngram_root_get_node(wtk_ngram_root_t* r, int idx);
wtk_ngram_node_t* wtk_ngram_prob(wtk_ngram_t *n, int *idx, int len, int *cnt);
wtk_ngram_node_t* wtk_ngram_node_get_child_array(wtk_ngram_node_t *node,
        int id);
int wtk_ngram_node_path_id(wtk_ngram_node_t *node, int *ids);
int wtk_ngram_node_depth(wtk_ngram_node_t *n);
double wtk_ngram_uni_bow(wtk_ngram_t *ngram, wtk_ngram_node_t *node);

typedef struct {
    wtk_ngram_node_t *root;
    int depth;
} wtk_ngram_pnode_t;

wtk_ngram_pnode_t wtk_ngram_node_parent(wtk_ngram_node_t *n);
wtk_ngram_node_t* wtk_ngram_node_parent2(wtk_ngram_node_t *n);

typedef struct {
    wtk_ngram_node_t *node;
    double bow;
} wtk_ngram_prob_t;

wtk_ngram_prob_t wtk_ngram_prob2(wtk_ngram_t *n, int *pi, int len);
wtk_ngram_prob_t wtk_ngram_next_node(wtk_ngram_t *ngram, wtk_ngram_node_t *node,
        int idx);
void wtk_ngram_print_node(wtk_ngram_t *n, wtk_ngram_node_t *node);
void wtk_ngram_print_parent(wtk_ngram_t *n, wtk_ngram_node_t *node);
void wtk_ngram_print(wtk_ngram_t *n);
void wtk_ngram_print_id(wtk_ngram_t *n, int id);
#ifdef __cplusplus
}
;
#endif
#endif
