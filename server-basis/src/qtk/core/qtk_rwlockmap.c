#include    "qtk/core/qtk_rwlockmap.h"

qtk_rwlockmap_t   *
qtk_rwlockmap_new(qtk_rwlockmap_f cmp, qtk_rwlockmap_del_f del)
{
    qtk_rwlockmap_t   *map;

    if(!(map = calloc(1, sizeof(*map))))
        goto err;
    if(!(qtk_rwlockmap_map(map) = qtk_map_new(cmp, del)))
        goto err;
    pthread_rwlock_init(&map->rwlock, NULL);
    return map;
err:
    if(map)
        qtk_rwlockmap_delete(map);
    return NULL;
}

int
qtk_rwlockmap_delete(qtk_rwlockmap_t *map)
{
    if(qtk_rwlockmap_map(map))
        qtk_map_delete(qtk_rwlockmap_map(map));
    pthread_rwlock_destroy(&map->rwlock);
    free(map);
    return 0;
}

int
qtk_rwlockmap_rdinsert(qtk_rwlockmap_t *map, void *key, void *value)
{
    pthread_rwlock_rdlock(&map->rwlock);
    qtk_map_insert(qtk_rwlockmap_map(map), key, value);
    pthread_rwlock_unlock(&map->rwlock);
    return 0;
}

int
qtk_rwlockmap_wrinsert(qtk_rwlockmap_t *map, void *key, void *value)
{
    pthread_rwlock_wrlock(&map->rwlock);
    qtk_map_insert(qtk_rwlockmap_map(map), key, value);
    pthread_rwlock_unlock(&map->rwlock);
    return 0;
}

int
qtk_rwlockmap_rdremove(qtk_rwlockmap_t *map, void *key)
{
    pthread_rwlock_rdlock(&map->rwlock);
    qtk_map_remove(qtk_rwlockmap_map(map), key);    
    pthread_rwlock_unlock(&map->rwlock);
    return 0;
}

int
qtk_rwlockmap_wrremove(qtk_rwlockmap_t *map, void *key)
{
    pthread_rwlock_wrlock(&map->rwlock);
    qtk_map_remove(qtk_rwlockmap_map(map), key);    
    pthread_rwlock_unlock(&map->rwlock);
    return 0;
}

void    *
qtk_rwlockmap_rdelem(qtk_rwlockmap_t *map, void *key)
{
    void    *r;

    pthread_rwlock_rdlock(&map->rwlock);
    r = qtk_map_elem(qtk_rwlockmap_map(map), key);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_wrelem(qtk_rwlockmap_t *map, void *key)
{
    void    *r;

    pthread_rwlock_wrlock(&map->rwlock);
    r = qtk_map_elem(qtk_rwlockmap_map(map), key);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}


void    *
qtk_rwlockmap_rdfold(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_rdlock(&map->rwlock);
    r = qtk_map_fold(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_wrfold(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_wrlock(&map->rwlock);
    r = qtk_map_fold(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_rdfoldl(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_rdlock(&map->rwlock);
    r = qtk_map_foldl(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_wrfoldl(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_wrlock(&map->rwlock);
    r = qtk_map_foldl(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_rdfoldr(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_rdlock(&map->rwlock);
    r = qtk_map_foldr(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}

void    *
qtk_rwlockmap_wrfoldr(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f)
{
    void    *r;

    pthread_rwlock_wrlock(&map->rwlock);
    r = qtk_map_foldr(qtk_rwlockmap_map(map), init, f);
    pthread_rwlock_unlock(&map->rwlock);
    return r;
}
