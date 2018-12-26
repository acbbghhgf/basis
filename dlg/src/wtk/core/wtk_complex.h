#ifndef WTK_CORE_WTK_COMPLEX
#define WTK_CORE_WTK_COMPLEX
#include "wtk/core/wtk_type.h" 
#include <math.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_complex wtk_complex_t;
struct wtk_complex
{
	float a;
	float b;
};

int isinvalid(double f);

void wtk_complex_check(wtk_complex_t *c,int n);

double*** wtk_double_new_p3(int n1,int n2,int n3);

float*** wtk_float_new_p3(int n1,int n2,int n3);
void wtk_float_delete_p3(float ***pf,int n1,int n2);
float** wtk_float_new_p2(int n1,int n2);
void wtk_float_p2_zero(float **p,int n1,int n2);
void wtk_float_delete_p2(float **pf,int n1);
double** wtk_double_new_p2(int n1,int n2);
void wtk_double_p2_zero(double **p,int n1,int n2);
void wtk_double_delete_p2(double **pf,int n1);
void wtk_float_p3_set(float ***p,int n1,int n2,int n3,float v);

void wtk_double_p3_set(double ***p,int n1,int n2,int n3,float v);

void wtk_complex_eye(wtk_complex_t **p,int n,wtk_complex_t v);
void wtk_complex_eye2(wtk_complex_t *p,int n);

wtk_complex_t** wtk_complex_new_p2(int n1,int n2);
wtk_complex_t*** wtk_complex_new_p3(int n1,int n2,int n3);
wtk_complex_t**** wtk_complex_new_p4(int n1,int n2,int n3,int n4);
void wtk_complex_delete_p4(wtk_complex_t ****p,int n1,int n2,int n3);
void wtk_complex_p2_mul(wtk_complex_t **p,int n1,int n2,float f);

void wtk_complex_identity(wtk_complex_t *a,int n);

void wtk_complex_delete_p2(wtk_complex_t **p2,int n1);
void wtk_complex_delete_p3(wtk_complex_t ***p3,int n1,int n2);
void wtk_complex_zero_p3(wtk_complex_t ***p,int n1,int n2,int n3);
void wtk_complex_cpy_p3(wtk_complex_t ***dst,wtk_complex_t ***src,int n1,int n2,int n3);
void wtk_complex_cpy_p2(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2);
void wtk_complex_zero_p2(wtk_complex_t **p,int n1,int n2);
void wtk_complex_p2_cpy(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2);


void wtk_complex_print(wtk_complex_t *c,int n);
void wtk_complex_print2(wtk_complex_t *c,int r,int col);
wtk_complex_t wtk_complex_div(wtk_complex_t *a,wtk_complex_t *b);
wtk_complex_t wtk_complex_mul(wtk_complex_t *a,wtk_complex_t *b);
void wtk_complex_sub(wtk_complex_t *a,wtk_complex_t b);

typedef struct wtk_dcomplex wtk_dcomplex_t;
struct wtk_dcomplex
{
	double a;
	double b;
};
wtk_dcomplex_t wtk_dcomplex_mul(wtk_dcomplex_t *a,wtk_complex_t *b);

wtk_dcomplex_t wtk_dcomplex_mul2(wtk_dcomplex_t *a,wtk_dcomplex_t *b);

void wtk_dcomplex_zero_p3(wtk_dcomplex_t ***p,int n1,int n2,int n3);

void wtk_dcomplex_print2(wtk_dcomplex_t *c,int r,int col);
void wtk_dcomplex_print(wtk_dcomplex_t *c,int n);


//a=b*c  |1*4|*|4*2|
void wtk_complex_matrix_mul(wtk_complex_t *a,wtk_complex_t *b,wtk_complex_t *c,int row,int col,int col2);
void wtk_complex_matrix_mul2(wtk_complex_t **a,wtk_complex_t **b,wtk_complex_t **c,int row,int col,int col2);

void print_complex(wtk_complex_t *c,int n);


typedef struct
{
	float *a;
	float *b;
	int n;
}wtk_complexa_t;

wtk_complexa_t* wtk_complexa_new(int n);
void wtk_complexa_delete(wtk_complexa_t *ca);
void wtk_complexa_zero(wtk_complexa_t *ca);
//a=b*c  |1*4|*|4*2|
void wtk_complexa_matrix_mul(wtk_complexa_t *a,wtk_complexa_t *b,wtk_complexa_t *c,int row,int col,int col2);
void wtk_complexa_print(wtk_complexa_t *ca);

void wtk_complex_inv(wtk_complex_t *a,int r,int c,wtk_complex_t *b);

int wtk_complex_invx(wtk_complex_t *a,int r,int c,wtk_complex_t *b,wtk_complex_t *at);
int wtk_complex_invx2(wtk_complex_t *a,int r,int c,wtk_complex_t *b);
int wtk_complex_invx3(wtk_complex_t **a,int r,int c,wtk_complex_t **b);

double wtk_complex_abs_det(wtk_complex_t *c,int n,char *bit);
void wtk_complex_det(wtk_complex_t *c,int n,char *bit,wtk_dcomplex_t *a,int step);

double wtk_complex_abs_det1(wtk_complex_t *c,wtk_dcomplex_t *c1,int n);
void wtk_dcomplex_det1(wtk_dcomplex_t *c, int n, wtk_dcomplex_t *a);

void wtk_complex_p2_print(wtk_complex_t **p,int n1,int n2);
void wtk_complex_p2_write(wtk_complex_t **p,int n1,int n2,char *fn);

void wtk_dcomplex_print(wtk_dcomplex_t *c,int n);
void wtk_dcomplex_p2_print(wtk_dcomplex_t **c,int n1,int n2);
wtk_dcomplex_t** wtk_dcomplex_new_p2(int n1,int n2);
void wtk_dcomplex_zero_p2(wtk_dcomplex_t **p,int n1,int n2);

wtk_dcomplex_t*** wtk_dcomplex_new_p3(int n1,int n2,int n3);
void wtk_dcomplex_zero_p3(wtk_dcomplex_t ***p,int n1,int n2,int n3);
void wtk_dcomplex_delete_p3(wtk_dcomplex_t ***p3,int n1,int n2);
void wtk_dcomplex_delete_p2(wtk_dcomplex_t **p2,int n1);

int wtk_dcomplex_invx3(wtk_dcomplex_t **a,int  n,wtk_dcomplex_t **b);
int wtk_dcomplex_lu_invx3(wtk_dcomplex_t **a,wtk_dcomplex_t **L,wtk_dcomplex_t **U,wtk_dcomplex_t **l,wtk_dcomplex_t **u,int n,wtk_dcomplex_t **b);

//int wtk_dcomplex_invx3(wtk_dcomplex_t **A, int n, unsigned char *is,unsigned char *js);

void wtk_dcomplex_matrix_mul(wtk_dcomplex_t **a,wtk_dcomplex_t **b,wtk_dcomplex_t **c,int row,int col,int col2);

void wtk_complex_p2_print2(wtk_complex_t **p,int n1,int n2);
void wtk_complex_p2_cpy(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2);
void wtk_dcomplex_p2_cpy(wtk_dcomplex_t **dst,wtk_dcomplex_t **src,int n1,int n2);
void wtk_dcomplex_p2_add_scale(wtk_dcomplex_t **dst,wtk_dcomplex_t **src,float alpha1,float alpha2,int n1,int n2);

void wtk_dcomplex_p2_add_scale_c(wtk_complex_t **dst,wtk_complex_t **src,float alpha,int n1,int n2);

void wtk_dcomplex_eye(wtk_dcomplex_t **p,int n,wtk_dcomplex_t v);
void wtk_dcomplex_p2_print2(wtk_dcomplex_t **c,int n1,int n2);
void wtk_dcomplex_mult(wtk_dcomplex_t **c,int n1,int n2,float f);

wtk_dcomplex_t**** wtk_dcomplex_new_p4(int n1,int n2,int n3,int n4);
void wtk_double_delete_p3(double ***pf,int n1,int n2);
void wtk_dcmplex_p2_mul(wtk_dcomplex_t **p,int n1,int n2,float f);
void wtk_complex_p2_cpy2(wtk_dcomplex_t **dst,wtk_complex_t **src,int n1,int n2);
double wtk_dcomplex_abs_det(wtk_dcomplex_t *c,int n,char *bit);

int wtk_dcomplex_invx(wtk_dcomplex_t *a,int r,int c,wtk_dcomplex_t *b,wtk_dcomplex_t *at);
int wtk_dcomplex_invx2(wtk_dcomplex_t *a,int r,int c,wtk_dcomplex_t *b);
void wtk_complex_cpy_p4(wtk_complex_t ***dst,wtk_dcomplex_t ***src,int n1,int n2,int n3);
void wtk_dcomplex_cpy_p3(wtk_dcomplex_t ***dst,wtk_complex_t ***src,int n1,int n2,int n3);
float wtk_complex_p2_dist(wtk_complex_t **p1,wtk_complex_t **p2,int n1,int n2);
float wtk_complex_p2_angle(wtk_complex_t **p1,wtk_complex_t **p2,int n1,int n2);
float wtk_complex_p2_max_abs(wtk_complex_t **p,int n1,int n2);

wtk_complex_t wtk_complex_p2_sum(wtk_complex_t **c,int n1,int n2);
void wtk_complex_p2_scale_add(wtk_complex_t **dst,wtk_complex_t **src,int n1,int n2,float f1);

// g=inv(a) *b;
void wtk_complex_guass_elimination(wtk_complex_t **a,wtk_complex_t **b,int n1,int n2);
int wtk_complex_guass_elimination_p1(wtk_complex_t *a,wtk_complex_t *b,int n1,int n2);
int wtk_dcomplex_guass_elimination(wtk_dcomplex_t **a,wtk_dcomplex_t **b,int n1,int n2);
#ifdef __cplusplus
};
#endif
#endif
