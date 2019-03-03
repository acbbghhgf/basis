#ifndef WTK_SEMDLG_WRDVEC_WTK_FAQBIN
#define WTK_SEMDLG_WRDVEC_WTK_FAQBIN
#include "wtk/core/wtk_type.h" 
#include "wtk/core/math/wtk_mat.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_robin.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_faqbin wtk_faqbin_t;

typedef struct wtk_faqbin_item wtk_faqbin_item_t;

struct wtk_faqbin_item {
    wtk_faqbin_item_t *left;
    wtk_faqbin_item_t *right;
    wtk_vecf_t *v;
    unsigned int offset;
};

struct wtk_faqbin {
    wtk_rbin2_t *rbin;
    FILE *f;
    wtk_faqbin_item_t *root;
    int vec_size;
    wtk_vecf_t *v;
    wtk_heap_t *heap;
    wtk_strbuf_t *buf;
    wtk_robin_t *rb;
    float prob;
    float thresh;
    float best_thresh;
    int f_of;
    int f_len;
    unsigned int offset;
};

wtk_faqbin_t* wtk_faqbin_new(char *fn, wtk_rbin2_t *rbin);
void wtk_faqbin_delete(wtk_faqbin_t *faq);
void wtk_faqbin_reset(wtk_faqbin_t *faq);
wtk_string_t wtk_faqbin_get(wtk_faqbin_t *faq, wtk_vecf_t *v);
wtk_robin_t* wtk_faqbin_get2(wtk_faqbin_t *faq, wtk_vecf_t *v);
#ifdef __cplusplus
}
;
#endif
#endif
