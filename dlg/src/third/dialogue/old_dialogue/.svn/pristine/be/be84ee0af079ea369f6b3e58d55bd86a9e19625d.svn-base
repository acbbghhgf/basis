#include "wtk_semfstr.h" 
wtk_string_t wtk_semfst_script_get_output_string(wtk_semfstr_t *r,wtk_semfst_script_t *script,wtk_act_t *act,char *name,int name_len);

int wtk_lua_semfstr_kv_get(lua_State *l)
{
	wtk_semfstr_t *sem;
	wtk_string_t k;
	wtk_string_t a;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfstr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	++i;
	if(!lua_isstring(l,i)){goto end;}
	a.data=(char*)lua_tolstring(l,i,&len);
	a.len=len;
	//wtk_debug("[%.*s]=[%.*s]\n",k.len,k.data,a.len,a.data);
	v=wtk_jsonkv_get(sem->jsonkv,k.data,k.len,a.data,a.len);
	lua_pushlstring(l,v.data,v.len);
	cnt=1;
end:
	return cnt;
}

int wtk_lua_semfstr_kv_set(lua_State *l)
{
	wtk_semfstr_t *sem;
	wtk_string_t k;
	wtk_string_t a;
	wtk_string_t v;
	size_t len;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semfstr_t*)lua_touserdata(l,i);
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
	wtk_jsonkv_set(sem->jsonkv,k.data,k.len,a.data,a.len,v.data,v.len);
end:
	return 0;
}

int wtk_lua_semfstr_script_get_output(lua_State *l)
{
	wtk_semfstr_t *r;
	wtk_semfst_script_t *s;
	wtk_act_t *act;
	wtk_string_t k;
	wtk_string_t v;
	size_t len;
	int i;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	r=(wtk_semfstr_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	s=(wtk_semfst_script_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isuserdata(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	act=(wtk_act_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		//wtk_debug("get =================================\n");
		goto end;
	}
	k.data=(char*)lua_tolstring(l,i,&len);
	k.len=len;
	//wtk_debug("get =================================\n");
	v=wtk_semfst_script_get_output_string(r,s,act,k.data,k.len);
	if(v.len>0)
	{
		lua_pushlstring(l,v.data,v.len);
		cnt=1;
	}
end:
	//wtk_debug("get =================================\n");
	return cnt;
}

void wtk_semfstr_link_lua(wtk_lua2_t *lua2)
{
	wtk_lua2_link_function(lua2,wtk_lua_semfstr_kv_get,"wtk_semfstr_kv_get");
	wtk_lua2_link_function(lua2,wtk_lua_semfstr_kv_set,"wtk_semfstr_kv_set");
	wtk_lua2_link_function(lua2,wtk_lua_semfstr_script_get_output,"wtk_semfstr_script_get_output");
}

void wtk_semfstr_load_lua(wtk_semfstr_t *r)
{
	wtk_string_t **strs;
	int i;

	if(!r->cfg->lua){return;}
	strs=(wtk_string_t**)(r->cfg->lua->slot);
	for(i=0;i<r->cfg->lua->nslot;++i)
	{
		wtk_lua2_load_file2(r->lua,strs[i]->data);
	}
}

wtk_semfstr_t* wtk_semfstr_new(wtk_semfstr_cfg_t *cfg,wtk_semfst_net_t *net,wtk_lexr_t *lexr,wtk_lua2_t *lua)
{
	wtk_semfstr_t *r;

	r=(wtk_semfstr_t*)wtk_malloc(sizeof(wtk_semfstr_t));
	r->cfg=cfg;
	r->heap=wtk_heap_new(4096);
	r->buf=wtk_strbuf_new(256,1);
	r->nxt_script=NULL;
	r->jsonkv=wtk_jsonkv_new(cfg->dn);
	r->ans=wtk_strbuf_new(256,1);
	r->result=wtk_strbuf_new(256,1);
	r->net=net;
	r->lexr=lexr;
	r->lua=lua;
	r->act_req=NULL;
	wtk_semfstr_load_lua(r);
	wtk_semfstr_link_lua(lua);
	return r;
}

void wtk_semfstr_delete(wtk_semfstr_t *r)
{
	wtk_strbuf_delete(r->result);
	wtk_strbuf_delete(r->ans);
	wtk_jsonkv_delete(r->jsonkv);
	wtk_strbuf_delete(r->buf);
	wtk_heap_delete(r->heap);
	wtk_free(r);
}

void wtk_semfstr_reset(wtk_semfstr_t *r)
{
	r->act_req=NULL;
	wtk_heap_reset(r->heap);
}

int wtk_semfstr_process_lua(wtk_semfstr_t *r,wtk_semfst_script_t *script,wtk_act_t *act,char *func,wtk_strbuf_t *buf)
{
	wtk_lua2_arg_t arg[4];
	//wtk_strbuf_t *buf=r->ans;

	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=r->lua_hook;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=r;
	arg[2].type=WTK_LUA2_THS;
	arg[2].v.ths=script;
	arg[3].type=WTK_LUA2_THS;
	arg[3].v.ths=act;
	wtk_strbuf_reset(buf);
	return wtk_lua2_process_arg(r->lua,func,buf,arg,arg+1,arg+2,arg+3,NULL);
}

int wtk_semfstr_output_match(wtk_semfstr_t *r,wtk_semfst_output_t *output,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_semfst_slot_t *slot;
	wtk_string_t *v;

	//wtk_debug("match=====================>\n");
	if(output->get_answer && r->ans->pos<=0)
	{
		//wtk_debug("match=====================>\n");
		return 0;
	}
	for(qn=output->slot_q.pop;qn;qn=qn->next)
	{
		slot=data_offset2(qn,wtk_semfst_slot_t,q_n);
		v=wtk_act_get_str_value(act,slot->k->data,slot->k->len);
		if(slot->v)
		{
			if(!v || wtk_string_cmp(v,slot->v->data,slot->v->len)!=0)
			{
				//wtk_debug("match=====================>\n");
				return 0;
			}
		}else
		{
			if(!v)
			{
				//wtk_debug("match=====================>\n");
				return 0;
			}
		}
	}
	return 1;
}

wtk_semfst_output_t* wtk_semfstr_get_output(wtk_semfstr_t *r,wtk_semfst_script_t* script,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_semfst_output_t *output;

	for(qn=script->output_q.pop;qn;qn=qn->next)
	{
		output=data_offset2(qn,wtk_semfst_output_t,q_n);
		if(wtk_semfstr_output_match(r,output,act))
		{
			//wtk_debug("output=%p\n",output);
			goto end;
		}
	}
	if(script->other)
	{
		output=script->other;
	}else
	{
		output=NULL;
	}
end:
	return output;
}


wtk_semfst_output_t* wtk_semfst_script_get_output(wtk_semfst_script_t *script,char *name,int name_len)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_semfst_output_t *output;
	wtk_semfst_slot_t *slot;

	for(qn=script->output_q.pop;qn;qn=qn->next)
	{
		output=data_offset2(qn,wtk_semfst_output_t,q_n);
		for(qn2=output->slot_q.pop;qn2;qn2=qn2->next)
		{
			slot=data_offset2(qn2,wtk_semfst_slot_t,q_n);
			if(wtk_string_cmp(slot->k,name,name_len)==0)
			{
				return output;
			}
		}
	}
	return NULL;
}

wtk_string_t wtk_semfst_output_get_output(wtk_semfstr_t *r,wtk_semfst_script_t *script,wtk_semfst_output_t *output,wtk_act_t *act)
{
	wtk_semfst_output_item_t* item;
	wtk_strbuf_t *buf=r->result;
	wtk_string_t *t;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	wtk_strbuf_reset(buf);
	item=wtk_semfst_output_get(output);
	switch(item->type)
	{
	case WTK_SEMFSTSTR_OUTPUT_STR:
		{
			wtk_queue_node_t *qn;
			wtk_semfst_output_str_item_t *si;

			//wtk_debug("len=%d\n",item->v.strq->len);
			for(qn=item->v.strq->pop;qn;qn=qn->next)
			{
				si=data_offset2(qn,wtk_semfst_output_str_item_t,q_n);
				//wtk_debug("si=%p [%.*s]\n",si,buf->pos,buf->data);
				//wtk_semfstr_output_str_item_print(si);
				if(si->is_var)
				{
					if(wtk_string_cmp_s(si->v.var,"answer")==0)
					{
						wtk_strbuf_push(buf,r->ans->data,r->ans->pos);
					}else
					{
						t=wtk_act_get_str_value(act,si->v.var->data,si->v.var->len);
						if(!t)
						{
							//wtk_act_print(&(act));
							wtk_debug("[%.*s] not found\n",si->v.var->len,si->v.var->data);
							return v;
						}
						wtk_strbuf_push(buf,t->data,t->len);
						//wtk_debug("[%.*s]=[%.*s]\n",t->len,t->data,buf->pos,buf->data);
					}
				}else
				{
					wtk_strbuf_push(buf,si->v.str->data,si->v.str->len);
				}
			}
		}
		break;
	case WTK_SEMFSTSTR_OUTPUT_LUA:
		wtk_semfstr_process_lua(r,script,act,item->v.lua,buf);
		break;
	}
	wtk_string_set(&(v),buf->data,buf->pos);
	return v;
}

wtk_string_t wtk_semfst_script_get_output_string(wtk_semfstr_t *r,wtk_semfst_script_t *script,wtk_act_t *act,char *name,int name_len)
{
	wtk_semfst_output_t *output;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	output=wtk_semfst_script_get_output(script,name,name_len);
	//wtk_debug("name=%p\n",output);
	if(!output){goto end;}
	v=wtk_semfst_output_get_output(r,script,output,act);
	if(v.len>0)
	{
		if(output->next)
		{
			r->nxt_script=output->next;
		}
	}
end:
	return v;
}


wtk_string_t wtk_semfstr_process_script(wtk_semfstr_t *r,wtk_semfst_script_t *script,char *data,int bytes)
{
	wtk_heap_t *heap=r->heap;
	wtk_lexr_t *lexr=r->lexr;
	wtk_string_t v;
	wtk_act_t act;
	int ret;
	wtk_semfst_output_t *output=NULL;

	//wtk_debug("===================>\n",script->);
	//wtk_debug("[%.*s] [%.*s]\n",bytes,data,script->name->len,script->name->data);
	wtk_string_set(&(v),0,0);
	ret=wtk_lexr_process(lexr,script->lexnet,data,bytes);
	if(ret!=0)
	{
		wtk_lexr_reset(lexr);
		goto end;
	}
	//wtk_lexr_print(lexr);
	if(!lexr->action_list || lexr->action_list->v.array->length==0)
	{
		output=script->other;
		goto end;
	}
	wtk_act_init_json(&(act),heap);
	act.v.json.json=wtk_json_item_dup(lexr->action,heap);
	wtk_lexr_reset(lexr);
	wtk_act_update(&(act));
	//wtk_act_print(&(act));
	ret=wtk_semfstr_process_lua(r,script,&act,r->cfg->lua_main,r->ans);
	if(ret!=0)
	{
		output=NULL;
		goto end;
	}
	output=wtk_semfstr_get_output(r,script,&act);
end:
	//wtk_debug("output=%p [%.*s]\n",output,r->ans->pos,r->ans->data);
	if(output)
	{
		v=wtk_semfst_output_get_output(r,script,output,&act);
		if(v.len==0)
		{
			return v;
		}
		//wtk_debug("[%.*s]\n",script->name->len,script->name->data);
		r->act_req=act.v.json.json;
		if(output->next)
		{
			r->nxt_script=output->next;
		}
	}
	//wtk_debug("v[%.*s]\n",v.len,v.data);
	//exit(0);
	return v;
}


wtk_string_t wtk_semfstr_process(wtk_semfstr_t *r,char *data,int bytes)
{
	wtk_queue_node_t *qn;
	wtk_semfst_sceen_t *s;
	wtk_string_t v;

	//wtk_debug("[%.*s]\n",bytes,data);
	r->act_req=NULL;
	wtk_strbuf_reset(r->result);
	wtk_string_set(&(v),0,0);
	//wtk_debug("next=%p\n",r->nxt_script);
	if(r->nxt_script)
	{
		v=wtk_semfstr_process_script(r,r->nxt_script,data,bytes);
		if(v.len>0){goto end;}
	}
	for(qn=r->net->sceen_q.pop;qn;qn=qn->next)
	{
		s=data_offset2(qn,wtk_semfst_sceen_t,q_n);
		//wtk_debug("[%.*s]\n",s->name->len,s->name->data);
		v=wtk_semfstr_process_script(r,s->init,data,bytes);
		if(v.len>0){goto end;}
	}
end:
	//wtk_debug("next=%p\n",r->nxt_script);
	//wtk_heap_reset(r->heap);
	//wtk_debug("[%.*s]\n",v.len,v.data);
	return v;
}

