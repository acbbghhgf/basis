#ifndef WLIB_CORE_WTK_ALLOC_H_
#define WLIB_CORE_WTK_ALLOC_H_
#include <stdlib.h>
#include "wtk_type.h"
#ifdef USE_MALLOC_H
#include <malloc.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
#define MEM_INLINE

#define WTK_ALIGNMENT   ((int)(sizeof(unsigned long)))    /* platform word */
#define WTK_ALIGNMENT_BIT (WTK_ALIGNMENT<<3)
#define wtk_align_ptr(ptr, align) \
	((align)>0 ? (void*)(((unsigned long)(ptr)+(align-1)) & (~(align-1))) : ptr)
    /*
#define wtk_align_ptr(p, a) \
	(uint8_t*)(((unsigned int)(p)+(a-1)) & (~(a-1)))
    */

#define wtk_round(size,align) \
 ((align)>0 ? (((size)+((align)-1))&(~((align)-1))) : size)

#define wtk_round_8(size) ((((size)&7)==0)? (size) : ((size)+8-((size)&7)))
#define wtk_round_16(size) ((((size)&15)==0)? (size) : ((size)+16-((size)&15)))

//#define wtk_round_word(size) ((((size)&(WTK_ALIGNMENT_BIT-1))==0)? (size) : ((size)+WTK_ALIGNMENT_BIT-((size)&(WTK_ALIGNMENT_BIT-1))))
#define wtk_round_word(size) wtk_round_16(size)
//for old version
#define wtk_round8(size) wtk_round_8(size)

//#define DEBUG_MEM

#ifdef MEM_INLINE
#ifdef DEBUG_MEM
#define wtk_free(p)		wtk_free_debug(p,__FUNCTION__,__LINE__)
#define wtk_malloc(n)		wtk_malloc_debug(n,__FUNCTION__,__LINE__)
#define wtk_calloc(nmem,size) wtk_calloc_debug(nmem,size,__FUNCTION__,__LINE__)
#else
#define wtk_free(p)		free(p)
#define wtk_malloc(n)		malloc(n)
#define wtk_calloc(nmem,size) calloc(nmem,size)
#endif
#else
void wtk_free(void* p);
void* wtk_malloc(size_t n);
void* wtk_calloc(int elems,int size);
#endif

void* wtk_memalign(size_t alignment,size_t size);
void print_data(char* data, int len);
#ifdef DEBUG_MEM
void* wtk_calloc_debug(int elems,int size,const char *f,int line);
void* wtk_malloc_debug(size_t n,const char *f,int line);
void wtk_free_debug(void *p,const char *f,int line);
#endif
#ifdef __cplusplus
};
#endif
#endif
