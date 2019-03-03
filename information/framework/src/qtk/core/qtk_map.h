/*
 * traditional
 * ================================================
 * map = {
 *    rbtree
 * }
 */
#ifndef QTK_CORE_QTK_MAP_H
#define QTK_CORE_QTK_MAP_H

#include    "qtk_rbtree.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*qtk_map_f)(void *, void *); /* cmp */
typedef int (*qtk_map_del_f)(void *, void *); /* key; value */

typedef struct qtk_map_node qtk_map_node_t;
struct qtk_map_node{
    void    *key;
    void    *value; 
    qtk_Rbtree_node_t   q_n;
};

typedef struct qtk_map qtk_map_t;
struct qtk_map{
    qtk_Rbtree_t    *t;
    qtk_map_del_f   del;
    pthread_spinlock_t  lock; 
};

static inline void *
qtk_map_get_key(void *node)
{
    qtk_map_node_t  *n;

    n = qtk_data_offset2(node, qtk_map_node_t, q_n);
    return n->key;
}

static inline void *
qtk_map_get_value(void *node)
{
    qtk_map_node_t  *n;

    n = qtk_data_offset2(node, qtk_map_node_t, q_n);
    return n->value;
}

int qtk_map_delete(qtk_map_t *map);
qtk_map_t   *qtk_map_new(qtk_map_f cmp, qtk_map_del_f del);

int qtk_map_remove(qtk_map_t *map, void *key);
int qtk_map_insert(qtk_map_t *map, void *key, void *value);

void    *qtk_map_elem(qtk_map_t *map, void *key);

void    *qtk_map_fold(qtk_map_t *map, void *init, qtk_map_f f);
void    *qtk_map_foldl(qtk_map_t *map, void *init, qtk_map_f f);
void    *qtk_map_foldr(qtk_map_t *map, void *init, qtk_map_f f);

#ifdef __cplusplus
};
#endif

#endif
