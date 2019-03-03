#ifndef WTK_VITE_MATH_WTK_FIXMATH_H_
#define WTK_VITE_MATH_WTK_FIXMATH_H_
#include "wtk/core/wtk_type.h"
#include "wtk_matrix.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	char *p;
	int row;
	int col;
	float scale;
}wtk_fixc_t;


wtk_fixc_t* wtk_fixc_new(wtk_matrix_t *mat);
void wtk_fixc_delete(wtk_fixc_t *fixc);


typedef struct
{
	int *p;
	int row;
	int col;
	float scale;
}wtk_fixi_t;


wtk_fixi_t* wtk_fixi_new(int row,int col);
void wtk_fixi_delete(wtk_fixi_t *fixi);

void wtk_fixi_mult_mv(wtk_fixi_t *fix,wtk_matrix_t *m,wtk_vector_t *src,float scale);

void wtk_fixi_scale(wtk_fixi_t *fix,wtk_vector_t *v,float scale);

void wtk_fixi_print(wtk_fixi_t *fix);

#ifdef __cplusplus
};
#endif
#endif
