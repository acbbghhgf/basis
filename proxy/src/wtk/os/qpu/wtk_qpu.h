#ifdef USE_QPU
#ifndef _WTK_QPU_H_
#define _WTK_QPU_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_qpu wtk_qpu_t;
typedef struct wtk_qpu_uniform wtk_qpu_uniform_t;

enum wtk_qpu_blas_order{
	wtk_qpu_blas_row_major=101,
	wtk_qpu_blas_col_major=102,
};

enum wtk_qpu_blas_transpose{
	wtk_qpu_blas_no_trans=111,
	wtk_qpu_blas_trans=112,
};

typedef struct wtk_qpu_matrix{
	  float	*m;
	  int row;
	  int col;
	  int max_row;
	  int max_col;
	  uint32_t gpu_mem_handle;
	  uint32_t gpu_mem_base;
}wtk_qpu_matrix_t;

struct wtk_qpu_uniform{
	uint32_t m;
	uint32_t n;
	uint32_t k;
	uint32_t alpha;
	uint32_t mat_a;
	uint32_t amin;
	uint32_t amax;
	uint32_t lda;
	uint32_t mat_b;
	uint32_t ldb;
	uint32_t beta;
	uint32_t mat_c;
	uint32_t ldc;
	uint32_t cur_debug;
	uint32_t index;
};

struct wtk_qpu{
	uint32_t *gcode;
	unsigned int gcode_bytes;
	unsigned int msg_byes;
	int uniform_num;
	unsigned int uniform_bytes;
	int debug_count;
	unsigned int debug_bytes;
	unsigned int total_bytes;
	uint32_t gpu_mem_handle;
	uint32_t gpu_mem_base;
	char	*arm_mem_base;
	char *arm_msg_base;
	char *arm_gcode_base;
	char * arm_uniform_base;
	char *arm_debug_base;
	uint32_t gpu_msg_base;
	uint32_t gpu_gcode_base;
	uint32_t gpu_uniform_base;
	uint32_t gpu_debug_base;
	float amin;
	float amax;
	int abits;
};


wtk_qpu_t* wtk_qpu_new(void);
void wtk_qpu_delete(wtk_qpu_t *q);

#define wtk_qpu_matrix_at(a,i,j)	(((a)->m)+((i)*((a)->col))+(j))
int wtk_qpu_matrix_init(wtk_qpu_t *q,wtk_qpu_matrix_t *mat,int row,int col);
void wtk_qpu_matrix_destroy(wtk_qpu_t *q,wtk_qpu_matrix_t *mat);
wtk_qpu_matrix_t* wtk_qpu_matrix_new(wtk_qpu_t *q,int row,int col);
void wtk_qpu_matrix_delete(wtk_qpu_t *q,wtk_qpu_matrix_t *mat);
void wtk_qpu_matrix_transpose(wtk_qpu_t *q,wtk_qpu_matrix_t *dst,wtk_qpu_matrix_t *src);
void wtk_qpu_matrix_print(wtk_qpu_t *q,wtk_qpu_matrix_t *mat);

void wtk_qpu_cblas_sgemm(
		wtk_qpu_t *q,
		int order,
		int transposeA,
		int transposeB,
		int m,
		int n,
		int k,
		float alpha,
		wtk_qpu_matrix_t *a,
		int lda,
		wtk_qpu_matrix_t *b,
		int ldb,
		float beta,
		wtk_qpu_matrix_t *c,
		int ldc);


#ifdef __cplusplus
};
#endif
#endif
#endif
