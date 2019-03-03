#include "wtk_chnlike.h"
#include <ctype.h>

wtk_chnlike_t *wtk_chnlike_new(wtk_chnlike_cfg_t *cfg,wtk_rbin2_t *rbin)
{
	wtk_chnlike_t *l;

	l=(wtk_chnlike_t*)wtk_malloc(sizeof(wtk_chnlike_t));
	l->cfg=cfg;
	l->peek=wtk_txtpeek_new(&(cfg->txtpeek));
	l->strlike=wtk_strlike_new(&(cfg->strlike));
	l->heap=wtk_heap_new(4096);
	l->buf=wtk_strbuf_new(256,1);
	if(cfg->use_bin)
	{
		if(rbin)
		{
			l->fkv=wtk_fkv_new4(rbin,cfg->dict_fn,1703);
		}else
		{
			l->fkv=wtk_fkv_new3(cfg->dict_fn);
		}
	}else
	{
		l->fkv=NULL;
	}
	return l;
}

void wtk_chnlike_delete(wtk_chnlike_t *l)
{
	if(l->fkv)
	{
		wtk_fkv_delete(l->fkv);
	}
	wtk_strbuf_delete(l->buf);
	wtk_heap_delete(l->heap);
	wtk_txtpeek_delete(l->peek);
	wtk_strlike_delete(l->strlike);
	wtk_free(l);
}

void wtk_chnlike_reset(wtk_chnlike_t *l)
{
	wtk_heap_reset(l->heap);
	if(l->fkv)
	{
		wtk_fkv_reset(l->fkv);
	}
}

void wtk_chnlike_txt_to_phn(wtk_chnlike_t *l,char *txt,int bytes,wtk_strbuf_t *buf)
{
	wtk_txtpeek_t  *p=l->peek;
	wtk_txtpeek_item_t item;
	wtk_strdict_phn_t *phn;
	int i;
	char *s,*e;
	int n;
	char c;
	wtk_string_t *v;

	//wtk_debug("[%.*s]\n",bytes,txt);
	wtk_strbuf_reset(buf);
	wtk_txtpeek_set(p,txt,bytes);
	while(1)
	{
		item=wtk_txtpeek_next(p);
		//wtk_debug("%d\n",item.v.len);
		if(item.type==WTK_TXTPEEK_EOF){break;}
		if(l->cfg->use_bin)
		{
			v=wtk_fkv_get_str(l->fkv,item.v.data,item.v.len);
			if(!v)
			{
				s=item.v.data;
				e=s+item.v.len;
				while(s<e)
				{
					n=wtk_utf8_bytes(*s);
					if(n==1)
					{
						c=toupper(*s);
						v=wtk_fkv_get_str(l->fkv,&c,1);
						if(v)
						{
							if(buf->pos>0)
							{
								wtk_strbuf_push_c(buf,' ');
							}
							wtk_strbuf_push(buf,v->data,v->len);
						}
					}
					s+=n;
				}
			}else
			{
				if(buf->pos>0)
				{
					wtk_strbuf_push_c(buf,' ');
				}
				wtk_strbuf_push(buf,v->data,v->len);
			}
		}else
		{
			phn=wtk_strdict_get(l->cfg->dict,item.v.data,item.v.len);
			//wtk_debug("[%.*s]=%p\n",item.v.len,item.v.data,phn);
			if(!phn)
			{
				s=item.v.data;
				e=s+item.v.len;
				while(s<e)
				{
					n=wtk_utf8_bytes(*s);
					if(n==1)
					{
						c=toupper(*s);
						phn=wtk_strdict_get(l->cfg->dict,&c,1);
						if(phn)
						{
							for(i=0;i<phn->nph;++i)
							{
								if(buf->pos>0)
								{
									wtk_strbuf_push_c(buf,' ');
								}
								wtk_strbuf_push(buf,phn->phns[i]->data,phn->phns[i]->len);
							}
						}
					}
					s+=n;
				}
			}else
			{
				for(i=0;i<phn->nph;++i)
				{
					if(buf->pos>0)
					{
						wtk_strbuf_push_c(buf,' ');
					}
					wtk_strbuf_push(buf,phn->phns[i]->data,phn->phns[i]->len);
				}
			}
		}
	}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
}

float wtk_chnlike_like(wtk_chnlike_t *l,char *txt1,int txt1_bytes,char *txt2,int txt2_bytes,int *err)
{
	wtk_heap_t *heap=l->heap;
	wtk_strbuf_t *buf=l->buf;
	wtk_string_t *v1,*v2;
	float f=0;
	int ret=-1;

	//wtk_debug("[%.*s]=[%.*s]\n",txt1_bytes,txt1,txt2_bytes,txt2);
	wtk_chnlike_txt_to_phn(l,txt1,txt1_bytes,buf);
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	if(buf->pos<=0){goto end;}
	v1=wtk_heap_dup_string(heap,buf->data,buf->pos);
	wtk_chnlike_txt_to_phn(l,txt2,txt2_bytes,buf);
	if(buf->pos<=0){goto end;}
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	v2=wtk_heap_dup_string(heap,buf->data,buf->pos);
	//wtk_debug("[%.*s]=[%.*s]\n",v1->len,v1->data,v2->len,v2->data);
	f=wtk_strlike_process(l->strlike,v1->data,v1->len,v2->data,v2->len);
	ret=0;
end:
	if(err)
	{
		*err=ret==0?0:1;
	}
	wtk_heap_reset(heap);
	return f;
}
