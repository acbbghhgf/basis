#include "wtk_slot.h"

wtk_slot_t* wtk_slot_new_h(wtk_heap_t *heap,int nslot,int elem_bytes)
{
	wtk_slot_t *s;

	s=(wtk_slot_t*)wtk_heap_malloc(heap,sizeof(*s));
	s->nslot=nslot;
	if(nslot>0)
	{
		s->slot=(void*)wtk_heap_malloc(heap,elem_bytes*nslot);
	}else
	{
		s->slot=0;
	}
	return s;
}
