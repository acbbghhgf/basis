#include "wtk_kdict.h" 


wtk_kdict_t* wtk_kdict_new(wtk_strpool_t *pool,int hint,void *ths,wtk_kdict_parse_value_f parse_f)
{
	wtk_kdict_t *k;

	k=(wtk_kdict_t*)wtk_malloc(sizeof(wtk_kdict_t));
	k->pool=pool;
	k->hash=wtk_str_hash_new(hint);
	k->parse_ths=ths;
	k->parse_f=parse_f;
	return k;
}

void wtk_kdict_delete(wtk_kdict_t *d)
{
	wtk_str_hash_delete(d->hash);
	wtk_free(d);
}

int wtk_kdict_bytes(wtk_kdict_t *d)
{
	return wtk_str_hash_bytes(d->hash)+sizeof(wtk_kdict_t);
}

int wtk_kdict_load(wtk_kdict_t *d,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	wtk_heap_t *heap=d->hash->heap;
	wtk_string_t *k;
	void *p;
	int ret;

	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		if(d->pool)
		{
			k=wtk_strpool_find(d->pool,buf->data,buf->pos,1);
		}else
		{
			k=wtk_heap_dup_string(heap,buf->data,buf->pos);
		}
		//wtk_debug("[%.*s]\n",k->len,k->data);
		ret=wtk_source_skip_sp(src,NULL);
		if(ret!=0){goto end;}
		ret=wtk_source_read_line(src,buf);
		if(ret!=0){goto end;}
		wtk_strbuf_strip(buf);
		if(d->parse_f)
		{
			p=d->parse_f(d->parse_ths,d,k,buf->data,buf->pos);
		}else
		{
			if(d->pool)
			{
				p=wtk_strpool_find(d->pool,buf->data,buf->pos,1);
			}else
			{
				p=wtk_heap_dup_string(heap,buf->data,buf->pos);
			}
			wtk_str_hash_add(d->hash,k->data,k->len,p);
		}
		if(!p){ret=-1;goto end;}
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_kdict_load_file(wtk_kdict_t *d,char *fn)
{
	return wtk_source_load_file(d,(wtk_source_load_handler_t)wtk_kdict_load,fn);
}

void* wtk_kdict_get(wtk_kdict_t *d,char *k,int k_bytes)
{
	return wtk_str_hash_find(d->hash,k,k_bytes);
}
