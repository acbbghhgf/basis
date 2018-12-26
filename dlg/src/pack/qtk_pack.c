#include "qtk_pack.h"
#include "qtk_pbase.h"
#include "qtk/os/qtk_alloc.h"

qtk_pack_t *qtk_pack_new()
{
    qtk_pack_t *p = qtk_calloc(1, sizeof(*p));
    p->heap = wtk_heap_new(512);
    p->buf = wtk_strbuf_new(128, 1);
    return p;
}

void qtk_pack_delete(qtk_pack_t *p)
{
    if (p->hdr) wtk_str_hash_delete(p->hdr);
    wtk_heap_delete(p->heap);
    wtk_strbuf_delete(p->buf);
    qtk_free(p);
}

int qtk_pack_reset(qtk_pack_t *p)
{
    if (p->hdr) wtk_str_hash_reset(p->hdr);
    wtk_heap_reset(p->heap);
    wtk_strbuf_reset(p->buf);
    p->key = NULL;
    return 0;
}

static int _encode_hdr(qtk_pack_t *p, hash_str_node_t *node)
{
    wtk_strbuf_push(p->buf, node->key.data, node->key.len);
    wtk_strbuf_push_c(p->buf, US);
    wtk_string_t *v = node->value;
    wtk_strbuf_push(p->buf, v->data, v->len);
    wtk_strbuf_push_c(p->buf, RS);
    return 0;
}

int qtk_pack_encode(qtk_pack_t *p)
{
    wtk_strbuf_reset(p->buf);
    if (NULL == p->hdr) return 0;
    wtk_str_hash_walk(p->hdr, (wtk_walk_handler_t)_encode_hdr, p);
    wtk_strbuf_push_c(p->buf, RS);
    return 0;
}

int qtk_pack_add(qtk_pack_t *p, const char *k, size_t kl, const char *v, size_t vl)
{
    wtk_string_t *key = wtk_heap_dup_string(p->heap, (char *)k, kl);
    wtk_string_t *value = wtk_heap_dup_string(p->heap, (char *)v, vl);
    if (!p->hdr) p->hdr = wtk_str_hash_new2(10, p->heap);
    wtk_str_hash_add(p->hdr, key->data, key->len, value);
    return 0;
}

wtk_string_t *qtk_pack_find(qtk_pack_t *p, const char *k, size_t kl)
{
    return (wtk_string_t *)wtk_str_hash_find(p->hdr, (char *)k, kl);
}
