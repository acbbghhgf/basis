#ifndef WTK_FST_STRLIKE_WTK_STRLIKE_H_
#define WTK_FST_STRLIKE_WTK_STRLIKE_H_
#include "wtk/core/wtk_type.h"
#include "wtk_strlike_cfg.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_strlike wtk_strlike_t;
struct wtk_strlike
{
	wtk_strlike_cfg_t *cfg;
	wtk_heap_t *heap;
};

wtk_strlike_t* wtk_strlike_new(wtk_strlike_cfg_t *cfg);
void wtk_strlike_delete(wtk_strlike_t *l);
float wtk_strlike_process(wtk_strlike_t *l,char *ref,int ref_bytes,char *rec,int rec_bytes);
#ifdef __cplusplus
};
#endif
#endif
