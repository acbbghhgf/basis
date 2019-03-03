#include "wtk_shash.h"

wtk_shash_t* wtk_shash_new(int nslot)
{
	wtk_shash_t *hash;

	hash=(wtk_shash_t*)wtk_malloc(sizeof(wtk_shash_t));
	hash->nslot=nslot;
	hash->slot=(wtk_slist_node_t*)wtk_calloc(nslot,sizeof(wtk_slist_node_t));
	hash->used=0;
	wtk_shash_reset(hash);
	return hash;
}

void wtk_shash_delete(wtk_shash_t *h)
{
	wtk_free(h->slot);
	wtk_free(h);
}

int wtk_shash_bytes(wtk_shash_t *h)
{
	int b;

	b=h->nslot*sizeof(wtk_slist_node_t);
	b+=sizeof(*h);
	return b;
}

void wtk_shash_reset(wtk_shash_t *h)
{
	//redesign later
	if(h->used>0)
	{
		if(h->nslot>40960)
		{
			wtk_free(h->slot);
			h->slot=(wtk_slist_node_t*)wtk_calloc(h->nslot,sizeof(wtk_slist_node_t));
		}else
		{
			memset(h->slot,0,sizeof(wtk_slist_node_t)*h->nslot);
		}
		h->used=0;
	}
}


void wtk_shash_add(wtk_shash_t *h,unsigned int id,wtk_slist_node_t *q_n)
{
	wtk_slist_node_t *s;
	unsigned int index;

	++h->used;
	index=id%h->nslot;
	s=h->slot+index;
	//wtk_debug("s->prev=%p\n",s->prev);
	q_n->prev=s->prev;
	s->prev=q_n;
}

void wtk_shash_dump2(wtk_shash_t *h,FILE *log)
{
	wtk_slist_node_t *s;
	int i,n;

	fprintf(log,"h n\n");
	for(i=0;i<h->nslot;i+=1)
	{
		s=h->slot+i;
		n=wtk_slist_len(s);
		if(n>0)
		{
			fprintf(log,"%d %d\n",i,n);
		}
	}
}

void wtk_shash_dump(wtk_shash_t *h,char *fn)
{
	FILE *f;

	f=fopen(fn,"w");
	wtk_shash_dump2(h,f);
	fclose(f);
}
