#ifndef WTK_LEX_NER_WTK_HMMNR
#define WTK_LEX_NER_WTK_HMMNR
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_fkv.h"
#include "wtk_hmmnr_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_hmmnr wtk_hmmnr_t;

typedef struct {
    wtk_queue_node_t q_n;
    float prob;
    int wrd_pos;
    int end_pos;
    int index;
} wtk_hmmnr_inst_t;

struct wtk_hmmnr {
    wtk_fkv_t *fkv;
    wtk_hmmne_t *ne;
    wtk_heap_t *heap;
    wtk_queue_t inst_q;
    wtk_queue_t output_q;
    wtk_hmmnr_inst_t *best_inst;
    int pos;
    int end_pos;
    wtk_string_t input;
    float prob;
    float wrd_pen;
    float prune_thresh;
};

wtk_hmmnr_t* wtk_hmmnr_new();
void wtk_hmmnr_delete(wtk_hmmnr_t *n);
void wtk_hmmnr_reset(wtk_hmmnr_t *n);
void wtk_hmmnr_set_fkv(wtk_hmmnr_t *n, char *fn);
float wtk_hmmnr_process(wtk_hmmnr_t *n, char *data, int bytes);
float wtk_hmmnr_process2(wtk_hmmnr_t *h, char *data, int bytes, float word_pen);
wtk_string_t wtk_hmmnr_rec(wtk_hmmnr_t *h, char *data, int bytes);
wtk_string_t wtk_hmmnr_rec2(wtk_hmmnr_t *h, char *data, int bytes,
        float wrd_pen, float prune_thresh);
#ifdef __cplusplus
}
;
#endif
#endif
