#include "wtk/core/wtk_robin.h"

wtk_robin_t* wtk_robin_new(int n)
{
	wtk_robin_t* r;

	r=(wtk_robin_t*)wtk_malloc(sizeof(*r)+n*sizeof(void**));
	r->nslot=n;
	r->pop=0;
	r->used=0;
	r->r=(void**)((char*)r+sizeof(*r));
	return r;
}

void wtk_robin_reset(wtk_robin_t *r)
{
	r->pop=r->used=0;
}

int wtk_robin_delete(wtk_robin_t* r)
{
	wtk_free(r);
	return 0;
}

void wtk_robin_push(wtk_robin_t* r,void *d)
{
	int index;

	index=(r->pop+r->used)%r->nslot;
	r->r[index]=d;
	++r->used;
}

void* wtk_robin_push2(wtk_robin_t *r,void *d)
{
	void *t;

	if(r->used==r->nslot)
	{
		t=wtk_robin_pop(r);
	}else
	{
		t=NULL;
	}
	wtk_robin_push(r,d);
	return t;
}

void* wtk_robin_next(wtk_robin_t* r)
{
	int index;

	if(r->used>=r->nslot)
	{
		wtk_robin_pop(r);
	}
	index=(r->pop+r->used)%r->nslot;
	++r->used;
	return r->r[index];
}

void* wtk_robin_pop(wtk_robin_t *r)
{
	void *d;

	d=r->r[r->pop];
	r->pop=(r->pop+1)%r->nslot;
	--r->used;
	return d;
}

void wtk_robin_pop2(wtk_robin_t *r,int n)
{
	r->pop=(r->pop+n)%r->nslot;
	r->used-=n;
}



/*
void* wtk_robin_at(wtk_robin_t *r,int i)
{
	return r->r[(r->pop+i)%r->nslot];
}
*/

void wtk_robin_peek_win_array(wtk_robin_t *robin,void **array,int pad_end)
{
	void *f;
	int i,pad,j;

	pad=robin->nslot-robin->used;
	i=0;
	if(pad>0 && !pad_end)
	{
		f=(void*)wtk_robin_at(robin,0);
		for(;i<pad;++i)
		{
			array[i]=f;
		}
	}
	for(j=0;j<robin->used;++i,++j)
	{
		f=(void*)wtk_robin_at(robin,j);
		array[i]=f;
	}
	if(i<robin->nslot)
	{
		f=(void*)wtk_robin_at(robin,robin->used-1);
		for(;i<robin->nslot;++i)
		{
			array[i]=f;
		}
	}
}
