#ifndef QTK_CORE_QTK_ARRAY_H
#define QTK_CORE_QTK_ARRAY_H
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
#define qtk_array_get(arr, i) ((arr)->data && (i)<(arr)->len) ? (arr)->data+i*(arr)->el_size : NULL;

typedef struct qtk_array qtk_array_t;

struct qtk_array {
    void *data;
    size_t len;
    size_t size;
    uint8_t el_size;
};


qtk_array_t* qtk_array_new(int elsize);
qtk_array_t* qtk_array_new2(int elsize, int initsize);
int qtk_array_delete(qtk_array_t *arr);
int qtk_array_reserve(qtk_array_t *arr, int len);
int qtk_array_resize(qtk_array_t *arr, int len);
int qtk_array_grow(qtk_array_t *arr);
void* qtk_array_add(qtk_array_t *arr, int len);
void* qtk_array_push(qtk_array_t *arr);
int qtk_array_pop(qtk_array_t *arr);
int qtk_array_set(qtk_array_t *arr, int idx, const void *data);

#ifdef __cplusplus
}
#endif
#endif
