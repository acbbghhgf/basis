#include "wtk_nbest.h" 


wtk_nbest_t* wtk_nbest_new(int size,int nbest,void *cmp_ths,wtk_nbest_cmp_f cmp)
{
	wtk_nbest_t *n;

	n=(wtk_nbest_t*)wtk_malloc(sizeof(wtk_nbest_t));
	n->item=(char*)wtk_malloc(size*(nbest+1));
	n->size=size;
	n->nslot=nbest;
	n->pos=0;
	n->ths=cmp_ths;
	n->cmp=cmp;
	return n;
}

void wtk_nbest_delete(wtk_nbest_t *n)
{
	wtk_free(n->item);
	wtk_free(n);
}

void wtk_nbest_reset(wtk_nbest_t *n)
{
	n->pos=0;
}

void* wtk_nbest_get(wtk_nbest_t *v,int idx)
{
	return v->item+idx*v->size;
}

void wtk_nbest_remove(wtk_nbest_t *v,int idx)
{
	char *px;

	if(idx==v->pos-1)
	{
		--v->pos;
	}else
	{
		//wtk_debug("idx=%d pos=%d\n",idx,v->pos-idx-1);
		px=v->item+idx*v->size;
		memmove(px,px+v->size,(v->pos-idx-1)*v->size);
		--v->pos;
	}
}

void wtk_nbest_add_slot(wtk_nbest_t *v,void *slot)
{
	int i;
	float f;
	char *px;

	for(i=v->pos-1,px=v->item+i*v->size;i>=0;--i,px-=v->size)
	{
		f=v->cmp(v->ths,px,slot);
		if(f<0)
		{
			memcpy(px+v->size,px,v->size);
			//v->item[i+1]=v->item[i];
		}else
		{
			memcpy(px+v->size,slot,v->size);
			//v->item[i+1]=item;
			if(v->pos<v->nslot)
			{
				++v->pos;
			}
			break;
		}
	}
	if(i<0)
	{
		//v->item[0]=item;
		memcpy(v->item,slot,v->size);
		if(v->pos<v->nslot)
		{
			++v->pos;
		}
	}
}

