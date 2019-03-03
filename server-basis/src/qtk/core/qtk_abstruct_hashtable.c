#include "qtk_abstruct_hashtable.h"


qtk_hashnode_t* qtk_hashtable_find_node(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key, wtk_queue_t **rv);


qtk_abstruct_hashtable_t* qtk_abstruct_hashtable_new(int nslot, int offset) {
    qtk_abstruct_hashtable_t *h;
    h=(qtk_abstruct_hashtable_t*)wtk_malloc(sizeof(*h));
    h->nslot = nslot;
    h->offset = offset;
    h->nelem = 0;
    h->heap = wtk_heap_new(4096);
    h->slot = (wtk_queue_t**)wtk_calloc(nslot,sizeof(wtk_queue_t*));
    h->find_node_f = qtk_hashtable_find_node;
    return h;
}

int qtk_hashtable_bytes(qtk_abstruct_hashtable_t *h) {
    int v;
    v = sizeof(*h) + h->nslot * sizeof(wtk_queue_t*);
    v += wtk_heap_bytes(h->heap);
    return v;
}


int qtk_hashtable_delete(qtk_abstruct_hashtable_t *h) {
    wtk_heap_delete(h->heap);
    wtk_free(h->slot);
    wtk_free(h);
    return 0;
}


int qtk_hashtable_delete2(qtk_abstruct_hashtable_t *h, wtk_walk_handler_t cleaner, void *udata) {
    int i;
    void *node;
    int offset = h->offset + offsetof(qtk_hashnode_t, n);

    for (i = 0; i < h->nslot; ++i) {
        if (h->slot[i]) {
            while ((node = wtk_queue_pop(h->slot[i]))) {
                cleaner(udata, node-offset);
            }
        }
    }
    qtk_hashtable_delete(h);
    return 0;
}


int qtk_hashtable_reset(qtk_abstruct_hashtable_t *h) {
    wtk_heap_reset(h->heap);
    memset(h->slot, 0, sizeof(wtk_queue_t*) * h->nslot);
    return 0;
}


int qtk_hashtable_add(qtk_abstruct_hashtable_t *h, void *value) {
    qtk_hashnode_t *n;
    int ret;
    wtk_queue_t **slot;

    n = (qtk_hashnode_t*)(value + h->offset);
    slot = h->find_slot_f(h, &n->key);
    if (NULL == *slot) {
        *slot = (wtk_queue_t*)wtk_heap_malloc(h->heap, sizeof(wtk_queue_t));
        wtk_queue_init(*slot);
    }
    ret = wtk_queue_push(*slot, &n->n);
    if (0 == ret) {
        ++h->nelem;
    }
    return ret;
}


qtk_hashnode_t* qtk_hashtable_find_node(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key,
                                   wtk_queue_t **rv) {
    wtk_queue_node_t *qn;
    qtk_hashnode_t *n, *r = NULL;
    wtk_queue_t **slot;
    wtk_queue_t *q;
    slot = h->find_slot_f(h, key);
    q = *slot;
    if (rv) {
        *rv = q;
    }
    if (NULL == q) {
        goto end;
    }
    for (qn = q->pop; qn; qn = qn->next) {
        n = (qtk_hashnode_t*)data_offset2(qn, qtk_hashnode_t, n);
        if (!h->cmp_f(key, &n->key)) {
            r = n;
            break;
        }
    }
end:
    return r;
}


void* qtk_hashtable_find(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key) {
    void *n;

    n = h->find_node_f(h, key, NULL);
    return n ? (n - h->offset) : 0;
}


void* qtk_hashtable_remove(qtk_abstruct_hashtable_t *h, qtk_hashable_t *key) {
    qtk_hashnode_t *n;
    wtk_queue_t *slot;

    n = h->find_node_f(h, key, &slot);
    if (!n) {
        return n;
    }
    wtk_queue_remove(slot, &n->n);
    --h->nelem;

    return n? (((void*)n) - h->offset) : 0;
}


int qtk_hashtable_walk(qtk_abstruct_hashtable_t* h,wtk_walk_handler_t handler,void* user_data) {
    int i, ret;
    ret = 0;

    for (i = 0; i < h->nslot; ++i) {
        if (h->slot[i]) {
            ret = wtk_queue_walk(h->slot[i],
                                 h->offset + offsetof(qtk_hashnode_t, n),
                                 handler,
                                 user_data);
            if (0 != ret) goto end;
        }
    }
end:
    return ret;
}


double qtk_hashtable_radio(qtk_abstruct_hashtable_t* h) {
    int sum = 0;
    int len = 0;
    int i;
    for (i = 0; i < h->nslot; ++i) {
        if (h->slot[i] && h->slot[i]->length) {
            sum += h->slot[i]->length;
            len++;
        }
    }
    if (len) {
        return (double)sum/len;
    } else {
        return 0.0;
    }
}
