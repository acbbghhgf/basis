#ifndef WTK_CORE_WTK_HASH_H_
#define WTK_CORE_WTK_HASH_H_
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_hash wtk_hash_t;
typedef unsigned int (*wtk_hash_f)(void* v,unsigned int nslot);


/**
 * @return 0 on equal.
 */
typedef int (*wtk_cmp_f)(void *v,void *k);

typedef struct
{
	wtk_queue_node_t q_n;
	void *v;
}wtk_hash_node_t;

struct wtk_hash
{
	wtk_heap_t *heap;
	wtk_queue_t **slot;
	unsigned int nslot;
};

/**
 * @brief create hash table;
 */
wtk_hash_t* wtk_hash_new(int nslot);

/**
 * @brief delete hash table;
 */
int wtk_hash_delete(wtk_hash_t *h);

/**
 * @brief reset hash table;
 */
int wtk_hash_reset(wtk_hash_t *h);

int wtk_hash_bytes(wtk_hash_t *h);

/**
 * @brief add element use hf hash v;
 */
int wtk_hash_add(wtk_hash_t *h,void *v,wtk_hash_f hf);
int wtk_hash_add2(wtk_hash_t *h,unsigned int id,void *v);

int wtk_hash_add_node(wtk_hash_t *h,void *v,wtk_hash_node_t *n,wtk_hash_f hf);

/**
 * @brief
 * 		hf, hash(k)
 *		cf:  cmp(v,k)
 */
wtk_hash_node_t* wtk_hash_find_node(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf);
/**
 * @brief find value by hf,cf;
 */
void* wtk_hash_find(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf);

wtk_hash_node_t* wtk_hash_remove(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf);

/**
 * @brief elements of hash;
 */
int wtk_hash_len(wtk_hash_t *h);

/*-------------------------------------------------------------------------*/
/**
 *  in handler(void* user_data,void* data),data is an pointer to wtk_hash_node_t.
 */
/*--------------------------------------------------------------------------*/
int wtk_hash_walk(wtk_hash_t* h,wtk_walk_handler_t handler,void* user_data);

void wtk_hash_print(wtk_hash_t *h);

//-------------------- iterator section ----------------------
typedef struct
{
	wtk_hash_t *hash;
	wtk_queue_node_t *cur_n;
	int next_index;
}wtk_hash_it_t;

wtk_hash_it_t wtk_hash_iterator(wtk_hash_t *hash);
wtk_hash_node_t* wtk_hash_it_next(wtk_hash_it_t *it);

//--------------------- hash function -------------------------
unsigned int wtk_hash_intptr(int *v,unsigned int nslot);
int wtk_hash_cmp_intptr(int *v1,int *v2);

//--------------------- test/example section -------------------
void wtk_hash_test_g();
#ifdef __cplusplus
};
#endif
#endif
