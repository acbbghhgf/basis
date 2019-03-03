#ifndef QTK_CORE_QTK_LOCKMAP_H
#define QTK_CORE_QTK_LOCKMAP_H

#include    "qtk_map.h"
#include    <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define qtk_lockmap_map(xxxx)  ((xxxx)->map) 

typedef qtk_map_f qtk_lockmap_f;
typedef qtk_map_del_f qtk_lockmap_del_f;
typedef struct qtk_lockmap qtk_lockmap_t; 
struct qtk_lockmap{
    qtk_map_t   *map;
    pthread_spinlock_t lock;
};

int qtk_lockmap_delete(qtk_lockmap_t *map);
qtk_lockmap_t   *qtk_lockmap_new(qtk_lockmap_f cmp, qtk_lockmap_del_f del);

int qtk_lockmap_remove(qtk_lockmap_t *map, void *key);
int qtk_lockmap_insert(qtk_lockmap_t *map, void *key, void *value);

void    *qtk_lockmap_elem(qtk_lockmap_t *map, void *key);

void    *qtk_lockmap_fold(qtk_lockmap_t *map, void *init, qtk_lockmap_f f);
void    *qtk_lockmap_foldl(qtk_lockmap_t *map, void *init, qtk_lockmap_f f);
void    *qtk_lockmap_foldr(qtk_lockmap_t *map, void *init, qtk_lockmap_f f);

#ifdef __cplusplus
};
#endif

#endif
