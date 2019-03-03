#include "wtk_larray.h"

wtk_larray_t* wtk_larray_new(uint32_t n,uint32_t size)
{
	wtk_larray_t* a;

	a=(wtk_larray_t*)wtk_malloc(sizeof(*a));
	a->slot_alloc=n;
	a->slot_size=size;
	a->nslot=0;
	a->slot=wtk_calloc(n,size);
	return a;
}

int wtk_larray_delete(wtk_larray_t* a)
{
	wtk_free(a->slot);
	wtk_free(a);
	return 0;
}

int wtk_larray_bytes(wtk_larray_t *a)
{
	int bytes=sizeof(wtk_larray_t);

	bytes+=a->slot_size*a->slot_alloc;
	return bytes;
}

wtk_larray_t* wtk_larray_dup(wtk_larray_t *src)
{
	wtk_larray_t* dst;

	dst=wtk_larray_new(src->nslot,src->slot_size);
	dst->nslot=src->nslot;
	memcpy(dst->slot,src->slot,src->nslot*src->slot_size);
	return dst;
}

void wtk_larray_cpy(wtk_larray_t *src,wtk_larray_t *dst)
{
	wtk_larray_reset(dst);
	if(dst->slot_alloc<src->slot_alloc)
	{
		wtk_larray_reset2(dst,src->slot_alloc);
	}
	memcpy(dst->slot,src->slot,src->nslot*src->slot_size);
	dst->nslot=src->nslot;
}

void wtk_larray_merge(wtk_larray_t *merged,wtk_larray_t *pad)
{
	void *dst;

	dst=wtk_larray_push_n(merged,pad->nslot);
	//print_data(dst,4);
	memcpy(dst,pad->slot,pad->slot_size*pad->nslot);
}

void wtk_larray_reset(wtk_larray_t *a)
{
	a->nslot=0;
}

void wtk_larray_reset2(wtk_larray_t *a,int n)
{
	if(a->slot_alloc!=n)
	{
		wtk_free(a->slot);
		a->slot=wtk_calloc(n,a->slot_size);
		a->slot_alloc=n;
	}
	a->nslot=0;
}

void* wtk_larray_push(wtk_larray_t* a)
{
	return wtk_larray_push_n(a,1);
}

void* wtk_larray_get(wtk_larray_t *a,int idx)
{
	//wtk_debug("[%p]=%p\n",a->slot,((char*)a->slot)+idx*a->slot_size);
	return (void*)(((char*)a->slot)+idx*a->slot_size);
}

void wtk_larray_push2(wtk_larray_t *a,void *src)
{
	void *dst;

	dst=wtk_larray_push_n(a,1);
	//print_data(dst,4);
	memcpy(dst,src,a->slot_size);
	//print_data(dst,4);
	//wtk_debug("src:%p,%d\n",src,a->slot_size);
}

void* wtk_larray_push_n(wtk_larray_t* a,uint32_t n)
{
	uint32_t alloc;
	void *s;

	if(a->nslot+n > a->slot_alloc)
	{
		alloc=2*max(n,a->slot_alloc);
		s=wtk_calloc(alloc,a->slot_size);
		memcpy(s,a->slot,a->slot_size*a->nslot);
		wtk_free(a->slot);
		a->slot=s;
		a->slot_alloc=alloc;
	}
	s=(char*)a->slot+a->slot_size*a->nslot;
	a->nslot+=n;
	return s;
}

wtk_flta_t* wtk_flta_new(int n)
{
	wtk_flta_t *f;

	f=(wtk_flta_t*)wtk_malloc(sizeof(wtk_flta_t));
	f->len=n;
	f->p=(float*)wtk_malloc(sizeof(float)*n);
	f->pos=0;
	wtk_flta_zero(f);
	return f;
}

void wtk_flta_delete(wtk_flta_t *a)
{
	wtk_free(a->p);
	wtk_free(a);
}

void wtk_flta_reset(wtk_flta_t *a)
{
	a->pos=0;
}

void wtk_flta_zero(wtk_flta_t *a)
{
	memset(a->p,0,a->len*sizeof(float));
}


//------------------------------- test/examle section ------------------
void wtk_larray_test_g()
{
	wtk_larray_t *a;

	a=wtk_larray_new(10,sizeof(int));
	*((int*)wtk_larray_push(a))=10;
	wtk_larray_delete(a);
}
