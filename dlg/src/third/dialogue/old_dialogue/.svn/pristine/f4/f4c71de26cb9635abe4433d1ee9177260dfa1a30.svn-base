#ifndef WTK_CORE_WTK_LARRAY_H_
#define WTK_CORE_WTK_LARRAY_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_larray wtk_larray_t;
#define wtk_larray_push2_s(l,t,v) {t x1=v;wtk_larray_push2(l,&x1);}

struct wtk_larray
{
	void* slot;
	uint32_t nslot;
	uint32_t slot_size;
	uint32_t slot_alloc;
};

wtk_larray_t* wtk_larray_new(uint32_t n,uint32_t size);
int wtk_larray_delete(wtk_larray_t *a);
int wtk_larray_bytes(wtk_larray_t *a);
wtk_larray_t* wtk_larray_dup(wtk_larray_t *a);
void wtk_larray_cpy(wtk_larray_t *src,wtk_larray_t *dst);
void wtk_larray_merge(wtk_larray_t *merged,wtk_larray_t *pad);
void wtk_larray_reset(wtk_larray_t *a);
void wtk_larray_reset2(wtk_larray_t *a,int n);
/**
 * @brief return address of element, for float element,will return float*;
 */
void* wtk_larray_push(wtk_larray_t* a);
void wtk_larray_push2(wtk_larray_t *a,void *src);
void* wtk_larray_push_n(wtk_larray_t* a,uint32_t n);
void* wtk_larray_get(wtk_larray_t *a,int idx);


typedef struct
{
	float *p;
	int len;
	int pos;
}wtk_flta_t;

wtk_flta_t* wtk_flta_new(int n);
void wtk_flta_delete(wtk_flta_t *a);
void wtk_flta_reset(wtk_flta_t *a);
void wtk_flta_zero(wtk_flta_t *a);
#define wtk_flta_push(a,f) (a)->p[(a)->pos++]=f


//------------------------------- test/examle section ------------------
void wtk_larray_test_g();
#ifdef __cplusplus
};
#endif
#endif
