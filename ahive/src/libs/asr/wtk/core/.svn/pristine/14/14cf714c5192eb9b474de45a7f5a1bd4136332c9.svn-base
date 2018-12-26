#include "wtk_hash.h"

wtk_hash_t* wtk_hash_new(int nslot)
{
	wtk_hash_t *hash;

	hash=(wtk_hash_t*)wtk_malloc(sizeof(*hash));
	hash->heap=wtk_heap_new(4096);
	hash->nslot=nslot;
	hash->slot=(wtk_queue_t**)wtk_calloc(nslot,sizeof(wtk_queue_t*));
	return hash;
}

int wtk_hash_delete(wtk_hash_t *h)
{
	wtk_heap_delete(h->heap);
	wtk_free(h->slot);
	wtk_free(h);
	return 0;
}

int wtk_hash_reset(wtk_hash_t *h)
{
	wtk_heap_reset(h->heap);
	memset(h->slot,0,sizeof(wtk_queue_t*)*h->nslot);
	return 0;
}

int wtk_hash_bytes(wtk_hash_t *h)
{
	int b;

	b=h->nslot*sizeof(wtk_queue_t*);
	b+=wtk_heap_bytes(h->heap);
	return b;
}

int wtk_hash_add(wtk_hash_t *h,void *v,wtk_hash_f hf)
{
	wtk_hash_node_t *n;

	n=(wtk_hash_node_t*)wtk_heap_malloc(h->heap,sizeof(*n));
	return wtk_hash_add_node(h,v,n,hf);
}

int wtk_hash_add2(wtk_hash_t *h,unsigned int id,void *v)
{
	wtk_hash_node_t *n;
	unsigned int index;
	int ret;

	n=(wtk_hash_node_t*)wtk_heap_malloc(h->heap,sizeof(wtk_hash_node_t));
	n->v=v;
	index=id%h->nslot;
	if(!h->slot[index])
	{
		h->slot[index]=(wtk_queue_t*)wtk_heap_malloc(h->heap,sizeof(wtk_queue_t));
		wtk_queue_init(h->slot[index]);
	}
	ret=wtk_queue_push(h->slot[index],&(n->q_n));
	return ret;
}

int wtk_hash_add_node(wtk_hash_t *h,void *v,wtk_hash_node_t *n,wtk_hash_f hf)
{
	unsigned int index;
	int ret;

	n->v=v;
	index=hf(v,h->nslot);
	if(!h->slot[index])
	{
		h->slot[index]=(wtk_queue_t*)wtk_heap_malloc(h->heap,sizeof(wtk_queue_t));
		wtk_queue_init(h->slot[index]);
	}
	ret=wtk_queue_push(h->slot[index],&(n->q_n));
	return ret;
}

wtk_hash_node_t* wtk_hash_queue_find_node(wtk_hash_t *h,void *k,unsigned int index,wtk_cmp_f cf)
{
	wtk_queue_node_t *qn;
	wtk_hash_node_t *n,*r;

	r=0;
	if(!h->slot[index]){goto end;}
	for(qn=h->slot[index]->pop;qn;qn=qn->next)
	{
		n=(wtk_hash_node_t*)data_offset(qn,wtk_hash_node_t,q_n);
		if(cf(n->v,k)==0)
		{
			r=n;break;
		}
	}
end:
	return r;
}

wtk_hash_node_t* wtk_hash_find_node(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf)
{
	unsigned int index;

	index=hf(k,h->nslot);
	return wtk_hash_queue_find_node(h,k,index,cf);
}

void* wtk_hash_find(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf)
{
	wtk_hash_node_t *n;
	void *v=0;

	n=wtk_hash_find_node(h,k,hf,cf);
	v=n?n->v:0;
	return v;
}

wtk_hash_node_t* wtk_hash_remove(wtk_hash_t *h,void *k,wtk_hash_f hf,wtk_cmp_f cf)
{
	wtk_hash_node_t *n;
	unsigned int index;

	index=hf(k,h->nslot);
	n=wtk_hash_queue_find_node(h,k,index,cf);
	if(!n){return n;}
	wtk_queue_remove(h->slot[index],&(n->q_n));
	return n;
}

int wtk_hash_len(wtk_hash_t *h)
{
	int len=0;
	int i;

	for(i=0;i<h->nslot;++i)
	{
		if(!h->slot[i]){continue;}
		len+=h->slot[i]->length;
	}
	return len;
}

void wtk_hash_print(wtk_hash_t *h)
{
	int i;

	for(i=0;i<h->nslot;++i)
	{
		if(h->slot[i])
		{
			wtk_debug("%d: %d elems.\n",i,h->slot[i]->length);
		}else
		{
			wtk_debug("%d: 0 elems.\n",i);
		}
	}
	wtk_debug("hash: %d\n",wtk_hash_len(h));
}

int wtk_hash_walk(wtk_hash_t* h,wtk_walk_handler_t handler,void* user_data)
{
	int i,ret;
	ret=0;

	for(i=0;i<h->nslot;++i)
	{
		if(h->slot[i])
		{
			ret=wtk_queue_walk(h->slot[i],offsetof(wtk_hash_node_t,q_n),handler,user_data);
			if(ret!=0){goto end;}
		}
	}
end:
	return ret;
}

unsigned int wtk_hash_intptr(int *v,unsigned int nslot)
{
	return (*v)%nslot;
}

int wtk_hash_cmp_intptr(int *v1,int *v2)
{
	return *v1-*v2;
}

//-------------------- iterator section ----------------------

void wtk_hash_it_move(wtk_hash_it_t *it)
{
	wtk_hash_t *hash=it->hash;
	int i;

	for(i=it->next_index;i<hash->nslot;++i)
	{
		if(hash->slot[i] && hash->slot[i]->length>0)
		{
			it->next_index=i+1;
			it->cur_n=hash->slot[i]->pop;
			break;
		}
	}
}

wtk_hash_it_t wtk_hash_iterator(wtk_hash_t *hash)
{
	wtk_hash_it_t it;

	it.hash=hash;
	it.next_index=0;
	it.cur_n=0;
	wtk_hash_it_move(&(it));
	return it;
}

wtk_hash_node_t* wtk_hash_it_next(wtk_hash_it_t *it)
{
	wtk_queue_node_t *q_n;
	wtk_hash_node_t *hash_n;

	q_n=it->cur_n;
	if(q_n)
	{
		if(q_n->next)
		{
			it->cur_n=q_n->next;
		}else
		{
			it->cur_n=0;
			wtk_hash_it_move(it);
		}
	}
	if(q_n)
	{
		hash_n=data_offset(q_n,wtk_hash_node_t,q_n);
	}else
	{
		hash_n=0;
	}
	return hash_n;
}

//--------------------- test/example section -------------------
typedef struct
{
	int k;
	int v;
}wtk_th_t;

static unsigned int wtk_th_hash(wtk_th_t* th,unsigned int nslot)
{
	unsigned int v;

	v=th->k%nslot;
	return v;
}

static int wtk_th_cmp(wtk_th_t *t1,wtk_th_t *t2)
{
	return t1->k-t2->k;
}
#include <math.h>

void wtk_hash_test_g()
{
	wtk_hash_t *h;
	wtk_heap_t *heap;
	int i,n,k;
	wtk_th_t tmp;
	wtk_th_t *v;

	n=8769;
	heap=wtk_heap_new(4096);
	h=wtk_hash_new(2057);
	for(i=0;i<n;++i)
	{
		k=rand();
		tmp.k=k;
		v=wtk_hash_find(h,&tmp,(wtk_hash_f)wtk_th_hash,(wtk_cmp_f)wtk_th_cmp);
		if(v){continue;}
		v=(wtk_th_t*)wtk_heap_malloc(heap,sizeof(*v));
		v->k=k;
		v->v=k;
		//wtk_debug("%d\n",k);
		wtk_hash_add(h,&(v),(wtk_hash_f)wtk_th_hash);
	}
	wtk_hash_print(h);
	wtk_hash_delete(h);
	wtk_heap_delete(heap);
}
