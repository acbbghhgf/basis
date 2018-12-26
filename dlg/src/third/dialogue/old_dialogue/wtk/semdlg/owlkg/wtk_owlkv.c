#include "wtk_owlkv.h" 
#include <ctype.h>
int wtk_owlkv_check_depend(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_json_t *json);
wtk_string_t* wtk_owlkv_if_get_var(wtk_json_t *json,char *k,int k_len);

wtk_owlkv_t* wtk_owlkv_new(wtk_owlkv_cfg_t *cfg,wtk_lexr_t *lex)
{
	wtk_owlkv_t *kv;

	kv=(wtk_owlkv_t*)wtk_malloc(sizeof(wtk_owlkv_t));
	kv->cfg=cfg;
	kv->class_kv=wtk_jsonkv_new(cfg->class_dn);
	kv->inst_kv=wtk_jsonkv_new(cfg->inst_dn);
	kv->lex=lex;
	kv->buf=wtk_strbuf_new(256,1);
	//wtk_debug("[%s]\n",cfg->owl_fn);
	kv->owl=wtk_owl_tree_new_fn(cfg->owl_fn);
	//wtk_owl_tree_print(kv->owl);
	//exit(0);
	kv->heap=wtk_heap_new(4096);
	kv->save_json=0;
	kv->tmp=wtk_strbuf_new(256,1);
	kv->nlg=wtk_nlg2_new_fn(cfg->nlg_fn,NULL);
	return kv;
}

void wtk_owlkv_delete(wtk_owlkv_t *kv)
{
	wtk_strbuf_delete(kv->tmp);
	wtk_nlg2_delete(kv->nlg);
	wtk_heap_delete(kv->heap);
	wtk_strbuf_delete(kv->buf);
	wtk_jsonkv_delete(kv->class_kv);
	wtk_jsonkv_delete(kv->inst_kv);
	wtk_free(kv);
}

#define wtk_owlkv_norm_lex_s(kv,net,data,len,k) wtk_owlkv_norm_lex(kv,net,data,len,k,sizeof(k)-1)

wtk_string_t* wtk_owlkv_norm_lex(wtk_owlkv_t *kg,wtk_lex_net_t *net,char *data,int len,char *k,int k_bytes)
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

wtk_string_t* wtk_owlkv_inst_to_class(wtk_owlkv_t *kv,char *p,int len)
{
	wtk_string_t *v;

	v=wtk_owlkv_norm_lex_s(kv,kv->cfg->net_class,p,len,"class");
	return v;
}

wtk_owl_item_t* wtk_owlkv_get_unselect_item_value(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_json_t *json)
{
	wtk_string_t *pv;
	wtk_owl_item_t *oi=NULL,*ti;
	wtk_larray_t *a;
	int i;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji,*nji;
	int b;
	int has_json;
	int has_njson;

	wtk_owl_item_print_path(item,buf);
	a=wtk_larray_new(item->nitem,sizeof(void*));
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_json=ji && ji->type==WTK_JSON_ARRAY && ji->v.array->length>0;
	//确认是否是否定
	wtk_strbuf_push_s(buf,"^");
	nji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_njson=nji && nji->type==WTK_JSON_ARRAY && nji->v.array->length>0;
	for(i=0;i<item->nitem;++i)
	{
		ti=item->items[i];
		pv=wtk_owl_item_find_attr_value_s(ti,"ask_active",-1);
		if(pv && wtk_string_cmp_s(pv,"0")==0)
		{
			continue;
		}
		if(has_njson)
		{
			b=wtk_json_array_has_string_value(nji,ti->k.str->data,ti->k.str->len);
			if(b)
			{
				continue;
			}
		}
		if(has_json)
		{
			b=wtk_json_array_has_string_value(ji,ti->k.str->data,ti->k.str->len);
			if(!b)
			{
				wtk_larray_push2(a,&(ti));
			}
		}else
		{
			wtk_larray_push2(a,&(ti));
		}
	}
	if(a->nslot>0)
	{
		i=wtk_random(0,a->nslot-1);
		oi=((wtk_owl_item_t**)a->slot)[i];
	}
	wtk_larray_delete(a);
	return oi;
}

wtk_owl_item_t* wtk_owlkv_find_item_topic(wtk_owlkv_t *kv,wtk_json_t *json,wtk_owl_item_t *item,wtk_json_item_t *ji)
{
	int i;
	wtk_owl_item_t *oi;
	wtk_json_item_t *ti;
	wtk_owl_item_attr_kv_t *akv;
	wtk_string_t *v;
	int b;

	if(item->use_k_inst)
	{
		return NULL;
	}
	v=wtk_owl_item_find_attr_value_s(item,"ask_active",-1);
	if(v && wtk_string_cmp_s(v,"0")==0)
	{
		return NULL;
	}
	if(item->v && item->nv>0)
	{
		if(ji)
		{
		//wtk_debug("[%.*s]=[%.*s]\n",item->k.str->len,item->k.str->data,item->v[0]->len,item->v[0]->data);
			ti=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
			if(!ti)
			{
				wtk_json_obj_add_str(json,ji,item->k.str->data,item->k.str->len,item->v[0]);
				//exit(0);
				kv->save_json=1;
			}
		}
		return NULL;
	}
	if(item->nitem<=0)
	{
		akv=wtk_owl_tree_find_attr_s(kv->owl,item,"ask",0);
		//wtk_debug("[%.*s] %p parent=%p\n",item->k.str->len,item->k.str->data,akv,item->parent_item);
		if(!akv)
		{
			return NULL;
		}
		b=wtk_owlkv_check_depend(kv,item,json);
		if(!b)
		{
			return NULL;
		}
		//wtk_debug("========================> ji=%p\n",ji);
		if(ji && ji->type==WTK_JSON_OBJECT)
		{
			v=wtk_owl_item_find_attr_value_s(item,"ref",-1);
			if(v)
			{
				ji=wtk_json_item_get_path_item(json->main,v->data,v->len,NULL);
			}else
			{
				ji=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
			}
			if(!ji || (ji->type==WTK_JSON_OBJECT && ji->v.object->length==0))
			{
				//wtk_debug("return\n");
				return item;
			}
		}else
		{
			//wtk_debug("return\n");
			//wtk_debug("type=%d/%d\n",ji->type,WTK_JSON_OBJECT);
			return item;
		}
	}
	if(ji)
	{
		//wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
		ti=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
		if(!ti)
		{
			ti=wtk_json_new_object(json);
			//wtk_debug("[%.*s]\n",oi->k.str->len,oi->k.str->data);
			wtk_json_obj_add_item2(json,ji,item->k.str->data,item->k.str->len,ti);
			kv->save_json=1;
		}
		ji=ti;
	}else
	{
		ti=NULL;
	}
	akv=wtk_owl_tree_find_attr_s(kv->owl,item,"select",0);
	if(akv)
	{
		if(ti && (ti->type!=WTK_JSON_OBJECT || ti->v.object->length>0))
		{
			//wtk_debug("return\n");
			//exit(0);
			return NULL;
		}
		oi=wtk_owlkv_get_unselect_item_value(kv,item,json);
		if(!oi)
		{
			return NULL;
		}
//		i=wtk_random(0,item->nitem-1);
//		oi=item->items[i];
//		if(oi->use_k_inst)
//		{
//			return NULL;
//		}
		oi=wtk_owlkv_find_item_topic(kv,json,oi,ji);
		if(oi)
		{
			return oi;
		}
	}else
	{
		for(i=0;i<item->nitem;++i)
		{
			oi=item->items[i];
			if(oi->use_k_inst)
			{
				continue;
			}
			oi=wtk_owlkv_find_item_topic(kv,json,oi,ji);
			if(oi)
			{
				return oi;
			}
		}
	}
	return NULL;
}

wtk_owl_item_t* wtk_owlkv_find_topic(wtk_owlkv_t *kv,wtk_json_t *json,wtk_owl_class_t *cls)
{
	wtk_owl_item_t *item;
	wtk_json_item_t *ji;
	int i;
	wtk_owl_item_t *oi;

	ji=json->main;
	for(i=0;i<cls->prop->nitem;++i)
	{
		item=cls->prop->items[i];
		oi=wtk_owlkv_find_item_topic(kv,json,item,ji);
		if(oi)
		{
			//wtk_debug("find prop\n");
			return oi;
		}
	}
	if(cls->parent)
	{
		oi=wtk_owlkv_find_topic(kv,json,cls->parent);
		if(oi)
		{
			//wtk_debug("find prop\n");
			return oi;
		}
	}
	return NULL;
}

wtk_owl_class_t* wtk_owlkv_get_class(wtk_owlkv_t *kv,char *hint,int hint_bytes)
{
	wtk_string_t *vp,v;
	wtk_owl_class_t *cls=NULL;

	if(kv->cfg->net_class)
	{
		vp=wtk_owlkv_norm_lex_s(kv,kv->cfg->net_class,hint,hint_bytes,"class");
		v=*vp;
	}else
	{
		wtk_string_set(&(v),hint,hint_bytes);
	}
	cls=wtk_owl_tree_find_class(kv->owl,v.data,v.len,0);
	return cls;
}

wtk_owl_item_t* wtk_owlkv_get_ask_topic(wtk_owlkv_t *kv,char *hint,int hint_bytes)
{
	//wtk_strbuf_t *buf=kv->buf;
	wtk_owl_class_t *cls;
	wtk_json_t *json;
	//wtk_owl_item_t *item;
	wtk_owl_item_t *oi=NULL;
	wtk_jsonkv_t *jkv=kv->inst_kv;

	kv->save_json=0;
	cls=wtk_owlkv_get_class(kv,hint,hint_bytes);
	if(!cls){goto end;}
	//wtk_strbuf_reset(buf);
	//wtk_debug("[%.*s]\n",hint_bytes,hint);
	json=wtk_jsonkv_get_json(jkv,hint,hint_bytes);
	oi=wtk_owlkv_find_topic(kv,json,cls);
	//wtk_owl_class_print(cls);
	//wtk_owl_item_print(item);
	if(kv->save_json)
	{
		//wtk_jsonkv_save_json(jkv,cls->item->k.str->data,cls->item->k.str->len,json);
		wtk_jsonkv_save_json(jkv,hint,hint_bytes,json);
	}
end:
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	//wtk_string_set(&(v),buf->data,buf->pos);
	return oi;
}

wtk_owl_item_t* wtk_owlkv_get_topic2(wtk_owlkv_t *kv,char *hint,int hint_bytes,char *vitem,int item_bytes)
{
	//wtk_strbuf_t *buf=kv->buf;
	wtk_owl_class_t *cls;
	wtk_json_t *json;
	//wtk_owl_item_t *item;
	wtk_owl_item_t *oi=NULL;
	wtk_jsonkv_t *jkv=kv->inst_kv;
	wtk_owl_item_t *item;
	wtk_json_item_t *ji;

	kv->save_json=0;
	cls=wtk_owlkv_get_class(kv,hint,hint_bytes);
	if(!cls){goto end;}
	//wtk_debug("[%.*s]\n",item_bytes,vitem);
	item=wtk_owlkv_get_owl_item(kv,cls->item->k.str->data,cls->item->k.str->len,vitem,item_bytes);
	if(!item){goto end;}
	//wtk_owl_item_print(item);
	json=wtk_jsonkv_get_json(jkv,hint,hint_bytes);
	if(!json){goto end;}
	ji=wtk_json_item_get_path_item(json->main,vitem,item_bytes,NULL);
	//wtk_json_item_print3(ji);
	oi=wtk_owlkv_find_item_topic(kv,json,item,ji);
	//wtk_owl_item_print(oi);
	if(kv->save_json)
	{
		wtk_jsonkv_save_json(jkv,cls->item->k.str->data,cls->item->k.str->len,json);
	}
end:
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	//wtk_string_set(&(v),buf->data,buf->pos);
	return oi;
}


typedef enum
{
	WTK_OWLKV_DEPEND_LT,
	WTK_OWLKV_DEPEND_LET,
	WTK_OWLKV_DEPEND_EQ,
	WTK_OWLKV_DEPEND_GET,
	WTK_OWLKV_DEPEND_GT,
}wtk_owlkv_depend_type_t;

typedef struct
{
	wtk_string_t k;
	wtk_string_t v;
	wtk_owlkv_depend_type_t type;
}wtk_owlkv_depend_t;

void wtk_owlkv_depend_print(wtk_owlkv_depend_t *d)
{
	printf("%.*s",d->k.len,d->k.data);
	switch(d->type)
	{
	case WTK_OWLKV_DEPEND_LT:
		printf("<");
		break;
	case WTK_OWLKV_DEPEND_LET:
		printf("<=");
		break;
	case WTK_OWLKV_DEPEND_EQ:
		printf("=");
		break;
	case WTK_OWLKV_DEPEND_GET:
		printf(">=");
		break;
	case WTK_OWLKV_DEPEND_GT:
		printf(">");
		break;
	}
	printf("%.*s\n",d->v.len,d->v.data);
}

int wtk_owlkv_depend_init(wtk_owlkv_depend_t *d,char *data,int bytes)
{
typedef enum
{
	WTK_OWLKV_DPEND_INIT,
	WTK_OWLKV_DPEND_KEY,
	WTK_OWLKV_DPEND_WAIT_SYM,
	WTK_OWLKV_DPEND_WAIT_SYM2,
	WTK_OWLKV_DPEND_V,
}wtk_owlkv_depend_state_t;
	char *s,*e;
	int n;
	wtk_owlkv_depend_state_t state;

	wtk_string_set(&(d->k),0,0);
	wtk_string_set(&(d->v),0,0);
	s=data;
	e=s+bytes;
	state=WTK_OWLKV_DPEND_INIT;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		switch(state)
		{
		case WTK_OWLKV_DPEND_INIT:
			if(n>1 || !isspace(*s))
			{
				d->k.data=s;
				state=WTK_OWLKV_DPEND_KEY;
			}
			break;
		case WTK_OWLKV_DPEND_KEY:
			if(n==1)
			{
				switch(*s)
				{
				case '>':
					d->k.len=s-d->k.data;
					d->type=WTK_OWLKV_DEPEND_GT;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				case '=':
					d->k.len=s-d->k.data;
					d->type=WTK_OWLKV_DEPEND_EQ;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				case '<':
					d->k.len=s-d->k.data;
					d->type=WTK_OWLKV_DEPEND_LT;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				default:
					if(isspace(*s))
					{
						d->k.len=s-d->k.data;
						state=WTK_OWLKV_DPEND_WAIT_SYM;
					}
					break;
				}
			}
			break;
		case WTK_OWLKV_DPEND_WAIT_SYM:
			if(n==1)
			{
				switch(*s)
				{
				case '>':
					d->type=WTK_OWLKV_DEPEND_GT;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				case '=':
					d->type=WTK_OWLKV_DEPEND_EQ;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				case '<':
					d->type=WTK_OWLKV_DEPEND_LT;
					state=WTK_OWLKV_DPEND_WAIT_SYM2;
					break;
				}
			}
			break;
		case WTK_OWLKV_DPEND_WAIT_SYM2:
			if(n>1 || !isspace(*s))
			{
				if(n==1&&*s=='=')
				{
					switch(d->type)
					{
					case WTK_OWLKV_DEPEND_LT:
						d->type=WTK_OWLKV_DEPEND_LET;
						break;
					case WTK_OWLKV_DEPEND_GT:
						d->type=WTK_OWLKV_DEPEND_GET;
						break;
					default:
						break;
					}
				}else
				{
					d->v.data=s;
					state=WTK_OWLKV_DPEND_V;
				}
			}
			break;
		case WTK_OWLKV_DPEND_V:
			if(n==1&&isspace(*s))
			{
				d->v.len=s-d->v.data;
			}else if(s+n>=e)
			{
				d->v.len=e-d->v.data;
			}
			break;
		}
		s+=n;
	}
	//wtk_owlkv_depend_print(d);
	return d->v.len>0?0:-1;
}

int wtk_owlkv_check_depend2(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_json_item_t *json)
{
	wtk_owl_item_attr_kv_t *akv;
	wtk_string_t *v;
	wtk_owlkv_depend_t d;
	int b=1;
	float ret;
	wtk_json_item_t *ji;

	akv=wtk_owl_tree_find_attr_s(kv->owl,item,"depend",0);
	if(!akv){goto end;}
	v=wtk_owl_item_attr_kv_get_value(akv,-1);
	if(!v){goto end;}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	ret=wtk_owlkv_depend_init(&(d),v->data,v->len);
	if(ret!=0){goto end;}
	wtk_owlkv_depend_print(&(d));
	ji=wtk_json_item_get_path_item(json,d.k.data,d.k.len,NULL);
	if(!ji){b=0;goto end;}
	//wtk_json_item_print3(ji);
	switch(ji->type)
	{
	case WTK_JSON_STRING:
		ret=wtk_string_cmp(ji->v.str,d.v.data,d.v.len);
		break;
	case WTK_JSON_NUMBER:
		ret=wtk_str_atof(d.v.data,d.v.len);
		ret=ji->v.number-ret;
		break;
	default:
		goto end;
		break;
	}
	switch(d.type)
	{
	case WTK_OWLKV_DEPEND_LT:
		b=ret<0?1:0;
		break;
	case WTK_OWLKV_DEPEND_LET:
		b=ret<=0?1:0;
		break;
	case WTK_OWLKV_DEPEND_EQ:
		b=ret==0?1:0;
		break;
	case WTK_OWLKV_DEPEND_GET:
		b=ret>=0?1:0;
		break;
	case WTK_OWLKV_DEPEND_GT:
		b=ret>0?1:0;
		break;
	}
	//wtk_debug("b=%d ret=%f\n",b,ret);
	//exit(0);
end:
	return b;
}



int wtk_owlkv_check_depend(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_json_t *json)
{
	wtk_string_t *v;
	int b=1;
	wtk_if_t *xif;
	wtk_heap_t *heap=kv->heap;

	v=wtk_owl_item_find_attr_value_s(item,"depend",-1);
	if(!v){goto end;}
	xif=wtk_if_new(heap,v->data,v->len);
	if(!xif){goto end;}
	b=wtk_if_check(xif,json,(wtk_if_get_var_f)wtk_owlkv_if_get_var);
end:
	//wtk_owl_item_print(item);
	//wtk_debug("depend=%d\n",b);
	return b;
}

wtk_owl_item_t* wtk_owlkv_set_inst_value(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_json_t *json,wtk_json_item_t *ji,char *a,int a_bytes)
{
	int i;
	wtk_owl_item_t *oi;
	wtk_json_item_t *ti;
	wtk_owl_item_attr_kv_t *akv;
	wtk_string_t *nm;
	wtk_lex_net_t *net;
	wtk_lexr_t *lex;
	wtk_json_item_t *tji;
	int b;
	int is_number=0;


	if(item->use_k_inst)
	{
		return NULL;
	}
	if(item->nitem<=0)
	{
		akv=wtk_owl_tree_find_attr_s(kv->owl,item,"lex",0);
		if(!akv)
		{
			//wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
			return NULL;
		}
		nm=wtk_owl_item_attr_kv_get_value(akv,-1);
		if(!nm)
		{
			wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
			return NULL;
		}
		net=wtk_str_hash_find(kv->cfg->hash,nm->data,nm->len);
		if(!net)
		{
			wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
			return NULL;
		}
		//b=wtk_owlkv_check_depend(kv,item,json->main);
		//wtk_debug("=============> [%.*s] found\n",item->k.str->len,item->k.str->data);
		//wtk_debug("[%.*s]=%d\n",item->k.str->len,item->k.str->data,b);
		nm=wtk_owl_item_find_attr_value_s(item,"type",-1);
		if(nm)
		{
			if(wtk_string_cmp_s(nm,"int")==0)
			{
				is_number=1;
			}
		}
		lex=kv->lex;
		wtk_lexr_process(lex,net,a,a_bytes);
		if(lex->action && lex->action->type==WTK_JSON_OBJECT)
		{
			//wtk_lexr_print(lex);
			tji=wtk_json_obj_get_s(lex->action,"request");
			if(tji && tji->type==WTK_JSON_OBJECT)
			{
				tji=wtk_json_obj_get_s(tji,"value");
				if(item)
				{
					//wtk_json_item_print3(tji);
					nm=wtk_json_item_get_str_value(tji);
					//wtk_debug("nm=%p\n",nm);
					if(nm)
					{
						//wtk_debug("nm=%.*s\n",nm->len,nm->data);
						wtk_json_obj_remove(ji,item->k.str->data,item->k.str->len);
						if(is_number)
						{
							b=wtk_str_atoi(nm->data,nm->len);
							wtk_json_obj_add_ref_number(json,ji,item->k.str->data,item->k.str->len,b);
						}else
						{
							wtk_json_obj_add_str2(json,ji,item->k.str->data,item->k.str->len,nm->data,nm->len);
						}
						//wtk_debug("=====================================>\n");
						//wtk_json_item_print3(ji);
						kv->save_json=1;
					}
					//exit(0);
				}
			}
		}
		wtk_lexr_reset(kv->lex);
		//exit(0);
	}
	if(item->nitem<=0)
	{
		return NULL;
	}
	if(item->k.str)
	{
		//wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
		ti=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
		if(!ti)
		{
			ti=wtk_json_new_object(json);
			wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
			wtk_json_obj_add_item2(json,ji,item->k.str->data,item->k.str->len,ti);
			kv->save_json=1;
		}
		ji=ti;
	}
	for(i=0;i<item->nitem;++i)
	{
		oi=item->items[i];
		if(oi->use_k_inst)
		{
			continue;
		}
		//wtk_debug("[%.*s]\n",oi->k.str->len,oi->k.str->data);
		//exit(0);
		oi=wtk_owlkv_set_inst_value(kv,oi,json,ji,a,a_bytes);
		if(oi)
		{
			return oi;
		}
	}
	return NULL;
}

void wtk_owlkv_set_class_value(wtk_owlkv_t *kv,wtk_owl_class_t *cls,wtk_json_t *json,char *a,int a_bytes)
{
	if(cls->prop)
	{
		wtk_owlkv_set_inst_value(kv,cls->prop,json,json->main,a,a_bytes);
	}
	if(cls->parent)
	{
		wtk_owlkv_set_class_value(kv,cls->parent,json,a,a_bytes);
	}
}

int wtk_owlkv_is_value_valid(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes)
{
	wtk_owl_item_t *oi;
	wtk_string_t *pv;
	int b=0;

	pv=wtk_owlkv_inst_to_class(kv,p,p_bytes);
	oi=wtk_owlkv_get_owl_item(kv,pv->data,pv->len,a,a_bytes);
	if(!oi)
	{
		goto end;
	}
	b=wtk_owlkv_get_matched_item_lex(kv,oi,v,v_bytes,NULL,1);
end:
	return b;
}

wtk_owl_item_t* wtk_owlkv_owl_item_set_value2(wtk_owlkv_t *kv,wtk_json_t *json,wtk_owl_item_t *oi,char *v,int v_bytes)
{
	wtk_heap_t *heap=kv->heap;
	wtk_string_t lex_v;
	wtk_string_t *pv;
	wtk_owl_item_t *ri=NULL;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	int b;

	//wtk_owl_item_print(oi);
	//wtk_debug("[%.*s] ni=%d\n",v_bytes,v,oi->nitem);
	if(oi->nitem>0)
	{
		//wtk_owl_item_print(oi);
		oi=wtk_owlkv_get_matched_item(kv,oi,v,v_bytes,&(lex_v));
		if(!oi){goto end;}
		wtk_owl_item_print(oi);
		//exit(0);
		if(lex_v.len>0)
		{
			//wtk_debug("[%.*s]\n",lex_v.len,lex_v.data);
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
			//v=lex_v.data;
			//v_bytes=lex_v.len;
		}
		//wtk_owl_item_print(oi);
		//wtk_debug("[%.*s]\n",v_bytes,v);
		ri=oi;
		b=0;
		if(oi->parent_item)
		{
			pv=wtk_owl_item_find_attr_value_s(oi->parent_item,"select",-1);
			if(pv)
			{
				b=1;
				oi=oi->parent_item;
				wtk_owl_item_print_path(oi,buf);
				if(wtk_string_cmp_s(pv,"1")==0)
				{
					//wtk_json_item_print3(json->main);
					//wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v_bytes,v);
					ji=wtk_json_item_add_path_item(json,json->main,buf->data,buf->pos,WTK_JSON_ARRAY);
					//wtk_json_item_print3(ji);
					if(!wtk_json_array_has_string_value(ji,v,v_bytes))
					{
						wtk_json_array_add_str(json,ji,v,v_bytes);
					}
				}else
				{
					wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
				}
			}
		}
		if(!b)
		{
			wtk_owl_item_print_path(oi,buf);
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
	}else
	{
		int b;

		b=wtk_owlkv_get_matched_item_lex(kv,oi,v,v_bytes,&(lex_v),0);
		if(b && lex_v.len>0)
		{
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
		}
		ri=oi;
		wtk_owl_item_print_path(oi,buf);
		pv=wtk_owl_item_find_attr_value_s(oi,"select",-1);
		if(pv && wtk_string_cmp_s(pv,"1")==0)
		{
			ji=wtk_json_item_add_path_item(json,json->main,buf->data,buf->pos,WTK_JSON_ARRAY);
			if(!wtk_json_array_has_string_value(ji,v,v_bytes))
			{
				wtk_json_array_add_str(json,ji,v,v_bytes);
			}
		}else
		{
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
		//exit(0);
	}
end:
	return ri;
}

void wtk_owlkv_owl_item_set_select_value(wtk_owlkv_t *kv,wtk_owl_item_t *oi,wtk_json_t *json,char *v,int v_bytes,wtk_string_t *pv)
{
	wtk_strbuf_t *buf;
	wtk_json_item_t *ji;

	buf=wtk_strbuf_new(256,1);
	oi=oi->parent_item;
	wtk_owl_item_print_path(oi,buf);
	if(wtk_string_cmp_s(pv,"1")==0)
	{
		//wtk_json_item_print3(json->main);
		//wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v_bytes,v);
		ji=wtk_json_item_add_path_item(json,json->main,buf->data,buf->pos,WTK_JSON_ARRAY);
		//wtk_json_item_print3(ji);
		if(!wtk_json_array_has_string_value(ji,v,v_bytes))
		{
			wtk_json_array_add_str(json,ji,v,v_bytes);
		}
	}else
	{
		wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
	}
	wtk_strbuf_delete(buf);
}

wtk_owl_item_t* wtk_owlkv_owl_get_matched_value_item(wtk_owlkv_t *kv,wtk_json_t *json,wtk_owl_item_t *oi,char *v,int v_bytes)
{
	wtk_heap_t *heap=kv->heap;
	wtk_string_t lex_v;
	wtk_string_t *pv;
	wtk_owl_item_t *ri=NULL;
	wtk_strbuf_t *buf=kv->buf;
	int b;

	//wtk_owl_item_print(oi);
	//wtk_debug("[%.*s] ni=%d\n",v_bytes,v,oi->nitem);
	if(oi->nitem>0)
	{
		//wtk_owl_item_print(oi);
		oi=wtk_owlkv_get_matched_item(kv,oi,v,v_bytes,&(lex_v));
		if(!oi){goto end;}
		wtk_owl_item_print(oi);
		//exit(0);
		if(lex_v.len>0)
		{
			//wtk_debug("[%.*s]\n",lex_v.len,lex_v.data);
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
			//v=lex_v.data;
			//v_bytes=lex_v.len;
		}
		//wtk_owl_item_print(oi);
		//wtk_debug("[%.*s]\n",v_bytes,v);
		//exit(0);
		ri=oi;
		b=0;
		if(oi->parent_item)
		{
			pv=wtk_owl_item_find_attr_value_s(oi->parent_item,"select",-1);
			if(pv)
			{
				b=1;
				wtk_owlkv_owl_item_set_select_value(kv,oi,json,v,v_bytes,pv);
			}
		}
		if(!b)
		{
			wtk_owl_item_print_path(oi,buf);
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
	}else
	{
		//我喜欢白色
		b=wtk_owlkv_get_matched_item_lex(kv,oi,v,v_bytes,&(lex_v),0);
		//wtk_debug("b=%d lex_len=%d\n",b,lex_v.len);
		if(b && lex_v.len>0)
		{
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
		}
		//wtk_debug("[%.*s]\n",v_bytes,v);
		ri=oi;
		wtk_owl_item_print_path(oi,buf);
		b=0;
		if(oi->parent_item)
		{
			pv=wtk_owl_item_find_attr_value_s(oi->parent_item,"select",-1);
			if(pv)
			{
				b=1;
				wtk_owlkv_owl_item_set_select_value(kv,oi,json,v,v_bytes,pv);
			}
		}
		if(b==0)
		{
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
		//exit(0);
	}
end:
	return ri;
}

wtk_owl_item_t* wtk_owlkv_get_matched_owl(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes)
{
	wtk_json_t *json;
	wtk_string_t inst;
	wtk_jsonkv_t *jkv=kv->inst_kv;
	wtk_owl_item_t *oi;
	wtk_string_t *pv;
	wtk_owl_item_t *ri=NULL;
	wtk_owl_class_t *cls;

	//wtk_debug("[%.*s]=[%.*s] [%.*s]\n",p_bytes,p,a_bytes,a,v_bytes,v);
	pv=wtk_owlkv_inst_to_class(kv,p,p_bytes);
	cls=wtk_owl_tree_find_class(kv->owl,pv->data,pv->len,0);
	if(!cls){goto end;}
	if(a_bytes<=0)
	{
		oi=cls->prop;
	}else
	{
		oi=wtk_owl_item_find_path(cls->prop,a,a_bytes);
	}
	//oi=wtk_owlkv_get_owl_item(kv,pv->data,pv->len,a,a_bytes);
	if(!oi)
	{
		goto end;
	}
	//wtk_owl_item_print(oi);
	wtk_string_set(&(inst),p,p_bytes);
	//wtk_debug("[%.*s]\n",inst.len,inst.data);
	json=wtk_jsonkv_get_json(jkv,inst.data,inst.len);
	//wtk_json_item_print3(json->main);
	ri=wtk_owlkv_owl_get_matched_value_item(kv,json,oi,v,v_bytes);
end:
	wtk_heap_reset(kv->heap);
	wtk_jsonkv_reset(jkv);
	return ri;
}

wtk_owl_item_t* wtk_owlkv_set_value(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes)
{
	wtk_json_t *json;
	wtk_string_t inst;
	wtk_jsonkv_t *jkv=kv->inst_kv;
	wtk_owl_item_t *oi,*ti;
	wtk_string_t *pv;
	wtk_owl_item_t *ri=NULL;
	wtk_owl_class_t *cls;

	//wtk_debug("[%.*s]=[%.*s] [%.*s]\n",p_bytes,p,a_bytes,a,v_bytes,v);
	pv=wtk_owlkv_inst_to_class(kv,p,p_bytes);
	cls=wtk_owl_tree_find_class(kv->owl,pv->data,pv->len,0);
	if(!cls){goto end;}
	if(a_bytes<=0)
	{
		oi=cls->prop;
	}else
	{
		oi=wtk_owl_item_find_path(cls->prop,a,a_bytes);
	}
	//oi=wtk_owlkv_get_owl_item(kv,pv->data,pv->len,a,a_bytes);
	if(!oi)
	{
		goto end;
	}
	//wtk_owl_item_print(oi);
	wtk_string_set(&(inst),p,p_bytes);
	//wtk_debug("[%.*s]\n",inst.len,inst.data);
	json=wtk_jsonkv_get_json(jkv,inst.data,inst.len);
	//wtk_json_item_print3(json->main);
	ri=wtk_owlkv_owl_get_matched_value_item(kv,json,oi,v,v_bytes);
	if(ri)
	{
		pv=wtk_owl_item_find_attr_value_s(ri,"ref_set",-1);
		if(pv)
		{
			wtk_strkv_parser_t kvp;
			int ret;

			wtk_strkv_parser_init(&(kvp),pv->data,pv->len);
			while(1)
			{
				ret=wtk_strkv_parser_next(&(kvp));
				if(ret!=0){break;}
				//wtk_debug("[%.*s]=[%.*s]\n",kvp.k.len,kvp.k.data,kvp.v.len,kvp.v.data);
				if(kvp.v.len>0)
				{
					ti=wtk_owl_item_find_path(cls->prop,kvp.k.data,kvp.k.len);
					if(ti)
					{
						wtk_owlkv_owl_get_matched_value_item(kv,json,ti,kvp.v.data,kvp.v.len);
					}
				}
			}
		}
	}
	//wtk_debug("[%.*s]\n",inst.len,inst.data);
	//wtk_json_item_print3(json->main);
	//exit(0);
	wtk_jsonkv_save_json(jkv,inst.data,inst.len,json);
//	wtk_owlkv_set_class_value(kv,cls,json,a,a_bytes);
//	wtk_json_item_print3(json->main);
//	exit(0);
//	if(kv->save_json)
//	{
//		wtk_json_item_print3(json->main);
//		wtk_jsonkv_save_json(jkv,inst.data,inst.len,json);
//	}
end:
	wtk_heap_reset(kv->heap);
	wtk_jsonkv_reset(jkv);
	return ri;
}

wtk_owl_item_t* wtk_owlkv_set_negative_value(wtk_owlkv_t *kv,char *p,int p_bytes,char *a,int a_bytes,char *v,int v_bytes)
{
	wtk_json_t *json;
	wtk_string_t inst;
	wtk_jsonkv_t *jkv=kv->inst_kv;
	wtk_owl_item_t *oi;
	wtk_string_t *pv;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	wtk_owl_item_t *ri=NULL;
	wtk_string_t lex_v;
	int b;
	wtk_heap_t *heap=kv->heap;

	//wtk_debug("[%.*s]=[%.*s] [%.*s]\n",p_bytes,p,a_bytes,a,v_bytes,v);
	pv=wtk_owlkv_inst_to_class(kv,p,p_bytes);
	oi=wtk_owlkv_get_owl_item(kv,pv->data,pv->len,a,a_bytes);
	if(!oi)
	{
		goto end;
	}
	//wtk_owl_item_print(oi);
	wtk_string_set(&(inst),p,p_bytes);
	json=wtk_jsonkv_get_json(jkv,inst.data,inst.len);
	//wtk_json_item_print3(json->main);
	if(oi->nitem>0)
	{
		oi=wtk_owlkv_get_matched_item(kv,oi,v,v_bytes,&(lex_v));
		if(!oi){goto end;}
		if(lex_v.len>0)
		{
			//wtk_debug("[%.*s]\n",lex_v.len,lex_v.data);
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
			//v=lex_v.data;
			//v_bytes=lex_v.len;
		}
		//wtk_owl_item_print(oi);
		//wtk_debug("[%.*s]\n",v_bytes,v);
		ri=oi;
		b=0;
		if(oi->parent_item)
		{
			pv=wtk_owl_item_find_attr_value_s(oi->parent_item,"select",-1);
			if(pv)
			{
				b=1;
				oi=oi->parent_item;
				wtk_owl_item_print_path(oi,buf);
				wtk_strbuf_push_s(buf,"^");
				if(wtk_string_cmp_s(pv,"1")==0)
				{
					//wtk_json_item_print3(json->main);
					//wtk_debug("[%.*s]=[%.*s]\n",buf->pos,buf->data,v_bytes,v);
					ji=wtk_json_item_add_path_item(json,json->main,buf->data,buf->pos,WTK_JSON_ARRAY);
					//wtk_json_item_print3(ji);
					if(!wtk_json_array_has_string_value(ji,v,v_bytes))
					{
						wtk_json_array_add_str(json,ji,v,v_bytes);
					}
				}else
				{
					wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
				}
			}
		}
		if(!b)
		{
			wtk_owl_item_print_path(oi,buf);
			wtk_strbuf_push_s(buf,"^");
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
	}else
	{
		int b;

		b=wtk_owlkv_get_matched_item_lex(kv,oi,v,v_bytes,&(lex_v),0);
		if(b && lex_v.len>0)
		{
			pv=wtk_heap_dup_string(heap,lex_v.data,lex_v.len);
			v=pv->data;
			v_bytes=pv->len;
		}
		ri=oi;
		pv=wtk_owl_item_find_attr_value_s(oi,"select",-1);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,a,a_bytes);
		wtk_strbuf_push_s(buf,"^");
		if(pv && wtk_string_cmp_s(pv,"1")==0)
		{
			ji=wtk_json_item_add_path_item(json,json->main,buf->data,buf->pos,WTK_JSON_ARRAY);
			if(!wtk_json_array_has_string_value(ji,v,v_bytes))
			{
				wtk_json_array_add_str(json,ji,v,v_bytes);
			}
		}else
		{
			wtk_json_item_set_path_str(json,buf->data,buf->pos,v,v_bytes);
		}
		//exit(0);
	}
	wtk_jsonkv_save_json(jkv,inst.data,inst.len,json);
//	wtk_owlkv_set_class_value(kv,cls,json,a,a_bytes);
//	wtk_json_item_print3(json->main);
//	exit(0);
//	if(kv->save_json)
//	{
//		wtk_json_item_print3(json->main);
//		wtk_jsonkv_save_json(jkv,inst.data,inst.len,json);
//	}
end:
	wtk_heap_reset(kv->heap);
	wtk_jsonkv_reset(jkv);
	return ri;
}

wtk_owl_item_t* wtk_owlkv_get_owl_item(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len)
{
	wtk_owl_item_t *pv=NULL;
	wtk_owl_class_t *cls;

	//wtk_debug("[%.*s] [%.*s]\n",p_len,p,item_len,item);
	cls=wtk_owl_tree_find_class(kv->owl,p,p_len,0);
	if(!cls){goto end;}
	if(item_len<=0)
	{
		pv=cls->prop;
		goto end;
	}
	pv=wtk_owl_item_find_path(cls->prop,item,item_len);
end:
	//exit(0);
	return pv;
}

wtk_string_t* wtk_owlkv_get_attr(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *a,int a_len)
{
	wtk_string_t *pv=NULL;
	wtk_owl_item_t *ci;

	ci=wtk_owlkv_get_owl_item(kv,p,p_len,item,item_len);
	if(!ci){goto end;}
	pv=wtk_owl_item_find_attr_value(ci,a,a_len,-1);
	if(!pv)
	{
		wtk_debug("[%.*s] not found\n",a_len,a);
		goto end;
	}
end:
	//exit(0);
	return pv;
}

wtk_string_t wtk_owlkv_gen_text(wtk_owlkv_t *kv,char *p,int p_len,char *nlg,int nlg_len,wtk_nlg2_gen_env_t *env)
{
	wtk_string_t v;
	//wtk_string_t *vp;
	wtk_heap_t *heap=kv->heap;
	wtk_nlg2_function_t f;
	wtk_json_t *json;
	int i;
	wtk_string_spliter_t sp;
	int ret;
	wtk_larray_t *a;
	wtk_larray_t *b;
	wtk_json_item_t *ji;
	wtk_string_t k,*kp;
	int ki;
	char tmp[64];

	a=wtk_larray_new(5,sizeof(void*));
	b=wtk_larray_new(5,sizeof(void*));
	wtk_string_set(&(v),0,0);
//	if(kv->cfg->net_inst)
//	{
//		vp=wtk_owlkv_norm_lex_s(kv,kv->cfg->net_inst,p,p_len,"inst");
//		v=*vp;
//	}else
	{
		wtk_string_set(&(v),p,p_len);
	}
	json=wtk_jsonkv_get_json(kv->inst_kv,v.data,v.len);
	wtk_nlg2_function_init(&(f));
	wtk_nlg2_function_parse(&(f),heap,nlg,nlg_len,NULL);
	for(i=0;i<f.narg;++i)
	{
		//wtk_debug("[%.*s]\n",f.args[i]->k->len,f.args[i]->k->data);
		if(f.args[i]->v)
		{
			if(f.args[i]->v->data[0]=='.')
			{
				ji=wtk_json_item_get_path_item(json->main,f.args[i]->v->data+1,f.args[i]->v->len-1,NULL);
				//swtk_debug("ji=%p\n",ji);
				if(ji)
				{
					kp=wtk_json_item_get_str_value2(ji);
					if(kp)
					{
						f.args[i]->v=wtk_heap_dup_string(heap,kp->data,kp->len);
						wtk_string_delete(kp);
					}
				}
			}
			//wtk_debug("continue\n");
			continue;
		}
		if(wtk_string_cmp_s(f.args[i]->k,"p")==0)
		{
			f.args[i]->v=&v;
		}else
		{
			wtk_larray_reset(a);
			wtk_larray_reset(b);
			wtk_string_spliter_init_s(&(sp),f.args[i]->k->data,f.args[i]->k->len,"|");
			while(1)
			{
				ret=wtk_string_spliter_next(&(sp));
				//wtk_debug("ret=%d\n",ret);
				if(ret<0){break;}
				//wtk_debug("[%.*s]\n",sp.v.len,sp.v.data);
				ji=wtk_json_item_get_path_item(json->main,sp.v.data,sp.v.len,&k);
				if(ji)
				{
					//wtk_json_item_print3(ji);
					if(ji->type==WTK_JSON_NUMBER)
					{
						sprintf(tmp,"%d",(int)(ji->v.number));
						//kp=ji->v.number;
						kp=wtk_heap_dup_string(heap,tmp,strlen(tmp));
					}else
					{
						kp=wtk_json_item_get_str_value(ji);
					}
					if(kp)
					{
						//wtk_json_item_print3(ji);
						wtk_larray_push2(a,&(kp));
						//wtk_debug("[%.*s]\n",k.len,k.data);
						kp=wtk_heap_dup_string(heap,k.data,k.len);
						wtk_larray_push2(b,&(kp));
					}
				}
			}
			if(a->nslot<=0)
			{
				//wtk_debug("[%.*s] wrong\n",f.args[i]->k->len,f.args[i]->k->data);
				//goto end;
			}else
			{
				ki=wtk_random(0,a->nslot-1);
				f.args[i]->v=((wtk_string_t**)a->slot)[ki];
				f.args[i]->k=((wtk_string_t**)b->slot)[ki];
			//wtk_debug("[%.*s]=[%.*s]\n",f.args[i]->k->len,f.args[i]->k->data,f.args[i]->v->len,f.args[i]->v->data);
			//f.args[i]->v=wtk_json_item_get_path_string(json->main,f.args[i]->k->data,f.args[i]->k->len);
			}
		}
		//wtk_debug("v=%p\n",f.args[i]->v);
	}
	//wtk_nlg2_function_print(&(f));
	v=wtk_nlg2_process_function(kv->nlg,&f,env);
//end:
	wtk_larray_delete(a);
	wtk_larray_delete(b);
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	return v;
}


void wtk_owl_class_dump_json(wtk_owl_item_t *item,wtk_json_t *json,wtk_json_item_t *ji)
{
	wtk_json_item_t *vi;
	int i,j;
	wtk_string_t *v;
	wtk_owl_item_t *oi;

//	if(item->k.str)
//	{
//		wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
//		vi=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
//		if(vi){return;}
//	}
	v=wtk_owl_item_find_attr_value_s(item,"virtual",-1);
	if(v && wtk_string_cmp_s(v,"1")==0)
	{
		return;
	}
	if(item->nitem>0)
	{
		//wtk_owl_item_print(item);
		v=wtk_owl_item_find_attr_value_s(item,"select",-1);
		if(v)
		{
			if(wtk_string_cmp_s(v,"0")==0)
			{

			}else
			{
				if(item->k.str)
				{
					vi=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
					if(!vi)
					{
						vi=wtk_json_new_array(json);
						wtk_json_obj_add_item2(json,ji,item->k.str->data,item->k.str->len,vi);
					}
					for(i=0;i<item->nitem;++i)
					{
						oi=item->items[i];
						if(oi->nv>0)
						{
							j=wtk_random(0,oi->nv-1);
							v=oi->v[j];
							if(!wtk_json_array_has_string_value(vi,v->data,v->len))
							{
								wtk_json_array_add_str(json,vi,v->data,v->len);
							}
						}
					}
				}
			}
		}else
		{
			if(item->k.str)
			{
				vi=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
				if(!vi)
				{
					vi=wtk_json_new_object(json);
					wtk_json_obj_add_item2(json,ji,item->k.str->data,item->k.str->len,vi);
					//wtk_json_item_print3(ji);
				}
			}else
			{
				vi=ji;
			}
			for(i=0;i<item->nitem;++i)
			{
				wtk_owl_class_dump_json(item->items[i],json,vi);
			}
		}
	}else
	{
		if(item->nv>0)
		{
			vi=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
			if(!vi)
			{
				i=wtk_random(0,item->nv-1);
				v=item->v[i];
				wtk_json_obj_add_str2(json,ji,item->k.str->data,item->k.str->len,v->data,v->len);
			}
		}
	}
	if(item->class_parent_item)
	{
		wtk_owl_class_dump_json(item->class_parent_item,json,ji);
	}
}


void wtk_owlkv_touch_inst(wtk_owlkv_t *kv,char *inst,int inst_len,char *xcls,int cls_len)
{
	wtk_json_t *json;
	wtk_owl_class_t *cls;

	//wtk_debug("[%.*s]=[%.*s]\n",inst_len,inst,cls_len,xcls);
	json=wtk_jsonkv_get_json(kv->inst_kv,inst,inst_len);
	if(json->main->type==WTK_JSON_OBJECT && json->main->v.object->length>0)
	{
		goto end;
	}else
	{
		cls=wtk_owl_tree_find_class(kv->owl,inst,inst_len,0);
		if(!cls)
		{
			cls=wtk_owl_tree_find_class(kv->owl,xcls,cls_len,0);
			if(!cls)
			{
				goto end;
			}
		}
		//wtk_debug("[%.*s]\n",cls->item->k.str->len,cls->item->k.str->data);
		//wtk_owl_class_print(cls);
		wtk_owl_class_dump_json(cls->prop,json,json->main);
		wtk_json_obj_add_str2_s(json,json->main,"class",cls->item->k.str->data,cls->item->k.str->len);
		//wtk_json_item_print3(json->main);
		//wtk_debug("[%.*s]\n",inst_len,inst);
		//exit(0);
		wtk_jsonkv_save_json(kv->inst_kv,inst,inst_len,json);
		//exit(0);
	}
end:
	//wtk_json_item_print3(json->main);
	//exit(0);
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
}

int wtk_owlkv_get_matched_item_lex(wtk_owlkv_t *kv,wtk_owl_item_t *item,char *a,int a_bytes,wtk_string_t* v,int use_parent)
{
	wtk_owl_item_attr_kv_t *akv;
	wtk_string_t *nm;
	wtk_lex_net_t *net;
	wtk_lexr_t *lex;
	wtk_json_item_t *tji;
	int b=0;
	wtk_strbuf_t *tmp=kv->tmp;
	wtk_string_t *xv;

	if(v)
	{
		wtk_string_set((v),0,0);
	}
	akv=wtk_owl_tree_find_attr2(kv->owl,item,"lex",3,0,use_parent);
	if(!akv)
	{
		//wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
		return 0;
	}
	nm=wtk_owl_item_attr_kv_get_value(akv,-1);
	if(!nm)
	{
		wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
		return 0;
	}
	net=wtk_str_hash_find(kv->cfg->hash,nm->data,nm->len);
	if(!net)
	{
		wtk_debug("[%.*s] not found\n",item->k.str->len,item->k.str->data);
		return 0;
	}
	//wtk_debug("[%.*s]\n",nm->len,nm->data);
	lex=kv->lex;
	wtk_lexr_process(lex,net,a,a_bytes);
	//wtk_lexr_print(lex);
	if(lex->action && lex->action->type==WTK_JSON_OBJECT)
	{
		//wtk_debug("[%.*s]\n",nm->len,nm->data);
		//wtk_lexr_print(lex);
		tji=wtk_json_obj_get_s(lex->action,"request");
		if(tji && tji->type==WTK_JSON_OBJECT)
		{
			tji=wtk_json_obj_get_s(tji,"value");
			//wtk_debug("tjiv=%p v=%p\n",tji,v);
			if(tji)
			{
				if(v)
				{
					wtk_strbuf_reset(tmp);
					xv=wtk_json_item_get_str_value2(tji);
					//wtk_debug("xv=%p\n",xv);
					if(xv)
					{
						wtk_strbuf_push(tmp,xv->data,xv->len);
						wtk_string_set((v),tmp->data,tmp->pos);
						wtk_string_delete(xv);
					}
				}
				b=1;
			}
		}
	}
	wtk_lexr_reset(kv->lex);
	return b;
}

wtk_owl_item_t* wtk_owlkv_get_matched_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,char *a,int a_bytes,wtk_string_t *v)
{
	wtk_owl_item_t *vi;
	int i;
	wtk_string_t px;

	if(v)
	{
		wtk_string_set(v,0,0);
	}
//	if(item->k.str)
//	{
//		wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
//	}
	if(item->nitem>0)
	{
		i=wtk_owlkv_get_matched_item_lex(kv,item,a,a_bytes,&px,1);
		//wtk_debug("i=%d\n",i);
		if(i)
		{
			vi=wtk_owl_item_find_item2(item,px.data,px.len);
			wtk_debug("vi=%p\n",vi);
			if(vi)
			{
				if(v)
				{
					*v=px;
				}
				wtk_debug("return vi=%p\n",vi);
				return vi;
			}
		}
		for(i=0;i<item->nitem;++i)
		{
			vi=wtk_owlkv_get_matched_item(kv,item->items[i],a,a_bytes,v);
			if(vi)
			{
				//wtk_debug("============found\n");
				return vi;
			}
		}
	}else
	{
		for(i=0;i<item->nv;++i)
		{
			if(wtk_string_cmp(item->v[i],a,a_bytes)==0)
			{
				//wtk_debug("============found\n");
				return item;//->parent_item;
			}
		}
		if(!item->use_k_inst && item->k.str && wtk_string_cmp(item->k.str,a,a_bytes)==0)
		{
			//wtk_debug("============found\n");
			return item;//->parent_item;
		}
		i=wtk_owlkv_get_matched_item_lex(kv,item,a,a_bytes,v,0);
		//wtk_debug("i=%d\n",i);
		if(i)
		{
			//wtk_debug("============found\n");
			return item;//->parent_item;
		}
	}
	if(item->class_parent_item)
	{
		return wtk_owlkv_get_matched_item(kv,item->class_parent_item,a,a_bytes,v);
	}
	return NULL;
}

wtk_owl_item_t* wtk_owlkv_get_owl_by_p(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len)
{
	wtk_owl_class_t *cls;
	wtk_owl_item_t *oi=NULL;

	//wtk_debug("[%.*s]\n",item_len,item);
	cls=wtk_owlkv_get_class(kv,p,p_len);
	if(!cls)
	{
		wtk_debug("cls[%.*s] not found\n",p_len,p);
		goto end;
	}
	oi=wtk_owl_item_find_path(cls->prop,item,item_len);
end:
	return oi;
}

wtk_owlkv_item_t wtk_owlkv_get_owl_value(wtk_owlkv_t *kv,char *p,int p_len,wtk_owl_item_t *oi)
{
	wtk_json_t *json;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	wtk_string_t *xv;
	wtk_json_obj_item_t *joi;
	wtk_owlkv_item_t vi;

	wtk_string_set(&(vi.v),0,0);
	wtk_string_set(&(vi.k),0,0);
	wtk_owl_item_print_path(oi,buf);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	json=wtk_jsonkv_get_json(kv->inst_kv,p,p_len);
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	if(!ji){goto end;}
	wtk_json_item_print3(ji);
	if(ji->type==WTK_JSON_OBJECT)
	{
		joi=wtk_json_obj_get_valid_item(ji);
		if(!joi){goto end;}
		wtk_debug("[%.*s]\n",joi->k.len,joi->k.data);
		xv=wtk_json_item_get_str_value2(joi->item);
		if(!xv){goto end;}
		wtk_strbuf_push_s(buf,".");
		wtk_strbuf_push(buf,joi->k.data,joi->k.len);
		wtk_string_set(&(vi.k),buf->data,buf->pos);
		wtk_strbuf_reset(kv->tmp);
		wtk_strbuf_push(kv->tmp,xv->data,xv->len);
		wtk_string_set(&(vi.v),kv->tmp->data,kv->tmp->pos);
		//wtk_debug("[%.*s]=[%.*s]\n",joi->k.len,joi->k.data,xv->len,xv->data);
		//exit(0);
	}else
	{
		wtk_string_set(&(vi.k),buf->data,buf->pos);
		xv=wtk_json_item_get_str_value2(ji);
		wtk_debug("xv=%p\n",xv);
		if(!xv){goto end;}
		wtk_debug("[%.*s]\n",xv->len,xv->data);
		wtk_strbuf_reset(kv->tmp);
		wtk_strbuf_push(kv->tmp,xv->data,xv->len);
		wtk_string_set(&(vi.v),kv->tmp->data,kv->tmp->pos);
		wtk_string_delete(xv);
	}
end:
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	return vi;
}

wtk_owlkv_item_t wtk_owlkv_get_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len)
{
	wtk_owl_item_t *oi;
	wtk_owlkv_item_t vi;

	wtk_string_set(&(vi.v),0,0);
	wtk_string_set(&(vi.k),0,0);
	//wtk_debug("[%.*s] [%.*s]\n",p_len,p,item_len,item);
	oi=wtk_owlkv_get_owl_by_p(kv,p,p_len,item,item_len);
	if(!oi){goto end;}
	vi=wtk_owlkv_get_owl_value(kv,p,p_len,oi);
end:
	return vi;
}

wtk_string_t wtk_owlkv_get_json_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len)
{
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_t *json;
	wtk_json_item_t *ji;
	wtk_string_t v,*xp;

	wtk_string_set(&(v),0,0);
	json=wtk_jsonkv_get_json(kv->inst_kv,p,p_len);
	ji=wtk_json_item_get_path_item(json->main,item,item_len,NULL);
	if(!ji){goto end;}
	xp=wtk_json_item_get_str_value2(ji);
	if(!xp){goto end;}
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,xp->data,xp->len);
	wtk_string_set(&(v),buf->data,buf->pos);
	wtk_string_delete(xp);
end:
	wtk_jsonkv_reset(kv->inst_kv);
	return v;
}

int wtk_owlkv_check_value(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *v,int v_len)
{
	wtk_owl_item_t *oi;
	int ret=0;
	wtk_json_t *json;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	int k;
	wtk_string_t lex_v;

	wtk_debug("[%.*s] [%.*s]\n",p_len,p,item_len,item);
	oi=wtk_owlkv_get_owl_by_p(kv,p,p_len,item,item_len);
	if(!oi){goto end;}
	//wtk_owl_item_print(oi);
	oi=wtk_owlkv_get_matched_item(kv,oi,v,v_len,&(lex_v));
	if(!oi){goto end;}
	v=lex_v.data;
	v_len=lex_v.len;
	oi=oi->parent_item;
	wtk_owl_item_print_path(oi,buf);
	wtk_debug("[%.*s]\n",buf->pos,buf->data);
	json=wtk_jsonkv_get_json(kv->inst_kv,p,p_len);
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	if(!ji){goto end;}
	wtk_json_item_print3(ji);
	ret=0;
	switch(ji->type)
	{
	case WTK_JSON_FALSE:
		if(wtk_str_equal_s(v,v_len,"false"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_TRUE:
		if(wtk_str_equal_s(v,v_len,"true"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_NULL:
		if(wtk_str_equal_s(v,v_len,"nil"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_STRING:
		if(wtk_str_equal(v,v_len,ji->v.str->data,ji->v.str->len))
		{
			ret=1;
		}
		break;
	case WTK_JSON_NUMBER:
		k=wtk_str_atoi(v,v_len);
		if(k==ji->v.number)
		{
			ret=1;
		}
		break;
	case WTK_JSON_ARRAY:
		ret=wtk_json_array_has_string_value(ji,v,v_len);
		break;
	case WTK_JSON_OBJECT:
		ji=wtk_json_obj_get(ji,v,v_len);
		if(ji)
		{
			ret=1;
		}
		break;
	}
end:
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	return ret;
}

wtk_string_t wtk_owkv_get_owl_nlg(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *nlg_nm,wtk_string_t *p)
{
	wtk_string_t vx;
	wtk_strbuf_t *buf=kv->buf;
	wtk_string_t *pv;
	wtk_owl_item_attr_kv_t *akv;

	//wtk_owl_item_print(item);
	wtk_string_set(&(vx),0,0);
	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,nlg_nm->data,nlg_nm->len);
	wtk_strbuf_push_s(buf,"(p=\"");
	wtk_strbuf_push(buf,p->data,p->len);
	wtk_strbuf_push_s(buf,"\"");
	if(item)
	{
		pv=wtk_owl_item_find_attr_value_s(item,"args",-1);
		//wtk_debug("pv=%p\n",pv);
		if(pv)
		{
			wtk_strbuf_push_s(buf,",");
			wtk_strbuf_push(buf,pv->data,pv->len);
		}
		if(item->use_k_inst==0 && item->k.str && item->parent_item)
		{
	//		wtk_owl_item_print(item);
	//		okv=wtk_owlkv_get_owl_value(kv,p->data,p->len,item);
	//		wtk_debug("[%.*s]=[%.*s]\n",okv.k.len,okv.k.data,okv.v.len,okv.v.data);
	//		exit(0);
			akv=wtk_owl_item_find_attr_s(item->parent_item,"select");
			if(akv)
			{
				wtk_strbuf_push_s(buf,",");
				wtk_strbuf_push_s(buf,"v=\"");
				wtk_strbuf_push(buf,item->k.str->data,item->k.str->len);
				wtk_strbuf_push_s(buf,"\"");
			}else
			{
				wtk_strbuf_t *tmp=kv->tmp;
				wtk_json_t *json;
				wtk_json_item_t *ji;
				wtk_string_t *v;

				wtk_owl_item_print_path(item,tmp);
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				json=wtk_jsonkv_get_json(kv->inst_kv,p->data,p->len);
				ji=wtk_json_item_get_path_item(json->main,tmp->data,tmp->pos,NULL);
				if(ji)
				{
					v=wtk_json_item_get_str_value2(ji);
					if(v)
					{
						wtk_strbuf_push_s(buf,",");
						wtk_strbuf_push_s(buf,"v=\"");
						//wtk_strbuf_push(buf,item->k.str->data,item->k.str->len);
						//wtk_strbuf_push_s(buf,"=\"");
						wtk_strbuf_push(buf,v->data,v->len);
						wtk_strbuf_push_s(buf,"\"");
						wtk_string_delete(v);
					}
				}
				wtk_jsonkv_reset(kv->inst_kv);
			}
		}
	}
	wtk_strbuf_push_s(buf,")");
	wtk_string_set(&(vx),buf->data,buf->pos);
	return vx;
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	//vx=wtk_owlkv_gen_text(kv,p->data,p->len,buf->data,buf->pos);
	//return vx;
}

wtk_string_t wtk_owkv_get_owl_nlg_text(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *nlg_nm,wtk_string_t *p,wtk_nlg2_gen_env_t *env)
{
	wtk_string_t v;

	v=wtk_owkv_get_owl_nlg(kv,item,nlg_nm,p);
	v=wtk_owlkv_gen_text(kv,p->data,p->len,v.data,v.len,env);
	return v;
}

wtk_string_t wtk_owlkv_ans(wtk_owlkv_t *kv,char *p,int p_len,char *item,int item_len,char *a,int a_bytes,char *v,int v_len)
{
	wtk_owl_item_t* oi,*ti;
	wtk_string_t vx;
	wtk_strbuf_t *buf=kv->buf;
	wtk_string_t *pv;

	wtk_string_set(&(vx),0,0);
	oi=wtk_owlkv_get_owl_by_p(kv,p,p_len,item,item_len);
	if(!oi){goto end;}
	wtk_strbuf_reset(buf);
	pv=wtk_owl_item_find_attr_value(oi,a,a_bytes,-1);
	if(!pv){goto end;}
	wtk_strbuf_push(buf,pv->data,pv->len);
	wtk_strbuf_push_s(buf,"(p=\"");
	wtk_strbuf_push(buf,p,p_len);
	wtk_strbuf_push_s(buf,"\"");
	pv=wtk_owl_item_find_attr_value_s(oi,"select",-1);
	if(pv)
	{
		ti=wtk_owl_item_find_item2(oi,v,v_len);
		if(ti)
		{
			oi=ti;
		}
	}
	pv=wtk_owl_item_find_attr_value_s(oi,"args",-1);
	if(pv)
	{
		wtk_strbuf_push_s(buf,",");
		wtk_strbuf_push(buf,pv->data,pv->len);
	}
	wtk_strbuf_push_s(buf,")");
	wtk_debug("[%.*s]\n",buf->pos,buf->data);
	vx=wtk_owlkv_gen_text(kv,p,p_len,buf->data,buf->pos,NULL);
end:
	//wtk_debug("[%.*s]\n",vx.len,vx.data);
	//exit(0);
	return vx;
}

wtk_string_t* wtk_owlkv_if_get_var(wtk_json_t *json,char *k,int k_len)
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

wtk_string_t* wtk_owlkv_owl_item_get_attr(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *a,wtk_string_t *p)
{
	wtk_owl_item_attr_kv_t *akv;
	wtk_string_t *v=NULL;
	wtk_larray_t *l=NULL;
	wtk_queue_node_t *qn;
	wtk_owl_item_attr_v_t *av;
	int ret;
	wtk_json_t *json=NULL;
	int i;

	akv=wtk_owl_item_find_attr(item,a->data,a->len);
	if(!akv){goto end;}
	l=wtk_larray_new(akv->v.len,sizeof(void*));
	for(qn=akv->v.pop;qn;qn=qn->next)
	{
		av=data_offset2(qn,wtk_owl_item_attr_v_t,q_n);
		if(av->xif)
		{
			if(!json)
			{
				json=wtk_jsonkv_get_json(kv->inst_kv,p->data,p->len);
			}
			ret=wtk_if_check(av->xif,json,(wtk_if_get_var_f)wtk_owlkv_if_get_var);
			if(ret==1)
			{
				wtk_larray_push2(l,&(av));
			}
		}else
		{
			wtk_larray_push2(l,&(av));
		}
	}
	if(l->nslot>0)
	{
		if(kv->cfg->use_random)
		{
			i=wtk_random(0,l->nslot-1);
		}else
		{
			i=0;
		}
//		qn=wtk_queue3_peek(item->glb_expr_q,i);
//		expr=data_offset2(qn,wtk_rexpr_t,q_n);
		av=((wtk_owl_item_attr_v_t**)l->slot)[i];
		v=av->v;
	}
end:
	if(l)
	{
		wtk_larray_delete(l);
	}
	if(json)
	{
		wtk_jsonkv_reset(kv->inst_kv);
	}
	return v;
}

wtk_owl_item_t* wtk_owlkv_get_unselect_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *p)
{
	wtk_string_t *pv;
	wtk_owl_item_t *oi=NULL,*ti;
	wtk_larray_t *a;
	int i;
	wtk_json_t *json=NULL;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji,*nji;
	int b;
	int has_json;
	int has_njson;

	pv=wtk_owl_item_find_attr_value_s(item,"select",-1);
	if(!pv){goto end;}
	wtk_owl_item_print_path(item,buf);
	a=wtk_larray_new(item->nitem,sizeof(void*));
	json=wtk_jsonkv_get_json(kv->inst_kv,p->data,p->len);
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_json=ji && ji->type==WTK_JSON_ARRAY && ji->v.array->length>0;
	//确认是否是否定
	wtk_strbuf_push_s(buf,"^");
	nji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_njson=nji && nji->type==WTK_JSON_ARRAY && nji->v.array->length>0;
	for(i=0;i<item->nitem;++i)
	{
		ti=item->items[i];
		pv=wtk_owl_item_find_attr_value_s(ti,"ask_active",-1);
		if(pv && wtk_string_cmp_s(pv,"0")==0)
		{
			continue;
		}
		if(has_njson)
		{
			b=wtk_json_array_has_string_value(nji,ti->k.str->data,ti->k.str->len);
			if(b)
			{
				continue;
			}
		}
		if(has_json)
		{
			b=wtk_json_array_has_string_value(ji,ti->k.str->data,ti->k.str->len);
			if(!b)
			{
				wtk_larray_push2(a,&(ti));
			}
		}else
		{
			wtk_larray_push2(a,&(ti));
		}
	}
	if(a->nslot>0)
	{
		i=wtk_random(0,a->nslot-1);
		oi=((wtk_owl_item_t**)a->slot)[i];
	}
	wtk_larray_delete(a);
end:
	if(json)
	{
		wtk_jsonkv_reset(kv->inst_kv);
	}
	return oi;
}

wtk_owl_item_t* wtk_owlkv_get_unselect_item2(wtk_owlkv_t *kv,wtk_json_t *json,wtk_owl_item_t *item)
{
	wtk_string_t *pv;
	wtk_owl_item_t *oi=NULL,*ti;
	wtk_larray_t *a;
	int i;
	wtk_strbuf_t *buf;
	wtk_json_item_t *ji,*nji;
	int b;
	int has_json;
	int has_njson;

	buf=wtk_strbuf_new(256,1);
	pv=wtk_owl_item_find_attr_value_s(item,"select",-1);
	if(!pv){goto end;}
	wtk_owl_item_print_path(item,buf);
	a=wtk_larray_new(item->nitem,sizeof(void*));
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_json=ji && ji->type==WTK_JSON_ARRAY && ji->v.array->length>0;
	//确认是否是否定
	wtk_strbuf_push_s(buf,"^");
	nji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	has_njson=nji && nji->type==WTK_JSON_ARRAY && nji->v.array->length>0;
	for(i=0;i<item->nitem;++i)
	{
		ti=item->items[i];
		pv=wtk_owl_item_find_attr_value_s(ti,"ask_active",-1);
		if(pv && wtk_string_cmp_s(pv,"0")==0)
		{
			continue;
		}
		if(has_njson)
		{
			b=wtk_json_array_has_string_value(nji,ti->k.str->data,ti->k.str->len);
			if(b)
			{
				continue;
			}
		}
		if(has_json)
		{
			b=wtk_json_array_has_string_value(ji,ti->k.str->data,ti->k.str->len);
			if(!b)
			{
				wtk_larray_push2(a,&(ti));
			}
		}else
		{
			wtk_larray_push2(a,&(ti));
		}
	}
	if(a->nslot>0)
	{
		if(kv->cfg->use_random)
		{
			i=wtk_random(0,a->nslot-1);
		}else
		{
			i=0;
		}
		oi=((wtk_owl_item_t**)a->slot)[i];
	}
	wtk_larray_delete(a);
end:
	wtk_strbuf_delete(buf);
	return oi;
}

wtk_owl_item_t* wtk_owlkv_get_select_item(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *p)
{
	wtk_string_t *pv;
	wtk_owl_item_t *oi=NULL,*ti;
	wtk_larray_t *a;
	int i;
	wtk_json_t *json=NULL;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	int b;

	pv=wtk_owl_item_find_attr_value_s(item,"select",-1);
	if(!pv){goto end;}
	wtk_owl_item_print_path(item,buf);
	a=wtk_larray_new(item->nitem,sizeof(void*));
	json=wtk_jsonkv_get_json(kv->inst_kv,p->data,p->len);
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	if(ji && ji->type==WTK_JSON_ARRAY && ji->v.array->length>0)
	{
		for(i=0;i<item->nitem;++i)
		{
			ti=item->items[i];
			b=wtk_json_array_has_string_value(ji,ti->k.str->data,ti->k.str->len);
			if(b)
			{
				wtk_larray_push2(a,&(ti));
			}
		}
		if(a->nslot>0)
		{
			i=wtk_random(0,a->nslot-1);
			oi=((wtk_owl_item_t**)a->slot)[i];
		}
	}
	wtk_larray_delete(a);
end:
	if(json)
	{
		wtk_jsonkv_reset(kv->inst_kv);
	}
	return oi;
}

wtk_owlkv_item_t wtk_owlkv_get_owl_item_by_rel(wtk_owlkv_t *kv,char *data,int len)
{
	wtk_lexr_t *lex;
	wtk_json_item_t *item,*ji;
	wtk_string_t *p,*v;
	wtk_owlkv_item_t okv;
	char *ps;
	wtk_array_t *a;
	int i;

	wtk_string_set(&(okv.k),0,0);
	wtk_string_set(&(okv.v),0,0);
	lex=kv->lex;
	wtk_lexr_process(lex,kv->cfg->net_rel,data,len);
	//wtk_lexr_print(lex);
	if(lex->action && lex->action->type==WTK_JSON_OBJECT)
	{
		item=wtk_json_obj_get_s(lex->action,"request");
		if(item && item->type==WTK_JSON_OBJECT)
		{
			//class="人",prop="prt.爸爸"
			ji=wtk_json_obj_get_s(item,"class");
			if(ji)
			{
				p=wtk_json_item_get_str_value(ji);
				if(p)
				{
					ji=wtk_json_obj_get_s(item,"prop");
					if(ji)
					{
						//wtk_json_item_print3(ji);
						v=wtk_json_item_get_str_value(ji);
						if(v)
						{
							ps=wtk_str_chr(v->data,v->len,'|');
							if(ps)
							{
								a=wtk_str_to_array(lex->heap,v->data,v->len,'|');
								if(a)
								{
									if(kv->cfg->use_random)
									{
										i=wtk_random(0,a->nslot-1);
									}else
									{
										i=0;
									}
									v=((wtk_string_t**)a->slot)[i];
								}
							}
							wtk_strbuf_reset(kv->buf);
							wtk_strbuf_push(kv->buf,p->data,p->len);
							wtk_strbuf_reset(kv->tmp);
							wtk_strbuf_push(kv->tmp,v->data,v->len);
							wtk_string_set(&(okv.k),kv->buf->data,kv->buf->pos);
							wtk_string_set(&(okv.v),kv->tmp->data,kv->tmp->pos);
						}
					}
				}
			}
		}
	}
	wtk_lexr_reset(lex);
	return okv;
}

wtk_owl_item_t* wtk_owlkv_get_check_owl_topic(wtk_owlkv_t *kv,wtk_owl_item_t *item,wtk_string_t *v,wtk_json_t *json)
{
	wtk_json_item_t *ji;
	wtk_owl_item_t *oi;
	int b;

	//wtk_owl_item_print(item);
	item=wtk_owl_item_find_path(item,v->data,v->len);
	//wtk_debug("item=%p\n",item);
	if(!item)
	{
		return NULL;
	}
	if(item->nitem)
	{
		//check select value;
		ji=wtk_json_item_get_path_item(json->main,v->data,v->len,NULL);
		if(ji && wtk_json_item_len(ji)>0)
		{
			return NULL;
		}
		b=wtk_owlkv_check_depend(kv,item,json);
		if(!b)
		{
			return NULL;
		}
		oi=wtk_owlkv_get_unselect_item2(kv,json,item);
		if(oi)
		{
			return oi;
		}else
		{
			if(kv->cfg->use_random)
			{
				b=wtk_random(0,item->nitem-1);
			}else
			{
				b=0;
			}
			return item->items[b];
		}
	}else
	{
		//判断当前是否有值
		ji=wtk_json_item_get_path_item(json->main,v->data,v->len,NULL);
		if(ji)
		{
			return NULL;
		}
		//判断依赖条件是否满足
		b=wtk_owlkv_check_depend(kv,item,json);
		if(!b)
		{
			return NULL;
		}
		return item;
	}
}

wtk_owl_item_t* wtk_owlkv_get_freeask_topic(wtk_owlkv_t *kv,char *hint,int hint_bytes)
{
	wtk_owl_class_t *cls;
	wtk_json_t *json;
	wtk_owl_item_t *oi=NULL,*item,*ti;
	wtk_string_t *v;
	int i;
	wtk_larray_t *a;

	cls=wtk_owlkv_get_class(kv,hint,hint_bytes);
	if(!cls){goto end;}
	json=wtk_jsonkv_get_json(kv->inst_kv,hint,hint_bytes);
	item=wtk_owl_item_find_item2_s(cls->prop,"freeask");
	if(!item || item->nitem<=0){goto end;}
	//wtk_owl_item_print(item);
	a=wtk_larray_new(item->nitem,sizeof(void*));
	for(i=0;i<item->nitem;++i)
	{
		ti=item->items[i];
		if(ti->use_k_inst || !ti->k.str){continue;}
		v=ti->k.str;
		//wtk_debug("[%.*s]\n",v->len,v->data);
		ti=wtk_owlkv_get_check_owl_topic(kv,cls->prop,v,json);
		if(ti)
		{
			//wtk_debug("[%.*s]\n",v->len,v->data);
			wtk_larray_push2(a,&(ti));
		}
	}
	wtk_debug("nslot=%d\n",a->nslot);
	if(a->nslot>0)
	{
		if(kv->cfg->use_random)
		{
			i=wtk_random(0,a->nslot-1);
		}else
		{
			i=0;
		}
		oi=((wtk_owl_item_t**)a->slot)[i];
	}
	wtk_larray_delete(a);
end:
	//exit(0);
	wtk_jsonkv_reset(kv->inst_kv);
	return oi;
}

wtk_json_item_t* wtk_owlkv_set_json_item_value2(wtk_json_t *json,wtk_json_item_t *ji,wtk_owl_item_t *item,wtk_string_t *v)
{
	wtk_owl_item_attr_kv_t *kv=NULL;
	wtk_string_t *vx=v;
	wtk_json_item_t *ti;

	//wtk_owl_item_print(item);
	if(item->parent_item)
	{
		ji=wtk_owlkv_set_json_item_value2(json,ji,item->parent_item,NULL);
		if(!ji)
		{
			return NULL;
		}
	}
	if(!item->use_k_inst && item->k.str)
	{
		//wtk_debug("[%.*s]\n",item->k.str->len,item->k.str->data);
		if(item->parent_item)
		{
			kv=wtk_owl_item_find_attr_s(item->parent_item,"select");
		}
		//wtk_json_item_print3(ji);
		//wtk_debug("=============== kv=%p %p\n",kv,item->parent_item);
		//wtk_owl_item_print(item);
		if(kv)
		{
			if(!vx || ji->type!=WTK_JSON_ARRAY){return NULL;}
			ti=wtk_json_array_get_string_value(ji,vx->data,vx->len);
			//wtk_debug("ti=%p\n",ti);
			if(!ti)
			{
				ji=wtk_json_array_add_str(json,ji,vx->data,vx->len);
			}else
			{
				ji=ti;
			}
		}else
		{
			if(ji->type==WTK_JSON_OBJECT)
			{
				ti=wtk_json_obj_get(ji,item->k.str->data,item->k.str->len);
				if(item->parent_item)
				{
					kv=wtk_owl_item_find_attr_s(item->parent_item,"select");
				}else
				{
					kv=NULL;
				}
				//wtk_debug("ti=%p vx=%p kv=%p\n",ti,vx,kv);
				if(!ti)
				{
					if(vx)
					{
						wtk_string_t *pv;
						int i;

						pv=wtk_owl_item_find_attr_value_s(item,"type",-1);
						if(pv && wtk_string_cmp_s(pv,"int")==0)
						{
							i=wtk_str_atoi(v->data,v->len);
							ji=wtk_json_obj_add_ref_number(json,ji,item->k.str->data,item->k.str->len,i);
						}else
						{
							ji=wtk_json_obj_add_str2(json,ji,item->k.str->data,item->k.str->len,vx->data,vx->len);
						}
					}else
					{
						if(kv)
						{
							ti=wtk_json_new_array(json);
						}else
						{
							ti=wtk_json_new_object(json);
						}
						wtk_json_obj_add_item2(json,ji,item->k.str->data,item->k.str->len,ti);
						ji=ti;
					}
				}else
				{
					if(kv && ji->type!=WTK_JSON_ARRAY)
					{
						return NULL;
					}
					ji=ti;
					if(vx)
					{
						//如果设置select型值, 如[pa.性格]=[活跃型]
						if(kv)
						{
							ti=wtk_json_array_get_string_value(ji,vx->data,vx->len);
							//wtk_debug("ti=%p\n",ti);
							if(!ti)
							{
								ji=wtk_json_array_add_str(json,ji,vx->data,vx->len);
							}
						}else if(ji->type==WTK_JSON_STRING)
						{
							//wtk_json_item_print3(ji);
							if(wtk_string_cmp(ji->v.str,vx->data,vx->len)!=0)
							{
								ji->v.str=wtk_heap_dup_string(json->heap,vx->data,vx->len);
							}
						}else if(ji->type==WTK_JSON_NUMBER)
						{
							ji->v.number=wtk_str_atof(vx->data,vx->len);
						}
					}
				}
			}else
			{
				return NULL;
			}
		}
	}
	return ji;
}


void wtk_owlkv_set_json_item_neg_value2(wtk_json_t *json,wtk_json_item_t *ji,wtk_owl_item_t *item,wtk_string_t *v)
{
	wtk_strbuf_t *buf;
	wtk_owl_item_attr_kv_t *kv=NULL;
	wtk_json_item_t *ti;

	buf=wtk_strbuf_new(256,1);
	if(item->parent_item)
	{
		kv=wtk_owl_item_find_attr_s(item->parent_item,"select");
		if(kv)
		{
			item=item->parent_item;
		}
	}
	wtk_owl_item_print_path(item,buf);
	wtk_debug("[%.*s]\n",buf->pos,buf->data);
	if(kv)
	{
		ti=wtk_json_item_get_path_item(ji,buf->data,buf->pos,NULL);
		if(ti)
		{
			if(ti->type==WTK_JSON_ARRAY)
			{
				wtk_json_array_remove_string_value(ti,v->data,v->len);
			}
		}
	}
	wtk_strbuf_push_s(buf,"^");
	if(kv)
	{
		ti=wtk_json_item_add_path_item(json,ji,buf->data,buf->pos,WTK_JSON_ARRAY);
		wtk_json_array_add_unq_str(json,ti,v->data,v->len);
	}else
	{
		wtk_json_item_set_path_str(json,buf->data,buf->pos,v->data,v->len);
	}
	wtk_strbuf_delete(buf);
	//exit(0);
}

void wtk_owlkv_set_json_item_value(wtk_owlkv_t *kv,wtk_json_t *json,wtk_json_item_t *item,wtk_owl_item_t *oi,char *v,int v_bytes,int is_neg)
{
	wtk_string_t vx;

	wtk_string_set(&(vx),v,v_bytes);
	if(is_neg)
	{
		wtk_owlkv_set_json_item_neg_value2(json,item,oi,&vx);
	}else
	{
		wtk_owlkv_set_json_item_value2(json,item,oi,&vx);
	}
}

void wtk_owlkv_update_ref_value(wtk_owlkv_t *kv,wtk_json_t *json,wtk_json_item_t *item,wtk_owl_item_t *oi,char *v,int v_bytes)
{
	wtk_string_t *pv;
	wtk_strkv_parser_t kvp;
	int ret;
	wtk_owl_item_t *ti;
	wtk_json_item_t *ji;
	wtk_owl_class_t *cls;

	pv=wtk_owl_item_find_attr_value_s(oi,"ref_set",-1);
	if(!pv){return;}
	ji=wtk_json_obj_get_s(item,"class");
	if(!ji || ji->type!=WTK_JSON_STRING){return;}
	cls=wtk_owl_tree_find_class(kv->owl,ji->v.str->data,ji->v.str->len,0);
	if(!cls){return;}
	wtk_strkv_parser_init(&(kvp),pv->data,pv->len);
	while(1)
	{
		ret=wtk_strkv_parser_next(&(kvp));
		if(ret!=0){break;}
		wtk_debug("[%.*s]=[%.*s]\n",kvp.k.len,kvp.k.data,kvp.v.len,kvp.v.data);
		ti=wtk_owl_item_find_path(cls->prop,kvp.k.data,kvp.k.len);
		if(!ti)
		{
			wtk_debug("ti=%p not found\n",ti);
			continue;
		}
		wtk_owlkv_set_json_item_value(kv,json,item,ti,kvp.v.data,kvp.v.len,0);
	}
}

void wtk_owlkv_set_item_value(wtk_owlkv_t *kv,char *inst,int inst_bytes,wtk_owl_item_t *item,char *v,int v_bytes,int is_neg)
{
	wtk_json_t *json;
	wtk_jsonkv_t *jkv=kv->inst_kv;
	int b;
	wtk_string_t lex_v;

	//wtk_debug("[%.*s]\n",inst.len,inst.data);
	json=wtk_jsonkv_get_json(jkv,inst,inst_bytes);
	wtk_json_item_print3(json->main);
	b=wtk_owlkv_get_matched_item_lex(kv,item,v,v_bytes,&lex_v,1);
	if(b && lex_v.len>0)
	{
		v=lex_v.data;
		v_bytes=lex_v.len;
	}
	wtk_owlkv_set_json_item_value(kv,json,json->main,item,v,v_bytes,is_neg);
	if(!is_neg)
	{
		wtk_owlkv_update_ref_value(kv,json,json->main,item,v,v_bytes);
	}
	//wtk_json_item_print3(json->main);
	wtk_jsonkv_save_json(jkv,inst,inst_bytes,json);
	wtk_heap_reset(kv->heap);
	wtk_jsonkv_reset(jkv);
}

wtk_string_t* wtk_owlkv_get_freetalk_nlg(wtk_owlkv_t *kv,char *hint,int hint_bytes)
{
	wtk_owl_class_t *cls;
	wtk_owl_item_t *oi,*item;
	int i;
	wtk_string_t *v=NULL;

	cls=wtk_owlkv_get_class(kv,hint,hint_bytes);
	if(!cls){goto end;}
	item=wtk_owl_item_find_item2_s(cls->prop,"freetalk");
	if(!item || item->nitem<=0){goto end;}
	//wtk_owl_item_print(item);
	if(kv->cfg->use_random)
	{
		i=wtk_random(0,item->nitem-1);
	}else
	{
		i=0;
	}
	oi=item->items[i];
	//oi=wtk_owl_item_find_path(cls->prop,oi->k.str->data,oi->k.str->len);
	v=oi->v[0];
end:
	return v;
}

int wtk_owlkv_item_check_value(wtk_owlkv_t *kv,char *p,int p_len,wtk_owl_item_t *oi,char *v,int v_len)
{
	int ret=0;
	wtk_json_t *json;
	wtk_strbuf_t *buf=kv->buf;
	wtk_json_item_t *ji;
	int k;
	wtk_string_t lex_v;

	wtk_owl_item_print(oi);
	oi=wtk_owlkv_get_matched_item(kv,oi,v,v_len,&(lex_v));
	//wtk_debug("oi=%p\n",oi);
	if(!oi){goto end;}
	v=lex_v.data;
	v_len=lex_v.len;
	oi=oi->parent_item;
	wtk_owl_item_print_path(oi,buf);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	json=wtk_jsonkv_get_json(kv->inst_kv,p,p_len);
	ji=wtk_json_item_get_path_item(json->main,buf->data,buf->pos,NULL);
	if(!ji){goto end;}
	wtk_json_item_print3(ji);
	ret=0;
	switch(ji->type)
	{
	case WTK_JSON_FALSE:
		if(wtk_str_equal_s(v,v_len,"false"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_TRUE:
		if(wtk_str_equal_s(v,v_len,"true"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_NULL:
		if(wtk_str_equal_s(v,v_len,"nil"))
		{
			ret=1;
		}
		break;
	case WTK_JSON_STRING:
		if(wtk_str_equal(v,v_len,ji->v.str->data,ji->v.str->len))
		{
			ret=1;
		}
		break;
	case WTK_JSON_NUMBER:
		k=wtk_str_atoi(v,v_len);
		if(k==ji->v.number)
		{
			ret=1;
		}
		break;
	case WTK_JSON_ARRAY:
		ret=wtk_json_array_has_string_value(ji,v,v_len);
		break;
	case WTK_JSON_OBJECT:
		ji=wtk_json_obj_get(ji,v,v_len);
		if(ji)
		{
			ret=1;
		}
		break;
	}
end:
	wtk_jsonkv_reset(kv->inst_kv);
	wtk_heap_reset(kv->heap);
	return ret;
}
int wtk_lua_owl_item_print(lua_State *l)
{
	wtk_owl_item_t *item;
	int i;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	item=(wtk_owl_item_t*)lua_touserdata(l,i);
	wtk_owl_item_print(item);
end:
	//wtk_debug("get =================================\n");
	return 0;
}

int wtk_lua_owl_item_get_attr(lua_State *l)
{
	wtk_owl_item_t *oi;
	int i;
	wtk_string_t p,*pv;
	size_t len;
	int cnt=0;

	i=1;
	if(!lua_isuserdata(l,i))
	{
		wtk_debug("1 param must be kg\n");
		goto end;
	}
	oi=(wtk_owl_item_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("1 param must be kg\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	if(wtk_string_cmp_s(&(p),".k"))
	{
		lua_pushlstring(l,oi->k.str->data,oi->k.str->len);
		cnt=1;
	}if(wtk_string_cmp_s(&(p),".v"))
	{
		if(oi->nv>0)
		{
			lua_pushlstring(l,oi->v[0]->data,oi->v[0]->len);
			cnt=1;
		}else
		{
			cnt=0;
		}
	}
	else
	{
		pv=wtk_owl_item_find_attr_value(oi,p.data,p.len,-1);
		if(pv)
		{
			lua_pushlstring(l,pv->data,pv->len);
			cnt=1;
		}
	}
end:
	return cnt;
}

int wtk_lua_owl_class_get_item(lua_State *l)
{
	wtk_owl_class_t *cls;
	int i;
	wtk_string_t p;
	size_t len;
	int cnt=0;
	wtk_owl_item_t *oi;

	i=1;
	if(!lua_isuserdata(l,i))
	{
		wtk_debug("1 param must be kg\n");
		goto end;
	}
	cls=(wtk_owl_class_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isstring(l,i))
	{
		wtk_debug("1 param must be kg\n");
		goto end;
	}
	p.data=(char*)lua_tolstring(l,i,&len);
	p.len=len;
	oi=wtk_owl_item_find_item2(cls->prop,p.data,p.len);
	if(oi)
	{
		lua_pushlightuserdata(l,oi);
		cnt=1;
	}
end:
	return cnt;
}

void wtk_owlkv_link_lua(wtk_lua2_t *lua2)
{
	wtk_lua2_link_function(lua2,wtk_lua_owl_item_print,"wtk_owl_item_print");
	wtk_lua2_link_function(lua2,wtk_lua_owl_item_get_attr,"wtk_owl_item_get_attr");
	wtk_lua2_link_function(lua2,wtk_lua_owl_class_get_item,"wtk_owl_class_get_item");
}
