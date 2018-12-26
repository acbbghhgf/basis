#ifndef WTK_CORE_WTK_STR_HASH_H_
#define WTK_CORE_WTK_STR_HASH_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include "wtk_queue.h"
#include "wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_str_hash wtk_str_hash_t;
#define wtk_str_hash_find_s(h,key) wtk_str_hash_find(h,key,sizeof(key)-1)
#define wtk_str_hash_remove_s(h,key) wtk_str_hash_remove(h,key,sizeof(key)-1)
#define wtk_str_hash_index(h,k,kb)  (hash_string_value_len(k,kb,(h)->nslot))
#define wtk_str_hash_add_s(h,k,v) wtk_str_hash_add(h,k,sizeof(k)-1,v)
#define wtk_str_hash_find_queue_s(h,k) wtk_str_hash_find_queue(h,k,sizeof(k)-1)
#define wtk_str_hash_find_node3_s(h,k,insert) wtk_str_hash_find_node3(h,k,sizeof(k)-1,insert)


typedef struct hash_str_node
{
	wtk_queue_node_t n;
	wtk_string_t key;
	void* value;
}hash_str_node_t;

typedef struct
{
	wtk_queue_node_t n;
	wtk_string_t key;
	union
	{
		int i;
		unsigned int u;
		float f;
		void *value;
	}v;
}wtk_hash_str_node_t;

struct wtk_str_hash
{
	wtk_heap_t *heap;
	wtk_queue_t **slot;
	int nslot;
	unsigned use_ref_heap:1;
};

/**
 * @brief create string hash;
 */
wtk_str_hash_t* wtk_str_hash_new(int nslot);

wtk_str_hash_t* wtk_str_hash_new2(int nslot,wtk_heap_t *heap);

/**
 * @brief memory occupied by hash;
 */
int wtk_str_hash_bytes(wtk_str_hash_t *h);

/**
 * @brief delete string hash;
 */
int wtk_str_hash_delete(wtk_str_hash_t *h);

/**
 * @brief reset string hash;
 */
int wtk_str_hash_reset(wtk_str_hash_t *h);

/**
 * @brief add k,v,str hash will not deep copy key, just point to memory of key;
 */
int wtk_str_hash_add(wtk_str_hash_t *h,char* key,int key_bytes,void *value);

/**
 * @brief add k,v,str hash will  copy key
 */
int wtk_str_hash_add2(wtk_str_hash_t *h,char* key,int key_bytes,void *value);

/**
 * @brief add k,v and the node of hash use the provided node n;
 */
int wtk_str_hash_add_node(wtk_str_hash_t *h,char* key,int key_bytes,void *value,hash_str_node_t* n);

/**
 * @brief find hash node bye key,and rv is the index of hash slot;
 */
hash_str_node_t* wtk_str_hash_find_node(wtk_str_hash_t *h, char* key,int key_bytes,uint32_t *rv);

hash_str_node_t* wtk_str_hash_find_node2(wtk_str_hash_t *h, char* key,int key_bytes,uint32_t index);

/**
 *	find node if not exist create new;
 */
hash_str_node_t* wtk_str_hash_find_node3(wtk_str_hash_t *h,char *k,int k_bytes,int insert);

/**
 * @brief find node which key start with key
 */
hash_str_node_t* wtk_str_hash_find_node_pre_key(wtk_str_hash_t *h, char* key,int key_bytes);

/**
 * @brief find v by k;
 */
void* wtk_str_hash_find(wtk_str_hash_t *h, char* key,int key_bytes);

/**
 * @brief remove hash node by key;
 */
hash_str_node_t* wtk_str_hash_remove(wtk_str_hash_t *h,char *key,int key_bytes);

/**
 * @brief malloc some memory from heap of hash;
 */
void* wtk_str_hash_malloc(wtk_str_hash_t *h,int bytes);

/**
 * @brief elem of hash table;
 */
int wtk_str_hash_elems(wtk_str_hash_t *h);

/*-------------------------------------------------------------------------*/
/**
 *  in handler(void* user_data,void* data),data is an pointer to hash_str_node_t.
 */
/*--------------------------------------------------------------------------*/
int wtk_str_hash_walk(wtk_str_hash_t* h,wtk_walk_handler_t handler,void* user_data);


/**
 *	cmp handler(void* user_data, void *v)
 */
int wtk_str_hash_findc(wtk_str_hash_t*h,char* k,int kb,wtk_cmp_handler_t cmp,void *user_data,void** v);

/**
 * @brief find same have val by key;
 */
wtk_queue_t* wtk_str_hash_find_queue(wtk_str_hash_t *h,char *k,int k_bytes);


//------------------------ iterator section -----------------------
typedef struct
{
	wtk_str_hash_t *hash;
	wtk_queue_node_t *cur_n;
	int next_index;
}wtk_str_hash_it_t;

wtk_str_hash_it_t wtk_str_hash_iterator(wtk_str_hash_t *hash);
hash_str_node_t* wtk_str_hash_it_next(wtk_str_hash_it_t *it);

//----------------------- new file ------------------------
wtk_str_hash_t* wtk_str_hash_new_file(char *fn);
//-------------------------- test/example section ------------------
void wtk_str_hash_test_g();
#ifdef __cplusplus
};
#endif
#endif
