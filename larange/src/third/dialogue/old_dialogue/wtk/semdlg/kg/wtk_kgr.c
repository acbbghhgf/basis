#include "wtk_kgr.h" 
#include "wtk/semdlg/semfld/wtk_semfld.h"
#include "wtk/semdlg/wtk_semdlg.h"
void wtk_kgr_touch_inst(wtk_kgr_t *kgr,char *p,int len);
wtk_kg_item_next_item_t* wtk_kgr_get_next(wtk_kgr_t *r,wtk_string_t *p,wtk_kg_item_t *item,wtk_kg_item_t *cur);
wtk_string_t wtk_kgr_process_nlg(wtk_kgr_t *r,char* nlg,int len);
wtk_string_t* wtk_kgr_get_kv(wtk_kgr_t *r,char *p,int p_len,char *k,int k_len);
void wtk_kgr_set_kv(wtk_kgr_t *r,char *p,int p_len,char *k,int k_len,char *v,int v_len);
wtk_string_t wtk_kgr_feed_lua(wtk_kgr_t *kgr,char *func);

int wtk_lua_kgr_touch_inst(lua_State *l)
{
	wtk_kgr_t *kg;
	int i;
	wtk_string_t p;
	size_t len;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	wtk_kgr_touch_inst(kg,p.data,p.len);
end:
	//wtk_debug("cnt=%d\n",cnt);
	return 0;
}

int wtk_lua_kgr_get_next(lua_State *l)
{
	wtk_kgr_t *kg;
	int i;
	wtk_string_t p,k;
	size_t len;
	int cnt=0;
	wtk_kg_item_t *item;
	wtk_kg_item_next_item_t *ni;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	item=wtk_kg_class_get_item(kg->cfg->kg,kg->cfg->kg->_class,k.data,k.len,0);
	if(!item || !item->next){goto end;}
	ni=wtk_kgr_get_next(kg,&p,item,NULL);
	if(!ni){goto end;}
	if(ni->is_item)
	{
		lua_pushlightuserdata(l,ni->v.item);
	}else
	{
		lua_pushstring(l,ni->v.lua);
	}
	cnt=1;
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_kg_item_get_attr(lua_State *l)
{
	wtk_kg_item_t *item;
	int i;
	wtk_string_t p;
	size_t len;
	wtk_string_t *v;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	item=(wtk_kg_item_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	v=wtk_kg_item_get_attr(item,p.data,p.len);
	if(v)
	{
		lua_pushlstring(l,v->data,v->len);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_kgr_process_nlg(lua_State *l)
{
	wtk_kgr_t *kg;
	int i;
	wtk_string_t p;
	size_t len;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	p=wtk_kgr_process_nlg(kg,p.data,p.len);
	if(p.len>0)
	{
		lua_pushlstring(l,p.data,p.len);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_set_next_item(lua_State *l)
{
	wtk_kgr_t *kg;
	wtk_kg_item_t *item;
	int i;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i)){goto end;}
	item=lua_touserdata(l,i);
	kg->next=item;
end:
	//wtk_debug("cnt=%d\n",cnt);
	return 0;
}

int wtk_lua_kgr_set_last_p(lua_State *l)
{
	wtk_kgr_t *kg;
	wtk_string_t p;
	size_t len;
	int i;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	wtk_strbuf_reset(kg->last_p);
	wtk_strbuf_push(kg->last_p,p.data,p.len);
	wtk_kgr_touch_inst(kg,p.data,p.len);
end:
	//wtk_debug("cnt=%d\n",cnt);
	return 0;
}

int wtk_lua_kgr_get_last_p(lua_State *l)
{
	wtk_kgr_t *kg;
	int i;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	if(kg->last_p->pos>0)
	{
		lua_pushlstring(l,kg->last_p->data,kg->last_p->pos);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_kgr_set_kv(lua_State *l)
{
	wtk_kgr_t *kg;
	wtk_string_t p,k,v;
	size_t len;
	int i;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
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
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_kgr_set_kv(kg,p.data,p.len,k.data,k.len,v.data,v.len);
end:
	return 0;
}

int wtk_lua_kgr_get_item(lua_State *l)
{
	wtk_kgr_t *kg;
	wtk_string_t p;
	size_t len;
	int i;
	int cnt=0;
	wtk_kg_item_t *item;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	item=wtk_kg_class_get_item(kg->cfg->kg,kg->cfg->kg->_class,p.data,p.len,0);
	if(item)
	{
		lua_pushlightuserdata(l,item);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}


int wtk_lua_kgr_get_kv(lua_State *l)
{
	wtk_kgr_t *kg;
	wtk_string_t p,k,*v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	v=wtk_kgr_get_kv(kg,p.data,p.len,k.data,k.len);
	if(v)
	{
		lua_pushlstring(l,v->data,v->len);
		wtk_string_delete(v);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

int wtk_lua_kgr_get_cur_item_name(lua_State *l)
{
	wtk_kgr_t *kg;
	int cnt=0;
	int i;

	i=1;
	//wtk_debug("n=%d\n",n);
	if(!lua_isuserdata(l,i)){goto end;}
	kg=(wtk_kgr_t*)lua_touserdata(l,i);
	if(kg->cur)
	{
		lua_pushlstring(l,kg->cur->name->data,kg->cur->name->len);
		cnt=1;
	}
end:
	//wtk_debug("cnt=%d\n",cnt);
	return cnt;
}

void wtk_kgr_link_lua(wtk_lua2_t *lua2)
{
	wtk_lua2_link_function(lua2,wtk_lua_kgr_touch_inst,"wtk_kgr_touch_inst");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_get_next,"wtk_kgr_get_next");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_process_nlg,"wtk_kgr_process_nlg");
	wtk_lua2_link_function(lua2,wtk_lua_set_next_item,"wtk_kgr_set_next_item");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_set_last_p,"wtk_kgr_set_last_p");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_get_last_p,"wtk_kgr_get_last_p");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_get_item,"wtk_kgr_get_item");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_get_kv,"wtk_kgr_get_kv");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_set_kv,"wtk_kgr_set_kv");
	wtk_lua2_link_function(lua2,wtk_lua_kgr_get_cur_item_name,"wtk_kgr_get_cur_item_name");


	wtk_lua2_link_function(lua2,wtk_lua_kg_item_get_attr,"wtk_kg_item_get_attr");
}



wtk_string_t wtk_kgr_get_json_map(wtk_kgr_t *kg,char *k,int k_len)
{
	wtk_strbuf_t *buf=kg->tmp;
	wtk_lua2_arg_t arg[5];
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	if(!kg->cfg->lua_json_map)
	{
		return v;
	}
	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=kg->fld->dlg;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=kg->fld;
	arg[2].type=WTK_LUA2_STRING;
	arg[2].v.str.data=k;
	arg[2].v.str.len=k_len;
	arg[3].type=WTK_LUA2_THS;
	arg[3].v.ths=kg;
	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(kg->fld->dlg->lua,kg->cfg->lua_json_map,buf,arg,arg+1,arg+2,arg+3,NULL);
	if(buf->pos>0)
	{
		wtk_string_set(&(v),buf->data,buf->pos);
	}
	return v;
}

wtk_kgr_t* wtk_kgr_new(wtk_kgr_cfg_t *cfg,struct wtk_semfld *fld,wtk_rbin2_t *rbin)
{
	wtk_kgr_t *r;

	r=(wtk_kgr_t*)wtk_malloc(sizeof(wtk_kgr_t));
	r->cfg=cfg;
	r->fld=fld;
	r->inst_kv=wtk_jsonkv_new(cfg->inst_dn);
	r->inst_kv->get_map_ths=r;
	r->inst_kv->get_map=(wtk_jsonkv_get_map_f)wtk_kgr_get_json_map;
	r->nlg=wtk_nlg2_new_fn(cfg->nlg_fn,rbin);
	r->fst=wtk_nlgfst2_new(r->nlg);
	r->tmp=wtk_strbuf_new(256,1);
	r->last_p=wtk_strbuf_new(256,1);
	wtk_kgr_reset(r);
	return r;
}

void wtk_kgr_update_env(wtk_kgr_t *kg)
{
	wtk_strbuf_t *buf=kg->tmp;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push_string(buf,kg->cfg->inst_dn);
	if(kg->fld->dlg->env.client.len>0)
	{
		wtk_strbuf_push_s(buf,"/");
		wtk_strbuf_push(buf,kg->fld->dlg->env.client.data,kg->fld->dlg->env.client.len);
	}
	wtk_strbuf_push_c(buf,0);
	wtk_jsonkv_set_dn(kg->inst_kv,buf->data);
}

void wtk_kgr_delete(wtk_kgr_t *r)
{
	wtk_strbuf_delete(r->last_p);
	wtk_strbuf_delete(r->tmp);
	wtk_nlgfst2_delete(r->fst);
	wtk_nlg2_delete(r->nlg);
	wtk_jsonkv_delete(r->inst_kv);
	wtk_free(r);
}

void wtk_kgr_reset(wtk_kgr_t *r)
{
	r->last=NULL;
	r->next=NULL;
}

int wtk_kgr_can_be_end(wtk_kgr_t *r)
{
	if(!r->fst->cur_state || r->fst->cur_state==r->fst->net->end)
	{
		return 1;
	}
	return 0;
}

void wtk_kgr_update_inst(wtk_kgr_t *r,wtk_json_t *json,wtk_string_t *p)
{
	wtk_kg_inst_t* inst;
	wtk_queue_node_t *qn;
	wtk_kg_inst_value_t *value;
	wtk_json_item_t *ji;
	int i;

	inst=wtk_kg_get_inst(r->cfg->kg,p->data,p->len,0);
	//wtk_debug("[%.*s]=%p\n",p->len,p->data,inst);
	if(p && wtk_string_cmp_s(p,"你")!=0)
	{
		wtk_json_item_set_path_str_s(json,"属性.姓名",p->data,p->len);
	}
	if(!inst){goto end;}
	for(qn=inst->item_q.pop;qn;qn=qn->next)
	{
		value=data_offset2(qn,wtk_kg_inst_value_t,q_n);
		//wtk_debug("type=%d\n",value->item->vtype);
		switch(value->item->vtype)
		{
		case WTK_KG_ITEM_INT:
			ji=wtk_json_item_add_path_item(json,json->main,value->item->name->data,value->item->name->len,WTK_JSON_NUMBER);
			ji->v.number=value->v.i;
			break;
		case WTK_KG_ITEM_STR:
			ji=wtk_json_item_add_path_item(json,json->main,value->item->name->data,value->item->name->len,WTK_JSON_STRING);
			ji->v.str=value->v.str;
			break;
		case WTK_KG_ITEM_ARRAY:
			ji=wtk_json_item_add_path_item(json,json->main,value->item->name->data,value->item->name->len,WTK_JSON_ARRAY);
			for(i=0;i<value->v.a.n;++i)
			{
				wtk_json_array_add_ref_str(json,ji,value->v.a.strs[i]);
			}
			break;
		}
	}
end:
	return;
}

void wtk_kgr_touch_inst(wtk_kgr_t *r,char *p,int len)
{
	wtk_jsonkv_t *kv=r->inst_kv;
	wtk_json_t *json;
	wtk_string_t v;

	json=wtk_jsonkv_get_json(kv,p,len);
	//wtk_json_item_print3(json->main);
	if(wtk_json_item_len(json->main)==0)
	{
		//如果inst第一次,更新默认值;
		if(kv->get_map)
		{
			v=kv->get_map(kv->get_map_ths,p,len);
			//wtk_debug("[%.*s]\n",v.len,v.data);
			if(v.len<=0)
			{
				wtk_string_set(&(v),p,len);
			}
		}else
		{
			wtk_string_set(&(v),p,len);
		}
		wtk_kgr_update_inst(r,json,&(v));
		wtk_jsonkv_save_json(kv,p,len,json);
	}
	wtk_jsonkv_reset(kv);
}

void wtk_kgr_lua_gen(wtk_kgr_t *r,char *func,wtk_nlg2_function_t *f,wtk_strbuf_t *buf)
{
	wtk_semfld_feed_lua(r->fld,r->fld->input_act,func,NULL,buf);
}

void wtk_kgr_set_kv(wtk_kgr_t *r,char *p,int p_len,char *k,int k_len,char *v,int v_len)
{
	wtk_jsonkv_t *kv=r->inst_kv;
	wtk_json_t *json;

	r->dirty=1;
	json=wtk_jsonkv_get_json(kv,p,p_len);
	//wtk_debug("[%.*s]=[%.*s]\n",k_len,k,v_len,v);
	wtk_json_item_set_path_str(json,k,k_len,v,v_len);
	wtk_jsonkv_save_json(kv,p,p_len,json);
	wtk_jsonkv_reset(kv);
}

wtk_string_t* wtk_kgr_get_kv(wtk_kgr_t *r,char *p,int p_len,char *k,int k_len)
{
	wtk_jsonkv_t *kv=r->inst_kv;
	wtk_json_t *json;
	wtk_json_item_t *ji;
	wtk_string_t *t=NULL;

	r->dirty=1;
	json=wtk_jsonkv_get_json(kv,p,p_len);
	ji=wtk_json_item_get_path_item(json->main,k,k_len,NULL);
	if(ji)
	{
		t=wtk_json_item_get_str_value2(ji);
	}
	wtk_jsonkv_reset(kv);
	return t;
}

wtk_string_t* wtk_kgr_get_json_var(wtk_json_t *json,char *k,int k_len)
{
	wtk_json_item_t *ji;
	wtk_string_t *v=NULL;
	wtk_string_t *t;

	ji=wtk_json_item_get_path_item(json->main,k,k_len,NULL);
	if(!ji){goto end;}
	t=wtk_json_item_get_str_value2(ji);
	if(!t){goto end;}
	v=wtk_heap_dup_string(json->heap,t->data,t->len);
	wtk_string_delete(t);
end:
	return v;
}

#define wtk_kgr_process_nlg_s(r,s) wtk_kgr_process_nlg(r,s,strlen(s))

wtk_string_t wtk_kgr_process_nlg(wtk_kgr_t *r,char* nlg,int len)
{
	wtk_string_t rx;
	wtk_nlg2_gen_env_t env;

	env.ths=r;
	env.lua_gen=(wtk_nlg2_get_lua_gen_f)wtk_kgr_lua_gen;
	rx=wtk_nlg2_process(r->nlg,nlg,len,&env);
	return rx;
}

wtk_string_t wtk_kgr_process_nlg2(wtk_kgr_t *r,wtk_kg_item_t* item,wtk_string_t* nlg,wtk_string_t *p,wtk_string_t *v)
{
	wtk_string_t rx;
	wtk_nlg2_gen_env_t env;
	wtk_strbuf_t *buf;

	env.ths=r;
	env.lua_gen=NULL;
	env.lua_gen2=(wtk_nlg2_get_lua_gen2_f)wtk_kgr_feed_lua;
	buf=wtk_strbuf_new(256,1);
	wtk_strbuf_push(buf,nlg->data,nlg->len);
	wtk_strbuf_push_s(buf,"(p=\"");
	wtk_strbuf_push(buf,p->data,p->len);
	if(v)
	{
		wtk_strbuf_push_s(buf,"\",v=\"");
		wtk_strbuf_push(buf,v->data,v->len);
	}
	if(item->attr && item->attr->args)
	{
		wtk_strbuf_push_s(buf,"\",");
		wtk_strbuf_push(buf,item->attr->args->data,item->attr->args->len);
	}else
	{
		wtk_strbuf_push_s(buf,"\"");
	}
	wtk_strbuf_push_s(buf,")");
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	rx=wtk_nlg2_process(r->nlg,buf->data,buf->pos,&env);
	//wtk_debug("[%.*s]\n",rx.len,rx.data);
	wtk_strbuf_delete(buf);
	return rx;
}

wtk_string_t wtk_kgr_process_nlg3(wtk_kgr_t *r,wtk_kg_item_t* item,wtk_string_t* nlg,wtk_string_t *p,wtk_string_t *v,
		wtk_string_t *cv)
{
	wtk_string_t rx;
	wtk_nlg2_gen_env_t env;
	wtk_strbuf_t *buf;

	env.ths=r;
	env.lua_gen=NULL;
	env.lua_gen2=(wtk_nlg2_get_lua_gen2_f)wtk_kgr_feed_lua;
	buf=wtk_strbuf_new(256,1);
	wtk_strbuf_push(buf,nlg->data,nlg->len);
	wtk_strbuf_push_s(buf,"(p=\"");
	wtk_strbuf_push(buf,p->data,p->len);
	if(v)
	{
		wtk_strbuf_push_s(buf,"\",v=\"");
		wtk_strbuf_push(buf,v->data,v->len);
	}
	if(cv)
	{
		wtk_strbuf_push_s(buf,"\",cv=\"");
		wtk_strbuf_push(buf,cv->data,cv->len);
	}
	if(item->attr && item->attr->args)
	{
		wtk_strbuf_push_s(buf,"\",");
		wtk_strbuf_push(buf,item->attr->args->data,item->attr->args->len);
	}else
	{
		wtk_strbuf_push_s(buf,"\"");
	}
	wtk_strbuf_push_s(buf,")");
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	rx=wtk_nlg2_process(r->nlg,buf->data,buf->pos,&env);
	//wtk_debug("[%.*s]\n",rx.len,rx.data);
	wtk_strbuf_delete(buf);
	return rx;
}

wtk_kg_item_next_item_t* wtk_kgr_get_next_item(wtk_kgr_t *r,wtk_kg_item_t *item,wtk_json_t *json,wtk_string_t *p,wtk_kg_item_t *cur_item)
{
	wtk_larray_t *a;
	wtk_queue_node_t *qn;
	wtk_kg_item_next_item_t *ni,*xi=NULL;
	wtk_json_item_t *ji;
	int b;
	int i;
	wtk_kg_item_t *ki;

	if(!item->next){goto end;}
	a=wtk_larray_new(item->next->next_q.len,sizeof(void*));
	for(qn=item->next->next_q.pop;qn;qn=qn->next)
	{
		ni=data_offset2(qn,wtk_kg_item_next_item_t,q_n);
		if(!ni->is_item)
		{
			wtk_larray_push2(a,&(ni));
			continue;
		}
		ki=ni->v.item;
		if(!ki->attr || !ki->attr->ask)
		{
			continue;
		}
		//wtk_debug("[%.*s]\n",ki->name->len,ki->name->data);
		ji=wtk_json_item_get_path_item(json->main,ki->name->data,ki->name->len,NULL);
		if(ji && wtk_json_item_len(ji)==0)
		{
			ji=NULL;
		}
		//wtk_debug("ji=%p\n",ji);
		if(!ji)
		{
			if(ni->_if)
			{
				b=wtk_if_check(ni->_if,json,(wtk_if_get_var_f)wtk_kgr_get_json_var);
			}else
			{
				b=1;
			}
			if(b)
			{
				if(cur_item && ni->is_item &&  ni->v.item==cur_item)
				{
					wtk_larray_reset(a);
					wtk_larray_push2(a,&(ni));
					break;
				}else
				{
					wtk_larray_push2(a,&(ni));
				}
			}
		}
	}
	//wtk_debug("n=%d\n",a->nslot);
	if(a->nslot>0)
	{
		if(r->cfg->use_random)
		{
			i=wtk_random(0,a->nslot-1);
		}else
		{
			i=0;
		}
		xi=((wtk_kg_item_next_item_t**)(a->slot))[i];
	}
	wtk_larray_delete(a);
end:
	return xi;
}

wtk_kg_item_next_item_t* wtk_kgr_get_next(wtk_kgr_t *r,wtk_string_t *p,wtk_kg_item_t *item,wtk_kg_item_t *cur)
{
	wtk_kg_item_next_item_t *xi;
	wtk_jsonkv_t *kv=r->inst_kv;
	wtk_json_t *json;

	xi=NULL;
	if(!item->next){goto end;}
	//wtk_debug("next[%.*s]\n",p->len,p->data);
	json=wtk_jsonkv_get_json(kv,p->data,p->len);
	xi=wtk_kgr_get_next_item(r,item,json,p,cur);
end:
	wtk_jsonkv_reset(kv);
	return xi;
}

int wtk_kgr_check_next(wtk_kgr_t *r,wtk_kg_item_t *item,wtk_json_t *json,wtk_string_t *p,wtk_strbuf_t *output,wtk_kg_item_t *cur_item)
{
	int ret=0;
	wtk_kg_item_next_item_t *xi;
	wtk_string_t v;

	if(!item->next){goto end;}
	xi=wtk_kgr_get_next_item(r,item,json,p,cur_item);
	if(!xi){goto end;}
	if(xi->is_item)
	{
		r->next=xi->v.item;
		v=wtk_kgr_process_nlg2(r,xi->v.item,xi->v.item->attr->ask,p,NULL);
	}else
	{
		v=wtk_kgr_feed_lua(r,xi->v.lua);
	}
	if(v.len>0)
	{
		wtk_strbuf_push(output,v.data,v.len);
		ret=1;
	}
end:
	return ret;
}


int wtk_kgr_check_topic(wtk_kgr_t *r,wtk_kg_class_t *cls,wtk_json_t *json,wtk_string_t *p,wtk_strbuf_t *output,wtk_kg_item_t *cur_item)
{
	int ret;

	//wtk_debug("ask=%p talk=%p\n",cls->freeask,cls->freetalk);
	//wtk_json_item_print3(json->main);
	if(cls->freeask)
	{
		ret=wtk_kgr_check_next(r,cls->freeask,json,p,output,cur_item);
		if(ret==1){goto end;}
	}
	if(cls->freetalk)
	{
		ret=wtk_kgr_check_next(r,cls->freetalk,json,p,output,NULL);
		if(ret==1){goto end;}
	}
	ret=0;
end:
	return ret;
}

wtk_string_t* wtk_kg_item_get_answer_value(wtk_kgr_t *kg,wtk_kg_item_t *item,wtk_json_item_t *ji)
{
	int idx;

	if(ji->type==WTK_JSON_ARRAY)
	{
		if(item->use_last_best)
		{
			idx=ji->v.array->length-1;
		}else
		{
			if(kg->cfg->use_random)
			{
				idx=wtk_random(0,ji->v.array->length-1);
			}else
			{
				idx=0;
			}
		}
		ji=wtk_json_array_get(ji,idx);
		return wtk_kg_item_get_answer_value(kg,item,ji);
	}else
	{
		return wtk_json_item_get_str_value2(ji);
	}
}


int wtk_kgr_feed2(wtk_kgr_t *r,wtk_kg_item_t *item,wtk_act_t *act,wtk_strbuf_t *buf)
{
	wtk_string_t xp,txp;
	wtk_kg_t *kg=r->cfg->kg;
	wtk_string_t *v,*p=NULL;
	int ret=-1;
	wtk_jsonkv_t *kv=r->inst_kv;
	wtk_json_t *json=NULL;
	//int save=0;
	wtk_json_item_t *ji;
	int ask_you=0;
	wtk_string_t txx;

	r->dirty=0;
	v=wtk_act_get_str_value_s(act,"p");
	if(!v)
	{
		if(r->last_p->pos>0)
		{
			wtk_string_set(&(txp),r->last_p->data,r->last_p->pos);
			v=&(txp);
		}
	}
	if(!v)
	{
		wtk_debug("p not found\n");
		goto end;
	}
	p=v;
	//wtk_debug("[%.*s]\n",v->len,v->data);
	//加载inst
	json=wtk_jsonkv_get_json(kv,v->data,v->len);
	//wtk_json_item_print3(json->main);
	if(wtk_json_item_len(json->main)==0)
	{
		//如果inst第一次,更新默认值;
		wtk_kgr_update_inst(r,json,v);
		wtk_jsonkv_save_json(kv,p->data,p->len,json);
	}
	//wtk_json_item_print3(json->main);
	v=wtk_act_get_str_value_s(act,"act");
	if(!v)
	{
		//wtk_debug("act not found\n");
		v=wtk_act_get_str_value_s(act,"confirm");
		if(!v)
		{
			goto end;
		}
		v=(wtk_string_t*)1;
	}
	ji=wtk_json_item_get_path_item(json->main,item->name->data,item->name->len,NULL);
	if(ji && wtk_json_item_len(ji)==0)
	{
		ji=NULL;
	}
	//wtk_debug("ji=%p\n",ji);
	//wtk_json_item_print3(ji);
	if(v==(wtk_string_t*)1)
	{
		//wtk_debug("confrim[%.*s]=%p\n",item->name->len,item->name->data,item->attr->confirm);
		if(item->attr && item->attr->confirm)
		{
			xp=wtk_kgr_process_nlg2(r,item,item->attr->confirm,p,NULL);
			if(xp.len>0)
			{
				wtk_strbuf_push(buf,xp.data,xp.len);
			}
		}else
		{
			//wtk_debug("confrim is nil\n");
			goto end;
		}
	}else if(wtk_string_cmp_s(v,"ask")==0)
	{
		if(ji)
		{
			v=wtk_kg_item_get_answer_value(r,item,ji);
			//wtk_debug("v=%p/%p\n",item->attr->answer,v);
			if(item->attr->answer)
			{
				xp=wtk_kgr_process_nlg2(r,item,item->attr->answer,p,v);
				if(xp.len>0)
				{
					wtk_strbuf_push(buf,xp.data,xp.len);
				}
			}else
			{
				wtk_debug("answer is nil\n");
			}
			wtk_string_delete(v);
		}else if(item->attr->ask2)
		{
			xp=wtk_kgr_process_nlg2(r,item,item->attr->ask2,p,NULL);
			if(xp.len>0)
			{
				r->next=item;
				wtk_strbuf_push(buf,xp.data,xp.len);
				goto end;
			}
		}else if(item->attr->ask)
		{
			xp=wtk_kgr_process_nlg_s(r,"unkown()");
			if(xp.len>0)
			{
				wtk_strbuf_push(buf,xp.data,xp.len);
			}
			xp=wtk_kgr_process_nlg2(r,item,item->attr->ask,p,NULL);
			if(xp.len>0)
			{
				r->next=item;
				wtk_strbuf_push(buf,xp.data,xp.len);
				goto end;
			}
		}
	}else if(wtk_string_cmp_s(v,"check")==0)
	{
		v=wtk_act_get_str_value_s(act,"cv");
		if(item->attr && item->attr->post_v)
		{
			txx=wtk_kgr_feed_lua(r,item->attr->post_v);
			if(txx.len>0)
			{
				v=&(txx);
			}
		}
		if(v && item->attr->check)
		{
			wtk_string_t *rv;

			if(ji)
			{
				rv=wtk_kg_item_get_answer_value(r,item,ji);
			}else
			{
				rv=NULL;
			}
			xp=wtk_kgr_process_nlg3(r,item,item->attr->check,p,rv,v);
			if(xp.len>0)
			{
				wtk_strbuf_push(buf,xp.data,xp.len);
			}
			if(rv)
			{
				wtk_string_delete(rv);
			}
		}
	}else if(wtk_string_cmp_s(v,"set")==0)
	{
		v=wtk_act_get_str_value_s(act,"v");
		if(item->attr && item->attr->post_v)
		{
			txx=wtk_kgr_feed_lua(r,item->attr->post_v);
			if(txx.len>0)
			{
				v=wtk_heap_dup_string(json->heap,txx.data,txx.len);
			}
			//wtk_debug("[%.*s]\n",txx.len,txx.data);
		}
		//save=1;
		switch(item->vtype)
		{
		case WTK_KG_ITEM_INT:
			if(!ji)
			{
				ji=wtk_json_item_add_path_item(json,json->main,item->name->data,item->name->len,WTK_JSON_NUMBER);
			}
			ji->v.number=wtk_str_atoi(v->data,v->len);
			break;
		case WTK_KG_ITEM_STR:
			if(!ji)
			{
				ji=wtk_json_item_add_path_item(json,json->main,item->name->data,item->name->len,WTK_JSON_STRING);
			}
			ji->v.str=v;
			break;
		case WTK_KG_ITEM_ARRAY:
			if(!ji)
			{
				ji=wtk_json_item_add_path_item(json,json->main,item->name->data,item->name->len,WTK_JSON_ARRAY);
			}
			if(wtk_json_array_has_string_value(ji,v->data,v->len)==0)
			{
				wtk_json_array_add_ref_str(json,ji,v);
			}
			break;
		}
		//wtk_debug("[%.*s]=[%.*s]\n",p->len,p->data,v->len,v->data);
		wtk_jsonkv_save_json(kv,p->data,p->len,json);
		if(item->attr->set && v)
		{
			//wtk_debug("[%.*s]=[%.*s]\n",p->len,p->data,v->len,v->data);
			xp=wtk_kgr_process_nlg2(r,item,item->attr->set,p,v);
			if(xp.len>0)
			{
				wtk_strbuf_push(buf,xp.data,xp.len);
			}
		}
		if(r->dirty)
		{
			json=wtk_jsonkv_get_json(kv,p->data,p->len);
		}
		ret=wtk_kgr_check_next(r,item,json,p,buf,NULL);//wtk_kgr_t *r,wtk_kg_item_t *item,wtk_json_t *json);
		if(ret==1)
		{
			ret=0;
			goto end;
		}
	}else
	{
		wtk_debug("unk act [%.*s]\n",v->len,v->data);
		goto end;
	}
	//wtk_debug("[%.*s]\n",p->len,p->data);
	if(wtk_string_cmp_s(p,"我")!=0)
	{
		if(r->dirty)
		{
			json=wtk_jsonkv_get_json(kv,p->data,p->len);
		}
		ret=wtk_kgr_check_topic(r,kg->_class,json,p,buf,NULL);
		if(ret==1)
		{
			ret=0;
			goto end;
		}
	}
	if(ask_you==0)
	{
		wtk_string_t me=wtk_string("你");

		p=&me;
		wtk_jsonkv_reset(kv);
		json=wtk_jsonkv_get_json(kv,p->data,p->len);

		ret=wtk_kgr_check_topic(r,kg->_class,json,p,buf,item);
		if(ret==1)
		{
			ret=0;
			goto end;
		}
	}
	ret=0;
end:
	wtk_jsonkv_reset(kv);
	//exit(0);
	return ret;
}

wtk_string_t wtk_kgr_feed_lua(wtk_kgr_t *kgr,char *func)
{
	wtk_string_t v;
	wtk_lua2_arg_t arg[4];
	wtk_strbuf_t *buf=kgr->tmp;

	wtk_string_set(&(v),0,0);
	//wtk_debug("lua=[%s]\n",func);
	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=kgr->fld->dlg;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=kgr->fld;
	arg[2].type=WTK_LUA2_THS;
	arg[2].v.ths=kgr->act;
	arg[3].type=WTK_LUA2_THS;
	arg[3].v.ths=kgr;
	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(kgr->fld->dlg->lua,func,buf,arg,arg+1,arg+2,arg+3,NULL);
	if(buf->pos>0)
	{
		wtk_string_set(&(v),buf->data,buf->pos);
	}
	return v;
}

int wtk_kgr_has_key(wtk_kgr_t *kgr,char *k,int k_bytes)
{
	wtk_string_t *v;

	v=wtk_act_get_str_value(kgr->act,k,k_bytes);
	return v?1:0;
}

wtk_string_t* wtk_kgr_get_value(wtk_kgr_t *kg,char *k,int k_bytes)
{
	wtk_string_t *v;

	//wtk_act_print(kg->act);
	v=wtk_act_get_str_value(kg->act,k,k_bytes);
	//wtk_debug("[%.*s]=%p\n",k_bytes,k,v);
	//wtk_act_print(kg->act);
	//exit(0);
	return v;
}

int wtk_kgr_nvalue(wtk_kgr_t *kg)
{
	if(kg->act_nv==-1)
	{
		kg->act_nv=wtk_act_nslot(kg->act);
	}
	//wtk_debug("nv=%d\n",kg->act_nv);
	return kg->act_nv;
}

int wtk_kgr_feed(wtk_kgr_t *r,wtk_act_t *act)
{
	wtk_kg_t *kg=r->cfg->kg;
	wtk_string_t *v=NULL,t;
	wtk_kg_item_t* item;
	wtk_nlgfst2_t *fst=r->fst;
	int ret=-1;
	wtk_nlgfst_act_t fst_act;
	wtk_strbuf_t *buf;

	//wtk_semslot_print(r->fld->slot);
	r->act_nv=-1;
	v=wtk_act_get_str_value_s(act,"item");

	if(v)
	{
		item=wtk_kg_class_get_item(kg,kg->_class,v->data,v->len,0);
	}else
	{
		item=NULL;
	}
//	if(item)
//	{
//		wtk_debug("item=%p next=%p [%.*s]\n",item,r->next,item->name->len,item->name->data);
//	}
	if(!item)
	{
		if(r->next)
		{
			item=r->next;
		}else
		{
			item=r->last;
		}
	}
	//wtk_debug("[%.*s]=%p/%p\n",v->len,v->data,item,item->attr);
	r->next=NULL;
	if(!item)
	{
		if(v)
		{
			wtk_debug("item[%.*s] not found\n",v->len,v->data);
		}
		goto end;
	}
	r->cur=item;
	if(item->nlg_net!=fst->net)
	{
		wtk_nlgfst2_set_net(fst,item->nlg_net);
	}
	r->act=act;
	if(r->cfg->lua_init)
	{
		t=wtk_kgr_feed_lua(r,r->cfg->lua_init);
		if(t.len>0)
		{
			wtk_semfld_set_output(r->fld,t.data,t.len);
			goto end;
		}
	}
	v=wtk_act_get_str_value_s(act,"p");
	if(v)
	{
		wtk_kgr_touch_inst(r,v->data,v->len);
	}
	buf=wtk_strbuf_new(256,1);
	//wtk_debug("item=%p net=%p\n",item,item->nlg_net);
	//wtk_act_print(act);
	if(item->nlg_net)
	{
		fst_act.ths=r;
		fst_act.buf=buf;
		fst_act.feed_lua=(wtk_nlg_act_get_lua_f)wtk_kgr_feed_lua;
		fst_act.has_key=(wtk_nlgfst_act_has_key_f)wtk_kgr_has_key;
		fst_act.get_value=(wtk_nlg_act_get_value_f)wtk_kgr_get_value;
		fst_act.nvalue=(wtk_nlgfst_act_nvalue_f)wtk_kgr_nvalue;
		ret=wtk_nlgfst2_feed(r->fst,&(fst_act));
	}else if(item->attr)
	{
		ret=wtk_kgr_feed2(r,item,act,buf);
	}
	v=wtk_act_get_str_value_s(act,"p");
	if(v)
	{
		wtk_strbuf_reset(r->last_p);
		wtk_strbuf_push(r->last_p,v->data,v->len);
	}
	//wtk_debug("[%.*s]\n",fst_act.buf->pos,fst_act.buf->data);
	//exit(0);
	if(buf->pos>0)
	{
		wtk_semfld_set_output(r->fld,buf->data,buf->pos);
		ret=0;
	}else
	{
		ret=1;
	}
	wtk_strbuf_delete(buf);
	r->last=item;
end:

	return ret;
}
