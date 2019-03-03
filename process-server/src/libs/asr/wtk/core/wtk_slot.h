#ifndef WTK_CORE_WTK_SLOT_H_
#define WTK_CORE_WTK_SLOT_H_
#include "wtk/core/wtk_type.h"
#include "wtk_heap.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_slot wtk_slot_t;

struct wtk_slot
{
	void *slot;
	int nslot;
};

wtk_slot_t* wtk_slot_new_h(wtk_heap_t *heap,int nslot,int elem_bytes);
#ifdef __cplusplus
};
#endif
#endif
