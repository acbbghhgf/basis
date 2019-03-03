#include "qtk_response.h"
#include "qtk_request.h"
#include "qtk/entry/qtk_socket.h"
#include "qtk/http/qtk_http_parser.h"


int qtk_response_init(qtk_response_t *rep, int sock_id) {
    rep->status = HTTP_OK;
    rep->content_length = 0;
    rep->sock_id = sock_id;
    rep->request = NULL;
    wtk_queue_init(&rep->hoard_n);
    return 0;
}


qtk_response_t *qtk_response_new() {
    qtk_response_t *rep;
    rep = wtk_calloc(1, sizeof(qtk_response_t));
    rep->heap = wtk_heap_new(2048);
    rep->body = wtk_strbuf_new(1024, 1);
    rep->hash = wtk_str_hash_new2(10, rep->heap);
    rep->buf = NULL;
    rep->heap_ref = 0;
    rep->sock_id = -1;
    return rep;
}


qtk_response_t* qtk_response_new_ref(wtk_heap_t *heap) {
    qtk_response_t *rep;

    rep = wtk_heap_zalloc(heap, sizeof(qtk_response_t));
    rep->heap = heap;
    rep->buf = NULL;
    rep->body = wtk_heap_zalloc(heap, sizeof(wtk_strbuf_t));
    rep->hash = wtk_str_hash_new2(10, rep->heap);
    qtk_response_init(rep, -1);
    rep->heap_ref = 1;
    rep->sock_id = -1;

    return rep;
}


void qtk_response_update_hdr(qtk_response_t *rep) {
    wtk_string_t *v = NULL;

    v = wtk_str_hash_find_s(rep->hash, "content-length");
    if (v) {
        rep->content_length = wtk_str_atoi(v->data, v->len);
    }
}


int qtk_response_delete(qtk_response_t *rep) {
    if (rep->request) {
        qtk_request_delete(rep->request);
    }
    if (rep->buf) {
        wtk_strbuf_delete(rep->buf);
    }
    if (rep->hash) {
        wtk_str_hash_delete(rep->hash);
    }
    if (rep->body) {
        if (rep->body) {
            wtk_strbuf_delete(rep->body);
        }
    }
    if (!rep->heap_ref) {
        if (rep->heap) {
            wtk_heap_delete(rep->heap);
        }
        wtk_free(rep);
    }
    return 0;
}


int qtk_response_process_body(qtk_response_t *rep, char* buf, int len, int *left) {
    int buf_left, cpy;
    wtk_strbuf_t *body;

    body = rep->body;
    buf_left = rep->content_length - body->pos;
    cpy = min(len, buf_left);
    wtk_strbuf_push(body, buf, cpy);
    *left = len - cpy;
    if (body->pos >= rep->content_length) {
        return 1;
    } else {
        return 0;
    }
}


static int qtk_response_hdrhash_walker(void *udata, void *data) {
    wtk_strbuf_t *buf = udata;
    hash_str_node_t *node = data;
    wtk_string_t *value = node->value;

    if (!wtk_string_equal_s(&node->key, "content-length")) {
        const char *p;
        if ((p = memchr(value->data, '\n', value->len))) {
            value->len = p - value->data;
        }
        wtk_strbuf_push(buf, node->key.data, node->key.len);
        wtk_strbuf_push_s(buf, ":");
        wtk_strbuf_push(buf, value->data, value->len);
        wtk_strbuf_push_s(buf, "\r\n");
    }

    return 0;
}


static wtk_strbuf_t* qtk_response_hdr_to_buf(qtk_response_t *rep) {
    char tmp[256];
    wtk_strbuf_t *buf = rep->buf;
    int n;

    if (NULL == buf) {
        buf = wtk_strbuf_new(256, 1);
        rep->buf = buf;
    }
    wtk_strbuf_reset(buf);

    n = sprintf(tmp, "HTTP/1.1 %d %s\r\n",
                rep->status, qtk_http_status_str(rep->status));
    wtk_strbuf_push(buf, tmp, n);

    wtk_str_hash_walk(rep->hash, qtk_response_hdrhash_walker, buf);

    if (rep->body && rep->body->pos > 0) {
        n = sprintf(tmp, "Content-Length:%d\r\n\r\n", rep->body->pos);
        wtk_strbuf_push(buf, tmp, n);
    } else {
        wtk_strbuf_push_s(buf, "Content-Length:0\r\n\r\n");
    }

    return buf;
}


int qtk_response_send(qtk_response_t *rep, qtk_socket_t *s) {
    wtk_strbuf_t *buf = qtk_response_hdr_to_buf(rep);
    wtk_strbuf_t *body = rep->body;

    if (body && body->pos) {
        if (qtk_socket_writable_bytes(s) >= buf->pos + body->pos) {
            return qtk_socket_send(s, buf->data, buf->pos)
                || qtk_socket_send(s, body->data, body->pos);
        } else {
            wtk_debug("%s buffer full\r\n", s->remote_addr->data);
        }
    } else {
        return qtk_socket_send(s, buf->data, buf->pos);
    }
    return -1;
}


qtk_response_t *qtk_response_new_from_str(wtk_heap_t *heap, char *data, int len, int status)
{
    qtk_response_t *rep = NULL;

    if (heap) {
        rep = qtk_response_new_ref(heap);
    } else {
        rep = qtk_response_new();
    }

    rep->status = status;

    if (data) {
        qtk_response_set_body(rep, data, len);
    }

    return rep;
}


void qtk_response_set_body(qtk_response_t *rep, const char *buf, int len) {
    if (rep->heap_ref) {
        char *p;
        p = (char*)wtk_heap_malloc(rep->heap, len);
        memcpy(p, buf, len);
        rep->body->data = p;
        rep->body->pos = len;
    } else {
        wtk_strbuf_reset(rep->body);
        wtk_strbuf_push(rep->body, buf, len);
    }
}


void qtk_response_print(qtk_response_t *r) {
    printf("################ response ###############\n");
    wtk_strbuf_t *buf = qtk_response_hdr_to_buf(r);
    wtk_strbuf_t *body = r->body;
    printf("%.*s", buf->pos, buf->data);
    if (body && body->pos > 0) {
        printf("%.*s\r\n", body->pos, body->data);
    }
    fflush(stdout);
}

int qtk_response_reset(qtk_response_t *rep) {
    if (rep->request) {
        qtk_request_delete(rep->request);
        rep->request = NULL;
    }
    wtk_str_hash_reset(rep->hash);
    if (rep->body) {
        wtk_strbuf_reset(rep->body);
    }

    if (!rep->heap_ref) {
        wtk_heap_reset(rep->heap);
        wtk_free(rep);
    }

    return 0;
}


void qtk_response_add_hdr(qtk_response_t *rep, const char *key, int klen, wtk_string_t *value) {
    wtk_str_hash_add2(rep->hash, (char*)key, klen, value);
}


wtk_string_t* qtk_response_get_hdr(qtk_response_t *rep, const char *key, int klen) {
    return wtk_str_hash_find(rep->hash, (char*)key, klen);
}
