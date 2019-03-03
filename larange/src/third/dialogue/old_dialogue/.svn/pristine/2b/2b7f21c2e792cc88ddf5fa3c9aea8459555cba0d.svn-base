#ifndef WTK_MATH_WTK_MATRIX_H_
#define WTK_MATH_WTK_MATRIX_H_
#include "wtk_vector.h"
#ifdef __cplusplus
extern "C" {
#endif
/**
 * m[3][3]	   |<-   double *    ->|<----------------         double     --------------------------->|
 *	   memory: | c0 | c1 | c2 | c3 | c10 | c11 | c12 | c13 | c21 | c22 | c23 | c30 | c31 | c32 | c33 |
 *	        => | c0 | c1 | c2 | c3 |   //c0 =rows(3) (double**)
 *	    	   | c10| c11| c12| c13|   //c1 ->pointer c10, c10=cols(3),(double*)
 *	    	   | c20| c21| c22| c23|
 *	    	   | c30| c31| c32| c33|
 *
 *	c0-c3: double array index,c0(int)=3, c1 -> c10, c10(int) =3
 *	c11-c12: array;
 */

///// fast exp() implementation
union {
	double d;
	struct {
		int j, i;
	} n;
} d2i;
#define EXP_A (1048576/M_LN2)
#define EXP_C 60801
#define FAST_EXP(y) (d2i.n.i = EXP_A*(y)+(1072693248-EXP_C),d2i.d)

typedef double* wtk_double_matrix_t;
typedef float* wtk_matrix_t;
typedef float* wtk_smatrix_t;
typedef wtk_matrix_t wtk_trimat_t;
typedef int* wtk_int_matrix_t;
typedef char* wtk_char_matrix_t;

#define wtk_double_matrix_bytes(r,c) wtk_round_word(wtk_double_vector_bytes(c)*r+wtk_round_word((r+1)*sizeof(wtk_double_vector_t*)))
#define wtk_matrix_bytes(r,c) wtk_round_word(wtk_vector_bytes(c)*r+wtk_round_word((r+1)*sizeof(wtk_vector_t*)))
#define wtk_smatrix_bytes(r,c) wtk_round_word(wtk_svector_bytes(c)*r+wtk_round_word((r+3)*sizeof(wtk_vector_t*)))
#define wtk_matrix_rows(m) (*(int*)m)
#define wtk_matrix_cols(m) (*(int*)(m[1]))
#define wtk_matrix_delete(m) free(m)
#define wtk_double_matrix_delete(m) wtk_free(m)

wtk_double_matrix_t* wtk_double_matrix_new(int nrows,int ncols);
wtk_double_matrix_t* wtk_double_matrix_new_h(wtk_heap_t *heap,int nrows,int ncols);
void wtk_double_matrix_zero(wtk_double_matrix_t *m);
void wtk_double_matrix_cpy(wtk_double_matrix_t *src,wtk_double_matrix_t *dst);
void wtk_double_matrix_init_identity(wtk_double_matrix_t *A);
void wtk_double_matrix_cpy(wtk_double_matrix_t *src,wtk_double_matrix_t *dst);
wtk_matrix_t* wtk_matrix_new(int nrows,int ncols);
int wtk_matrix_bytes2(wtk_matrix_t *m);
wtk_matrix_t* wtk_matrix_newh(wtk_heap_t* h,int nrows,int ncols);
wtk_smatrix_t* wtk_smatrix_newh(wtk_heap_t *h,int nrows,int ncols);
void wtk_matrix_add(wtk_matrix_t *m,wtk_matrix_t *a);

/**
 * m=|a|*|b|
 */
void wtk_matrix_multi(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b);
void wtk_matrix_to_vector(wtk_matrix_t *m,wtk_vector_t *v);
void wtk_matrix_print(wtk_matrix_t *m);

void wtk_matrix_transpose(wtk_matrix_t *dst,wtk_matrix_t *src);
void wtk_matrix_cpy(wtk_matrix_t *src,wtk_matrix_t *dst);
wtk_matrix_t* wtk_matrix_transpose2(wtk_matrix_t *a);
void wtk_matrix_scale(wtk_matrix_t *m,float scale);

double wtk_matrix_max(wtk_matrix_t *m);
double wtk_matrix_min(wtk_matrix_t *m);
double wtk_matrix_max_abs(wtk_matrix_t *m);

wtk_int_matrix_t* wtk_int_matrix_new(int r,int c);
void wtk_int_matrix_print(wtk_int_matrix_t *m);
wtk_matrix_t* wtk_matrix_new2(int r,int col);
void wtk_matrix_multi6(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b);

wtk_char_matrix_t* wtk_char_matrix_new(int r,int c);
void wtk_matrix_multi2(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b);
void wtk_matrix_multi3(wtk_matrix_t *m,wtk_matrix_t *a,wtk_matrix_t *b);
float wtk_matrix_avg(wtk_matrix_t *m);

typedef double(*wtk_matrix_random_f)();
void wtk_matrix_set_random(wtk_matrix_t *m,wtk_matrix_random_f random);
void wtk_matrix_zero(wtk_matrix_t *m);
void wtk_matrix_set_init_value(wtk_matrix_t *m,double f);

/**
 *	|1*src| *|src* dst| = |1*dst|
 */
void wtk_vector_mult_matrix(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m);
/**
 * [s,e)
 */
void wtk_vector_mult_matrix2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int s,int e);


/**
 *	|dst * src| *|src*1|=|dst*1|
 */
void wtk_vector_mult_matrix_rev(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int add);
void wtk_vector_mult_matrix_rev2(wtk_vector_t *dst,wtk_vector_t *src,wtk_matrix_t *m,int s,int e);
void wtk_matrix_add2(wtk_matrix_t *dst,wtk_matrix_t *src,float f1,float f2);

double wtk_fast_exp(double y);
float wtk_fast_exp2(float y);
#ifdef __cplusplus
};
#endif
#endif
