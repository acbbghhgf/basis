#include "qtk_http_hdr.h"


qtk_http_hdr_t* qtk_httphdr_new() {
	qtk_http_hdr_t* hdr;

	hdr = (qtk_http_hdr_t*)wtk_calloc(1,sizeof(*hdr));
	hdr->heap = wtk_heap_new(1024);
    wtk_queue_init(&hdr->head_q);

    return hdr;
}


int qtk_httphdr_delete(qtk_http_hdr_t *hdr) {
	wtk_heap_delete(hdr->heap);
	wtk_free(hdr);
	return 0;
}


int qtk_httphdr_init(qtk_http_hdr_t *hdr) {
	return 0;
}


int qtk_httphdr_reset(qtk_http_hdr_t *hdr) {
	wtk_heap_reset(hdr->heap);
	return 0;
}


void qtk_httphdr_set_string(qtk_http_hdr_t *hdr, wtk_string_t *str, const char *data,int bytes) {
	str->data = wtk_heap_dup_data(hdr->heap,data,bytes);
	str->len = bytes;
}


int qtk_httphdr_add(qtk_http_hdr_t *hdr, const char *key, int klen,
                     const char *value, int vlen) {
    if (wtk_str_equal_s(key, klen, "content-length")) {
        hdr->content_length = wtk_str_atoi((char*)value, vlen);
    } else {
        /* not necessary for key dup when saving key */
        qtk_http_hdr_item_t *item;
        item = wtk_heap_malloc(hdr->heap, sizeof(*item));
        wtk_queue_node_init(&item->node);
        wtk_string_set(&item->key, (char*)key, klen);
        qtk_httphdr_set_string(hdr, &item->value, value, vlen);
        wtk_queue_push(&hdr->head_q, &item->node);
    }
    return 0;
}


wtk_string_t* qtk_httphdr_find_hdr(qtk_http_hdr_t *hdr, const char *key, int klen) {
    wtk_queue_node_t *node;
    for (node = hdr->head_q.pop; node; node=node->next) {
        qtk_http_hdr_item_t *item;
        item = data_offset2(node, qtk_http_hdr_item_t, node);
        if (wtk_str_equal(item->key.data, item->key.len, key, klen)) {
            return &item->value;
        }
    }
    return NULL;
}


void qtk_httphdr_set_repstaus(qtk_http_hdr_t *hdr, const char *buf, int len) {
    hdr->rep.status = wtk_str_atoi((char*)buf, len);
}
