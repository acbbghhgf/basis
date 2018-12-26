#include "wtk_vpool2.h" 

wtk_vpool2_t* wtk_vpool2_new(int bytes,int max_free)
{
	wtk_vpool2_t *v;

	v=(wtk_vpool2_t*)wtk_malloc(sizeof(wtk_vpool2_t));
	v->heap=wtk_bit_heap_new2(bytes);
	v->bytes=bytes;
	v->cache=wtk_robin_new(max_free);
	return v;
}

void wtk_vpool2_delete(wtk_vpool2_t *v)
{
	wtk_bit_heap_delete(v->heap);
	wtk_robin_delete(v->cache);
	wtk_free(v);
}

void wtk_vpool2_reset(wtk_vpool2_t *v)
{
	wtk_bit_heap_reset(v->heap);
	wtk_robin_reset(v->cache);
}

void* wtk_vpool2_pop(wtk_vpool2_t *v)
{
	void *p;

	//wtk_debug("cache[%p] %p\n",v,v->cache);
	if(v->cache->used>0)
	{
		p=wtk_robin_pop(v->cache);
	}else
	{
		//p=wtk_vpool_new_item(v);
		p=wtk_bit_heap_malloc(v->heap);
	}
	return p;
}

void wtk_vpool2_push(wtk_vpool2_t *v,void *p)
{
	if(wtk_robin_is_full(v->cache))
	{
		wtk_bit_heap_free(v->heap,p);
		//wtk_vpool_delete_item(v,usr_data);
	}else
	{
		wtk_robin_push(v->cache,p);
	}
}

int wtk_vpool2_bytes(wtk_vpool2_t *v)
{
	return  wtk_bit_heap_bytes(v->heap);
}
