#ifndef QTK_CORE_QTK_LOCKBHEAP_H
#define QTK_CORE_QTK_LOCKBHEAP_H

#include    <pthread.h>
#include    "qtk_bheap.h"

#ifdef __cplusplus
extern "C" {
#endif

#define qtk_lockbheap_bheap(xxxx) ((xxxx)->bheap)

typedef qtk_bheap_f qtk_lockbheap_f;
typedef qtk_bheap_del_f qtk_lockbheap_del_f;

typedef qtk_bheap_node_t qtk_lockbheap_node_t;
typedef struct qtk_lockbheap qtk_lockbheap_t;
struct qtk_lockbheap{
    qtk_bheap_t *bheap;
    pthread_spinlock_t lock;
};

int qtk_lockbheap_delete(qtk_lockbheap_t *bp);
qtk_lockbheap_t *qtk_lockbheap_new(int init_size, qtk_lockbheap_f cmp, qtk_lockbheap_del_f del);

qtk_lockbheap_node_t* qtk_lockbheap_insert(qtk_lockbheap_t *bp, void *data);
int qtk_lockbheap_remove(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n);
int qtk_lockbheap_increase(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n);
int qtk_lockbheap_decrease(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n);

void    *qtk_lockbheap_pop(qtk_lockbheap_t *bp);
qtk_lockbheap_node_t    *qtk_lockbheap_head(qtk_lockbheap_t *bp);

#ifdef __cplusplus
};
#endif

#endif
