#ifndef WTK_CORE_WTK_SORT_H_
#define WTK_CORE_WTK_SORT_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 *	return  src-dst
 */
typedef float (*wtk_qsort_cmp_f)(void *app_data,void *src,void *dst);

/**
 * return usr_data - dst_data;
 */
typedef int(*wtk_search_cmp_f)(void *usr_data,void *dst_data);

/**
 * @brief 0<1<2<3
 * @param s, the first elem;
 * @param e, the last  elem;
 * @param size: bytes of element;
 */
void wtk_qsort(void *s,void *e,size_t size,wtk_qsort_cmp_f cmp,void *app_data,void *tmp_elem);
void wtk_qsort2(void *base,size_t nmemb,size_t size,wtk_qsort_cmp_f cmp,void *app_data);
void wtk_qsort3(void *base,size_t nmemb,size_t size,wtk_qsort_cmp_f cmp,void *app_data,void *tmp_elem);

void wtk_qsort_float(float *f,int len);
void wtk_qsort_double(double *f,int len);

/**
 *  @brief search by element 0,1,2,3
 *	@param s, the first elem;
 *	@param e, the last elem;
 *	@param size, bytes of element;
 */
void* wtk_inc_search(void *s,void *e,int size,wtk_search_cmp_f cmp,void *usr_data);

/**
 *  @brief search in order array;
 *	@param s, the first elem;
 *	@param e, the last elem;
 *	@param size, bytes of element;
 *	[s,e]
 */
void* wtk_binary_search(void *s,void *e,int size,wtk_search_cmp_f cmp,void *usr_data);
#ifdef __cplusplus
};
#endif
#endif
