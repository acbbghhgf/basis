#ifndef WTK_MATH_WTK_VECTOR_BUFFER_H_
#define WTK_MATH_WTK_VECTOR_BUFFER_H_
#include "wtk_vector.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_vector_buffer wtk_vector_buffer_t;
#define wtk_vector_buffer_valid_len(b) ((b->cur)-(b->start))
#define wtk_vector_buffer_left_samples(b) ((b->end)-(b->cur))

struct wtk_vector_buffer
{
	float *rstart;
	float *start;
	float *cur;
	float *end;
	char odd_char;
	unsigned odd:1;
};

wtk_vector_buffer_t *wtk_vector_buffer_new(int size);

int wtk_vector_buffer_delete(wtk_vector_buffer_t *b);

int wtk_vector_buffer_reset(wtk_vector_buffer_t *b);
/*
 * @brief push sample data to vector.
 * @return samples that saved into buffer.
 */
int wtk_vector_buffer_push(wtk_vector_buffer_t *v,short* data,int samples);
int wtk_vector_buffer_push_float(wtk_vector_buffer_t *v,float* data,int samples);

/**
 * @brief peek vector from buffer.
 */
int wtk_vector_buffer_peek(wtk_vector_buffer_t *b,wtk_vector_t *v,int is_end);
int wtk_vector_buffer_copy_data(wtk_vector_buffer_t *b,float *data,int samples);
float* wtk_vector_buffer_peek_data(wtk_vector_buffer_t *b,int samples);

/**
 * use this function,be clear that samples must be smaller than data in buffer.
 * @brief skip sample,and make sure there is valid continuous memory with left count.
 */
void wtk_vector_buffer_skip(wtk_vector_buffer_t *b,int samples,int left_enough);
int wtk_vector_buffer_push_c(wtk_vector_buffer_t *v,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
