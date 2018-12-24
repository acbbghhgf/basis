#include "qtk_alloc.h"

void *qtk_lalloc(void *ptr, size_t osz, size_t nsz)
{
    if (0 == nsz) {
        qtk_raw_free(ptr);
        return NULL;
    } else {
        return qtk_raw_realloc(ptr, nsz);
    }
}
