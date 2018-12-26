#include "wtk_stridx.h"

wtk_stridx_t* wtk_stridx_new(int n)
{
	wtk_stridx_t *si;

	si=(wtk_stridx_t*)wtk_malloc(sizeof(wtk_stridx_t));
	si->items=(wtk_stridx_item_t**)wtk_calloc(n,sizeof(wtk_stridx_item_t*));
	si->alloc=n;
	si->used=0;
	si->hash=wtk_str_hash_new(n*2+1);
	return si;
}

void wtk_stridx_delete(wtk_stridx_t *si)
{
	wtk_str_hash_delete(si->hash);
	wtk_free(si->items);
	wtk_free(si);
}


int wtk_stridx_get_id(wtk_stridx_t *idx,char *data,int bytes,int insert)
{
	wtk_stridx_item_t *item;

	item=(wtk_stridx_item_t*)wtk_str_hash_find(idx->hash,data,bytes);
	if(item  || insert==0)
	{
		goto end;
	}
	item=(wtk_stridx_item_t*)wtk_heap_malloc(idx->hash->heap,sizeof(wtk_stridx_item_t));
	item->idx=idx->used;
	idx->items[idx->used]=item;
	++idx->used;
	item->str=wtk_heap_dup_string(idx->hash->heap,data,bytes);
	wtk_str_hash_add(idx->hash,item->str->data,item->str->len,item);
end:
	return item?item->idx:-1;
}

wtk_string_t*  wtk_stridx_get_str(wtk_stridx_t *idx,int id)
{

	if(id<0 || id>=idx->used)
	{
		return NULL;
	}
	return idx->items[id]->str;
}

