#include "wtk_posdict.h"
#include <math.h>


wtk_posdict_t* wtk_posdict_new()
{
	wtk_posdict_t *dict;

	dict=(wtk_posdict_t*)wtk_malloc(sizeof(wtk_posdict_t));
	dict->v.map=NULL;
	dict->use_bin=0;
	return dict;
}

void wtk_posdict_delete(wtk_posdict_t *d)
{
	if(d->use_bin)
	{
		if(d->v.kv)
		{
			wtk_fkv2_delete(d->v.kv);
		}
	}else
	{
		if(d->v.map)
		{
			wtk_str_hash_delete(d->v.map);
		}
	}
	wtk_free(d);
}

void wtk_posdict_set_kv(wtk_posdict_t *d,char *fn)
{
	d->use_bin=1;
	d->v.kv=wtk_fkv2_new(fn);
}

void wtk_posdict_set_kv2(wtk_posdict_t *d,wtk_rbin2_t *rbin,char *fn)
{
	d->use_bin=1;
	d->v.kv=wtk_fkv2_new2(rbin,fn);
	//wtk_debug("%s:%p\n",fn,d->v.kv);
}

wtk_posdict_wrd_t* wtk_posdict_get(wtk_posdict_t *d,char *wrd,int wrd_bytes,int insert)
{
	wtk_posdict_wrd_t *w;
	wtk_str_hash_t *map;

	if(d->use_bin)
	{
		wtk_fkv_env_t env;
		int ret;
		char *s,*e;
		int n;

		wtk_fkv_env_init(d->v.kv,&(env));
		//wtk_debug("[%.*s]=%p\n",wrd_bytes,wrd,d->v.kv);
		s=wrd;
		e=s+wrd_bytes;
		while(s<e)
		{
			n=wtk_utf8_bytes(*s);
			//wtk_debug("search [%.*s]\n",n,s);
			ret=wtk_fkv2_get(d->v.kv,&env,s,n);
			if(ret!=0){return NULL;}
			//wtk_debug("ret=%d %.*s is_end=%d\n",ret,n,s,env.is_end);
			s+=n;
		}
		if(!env.is_end)
		{
			return NULL;
		}
		w=&(d->wrd);
		wtk_string_set(&(w->wrd),wrd,wrd_bytes);
		memcpy(&(w->freq),env.v.str.data,4);
		wtk_string_set(&(w->pos),env.v.str.data+4,env.v.str.len-4);
		//wtk_debug("[%.*s] %f\n",w->pos.len,w->pos.data,w->freq);
		return w;
	}
	map=d->v.map;
	w=(wtk_posdict_wrd_t*)wtk_str_hash_find(map,wrd,wrd_bytes);
	if(w || !insert){return w;}
	w=(wtk_posdict_wrd_t*)wtk_heap_malloc(map->heap,sizeof(wtk_posdict_wrd_t));
	w->freq=0;
	wtk_string_set(&(w->pos),0,0);
	wtk_heap_fill_string(map->heap,&(w->wrd),wrd,wrd_bytes);
	wtk_str_hash_add(map,w->wrd.data,w->wrd.len,w);
	return w;
}

void wtk_posdict_update_freq(wtk_posdict_t *d,double tot)
{
	wtk_str_hash_it_t it;
	hash_str_node_t *node;
	wtk_posdict_wrd_t *wrd;

	it=wtk_str_hash_iterator(d->v.map);
	tot=1.0/tot;
	while(1)
	{
		node=wtk_str_hash_it_next(&(it));
		if(!node){break;}
		wrd=(wtk_posdict_wrd_t*)node->value;
		//wtk_debug("freq=%f\n",wrd->freq);
		wrd->freq=log(wrd->freq*tot);
		//wtk_debug("[%.*s]=%f\n",wrd->wrd.len,wrd->wrd.data,wrd->freq);
		//exit(0);
	}
}


int wtk_posdict_load(wtk_posdict_t *d,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	wtk_posdict_wrd_t *wrd;
	float f;
	int ret;
	double tot=0;

	buf=wtk_strbuf_new(256,1);
	d->v.map=wtk_str_hash_new(40371);
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;break;}
		wrd=wtk_posdict_get(d,buf->data,buf->pos,1);
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		ret=wtk_source_read_float(src,&f,1,0);
		if(ret!=0){goto end;}
		//wtk_debug("freq=%f\n",f);
		wrd->freq=f;
		tot+=f;
		ret=wtk_source_read_string(src,buf);
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		if(ret!=0){goto end;}
		wtk_heap_fill_string(d->v.map->heap,&(wrd->pos),buf->data,buf->pos);
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		//exit(0);
	}
	wtk_posdict_update_freq(d,tot);
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}
