#ifndef CORE_QTK_ALLOC_H_
#define CORE_QTK_ALLOC_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

#define qtk_malloc(n)                   malloc(n)
#define qtk_calloc(nmemb, sz)           calloc(nmemb, sz)
#define qtk_free(p)                     free(p)
#define qtk_realloc(ptr, sz)            realloc(ptr, sz)

#define qtk_raw_maloc(n)                malloc(n)
#define qtk_raw_realloc(ptr, sz)        realloc(ptr, sz)
#define qtk_raw_free(p)                 free(p)
#define qtk_raw_calloc(nmemb, sz)       calloc(nmemb, sz)

void *qtk_lalloc(void *ptr, size_t osz, size_t nsz);

#ifdef __cplusplus
};
#endif
#endif
