#ifdef USE_CRF
#include "wtk_actkg.h" 
void wtk_actkg_save_jsonkv_section(wtk_jsonkv_t *kv,wtk_string_t *p,wtk_string_t *v,wtk_string_t *t,wtk_string_t *section,wtk_string_t *av);
void wtk_actkg_set_ask(wtk_actkg_t *kg,wtk_owl_item_t *item,wtk_string_t *p);
wtk_string_t wtk_actkg_nlgfst_feed_lua(wtk_actkg_t *kg,char *func);
#define wtk_actkg_norm_lex_s(kg,net,data,len,k) wtk_actkg_norm_lex(kg,net,data,len,k,sizeof(k)-1)
#define wtk_actkg_get_norm_wrd_s(kg,item,data,len,k) wtk_actkg_get_norm_wrd(kg,item,data,len,k,sizeof(k)-1)
wtk_string_t* wtk_actkg_get_norm_wrd(wtk_heap_t *heap,wtk_json_item_t *item,char *data,int len,char *k,int k_bytes);

int wtk_lua_crfact_get(lua_State *l)
{
	wtk_crfact_t *act;
	wtk_string_t k;
	wtk_string_t *v;
	int i;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	act=(wtk_crfact_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("get =================================\n");
	v=wtk_crfact_get_value(act,k.data,k.len);
	if(v && v->len>0)
	{
		lua_pushlstring(l,v->data,v->len);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_crfact_get_full(lua_State *l)
{
	wtk_strbuf_t *buf;
	wtk_crfact_t *act;
	wtk_string_t k;
	int i;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	act=(wtk_crfact_t*)lua_touserdata(l,i);
	if(!act->item){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("get =================================\n");
	buf=wtk_strbuf_new(256,1);
	wtk_crfact_item_get_full_value(act->item,k.data,k.len,buf);
	//wtk_debug("[%.*s]=[%.*s]\n",k.len,k.data,buf->pos,buf->data);
	if(buf->pos>0)
	{
		lua_pushlstring(l,buf->data,buf->pos);
		cnt=1;
	}
	wtk_strbuf_delete(buf);
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_crfact_print(lua_State *l)
{
	wtk_crfact_t *act;
	int i;

	//wtk_debug("=======================> type=%d\n",lua_type(l,1));
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	act=(wtk_crfact_t*)lua_touserdata(l,i);
	//wtk_debug("=======================>\n");
	wtk_crfact_print(act);
end:
	//wtk_debug("get =================================\n");
	return 0;
}

int wtk_lua_actkg_new_act(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_string_t k;
	wtk_crfact_t *act;
	int i;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("get =================================\n");
	act=wtk_crfact_new2(kg->heap,k.data,k.len);
	//wtk_crfact_print(act);
	if(act)
	{
		lua_pushlightuserdata(l,act);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_last_act(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_crfact_t *act;
	int i;
	int cnt=0;
	wtk_actkg_history_t *history;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	if(kg->history->used<=0){goto end;}
	history=(wtk_actkg_history_t*)wtk_robin_at(kg->history,kg->history->used-1);
	if(history->ask->pos<=0){goto end;}
	//wtk_debug("get =================================\n");
	act=wtk_crfact_new2(kg->heap,history->ask->data,history->ask->pos);
	//wtk_crfact_print(act);
	if(act)
	{
		lua_pushlightuserdata(l,act);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_ask_value(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	if(kg->ask_yes_set2)
	{
		lua_pushlstring(l,kg->ask_yes_set2->data,kg->ask_yes_set2->len);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_ask_key(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	if(kg->ask_k2->pos>0)
	{
		lua_pushlstring(l,kg->ask_k2->data,kg->ask_k2->pos);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_ask_owl_item(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	if(kg->owl_item2)
	{
		lua_pushlightuserdata(l,kg->owl_item2);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_ask_p(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	if(kg->ask_p2->pos)
	{
		lua_pushlstring(l,kg->ask_p2->data,kg->ask_p2->pos);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_owl_is_value_valid(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,v,a;
	size_t len;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("p not found\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("a not found\n");
		goto end;
	}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("v not found\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	i=wtk_owlkv_is_value_valid(kg->owlkv,p.data,p.len,v.data,v.len,a.data,a.len);
	lua_pushboolean(l,i);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_actkg_owlkv_set_negative_value(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,v,a;
	size_t len;
	int cnt=0;
	wtk_owl_item_t *item;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("p not found\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("a not found\n");
		goto end;
	}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("v not found\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	item=wtk_owlkv_set_negative_value(kg->owlkv,p.data,p.len,v.data,v.len,a.data,a.len);
	if(item)
	{
		lua_pushlightuserdata(l,item);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_actkg_owlkv_get_matched_value_item(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,v,a;
	size_t len;
	int cnt=0;
	wtk_owl_item_t *item;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("p not found\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("a not found\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("v not found\n");
		goto end;
	}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	item=wtk_owlkv_get_matched_owl(kg->owlkv,p.data,p.len,a.data,a.len,v.data,v.len);
	if(item)
	{
		lua_pushlightuserdata(l,item);
		cnt=1;
	}
end:
	return cnt;
}


int wtk_lua_actkg_owlkv_set_value(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,v,a;
	size_t len;
	int cnt=0;
	wtk_owl_item_t *item;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("p not found\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("a not found\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("v not found\n");
		goto end;
	}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	item=wtk_owlkv_set_value(kg->owlkv,p.data,p.len,a.data,a.len,v.data,v.len);
	if(item)
	{
		lua_pushlightuserdata(l,item);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_actkg_owlkv_owl_item_get_attr(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_owl_item_t *item;
	int i;
	wtk_string_t a,p;
	size_t len;
	int cnt=0;
	wtk_string_t *v;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	item=(wtk_owl_item_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	//wtk_owl_item_print(item);
	v=wtk_owlkv_owl_item_get_attr(kg->owlkv,item,&a,&p);
	if(v && v->len>0)
	{
		lua_pushlstring(l,v->data,v->len);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_actkg_owlkv_ans(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,k,a,v;
	size_t len;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	v=wtk_owlkv_ans(kg->owlkv,p.data,p.len,k.data,k.len,a.data,a.len,v.data,v.len);
	if(v.len>0)
	{
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_actkg_owlkv_get_rel(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t a;
	size_t len;
	int cnt=0;
	wtk_owlkv_item_t vi;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i))
	{
		wtk_debug("1 param must be kg\n");
		goto end;
	}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("2 param must be kg\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	//wtk_debug("[%.*s]\n",a.len,a.data);
	vi=wtk_owlkv_get_owl_item_by_rel(kg->owlkv,a.data,a.len);
	if(vi.k.len>0)
	{
		lua_pushlstring(l,vi.k.data,vi.k.len);
		lua_pushlstring(l,vi.v.data,vi.v.len);
		cnt=2;
	}else
	{
		wtk_debug("get rel failed\n");
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_actkg_owlkv_get_value(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,a;
	size_t len;
	int cnt=0;
	wtk_owlkv_item_t oi;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	oi=wtk_owlkv_get_value(kg->owlkv,p.data,p.len,a.data,a.len);
	if(oi.k.len>0 && oi.v.len>0)
	{
		lua_pushlstring(l,oi.k.data,oi.k.len);
		lua_pushlstring(l,oi.v.data,oi.v.len);
		cnt=2;
	}
end:
	wtk_debug("cnt=%d\n",cnt);
	return cnt;
}


void wtk_actkg_nlgfst_feed_nlg_lua(wtk_actkg_t *kg,char *func,wtk_nlg2_function_t *f,wtk_strbuf_t *buf)
{
	wtk_lua2_arg_t arg[4];

	//wtk_debug("lua=[%s]\n",func);
	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=kg;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=f;
	arg[2].type=WTK_LUA2_THS;
	arg[2].v.ths=kg->ext_ths;
	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(kg->lua,func,buf,arg,arg+1,arg+2,NULL);
}


int wtk_lua_actkg_owlkv_gen_text(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,x;
	size_t len;
	int cnt=0;
	wtk_string_t pv;
	wtk_nlg2_gen_env_t env;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	x.data=(char*)lua_tolstring(l,i,&len);
	x.len=len;
	//wtk_debug("[%.*s]=[%.*s]\n",p.len,p.data,a.len,a.data);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,x.data,x.len);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	pv=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,x.data,x.len,&env);
	if( pv.len>0)
	{
		lua_pushlstring(l,pv.data,pv.len);
		cnt=1;
	}
	//exit(0);
end:
	return  cnt;
}

int wtk_lua_actkg_owlkv_check_value(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,x,v;
	size_t len;
	int cnt=0;
	//wtk_string_t pv;
	int ret;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	x.data=(char*)lua_tolstring(l,i,&len);
	x.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]=[%.*s]\n",p.len,p.data,a.len,a.data);
	ret=wtk_owlkv_check_value(kg->owlkv,p.data,p.len,x.data,x.len,v.data,v.len);
	lua_pushnumber(l,ret);
	cnt=1;
	//exit(0);
end:
	return  cnt;
}


int wtk_lua_actkg_owlkv_get_attr(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,x,a;
	size_t len;
	int cnt=0;
	wtk_string_t *pv;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	x.data=(char*)lua_tolstring(l,i,&len);
	x.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	//wtk_debug("[%.*s]=[%.*s]\n",p.len,p.data,a.len,a.data);
	pv=wtk_owlkv_get_attr(kg->owlkv,p.data,p.len,x.data,x.len,a.data,a.len);
	if(pv && pv->len>0)
	{
		lua_pushlstring(l,pv->data,pv->len);
		cnt=1;
	}
	//exit(0);
end:
	return  cnt;
}

int wtk_lua_actkg_owlkv_get_str_nlg_text(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,a,x,v;
	size_t len;
	int cnt=0;
	wtk_string_t *pv;
	wtk_owl_item_t *item,*ti;
	wtk_nlg2_gen_env_t env;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	x.data=(char*)lua_tolstring(l,i,&len);
	x.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	item=wtk_owlkv_get_owl_by_p(kg->owlkv,p.data,p.len,x.data,x.len);
	if(!item)
	{
		wtk_debug("[%.*s] [%.*s] not found\n",p.len,p.data,x.len,x.data)
		goto end;
	}
	if(item->nitem>0)
	{
		if(wtk_string_cmp_s(&(a),"ask")==0)
		{
			ti=wtk_owlkv_get_unselect_item(kg->owlkv,item,&p);
			if(ti)
			{
				item=ti;
			}
		}else if(wtk_string_cmp_s(&(a),"talk")==0)
		{
			ti=wtk_owlkv_get_select_item(kg->owlkv,item,&p);
			if(ti)
			{
				item=ti;
			}
		}
	}
	//wtk_debug("item=%p\n",item);
	pv=wtk_owl_item_find_attr_value(item,a.data,a.len,-1);
	if(!pv)
	{
		wtk_debug("[%.*s] not found\n",a.len,a.data);
		goto end;
	}
	wtk_debug("[%.*s]\n",pv->len,pv->data);
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,pv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v.data,v.len);
	//wtk_debug("gen text [%.*s] failed\n",v.len,v.data);
	x=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	//wtk_debug("[%.*s]\n",v.len,v.data);
	//wtk_debug("a=[%.*s]\n",a.len,a.data);
	if(x.len>0)
	{
		if(wtk_string_cmp_s(&(a),"ask")==0)
		{
			//wtk_debug("set ask\n");
			wtk_actkg_set_ask(kg,item,&p);
		}
		lua_pushlstring(l,x.data,x.len);
		cnt=1;
	}else
	{
		//wtk_debug("gen text [%.*s] failed\n",v.len,v.data);
	}
end:
	return  cnt;
}

//wtk_lua_actkg_owlkv_owl_item_get_next_nlg_text(kg,owl_item,"ask",p);
int wtk_lua_actkg_owlkv_owl_item_get_next_nlg_text(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,a,v;
	size_t len;
	int cnt=0;
	wtk_string_t *pv;
	wtk_owl_item_t *item;
	wtk_nlg2_gen_env_t env;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	item=(wtk_owl_item_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	pv=wtk_owl_item_find_attr_value_s(item,"next",-1);
	if(!pv)
	{
		wtk_debug("next not found\n");
		goto end;
	}
	///wtk_debug("[%.*s]\n",pv->len,pv->data);
	item=wtk_owlkv_get_owl_by_p(kg->owlkv,p.data,p.len,pv->data,pv->len);
	if(!item){goto end;}
	pv=wtk_owl_item_find_attr_value(item,a.data,a.len,-1);
	if(!pv){goto end;}
	//wtk_debug("[%.*s]\n",pv->len,pv->data);
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,pv,&p);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v.data,v.len);
	v=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	if(v.len>0)
	{
		if(wtk_string_cmp_s(&(a),"ask")==0)
		{
			wtk_actkg_set_ask(kg,item,&p);
		}
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return  cnt;
}

int wtk_lua_actkg_owlkv_get_owl_nlg_text(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t p,a,v;
	size_t len;
	int cnt=0;
	wtk_string_t *pv;
	wtk_owl_item_t *item;
	wtk_nlg2_gen_env_t env;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i))
	{
		wtk_debug("first param must be kg\n");
		goto end;
	}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isuserdata(l,i))
	{
		wtk_debug("2 param must be owl_item\n");
		goto end;
	}
	item=(wtk_owl_item_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("3 param must be string\n");
		goto end;
	}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("4 param must be string\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	//wtk_debug("============================[%.*s]\n",a.len,a.data);
	pv=wtk_owl_item_find_attr_value(item,a.data,a.len,-1);
	if(!pv)
	{
		wtk_debug("[%.*s] not found\n",a.len,a.data);
		goto end;
	}
	//wtk_debug("[%.*s]\n",pv->len,pv->data);
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,pv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	v=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	if(v.len>0)
	{
		if(wtk_string_cmp_s(&(a),"ask")==0)
		{
			wtk_actkg_set_ask(kg,item,&p);
		}
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}else
	{
		wtk_debug("gen failed\n");
	}
end:
	wtk_debug("cnt=%d\n",cnt);
	return  cnt;
}

int wtk_lua_actkg_get_nlg(lua_State *l)
{
	wtk_actkg_t *kg;
	int i,n;
	int cnt=0;
	wtk_string_t p;
	size_t len;
	wtk_owl_class_t *cls=NULL;
	wtk_owl_item_t *item=NULL;
	wtk_string_t person;
	wtk_string_t extv;

	n=lua_gettop(l);
	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	person.data=NULL;
	person.len=0;
	for(++i;i<=n;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		p.data=(char*)lua_tolstring(l,i,&len);
		p.len=len;
		//wtk_debug("[%.*s]\n",p.len,p.data);
		switch(i)
		{
		case 2:
			person=p;
			cls=wtk_owlkv_get_class(kg->owlkv,p.data,p.len);
			if(!cls)
			{
				wtk_debug("%.*s not found\n",p.len,p.data);
				goto end;
			}
			break;
		case 3:
			item=wtk_owl_class_find_prop(cls,&p);
			if(!item)
			{
				wtk_debug("%.*s not found\n",p.len,p.data);
				goto end;
			}
			break;
		default:
			if(i==n-1)
			{
				wtk_owl_item_attr_kv_t *kv;
				wtk_string_t *v;

				kv=wtk_owl_tree_find_attr(kg->owlkv->owl,item,p.data,p.len,0);
				if(!kv||kv->v.len<=0)
				{
					wtk_debug("%.*s not found\n",p.len,p.data);
					goto end;
				}
				++i;
				if(!lua_isstring(l,i)){goto end;}
				extv.data=(char*)lua_tolstring(l,i,&len);
				extv.len=len;
				v=wtk_owl_item_attr_kv_get_value(kv,-1);
				if(v)
				{
					wtk_strbuf_t *buf;
					wtk_string_t *xv;
					wtk_strbuf_t *buf2;

					buf2=wtk_strbuf_new(256,1);
					wtk_strbuf_push(buf2,p.data,p.len);
					wtk_strbuf_push_s(buf2,"_args");
					//wtk_owl_item_print(item);
					buf=wtk_strbuf_new(256,1);
					wtk_strbuf_push(buf,v->data,v->len);
					wtk_strbuf_push_s(buf,"(p=\"");
					wtk_strbuf_push(buf,person.data,person.len);
					wtk_strbuf_push_s(buf,"\"");
					kv=wtk_owl_tree_find_attr(kg->owlkv->owl,item,buf2->data,buf2->pos,0);
					if(kv)
					{
						xv=wtk_owl_item_attr_kv_get_value(kv,-1);
						if(xv)
						{
							wtk_strbuf_push_s(buf,",");
							wtk_strbuf_push(buf,xv->data,xv->len);
						}
					}
					if(extv.len>0)
					{
						wtk_strbuf_push_s(buf,",");
						wtk_strbuf_push(buf,extv.data,extv.len);
					}
					wtk_strbuf_push_s(buf,")");
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					lua_pushlstring(l,buf->data,buf->pos);
					wtk_strbuf_delete(buf);
					wtk_strbuf_delete(buf2);
					cnt=1;
					goto end;
				}
			}else
			{
				//wtk_owl_item_print(item);
				item=wtk_owl_item_find_item2(item,p.data,p.len);
				if(!item)
				{
					wtk_debug("%.*s not found\n",p.len,p.data);
					goto end;
				}
			}
			break;
		}

	}
	//exit(0);
end:

	return cnt;
}

int wtk_lua_jsonkv_get(lua_State *l)
{
	wtk_jsonkv_t *kv=NULL;
	int i,n;
	int cnt=0;
	wtk_string_t p;
	size_t len;
	wtk_json_t *json;
	wtk_json_item_t *item;
	wtk_strbuf_t *buf;

	n=lua_gettop(l);
	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	json=wtk_jsonkv_get_json(kv,p.data,p.len);
	if(!json){goto end;}
	item=json->main;
	for(++i;i<=n;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		p.data=(char*)lua_tolstring(l,i,&len);
		p.len=len;
		//wtk_debug("[%.*s]\n",p.len,p.data);
		if(item->type!=WTK_JSON_OBJECT)
		{
			goto end;
		}
		item=wtk_json_obj_get(item,p.data,p.len);
		if(!item)
		{
			//wtk_debug("[%.*s] not found\n",p.len,p.data);
			goto end;
		}
	}
	buf=wtk_strbuf_new(256,1);
	//wtk_json_item_print3(item);
	wtk_json_item_print(item,buf);
	if(buf->pos>0)
	{
		lua_pushlstring(l,buf->data,buf->pos);
		cnt=1;
	}
	wtk_strbuf_delete(buf);
end:
	//exit(0);
	if(kv)
	{
		wtk_jsonkv_reset(kv);
	}
	return cnt;
}

int wtk_lua_jsonkv_add_object(lua_State *l)
{
	wtk_jsonkv_t *kv=NULL;
	int i,n;
	int cnt=0;
	wtk_string_t p;
	wtk_string_t inst;
	size_t len;
	wtk_json_t *json;
	wtk_json_item_t *item,*vi;
	int b=0;

	n=lua_gettop(l);
	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	inst=p;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	json=wtk_jsonkv_get_json(kv,p.data,p.len);
	if(!json){goto end;}
	item=json->main;
	//wtk_debug("item=%p\n",item);
	for(++i;i<=n;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		p.data=(char*)lua_tolstring(l,i,&len);
		p.len=len;
		if(item->type!=WTK_JSON_OBJECT)
		{
			wtk_debug("[%.*s] must be object.\n",p.len,p.data);
			goto end;
		}
		vi=wtk_json_obj_get(item,p.data,p.len);
		if(!vi)
		{
			vi=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,item,p.data,p.len,vi);
			b=1;
		}
		item=vi;
	}
	if(b)
	{
		//wtk_json_item_print3(json->main);
		wtk_jsonkv_save_json(kv,inst.data,inst.len,json);
	}
end:
	//exit(0);
	if(kv)
	{
		wtk_jsonkv_reset(kv);
	}
	return cnt;
}

int wtk_lua_jsonkv_set_str(lua_State *l)
{
	wtk_jsonkv_t *kv=NULL;
	int i;
	wtk_string_t p,k,v;
	size_t len;
	wtk_json_t *json;
	int ret;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	json=wtk_jsonkv_get_json(kv,p.data,p.len);
	if(!json){goto end;}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	ret=wtk_json_item_set_path_str(json,k.data,k.len,v.data,v.len);
	if(ret==0)
	{
		wtk_jsonkv_save_json(kv,p.data,p.len,json);
	}
end:
	if(kv)
	{
		wtk_jsonkv_reset(kv);
	}
	return 0;
}

int wtk_lua_jsonkv_set_str_x(lua_State *l)
{
	wtk_jsonkv_t *kv=NULL;
	int i,n;
	int cnt=0;
	wtk_string_t p,v;
	wtk_string_t inst;
	size_t len;
	wtk_json_t *json;
	wtk_json_item_t *item,*vi;

	n=lua_gettop(l);
	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	inst=p;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	json=wtk_jsonkv_get_json(kv,p.data,p.len);
	if(!json){goto end;}
	item=json->main;
	//wtk_debug("item=%p\n",item);
	for(++i;i<n-1;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		p.data=(char*)lua_tolstring(l,i,&len);
		p.len=len;
		if(item->type!=WTK_JSON_OBJECT)
		{
			wtk_debug("[%.*s] must be object.\n",p.len,p.data);
			goto end;
		}
		vi=wtk_json_obj_get(item,p.data,p.len);
		if(!vi)
		{
			vi=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,item,p.data,p.len,vi);
		}
		item=vi;
	}
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	//wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_json_obj_remove(item,p.data,p.len);
	wtk_json_obj_add_str2(json,item,p.data,p.len,v.data,v.len);
	wtk_jsonkv_save_json(kv,inst.data,inst.len,json);
end:
	//exit(0);
	if(kv)
	{
		wtk_jsonkv_reset(kv);
	}
	return cnt;
}

int wtk_lua_jsonkv_add_array_value(lua_State *l)
{
	wtk_jsonkv_t *kv=NULL;
	int i,n;
	int cnt=0;
	wtk_string_t p;
	wtk_string_t inst;
	size_t len;
	wtk_json_t *json;
	wtk_json_item_t *item,*vi;
	int b=0;
	int x;

	n=lua_gettop(l);
	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	inst=p;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	json=wtk_jsonkv_get_json(kv,p.data,p.len);
	if(!json){goto end;}
	item=json->main;
	//wtk_debug("item=%p\n",item);
	for(++i;i<=n-2;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		p.data=(char*)lua_tolstring(l,i,&len);
		p.len=len;
		if(item->type!=WTK_JSON_OBJECT)
		{
			wtk_debug("[%.*s] must be object.\n",p.len,p.data);
			goto end;
		}
		vi=wtk_json_obj_get(item,p.data,p.len);
		if(!vi)
		{
			vi=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,item,p.data,p.len,vi);
			b=1;
		}
		item=vi;
	}
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	vi=wtk_json_obj_get(item,p.data,p.len);
	if(!vi)
	{
		vi=wtk_json_new_array(json);
		//wtk_json_array_add_str(json,vi,p.data,p.len);
		wtk_json_obj_add_item2(json,item,p.data,p.len,vi);
		b=1;
	}else if(vi->type!=WTK_JSON_ARRAY)
	{
		wtk_debug("[%.*s] value must be array\n",p.len,p.data);
		goto end;
	}
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	//wtk_debug("[%.*s]\n",p.len,p.data);
	x=wtk_json_array_has_string_value(vi,p.data,p.len);
	if(x==0)
	{
		wtk_json_array_add_str(json,vi,p.data,p.len);
		b=1;
	}
	if(b)
	{
		//wtk_json_item_print3(json->main);
		wtk_jsonkv_save_json(kv,inst.data,inst.len,json);
	}
end:
	//exit(0);
	if(kv)
	{
		wtk_jsonkv_reset(kv);
	}
	return cnt;
}

int wtk_lua_jsonkv_save_pvt(lua_State *l)
{
	wtk_jsonkv_t *kv;
	wtk_string_t p,v,t,section,value;
	int i;
	size_t len;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kv=(wtk_jsonkv_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	t.data=(char*)lua_tolstring(l,i,&len);
	t.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	section.data=(char*)lua_tolstring(l,i,&len);
	section.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	value.data=(char*)lua_tolstring(l,i,&len);
	value.len=len;
	//wtk_debug("get =================================\n");
	wtk_actkg_save_jsonkv_section(kv,&p,&v,&t,&section,&value);
end:
	return 0;
}

int wtk_lua_actkg_save_act(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_crfact_t *act;
	int i;
	int ret;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	act=(wtk_crfact_t*)lua_touserdata(l,i);
	ret=wtk_actkg_save_act(kg,act);
	lua_pushnumber(l,ret);
	cnt=1;
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_save_answer_act(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,k.data,k.len);
end:
	//wtk_debug("get =================================\n");
	return 0;
}

int wtk_lua_actkg_get_ext(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);

	if(kg->ext_ths)
	{
		lua_pushlightuserdata(l,kg->ext_ths);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_input(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);

	if(kg->input.len>0)
	{
		lua_pushlstring(l,kg->input.data,kg->input.len);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_process_nlg(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	k=wtk_actkg_process_nlg(kg,k.data,k.len);
	wtk_debug("[%.*s]\n",k.len,k.data)
	lua_pushlstring(l,k.data,k.len);
	cnt=1;
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_set_state(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("set state[%.*s]\n",k.len,k.data);
	wtk_nlgfst_set_state(kg->nlgfst,k.data,k.len);
end:
	//wtk_debug("get =================================\n");
	return 0;
}

void wtk_actkg_set_ask(wtk_actkg_t *kg,wtk_owl_item_t *item,wtk_string_t *p)
{
	wtk_owl_item_attr_kv_t *kv;
	wtk_string_t *xv;

	//wtk_owl_item_print(item);
	kg->owl_item=item;
	kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"ask_yes_set",0);
	if(kv)
	{
		kg->ask_yes_set=wtk_owl_item_attr_kv_get_value(kv,-1);
	}else
	{
		kg->ask_yes_set=item->k.str;
	}
	wtk_strbuf_reset(kg->ask_p);
	wtk_strbuf_push(kg->ask_p,p->data,p->len);
	xv=wtk_owl_item_find_attr_value_s(item,"ask_yes_key",-1);
	if(xv)
	{
		wtk_strbuf_reset(kg->ask_k);
		wtk_strbuf_push(kg->ask_k,xv->data,xv->len);
	}else
	{
		if(item->parent_item)
		{
			kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item->parent_item,"select",0);
			if(kv)
			{
				wtk_owl_item_print_path(item->parent_item,kg->ask_k);
			}else
			{
				wtk_owl_item_print_path(item,kg->ask_k);
			}
		}else
		{
			wtk_owl_item_print_path(item,kg->ask_k);
		}
	}
}

int wtk_lua_actkg_owlkv_get_next_nlg2(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_string_t p;
	wtk_string_t v;
	int cnt=0;
	int i;
	size_t len;
	wtk_owl_item_t *item;
	wtk_string_t *xv;
	wtk_nlg2_gen_env_t env;

	//wtk_debug("get next nlg\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	item=wtk_owlkv_get_ask_topic(kg->owlkv,p.data,p.len);
	if(!item)
	{
		wtk_debug("get topic failed\n");
		goto end;
	}
	//wtk_owl_item_print(item);
	xv=wtk_owl_item_find_attr_value_s(item,"ask",-1);
	if(!xv){goto end;}
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,xv,&p);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v.data,v.len);
	v=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	if(v.len>0)
	{
		wtk_actkg_set_ask(kg,item,&p);
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_actkg_owlkv_get_tuijian_nlg(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_string_t p;
	wtk_string_t v;
	int cnt=0;
	int i;
	size_t len;
	wtk_owl_item_t *item;
	wtk_string_t *xv;
	wtk_nlg2_gen_env_t env;

	//wtk_debug("get next nlg\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	item=wtk_owlkv_get_freeask_topic(kg->owlkv,p.data,p.len);
	if(!item)
	{
		wtk_debug("get topic failed\n");
		goto end;
	}
	//wtk_owl_item_print(item);
	xv=wtk_owl_item_find_attr_value_s(item,"ask",-1);
	if(!xv)
	{
		wtk_debug("get attr ask failed\n");
		goto end;
	}
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,xv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v.data,v.len);
	v=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	if(v.len>0)
	{
		wtk_actkg_set_ask(kg,item,&p);
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}


int wtk_lua_actkg_owlkv_get_next_nlg(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_string_t p;
	wtk_string_t v;
	int cnt=0;
	int i;
	size_t len;
	wtk_owl_item_t *item;
	wtk_string_t *xv;
	wtk_nlg2_gen_env_t env;

	//wtk_debug("get next nlg\n");
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	env.ths=kg;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_actkg_nlgfst_feed_nlg_lua;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	item=wtk_owlkv_get_freeask_topic(kg->owlkv,p.data,p.len);
	if(!item)
	{
		wtk_debug("get topic failed\n");
		goto end;
	}
	//wtk_owl_item_print(item);
	xv=wtk_owl_item_find_attr_value_s(item,"ask",-1);
	if(!xv)
	{
		wtk_debug("get attr ask failed\n");
		goto end;
	}
	v=wtk_owkv_get_owl_nlg(kg->owlkv,item,xv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v.data,v.len);
	v=wtk_owlkv_gen_text(kg->owlkv,p.data,p.len,v.data,v.len,&env);
	//v=wtk_owkv_get_owl_nlg_text(kg->owlkv,item,pv,&p);
	wtk_debug("[%.*s]\n",v.len,v.data);
	if(v.len>0)
	{
		wtk_actkg_set_ask(kg,item,&p);
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	return cnt;
}

int wtk_lua_actkg_owlkv_get_topic(lua_State *l)
{
	wtk_actkg_t *kg;
	wtk_string_t p;
	wtk_string_t v;
	int cnt=0;
	int i;
	size_t len;
	wtk_owl_item_t *item;
	wtk_owl_item_attr_kv_t *kv;
	wtk_string_t *xv;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	item=wtk_owlkv_get_topic2(kg->owlkv,p.data,p.len,v.data,v.len);
	if(item)
	{
		//wtk_owl_item_print(item);
		kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"ask",0);
		if(kv)
		{
			xv=wtk_owl_item_attr_kv_get_value(kv,-1);
			if(xv)
			{
				wtk_strbuf_t *buf;

				kg->owl_item=item;
				//wtk_owl_item_print(item);
				//exit(0);
				buf=wtk_strbuf_new(256,1);
				wtk_strbuf_push(buf,xv->data,xv->len);
				wtk_strbuf_push_s(buf,"(p=\"");
				wtk_strbuf_push(buf,p.data,p.len);
				wtk_strbuf_push_s(buf,"\"");
				kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"args",0);
				if(kv)
				{
					xv=wtk_owl_item_attr_kv_get_value(kv,-1);
					if(xv)
					{
						wtk_strbuf_push_s(buf,",");
						wtk_strbuf_push(buf,xv->data,xv->len);
					}
				}
				wtk_strbuf_push_s(buf,")");
				kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"ask_yes_set",0);
				if(kv)
				{
					kg->ask_yes_set=wtk_owl_item_attr_kv_get_value(kv,-1);
					wtk_strbuf_reset(kg->ask_p);
					wtk_strbuf_push(kg->ask_p,p.data,p.len);

					xv=wtk_owl_item_find_attr_value_s(item,"ask_yes_key",-1);
					if(xv)
					{
						wtk_strbuf_reset(kg->ask_k);
						wtk_strbuf_push(kg->ask_k,xv->data,xv->len);
					}else
					{
						if(item->parent_item)
						{
							kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item->parent_item,"select",0);
							if(kv)
							{
								wtk_owl_item_print_path(item->parent_item,kg->ask_k);
							}else
							{
								wtk_owl_item_print_path(item,kg->ask_k);
							}
						}else
						{
							wtk_owl_item_print_path(item,kg->ask_k);
						}
					}
					//wtk_debug("[%.*s]\n",kg->ask_k->pos,kg->ask_k->data);
					//exit(0);
				}
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				lua_pushlstring(l,buf->data,buf->pos);
				wtk_strbuf_delete(buf);
				cnt=1;
				goto end;
			}
		}
	}
end:
	return cnt;
}

int wtk_lua_actkg_next_topic(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;
	int n;
	wtk_owl_item_t *item;
	int cnt=0;
	wtk_owl_item_attr_kv_t *kv;
	wtk_string_t *xv;

	n=lua_gettop(l);
	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	for(i=2;i<=n;++i)
	{
		if(!lua_isstring(l,i)){goto end;}
		k.data=(char*)lua_tolstring(l,i,&len);
		k.len=len;
		//wtk_debug("v[%d]=[%.*s]\n",i,k.len,k.data);
		item=wtk_owlkv_get_ask_topic(kg->owlkv,k.data,k.len);
		//wtk_debug("item=%p\n",item);
		//exit(0);
		if(item)
		{
			//wtk_owl_item_print(item);
			kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"ask",0);
			if(kv)
			{
				xv=wtk_owl_item_attr_kv_get_value(kv,-1);
				if(xv)
				{
					wtk_strbuf_t *buf;

					kg->owl_item=item;
					//wtk_owl_item_print(item);
					buf=wtk_strbuf_new(256,1);
					wtk_strbuf_push(buf,xv->data,xv->len);
					wtk_strbuf_push_s(buf,"(p=\"");
					wtk_strbuf_push(buf,k.data,k.len);
					wtk_strbuf_push_s(buf,"\"");
					kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"args",0);
					if(kv)
					{
						xv=wtk_owl_item_attr_kv_get_value(kv,-1);
						if(xv)
						{
							wtk_strbuf_push_s(buf,",");
							wtk_strbuf_push(buf,xv->data,xv->len);
						}
					}
					wtk_strbuf_push_s(buf,")");
					kv=wtk_owl_tree_find_attr_s(kg->owlkv->owl,item,"ask_yes_set",0);
					if(kv)
					{
						kg->ask_yes_set=wtk_owl_item_attr_kv_get_value(kv,-1);
						wtk_strbuf_reset(kg->ask_p);
						wtk_strbuf_push(kg->ask_p,k.data,k.len);
						wtk_owl_item_print_path(item,kg->ask_k);
						//wtk_debug("[%.*s]\n",kg->ask_k->pos,kg->ask_k->data);
						//exit(0);
					}
					//wtk_debug("[%.*s]\n",buf->pos,buf->data);
					lua_pushlstring(l,buf->data,buf->pos);
					wtk_strbuf_delete(buf);
					cnt=1;
					goto end;
				}
			}
		}
	}
	//wtk_debug("set state[%.*s]\n",k.len,k.data);
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_v_expand(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	k=wtk_actkg_get_v_expand(kg,k.data,k.len);
	lua_pushlstring(l,k.data,k.len);
	cnt=1;
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_use_lex(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	int v;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	v=lua_tonumber(l,i);
	kg->use_lex=v;
end:
	//wtk_debug("get =================================\n");
	return 0;
}

int wtk_lua_actkg_get_class_inst(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k;
	size_t len;
	int cnt=0;
	wtk_lexr_t *lex;
	wtk_json_item_t *item;
	wtk_string_t *vp;
	wtk_heap_t *heap;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	lex=kg->lex;
	heap=kg->heap;
	//wtk_debug("[%.*s]\n",k.len,k.data);
	wtk_lexr_process(lex,kg->cfg->owlkv.net_class,k.data,k.len);
	item=wtk_json_obj_get_s(lex->action,"request");
	//wtk_json_item_print3(item);
	vp=wtk_actkg_get_norm_wrd_s(heap,item,NULL,0,"inst");
	if(vp)
	{
		lua_pushlstring(l,vp->data,vp->len);
	}else
	{
		lua_pushlstring(l,k.data,k.len);
	}
	vp=wtk_actkg_get_norm_wrd_s(heap,item,NULL,0,"class");
	if(vp)
	{
		lua_pushlstring(l,vp->data,vp->len);
	}else
	{
		lua_pushlstring(l,k.data,k.len);
	}
	cnt=2;
	wtk_lexr_reset(lex);
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_lex(lua_State *l)
{
	wtk_actkg_t *kg;
	int i;
	wtk_string_t k,input,section;
	size_t len;
	int cnt=0;
	wtk_lex_net_t *net;
	wtk_lexr_t *lex;
	wtk_json_item_t *item;
	wtk_string_t *vp;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("[%.*s]\n",k.len,k.data);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	input.data=(char*)lua_tolstring(l,i,&len);
	input.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	section.data=(char*)lua_tolstring(l,i,&len);
	section.len=len;
	wtk_debug("[%.*s]=[%.*s]\n",k.len,k.data,section.len,section.data);
	if(wtk_string_cmp_s(&(k),"rrt")==0)
	{
		net=kg->cfg->net_rrt;
		//wtk_debug("get rrt\n");
	}else if(wtk_string_cmp_s(&(k),"class")==0)
	{
		net=kg->cfg->owlkv.net_class;
	}else if(wtk_string_cmp_s(&(k),"vt")==0)
	{
		net=kg->cfg->net_vt;
	}else if(wtk_string_cmp_s(&(k),"rt")==0)
	{
		net=kg->cfg->net_rt;
	}else if(wtk_string_cmp_s(&(k),"pa")==0)
	{
		net=kg->cfg->net_pa;
	}else
	{
		goto end;
	}
	if(!net)
	{
		goto end;
	}
	lex=kg->lex;
	wtk_lexr_process(lex,net,input.data,input.len);
	//wtk_lexr_print(lex);
	if(lex->action && lex->action->type==WTK_JSON_OBJECT)
	{
		item=wtk_json_obj_get_s(lex->action,"request");
		if(item && item->type==WTK_JSON_OBJECT)
		{
			item=wtk_json_obj_get(item,section.data,section.len);
			if(item)
			{
				vp=wtk_json_item_get_str_value(item);
				if(vp)
				{
					lua_pushlstring(l,vp->data,vp->len);
					cnt=1;
				}
			}
		}
	}
	wtk_lexr_reset(lex);
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_class_kv(lua_State *l)
{
	wtk_actkg_t *kg;
	int cnt=0;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	lua_pushlightuserdata(l,kg->owlkv->class_kv);
	cnt=1;
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_actkg_get_inst_kv(lua_State *l)
{
	wtk_actkg_t *kg;
	int cnt=0;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_actkg_t*)lua_touserdata(l,i);
	lua_pushlightuserdata(l,kg->owlkv->inst_kv);
	cnt=1;
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_nlg_function_get(lua_State *l)
{
	wtk_nlg2_function_t *f;
	wtk_string_t k;
	wtk_string_t *v;
	int i;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	f=(wtk_nlg2_function_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("get =================================\n");
	v=wtk_nlg2_function_get(f,k.data,k.len);
	if(v && v->len>0)
	{
		lua_pushlstring(l,v->data,v->len);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

int wtk_lua_nlg_function_print(lua_State *l)
{
	wtk_nlg2_function_t *f;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	f=(wtk_nlg2_function_t*)lua_touserdata(l,i);
	wtk_nlg2_function_print(f);
end:
	//wtk_debug("get =================================\n");
	return 0;
}


void wtk_actkg_link_lua(wtk_lua2_t *lua2)
{
	wtk_owlkv_link_lua(lua2);
	wtk_lua2_link_function(lua2,wtk_lua_nlg_function_print,"wtk_nlg_function_print");
	wtk_lua2_link_function(lua2,wtk_lua_nlg_function_get,"wtk_nlg_function_get");

	wtk_lua2_link_function(lua2,wtk_lua_crfact_get,"wtk_crfact_get");
	wtk_lua2_link_function(lua2,wtk_lua_crfact_get_full,"wtk_crfact_get_full");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_new_act,"wtk_actkg_new_act");
	wtk_lua2_link_function(lua2,wtk_lua_crfact_print,"wtk_crfact_print");

	wtk_lua2_link_function(lua2,wtk_lua_jsonkv_save_pvt,"wtk_jsonkv_save_pvt");
	wtk_lua2_link_function(lua2,wtk_lua_jsonkv_get,"wtk_jsonkv_get");
	wtk_lua2_link_function(lua2,wtk_lua_jsonkv_add_array_value,"wtk_jsonkv_add_array_value");
	wtk_lua2_link_function(lua2,wtk_lua_jsonkv_add_object,"wtk_jsonkv_add_object");
	wtk_lua2_link_function(lua2,wtk_lua_jsonkv_set_str,"wtk_jsonkv_set_str");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_last_act,"wtk_actkg_get_last_act");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_nlg,"wtk_actkg_get_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_class_kv,"wtk_actkg_get_class_kv");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_inst_kv,"wtk_actkg_get_inst_kv");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_lex,"wtk_actkg_get_lex");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_use_lex,"wtk_actkg_use_lex");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_set_state,"wtk_actkg_set_state");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_next_topic,"wtk_actkg_next_topic");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_topic,"wtk_actkg_owlkv_get_topic");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_ask_value,"wtk_actkg_get_ask_value");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_ask_key,"wtk_actkg_get_ask_key");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_ask_p,"wtk_actkg_get_ask_p");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_ask_owl_item,"wtk_actkg_get_ask_owl_item");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_next_nlg,"wtk_actkg_owlkv_get_next_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_tuijian_nlg,"wtk_actkg_owlkv_get_tuijian_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_attr,"wtk_actkg_owlkv_get_attr");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_gen_text,"wtk_actkg_owlkv_gen_text");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_check_value,"wtk_actkg_owlkv_check_value");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_set_value,"wtk_actkg_owlkv_set_value");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_matched_value_item,"wtk_actkg_owlkv_get_matched_value_item");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_set_negative_value,"wtk_actkg_owlkv_set_negative_value");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_value,"wtk_actkg_owlkv_get_value");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owl_is_value_valid,"wtk_actkg_owl_is_value_valid");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_ans,"wtk_actkg_owlkv_ans");

	//owl_item=wtk_actkg_owlkv_get_rel(kg,"你","爸爸");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_rel,"wtk_actkg_owlkv_get_rel");

	//wtk_actkg_owlkv_get_owl_nlg_text(kg,owl_item,"set",p);
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_owl_nlg_text,"wtk_actkg_owlkv_get_owl_nlg_text");
	//wtk_actkg_owlkv_get_str_nlg_text(kg,xv,"ask",p); xv=pvt.喜欢
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_get_str_nlg_text,"wtk_actkg_owlkv_get_str_nlg_text");
	//wtk_actkg_owlkv_owl_item_get_attr(kg,owl_item,"next",p);
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_owl_item_get_attr,"wtk_actkg_owlkv_owl_item_get_attr");
	//wtk_actkg_owlkv_owl_item_get_next_nlg_text(kg,owl_item,"ask",p);  return nlg text
	wtk_lua2_link_function(lua2,wtk_lua_actkg_owlkv_owl_item_get_next_nlg_text,"wtk_actkg_owlkv_owl_item_get_next_nlg_text");


	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_class_inst,"wtk_actkg_get_class_inst");



	wtk_lua2_link_function(lua2,wtk_lua_actkg_save_act,"wtk_actkg_save_act");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_save_answer_act,"wtk_actkg_save_answer_act");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_process_nlg,"wtk_actkg_process_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_input,"wtk_actkg_get_input");
	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_ext,"wtk_actkg_get_ext");

	wtk_lua2_link_function(lua2,wtk_lua_actkg_get_v_expand,"wtk_actkg_get_v_expand");
}

wtk_actkg_history_t* wtk_actkg_history_new()
{
	wtk_actkg_history_t *h;

	h=(wtk_actkg_history_t*)wtk_malloc(sizeof(wtk_actkg_history_t));
	h->ans=wtk_strbuf_new(256,1);
	h->ask=wtk_strbuf_new(256,1);
	return h;
}

void wtk_actkg_history_delete(wtk_actkg_history_t *h)
{
	wtk_strbuf_delete(h->ans);
	wtk_strbuf_delete(h->ask);
	wtk_free(h);
}

wtk_string_t wtk_actkg_get_usr(wtk_actkg_t *kg,char *k,int k_bytes)
{
	wtk_string_t v;

	if(kg->lua && kg->cfg->lua_usr_get && wtk_str_equal_s(k,k_bytes,"你"))
	{
		v=wtk_actkg_nlgfst_feed_lua(kg,kg->cfg->lua_usr_get);
		return v;
	}else
	{
		wtk_string_set(&(v),0,0);
		return v;
	}
}

wtk_actkg_t* wtk_actkg_new(wtk_actkg_cfg_t *cfg,wtk_lua2_t *lua,wtk_lexr_t *lex)
{
	wtk_actkg_t *kg;
	int i;

	kg=(wtk_actkg_t*)wtk_malloc(sizeof(wtk_actkg_t));
	kg->cfg=cfg;
	kg->lua=lua;
	kg->lex=lex;
	kg->vkv=wtk_fkv_new3(cfg->v_expand_fn);
	if(cfg->lua_fn && lua)
	{
		wtk_string_t **strs;

		strs=(wtk_string_t**)(cfg->lua_fn->slot);
		for(i=0;i<cfg->lua_fn->nslot;++i)
		{
			wtk_lua2_load_file2(lua,strs[i]->data);
		}
		wtk_actkg_link_lua(lua);
	}
	kg->crfact=wtk_crfact_parser_new(&(cfg->crfact));
	kg->nlgfst=wtk_nlgfst_new(NULL,cfg->nlg_fn,cfg->fst_fn);
	//wtk_nlgnet_print(kg->nlgfst->net);
	//wtk_nlg_print(kg->nlgfst->nlg);
	//exit(0);
	kg->buf=wtk_strbuf_new(256,1);
	kg->tmp=wtk_strbuf_new(256,1);
	kg->owlkv=wtk_owlkv_new(&(cfg->owlkv),lex);
	if(lua)
	{
		kg->owlkv->inst_kv->get_map_ths=kg;
		kg->owlkv->inst_kv->get_map=(wtk_jsonkv_get_map_f)wtk_actkg_get_usr;
	}
	kg->heap=wtk_heap_new(4096);
	kg->ans_act=wtk_strbuf_new(256,1);
	kg->use_lex=0;
	kg->owl_item=NULL;
	kg->owl_item2=NULL;
	kg->ask_yes_set=NULL;
	kg->ask_yes_set2=NULL;
	kg->ask_p=wtk_strbuf_new(256,1);
	kg->ask_p2=wtk_strbuf_new(256,1);
	kg->ask_k=wtk_strbuf_new(256,1);
	kg->ask_k2=wtk_strbuf_new(256,1);
	kg->history=wtk_robin_new(cfg->nhist);
	for(i=0;i<cfg->nhist;++i)
	{
		kg->history->r[i]=wtk_actkg_history_new();
	}
	kg->ext_ths=NULL;
	return kg;
}

void wtk_actkg_delete(wtk_actkg_t *kg)
{
	int i;

	wtk_strbuf_delete(kg->ask_k);
	wtk_strbuf_delete(kg->ask_k2);
	wtk_strbuf_delete(kg->ask_p);
	wtk_strbuf_delete(kg->ask_p2);
	for(i=0;i<kg->cfg->nhist;++i)
	{
		wtk_actkg_history_delete(kg->history->r[i]);
	}
	wtk_robin_delete(kg->history);
	wtk_owlkv_delete(kg->owlkv);
	wtk_strbuf_delete(kg->ans_act);
	wtk_heap_delete(kg->heap);
	wtk_strbuf_delete(kg->tmp);
	wtk_strbuf_delete(kg->buf);
	wtk_nlgfst_delete(kg->nlgfst);
	wtk_crfact_parser_delete(kg->crfact);
	wtk_free(kg);
}

void wtk_actkg_reset(wtk_actkg_t *kg)
{
	wtk_strbuf_reset(kg->ans_act);
	wtk_heap_reset(kg->heap);
	wtk_strbuf_reset(kg->buf);
}

void wtk_actkg_set_ext(wtk_actkg_t *kg,void *ths)
{
	kg->ext_ths=ths;
}

wtk_string_t wtk_actkg_get_output(wtk_actkg_t *kg)
{
	wtk_string_t v;

	wtk_string_set(&(v),kg->buf->data,kg->buf->pos);
	return v;
}

wtk_string_t wtk_actkg_get_v_expand(wtk_actkg_t *kg,char *v,int v_bytes)
{
	wtk_string_t *vp;
	wtk_string_t t;

	wtk_string_set(&(t),0,0);
	vp=wtk_fkv_get_str(kg->vkv,v,v_bytes);
	if(!vp){goto end;}
	wtk_string_set(&(t),vp->data,vp->len);
end:
	return t;
}

wtk_string_t wtk_actkg_process_nlg(wtk_actkg_t *kg,char *v,int v_bytes)
{
	wtk_strbuf_reset(kg->ans_act);
	wtk_strbuf_push(kg->ans_act,v,v_bytes);
	return wtk_nlg_process_nlg_str2(kg->nlgfst->nlg,v,v_bytes,kg,(wtk_nlg_act_get_lua_f)wtk_actkg_nlgfst_feed_lua);
}

void wtk_actkg_save_jsonkv(wtk_jsonkv_t *kv,wtk_string_t *p,wtk_string_t *v,wtk_string_t *t,wtk_string_t *a,wtk_string_t *av)
{
	wtk_json_t *json;
	wtk_json_item_t *vi,*ti;
	int b=0;
	int x;

	json=wtk_jsonkv_get_json(kv,p->data,p->len);
	if(!json){goto end;}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	vi=wtk_json_obj_get(json->main,v->data,v->len);
	if(!vi)
	{
		vi=wtk_json_new_object(json);
		wtk_json_obj_add_item2(json,json->main,v->data,v->len,vi);
		b=1;
	}
	//wtk_debug("[%.*s]\n",t->len,t->data);
	ti=wtk_json_obj_get(vi,t->data,t->len);
	if(!ti)
	{
		ti=wtk_json_new_object(json);
		wtk_json_obj_add_item2(json,vi,t->data,t->len,ti);
		b=1;
	}
	if(a)
	{
		vi=wtk_json_obj_get(ti,a->data,a->len);
		if(!vi)
		{
			vi=wtk_json_new_array(json);
			wtk_json_array_add_str(json,vi,av->data,av->len);
			wtk_json_obj_add_item2(json,ti,a->data,a->len,vi);
			b=1;
		}else
		{
			x=wtk_json_array_has_string_value(vi,av->data,av->len);
			if(x==0)
			{
				wtk_json_array_add_str(json,vi,av->data,av->len);
				b=1;
			}
		}
	}
	if(b)
	{
		//wtk_json_item_print3(json->main);
		wtk_jsonkv_save_json(kv,p->data,p->len,json);
	}
end:
	return;
}

void wtk_actkg_save_jsonkv_section(wtk_jsonkv_t *kv,wtk_string_t *p,wtk_string_t *v,wtk_string_t *t,wtk_string_t *section,wtk_string_t *av)
{
	wtk_json_t *json;
	wtk_json_item_t *vi,*ti;
	int b=0;
	int x;

	json=wtk_jsonkv_get_json(kv,p->data,p->len);
	if(!json){goto end;}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	vi=wtk_json_obj_get(json->main,v->data,v->len);
	if(!vi)
	{
		vi=wtk_json_new_object(json);
		wtk_json_obj_add_item2(json,json->main,v->data,v->len,vi);
		b=1;
	}
	//wtk_debug("[%.*s]\n",t->len,t->data);
	ti=wtk_json_obj_get(vi,t->data,t->len);
	if(!ti)
	{
		ti=wtk_json_new_object(json);
		wtk_json_obj_add_item2(json,vi,t->data,t->len,ti);
		b=1;
	}
	vi=wtk_json_obj_get(ti,section->data,section->len);
	if(!vi)
	{
		vi=wtk_json_new_array(json);
		wtk_json_array_add_str(json,vi,av->data,av->len);
		wtk_json_obj_add_item2(json,ti,section->data,section->len,vi);
		b=1;
	}else
	{
		x=wtk_json_array_has_string_value(vi,av->data,av->len);
		if(x==0)
		{
			wtk_json_array_add_str(json,vi,av->data,av->len);
			b=1;
		}
	}
	if(b)
	{
		//wtk_json_item_print3(json->main);
		wtk_jsonkv_save_json(kv,p->data,p->len,json);
	}
end:
	return;
}


wtk_string_t* wtk_actkg_get_jsonkv(wtk_jsonkv_t *kv,wtk_string_t *p,wtk_string_t *v,wtk_string_t *t,wtk_string_t *a)
{
	wtk_json_t *json;
	wtk_json_item_t *vi,*ti;
	wtk_string_t *pv=NULL;

	json=wtk_jsonkv_get_json(kv,p->data,p->len);
	if(!json){goto end;}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	vi=wtk_json_obj_get(json->main,v->data,v->len);
	if(!vi || vi->type!=WTK_JSON_OBJECT){goto end;}
	//wtk_debug("[%.*s]\n",t->len,t->data);
	if(t)
	{
		ti=wtk_json_obj_get(vi,t->data,t->len);
		if(!ti || ti->type!=WTK_JSON_OBJECT)
		{
			goto end;
		}
	}else
	{
		ti=vi;
	}
	vi=wtk_json_obj_get(ti,a->data,a->len);
	if(!vi){goto end;}
	pv=wtk_json_item_get_str_value(vi);
end:
	return pv;
}

int wtk_actkg_save_act(wtk_actkg_t *kg,wtk_crfact_t *act)
{
	exit(0);
//	int ret=-1;
//	wtk_string_t *p,*v,*t;
//
//	//wtk_crfact_print(act);
//	if(!act->item->p || !act->item->p->value|| !act->item->p->value->inst){goto end;}
//	p=act->item->p->value->inst;
////	if(!act->item->v || !act->item->v->value|| !act->item->v->value->inst){goto end;}
////	v=act->item->v->value->inst;
//	v=NULL;
//	if(!act->item->t || !act->item->t->value|| !act->item->t->value->inst){goto end;}
//	t=act->item->t->value->inst;
//	wtk_actkg_save_jsonkv(kg->inst_kv,p,v,t,NULL,NULL);
//	wtk_jsonkv_reset(kg->inst_kv);
//	ret=0;
//end:
//	//exit(0);
//	return ret;
	exit(0);
	return 0;
}

/**
 * return 0 for success, other break;
 */
wtk_string_t wtk_actkg_nlgfst_feed_lua(wtk_actkg_t *kg,char *func)
{
	wtk_string_t v;
	wtk_lua2_arg_t arg[4];
	wtk_strbuf_t *buf=kg->tmp;

	wtk_string_set(&(v),0,0);
	//wtk_debug("lua=[%s]\n",func);
	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=kg;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=kg->act;
	arg[2].type=WTK_LUA2_THS;
	arg[2].v.ths=kg->ext_ths;
	//wtk_debug("=================> semdlg=%p %s\n",arg[2].v.ths,func);
	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(kg->lua,func,buf,arg,arg+1,arg+2,NULL);
	if(buf->pos>0)
	{
		wtk_string_set(&(v),buf->data,buf->pos);
	}
	return v;
}

int wtk_actkg_nlgfst_has_key(wtk_actkg_t *kg,char *k,int k_bytes)
{
	//wtk_debug("[%.*s]\n",k_bytes,k);
	//exit(0);
	if(!kg->act || !kg->act->item)
	{
		return 0;
	}
	return wtk_crfact_item_has_key(kg->act->item,k,k_bytes);
}

wtk_string_t* wtk_actkg_nlgfst_get_value(wtk_actkg_t *kg,char *k,int k_bytes)
{
	//wtk_debug("[%.*s]\n",k_bytes,k);
	return wtk_crfact_get_value(kg->act,k,k_bytes);
}

int wtk_actkg_nlgfst_nvalue(wtk_actkg_t *kg)
{

	if(!kg->act->item)
	{
		return 0;
	}
	return wtk_crfact_item_nvalue(kg->act->item);
}



wtk_string_t* wtk_actkg_get_norm_wrd(wtk_heap_t *heap,wtk_json_item_t *item,char *data,int len,char *k,int k_bytes)
{
	wtk_string_t *vp=NULL;

	if(item && item->type==WTK_JSON_OBJECT)
	{
		item=wtk_json_obj_get(item,k,k_bytes);
		if(item)
		{
			vp=wtk_json_item_get_str_value(item);
			if(vp)
			{
				//wtk_debug("[%.*s]\n",vp->len,vp->data);
				vp=wtk_heap_dup_string(heap,vp->data,vp->len);
			}
		}
	}
	if(!vp && len>0)
	{
		vp=wtk_heap_dup_string(heap,data,len);
	}
	return vp;
}

wtk_string_t* wtk_actkg_norm_lex(wtk_actkg_t *kg,wtk_lex_net_t *net,char *data,int len,char *k,int k_bytes)
{
	wtk_lexr_t *lex=kg->lex;
	wtk_heap_t *heap=kg->heap;
	wtk_json_item_t *item;
	wtk_string_t *vp=NULL;

	wtk_lexr_process(lex,net,data,len);
	if(lex->action && lex->action->type==WTK_JSON_OBJECT)
	{
		item=wtk_json_obj_get_s(lex->action,"request");
		if(item && item->type==WTK_JSON_OBJECT)
		{
			item=wtk_json_obj_get(item,k,k_bytes);
			if(item)
			{
				vp=wtk_json_item_get_str_value(item);
				if(vp)
				{
					//wtk_debug("[%.*s]\n",vp->len,vp->data);
					vp=wtk_heap_dup_string(heap,vp->data,vp->len);
				}
			}
		}
	}
	if(!vp)
	{
		vp=wtk_heap_dup_string(heap,data,len);
	}
	wtk_lexr_reset(lex);
	return vp;
}

void wtk_actkg_update_act_class(wtk_actkg_t *kg,wtk_crfact_t *act)
{
	wtk_lexr_t *lex=kg->lex;
	wtk_heap_t *heap=kg->heap;
	wtk_json_item_t *item;

	wtk_lexr_process(lex,kg->cfg->owlkv.net_class,act->item->p->wrd->data,act->item->p->wrd->len);
	item=wtk_json_obj_get_s(lex->action,"request");
	//wtk_json_item_print3(item);
	act->item->p->inst=wtk_actkg_get_norm_wrd_s(heap,item,act->item->p->wrd->data,act->item->p->wrd->len,"inst");
	act->item->p->cls=wtk_actkg_get_norm_wrd_s(heap,item,act->item->p->wrd->data,act->item->p->wrd->len,"class");
	wtk_lexr_reset(lex);
}

void wtk_actkg_update_act_class2(wtk_actkg_t *kg,wtk_crfact_t *act,wtk_string_t *v)
{
	wtk_lexr_t *lex=kg->lex;
	wtk_heap_t *heap=kg->heap;
	wtk_json_item_t *item;

	wtk_lexr_process(lex,kg->cfg->owlkv.net_class,v->data,v->len);
	item=wtk_json_obj_get_s(lex->action,"request");
	if(item)
	{
		act->item->p->cls=wtk_actkg_get_norm_wrd_s(heap,item,v->data,v->len,"class");
	}
	wtk_lexr_reset(lex);
}

void wtk_actkg_norm(wtk_actkg_t *kg,wtk_crfact_t *act)
{
	wtk_lexr_t *lex=kg->lex;
	wtk_heap_t *heap=kg->heap;
	wtk_json_item_t *item;
	wtk_string_t v;

//	//wtk_crfact_print(act);
	//wtk_debug("class=%p inst=%p\n",kg->cfg->owlkv.net_class,kg->cfg->owlkv.net_inst);
	if(act->item->p && act->item->p && act->item->p->wrd)
	{
		if(kg->cfg->owlkv.net_class)
		{
			wtk_actkg_update_act_class(kg,act);
			if(kg->lua && act->item->p->inst && wtk_string_cmp_s(act->item->p->inst,"你")==0)
			{
				v=wtk_actkg_nlgfst_feed_lua(kg,kg->cfg->lua_usr_get);
				//wtk_debug("[%.*s]\n",v.len,v.data);
				if(v.len>0)
				{
					wtk_actkg_update_act_class2(kg,act,&v);
				}
			}
		}
		if(act->item->p->inst && act->item->p->cls)
		{
			wtk_owlkv_touch_inst(kg->owlkv,act->item->p->inst->data,act->item->p->inst->len,act->item->p->cls->data,act->item->p->cls->len);
		}
	}
	if(act->item->t && act->item->t && act->item->t->wrd)
	{
		if(kg->cfg->owlkv.net_class)
		{
			//act->item->t->cls=wtk_actkg_norm_lex_s(kg,kg->cfg->owlkv.net_class,act->item->t->wrd->data,act->item->t->wrd->len,"class");
			//wtk_debug("[%.*s]\n",act->item->t->value->cls->len,act->item->t->value->cls->data);
			wtk_lexr_process(lex,kg->cfg->owlkv.net_class,act->item->t->wrd->data,act->item->t->wrd->len);
			item=wtk_json_obj_get_s(lex->action,"request");
			//wtk_json_item_print3(item);
			act->item->t->cls=wtk_actkg_get_norm_wrd_s(heap,item,act->item->t->wrd->data,act->item->t->wrd->len,"class");
			act->item->t->inst=wtk_actkg_get_norm_wrd_s(heap,item,act->item->t->wrd->data,act->item->t->wrd->len,"inst");
			wtk_lexr_reset(lex);
		}
	}
	if(kg->cfg->net_vt)
	{
		if(act->item->vt && act->item->vt->wrd)
		{
			wtk_strbuf_t *buf;

			buf=wtk_strbuf_new(256,1);
			wtk_crfact_item_merge_vt(act->item,buf);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			//act->item->vt->inst=wtk_actkg_norm_lex_s(kg,kg->cfg->net_vt,act->item->vt->wrd->data,act->item->vt->wrd->len,"inst");
			act->item->vt->inst=wtk_actkg_norm_lex_s(kg,kg->cfg->net_vt,buf->data,buf->pos,"inst");
			wtk_strbuf_delete(buf);
		}
		if(act->item->pv && act->item->pv->wrd)
		{
			//wtk_debug("[%.*s]\n",act->item->pv->wrd->len,act->item->pv->wrd->data);
			act->item->pv->inst=wtk_actkg_norm_lex_s(kg,kg->cfg->net_vt,act->item->pv->wrd->data,act->item->pv->wrd->len,"inst");
		}
		//wtk_debug("[%.*s]\n",act->item->v->value->inst->len,act->item->v->value->inst->data);
	}
	if(kg->cfg->net_rt && act->item->rt && act->item->rt->wrd)
	{
		act->item->rt->inst=wtk_actkg_norm_lex_s(kg,kg->cfg->net_rt,act->item->rt->wrd->data,act->item->rt->wrd->len,"inst");
	}
	//wtk_crfact_print(act);
	//exit(0);
//	//inst
//	//exit(0);
	//if(act->item->p && act->item->p->inst && wtk_string_cmp_s(act->item->p->inst))
	return;
}

wtk_crfact_t* wtk_actkg_process_lex(wtk_actkg_t *kg,char *s,int len)
{
	wtk_json_item_t *item;
	wtk_lexr_t *lex=kg->lex;
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *ji;
	wtk_string_t *v;
	wtk_crfact_t *act=NULL;

	wtk_lexr_process(lex,kg->cfg->net_parser,s,len);
	wtk_lexr_print(lex);
	if(lex->action)
	{
		item=wtk_json_obj_get_s(lex->action,"request");
		if(item && item->type==WTK_JSON_OBJECT)
		{
			for(qn=item->v.object->pop;qn;qn=qn->next)
			{
				ji=data_offset(qn,wtk_json_obj_item_t,q_n);
				v=wtk_json_item_get_str_value(ji->item);
				if(ji && v)
				{
					if(!act)
					{
						act=wtk_crfact_new(kg->heap);
					}
					//wtk_debug("[%.*s]=[%.*s]\n",ji->k.len,ji->k.data,v->len,v->data);
					wtk_crfact_set(act,kg->heap,ji->k.data,ji->k.len,v->data,v->len);
				}
			}
		}
	}
	//wtk_crfact_print(act);
	//exit(0);
	wtk_lexr_reset(lex);
	return act;
}

wtk_string_t wtk_actkg_process(wtk_actkg_t *kg,char *s,int len)
{
	wtk_crfact_t *act;
	wtk_nlgfst_act_t fst_act;
	wtk_string_t v;

	wtk_string_set(&(kg->input),s,len);
	wtk_strbuf_reset(kg->buf);
	kg->act=NULL;
	kg->owl_item2=kg->owl_item;
	kg->owl_item=NULL;
	kg->ask_yes_set2=kg->ask_yes_set;
	kg->ask_yes_set=NULL;
	wtk_strbuf_reset(kg->ask_k2);
	if(kg->ask_k->pos>0)
	{
		wtk_strbuf_push(kg->ask_k2,kg->ask_k->data,kg->ask_k->pos);
		wtk_strbuf_reset(kg->ask_k);
	}
	wtk_strbuf_reset(kg->ask_p2);
	if(kg->ask_p->pos>0)
	{
		wtk_strbuf_push(kg->ask_p2,kg->ask_p->data,kg->ask_p->pos);
		wtk_strbuf_reset(kg->ask_p);
	}
	wtk_string_set(&(v),0,0);
	if(kg->use_lex && kg->cfg->net_parser)
	{
		act=wtk_actkg_process_lex(kg,s,len);
		kg->use_lex=0;
	}else
	{
		wtk_debug("process lex\n");
		act=wtk_actkg_process_lex(kg,s,len);
		if(!act)
		{
			act=wtk_crfact_parser_process(kg->crfact,s,len);
		}
	}
	if(!act){goto end;}
	//wtk_crfact_print(act);
	wtk_actkg_norm(kg,act);
	//wtk_debug("========= input act ==========\n");
	wtk_crfact_print(act);
	//exit(0);
	kg->act=act;
	fst_act.ths=kg;
	fst_act.buf=kg->buf;
	fst_act.feed_lua=(wtk_nlg_act_get_lua_f)wtk_actkg_nlgfst_feed_lua;
	fst_act.has_key=(wtk_nlgfst_act_has_key_f)wtk_actkg_nlgfst_has_key;
	fst_act.get_value=(wtk_nlg_act_get_value_f)wtk_actkg_nlgfst_get_value;
	fst_act.nvalue=(wtk_nlgfst_act_nvalue_f)wtk_actkg_nlgfst_nvalue;
	wtk_debug("[%.*s]\n",kg->nlgfst->cur_state->name->len,kg->nlgfst->cur_state->name->data);
	wtk_nlgfst_feed2(kg->nlgfst,&(fst_act));
	//wtk_debug("[%.*s]\n",kg->buf->pos,kg->buf->data);
	wtk_string_set(&(v),kg->buf->data,kg->buf->pos);
	if(act->item && kg->ans_act->pos>0)
	{
		wtk_actkg_history_t* history;

		history=(wtk_actkg_history_t*)wtk_robin_next(kg->history);
		wtk_strbuf_reset(history->ask);
		wtk_crfact_item_print2(act->item,history->ask);
		wtk_strbuf_reset(history->ans);
		wtk_strbuf_push(history->ans,kg->ans_act->data,kg->ans_act->pos);
	}
end:
	//exit(0);
	return v;
}



#endif
