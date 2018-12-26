#ifdef USE_CRF
#ifndef WTK_SEMDLG_OWLKG_WTK_CRF
#define WTK_SEMDLG_OWLKG_WTK_CRF
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_crf wtk_crf_t;

struct wtk_crf
{
	void *tager;
};

wtk_crf_t* wtk_crf_new(char *fn);
void wtk_crf_delete(wtk_crf_t *c);
void wtk_crf_reset(wtk_crf_t *c);
int wtk_crf_add(wtk_crf_t *c,char *s);
const char* wtk_crf_get_result(wtk_crf_t *c);
int wtk_crf_nresult(wtk_crf_t *c);
const char* wtk_crf_get(wtk_crf_t *c,int i);
float wtk_crf_get_prob(wtk_crf_t *c,int i);
int wtk_crf_process(wtk_crf_t *c);
#ifdef __cplusplus
};
#endif
#endif
#endif
