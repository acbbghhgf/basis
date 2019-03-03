#include "wtk_strlike_cfg.h"

int wtk_strlike_cfg_init(wtk_strlike_cfg_t *cfg)
{
	cfg->cost_del=1;
	cfg->cost_ins=1;
	cfg->cost_sub=1;
	cfg->use_eq_len=1;
	cfg->en_filter=NULL;
	cfg->cn_filter=NULL;
	cfg->sp_filter=NULL;
	cfg->use_char=0;
	return 0;
}

int wtk_strlike_cfg_clean(wtk_strlike_cfg_t *cfg)
{
	if(cfg->en_filter)
	{
		wtk_str_hash_delete(cfg->en_filter);
	}
	if(cfg->cn_filter)
	{
		wtk_str_hash_delete(cfg->cn_filter);
	}
	if(cfg->sp_filter)
	{
		wtk_str_hash_delete(cfg->sp_filter);
	}
	return 0;
}

int wtk_strlike_cfg_update_local(wtk_strlike_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;
	int n;
	char *s,*e;

	wtk_local_cfg_update_cfg_f(lc,cfg,cost_del,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,cost_ins,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,cost_sub,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_eq_len,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_char,v);

	v=wtk_local_cfg_find_string_s(lc,"en_filter");
	if(v)
	{
		cfg->en_filter=wtk_str_hash_new(v->len*2+1);
		s=v->data;
		e=s+v->len;
		while(s<e)
		{
			n=wtk_utf8_bytes(*s);
			v=wtk_heap_dup_string(cfg->en_filter->heap,s,n);
			wtk_str_hash_add(cfg->en_filter,v->data,v->len,v);
			s+=n;
		}
	}
	v=wtk_local_cfg_find_string_s(lc,"cn_filter");
	if(v)
	{
		cfg->cn_filter=wtk_str_hash_new(v->len*2+1);
		s=v->data;
		e=s+v->len;
		while(s<e)
		{
			n=wtk_utf8_bytes(*s);
			v=wtk_heap_dup_string(cfg->cn_filter->heap,s,n);
			wtk_str_hash_add(cfg->cn_filter,v->data,v->len,v);
			s+=n;
		}
	}
	v=wtk_local_cfg_find_string_s(lc,"sp_filter");
	if(v)
	{
		cfg->sp_filter=wtk_str_hash_new(v->len*2+1);
		s=v->data;
		e=s+v->len;
		while(s<e)
		{
			n=wtk_utf8_bytes(*s);
			v=wtk_heap_dup_string(cfg->sp_filter->heap,s,n);
			wtk_str_hash_add(cfg->sp_filter,v->data,v->len,v);
			s+=n;
		}
	}
	return 0;
}

int wtk_strlike_cfg_update(wtk_strlike_cfg_t *cfg)
{
	return 0;
}

int wtk_strlike_cfg_is_en(wtk_strlike_cfg_t *cfg,char *data,int bytes)
{
	wtk_string_t *p;

	if(cfg->en_filter)
	{
		p=wtk_str_hash_find(cfg->en_filter,data,bytes);
		//wtk_debug("[%.*s]=%p\n",bytes,data,p);
		return p?0:1;
	}else
	{
		return 1;
	}
}

int wtk_strlike_cfg_is_cn(wtk_strlike_cfg_t *cfg,char *data,int bytes)
{
	wtk_string_t *p;

	if(cfg->cn_filter)
	{
		p=wtk_str_hash_find(cfg->cn_filter,data,bytes);
		return p?0:1;
	}else
	{
		return 1;
	}
}

int wtk_strlike_cfg_is_sp(wtk_strlike_cfg_t *cfg,char *data,int bytes)
{
	wtk_string_t *p;

	if(cfg->sp_filter)
	{
		p=wtk_str_hash_find(cfg->sp_filter,data,bytes);
		//wtk_debug("[%.*s]=%p\n",bytes,data,p);
		return p?1:0;
	}else
	{
		return 0;
	}
}

