#ifndef _PACK_QTK_PACK_H_
#define _PACK_QTK_PACK_H_
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str_hash.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_pack qtk_pack_t;

#define qtk_pack_add_s(p, k, v, vl) qtk_pack_add(p, k, sizeof(k)-1, v, vl)
#define qtk_pack_find_s(p, k)       qtk_pack_find(p, k, sizeof(k)-1)

struct qtk_pack {
    wtk_str_hash_t *hdr;
    wtk_heap_t *heap;
    wtk_string_t *key;              /* for parser use */
    wtk_strbuf_t *buf;
};

qtk_pack_t *qtk_pack_new();
void qtk_pack_delete(qtk_pack_t *p);
int qtk_pack_reset(qtk_pack_t *p);
int qtk_pack_encode(qtk_pack_t *p);
int qtk_pack_add(qtk_pack_t *p, const char *k, size_t kl, const char *v, size_t vl);
wtk_string_t *qtk_pack_find(qtk_pack_t *p, const char *k, size_t kl);

#ifdef __cplusplus
};
#endif
#endif
