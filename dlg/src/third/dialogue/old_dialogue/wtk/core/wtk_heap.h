#ifndef WLIB_CORE_WTK_HEAP_H_
#define WLIB_CORE_WTK_HEAP_H_
#include "wtk/core/wtk_type.h"
#include "wtk_alloc.h"
#include "wtk_queue.h"
#include "wtk_queue2.h"
#include "wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif
struct wtk_array;

typedef struct wtk_heap wtk_heap_t;
typedef struct wtk_heap_block wtk_heap_block_t;
typedef struct wtk_heap_large wtk_heap_large_t;

#define wtk_heap_dup_string_s(h,s) wtk_heap_dup_string(h,s,sizeof(s)-1)

struct wtk_heap_block
{
	wtk_queue_node_t q_n;
	char *first;
	char *end;
	char *cur;
	unsigned int failed;
};

struct wtk_heap_large
{
	wtk_queue_node_t q_n;
	void *data;
	int size;
};

struct wtk_heap
{
	wtk_queue2_t block_q;
	wtk_queue2_t large_q;
	int align;
	int page_size;
	int alloc_size;
	int large_thresh;
	wtk_heap_block_t *cur;
};

wtk_heap_t* wtk_heap_new(int page_size);
wtk_heap_t* wtk_heap_new2(int page_size,int align_size);
void wtk_heap_delete(wtk_heap_t *heap);
void wtk_heap_reset(wtk_heap_t *heap);
int wtk_heap_bytes(wtk_heap_t *heap);
void* wtk_heap_malloc(wtk_heap_t *heap,int bytes);
void wtk_heap_free(wtk_heap_t *heap,void *p);
void wtk_heap_free_large(wtk_heap_t *heap,void *p);

/**
 * @brief allocate from heap and set zero.
 */
void* wtk_heap_zalloc(wtk_heap_t* heap,size_t size);

/**
 * @brief duplicate wtk_string_t;
 */
wtk_string_t* wtk_heap_dup_string(wtk_heap_t *h,char *s,int sl);

/**
 * @return string with 0 end.
 */
wtk_string_t* wtk_heap_dup_string2(wtk_heap_t *h,char *s,int sl);

/**
 * @brief return string will '\0' end.
 */
char *wtk_heap_dup_str(wtk_heap_t *heap,char* s);

/**
 * @brief return string will '\0 end with string;
 */
char* wtk_heap_dup_str2(wtk_heap_t *heap,char *data,int len);

/**
 * @brief duplicate data;
 */
char* wtk_heap_dup_data(wtk_heap_t *h,const char *s,int l);

/**
 * @brief print heap;
 */
void wtk_heap_print(wtk_heap_t *heap);

/**
 * @brief fill string with data;
 */
void wtk_heap_fill_string(wtk_heap_t *heap,wtk_string_t *str,char *data,int bytes);


struct wtk_array* wtk_utf8_string_to_chars(wtk_heap_t *heap,char *data,int bytes);

void wtk_heap_add_large(wtk_heap_t *heap,char *p,int size);
//=================== Test Section ==================
void wtk_heap_test_g();
#ifdef __cplusplus
};
#endif
#endif
