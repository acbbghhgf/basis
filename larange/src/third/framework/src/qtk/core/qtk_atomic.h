#ifndef QTK_UTIL_QTK_ATOMIC_H
#define QTK_UTIL_QTK_ATOMIC_H

#ifdef __cplusplus
extern "C" {
#endif

#define QTK_ATOM_CAS(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define QTK_ATOM_CAS_POINTER(ptr, oval, nval) __sync_bool_compare_and_swap(ptr, oval, nval)
#define QTK_ATOM_INC(ptr) __sync_add_and_fetch(ptr, 1)
#define QTK_ATOM_FINC(ptr) __sync_fetch_and_add(ptr, 1)
#define QTK_ATOM_DEC(ptr) __sync_sub_and_fetch(ptr, 1)
#define QTK_ATOM_FDEC(ptr) __sync_fetch_and_sub(ptr, 1)
#define QTK_ATOM_ADD(ptr, n) __sync_add_and_fetch(ptr, n)
#define QTK_ATOM_SUB(ptr, n) __sync_sub_and_fetch(ptr, n)
#define QTK_ATOM_AND(ptr, n) __sync_and_and_fetch(ptr, n)

#ifdef __cplusplus
}
#endif

#endif
