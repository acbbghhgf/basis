#ifndef WTK_MATH_WTK_VECTOR_H_
#define WTK_MATH_WTK_VECTOR_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_alloc.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 *	v=[ 0,  1 ,  2,   3 ,  ... ,n  ]
 *	  size v[0] v[1] v[2]  ... v[n]
 */
//#define wtk_vector_type_bytes(size,type) ((size+1)*sizeof(type))
#define wtk_vector_type_bytes(size,type) (wtk_round((size+1)*sizeof(type),8))
#define wtk_vector_bytes(size) wtk_vector_type_bytes(size,float)
#define wtk_double_vector_bytes(size) wtk_vector_type_bytes(size,double)
#define wtk_svector_bytes(size) ((size+1)*sizeof(float)+2*sizeof(void*))
#define wtk_int_vector_bytes(size) wtk_vector_type_bytes(size,int)

#define wtk_vector_size(v) (*(int*)(v))
#define wtk_short_vector_size(v) (*(short*)v)
#define wtk_vector_init(v,size) (*((int*)v)=size)
//((int)(v[0]))
#define wtk_type_vector(type) CAT(CAT(wtk_,type),_vector_t)
#define wtk_type_vector_new_dec(t) wtk_type_vector(t)* CAT(CAT(wtk_,t),_vector_new)(int size)
#define wtk_type_vector_new_imp(t) \
wtk_type_vector_new_dec(t) \
{ \
	wtk_type_vector(t)* v; \
	\
	v=(wtk_type_vector(t)*)wtk_malloc(wtk_vector_type_bytes(size,t)); \
	if(sizeof(t)>=sizeof(int)) \
	{ \
		*((int*)v)=size; \
	}\
	{\
		*((short*)v)=size;\
	}\
	return v; \
}

#define wtk_type_vector_newh_dec(t) wtk_type_vector(t)* CAT(CAT(wtk_,t),_vector_newh)(wtk_heap_t *h,int size)
#define wtk_type_vector_newh_imp(t) \
wtk_type_vector_newh_dec(t) \
{ \
	wtk_type_vector(t)* v; \
	\
	v=(wtk_type_vector(t)*)wtk_heap_malloc(h,wtk_vector_type_bytes(size,t)); \
	if(sizeof(t)>=sizeof(int)) \
	{ \
		*((int*)v)=size; \
	}\
	{\
		*((short*)v)=size;\
	}\
	return v; \
}

#define wtk_vector_do_p(v,pre,after) \
{ \
	wtk_vector *s,*e;\
	s=v;e=v+wtk_vector_size(v);\
	while((e-s)>=4)\
	{\
		pre *(++s) after;\
		pre *(++s) after;\
		pre *(++s) after;\
		pre *(++s) after;\
	}\
	while(s<e)\
	{\
		pre *(++s) after; \
	}\
}

#define wtk_vector_do_i(v,pre,after) \
{ \
	int i,size; \
	i=1;size=wtk_vector_size(v);\
	for(i=1;i<=(size-4);)\
	{\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
	}\
	for(;i<=size;++i)\
	{\
		pre v[i] after;\
	}\
}

#define wtk_vector_do_i_step(v,pre,after,step) \
{ \
	int i,size; \
	i=1;size=wtk_vector_size(v);\
	for(i=1;i<=(size-step);)\
	{\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
		pre v[i] after;++i;\
	}\
	for(;i<=size;++i)\
	{\
		pre v[i] after;\
	}\
}
#define wtk_vector_delete(v) wtk_free(v)

typedef float wtk_vector;
typedef double wtk_double_vector_t;
typedef int wtk_int_vector_t;
typedef wtk_vector wtk_vector_t;
typedef short wtk_short_vector_t;
/* Shared versions */
typedef wtk_vector_t wtk_svector_t;    /* shared vector[1..size]   */

wtk_type_vector_new_dec(double);
wtk_type_vector_new_dec(short);
wtk_type_vector_newh_dec(short);
wtk_type_vector_newh_dec(double);


wtk_vector_t* wtk_vector_new_h(wtk_heap_t *heap,int size);
wtk_int_vector_t* wtk_int_vector_new_h(wtk_heap_t *heap,int size);
wtk_vector_t* wtk_vector_new(int size);
int wtk_vector_bytes2(wtk_vector_t *v);
wtk_vector_t *wtk_vector_dup(wtk_vector_t *src);
void wtk_vector_cpy(wtk_vector_t *src,wtk_vector_t *dst);
void wtk_double_vector_cpy(wtk_double_vector_t *src,wtk_double_vector_t *dst);
void wtk_vector_zero(wtk_vector_t *v);
void wtk_double_vector_zero(wtk_double_vector_t *v);
float wtk_vector_sum(wtk_vector_t* v);
void wtk_vector_print(wtk_vector_t* v);
void wtk_short_vector_print(wtk_short_vector_t* v);
wtk_svector_t* wtk_svector_newh(wtk_heap_t* heap, int size);
wtk_svector_t* wtk_svector_dup(wtk_heap_t* heap, wtk_svector_t *src);
void wtk_set_use(void **m,int n);
int wtk_get_use(void **m);
void wtk_inc_use(void **m);
void wtk_dec_use(void **m);
void wtk_set_hook(void **m,void *h);
void* wtk_get_hook(void **m);
float wtk_math_max(float *a,int len);
float wtk_vector_max_abs(wtk_vector_t *v);
void wtk_vector_fix_scale(wtk_vector_t *v,float scale);
void wtk_vector_set_init_value(wtk_vector_t *v,float f);
wtk_vector_t *wtk_vector_new2(int size);
void wtk_vector_delete2(wtk_vector_t *v);


#ifdef __cplusplus
};
#endif
#endif
