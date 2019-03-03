#ifndef WTK_LMLEX_LMREC_WTK_LMREC_H_
#define WTK_LMLEX_LMREC_WTK_LMREC_H_
#include "wtk/core/wtk_type.h"
#include "wtk/lex/lmlex/ngram/wtk_ngram.h"
#include "wtk/lex/pool/wtk_lexpool.h"
#include "wtk/core/wtk_vpool.h"
#include "wtk/lex/lmlex/lmres/wtk_lmres.h"
#include "wtk_lmrec_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lmrec wtk_lmrec_t;
typedef struct wtk_lmrec_align wtk_lmrec_align_t;
#define wtk_lmrec_process_s(r,s) wtk_lmrec_process(r,s,sizeof(s)-1)

#ifndef LZERO
#define LZERO -10000000000
#endif

struct wtk_lmrec_align {
    wtk_lmrec_align_t *prev;
    //wtk_queue_node_t q_n;
    wtk_string_t *str;
    wtk_ngram_node_t *node;
    float prob;
};

typedef struct {
    wtk_queue_node_t q_n;
    wtk_ngram_node_t *node;	//already matched node;
    //wtk_ngram_node_t *root;
    //wtk_queue2_t align_q;
    wtk_lmrec_align_t *align;
    //wtk_ngram_node_t *lex_after_node;
    wtk_lexpool_item_t *item;
    wtk_lexpool_rec_t *rec;
    float prob;	//lm_prob+oov
    float pen;
} wtk_lmrec_tok_t;

struct wtk_lmrec {
    wtk_lmrec_cfg_t *cfg;
    wtk_lmres_t *res;
    //---------- decoding section -----------
    wtk_heap_t *heap;
    double prob;
    double ppl;
    double best_like;
    wtk_lmrec_tok_t *max_tok;
    int nwrd;
    wtk_lmrec_tok_t *best_tok;
    wtk_vpool_t *tokpool;
    wtk_queue_t tok_q;
    wtk_queue_t lex_tok_q;

    wtk_json_t *json;
    wtk_json_item_t *action;
    wtk_json_item_t *request;
    wtk_strbuf_t *buf;
};

wtk_lmrec_t* wtk_lmrec_new(wtk_lmrec_cfg_t *cfg);
void wtk_lmrec_delete(wtk_lmrec_t *r);
void wtk_lmrec_start(wtk_lmrec_t *r, wtk_lmres_t *res);
void wtk_lmrec_reset(wtk_lmrec_t *r);
double wtk_lmrec_process(wtk_lmrec_t *r, char *data, int bytes);
//------------------- decoding -------------------------------
wtk_lexpool_item_t* wtk_lmrec_get_lex_item(wtk_lmrec_t *r, unsigned int idx);
wtk_string_t wtk_lmrec_process2(wtk_lmrec_t *rec, wtk_lmres_t *res,
        wtk_heap_t *heap, char *data, int bytes);
wtk_lmrec_tok_t* wtk_lmrec_get_best_tok(wtk_lmrec_t *r);
int wtk_lmrec_tostring(wtk_lmrec_t *r, wtk_strbuf_t *buf);
void wtk_lmrec_print(wtk_lmrec_t *r);
void wtk_lmrec_print_align(wtk_lmrec_t *r, wtk_lmrec_align_t *a);
void wtk_lmrec_print_all(wtk_lmrec_t *r);
#ifdef __cplusplus
}
;
#endif
#endif
