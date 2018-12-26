#include "wtk/core/wtk_str_hash.h"

wtk_str_hash_t* wtk_str_hash_new(int nslot)
{
	return wtk_str_hash_new2(nslot,NULL);
}

wtk_str_hash_t* wtk_str_hash_new2(int nslot,wtk_heap_t *heap)
{
	wtk_str_hash_t *h;

	h=(wtk_str_hash_t*)wtk_malloc(sizeof(*h));
	h->nslot=nslot;
	if(heap)
	{
		h->heap=heap;
		h->use_ref_heap=1;
	}else
	{
		h->heap=wtk_heap_new(4096);
		h->use_ref_heap=0;
	}
	h->slot=(wtk_queue_t**)wtk_calloc(nslot,sizeof(wtk_queue_t*));
	return h;
}

int wtk_str_hash_bytes(wtk_str_hash_t *h)
{
	int v;

	v=sizeof(*h)+h->nslot*sizeof(wtk_queue_t*);
	if(!h->use_ref_heap)
	{
		v+=wtk_heap_bytes(h->heap);
	}
	return v;
}

int wtk_str_hash_delete(wtk_str_hash_t *h)
{
	if(!h->use_ref_heap)
	{
		wtk_heap_delete(h->heap);
	}
	wtk_free(h->slot);
	wtk_free(h);
	return 0;
}

int wtk_str_hash_reset(wtk_str_hash_t *h)
{
	if(!h->use_ref_heap)
	{
		wtk_heap_reset(h->heap);
	}
	memset(h->slot,0,sizeof(wtk_queue_t*) * h->nslot);
	return 0;
}

int wtk_str_hash_add(wtk_str_hash_t *h,char* key,int key_bytes,void *value)
{
	hash_str_node_t *n;

	n=(hash_str_node_t*)wtk_heap_zalloc(h->heap,sizeof(hash_str_node_t));
	return wtk_str_hash_add_node(h,key,key_bytes,value,n);
}

int wtk_str_hash_add2(wtk_str_hash_t *h,char* key,int key_bytes,void *value)
{
	char *p;

	p=wtk_heap_malloc(h->heap,key_bytes);
	memcpy(p,key,key_bytes);
	//print_hex(p,key_bytes);
	return wtk_str_hash_add(h,p,key_bytes,value);
}

int wtk_str_hash_add_node(wtk_str_hash_t *h,char* key,int key_bytes,void *value,hash_str_node_t* n)
{
	uint32_t index;
	int ret;

	wtk_string_set((&(n->key)),key,key_bytes);
	//wtk_string_set((&(n->value)),value,value_bytes);
	n->value=value;
	index=hash_string_value_len(key,key_bytes,h->nslot);
	if(!h->slot[index])
	{
		h->slot[index]=(wtk_queue_t*)wtk_heap_malloc(h->heap,sizeof(wtk_queue_t));
		wtk_queue_init(h->slot[index]);
	}
	ret=wtk_queue_push(h->slot[index],&(n->n));
	return ret;
}

wtk_queue_t* wtk_str_hash_find_queue(wtk_str_hash_t *h,char *k,int k_bytes)
{
	uint32_t index;

	index=hash_string_value_len(k,k_bytes,h->nslot);
	return h->slot[index];
}

hash_str_node_t* wtk_str_hash_find_node(wtk_str_hash_t *h, char* key,int key_bytes,uint32_t *rv)
{
	wtk_queue_node_t *qn;
	hash_str_node_t *n,*r;
	uint32_t index;
	wtk_queue_t *q;

	r=0;
	index=hash_string_value_len(key,key_bytes,h->nslot);
	if(rv){*rv=index;}
	q=h->slot[index];
	if(!q)// || q->length==0)
	{
		//wtk_debug("empty slot: %s not found\n",key);
		goto end;
	}
	for(qn=q->pop;qn;qn=qn->next)
	{
		n=(hash_str_node_t*)data_offset2(qn,hash_str_node_t,n);
		//n=(hash_str_node_t*)wtk_queue_node_data(qn,hash_str_node_t,n);
		//if((n->key.len==key_bytes) && (strncmp(key,n->key.data,key_bytes)==0))
		if((n->key.len==key_bytes) && (memcmp(key,n->key.data,key_bytes)==0))
		{
			r=n;
			break;
		}
	}
end:
	return r;
}

hash_str_node_t* wtk_str_hash_find_node3(wtk_str_hash_t *h,char *k,int k_bytes,int insert)
{
	wtk_queue_node_t *qn;
	hash_str_node_t *n,*r;
	uint32_t index;
	wtk_queue_t *q;

	r=0;
	index=hash_string_value_len(k,k_bytes,h->nslot);
	q=h->slot[index];
	if(!q)// || q->length==0)
	{
		//wtk_debug("empty slot: %s not found\n",key);
		q=h->slot[index]=(wtk_queue_t*)wtk_heap_malloc(h->heap,sizeof(wtk_queue_t));
		wtk_queue_init(q);
		goto end;
	}
	for(qn=q->pop;qn;qn=qn->next)
	{
		n=(hash_str_node_t*)data_offset2(qn,hash_str_node_t,n);
		//n=(hash_str_node_t*)wtk_queue_node_data(qn,hash_str_node_t,n);
		//if((n->key.len==key_bytes) && (strncmp(key,n->key.data,key_bytes)==0))
		if((n->key.len==k_bytes) && (memcmp(k,n->key.data,k_bytes)==0))
		{
			r=n;
			break;
		}
	}
end:
	if(!r && insert)
	{
		n=(hash_str_node_t*)wtk_heap_malloc(h->heap,sizeof(hash_str_node_t));
		wtk_heap_fill_string(h->heap,&(n->key),k,k_bytes);
		n->value=NULL;
		wtk_queue_push(q,&(n->n));
		r=n;
	}
	return r;
}

hash_str_node_t* wtk_str_hash_find_node2(wtk_str_hash_t *h, char* key,int key_bytes,uint32_t index)
{
	wtk_queue_node_t *qn;
	hash_str_node_t *n,*r;
	wtk_queue_t *q;

	r=0;n=0;
	q=h->slot[index];
	if(!q)// || q->length==0)
	{
		//wtk_debug("empty slot: %s not found\n",key);
		goto end;
	}
	for(qn=q->pop;qn;qn=qn->next)
	{
		n=(hash_str_node_t*)data_offset2(qn,hash_str_node_t,n);
		//n=(hash_str_node_t*)wtk_queue_node_data(qn,hash_str_node_t,n);
		if((n->key.len==key_bytes) && (strncmp(key,n->key.data,key_bytes)==0))
		//if((n->key.len==key_bytes) && (memcmp(key,n->key.data,key_bytes)==0))
		{
			r=n;
			break;
		}
	}
end:
	return r;
}

hash_str_node_t* wtk_str_hash_find_node_pre_key(wtk_str_hash_t *h, char* key,int key_bytes)
{
	wtk_queue_node_t *qn;
	hash_str_node_t *n,*r;
	uint32_t index;

	r=0;n=0;
	index=hash_string_value_len(key,key_bytes,h->nslot);
	if(!h->slot[index])
	{
		//wtk_debug("empty slot: %s not found\n",key);
		goto end;
	}
	for(qn=h->slot[index]->pop;qn;qn=qn->next)
	{
		n=(hash_str_node_t*)wtk_queue_node_data(qn,hash_str_node_t,n);
		if((n->key.len>=key_bytes) && (strncmp(key,n->key.data,key_bytes)==0))
		{
			r=n;
			break;
		}
	}
end:
	return r;
}

/*
int wtk_str_hash_index(wtk_str_hash_t* h,char *key,int key_bytes)
{
	return hash_string_value_len(key,key_bytes,h->nslot);
}
*/

int wtk_str_hash_cmp_findc(void** x,hash_str_node_t *n)
{
	wtk_cmp_handler_t cmp=(wtk_cmp_handler_t)x[0];
	void *u=x[1];

	return cmp(u,n->value);
}

int wtk_str_hash_findc(wtk_str_hash_t*h,char* k,int kb,wtk_cmp_handler_t cmp,void *user_data,void** v)
{
	wtk_queue_t* q;
	hash_str_node_t *hn;
	int ret,index;
	void* x[2];

	ret=-1;
	index=wtk_str_hash_index(h,k,kb);
	q=h->slot[index];
	if(!q){goto end;}
	x[0]=cmp;
	x[1]=user_data;
	hn=(hash_str_node_t*)wtk_queue_find(q,offsetof(hash_str_node_t,n),(wtk_cmp_handler_t)wtk_str_hash_cmp_findc,x);
	if(!hn){goto end;}
	*v=hn->value;
	ret=0;
end:
	return ret;
}

void* wtk_str_hash_find(wtk_str_hash_t *h, char* key,int key_bytes)
{
	hash_str_node_t *n;

	n=wtk_str_hash_find_node(h,key,key_bytes,0);
	return n ? (n->value) : 0;
}

hash_str_node_t* wtk_str_hash_remove(wtk_str_hash_t *h,char *key,int key_bytes)
{
	hash_str_node_t *n;
	uint32_t index;

	n=wtk_str_hash_find_node(h,key,key_bytes,&index);
	if(!n){return n;}
	wtk_queue_remove(h->slot[index],&(n->n));
	return n;
}

void* wtk_str_hash_malloc(wtk_str_hash_t *h,int bytes)
{
	return wtk_heap_malloc(h->heap,bytes);
}

int wtk_str_hash_walk(wtk_str_hash_t* h,wtk_walk_handler_t handler,void* user_data)
{
	int i,ret;
	ret=0;

	for(i=0;i<h->nslot;++i)
	{
		if(h->slot[i])
		{
			ret=wtk_queue_walk(h->slot[i],offsetof(hash_str_node_t,n),handler,user_data);
			if(ret!=0){goto end;}
		}
	}
end:
	return ret;
}

int wtk_str_hash_elems(wtk_str_hash_t *h)
{
	int i,c;

	c=0;
	for(i=0;i<h->nslot;++i)
	{
		if(h->slot[i])
		{
			c+=h->slot[i]->length;
		}
	}
	return c;
}

//------------------------ iterator section -----------------------
void wtk_str_hash_it_move(wtk_str_hash_it_t *it)
{
	wtk_str_hash_t *hash=it->hash;
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

wtk_str_hash_it_t wtk_str_hash_iterator(wtk_str_hash_t *hash)
{
	wtk_str_hash_it_t it;

	it.hash=hash;
	it.next_index=0;
	it.cur_n=0;
	wtk_str_hash_it_move(&(it));
	return it;
}

hash_str_node_t* wtk_str_hash_it_next(wtk_str_hash_it_t *it)
{
	wtk_queue_node_t *q_n;
	hash_str_node_t *hash_n;

	q_n=it->cur_n;
	if(q_n)
	{
		if(q_n->next)
		{
			it->cur_n=q_n->next;
		}else
		{
			it->cur_n=0;
			wtk_str_hash_it_move(it);
		}
	}
	if(q_n)
	{
		hash_n=data_offset(q_n,hash_str_node_t,n);
	}else
	{
		hash_n=0;
	}
	return hash_n;
}

//------------------------- new file ---------------------
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_source.h"

int wtk_str_hash_load(wtk_str_hash_t *hash,wtk_source_t *src)
{
	wtk_heap_t *heap=hash->heap;
	wtk_strbuf_t *buf;
	wtk_string_t k,*v;
	int ret;

	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		wtk_heap_fill_string(heap,&k,buf->data,buf->pos);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		v=wtk_heap_dup_string(heap,buf->data,buf->pos);
		wtk_str_hash_add(hash,k.data,k.len,v);
	}
end:
	wtk_strbuf_delete(buf);
	return 0;
}

wtk_str_hash_t* wtk_str_hash_new_file(char *fn)
{
	wtk_str_hash_t *hash;
	int n;
	int ret;

	n=wtk_file_lines(fn);
	n=2*n+1;
	hash=wtk_str_hash_new(n);
	ret=wtk_source_load_file(hash,(wtk_source_load_handler_t)wtk_str_hash_load,fn);
	if(ret!=0)
	{
		wtk_str_hash_delete(hash);
		hash=NULL;
	}
	return hash;
}

//-------------------------- test/example section ------------------
void wtk_str_hash_test_g()
{
	wtk_str_hash_t *hash;
	wtk_queue_node_t *qn;
	hash_str_node_t *hash_n;
	int i;

	hash=wtk_str_hash_new(13);
	wtk_str_hash_add_s(hash,"first","a");
	wtk_str_hash_add_s(hash,"second","b");
	wtk_str_hash_add_s(hash,"third","c");
	for(i=0;i<hash->nslot;++i)
	{
		if(!hash->slot[i]){continue;}
		for(qn=hash->slot[i]->pop;qn;qn=qn->next)
		{
			hash_n=data_offset(qn,hash_str_node_t,n);
			wtk_debug("index=%d: [%.*s]=[%s]\n",i,hash_n->key.len,hash_n->key.data,(char*)hash_n->value);
		}
	}
}
