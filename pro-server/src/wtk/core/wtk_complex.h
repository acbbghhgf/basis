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

float*** wtk_float_new_p3(int n1,int n2,int n3);
void wtk_float_delete_p3(float ***pf,int n1,int n2);
float** wtk_float_new_p2(int n1,int n2);

wtk_complex_t** wtk_complex_new_p2(int n1,int n2);
wtk_complex_t*** wtk_complex_new_p3(int n1,int n2,int n3);
wtk_complex_t**** wtk_complex_new_p4(int n1,int n2,int n3,int n4);
void wtk_complex_delete_p4(wtk_complex_t ****p,int n1,int n2,int n3);

void wtk_complex_delete_p2(wtk_complex_t **p2,int n1);
void wtk_complex_delete_p3(wtk_complex_t ***p3,int n1,int n2);
void wtk_complex_zero_p3(wtk_complex_t ***p,int n1,int n2,int n3);


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

void wtk_dcomplex_print2(wtk_dcomplex_t *c,int r,int col);


//a=b*c  |1*4|*|4*2|
void wtk_complex_matrix_mul(wtk_complex_t *a,wtk_complex_t *b,wtk_complex_t *c,int row,int col,int col2);

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

void wtk_complex_invx(wtk_complex_t *a,int r,int c,wtk_complex_t *b,wtk_complex_t *at);
double wtk_complex_abs_det(wtk_complex_t *c,int n,char *bit);
#ifdef __cplusplus
};
#endif
#endif
