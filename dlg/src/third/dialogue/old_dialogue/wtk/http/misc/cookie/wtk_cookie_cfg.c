#include "wtk_cookie_cfg.h"

int wtk_cookie_cfg_init(wtk_cookie_cfg_t *cfg)
{
	cfg->cookie=0;
	cfg->hash=0;
	cfg->lc=0;
	cfg->str.len=0;
	cfg->update_cookie=1;
	return 0;
}

int wtk_cookie_cfg_clean(wtk_cookie_cfg_t *cfg)
{
	if(cfg->cookie)
	{
		wtk_strbuf_delete(cfg->cookie);
	}
	if(cfg->hash)
	{
		wtk_str_hash_delete(cfg->hash);
	}
	return 0;
}

int wtk_cookie_cfg_update_local(wtk_cookie_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_queue_node_t *n;
	wtk_cfg_item_t *p;
	wtk_string_t *v;
	int ret=0;

	//wtk_local_cfg_print(lc);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,str,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,update_cookie,v);
	cfg->lc=lc;
	if(cfg->str.len<=0 || lc->cfg->queue.length<=0){goto end;}
	cfg->cookie=wtk_strbuf_new(256,1);
	cfg->hash=wtk_str_hash_new(lc->cfg->queue.length*2);
	if(cfg->str.len>0)
	{
		wtk_cookie_cfg_update_cookie(cfg,cfg->str.data,cfg->str.len);
	}else
	{
		for(n=lc->cfg->queue.pop;n;n=n->next)
		{
			p=data_offset(n,wtk_cfg_item_t,n);
			if(p->type!=WTK_CFG_STRING)
			{
				wtk_debug("[%.*s] must be string.\n",p->key->len,p->key->data);
				ret=-1;
				goto end;
			}
			//wtk_str_hash_add(cfg->hash,p->key->data,p->key->len,p->value.str);
			if(cfg->cookie->pos>=0)
			{
				wtk_strbuf_push_s(cfg->cookie,";");
			}
			wtk_strbuf_push(cfg->cookie,p->key->data,p->key->len);
			wtk_strbuf_push_s(cfg->cookie,"=");
			wtk_strbuf_push(cfg->cookie,p->value.str->data,p->value.str->len);
		}
	}
end:
	return ret;
}

int wtk_cookie_cfg_update(wtk_cookie_cfg_t *cfg)
{
	return 0;
}

//------------------------------ set-cookie update ----------------------
#include <ctype.h>
typedef enum
{
	WTK_COOKIE_WAIT_KEY,
	WTK_COOKIE_KEY,
	WTK_COOKIE_WAIT_VALUE,
	WTK_COOKIE_VALUE,
}wtk_cookie_state_t;

void wtk_cookie_cfg_update_hash(wtk_cookie_cfg_t *cfg)
{
	wtk_queue_node_t *n;
	wtk_cfg_item_t *p;
	wtk_string_t *v;
	wtk_str_hash_t *hash=cfg->hash;

	for(n=cfg->lc->cfg->queue.pop;n;n=n->next)
	{
		p=data_offset(n,wtk_cfg_item_t,n);
		v=(wtk_string_t*)wtk_str_hash_find(hash,p->key->data,p->key->len);
		if(!v)
		{
			wtk_str_hash_add(hash,p->key->data,p->key->len,p->value.str);
		}
	}
}

int wtk_cookie_update_item(wtk_strbuf_t *buf,hash_str_node_t *node)
{
	wtk_string_t *v;

	if(buf->pos>0)
	{
		wtk_strbuf_push_s(buf,";");
	}
	wtk_strbuf_push(buf,node->key.data,node->key.len);
	wtk_strbuf_push_s(buf,"=");
	v=(wtk_string_t*)node->value;
	wtk_strbuf_push(buf,v->data,v->len);
	return 0;
}

void wtk_cookie_cfg_update_buf(wtk_cookie_cfg_t *cfg)
{
	wtk_strbuf_t *buf=cfg->cookie;

	wtk_strbuf_reset(buf);
	wtk_str_hash_walk(cfg->hash,(wtk_walk_handler_t)wtk_cookie_update_item,buf);
}



void wtk_cookie_cfg_update_cookie(wtk_cookie_cfg_t *cfg,char *data,int len)
{
	wtk_str_hash_t *hash=cfg->hash;
	hash_str_node_t *n;
	wtk_string_t k,v;
	wtk_string_t *x1,*x2;
	char *s,*e;
	wtk_cookie_state_t state;
	char c;
	int set=0;

	if(!cfg->hash)
	{
		return;
	}
//wtk_debug("[%.*s]\n",len,data);
	//wtk_str_hash_reset(hash);
	state=WTK_COOKIE_WAIT_KEY;
	s=data;e=s+len;
	wtk_string_set(&v,0,0);
	wtk_string_set(&k,0,0);
	while(s<e)
	{
		c=*s;
		//wtk_debug("c=%c,state=%d\n",c,state);
		switch(state)
		{
		case WTK_COOKIE_WAIT_KEY:
			if(!isspace(c))
			{
				k.data=s;
				state=WTK_COOKIE_KEY;
			}
			break;
		case WTK_COOKIE_KEY:
			if(isspace(c) || c=='=')
			{
				k.len=s-k.data;
				state=WTK_COOKIE_WAIT_VALUE;
			}
			break;
		case WTK_COOKIE_WAIT_VALUE:
			if(isspace(c))
			{
				break;
			}else
			{
				v.data=s;
				state=WTK_COOKIE_VALUE;
				//goto cookie value;
			}
		case WTK_COOKIE_VALUE:
			if(isspace(c) || (s==(e-1)) || c==';')
			{
				if(s==e-1)
				{
					v.len=s-v.data+1;
				}else
				{
					v.len=s-v.data;
				}
				//wtk_debug("[%.*s]=[%.*s]\n",k.len,k.data,v.len,v.data);
				n=wtk_str_hash_find_node(hash,k.data,k.len,0);
				if(n)
				{
					x2=(wtk_string_t*)n->value;
					if(wtk_string_cmp(x2,v.data,v.len)!=0)
					{
						if(!set)
						{
							set=1;
							wtk_str_hash_reset(hash);
						}
						wtk_heap_fill_string(hash->heap,(wtk_string_t*)n->value,v.data,v.len);
					}
				}else
				{
					if(!set)
					{
						wtk_str_hash_reset(hash);
						set=1;
					}
					x1=wtk_heap_dup_string(hash->heap,k.data,k.len);
					x2=wtk_heap_dup_string(hash->heap,v.data,v.len);
					wtk_str_hash_add(hash,x1->data,x1->len,x2);
				}
				state=WTK_COOKIE_WAIT_KEY;
			}
			break;
		}
		++s;
	}
	if(set)
	{
		if(cfg->str.len<=0)
		{
			wtk_cookie_cfg_update_hash(cfg);
		}
		wtk_cookie_cfg_update_buf(cfg);
	}
}
