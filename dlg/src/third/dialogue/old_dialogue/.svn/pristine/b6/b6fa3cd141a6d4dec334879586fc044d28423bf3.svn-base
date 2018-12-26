#ifndef WTK_CORE_WTK_RINGARRAY_H_
#define WTK_CORE_WTK_RINGARRAY_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_ringarray wtk_ringarray_t;

/**
 * @brief ring array is used for save fixed size array,
 *  |1|2|3|4|5|
 *  => push 6
 *  |2|3|4|5|6|
 */
struct wtk_ringarray
{
	void *slot;
	int slot_size;
	int nslot;
	int used;
};

wtk_ringarray_t* wtk_ringarray_new(int nslot,int slot_size);
void wtk_ringarray_delete(wtk_ringarray_t *r);
void wtk_ringarray_push(wtk_ringarray_t *r,void *item);

//-------------- float ringarray section ---------
float wtk_ringarray_mean_value(wtk_ringarray_t *r);
void wtk_ringarray_print_float(wtk_ringarray_t *r);
//----------- test/example section ---------------
void wtk_ringarray_test_g();
#ifdef __cplusplus
};
#endif
#endif
