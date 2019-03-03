#ifndef QTK_CORE_QTK_BHEAP_H
#define QTK_CORE_QTK_BHEAP_H
#include "wtk/core/wtk_type.h"
#include    <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*qtk_bheap_del_f)(void *);
typedef int (*qtk_bheap_del2_f)(void *, void *);
typedef int (*qtk_bheap_f)(void *, void *); /* cmp */

typedef struct qtk_bheap_node qtk_bheap_node_t;
struct qtk_bheap_node{
    uint32_t idx;
    void *data;
};

typedef struct qtk_bheap qtk_bheap_t;
struct qtk_bheap{
    int used;
    int nslot;
    qtk_bheap_f    cmp;
    qtk_bheap_del_f del;
    qtk_bheap_node_t   **slots;
};

static inline qtk_bheap_node_t  *
qtk_bheap_head(qtk_bheap_t *bp)
{
    return bp->used ? bp->slots[1] : NULL;
}

int qtk_bheap_delete(qtk_bheap_t *bp);
int qtk_bheap_delete2(qtk_bheap_t *bp, qtk_bheap_del2_f del, void *udata);
qtk_bheap_t *qtk_bheap_new(int init_size, qtk_bheap_f cmp, qtk_bheap_del_f del);

void* qtk_bheap_insert_node(qtk_bheap_t *bp, qtk_bheap_node_t *data);
void* qtk_bheap_insert(qtk_bheap_t *bp, void *data);
int qtk_bheap_remove_node(qtk_bheap_t *bp, qtk_bheap_node_t *n);
int qtk_bheap_remove(qtk_bheap_t *bp, qtk_bheap_node_t *n);
int qtk_bheap_increase(qtk_bheap_t *bp, qtk_bheap_node_t *n);
int qtk_bheap_decrease(qtk_bheap_t *bp, qtk_bheap_node_t *n);

void *qtk_bheap_pop(qtk_bheap_t *bp);

void qtk_bheap_del_node(qtk_bheap_t *bp, qtk_bheap_node_t *n);

#ifdef __cplusplus
};
#endif

#endif
