#ifndef _QTK_UTIL_QTK_FILE_H
#define _QTK_UTIL_QTK_FILE_H
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"


#ifdef __cplusplus
extern "C" {
#endif


int qtk_remove_dir(const char *dir);
int qtk_remove_file(const char *file);
int qtk_write_file(FILE* file, const char* data, int len, int* writed);
char* qtk_read_file_to_heap(const char* fn, int offset, int *n, wtk_heap_t *heap);
uint64_t qtk_file_length(const char *fn);


#ifdef __cplusplus
}
#endif

#endif
