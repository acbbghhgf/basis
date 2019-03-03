#ifndef WTK_FST_STRLIKE_WTK_CHNLIKE
#define WTK_FST_STRLIKE_WTK_CHNLIKE
#include "wtk/core/wtk_type.h" 
#include "wtk/core/text/wtk_txtpeek.h"
#include "wtk/core/wtk_fkv.h"
#include "wtk_strlike.h"
#include "wtk_chnlike_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_chnlike wtk_chnlike_t;

struct wtk_chnlike
{
	wtk_chnlike_cfg_t *cfg;
	wtk_txtpeek_t *peek;
	wtk_strlike_t *strlike;
	wtk_heap_t *heap;
	wtk_strbuf_t *buf;
	wtk_fkv_t *fkv;
};


wtk_chnlike_t *wtk_chnlike_new(wtk_chnlike_cfg_t *cfg,wtk_rbin2_t *rbin);
void wtk_chnlike_delete(wtk_chnlike_t *l);
void wtk_chnlike_reset(wtk_chnlike_t *l);
float wtk_chnlike_like(wtk_chnlike_t *l,char *txt1,int txt1_bytes,char *txt2,int txt2_bytes,int *err);
#ifdef __cplusplus
};
#endif
#endif
