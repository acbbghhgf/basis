#include "qtk_array.h"
#include "wtk/core/wtk_str.h"
#include <assert.h>

qtk_array_t* qtk_array_new(int elsize) {
    return qtk_array_new2(elsize, 0);
}

qtk_array_t* qtk_array_new2(int elsize, int initsize) {
    int s = 0;
    qtk_array_t *arr = wtk_calloc(1, sizeof(*arr));
    arr->el_size = elsize;
    if (initsize) {
        s = 1;
        while (s < initsize) s *= 2;
        arr->data = wtk_malloc(elsize * s);
        arr->size = s;
    }
    return arr;
}

int qtk_array_delete(qtk_array_t *arr) {
    if (arr->data) {
        wtk_free(arr->data);
    }
    wtk_free(arr);
    return 0;
}


int qtk_array_reserve(qtk_array_t *arr, int len) {
    assert(len >= arr->len);
    if (NULL == arr->data) {
        int s = 1;
        while (s < len) s *= 2;
        arr->data = wtk_malloc(arr->el_size * s);
        arr->size = s;
    } else {
        int new_sz = arr->size;
        if (len > arr->size) {
            while (new_sz < len) new_sz *= 2;
        } else {
            while (len <= new_sz) new_sz /= 2;
            if (0 == new_sz) new_sz = 1;
            else new_sz *= 2;
        }
        void *data = wtk_malloc(arr->el_size * new_sz);
        if (NULL == data) return -1;
        memcpy(data, arr->data, arr->len * arr->el_size);
        wtk_free(arr->data);
        arr->data = data;
        arr->size = new_sz;
    }
    return 0;
}


int qtk_array_resize(qtk_array_t *arr, int len) {
    if (len > arr->size) {
        if (qtk_array_reserve(arr, len) < 0) return -1;
    }
    memset(arr->data + arr->el_size*arr->len, 0, arr->el_size*(len-arr->len));
    arr->len = len;
    return 0;
}


int qtk_array_grow(qtk_array_t *arr) {
    if (NULL == arr->data) {
        return qtk_array_reserve(arr, 1);
    } else {
        int new_sz = arr->size * 2;
        void *data = wtk_malloc(arr->el_size * new_sz);
        if (NULL == data) return -1;
        memcpy(data, arr->data, arr->len * arr->el_size);
        wtk_free(arr->data);
        arr->data = data;
        arr->size = new_sz;
    }
    return 0;
}


void* qtk_array_add(qtk_array_t *arr, int len) {
    if (arr->len + len > arr->size) {
        if (qtk_array_reserve(arr, arr->len + len) < 0) {
            return NULL;
        }
    }
    void *p = arr->data + arr->el_size*arr->len;
    memset(p, 0, arr->el_size*len);
    arr->len += len;
    return p;
}


void* qtk_array_push(qtk_array_t *arr) {
    return qtk_array_add(arr, 1);
}


int qtk_array_pop(qtk_array_t *arr) {
    if (0 == arr->len) return -1;
    arr->len -= 1;
    return -1;
}


int qtk_array_set(qtk_array_t *arr, int idx, const void *data) {
    assert(idx >= 0);
    if (idx >= arr->len) {
        if (qtk_array_resize(arr, idx+1) < 0) return -1;
    }
    memcpy(arr->data + arr->el_size * idx, data, arr->el_size);
    return 0;
}
