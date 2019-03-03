#ifndef WTK_SEMDLG_WRDVEC_WTK_FAQ2BIN
#define WTK_SEMDLG_WRDVEC_WTK_FAQ2BIN
#include "wtk/core/wtk_type.h" 
#include "wtk_faq2bin_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faq2bin wtk_faq2bin_t;

typedef struct {
    wtk_queue_node_t q_n;
    wtk_queue_node_t cls_n;
    wtk_vecf_t *v;
    wtk_string_t *q;
    wtk_array_t *a;	//wtk_string_t*
} wtk_qa_item_t;

typedef struct wtk_faq_cls_item wtk_faq_cls_item_t;

struct wtk_faq_cls_item {
    wtk_queue_node_t q_n;
    wtk_queue_t item_q;
    wtk_faq_cls_item_t *parent;
    wtk_faq_cls_item_t *left;
    wtk_faq_cls_item_t *right;
    wtk_vecf_t *v;
    wtk_vecf_t *v2;
    int n;
    int index;
    float sse;
};

struct wtk_faq2bin {
    wtk_faq2bin_cfg_t *cfg;
    wtk_wrdvec_t *wrdvec;
    wtk_heap_t *heap;
    wtk_queue_t item_q;	//wtk_qa_item_t
    wtk_queue_t cls_q;
    wtk_faq_cls_item_t *root;
};

wtk_faq2bin_t* wtk_faq2bin_new(wtk_faq2bin_cfg_t *cfg);
void wtk_faq2bin_delete(wtk_faq2bin_t *faq);
int wtk_faq2bin_process(wtk_faq2bin_t *faq, char *fn);
void wtk_faq2bin_write(wtk_faq2bin_t *faq, char *fn);
void wtk_faq2bin_write2(wtk_faq2bin_t *faq, char *fn);
#ifdef __cplusplus
}
;
#endif
#endif
