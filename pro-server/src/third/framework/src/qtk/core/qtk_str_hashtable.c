#include "qtk_str_hashtable.h"


static uint32_t qtk_strhash_hash(wtk_string_t *key) {
    uint32_t hashval=0;
    unsigned char *p;
    unsigned char *e;

    p = (unsigned char*)key->data;
    e = p + key->len;
    while (p < e) {
        hashval = (hashval*131) + (*p++);
    }
    return hashval & 0x7FFFFFFF;
}


wtk_queue_t** qtk_str_hash_findslot(qtk_abstruct_hashtable_t *h, wtk_string_t *key) {
    uint32_t index = h->hash_f((qtk_hashable_t*)key) % h->nslot;
    return h->slot + index;
}


qtk_abstruct_hashtable_t* qtk_str_hashtable_new(int nslot, int offset) {
    qtk_abstruct_hashtable_t *h;
    h = qtk_abstruct_hashtable_new(nslot, offset);
    h->find_slot_f = (qtk_hashtable_find_slot_f)qtk_str_hash_findslot;
    h->hash_f = (qtk_hash_f)qtk_strhash_hash;
    h->cmp_f = (qtk_cmp_f)wtk_string_cmp2;
    return h;
}


static int qtk_str_growhash_grow(qtk_abstruct_hashtable_t *h) {
    int nslot = h->nslot * 2;
    int nbytes = sizeof(*h->slot) * h->nslot;
    wtk_queue_t **slot = wtk_malloc(nbytes * 2);
    if (slot) {
        memcpy(slot, h->slot, nbytes);
        memset(slot + h->nslot, 0, nbytes);
        wtk_free(h->slot);
        h->slot = slot;
        h->nslot = nslot;
        return 0;
    } else {
        return -1;
    }
}


wtk_queue_t** qtk_str_growhash_findslot(qtk_abstruct_hashtable_t *h, wtk_string_t *key) {
    uint32_t index;
    if (h->nelem >= h->nslot) {
        qtk_str_growhash_grow(h);
    }
    index = h->hash_f((qtk_hashable_t*)key) % h->nslot;
    return h->slot + index;
}


static qtk_hashnode_t* qtk_str_growhash_findnode(qtk_abstruct_hashtable_t *h,
                                                 qtk_hashable_t *key, wtk_queue_t **slot) {
    wtk_queue_node_t *qn;
    qtk_hashnode_t *n, *r;
    wtk_queue_t *q;
    int index, nslot;

    if (h->nelem >= h->nslot) {
        qtk_str_growhash_grow(h);
    }
    index = h->hash_f(key);
    nslot = h->nslot;
    do {
        q = h->slot[index % nslot];
        if (NULL == q) continue;
        for (qn = q->pop; qn; qn = qn->next) {
            n = (qtk_hashnode_t*)data_offset2(qn, qtk_hashnode_t, n);
            if (!h->cmp_f((qtk_hashable_t*)key, &n->key)) {
                wtk_queue_remove(q, qn);
                q = h->slot[index % h->nslot];
                if (NULL == q) {
                    q = (wtk_queue_t*)wtk_heap_malloc(h->heap, sizeof(wtk_queue_t));
                    wtk_queue_init(q);
                    h->slot[index % h->nslot] = q;
                }
                wtk_queue_push(q, qn);
                r = n;
                if (slot) *slot = q;
                goto end;
            }
        }
    } while (!(nslot & 1) && (nslot /= 2));
    r = NULL;
    if (slot) *slot = NULL;
end:
    return r;
}


qtk_abstruct_hashtable_t* qtk_str_growhash_new(int nslot, int offset) {
    qtk_abstruct_hashtable_t *h;
    if (! (nslot & 1)) nslot++;
    h = qtk_abstruct_hashtable_new(nslot, offset);
    h->find_slot_f = (qtk_hashtable_find_slot_f)qtk_str_growhash_findslot;
    h->find_node_f = (qtk_hashtable_find_node_f)qtk_str_growhash_findnode;
    h->hash_f = (qtk_hash_f)qtk_strhash_hash;
    h->cmp_f = (qtk_cmp_f)wtk_string_cmp2;
    return h;
}


typedef struct qtk_hash_test {
    qtk_hashnode_t node;
    int value;
} qtk_hash_test_t;


int qtk_str_hashtable_test() {
    qtk_abstruct_hashtable_t* hash = qtk_str_growhash_new(2, offsetof(qtk_hash_test_t, node));
    wtk_heap_t *heap;
    int i;
    qtk_hash_test_t *p;
    heap = wtk_heap_new(4096);
    for (i = 0; i < 10000; i++) {
        char buf[32], *pbuf;
        wtk_string_t *key;
        int len;
        p = wtk_malloc(sizeof(*p));
        key = (wtk_string_t*)&p->node.key;
        sprintf(buf, "%d", i);
        len = strlen(buf);
        pbuf = wtk_heap_malloc(heap, len);
        memcpy(pbuf, buf, len);
        wtk_string_set(key, pbuf, len);
        p->value = i;
        qtk_hashtable_add(hash, p);
    }
    wtk_debug("radio: %lf\r\n", qtk_hashtable_radio(hash));
    for (i = 0; i < 10000; i++) {
        char buf[32];
        wtk_string_t key;
        sprintf(buf, "%d", i);
        wtk_string_set(&key, buf, strlen(buf));
        p = qtk_hashtable_find(hash, (qtk_hashable_t*)&key);
        if (!p) {
            printf("not found : %s\r\n", buf);
        }
    }
    wtk_debug("radio: %lf\r\n", qtk_hashtable_radio(hash));

    return 0;
}
