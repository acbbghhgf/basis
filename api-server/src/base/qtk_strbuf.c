#include "qtk_strbuf.h"
#include "os/qtk_alloc.h"

#include <string.h>

#define MAX(a, b)       (((a) > (b)) ? (a) : (b))
#define MIN(a, b)       (((a) < (b)) ? (a) : (b))

static int _strbuf_expand(qtk_strbuf_t *sb, int len)
{
    int new_cap;
    if (sb->max > 0) {
        if ((sb->len + len) > sb->max) return -1;
        new_cap = MIN((sb->len+len) << 1, sb->max);
    } else {
        new_cap = (sb->len+len) << 1;
    }
    sb->cap = new_cap;
    sb->data = qtk_realloc(sb->data, new_cap);
    return 0;
}

qtk_strbuf_t *qtk_strbuf_new(int init_sz, int max_sz)
{
    if (max_sz > 0 && init_sz > max_sz)     return NULL;
    qtk_strbuf_t *sb = qtk_malloc(sizeof(*sb));
    sb->data = qtk_malloc(init_sz);
    sb->len = 0;
    sb->cap = init_sz;
    sb->max = max_sz;
    return sb;
}

int qtk_strbuf_append(qtk_strbuf_t *sb, char *s, int len)
{
    if ((sb->len + len) > sb->cap)
        if (_strbuf_expand(sb, len))    return -1;
    memmove(sb->data + sb->len, s, len);
    sb->len += len;
    return 0;
}

int qtk_strbuf_append_c(qtk_strbuf_t *sb, char c)
{
    if ((sb->len + 1) > sb->cap)
        if (_strbuf_expand(sb, 1))    return -1;
    *(sb->data + sb->len) = c;
    sb->len += 1;
    return 0;
}

void qtk_strbuf_delete(qtk_strbuf_t *sb)
{
    qtk_free(sb->data);
    qtk_free(sb);
}
