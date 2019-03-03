#include <ctype.h>
#include "wtk_owlkg.h" 

void wtk_owlkg_bind_item_nlg(wtk_owlkg_t *kg,wtk_owl_item_t *item)
{
	wtk_nlg_root_t *root;
	int i;

	if(item->nv>0 && !item->use_k_inst && wtk_string_cmp_withstart_s(item->k.str,"nlg.")==0)
	{
		//wtk_debug("[%.*s] not found\n",item->v[0]->len,item->v[0]->data);
		//item->hook=wtk_nlg_get3(kg->nlg,item->v[0]->data,item->v[0]->len);
		root=wtk_nlg_get_root(kg->nlg,item->v[0]->data,item->v[0]->len,0);
		if(root && root->item)
		{
			item->hook=root->item;
		}else if(root && root->attr_q.length>0)
		{
			item->hook=wtk_nlg_root_get_default_item(root);
		}
		//wtk_debug("[%.*s]=%p\n",item->v[0]->len,item->v[0]->data,item->hook);
		if(!item->hook)
		{
			wtk_nlg_print(kg->nlg);
			wtk_debug("[%.*s] not found\n",item->v[0]->len,item->v[0]->data);
			exit(0);
		}
	}
	for(i=0;i<item->nitem;++i)
	{
		wtk_owlkg_bind_item_nlg(kg,item->items[i]);
	}
}

void wtk_owlkg_bind_prop_nlg(wtk_owlkg_t *kg,wtk_owl_item_t *prop)
{
	int i;

	for(i=0;i<prop->nitem;++i)
	{
		wtk_owlkg_bind_item_nlg(kg,prop->items[i]);
	}
}

void wtk_owlkg_bind_cls_nlg(wtk_owlkg_t *kg,wtk_owl_class_t *cls)
{
	//wtk_owlkg_bind_item_nlg(kg,cls->item);
	wtk_owlkg_bind_prop_nlg(kg,cls->prop);
}

void wtk_owlkg_bind_nlg(wtk_owlkg_t *kg)
{
	wtk_str_hash_it_t it;
	hash_str_node_t *node;
	wtk_owl_class_t *cls;

	it=wtk_str_hash_iterator(kg->owl->cls_map);
	while(1)
	{
		node=wtk_str_hash_it_next(&(it));
		if(!node){break;}
		cls=(wtk_owl_class_t*)node->value;
		wtk_owlkg_bind_cls_nlg(kg,cls);
	}
}

wtk_owlkg_t* wtk_owlkg_new(wtk_owlkg_cfg_t *cfg,wtk_lua2_t *lua)
{
	wtk_owlkg_t *kg;

	kg=(wtk_owlkg_t*)wtk_malloc(sizeof(wtk_owlkg_t));
	kg->cfg=cfg;
	kg->lua=lua;
	kg->kv=wtk_jsonkv_new(cfg->brain_dn);
	kg->owl=wtk_owl_tree_new_fn(cfg->owl);
	//wtk_owl_tree_print(kg->owl);
	//exit(0);
	kg->nlg=wtk_nlg_new(cfg->nlg);
	kg->buf=wtk_strbuf_new(256,1);
	kg->heap=wtk_heap_new(4096);
	wtk_owlkg_bind_nlg(kg);
	return kg;
}

void wtk_owlkg_delete(wtk_owlkg_t *kg)
{
	wtk_heap_delete(kg->heap);
	wtk_jsonkv_delete(kg->kv);
	wtk_strbuf_delete(kg->buf);
	wtk_owl_tree_delete(kg->owl);
	wtk_nlg_delete(kg->nlg);
	wtk_free(kg);
}

void wtk_owlkg_act_init(wtk_owlkg_act_t *act)
{
	act->type=WTK_OWLKG_ACT_ASK;
	wtk_queue_init(&(act->item_q));
}

void wtk_owlkg_act_print(wtk_owlkg_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_owlkg_act_item_t *item;

	switch(act->type)
	{
	case WTK_OWLKG_ACT_ASK:
		printf("owl.ask(");
		break;
	case WTK_OWLKG_ACT_GET:
		printf("owl.get(");
		break;
	case WTK_OWLKG_ACT_ADD:
		printf("owl.add(");
		break;
	case WTK_OWLKG_ACT_DEL:
		printf("owl.del(");
		break;
	case WTK_OWLKG_ACT_CHECK:
		printf("owl.check(");
		break;
	}
	for(qn=act->item_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_owlkg_act_item_t,q_n);
		if(qn!=act->item_q.pop)
		{
			printf(",");
		}
		//wtk_debug("item=%p k=%p v=%p\n",item,item->k,item->v);
		printf("%.*s",item->k->len,item->k->data);
		if(item->v)
		{
			printf(":%.*s",item->v->len,item->v->data);
		}
	}
	printf(")\n");
}

wtk_string_t wtk_owlkg_process_lua(wtk_owlkg_t *kg,char *func)
{
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	return v;
}

typedef struct
{
	wtk_owlkg_t *kg;
	wtk_string_t *v;
	wtk_owlkg_act_t *act;
}wtk_owlkg_act_info_t;

wtk_string_t* wtk_owlkg_act_info_get_nlg_value(wtk_owlkg_act_info_t *kg,char *k,int k_len)
{
	int idx;
	wtk_queue_node_t *qn;
	wtk_owlkg_act_item_t *item;
	int i=0;

	//wtk_debug("[%.*s]\n",k_len,k);
	if(wtk_str_equal_s(k,k_len,"_v"))
	{
		return kg->v;
	}else
	{
		if(k_len>2 && k[k_len-2]=='.' && isdigit(k[k_len-1]))
		{
			idx=k[k_len-1]-'0';
			k_len-=2;
		}else
		{
			idx=-1;
		}
		i=0;
		for(qn=kg->act->item_q.pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_owlkg_act_item_t,q_n);
			if(wtk_string_cmp(item->owl_item->k.str,k,k_len)==0)
			//wtk_owl_item_print(item->owl_item);
			//if(wtk_owl_item_match_value(item->owl_item,k,k_len))
			{
				if(idx==-1)
				{
					return item->v;
				}
				if(idx==i)
				{
					return item->v;
				}
				++i;
			}
			//exit(0);
		}
	}
	return NULL;
}

wtk_string_t wtk_owlkg_act_info_get_lua(wtk_owlkg_act_info_t *kg,char *func)
{
	return wtk_owlkg_process_lua(kg->kg,func);
}


int wtk_owlkg_process_nlg(wtk_owlkg_t *kg,wtk_owl_item_t *oi,wtk_owlkg_act_t *act,wtk_string_t *v,wtk_strbuf_t *buf)
{
	wtk_nlg_item_t *item;
	wtk_owlkg_act_info_t info;

	item=(wtk_nlg_item_t*)oi->hook;
	wtk_strbuf_reset(buf);
	if(!item)
	{
		wtk_debug("[%.*s] not found\n",oi->v[0]->len,oi->v[0]->data);
		return -1;
	}
	info.v=v;
	info.kg=kg;
	info.act=act;
	return wtk_nlg_process(kg->nlg,item,buf,&(info),(wtk_nlg_act_get_value_f)wtk_owlkg_act_info_get_nlg_value,(wtk_nlg_act_get_lua_f)wtk_owlkg_act_info_get_lua);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
}

wtk_json_obj_item_t* wtk_json_item_obj_get_random(wtk_json_item_t *ji)
{
	wtk_queue_node_t *qn;
	int v;
	wtk_json_obj_item_t *item;
	int i;

	if(ji->type!=WTK_JSON_OBJECT){return NULL;}
	v=wtk_random(0,ji->v.object->length-1);
	//wtk_debug("v=%d\n",v);
	for(i=0,qn=ji->v.object->pop;qn;qn=qn->next,++i)
	{
		if(i==v)
		{
			item=data_offset2(qn,wtk_json_obj_item_t,q_n);
			return item;
		}
	}
	return NULL;
}

wtk_string_t wtk_owlkg_process_act(wtk_owlkg_t *kg,wtk_owlkg_act_t *act)
{
	wtk_string_t v;
	wtk_strbuf_t *buf=kg->buf;
	wtk_owl_class_t *cls;
	wtk_owlkg_act_item_t *item=NULL,*inst_item;
	wtk_json_t *json=NULL;
	wtk_queue_node_t *qn;
	wtk_owl_item_t *sub_prop,*oi;
	wtk_json_item_t *json_item,*ji;
	int ret;

	wtk_string_set(&(v),0,0);
	if(act->item_q.length<=0)
	{
		wtk_debug("query item length is null.\n");
		goto end;
	}
	item=data_offset2(act->item_q.pop,wtk_owlkg_act_item_t,q_n);
	if(!item->v)
	{
		wtk_debug("query item inst is null.\n");
		goto end;
	}
	cls=wtk_owl_tree_find_class(kg->owl,item->k->data,item->k->len,0);
	if(!cls)
	{
		wtk_debug("query [%.*s] class not found.\n",item->k->len,item->k->data);
		goto end;
	}
	item->owl_item=cls->item;
	//wtk_owl_class_print(cls);
	json=wtk_jsonkv_get_json(kg->kv,item->v->data,item->v->len);
	if(!json)
	{
		wtk_debug("query [%.*s] inst not found.\n",item->v->len,item->v->data);
		goto end;
	}
	inst_item=item;
	sub_prop=NULL;
	json_item=json->main;
	for(qn=act->item_q.pop->next;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_owlkg_act_item_t,q_n);
		if(sub_prop)
		{
			sub_prop=wtk_owl_item_find_item(sub_prop,item->k);
		}else
		{
			sub_prop=wtk_owl_class_find_prop(cls,item->k);
		}
		if(!sub_prop)
		{
			wtk_debug("[%.*s] prop not found\n",item->k->len,item->k->data);
			goto end;
		}
		item->owl_item=sub_prop;
		//wtk_debug("[%.*s]\n",item->k->len,item->k->data);
		ji=wtk_json_obj_get(json_item,item->k->data,item->k->len);
		if(!ji)
		{
			ji=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,json_item,item->k->data,item->k->len,ji);
		}
		json_item=ji;
	}
	if(!sub_prop){goto end;}
	//wtk_owl_item_print(sub_prop);
	//wtk_debug("json=%p\n",json);
	switch(act->type)
	{
	case WTK_OWLKG_ACT_ASK:
		//wtk_json_item_print3(json_item);
		{
			wtk_json_obj_item_t *joi;

			joi=wtk_json_item_obj_get_random(json_item);
			if(joi)
			{
				oi=wtk_owl_item_find_item2_s(sub_prop,"nlg.ask_found");
				if(oi && oi->nv>0)
				{
					//wtk_debug("[%.*s]\n",oi->v[0]->len,oi->v[0]->data);
					ret=wtk_owlkg_process_nlg(kg,oi,act,&(joi->k),buf);
					if(ret==0 && buf->pos>0)
					{
						wtk_string_set(&(v),buf->data,buf->pos);
					}
				}
			}else
			{
				oi=wtk_owl_item_find_item2_s(sub_prop,"nlg.ask_not_found");
				if(oi && oi->nv>0)
				{
					//wtk_debug("[%.*s]\n",oi->v[0]->len,oi->v[0]->data);
					ret=wtk_owlkg_process_nlg(kg,oi,act,NULL,buf);
					if(ret==0 && buf->pos>0)
					{
						wtk_string_set(&(v),buf->data,buf->pos);
					}
				}
			}
		}

		break;
	case WTK_OWLKG_ACT_ADD:
		{
			//int b;

			//wtk_debug("set %p %p:%p\n",item,item->k,item->v);
			//wtk_json_item_print3(json_item);
			//wtk_json_item_print3(json->main);
			if(item->v)
			{
				ji=wtk_json_obj_get(json_item,item->v->data,item->v->len);
				if(!ji)
				{
					ji=wtk_json_new_object(json);
					wtk_json_obj_add_item2(json,json_item,item->v->data,item->v->len,ji);
					wtk_jsonkv_save_json(kg->kv,inst_item->v->data,inst_item->v->len,json);
				}
			}
			//if(b)
			{
				oi=wtk_owl_item_find_item2_s(sub_prop,"nlg.add");
				if(oi && oi->nv>0)
				{
					//wtk_debug("[%.*s]\n",oi->v[0]->len,oi->v[0]->data);
					ret=wtk_owlkg_process_nlg(kg,oi,act,item->v,buf);
					if(ret==0 && buf->pos>0)
					{
						wtk_string_set(&(v),buf->data,buf->pos);
					}
				}
			}
		}
		break;
	case WTK_OWLKG_ACT_GET:
		{
			wtk_json_obj_item_t *joi;

			joi=wtk_json_item_obj_get_random(json_item);
			if(joi)
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,joi->k.data,joi->k.len);
				wtk_string_set(&(v),buf->data,buf->pos);
			}
		}
		break;
	case WTK_OWLKG_ACT_DEL:
		{
			//int b;

			//wtk_debug("set %p %p:%p\n",item,item->k,item->v);
			//wtk_json_item_print3(json_item);
			//wtk_json_item_print3(json->main);
			if(item->v)
			{
				ji=wtk_json_obj_remove(json_item,item->v->data,item->v->len);
				if(ji)
				{
					wtk_jsonkv_save_json(kg->kv,inst_item->v->data,inst_item->v->len,json);
				}
			}
		}
		break;
	case WTK_OWLKG_ACT_CHECK:
		{
			//int b;

			//wtk_debug("set %p %p:%p\n",item,item->k,item->v);
			//wtk_json_item_print3(json_item);
			//wtk_json_item_print3(json->main);
			if(item->v)
			{
				ji=wtk_json_obj_get(json_item,item->v->data,item->v->len);
				if(ji)
				{
					oi=wtk_owl_item_find_item2_s(sub_prop,"nlg.check_yes");
					if(oi && oi->nv>0)
					{
						//wtk_debug("[%.*s]\n",oi->v[0]->len,oi->v[0]->data);
						ret=wtk_owlkg_process_nlg(kg,oi,act,item->v,buf);
						if(ret==0 && buf->pos>0)
						{
							wtk_string_set(&(v),buf->data,buf->pos);
						}
					}
				}else
				{
					oi=wtk_owl_item_find_item2_s(sub_prop,"nlg.check_unk");
					if(oi && oi->nv>0)
					{
						//wtk_debug("[%.*s]\n",oi->v[0]->len,oi->v[0]->data);
						ret=wtk_owlkg_process_nlg(kg,oi,act,item->v,buf);
						if(ret==0 && buf->pos>0)
						{
							wtk_string_set(&(v),buf->data,buf->pos);
						}
					}
				}
			}
		}
		break;
	}
end:
	if(json)
	{
		wtk_json_reset(json);
	}
	//wtk_debug("[%.*s]\n",v.len,v.data);
	//exit(0);
	return v;
}

/*
#owl.set(人:爸爸,feeling,人,喜欢:妈妈)
  => class, inst
  =>   ->item
  =>   ->item
  =>   ->item:value
#owl.get(人:爸爸,feeling,人,喜欢)
#owl.ask(人:爸爸,feeling,人,喜欢)
<class="人">
	<prop>
		<item="feeling">
			<item="人">
				<item="喜欢">
					<item=ask_nlg v="person_like_person_ask"/>
					<item=set_nlg v="person_like_person_set"/>
					<item=get_nlg v="person_like_person_get"/>
				</item>
				<item="讨厌">
					<item=set_nlg v="person_dislike_person_set"/>
				</item>
			</item>
			<item="动物">
				<item="喜欢">
					<item=set_nlg v="person_like_animal_set"/>
					<item=ask_nlg v="person_like_animal_ask"/>
				</item>
				<item="讨厌">
					<item=set_nlg v="person_dislike_animal_set"/>
				</item>
			</item>
		</item>
	</prop>
*/
wtk_string_t wtk_owlkg_process(wtk_owlkg_t *kg,char *data,int len)
{
typedef enum
{
	WTK_OWLKG_ACT_INIT,
	WTK_OWLKG_ACT_NAME,
	WTK_OWLKG_ACT_WAIT_ITEM,
	WTK_OWLKG_ACT_NAME_WAIT_BRACE,
	WTK_OWLKG_ACT_ITEM_KEY,
	WTK_OWLKG_ACT_ITEM_KEY_WAIT_VALUE,
	WTK_OWLKG_ACT_ITEM_KEY_WAIT_NXT,
	WTK_OWLKG_ACT_ITEM_VALUE,
	WTK_OWLKG_ACT_WAIT_ITEM_NXT,
}wtk_owlkg_act_state_t;
	wtk_string_t v;
	char *s,*e;
	int n;
	wtk_strbuf_t *buf=kg->buf;
	wtk_owlkg_act_t act;
	wtk_heap_t *heap=kg->heap;
	wtk_owlkg_act_state_t state;
	wtk_owlkg_act_item_t *item=NULL;

	wtk_owlkg_act_init(&(act));
	state=WTK_OWLKG_ACT_INIT;
	wtk_string_set(&(v),0,0);
	s=data;e=s+len;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		switch(state)
		{
		case WTK_OWLKG_ACT_INIT:
			if(n>1 || !isspace(*s))
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,s,n);
				state=WTK_OWLKG_ACT_NAME;
			}
			break;
		case WTK_OWLKG_ACT_NAME:
			if(n==1 && (*s=='('||isspace(*s)))
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				if(wtk_str_equal_s(buf->data,buf->pos,"owl.ask"))
				{
					act.type=WTK_OWLKG_ACT_ASK;
				}else if(wtk_str_equal_s(buf->data,buf->pos,"owl.get"))
				{
					act.type=WTK_OWLKG_ACT_GET;
				}else if(wtk_str_equal_s(buf->data,buf->pos,"owl.add"))
				{
					act.type=WTK_OWLKG_ACT_ADD;
				}else if(wtk_str_equal_s(buf->data,buf->pos,"owl.del"))
				{
					act.type=WTK_OWLKG_ACT_DEL;
				}else if(wtk_str_equal_s(buf->data,buf->pos,"owl.check"))
				{
					act.type=WTK_OWLKG_ACT_CHECK;
				}else
				{
					wtk_debug("unkown act [%.*s]\n",buf->pos,buf->data);
					goto end;
				}
				if(*s=='(')
				{
					state=WTK_OWLKG_ACT_WAIT_ITEM;
				}else
				{
					state=WTK_OWLKG_ACT_NAME_WAIT_BRACE;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_OWLKG_ACT_NAME_WAIT_BRACE:
			if(n==1 && *s=='(')
			{
				state=WTK_OWLKG_ACT_WAIT_ITEM;
			}
			break;
		case WTK_OWLKG_ACT_WAIT_ITEM_NXT:
			if(n==1)
			{
				switch(*s)
				{
				case ',':
					state=WTK_OWLKG_ACT_WAIT_ITEM;
					break;
				}
			}
			break;
		case WTK_OWLKG_ACT_WAIT_ITEM:
			if(n>1 || !isspace(*s))
			{
				if(*s==')')
				{

				}else
				{
					wtk_strbuf_reset(buf);
					wtk_strbuf_push(buf,s,n);
					state=WTK_OWLKG_ACT_ITEM_KEY;
				}
			}
			break;
		case WTK_OWLKG_ACT_ITEM_KEY:
			if(n==1 &&(*s==':'||*s==','||*s==')' || isspace(*s)))
			{
				//wtk_debug("[%.*s]\n",n,s);
				item=(wtk_owlkg_act_item_t*)wtk_heap_malloc(heap,sizeof(wtk_owlkg_act_item_t));
				item->k=wtk_heap_dup_string(heap,buf->data,buf->pos);
				item->v=NULL;
				//wtk_debug("[%.*s]\n",item->k->len,item->k->data);
				wtk_queue_push(&(act.item_q),&(item->q_n));
				//exit(0);
				switch(*s)
				{
				case ':':
					state=WTK_OWLKG_ACT_ITEM_KEY_WAIT_VALUE;
					break;
				case ',':
					state=WTK_OWLKG_ACT_WAIT_ITEM;
					break;
				case ')':
					break;
				default:
					state=WTK_OWLKG_ACT_ITEM_KEY_WAIT_NXT;
					break;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		case WTK_OWLKG_ACT_ITEM_KEY_WAIT_NXT:
			if(n==1)
			{
				switch(*s)
				{
				case ':':
					state=WTK_OWLKG_ACT_ITEM_KEY_WAIT_VALUE;
					break;
				case ',':
					state=WTK_OWLKG_ACT_WAIT_ITEM;
					break;
				case ')':
					break;
				default:
					break;
				}
			}
			break;
		case WTK_OWLKG_ACT_ITEM_KEY_WAIT_VALUE:
			if(n>1 || !isspace(*s))
			{
				wtk_strbuf_reset(buf);
				wtk_strbuf_push(buf,s,n);
				state=WTK_OWLKG_ACT_ITEM_VALUE;
			}
			break;
		case WTK_OWLKG_ACT_ITEM_VALUE:
			if(n==1 && (*s==','||*s==')'||isspace(*s)))
			{
				item->v=wtk_heap_dup_string(heap,buf->data,buf->pos);
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				switch(*s)
				{
				case ',':
					state=WTK_OWLKG_ACT_WAIT_ITEM;
					break;
				case ')':
					break;
				default:
					state=WTK_OWLKG_ACT_WAIT_ITEM_NXT;
					break;
				}
			}else
			{
				wtk_strbuf_push(buf,s,n);
			}
			break;
		}
		s+=n;
	}
	//wtk_owlkg_act_print(&(act));
	v=wtk_owlkg_process_act(kg,&(act));
end:
	wtk_heap_reset(heap);
	return v;
}
