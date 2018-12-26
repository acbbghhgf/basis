#include "wtk_kv_cache.h"

wtk_kv_item_t* wtk_kv_item_new(char *k,int kl,char *v,int vl)
{
	wtk_kv_item_t *kv;

	kv=(wtk_kv_item_t*)wtk_calloc(1,sizeof(*kv));
	if(k && kl>0)
	{
		kv->k=wtk_string_dup_data(k,kl);
	}else
	{
		kv->k=0;
	}
	kv->v_is_ref=0;
	if(v && vl>0)
	{
		kv->v=wtk_string_dup_data(v,vl);
	}else
	{
		kv->v=0;
	}
	return kv;
}

int wtk_kv_item_delete(wtk_kv_item_t *kv)
{
	wtk_string_delete(kv->k);
	if(kv->v && !kv->v_is_ref)
	{
		wtk_string_delete(kv->v);
	}
	wtk_free(kv);
	return 0;
}

wtk_kv_cache_t* wtk_kv_cache_new(int max_active_slot)
{
	wtk_kv_cache_t *c;
	int nslot;

	c=(wtk_kv_cache_t*)wtk_malloc(sizeof(*c));
	c->max_active_slot=max_active_slot;
	if(c->max_active_slot>0)
	{
		nslot=c->max_active_slot*2+1;
	}else
	{
		nslot=25007;
	}
	c->hash=wtk_str_hash_new(nslot);
	wtk_queue_init(&(c->kv_link_queue));
	return c;
}

void wtk_kv_cache_clean_queue(wtk_kv_cache_t *c)
{
	wtk_queue_node_t *n;
	wtk_kv_item_t *kv;

	while(1)
	{
		n=wtk_queue_pop(&(c->kv_link_queue));
		if(!n){break;}
		kv=data_offset(n,wtk_kv_item_t,link_n);
		wtk_kv_item_delete(kv);
	}
}

int wtk_kv_cache_delete(wtk_kv_cache_t *c)
{
	wtk_kv_cache_clean_queue(c);
	wtk_str_hash_delete(c->hash);
	wtk_free(c);
	return 0;
}

void wtk_kv_cache_reset(wtk_kv_cache_t *c)
{
	//wtk_str_hash_walk(c->hash,(wtk_walk_handler_t)wtk_cache_delete_kv,c);
	wtk_str_hash_reset(c->hash);
	wtk_kv_cache_clean_queue(c);
}

void wtk_kv_cache_del_kv(wtk_kv_cache_t *c,wtk_kv_item_t *kv)
{
	wtk_queue_remove(&(c->kv_link_queue),&(kv->link_n));
	wtk_str_hash_remove(c->hash,kv->k->data,kv->k->len);
	wtk_kv_item_delete(kv);
	return;
}

void wtk_kv_cache_remove_oldest(wtk_kv_cache_t *c)
{
	wtk_queue_node_t *n;
	wtk_kv_item_t *kv;

	n=c->kv_link_queue.pop;
	if(!n){goto end;}
	kv=data_offset(n,wtk_kv_item_t,link_n);
	wtk_kv_cache_del_kv(c,kv);
end:
	return;
}

wtk_string_t* wtk_kv_cache_add(wtk_kv_cache_t *c,char *k,int k_bytes,char *v,int v_bytes)
{
	wtk_kv_item_t *kv;

	if((c->max_active_slot>0) && (c->kv_link_queue.length>=c->max_active_slot))
	{
		wtk_kv_cache_remove_oldest(c);
	}
	kv=wtk_kv_item_new(k,k_bytes,v,v_bytes);
	wtk_str_hash_add_node(c->hash,kv->k->data,kv->k->len,kv,&(kv->hash_n));
	wtk_queue_push(&(c->kv_link_queue),&(kv->link_n));
	return kv->v;
}

wtk_string_t* wtk_kv_cache_add2(wtk_kv_cache_t *c,char *k,int k_bytes,wtk_string_t *v)
{
	wtk_kv_item_t *kv;

	if((c->max_active_slot>0) && (c->kv_link_queue.length>=c->max_active_slot))
	{
		wtk_kv_cache_remove_oldest(c);
	}
	kv=wtk_kv_item_new(k,k_bytes,0,0);
	kv->v=v;
	kv->v_is_ref=1;
	wtk_str_hash_add_node(c->hash,kv->k->data,kv->k->len,kv,&(kv->hash_n));
	wtk_queue_push(&(c->kv_link_queue),&(kv->link_n));
	return kv->v;
}

wtk_string_t *wtk_kv_cache_get(wtk_kv_cache_t *c,char *k,int k_bytes)
{
	wtk_kv_item_t *kv;
	wtk_queue_node_t *n;

	kv=(wtk_kv_item_t*)wtk_str_hash_find(c->hash,k,k_bytes);
	if(!kv){return 0;}
	if(kv)
	{
		//move the most active kv to the tailer.
		n=&(kv->link_n);
		wtk_queue_touch_node(&(c->kv_link_queue),n);
	}
	return kv->v;
}

void wtk_kv_cache_del(wtk_kv_cache_t *c,char *k,int k_bytes)
{
	wtk_kv_item_t *kv;

	kv=(wtk_kv_item_t*)wtk_str_hash_find(c->hash,k,k_bytes);
	if(!kv){return;}
	wtk_kv_cache_del_kv(c,kv);
}
