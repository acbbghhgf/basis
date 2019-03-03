#include "wtk_sparsem.h" 

wtk_sparsem_t* wtk_sparsem_new(int row,int col,int hint)
{
	wtk_sparsem_t *m;

	m=(wtk_sparsem_t*)wtk_malloc(sizeof(wtk_sparsem_t));
	m->row=row;
	m->col=col;
	m->hint=hint;
	m->hash=(wtk_sparsec_t**)wtk_calloc(hint,sizeof(wtk_sparsec_t*));
	m->heap=wtk_heap_new(4096);

	m->addr=NULL;
	m->pos=0;
	m->nslot=50;
	m->nbytes=m->nslot*sizeof(wtk_sparsec_t);
	return m;
}

void wtk_sparsem_delete(wtk_sparsem_t *m)
{
	wtk_free(m->hash);
	wtk_heap_delete(m->heap);
	wtk_free(m);
}

int wtk_sparsem_bytes(wtk_sparsem_t *m)
{
	return sizeof(wtk_sparsem_t)+sizeof(wtk_sparsec_t*)*m->hint+wtk_heap_bytes(m->heap);
}

void wtk_sparsem_set(wtk_sparsem_t *m,int i,int j,float v)
{
	unsigned int k,idx;
	wtk_sparsec_t *c1;

	k=i*m->col+j;
	idx=k%m->hint;
	if(m->addr==NULL)
	{
		m->addr=(wtk_sparsec_t*)wtk_heap_malloc(m->heap,m->nbytes);
		c1=m->addr;
		m->pos=1;
	}else
	{
		c1=m->addr+(m->pos++);
		if(m->pos>=m->nslot)
		{
			m->pos=0;
			m->addr=NULL;
		}
	}
	c1->k=k;
	c1->v=v;
	c1->next=m->hash[idx];
	m->hash[idx]=c1;
}

float wtk_sparsem_get(wtk_sparsem_t *m,int i,int j)
{
	unsigned int k,idx;
	wtk_sparsec_t *c;

	k=i*m->col+j;
	idx=k%m->hint;
	c=m->hash[idx];
	while(c)
	{
		if(c->k==k)
		{
			return c->v;
		}
		c=c->next;
	}
	//wtk_debug("[%d/%d] not found.\n",i,j);
	return 0;
}
