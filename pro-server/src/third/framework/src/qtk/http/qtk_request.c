#include "qtk_request.h"
#include "qtk/entry/qtk_socket.h"
#include "ctype.h"
#include "wtk/os/wtk_fd.h"


static qtk_request_str_to_content_type_f str_to_content=0;

void qtk_request_set_str_to_content_type_f_g(qtk_request_str_to_content_type_f f)
{
    str_to_content=f;
}

qtk_request_t* qtk_request_new()
{
    qtk_request_t* r;

    r=(qtk_request_t*)wtk_calloc(1,sizeof(*r));
    r->sock_id = -1;
    r->heap=wtk_heap_new(1024);
    r->hash = wtk_str_hash_new2(10, r->heap);
    r->buf = NULL;
    r->body=NULL;

    return r;
}


void qtk_request_copy(qtk_request_t *dest, qtk_request_t *src)
{
    dest->heap = src->heap;
    dest->buf = src->buf;
    dest->body = src->body;
    dest->hash = src->hash;
    dest->sock_id = src->sock_id;
}


int qtk_request_bytes(qtk_request_t *r)
{
    int b=sizeof(qtk_request_t);

    if(r->body)
    {
        b+=wtk_strbuf_bytes(r->body);
    }
    if (r->buf) {
        b+=wtk_strbuf_bytes(r->buf);
    }

    b+=wtk_heap_bytes(r->heap);

    return b;
}

int qtk_request_delete(qtk_request_t *r)
{
    if (r->buf) {
        wtk_strbuf_delete(r->buf);
    }
    if(r->body)
    {
        wtk_strbuf_delete(r->body);
    }
    if (r->hash) {
        wtk_str_hash_delete(r->hash);
    }
    wtk_heap_delete(r->heap);
    wtk_free(r);
    return 0;
}

int qtk_request_update(qtk_request_t *r)
{
    r->method=HTTP_UNKNOWN;
    wtk_string_set(&r->url, NULL, 0);
    wtk_string_set(&r->params, NULL, 0);
    r->content_length=0;

    return 0;
}

int qtk_request_init(qtk_request_t *r, int sock_id)
{
    r->sock_id = sock_id;
    r->params.len=0;
    qtk_request_update(r);
    /*r->response = qtk_response_new_ref(r->heap);*/
    return 0;
}

int qtk_request_reset(qtk_request_t *r)
{
    wtk_heap_reset(r->heap);
    if (r->buf) {
        wtk_strbuf_reset(r->buf);
    }
    if (r->body)
    {
        wtk_strbuf_reset(r->body);
    }
    qtk_request_update(r);
    return 0;
}


wtk_string_t* qtk_request_get_hdr(qtk_request_t *r, const char *key, int len) {
    return wtk_str_hash_find(r->hash, (char*)key, len);
}


static int qtk_request_update_content_length(qtk_request_t *r,char *v,int v_len)
{
    r->content_length=wtk_str_atoi(v,v_len);
    if(r->content_length>0)
    {
        if(r->body)
        {
            if(r->content_length > r->body->length)
            {
                wtk_strbuf_delete(r->body);
                r->body=0;
            }
        }
        if(!r->body)
        {
            r->body=wtk_strbuf_new(wtk_round(r->content_length,16), 1);
        }else
        {
            wtk_strbuf_reset(r->body);
        }
    }
    return 0;
}


void qtk_request_set_string(qtk_request_t *r,wtk_string_t *str,char *data,int bytes)
{
    str->data=wtk_heap_dup_data(r->heap,data,bytes);
    str->len=bytes;
}


int qtk_request_process_body(qtk_request_t *r,char* buf,int len,int *left)
{
    int buf_left,cpy;
    wtk_strbuf_t *body;

    body = r->body;
    buf_left=r->content_length-body->pos;
    cpy=min(len,buf_left);
    wtk_strbuf_push(body,buf,cpy);
    *left=len-cpy;
    if( body->pos >= r->content_length)
    {
        return 1;
    } else {
        return 0;
    }
}


static int qtk_request_hdrhash_walker(void *udata, void *data) {
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


static wtk_strbuf_t* qtk_request_hdr_to_buf(qtk_request_t *r) {
    wtk_strbuf_t *buf = r->buf;
    if (NULL == buf) {
        buf = wtk_strbuf_new(256, 1);
        r->buf = buf;
    }
    wtk_strbuf_reset(buf);
    switch (r->method) {
    case HTTP_GET:
        wtk_strbuf_push_s(buf, "GET ");
        break;
    case HTTP_POST:
        wtk_strbuf_push_s(buf, "POST ");
        break;
    case HTTP_PUT:
        wtk_strbuf_push_s(buf, "PUT ");
        break;
    case HTTP_HEAD:
        wtk_strbuf_push_s(buf, "HEAD ");
        break;
    case HTTP_DELETE:
        wtk_strbuf_push_s(buf, "DELETE ");
        break;
    default:
        break;
    }
    wtk_strbuf_push(buf, r->url.data, r->url.len);
    wtk_strbuf_push_s(buf," HTTP/1.1\r\n");

    wtk_str_hash_walk(r->hash, qtk_request_hdrhash_walker, buf);

    if (r->body) {
        int n;
        char tmp[256];
        n = sprintf(tmp, "Content-Length:%d\r\n\r\n", r->body->pos);
        wtk_strbuf_push(buf, tmp, n);
    } else {
        wtk_strbuf_push_s(buf, "Content-Length:0\r\n\r\n");
    }

    return buf;
}


int qtk_request_send(qtk_request_t *r, qtk_socket_t *s) {
    wtk_strbuf_t *buf = qtk_request_hdr_to_buf(r);
    wtk_strbuf_t *body = r->body;

    if (body && body->pos > 0) {
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


int qtk_request_send2(qtk_request_t *r, int s) {
    wtk_strbuf_t *buf = qtk_request_hdr_to_buf(r);
    wtk_strbuf_t *body = r->body;

    if (body && body->pos > 0) {
        return wtk_fd_send(s, buf->data, buf->pos, NULL)
            || wtk_fd_send(s, body->data, body->pos, NULL);
    } else {
        return wtk_fd_send(s, buf->data, buf->pos, NULL);
    }
    return -1;
}


void qtk_request_update_hdr(qtk_request_t *r) {
    wtk_string_t *v = wtk_str_hash_find_s(r->hash, "content-length");
    if (v) {
        qtk_request_update_content_length(r, v->data, v->len);
    }
}


void qtk_request_add_hdr(qtk_request_t *r, const char *key, int klen, wtk_string_t *value) {
    wtk_str_hash_add2(r->hash, (char*)key, klen, value);
}


void qtk_request_set_body(qtk_request_t *r, const char *buf, int len) {
    if (NULL == r->body) {
        r->body = wtk_strbuf_new(wtk_round(len, 16), 1);
    }
    wtk_strbuf_reset(r->body);
    wtk_strbuf_push(r->body, buf, len);
}


void qtk_request_print(qtk_request_t *r)
{
    wtk_strbuf_t *buf = qtk_request_hdr_to_buf(r);
    printf("%.*s", buf->pos, buf->data);
    if(r->body && r->body->pos)
    {
        printf("%.*s\n",r->body->pos,r->body->data);
    }
    fflush(stdout);
}


const char *qtk_request_method_s(qtk_request_t *r) {
    switch (r->method) {
    case HTTP_GET:
        return "GET";
    case HTTP_POST:
        return "POST";
    case HTTP_HEAD:
        return "HEAD";
    case HTTP_PUT:
        return "PUT";
    case HTTP_DELETE:
        return "DELETE";
    default:
        return "UNKNOWN";
    }
}
