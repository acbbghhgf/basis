#ifndef WTK_CORE_SEGMENTER_WTK_WVEC
#define WTK_CORE_SEGMENTER_WTK_WVEC
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk/core/math/wtk_mat.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_wvec wtk_wvec_t;

typedef double wtk_wvec_float_t;

typedef struct
{
	wtk_string_t *name;
	//wtk_vecf_t *m;
	wtk_wvec_float_t *p;
	int wrd_idx;
}wtk_wvec_item_t;


struct wtk_wvec
{
	int voc_size;
	int vec_size;
	wtk_wvec_float_t *v1;
	wtk_wvec_float_t *v2;
	wtk_str_hash_t *hash;
	wtk_wvec_item_t **wrds;
	wtk_wvec_item_t *unk;
};

wtk_wvec_t* wtk_wvec_new(char *fn);
void wtk_wvec_delete(wtk_wvec_t *v);
float wtk_wvec_snt_to_vec(wtk_wvec_t *v,char *s,int s_bytes,wtk_vecf_t *vec);
float wtk_wvec_like(wtk_wvec_t *v,char *s1,int s1_bytes,char *s2,int s2_bytes);
wtk_wvec_item_t* wtk_wvec_find(wtk_wvec_t *cfg,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
