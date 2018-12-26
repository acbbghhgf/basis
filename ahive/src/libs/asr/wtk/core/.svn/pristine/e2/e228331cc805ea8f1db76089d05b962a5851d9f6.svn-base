#include "wtk_ringarray.h"

wtk_ringarray_t* wtk_ringarray_new(int nslot,int slot_size)
{
	wtk_ringarray_t *a;

	a=(wtk_ringarray_t*)wtk_malloc(sizeof(*a));
	a->nslot=nslot;
	a->slot_size=slot_size;
	a->used=0;
	a->slot=(void*)wtk_calloc(nslot,slot_size);
	return a;
}

void wtk_ringarray_delete(wtk_ringarray_t *r)
{
	wtk_free(r->slot);
	wtk_free(r);
}

void wtk_ringarray_push(wtk_ringarray_t *r,void *item)
{
	if(r->used<r->nslot)
	{
		memcpy(((char*)r->slot)+r->used*r->slot_size,item,r->slot_size);
		++r->used;
	}else
	{
		memmove(r->slot,(char*)r->slot+r->slot_size,(r->nslot-1)*r->slot_size);
		memcpy(((char*)r->slot)+(r->nslot-1)*r->slot_size,item,r->slot_size);
	}
}

float wtk_ringarray_mean_value(wtk_ringarray_t *r)
{
	float v=0;
	float *f;
	int i;

	if(r->used<=0){return v;}
	f=(float*)r->slot;
	for(i=0;i<r->used;++i)
	{
		v+=f[i];
	}
	return v/r->used;
}

void wtk_ringarray_print_float(wtk_ringarray_t *r)
{
	float *f;
	int i;

	wtk_debug("-----------------------\n");
	f=(float*)r->slot;
	for(i=0;i<r->used;++i)
	{
		printf("v[%d]=%f\n",i,f[i]);
	}
}

void wtk_ringarray_test_g()
{
	wtk_ringarray_t *a;
	int i;
	float f;

	a=wtk_ringarray_new(3,sizeof(float));
	for(i=0;i<10;++i)
	{
		f=i;
		wtk_ringarray_push(a,&f);
		wtk_ringarray_print_float(a);
	}
	wtk_ringarray_delete(a);
}
