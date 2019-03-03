#include    "qtk/core/qtk_lockmap.h"

qtk_lockmap_t   *
qtk_lockmap_new(qtk_lockmap_f cmp, qtk_lockmap_del_f del)
{
    qtk_lockmap_t   *map;

    if(!(map = calloc(1, sizeof(*map))))
        goto err;
    if(!(qtk_lockmap_map(map) = qtk_map_new(cmp, del)))
        goto err;
    pthread_spin_init(&map->lock, PTHREAD_PROCESS_SHARED);
    return map;
err:
    if(map)
        qtk_lockmap_delete(map);
    return NULL;
}

int
qtk_lockmap_delete(qtk_lockmap_t *map)
{
    if(qtk_lockmap_map(map))
        qtk_map_delete(qtk_lockmap_map(map));
    pthread_spin_destroy(&map->lock);
    free(map);
    return 0;
}

int
qtk_lockmap_insert(qtk_lockmap_t *map, void *key, void *value)
{
    pthread_spin_lock(&map->lock);
    qtk_map_insert(qtk_lockmap_map(map), key, value);
    pthread_spin_unlock(&map->lock);
    return 0;
}

int
qtk_lockmap_remove(qtk_lockmap_t *map, void *key)
{
    pthread_spin_lock(&map->lock);
    qtk_map_remove(qtk_lockmap_map(map), key);    
    pthread_spin_unlock(&map->lock);
    return 0;
}

void    *
qtk_lockmap_elem(qtk_lockmap_t *map, void *key)
{
    void    *r;

    pthread_spin_lock(&map->lock);
    r = qtk_map_elem(qtk_lockmap_map(map), key);
    pthread_spin_unlock(&map->lock);
    return r;
}

void    *
qtk_lockmap_fold(qtk_lockmap_t *map, void *init, qtk_lockmap_f f)
{
    void    *r;

    pthread_spin_lock(&map->lock);
    r = qtk_map_fold(qtk_lockmap_map(map), init, f);
    pthread_spin_unlock(&map->lock);
    return r;
}

void    *
qtk_lockmap_foldl(qtk_lockmap_t *map, void *init, qtk_lockmap_f f)
{
    void    *r;

    pthread_spin_lock(&map->lock);
    r = qtk_map_foldl(qtk_lockmap_map(map), init, f);
    pthread_spin_unlock(&map->lock);
    return r;
}

void    *
qtk_lockmap_foldr(qtk_lockmap_t *map, void *init, qtk_lockmap_f f)
{
    void    *r;

    pthread_spin_lock(&map->lock);
    r = qtk_map_foldr(qtk_lockmap_map(map), init, f);
    pthread_spin_unlock(&map->lock);
    return r;
}
