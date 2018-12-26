#include "wtk_strdict.h" 
#include "wtk/core/wtk_larray.h"

wtk_strdict_t* wtk_strdict_new(int hash_hint)
{
	wtk_strdict_t *d;

	d=(wtk_strdict_t*)wtk_malloc(sizeof(wtk_strdict_t));
	d->hash=wtk_str_hash_new(hash_hint);
	return d;
}

void wtk_strdict_delete(wtk_strdict_t *d)
{
	wtk_str_hash_delete(d->hash);
	wtk_free(d);
}

void wtk_strdict_reset(wtk_strdict_t *d)
{
	wtk_str_hash_reset(d->hash);
}

wtk_strdict_phn_t* wtk_strdict_get(wtk_strdict_t *d,char *k,int k_bytes)
{
	return (wtk_strdict_phn_t*)wtk_str_hash_find(d->hash,k,k_bytes);
}

int wtk_strdict_load(wtk_strdict_t *d,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	wtk_string_t *k,*v;
	wtk_heap_t *heap;
	int nl;
	int ret;
	wtk_larray_t *a;
	wtk_strdict_phn_t *phn;
	int i;
	//int cnt=0;

	a=wtk_larray_new(10,sizeof(void*));
	buf=wtk_strbuf_new(64,1);
	heap=d->hash->heap;
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		k=wtk_heap_dup_string(heap,buf->data,buf->pos);
		wtk_larray_reset(a);
		while(1)
		{
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			v=wtk_heap_dup_string(heap,buf->data,buf->pos);
			wtk_larray_push2(a,&v);
			//wtk_debug("[%.*s]=[%.*s]\n",k->len,k->data,v->len,v->data);
			wtk_source_skip_sp(src,&nl);
			if(nl){break;}
		}
		phn=(wtk_strdict_phn_t*)wtk_heap_malloc(heap,sizeof(wtk_strdict_phn_t));
		phn->nph=a->nslot;
		phn->phns=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*a->nslot);
		for(i=0;i<phn->nph;++i)
		{
			phn->phns[i]=((wtk_string_t**)a->slot)[i];
		}
		//++cnt;
		wtk_str_hash_add(d->hash,k->data,k->len,phn);
	}
	ret=0;
end:
	//wtk_debug("cnt=%d\n",cnt);
	wtk_larray_delete(a);
	wtk_strbuf_delete(buf);
	return ret;
}
