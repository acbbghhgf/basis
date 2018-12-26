#include "wtk_harray.h"


wtk_harray_t* wtk_harray_new(int nslot1,int nslot2)
{
	wtk_harray_t *h;

	h=(wtk_harray_t*)wtk_malloc(sizeof(*h));
	h->nslot1=nslot1;
	h->nslot2=nslot2;
	h->heap=wtk_heap_new(1024);
	h->slot=(void**)wtk_calloc(nslot1,sizeof(void*));
	return h;
}

void wtk_harray_delete(wtk_harray_t *h)
{
	wtk_heap_delete(h->heap);
	wtk_free(h->slot);
	wtk_free(h);
}

int wtk_harray_bytes(wtk_harray_t *h)
{
	int bytes;

	bytes=sizeof(void*)*h->nslot1;
	//wtk_debug("bytes=%fM\n",bytes*1.0/(1024*1024));
	bytes+=wtk_heap_bytes(h->heap);
	//wtk_debug("bytes=%fM\n",bytes*1.0/(1024*1024));
	return bytes;
}

void wtk_harray_reset(wtk_harray_t *h)
{
	wtk_heap_reset(h->heap);
	memset(h->slot,0,h->nslot1*sizeof(void*));
}

void wtk_harray_set(wtk_harray_t *h,unsigned long idx,void *p)
{
	unsigned long index;
	void **px;

	index=idx/h->nslot2;
	//wtk_debug("idx=%lu index=%lu\n",idx,index);
	px=&(h->slot[index]);
	if(!*px)
	{
		//*px=wtk_calloc(h->nslot2,sizeof(void*));
		*px=wtk_heap_zalloc(h->heap,sizeof(void*)*h->nslot2);
		//wtk_debug("v[%lu]=%p\n",index,*px);
	}
	px=(void**)*px;
	//index=idx-h->nslot2*index;
	index=idx%h->nslot2;
	//wtk_debug("idx=%lu index=%lu\n",idx,index);
	px[index]=p;
}

void* wtk_harray_get(wtk_harray_t *h,unsigned long idx)
{
	unsigned long index;
	void **px;

	index=idx/h->nslot2;
	//wtk_debug("idx=%lu index=%lu\n",idx,index);
	px=(void**)(h->slot[index]);
	if(!px)
	{
		return NULL;
	}
	//wtk_debug("v[%lu/%lu/%d]=%p\n",idx,index,h->nslot1,px);
	index=idx%h->nslot2;//idx-h->nslot2*index;
	return px[index];
}
