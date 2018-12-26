#include "wtk_wvec.h" 
#include <ctype.h>

int wtk_wvec_load(wtk_wvec_t *cfg,wtk_source_t *src)
{
	int v[2];
	wtk_strbuf_t *buf;
	int ret;
	wtk_wvec_item_t *item;
	int idx=0;
	int i;

	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_int(src,v,2,0);
	cfg->voc_size=v[0];
	cfg->vec_size=v[1];
	cfg->wrds=(wtk_wvec_item_t**)wtk_calloc(cfg->voc_size,sizeof(wtk_wvec_item_t*));
	cfg->hash=wtk_str_hash_new(cfg->voc_size+3);
	//cfg->v1=wtk_vecf_new(cfg->vec_size);
	//cfg->v2=wtk_vecf_new(cfg->vec_size);
	cfg->v1=(wtk_wvec_float_t*)wtk_calloc(cfg->vec_size,sizeof(wtk_wvec_float_t));
	cfg->v2=(wtk_wvec_float_t*)wtk_calloc(cfg->vec_size,sizeof(wtk_wvec_float_t));
	while(1)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){ret=0;goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		item=(wtk_wvec_item_t*)wtk_heap_malloc(cfg->hash->heap,sizeof(wtk_wvec_item_t));
		item->wrd_idx=idx;
		cfg->wrds[idx++]=item;
		item->name=wtk_heap_dup_string(cfg->hash->heap,buf->data,buf->pos);
		item->p=(wtk_wvec_float_t*)wtk_calloc(cfg->vec_size,sizeof(wtk_wvec_float_t));
		ret=wtk_source_read_double(src,item->p,cfg->vec_size);
		wtk_str_hash_add(cfg->hash,item->name->data,item->name->len,item);
	}
end:
	item=cfg->unk=(wtk_wvec_item_t*)wtk_heap_malloc(cfg->hash->heap,sizeof(wtk_wvec_item_t));
	item->name=wtk_heap_dup_string(cfg->hash->heap,"unk",3);
	item->p=(wtk_wvec_float_t*)wtk_calloc(cfg->vec_size,sizeof(wtk_wvec_float_t));
	for(i=0;i<cfg->vec_size;++i)
	{
		item->p[i]=1.0/cfg->vec_size;
	}
	wtk_strbuf_delete(buf);
	return ret;
}

wtk_wvec_t* wtk_wvec_new(char *fn)
{
	wtk_wvec_t *v;

	v=(wtk_wvec_t*)wtk_malloc(sizeof(wtk_wvec_t));
	v->hash=NULL;
	v->wrds=NULL;
	v->v1=NULL;
	v->v2=NULL;
	v->unk=NULL;
	wtk_source_load_file(v,(wtk_source_load_handler_t)wtk_wvec_load,fn);
	return v;
}

void wtk_wvec_delete(wtk_wvec_t *v)
{
	if(v->v1)
	{
		wtk_free(v->v1);
		//wtk_vecf_delete(v->v1);
	}
	if(v->v2)
	{
		wtk_free(v->v2);
		//wtk_vecf_delete(v->v2);
	}
	if(v->hash)
	{
		wtk_str_hash_delete(v->hash);
	}
	if(v->wrds)
	{
		wtk_free(v->wrds);
	}
	wtk_free(v);
}

wtk_wvec_item_t* wtk_wvec_find(wtk_wvec_t *cfg,char *data,int bytes)
{
	return (wtk_wvec_item_t*)wtk_str_hash_find(cfg->hash,data,bytes);
}


//float wtk_wvec_snt_to_vec(wtk_wvec_t *v,char *s,int s_bytes,wtk_vecf_t *vec)
//{
//	int init;
//	wtk_string_t str;
//	char *e;
//	int n;
//	wtk_wvec_item_t *item;
//
//	wtk_vecf_zero(vec);
//	e=s+s_bytes;
//	init=0;
//	wtk_string_set(&(str),0,0);
//	while(s<e)
//	{
//		n=wtk_utf8_bytes(*s);
//		if(init==0)
//		{
//			if(n>1 || !isspace(*s))
//			{
//				str.data=s;
//				str.len=0;
//				init=1;
//			}
//		}else
//		{
//			if(n==1 && isspace(*s))
//			{
//				str.len=s-str.data;
//				//wtk_debug("[%.*s]\n",str.len,str.data);
//				item=wtk_wvec_find(v,str.data,str.len);
//				if(item)
//				{
//					//wtk_vecf_print(item->m);
//					wtk_vecf_add(vec,item->m->p);
//				}
//				str.data=NULL;
//				init=0;
//			}
//		}
//		s+=n;
//	}
//	if(init==1 && str.data>0)
//	{
//		str.len=e-str.data;
//		//wtk_debug("[%.*s]\n",str.len,str.data);
//		item=wtk_wvec_find(v,str.data,str.len);
//		if(item)
//		{
//			wtk_vecf_add(vec,item->m->p);
//		}
//	}
//	return wtk_vecf_norm(vec);
//	//return 1.00;
//}
//
//
//float wtk_wvec_like(wtk_wvec_t *v,char *s1,int s1_bytes,char *s2,int s2_bytes)
//{
//	wtk_vecf_t *v1=v->v1;
//	wtk_vecf_t *v2=v->v2;
//	float f;
//
//	wtk_wvec_snt_to_vec(v,s1,s1_bytes,v1);
//	wtk_wvec_snt_to_vec(v,s2,s2_bytes,v2);
//	f=wtk_vecf_cos(v1,v2);
//	//wtk_debug("[%.*s]=[%.*s] %f\n",s1_bytes,s1,s2_bytes,s2,f);
//	return f;
//}

float wtk_wvec_snt_to_vec(wtk_wvec_t *v,char *s,int s_bytes,wtk_vecf_t *vec)
{
	return 0;
}

float wtk_wvec_like(wtk_wvec_t *v,char *s1,int s1_bytes,char *s2,int s2_bytes)
{
	return 0;
}

