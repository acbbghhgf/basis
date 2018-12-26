#ifndef WTK_RNN_WTK_WRDVEC
#define WTK_RNN_WTK_WRDVEC
#include "wtk/core/wtk_type.h"
#include "wtk/core/math/wtk_mat.h"
#include "wtk_wrdvec_cfg.h"
#include "wtk/core/wtk_robin.h"
#include "wtk/core/rbin/wtk_rbin2.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_wrdvec wtk_wrdvec_t;

struct wtk_wrdvec {
    wtk_wrdvec_cfg_t *cfg;
    wtk_rbin2_t *rbin;
    wtk_segmenter_t *seg;
    wtk_vecf_t *v1;
    wtk_vecf_t *v2;
    wtk_vecf_t *vx;
    wtk_strbuf_t *buf;
    FILE *bin;
    int f_of;
    int f_len;
};

wtk_wrdvec_t* wtk_wrdvec_new(wtk_wrdvec_cfg_t *cfg, wtk_rbin2_t *rbin);
void wtk_wrdvec_delete(wtk_wrdvec_t *v);
float wtk_wrdvec_like(wtk_wrdvec_t *v, char *s1, int s1_bytes, char *s2,
        int s2_bytes);
void wtk_wrdvec_test(wtk_wrdvec_t *v, char *s1, int s1_bytes, char *s2,
        int s2_bytes);
void wtk_wrdvec_like2(wtk_wrdvec_t *v, char *s1, int s1_bytes);
wtk_string_t wtk_wrdvec_best_like(wtk_wrdvec_t *v, char *dst, int dst_bytes,
        char *src, int src_bytes, float *pf);

/**
 * @brief use segmenter
 */
void wtk_wrdvec_snt_to_vec(wtk_wrdvec_t *v, char *s1, int s1_bytes,
        wtk_vecf_t *v1);

void wtk_wrdvec_snt_to_vec3(wtk_wrdvec_t *v, wtk_string_t **strs, int nstrs,
        wtk_vecf_t *v1);

/**
 * @brief use space as seg
 */
float wtk_wrdvec_snt_to_vec2(wtk_wrdvec_t *v, char *s, int s_bytes,
        wtk_vecf_t *vx);

wtk_robin_t* wtk_wrdvec_find_best_like_word(wtk_wrdvec_t *v, char *s1,
        int s1_bytes, int len, float thresh);
#ifdef __cplusplus
}
;
#endif
#endif
