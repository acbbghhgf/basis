/*
 * wtk_atomic.h
 *
 *  Created on: 2015-7-27
 *      Author: xjl
 */

#ifndef WTK_ATOMIC_H_
#define WTK_ATOMIC_H_
#include "wtk/core/wtk_type.h"
#ifdef __cplusplus
extern "C"{
#endif

#define wtk_atomic_cmp_set(ptr,oldvar,newvar) __sync_bool_compare_and_swap(ptr,oldvar,newvar)
#define wtk_atomic_fetch_add(var,value) __sync_fetch_and_add(var,value)

#define wtk_memory_barrier() __sync_synchronize()
#define wtk_cpu_pause() __asm__("pause")

#ifdef __cplusplus
};
#endif
#endif /* WTK_ATOMIC_H_ */
