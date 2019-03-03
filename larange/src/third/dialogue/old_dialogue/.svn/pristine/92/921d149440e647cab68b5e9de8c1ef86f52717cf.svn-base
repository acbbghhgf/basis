#ifndef WTK_SEMDLG_WRDVEC_WTK_VECFAQ
#define WTK_SEMDLG_WRDVEC_WTK_VECFAQ
#include "wtk/core/wtk_type.h" 
#include "wtk_vecfaq_cfg.h"
#include "wtk_faqbin.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vecfaq wtk_vecfaq_t;

struct wtk_vecfaq {
    wtk_vecfaq_cfg_t *cfg;
    wtk_faqbin_t **map_bins;
    wtk_faqbin_t **bins;
    wtk_wrdvec_t *wrdvec;
    wtk_vecf_t *last_vec;
    float prob;
    int index;
};

wtk_vecfaq_t* wtk_vecfaq_new(wtk_vecfaq_cfg_t *cfg, wtk_rbin2_t *rbin);
void wtk_vecfaq_delete(wtk_vecfaq_t *faq);
void wtk_vecfaq_reset(wtk_vecfaq_t *faq);
wtk_string_t wtk_vecfaq_get3(wtk_vecfaq_t *faq, char *s, int s_len, char *input,
        int input_len);
wtk_string_t wtk_vecfaq_get(wtk_vecfaq_t *faq, char *data, int bytes);
void wtk_vecfaq_set_wrdvec(wtk_vecfaq_t *v, wtk_wrdvec_t *wvec);
wtk_robin_t* wtk_vecfaq_get2(wtk_vecfaq_t *faq, char *data, int bytes);
#ifdef __cplusplus
}
;
#endif
#endif
