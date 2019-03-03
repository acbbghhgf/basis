#include    <stdlib.h>
#include    "qtk/core/qtk_lockbheap.h"

qtk_lockbheap_t *
qtk_lockbheap_new(int init_size, qtk_lockbheap_f cmp, qtk_lockbheap_del_f del)
{
    qtk_lockbheap_t *bp;

    if(!(bp = calloc(1, sizeof(*bp))))
        goto err;
    if(!(qtk_lockbheap_bheap(bp) = qtk_bheap_new(init_size, cmp, del)))
        goto err;
    pthread_spin_init(&bp->lock, PTHREAD_PROCESS_SHARED);
    return bp;
err:
    if(bp)
        qtk_lockbheap_delete(bp);
    return NULL;
}

int
qtk_lockbheap_delete(qtk_lockbheap_t *bp)
{
    if(qtk_lockbheap_bheap(bp))
        qtk_bheap_delete(qtk_lockbheap_bheap(bp));
    pthread_spin_destroy(&bp->lock);
    free(bp);
    return 0;
}

qtk_lockbheap_node_t  *
qtk_lockbheap_head(qtk_lockbheap_t *bp)
{
    void    *r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_head(qtk_lockbheap_bheap(bp));
    pthread_spin_unlock(&bp->lock);
    return r;
}

void *
qtk_lockbheap_pop(qtk_lockbheap_t *bp)
{
    qtk_lockbheap_node_t    *r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_pop(qtk_lockbheap_bheap(bp));
    pthread_spin_unlock(&bp->lock);
    return r;
}

qtk_lockbheap_node_t*
qtk_lockbheap_insert(qtk_lockbheap_t *bp, void *data)
{
    qtk_lockbheap_node_t *r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_insert(qtk_lockbheap_bheap(bp), data);
    pthread_spin_unlock(&bp->lock);
    return r;
}

int
qtk_lockbheap_remove(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n)
{
    int r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_remove(qtk_lockbheap_bheap(bp), n);
    pthread_spin_unlock(&bp->lock);
	return r;
}

int
qtk_lockbheap_increase(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n)
{
    int r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_increase(qtk_lockbheap_bheap(bp), n);
    pthread_spin_unlock(&bp->lock);
    return r;
}

int
qtk_lockbheap_decrease(qtk_lockbheap_t *bp, qtk_lockbheap_node_t *n)
{
    int r;

    pthread_spin_lock(&bp->lock);
    r = qtk_bheap_decrease(qtk_lockbheap_bheap(bp), n);
    pthread_spin_unlock(&bp->lock);
    return r;
}
