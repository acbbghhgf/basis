#ifndef WTK_CORE_WTK_NBEST
#define WTK_CORE_WTK_NBEST
#include "wtk/core/wtk_type.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nbest wtk_nbest_t;


//dst-src;
typedef float(*wtk_nbest_cmp_f)(void *ths,void *dst,void *src);

struct wtk_nbest
{
	char *item;
	int size;
	int nslot;
	int pos;
	void *ths;
	wtk_nbest_cmp_f cmp;
};

wtk_nbest_t* wtk_nbest_new(int size,int nbest,void *cmp_ths,wtk_nbest_cmp_f cmp);
void wtk_nbest_delete(wtk_nbest_t *n);
void wtk_nbest_reset(wtk_nbest_t *n);
void wtk_nbest_add_slot(wtk_nbest_t *v,void *slot);
void wtk_nbest_remove(wtk_nbest_t *v,int idx);
void* wtk_nbest_get(wtk_nbest_t *v,int idx);
#ifdef __cplusplus
};
#endif
#endif
