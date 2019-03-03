#ifndef WLIB_CORE_WTK_BIT_HEAP_H_
#define WLIB_CORE_WTK_BIT_HEAP_H_
#include "wtk/core/wtk_type.h"
#include "wtk_alloc.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_bit_heap wtk_bit_heap_t;
//typedef int (*heap_clean_handler)(void* data);

typedef struct _HeapBlock
{
	size_t elem_num;
	size_t elem_free;
	size_t first_free;
	uint8_t	*bitmap;
	uint8_t	*data;
	struct _HeapBlock* next;
}HeapBlock;

struct wtk_bit_heap
{
	HeapBlock* block_list;
	//heap_clean_handler cleaner;
	size_t elem_size;
	size_t elem_cur;
	size_t elem_min;
	size_t elem_max;
	size_t tot_alloc;
	size_t tot_used;
	float growf;
};

/**
 * @brief allocate bitmap heap.
 */
wtk_bit_heap_t* wtk_bit_heap_new(size_t elem_size,size_t elem_min,size_t elem_max,float growf);

/**
 * @brief allocate bitmap heap with elem_size.
 */
wtk_bit_heap_t* wtk_bit_heap_new2(size_t elem_size);

/**
 * @brief release all memory.
 */
int wtk_bit_heap_delete(wtk_bit_heap_t* heap);


int wtk_bit_heap_bytes(wtk_bit_heap_t *heap);

/**
 * @brief reset heap.
 */
int wtk_bit_heap_reset(wtk_bit_heap_t* heap);

/**
 * @brief allocate memory from heap.
 */
void* wtk_bit_heap_malloc(wtk_bit_heap_t* heap);

/**
 * @brief allocate with zero.
 */
void* wtk_bit_heap_zmalloc(wtk_bit_heap_t *heap);

/**
 * @brief delete bitmap memory.
 */
int wtk_bit_heap_free(wtk_bit_heap_t* heap,void* p);

/**
 * @brief check heap is valid or not.
 */
int wtk_bit_heap_is_valid(wtk_bit_heap_t *heap);

/**
 * @brief print bit heap;
 */
void wtk_bit_heap_print(wtk_bit_heap_t *heap);

//================ test section ===============
void wtk_bit_heap_test_g();
#ifdef __cplusplus
};
#endif
#endif
