#include "wtk_strpool.h"

wtk_strpool_t* wtk_strpool_new(int nhint)
{
	wtk_strpool_t *p;

	p=(wtk_strpool_t*)wtk_malloc(sizeof(*p));
	p->hash=wtk_str_hash_new(nhint);
	return p;
}

void wtk_strpool_delete(wtk_strpool_t *p)
{
	wtk_str_hash_delete(p->hash);
	wtk_free(p);
}

int wtk_strpool_bytes(wtk_strpool_t *p)
{
	return wtk_str_hash_bytes(p->hash)+sizeof(wtk_strpool_t);
}

void wtk_strpool_reset(wtk_strpool_t *p)
{
	wtk_str_hash_reset(p->hash);
}

wtk_strpool_item_t* wtk_strpool_find_item2(wtk_strpool_t *p,char *v,int v_len,int insert,int *is_new)
{
	wtk_strpool_item_t *item;

	item=wtk_str_hash_find(p->hash,v,v_len);
	if(item || insert==0)
	{
		if(is_new)
		{
			*is_new=0;
		}
		goto end;
	}
	item=(wtk_strpool_item_t*)wtk_heap_malloc(p->hash->heap,sizeof(wtk_strpool_item_t));
	wtk_heap_fill_string(p->hash->heap,&(item->v),v,v_len);
	//wtk_debug("[%.*s]\n",item->v.len,item->v.data);
	item->hook=NULL;
	wtk_str_hash_add(p->hash,item->v.data,item->v.len,item);
	if(is_new)
	{
		*is_new=1;
	}
end:
	return item;
}

wtk_strpool_item_t* wtk_strpool_find_item(wtk_strpool_t *p,char *v,int v_len,int insert)
{
	return wtk_strpool_find_item2(p,v,v_len,insert,NULL);
}

wtk_string_t* wtk_strpool_find(wtk_strpool_t *p,char *v,int v_len,int insert)
{
	wtk_strpool_item_t *item;
	wtk_string_t *x=0;

	item=wtk_strpool_find_item(p,v,v_len,insert);
	if(!item){goto end;}
	x=&(item->v);
end:
	return x;
}
