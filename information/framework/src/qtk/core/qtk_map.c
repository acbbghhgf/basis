#include    "qtk/core/qtk_map.h"

static int  qtk_map_clean(void *node, void *init);

qtk_map_t   *
qtk_map_new(qtk_map_f cmp, qtk_map_del_f del)
{
    qtk_map_t   *map;

    if(!(map = calloc(1, sizeof(*map))))
        goto err;
    if(!(map->t = qtk_Rbtree_new((qtk_rbtree_node_cmp_f)cmp)))
        goto err;
    map->del = del;
    return map;
err:
    if(map)
        qtk_map_delete(map);
    return NULL;
}

int
qtk_map_delete(qtk_map_t *map)
{
    if(map->t){
        qtk_map_fold(map, map, qtk_map_clean);
        qtk_Rbtree_delete(map->t);
    }        
    free(map);
    return 0;
}

int
qtk_map_remove(qtk_map_t *map, void *key)
{
    qtk_map_node_t  *n, tm;
    qtk_rbtree_node_t   *node;

    tm.key = key;
    if((node = qtk_Rbtree_remove(map->t, &tm.q_n))){
        n = qtk_data_offset2(node, qtk_map_node_t, q_n);
        map->del(n->key, n->value);
        free(n);
        return 0;
    }
    return -1;
}

int
qtk_map_insert(qtk_map_t *map, void *key, void *value)
{
    qtk_map_node_t  *n;

    if(!(n = calloc(1, sizeof*n)))
        return -1;
    n->key = key;
    n->value = value;
    return qtk_Rbtree_insert(map->t, &n->q_n);
}

void    *
qtk_map_elem(qtk_map_t *map, void *key)
{
    qtk_map_node_t  tm, *n;
    qtk_Rbtree_node_t   *node;

    tm.key = key;
    if((node = qtk_Rbtree_elem(map->t, &tm.q_n))){
        n = qtk_data_offset2(node, qtk_map_node_t, q_n);
        return n->value;
    }
    return NULL;
}

void    *
qtk_map_fold(qtk_map_t *map, void *init, qtk_map_f f)
{
    return qtk_Rbtree_fold(map->t, init, (qtk_Rbtree_f)f);
}

void    *
qtk_map_foldl(qtk_map_t *map, void *init, qtk_map_f f)
{
    return qtk_Rbtree_foldl(map->t, init, (qtk_Rbtree_f)f);
}

void    *
qtk_map_foldr(qtk_map_t *map, void *init, qtk_map_f f)
{
    return qtk_Rbtree_foldr(map->t, init, (qtk_Rbtree_f)f);
}

static int 
qtk_map_clean(void *node, void *init)
{
    qtk_map_t   *map;
    qtk_map_node_t  *n;

    map = (qtk_map_t *)init;
    n = qtk_data_offset2(node, qtk_map_node_t, q_n);
    map->del(n->key, n->value);
    free(n);
    return 0;
}
