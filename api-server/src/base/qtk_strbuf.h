#ifndef _BASE_QTK_STRBUF_H_
#define _BASE_QTK_STRBUF_H_

typedef struct qtk_strbuf qtk_strbuf_t;
#include <stdint.h>

struct qtk_strbuf {
    char *data;
    uint32_t len;
    uint32_t cap;
    int max;
};

#define qtk_strbuf_reset(sb)                (sb)->len = 0
#define qtk_strbuf_append_str(sb, str)      \
    qtk_strbuf_append(sb, str, sizeof(str)-1)

qtk_strbuf_t *qtk_strbuf_new(int init_sz, int max_sz);
int qtk_strbuf_append(qtk_strbuf_t *sb, char *s, int len);
int qtk_strbuf_append_c(qtk_strbuf_t *sb, char c);
void qtk_strbuf_delete(qtk_strbuf_t *sb);

#endif
