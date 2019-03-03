#include    <stdlib.h>
#include    "qtk/core/qtk_bheap.h"


qtk_bheap_node_t* qtk_bheap_new_node(qtk_bheap_t *bp) {
    return wtk_malloc(sizeof(qtk_bheap_node_t));
}


void qtk_bheap_del_node(qtk_bheap_t *bp, qtk_bheap_node_t *n) {
    wtk_free(n);
}


qtk_bheap_t *
qtk_bheap_new(int init_size, qtk_bheap_f cmp, qtk_bheap_del_f del)
{
    qtk_bheap_t *bp;

    if(!cmp)
        return NULL;
    if(!(bp = calloc(1, sizeof(*bp))))
        goto err;
    bp->cmp = cmp;
    bp->del = del;
    bp->nslot = init_size;
    if(!(bp->slots = calloc(bp->nslot + 1, sizeof(qtk_bheap_node_t *))))
        goto err;
    bp->slots[0] = NULL;
    return bp;
err:
    if(bp)
        qtk_bheap_delete(bp);
    return NULL;
}

int qtk_bheap_delete(qtk_bheap_t *bp) {
    return qtk_bheap_delete2(bp, NULL, NULL);
}

int
qtk_bheap_delete2(qtk_bheap_t *bp, qtk_bheap_del2_f del, void *udata)
{
    int i;

    if(bp->slots){
        if (del) {
            for(i = 0; i <= bp->used; i++)
                if (bp->slots[i])
                    del(udata, bp->slots[i]->data);
        } else if (bp->del) {
            for(i = 0; i <= bp->used; i++)
                if (bp->slots[i])
                    bp->del(bp->slots[i]->data);
        }
        for(i = 0; i <= bp->used; i++)
            if (bp->slots[i])
                qtk_bheap_del_node(bp, bp->slots[i]);
        free(bp->slots);
    }
    free(bp);
    return 0;
}


void* qtk_bheap_pop(qtk_bheap_t *bp)
{
    qtk_bheap_node_t    *r;
    void *data = NULL;

    if(!(r = qtk_bheap_head(bp)))
        return NULL;

    data = r->data;
    qtk_bheap_remove(bp, r);
    return data;
}

qtk_bheap_node_t* qtk_bheap_pop_node(qtk_bheap_t *bp) {
    qtk_bheap_node_t    *r;

    if(!(r = qtk_bheap_head(bp)))
        return NULL;

    qtk_bheap_remove_node(bp, r);
    return r;
}

void *qtk_bheap_insert_node(qtk_bheap_t *bp, qtk_bheap_node_t *n) {
    int	i;

    if(bp->used == bp->nslot){
        qtk_bheap_node_t    **sp = bp->slots;
        if(!(bp->slots = realloc(bp->slots, ((bp->nslot *= 2) + 1) * sizeof(qtk_bheap_node_t *)))){
            bp->slots = sp;
            bp->nslot /= 2;
            wtk_free(n);
            return NULL;
        }
    }
    for(i = ++bp->used; i/2 && bp->cmp(bp->slots[i / 2]->data, n->data) > 0; i /= 2){
        bp->slots[i] = bp->slots[i / 2];
        bp->slots[i]->idx = i;
    }
    n->idx = i;
    bp->slots[i] = n;

    return n;
}

void* qtk_bheap_insert(qtk_bheap_t *bp, void *data)
{
    qtk_bheap_node_t *n = qtk_bheap_new_node(bp);
    n->data = data;

	return qtk_bheap_insert_node(bp, n);
}


int qtk_bheap_remove_node(qtk_bheap_t *bp, qtk_bheap_node_t *n) {
	int	i, c;
    qtk_bheap_node_t    *tmp;

	if(bp->used == 0)
		return -1;
    tmp = bp->slots[bp->used--];
    for(i = n->idx; i * 2 <= bp->used; i = c){
        c = i * 2;
        if(c < bp->used && bp->cmp(bp->slots[c + 1]->data, bp->slots[c]->data) < 0)
            c++;
        if(bp->cmp(tmp->data, bp->slots[c]->data) > 0){
            bp->slots[i] = bp->slots[c];
            bp->slots[i]->idx = i;
        }else
            break;
	}
    bp->slots[i] = tmp;
    bp->slots[i]->idx = i;
    return 0;
}

int qtk_bheap_remove(qtk_bheap_t *bp, qtk_bheap_node_t *n)
{
    if (qtk_bheap_remove_node(bp, n) < 0) {
        return -1;
    }
    qtk_bheap_del_node(bp, n);
	return 0;
}

int
qtk_bheap_increase(qtk_bheap_t *bp, qtk_bheap_node_t *n)
{
    int i, c;

    for(i = n->idx; i * 2 <= bp->used; i = c){
        c = i * 2;
        if(c < bp->used && bp->cmp(bp->slots[c + 1]->data, bp->slots[c]->data) < 0)
            c++;
        if(bp->cmp(n->data, bp->slots[c]->data) > 0){
            bp->slots[i] = bp->slots[c];
            bp->slots[i]->idx = i;
        }else
            break;
    }
    n->idx = i;
    bp->slots[i] = n;
    return 0;
}

int
qtk_bheap_decrease(qtk_bheap_t *bp, qtk_bheap_node_t *n)
{
    int i;

    for(i = n->idx; i/2 && bp->cmp(bp->slots[i / 2]->data, n->data) > 0; i /= 2){
        bp->slots[i] = bp->slots[i / 2];
        bp->slots[i]->idx = i;
    }
    n->idx = i;
    bp->slots[i] = n;
    return 0;
}
