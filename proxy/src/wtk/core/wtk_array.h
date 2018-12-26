#ifndef WTK_CORE_WTK_ARRAY_H_
#define WTK_CORE_WTK_ARRAY_H_
#include "wtk/core/wtk_type.h"
#include "wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_array wtk_array_t;

struct wtk_array
{
	void	*slot;
	uint32_t nslot;
	uint32_t slot_size;
	uint32_t slot_alloc;
	wtk_heap_t* heap;
};

/**
 * @brief create array from heap.
 */
wtk_array_t* wtk_array_new_h(wtk_heap_t* h,uint32_t n,uint32_t size);

/**
 * @brief dispose array.
 */
int wtk_array_dispose(wtk_array_t* a);

void wtk_array_reset(wtk_array_t *a);

/**
 * @brief push 1 element and return address
 */
void* wtk_array_push(wtk_array_t* a);

void wtk_array_push2(wtk_array_t *a,void *addr);

/**
 * @brief push n element and return address
 */
void* wtk_array_push_n(wtk_array_t* a,uint32_t n);

/**
 * @brief int array print
 */
void wtk_array_print_int(wtk_array_t *a);

/**
 * @brief float array print
 */
void wtk_array_print_float(wtk_array_t *a);

/**
 * @brief wtk_string_t* array print
 */
void wtk_array_print_string(wtk_array_t *a);

int wtk_array_str_in(wtk_array_t *a,char *s,int s_bytes);

int wtk_array_str_has(wtk_array_t *a,char *s,int s_bytes);

int wtk_array_str_end_with(wtk_array_t *a,char *s,int s_bytes);

int wtk_array_str_start_with(wtk_array_t *a,char *s,int s_bytes);

/**
 * @brief split by seperate char;
 */
wtk_array_t* wtk_str_to_array(wtk_heap_t *heap,char *data,int bytes,char sep);

/*
 * @brief split by ws
 */
wtk_array_t* wtk_str_to_array2(wtk_heap_t *heap,char *data,int bytes);

/**
 *	@brief utf-8 string to chars;
 */
wtk_array_t* wtk_str_to_chars(wtk_heap_t *heap,char *data,int bytes);
#ifdef __cplusplus
};
#endif
#endif
