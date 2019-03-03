#ifndef WTK_CORE_MATH_WTK_IMAT_H_
#define WTK_CORE_MATH_WTK_IMAT_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef int* wtk_imat_t;
#define wtk_imat_bytes(n,m) (sizeof(int)+sizeof(int*)*(n+1)+n*m*sizeof(int))
#define wtk_fmat_bytes(n,m) (sizeof(int)+sizeof(float*)*(n+1)+n*m*sizeof(float))
#define wtk_imat_data(i) ((char*)i+sizeof(int*)*(wtk_imat_row(i)+1))
#define wtk_imat_row(m) *((int*)(m)-1)
#define wtk_imat_col(m) *((int*)(m))

/**
 * m=|N*M|
 * m[1][1]=0
 * m[N][M]=0
 */
wtk_imat_t* wtk_imat_new(int row,int col);
void wtk_imat_delete(wtk_imat_t *m);
void wtk_imat_reset(wtk_imat_t *m);
void wtk_imat_print(wtk_imat_t *m);
double wtk_imat_euclid_distance(wtk_imat_t *m1,wtk_imat_t *m2);
double wtk_imat_cos(wtk_imat_t *m1,wtk_imat_t *m2);
//---------------- float mat section -----------------
typedef float* wtk_fmat_t;
wtk_fmat_t* wtk_fmat_new(int row,int col);
void wtk_fmat_delete(wtk_fmat_t *m);
void wtk_fmat_print(wtk_fmat_t *m);
#ifdef __cplusplus
};
#endif
#endif
