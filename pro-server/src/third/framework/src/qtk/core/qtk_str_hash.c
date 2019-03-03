#include "qtk/core/qtk_str_hash.h"


qtk_str_hash_t* qtk_str_hash_new(int nslot, int offset)
{
    qtk_str_hash_t *h;

    h=(qtk_str_hash_t*)wtk_malloc(sizeof(*h));
    h->nslot = nslot;
    h->offset = offset;
    h->nelem = 0;
    h->heap = wtk_heap_new(4096);
    h->slot = (wtk_queue_t**)wtk_calloc(nslot,sizeof(wtk_queue_t*));
    return h;
}


int qtk_str_hash_bytes(qtk_str_hash_t *h)
{
    int v;

    v = sizeof(*h)+h->nslot*sizeof(wtk_queue_t*);
    v += wtk_heap_bytes(h->heap);
    return v;
}


int qtk_str_hash_delete(qtk_str_hash_t *h)
{
    wtk_heap_delete(h->heap);
    wtk_free(h->slot);
    wtk_free(h);
    return 0;
}


int qtk_str_hash_reset(qtk_str_hash_t *h)
{
    wtk_heap_reset(h->heap);
    memset(h->slot,0,sizeof(wtk_queue_t*) * h->nslot);
    return 0;
}


int qtk_str_hash_add(qtk_str_hash_t *h, void *value)
{
    qtk_hash_str_node_t *n;
    int ret;
    int index;

    n = (qtk_hash_str_node_t*)(value + h->offset);
    index = hash_string_value_len(n->key->data, n->key->len, h->nslot);
    if (!h->slot[index]) {
        h->slot[index] = (wtk_queue_t*)wtk_heap_malloc(h->heap, sizeof(wtk_queue_t));
        wtk_queue_init(h->slot[index]);
    }
    ret = wtk_queue_push(h->slot[index], &(n->n));
    if (0 == ret) {
        ++h->nelem;
    }
    return ret;
}


wtk_queue_t* qtk_str_hash_find_queue(qtk_str_hash_t *h,char *k,int k_bytes)
{
    uint32_t index;

    index = hash_string_value_len(k,k_bytes,h->nslot);
    return h->slot[index];
}

qtk_hash_str_node_t* qtk_str_hash_find_node(qtk_str_hash_t *h, char* key,int key_bytes,uint32_t *rv)
{
    wtk_queue_node_t *qn;
    qtk_hash_str_node_t *n,*r;
    uint32_t index;
    wtk_queue_t *q;

    r = 0;
    index = hash_string_value_len(key,key_bytes,h->nslot);
    if (rv) { *rv = index; }
    q = h->slot[index];
    if (!q) {
        //wtk_debug("empty slot: %s not found\n",key);
        goto end;
    }
    for (qn = q->pop; qn; qn = qn->next) {
        n = (qtk_hash_str_node_t*)data_offset2(qn,qtk_hash_str_node_t,n);
        if ((n->key->len==key_bytes) && (memcmp(key,n->key->data,key_bytes)==0)) {
            r = n;
            break;
        }
    }
end:
    return r;
}


void* qtk_str_hash_find(qtk_str_hash_t *h, char* key,int key_bytes)
{
    void *n;

    n = qtk_str_hash_find_node(h, key, key_bytes, NULL);
    return n ? (n - h->offset) : 0;
}


void* qtk_str_hash_remove(qtk_str_hash_t *h,char *key,int key_bytes)
{
    qtk_hash_str_node_t *n;
    uint32_t index;

    n = qtk_str_hash_find_node(h, key, key_bytes, &index);
    if (!n) { return n; }
    wtk_queue_remove(h->slot[index], &(n->n));
    --h->nelem;

    return n ? (((void*)n) - h->offset) : 0;
}


int qtk_str_hash_walk(qtk_str_hash_t* h, wtk_walk_handler_t handler, void* user_data)
{
    int i,ret;
    ret = 0;

    for (i = 0; i < h->nslot; ++i) {
        if (h->slot[i]) {
            ret = wtk_queue_walk(h->slot[i],
                                 h->offset + offsetof(qtk_hash_str_node_t, n),
                                 handler,
                                 user_data);
            if (ret != 0) { goto end; }
        }
    }
end:
    return ret;
}


//------------------------ iterator section -----------------------
void qtk_str_hash_it_move(qtk_str_hash_it_t *it)
{
    qtk_str_hash_t *hash = it->hash;
    int i;

    for(i = it->next_index; i < hash->nslot; ++i) {
        if(hash->slot[i] && hash->slot[i]->length > 0) {
            it->next_index = i + 1;
            it->cur_n = hash->slot[i]->pop;
            break;
        }
    }
}


qtk_str_hash_it_t qtk_str_hash_iterator(qtk_str_hash_t *hash)
{
    qtk_str_hash_it_t it;

    it.hash = hash;
    it.next_index = 0;
    it.cur_n = 0;
    qtk_str_hash_it_move(&(it));
    return it;
}


qtk_hash_str_node_t* qtk_str_hash_it_next(qtk_str_hash_it_t *it)
{
    wtk_queue_node_t *q_n;
    qtk_hash_str_node_t *hash_n;

    q_n = it->cur_n;
    if (q_n) {
        if (q_n->next) {
            it->cur_n = q_n->next;
        } else {
            it->cur_n = 0;
            qtk_str_hash_it_move(it);
        }
    }
    if (q_n) {
        hash_n = data_offset(q_n, qtk_hash_str_node_t, n);
    } else {
        hash_n = 0;
    }
    return hash_n;
}


//-------------------------- test/example section ------------------

typedef struct qtk_hash_test {
    qtk_hash_str_node_t node;
    char *value;
} qtk_hash_test_t;
void qtk_str_hash_test_g()
{
    qtk_str_hash_t *hash;
    wtk_queue_node_t *qn;
    qtk_hash_str_node_t *hash_n;
    qtk_hash_test_t *value;
    int i;

    qtk_hash_test_t a = {{{NULL}, wtk_string_dup("first")}, "a"};
    qtk_hash_test_t b = {{{NULL}, wtk_string_dup("second")}, "b"};
    qtk_hash_test_t c = {{{NULL}, wtk_string_dup("third")}, "c"};
    hash = qtk_str_hash_new(13, offsetof(qtk_hash_test_t, node));
    qtk_str_hash_add(hash, &a);
    qtk_str_hash_add(hash, &b);
    qtk_str_hash_add(hash, &c);
    for(i=0;i<hash->nslot;++i)
    {
        if(!hash->slot[i]){continue;}
        for(qn=hash->slot[i]->pop;qn;qn=qn->next)
        {
            hash_n = data_offset(qn,qtk_hash_str_node_t,n);
            value = ((void*)hash_n) + hash->offset;
            wtk_debug("index=%d: [%.*s]=[%s]\n",i,hash_n->key->len,hash_n->key->data,(char*)value->value);
        }
    }
    value = qtk_str_hash_find_s(hash, "first");
    wtk_debug("find [first]: %s\r\n", value->value);
    value = qtk_str_hash_find_s(hash, "second");
    wtk_debug("find [second]: %s\r\n", value->value);
    value = qtk_str_hash_find_s(hash, "third");
    wtk_debug("find [third]: %s\r\n", value->value);

    value = qtk_str_hash_remove_s(hash, "first");
    wtk_debug("remove [first]: %s\r\n", value->value);
    value = qtk_str_hash_remove_s(hash, "second");
    wtk_debug("remove [second]: %s\r\n", value->value);
    value = qtk_str_hash_remove_s(hash, "third");
    wtk_debug("remove [third]: %s\r\n", value->value);

    for(i=0;i<hash->nslot;++i)
    {
        if(!hash->slot[i]){continue;}
        for(qn=hash->slot[i]->pop;qn;qn=qn->next)
        {
            hash_n = data_offset(qn,qtk_hash_str_node_t,n);
            value = ((void*)hash_n) + hash->offset;
            wtk_debug("index=%d: [%.*s]=[%s]\n",i,hash_n->key->len,hash_n->key->data,(char*)value->value);
        }
    }
}
