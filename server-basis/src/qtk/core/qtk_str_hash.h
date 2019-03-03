#ifndef QTK_UTIL_QTK_STR_HASH_H_
#define QTK_UTIL_QTK_STR_HASH_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef int (*qtk_hash_walk_handler_t)(void *user_data,void* data);
typedef struct qtk_str_hash qtk_str_hash_t;
#define qtk_str_hash_find_s(h,key) qtk_str_hash_find(h,key,sizeof(key)-1)
#define qtk_str_hash_remove_s(h,key) qtk_str_hash_remove(h,key,sizeof(key)-1)
#define qtk_str_hash_remove_node(h, n) ((n)->key && qtk_str_hash_remove(h, (n)->key->data, (n)->key->len))
#define qtk_str_hash_index(h,k,kb)  (hash_string_value_len(k,kb,(h)->nslot))
#define qtk_str_hash_find_queue_s(h,k) qtk_str_hash_find_queue(h,k,sizeof(k)-1)


typedef struct qtk_hash_str_node
{
    wtk_queue_node_t n;
    wtk_string_t *key;
} qtk_hash_str_node_t;


struct qtk_str_hash
{
    wtk_heap_t *heap;
    wtk_queue_t **slot;
    int nslot;
    int offset;
    int nelem;
};

/**
 * @brief create string hash;
 */
qtk_str_hash_t* qtk_str_hash_new(int nslot, int offset);

/**
 * @brief memory occupied by hash;
 */
int qtk_str_hash_bytes(qtk_str_hash_t *h);

/**
 * @brief delete string hash;
 */
int qtk_str_hash_delete(qtk_str_hash_t *h);

/**
 * @brief reset string hash;
 */
int qtk_str_hash_reset(qtk_str_hash_t *h);

/**
 * @brief add k,v
 */
int qtk_str_hash_add(qtk_str_hash_t *h, void *value);

/**
 * @brief find hash node bye key,and rv is the index of hash slot;
 */
qtk_hash_str_node_t* qtk_str_hash_find_node(qtk_str_hash_t *h, char* key,int key_bytes,uint32_t *rv);

/**
 * @brief find v by k;
 */
void* qtk_str_hash_find(qtk_str_hash_t *h, char* key,int key_bytes);

/**
 * @brief remove hash node by key;
 */
void* qtk_str_hash_remove(qtk_str_hash_t *h,char *key,int key_bytes);


/*-------------------------------------------------------------------------*/
/**
 *  in handler(void* user_data,void* data),data is an pointer to hash_str_node_t.
 */
/*--------------------------------------------------------------------------*/
int qtk_str_hash_walk(qtk_str_hash_t* h,wtk_walk_handler_t handler,void* user_data);


//------------------------ iterator section -----------------------
typedef struct
{
    qtk_str_hash_t *hash;
    wtk_queue_node_t *cur_n;
    int next_index;
}qtk_str_hash_it_t;

qtk_str_hash_it_t qtk_str_hash_iterator(qtk_str_hash_t *hash);
qtk_hash_str_node_t* qtk_str_hash_it_next(qtk_str_hash_it_t *it);

//-------------------------- test/example section ------------------
void qtk_str_hash_test_g();
#ifdef __cplusplus
};
#endif
#endif
