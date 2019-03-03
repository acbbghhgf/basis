#ifndef _QTK_UTIL_QTK_ABSTRUCT_HASHTABLE_H
#define _QTK_UTIL_QTK_ABSTRUCT_HASHTABLE_H
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define empty_queue_node {NULL, NULL}
#define qtk_hash_node(s) {.key._str = wtk_string(s), .n = empty_queue_node}

typedef struct qtk_hashable qtk_hashable_t;
typedef struct qtk_hashnode qtk_hashnode_t;
typedef struct qtk_abstruct_hashtable qtk_abstruct_hashtable_t;
typedef int (*qtk_hashtable_f)(qtk_abstruct_hashtable_t *h);
typedef int (*qtk_hashtable_key_f)(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key);
typedef int (*qtk_hashtable_value_f)(qtk_abstruct_hashtable_t *h, void *value);
typedef qtk_hashnode_t* (*qtk_hashtable_find_node_f)(qtk_abstruct_hashtable_t *h,
                                         qtk_hashable_t *key, wtk_queue_t **slot);
typedef wtk_queue_t** (*qtk_hashtable_find_slot_f)(qtk_abstruct_hashtable_t *h,
                                                   qtk_hashable_t *key);
typedef uint32_t (*qtk_hash_f)(qtk_hashable_t *key);
typedef int (*qtk_cmp_f)(qtk_hashable_t *k1, qtk_hashable_t *k2);

struct qtk_hashable {
    union {
        wtk_string_t _str;
        int _number;
    };
};

struct qtk_hashnode {
    wtk_queue_node_t n;
    qtk_hashable_t key;
};

struct qtk_abstruct_hashtable {
    wtk_heap_t *heap;
    wtk_queue_t **slot;
    int nslot;
    int offset;
    int nelem;
    qtk_hashtable_find_node_f find_node_f;
    qtk_hashtable_find_slot_f find_slot_f;
    qtk_hash_f hash_f;
    qtk_cmp_f cmp_f;
};

/**
 * @brief create string hash;
 */
qtk_abstruct_hashtable_t* qtk_abstruct_hashtable_new(int nslot, int offset);

/**
 * @brief memory occupied by hash;
 */
int qtk_hashtable_bytes(qtk_abstruct_hashtable_t *h);

/**
 * @brief delete string hash;
 */
int qtk_hashtable_delete(qtk_abstruct_hashtable_t *h);

/**
 * @brief delete string hash and apply cleaner to all elements;
 */
int qtk_hashtable_delete2(qtk_abstruct_hashtable_t *h, wtk_walk_handler_t cleaner, void *udata);

/**
 * @brief reset string hash;
 */
int qtk_hashtable_reset(qtk_abstruct_hashtable_t *h);

/**
 * @brief add k,v
 */
int qtk_hashtable_add(qtk_abstruct_hashtable_t *h, void *value);

/**
 * @brief find hash node bye key,and rv is the index of hash slot;
 */
qtk_hashnode_t* qtk_hashtable_find_node(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key, wtk_queue_t **rv);

/**
 * @brief find v by k;
 */
void* qtk_hashtable_find(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key);

/**
 * @brief remove hash node by key;
 */
void* qtk_hashtable_remove(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key);


/*-------------------------------------------------------------------------*/
/**
 *  in handler(void* user_data,void* data),data is an pointer to hash_str_node_t.
 */
/*--------------------------------------------------------------------------*/
int qtk_hashtable_walk(qtk_abstruct_hashtable_t* h,wtk_walk_handler_t handler,void* user_data);


double qtk_hashtable_radio(qtk_abstruct_hashtable_t* h);


#ifdef __cplusplus
}
#endif

#endif
