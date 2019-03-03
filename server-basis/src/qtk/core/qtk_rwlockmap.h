#ifndef QTK_CORE_QTK_RWLOCKMAP_H
#define QTK_CORE_QTK_RWLOCKMAP_H

#include    "qtk_map.h"
#include    <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define qtk_rwlockmap_map(xxxx)  ((xxxx)->map) 

typedef qtk_map_f qtk_rwlockmap_f;
typedef qtk_map_del_f qtk_rwlockmap_del_f;
typedef struct qtk_rwlockmap qtk_rwlockmap_t; 
struct qtk_rwlockmap{
    qtk_map_t   *map;
    pthread_rwlock_t    rwlock;
};

int qtk_rwlockmap_delete(qtk_rwlockmap_t *map);
qtk_rwlockmap_t   *qtk_rwlockmap_new(qtk_rwlockmap_f cmp, qtk_rwlockmap_del_f del);

int qtk_rwlockmap_rdremove(qtk_rwlockmap_t *map, void *key);
int qtk_rwlockmap_wrremove(qtk_rwlockmap_t *map, void *key);

int qtk_rwlockmap_rdinsert(qtk_rwlockmap_t *map, void *key, void *value);
int qtk_rwlockmap_wrinsert(qtk_rwlockmap_t *map, void *key, void *value);

void    *qtk_rwlockmap_rdelem(qtk_rwlockmap_t *map, void *key);
void    *qtk_rwlockmap_wrelem(qtk_rwlockmap_t *map, void *key);

void    *qtk_rwlockmap_rdfold(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);
void    *qtk_rwlockmap_rdfoldl(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);
void    *qtk_rwlockmap_rdfoldr(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);

void    *qtk_rwlockmap_wrfold(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);
void    *qtk_rwlockmap_wrfoldl(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);
void    *qtk_rwlockmap_wrfoldr(qtk_rwlockmap_t *map, void *init, qtk_rwlockmap_f f);

#ifdef __cplusplus
};
#endif

#endif
