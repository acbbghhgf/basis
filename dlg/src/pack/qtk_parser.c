#include "qtk_parser.h"
#include "qtk/os/qtk_base.h"
#include "qtk/os/qtk_alloc.h"
#include "pipe/qtk_pipe.h"

#include <assert.h>

/*#define _parser_update_hdr(ps) {                                                            \
    qtk_pack_t *_pack = ps->pack;                                                           \
    wtk_string_t *_v = wtk_heap_dup_string(_pack->heap, _pack->buf->data, _pack->buf->pos); \
    wtk_str_hash_add(_pack->hdr, _pack->key->data, _pack->key->len, _v);                    \
}*/
static void _parser_update_hdr(qtk_parser_t *ps)
{
    qtk_pack_t *_pack = ps->pack;                                                           \
    wtk_string_t *_v = wtk_heap_dup_string(_pack->heap, _pack->buf->data, _pack->buf->pos); \
    qtk_pack_add(_pack, _pack->key->data, _pack->key->len, _v->data, _v->len);
}

static int _parser_feed(qtk_parser_t *ps, const char *d, size_t len, int *left)
{
    const char *s, *e;
    char c;
    s = d;
    e = d + len;
    qtk_pack_t *p = ps->pack;
    while(s < e) {
        c = *s;
        switch(ps->state) {
        case QTK_PARSER_INIT:
            if (c == US) {
                p->key = wtk_heap_dup_string(p->heap, p->buf->data, p->buf->pos);
                wtk_strbuf_reset(p->buf);
                ps->state = QTK_PARSER_US;
            } else {
                wtk_strbuf_push_c(p->buf, c);
            }
            break;
        case QTK_PARSER_US:
            if (c == RS) {
                _parser_update_hdr(ps);
                wtk_strbuf_reset(p->buf);
                ps->state = QTK_PARSER_RS;
            } else {
                wtk_strbuf_push_c(p->buf, c);
            }
            break;
        case QTK_PARSER_RS:
            if (c == RS) {
                ps->state = QTK_PARSER_DONE;
                goto end;
            } else if (c == US) {
                p->key = wtk_heap_dup_string(p->heap, p->buf->data, p->buf->pos);
                wtk_strbuf_reset(p->buf);
                ps->state = QTK_PARSER_US;
            } else {
                wtk_strbuf_push_c(p->buf, c);
            }
            break;
        case QTK_PARSER_DONE:
        default:
            return -1;
        }
        ++s;
    }

end:
    *left = e - s - 1;
    return 0;
}

static void _parser_reset(qtk_parser_t *ps)
{
    if (ps->pack) {
        ps->pack_unlink(ps->ud, ps);
        ps->pack = NULL;
    }
    ps->state = QTK_PARSER_INIT;
}

static int _parser_process(qtk_parser_t *ps, qtk_pipe_t *p, const char *d, size_t len)
{
    if (!ps->pack) ps->pack_empty(ps->ud, ps);
    int left;
    int ret = _parser_feed(ps, d, len, &left);
    if (ret) goto err;
    if (ps->state == QTK_PARSER_DONE) {
        ps->pack_ready(ps->ud, ps);
        _parser_reset(ps);
    }
    if (left > 0) ps->f(ps, p, d+len-left, left);
    return 0;
err:
    qtk_debug("error pack\n");
    _parser_reset(ps);
    return -1;
}

qtk_parser_t *qtk_parser_new(void *ud)
{
    qtk_parser_t *ps = qtk_calloc(1, sizeof(*ps));
    ps->state = QTK_PARSER_INIT;
    ps->f = _parser_process;
    ps->ud = ud;
    return ps;
}

void qtk_parser_delete(qtk_parser_t *ps)
{
    qtk_parser_clean(ps);
    qtk_free(ps);
}

void qtk_parser_set_handler(qtk_parser_t *ps, qtk_parser_handler empty,
                            qtk_parser_handler ready, qtk_parser_handler unlink)
{
    ps->pack_empty = empty;
    ps->pack_ready = ready;
    ps->pack_unlink = unlink;
}

void qtk_parser_clone(qtk_parser_t *dst, qtk_parser_t *src)
{
    assert(src->state == QTK_PARSER_INIT);
    assert(src->pack == NULL);
    *dst = *src;
}

void qtk_parser_clean(qtk_parser_t *ps)
{
    if (ps->pack) ps->pack_unlink(ps->ud, ps);
}
