#include "wtk_vpool.h"

void* wtk_vpool_new_item(wtk_vpool_t *v)
{
	switch(v->type)
	{
	case WTK_VPOOL_BITHEAP:
		return wtk_bit_heap_malloc(v->v.bitheap);
		break;
	case WTK_VPOOL_HEAP:
		return wtk_heap_malloc(v->v.heap,v->alloc);
		break;
	case WTK_VPOOL_CHEAP:
		return wtk_malloc(v->alloc);
		break;
	}
	return NULL;
}

int wtk_vpool_delete_item(wtk_vpool_t *v,void *data)
{
	switch(v->type)
	{
	case WTK_VPOOL_BITHEAP:
		wtk_bit_heap_free(v->v.bitheap,data);
		break;
	case WTK_VPOOL_HEAP:
		break;
	case WTK_VPOOL_CHEAP:
		wtk_free(data);
		break;
	}
	return 0;
}

wtk_vpool_t* wtk_vpool_new(int bytes,int max_free)
{
	return wtk_vpool_new2(bytes,max_free,max_free);
}

wtk_vpool_t* wtk_vpool_new2(int bytes,int max_free,int reset_free)
{
	//return wtk_vpool_new3(bytes,max_free,reset_free,WTK_VPOOL_HEAP);
	return wtk_vpool_new3(bytes,max_free,reset_free,WTK_VPOOL_BITHEAP);
}

wtk_vpool_t* wtk_vpool_new3(int bytes,int max_free,int reset_free,wtk_vpool_type_t type)
{
	return wtk_vpool_new4(bytes,max_free,reset_free,type,128);
}

wtk_vpool_t* wtk_vpool_new4(int bytes,int max_free,int reset_free,wtk_vpool_type_t type,int max_item)
{
	wtk_vpool_t *v;

	v=(wtk_vpool_t*)wtk_malloc(sizeof(*v));
	v->max=max_item;
	v->type=type;
	v->bytes=bytes;
	v->alloc=bytes+sizeof(wtk_queue_node_t);
	//v->alloc=wtk_round(bytes+sizeof(wtk_queue_node_t),16);
	v->reset_free=reset_free;
	v->cache=wtk_robin_new(1024);
	if(v->bytes>v->max)
	{
		v->type=WTK_VPOOL_CHEAP;
		wtk_hoard_init2(&(v->hoard),bytes,max_free,
				(wtk_new_handler_t)wtk_vpool_new_item,
				(wtk_delete_handler2_t)wtk_vpool_delete_item,
				v);
	}else
	{
		switch(v->type)
		{
		case WTK_VPOOL_BITHEAP:
			v->v.bitheap=wtk_bit_heap_new(v->alloc,4096/v->alloc,40960,1.0);
			//计算中删除耗时
			/*
			wtk_hoard_init2(&(v->hoard),bytes,max_free,
					(wtk_new_handler_t)wtk_vpool_new_item,
					(wtk_delete_handler2_t)wtk_vpool_delete_item,
					v);
			*/
			break;
		case WTK_VPOOL_HEAP:
			v->v.heap=wtk_heap_new(4096);
			break;
		case WTK_VPOOL_CHEAP:
			break;
		}
		wtk_hoard_init2(&(v->hoard),bytes,max_free,
			(wtk_new_handler_t)wtk_vpool_new_item,
			(wtk_delete_handler2_t)0,
			v);
	}
	return v;
}

void wtk_vpool_delete(wtk_vpool_t *v)
{
	wtk_robin_delete(v->cache);
	switch(v->type)
	{
	case WTK_VPOOL_BITHEAP:
		wtk_bit_heap_delete(v->v.bitheap);
		break;
	case WTK_VPOOL_HEAP:
		wtk_heap_delete(v->v.heap);
		break;
	case WTK_VPOOL_CHEAP:
		wtk_hoard_clean(&(v->hoard));
		break;
	}
	wtk_free(v);
}

int wtk_vpool_bytes(wtk_vpool_t *v)
{
	int n;

	switch(v->type)
	{
	case WTK_VPOOL_BITHEAP:
		n=wtk_bit_heap_bytes(v->v.bitheap);
		break;
	case WTK_VPOOL_HEAP:
		n=wtk_heap_bytes(v->v.heap);
		break;
	case WTK_VPOOL_CHEAP:
		n=v->hoard.use_length+v->hoard.cur_free;
		n*=v->alloc;
		break;
	default:
		n=0;
		break;
	}
	return n;
}

void wtk_vpool_reset(wtk_vpool_t *v)
{
	int max_free;

	wtk_robin_reset(v->cache);
	switch(v->type)
	{
	case WTK_VPOOL_BITHEAP:
		wtk_hoard_reset(&(v->hoard));
		wtk_bit_heap_reset(v->v.bitheap);
		break;
	case WTK_VPOOL_HEAP:
		wtk_hoard_reset(&(v->hoard));
		wtk_heap_reset(v->v.heap);
		break;
	case WTK_VPOOL_CHEAP:
		max_free=v->hoard.max_free;
		v->hoard.max_free=v->reset_free;
		//wtk_debug("max free=%d\n",v->hoard.max_free);
		wtk_hoard_reuse(&(v->hoard));
		v->hoard.max_free=max_free;
		//wtk_debug("max free=%d cur_free=%d\n",v->hoard.max_free,v->hoard.cur_free);
		break;
	}
}

#ifdef DEUBG_MEM
#define DX
void* wtk_vpool_pop(wtk_vpool_t *v)
{
	static int ki=0;
	void *p;

	p=wtk_malloc(v->bytes);
	++ki;
#ifndef DX
	wtk_debug("new[%d] %p\n",ki,p);
#endif
	return p;
}

void wtk_vpool_push(wtk_vpool_t *v,void *usr_data)
{
	static int ki=0;

	++ki;
#ifndef DX
	wtk_debug("delete[%d] %p\n",ki,usr_data);
#else
	if(ki==259038 || ki==277156)
	{
		//exit(0);
	}
#endif
	wtk_free(usr_data);
}
#else
void* wtk_vpool_pop(wtk_vpool_t *v)
{
	void *p;

	//wtk_debug("cache[%p] %p\n",v,v->cache);
	if(v->cache->used>0)
	{
		p=wtk_robin_pop(v->cache);
	}else
	{
		//p=wtk_vpool_new_item(v);
		p=wtk_hoard_pop(&(v->hoard));
	}
	return p;
}

void wtk_vpool_push(wtk_vpool_t *v,void *usr_data)
{
	//wtk_debug("push[%p] %p/%p\n",v,v->cache,usr_data);
	if(wtk_robin_is_full(v->cache))
	{
		//wtk_vpool_delete_item(v,usr_data);
		wtk_hoard_push(&(v->hoard),usr_data);
	}else
	{
		wtk_robin_push(v->cache,usr_data);
	}
}
#endif
