#include <ctype.h>
#include "wtk_kgc.h" 
int wtk_kgc_feed(wtk_kgc_t *kgc,wtk_string_t *v);
int wtk_kgc_compile_file(wtk_kgc_t *kg,char *fn);

wtk_kgc_t* wtk_kgc_new()
{
	wtk_kgc_t *kgc;

	kgc=(wtk_kgc_t*)wtk_malloc(sizeof(wtk_kgc_t));
	kgc->pwd=NULL;
	kgc->heap=wtk_heap_new(4096);
	kgc->buf=wtk_strbuf_new(256,1);
	kgc->strparser=wtk_string_parser2_new();
	kgc->rbin=NULL;
	wtk_kgc_reset(kgc);
	return kgc;
}

void wtk_kgc_delete(wtk_kgc_t *kgc)
{
	wtk_string_parser2_delete(kgc->strparser);
	wtk_strbuf_delete(kgc->buf);
	wtk_heap_delete(kgc->heap);
	wtk_free(kgc);
}

void wtk_kgc_set_state(wtk_kgc_t *kgc,wtk_kgc_state_t state)
{
	kgc->state=state;
	kgc->sub_state=-1;
}

void wtk_kgc_reset(wtk_kgc_t *kgc)
{
	wtk_heap_reset(kgc->heap);
	wtk_kgc_set_state(kgc,WTK_KGC_INIT);
	kgc->scope=WTK_KGC_SCOPE_INIT;
	kgc->attr=WTK_KGC_ATTR_INIT;
	kgc->cls=NULL;
	kgc->item=NULL;
	kgc->inst=NULL;
}

int wtk_kgc_feed_init(wtk_kgc_t *kgc,wtk_string_t *v)
{
enum
{
	WTK_KGC_INIT_INIT=-1,
	WTK_KGC_INIT_WAIT_END,
};
	wtk_strbuf_t *buf=kgc->buf;
	wtk_strkv_parser_t p;
	int ret=0;
	char c;

	switch(kgc->sub_state)
	{
	case WTK_KGC_INIT_INIT:
		if(v->len==1)
		{
			c=v->data[0];
			switch(c)
			{
			case '[':
				wtk_strbuf_reset(buf);
				kgc->sub_state=WTK_KGC_INIT_WAIT_END;
				break;
			case '#':
				wtk_kgc_set_state(kgc,WTK_KGC_COMMENT);
				break;
			}
		}
		break;
	case WTK_KGC_INIT_WAIT_END:
		if(v->len==1 && v->data[0]==']')
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			wtk_strkv_parser_init(&(p),buf->data,buf->pos);
			while(1)
			{
				ret=wtk_strkv_parser_next(&(p));
				if(ret!=0){break;}
				//wtk_debug("[%.*s]=[%.*s]\n",p.k.len,p.k.data,p.v.len,p.v.data);
				if(wtk_string_cmp_s(&(p.k),"class")==0)
				{
					kgc->cls=wtk_kg_find_class(kgc->kg,p.v.data,p.v.len,1);
					kgc->scope=WTK_KGC_SCOPE_CLASS;
				}else if(wtk_string_cmp_s(&(p.k),"item")==0)
				{
					if(!kgc->cls)
					{
						wtk_debug("class not found\n");
						goto end;
					}
					//wtk_debug("[%.*s]=[%.*s] not supported\n",p.k.len,p.k.data,p.v.len,p.v.data);
					kgc->item=wtk_kg_class_get_item(kgc->kg,kgc->cls,p.v.data,p.v.len,1);
					kgc->scope=WTK_KGC_SCOPE_ITEM;
				}else if(wtk_string_cmp_s(&(p.k),"attr")==0)
				{
					kgc->attr=WTK_KGC_ATTR_ATTR;
					wtk_kgc_set_state(kgc,WTK_KGC_VALUE);
				}else if(wtk_string_cmp_s(&(p.k),"fst")==0)
				{
					kgc->attr=WTK_KGC_ATTR_FST;
					wtk_kgc_set_state(kgc,WTK_KGC_VALUE);
				}else if(wtk_string_cmp_s(&(p.k),"next")==0)
				{
					kgc->attr=WTK_KGC_ATTR_NEXT;
					wtk_kgc_set_state(kgc,WTK_KGC_VALUE);
				}else if(wtk_string_cmp_s(&(p.k),"inst")==0)
				{
					kgc->inst=wtk_kg_get_inst(kgc->kg,p.v.data,p.v.len,1);
					kgc->scope=WTK_KGC_SCOPE_INST;
				}
				else if(wtk_string_cmp_s(&(p.k),"value")==0)
				{
					kgc->attr=WTK_KGC_ATTR_VALUE;
					wtk_kgc_set_state(kgc,WTK_KGC_VALUE);
				}
				else if(wtk_string_cmp_s(&(p.k),"virtual")==0)
				{
					if(!kgc->item)
					{
						wtk_debug("item not found \n");
						ret=-1;
						goto end;
					}
					kgc->item->_virtual=wtk_str_atoi(p.v.data,p.v.len);
				}else if(wtk_string_cmp_s(&(p.k),"use_last_best")==0)
				{
					if(!kgc->item)
					{
						wtk_debug("item not found \n");
						ret=-1;
						goto end;
					}
					kgc->item->use_last_best=wtk_str_atoi(p.v.data,p.v.len);
				}else if(wtk_string_cmp_s(&(p.k),"type")==0)
				{
					if(!kgc->item)
					{
						wtk_debug("item not found \n");
						ret=-1;
						goto end;
					}
					//wtk_debug("[%.*s]=[%.*s] not supported\n",p.k.len,p.k.data,p.v.len,p.v.data);
					if(wtk_string_cmp_s(&(p.v),"int")==0)
					{
						kgc->item->vtype=WTK_KG_ITEM_INT;
					}else if(wtk_string_cmp_s(&(p.v),"string")==0)
					{
						kgc->item->vtype=WTK_KG_ITEM_STR;
					}else if(wtk_string_cmp_s(&(p.v),"array")==0)
					{
						kgc->item->vtype=WTK_KG_ITEM_ARRAY;
					}
				}
				else
				{
					wtk_debug("[%.*s]=[%.*s] not supported\n",p.k.len,p.k.data,p.v.len,p.v.data);
					exit(0);
				}
			}
			kgc->sub_state=WTK_KGC_INIT_INIT;
			ret=0;
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	}
end:
	return ret;
}


wtk_string_t* wtk_string_parser2_get_var(wtk_kgc_t *kgc,char *k,int k_len)
{
	//wtk_debug("[%.*s]=%p\n",k_len,k,kgc->pwd);
	if(wtk_str_equal_s(k,k_len,"pwd"))
	{
		return kgc->pwd;
	}else
	{
		return NULL;
	}
}

int wtk_kgc_feed_comment(wtk_kgc_t *kgc,wtk_string_t *v)
{
enum
{
	WTK_KGC_COMMENT_INIT=-1,
	WTK_KGC_COMMENT_HINT,
	WTK_KGC_COMMENT_WAIT_END,
	WTK_KGC_COMMENT_INCLUDE,
};
	wtk_string_parser2_t *p=kgc->strparser;
	wtk_strbuf_t *buf=kgc->buf;
	int ret=0;

	switch(kgc->sub_state)
	{
	case WTK_KGC_COMMENT_INIT:
		if(v->len==1 && !isspace(v->data[0]))
		{
			wtk_strbuf_reset(buf);
			wtk_strbuf_push(buf,v->data,v->len);
			kgc->sub_state=WTK_KGC_COMMENT_HINT;
		}else
		{
			kgc->sub_state=WTK_KGC_COMMENT_WAIT_END;
		}
		break;
	case WTK_KGC_COMMENT_HINT:
		if(v->len==1 && isspace(v->data[0]))
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			if(wtk_str_equal_s(buf->data,buf->pos,"include"))
			{
				wtk_string_parser2_reset(p);
				wtk_string_parser2_set(p,kgc,(wtk_string_parser2_get_var_f)wtk_string_parser2_get_var);
				kgc->sub_state=WTK_KGC_COMMENT_INCLUDE;
			}else
			{
				kgc->sub_state=WTK_KGC_COMMENT_WAIT_END;
			}
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_KGC_COMMENT_WAIT_END:
		if(v->len==1 && v->data[0]=='\n')
		{
			wtk_kgc_set_state(kgc,WTK_KGC_INIT);
		}
		break;
	case WTK_KGC_COMMENT_INCLUDE:
		//wtk_debug("[%.*s]\n",v->len,v->data);
		ret=wtk_string_parse2(p,v);
		if(ret>0)
		{
			//wtk_debug("[%.*s]\n",p->buf->pos,p->buf->data);
			wtk_kgc_set_state(kgc,WTK_KGC_INIT);
			if(ret>1)
			{
				return wtk_kgc_feed(kgc,v);
			}else
			{
				wtk_strbuf_push_c(p->buf,0);
				return wtk_kgc_compile_file(kgc,p->buf->data);
			}
		}
		break;
	}
	return ret;
}



int wtk_kgc_update_value(wtk_kgc_t *kgc)
{
	wtk_strbuf_t *buf=kgc->buf;
	int ret=0;

	//.wtk_debug("[%.*s]\n",buf->pos,buf->data);
	switch(kgc->attr)
	{
	case WTK_KGC_ATTR_ATTR:
		if(!kgc->item)
		{
			wtk_debug("lex item not found\n");
			ret=-1;
			goto end;
		}
		ret=wtk_kg_item_set_attr(kgc->kg,kgc->item,buf->data,buf->pos);
		break;
	case WTK_KGC_ATTR_NEXT:
		if(!kgc->item)
		{
			wtk_debug("lex item not found\n");
			ret=-1;
			goto end;
		}
		ret=wtk_kg_item_set_next(kgc->kg,kgc->item,buf->data,buf->pos);
		break;
	case WTK_KGC_ATTR_VALUE:
		if(!kgc->inst)
		{
			wtk_debug("cls not found\n");
			ret=-1;
			goto end;
		}
		wtk_kg_set_inst_value(kgc->kg,kgc->inst,buf->data,buf->pos);
		//exit(0);
		break;
	case WTK_KGC_ATTR_FST:
		if(!kgc->item)
		{
			wtk_debug("lex item not found\n");
			ret=-1;
			goto end;
		}
		kgc->item->nlg_net=wtk_nlgnet_new2(kgc->kg->heap,buf->data,buf->pos);
		break;
	default:
		wtk_debug("[%.*s]\n",buf->pos,buf->data);
		exit(0);
		break;
	}
end:
	return ret;
}

int wtk_kgc_feed_value(wtk_kgc_t *kgc,wtk_string_t *v)
{
enum
{
	WTK_KGC_VALUE_INIT=-1,
	WTK_KGC_VALUE_LEFT1,
	WTK_KGC_VALUE_LEFT2,
	WTK_KGC_VALUE_VALUE,
	WTK_KGC_VALUE_RIGHT1,
	WTK_KGC_VALUE_RIGHT2,
};
	int ret=0;
	wtk_strbuf_t *buf=kgc->buf;

	//wtk_debug("[%.*s]\n",v->len,v->data);
	//exit(0);
	switch(kgc->sub_state)
	{
	case WTK_KGC_VALUE_INIT:
		if(v->len==1 && v->data[0]=='{')
		{
			kgc->sub_state=WTK_KGC_VALUE_LEFT1;
		}
		break;
	case WTK_KGC_VALUE_LEFT1:
		if(v->len==1 && v->data[0]=='{')
		{
			kgc->sub_state=WTK_KGC_VALUE_LEFT2;
		}
		break;
	case WTK_KGC_VALUE_LEFT2:
		if(v->len==1 && v->data[0]=='{')
		{
			kgc->sub_state=WTK_KGC_VALUE_VALUE;
			wtk_strbuf_reset(buf);
		}
		break;
	case WTK_KGC_VALUE_VALUE:
		if(v->len==1 && v->data[0]=='}')
		{
			kgc->sub_state=WTK_KGC_VALUE_RIGHT1;
		}else
		{
			wtk_strbuf_push(buf,v->data,v->len);
		}
		break;
	case WTK_KGC_VALUE_RIGHT1:
		if(v->len==1 && v->data[0]=='}')
		{
			kgc->sub_state=WTK_KGC_VALUE_RIGHT2;
		}else
		{
			wtk_strbuf_push_c(buf,'}');
			wtk_strbuf_push(buf,v->data,v->len);
			kgc->sub_state=WTK_KGC_VALUE_VALUE;
		}
		break;
	case WTK_KGC_VALUE_RIGHT2:
		if(v->len==1 && v->data[0]=='}')
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			ret=wtk_kgc_update_value(kgc);
			if(ret!=0){goto end;}
			wtk_kgc_set_state(kgc,WTK_KGC_INIT);
		}else
		{
			wtk_strbuf_push_s(buf,"}}");
			wtk_strbuf_push(buf,v->data,v->len);
			kgc->sub_state=WTK_KGC_VALUE_VALUE;
		}
		break;
	}
end:
	return ret;
}

int wtk_kgc_feed(wtk_kgc_t *kgc,wtk_string_t *v)
{
	int ret=-1;

	switch(kgc->state)
	{
	case WTK_KGC_INIT:
		ret=wtk_kgc_feed_init(kgc,v);
		break;
	case WTK_KGC_COMMENT:
		ret=wtk_kgc_feed_comment(kgc,v);
		break;
	case WTK_KGC_VALUE:
		ret=wtk_kgc_feed_value(kgc,v);
		break;
	}
	return ret;
}

int wtk_kgc_compile_string(wtk_kgc_t *kg,char *data,int len)
{
	int ret=-1;
	char *s,*e;
	int n;
	wtk_string_t v;

	s=data;
	e=s+len;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		wtk_string_set(&(v),s,n);
		ret=wtk_kgc_feed(kg,&(v));
		if(ret!=0)
		{
			goto end;
		}
		s+=n;
	}
	ret=0;
end:
	return ret;
}

int wtk_kgc_compile_file(wtk_kgc_t *kg,char *fn)
{
	wtk_heap_t *heap=kg->heap;
	int ret=-1;
	wtk_string_t *pwd;
	wtk_string_t v;
	char *data=NULL;
	int len;
	wtk_rbin2_item_t *ritem=NULL;

	v=wtk_dir_name2(fn,strlen(fn),'/');
	pwd=kg->pwd;
	kg->pwd=wtk_heap_dup_string(heap,v.data,v.len);
	if(kg->rbin)
	{
		ritem=wtk_rbin2_get3(kg->rbin,fn,strlen(fn),0);
		if(!ritem){goto end;}
		data=ritem->data->data;
		len=ritem->data->len;
	}else
	{
		data=file_read_buf(fn,&len);
	}
	if(!data){goto end;}
	ret=wtk_kgc_compile_string(kg,data,len);
	kg->pwd=pwd;
end:
	if(kg->rbin)
	{
		if(ritem)
		{
			wtk_rbin2_item_clean(ritem);
		}
	}else
	{
		if(data)
		{
			wtk_free(data);
		}
	}
	return ret;
}

wtk_kg_t* wtk_kgc_compile(wtk_kgc_t *kgc,char *fn)
{
	wtk_kg_t* kg;

	kg=wtk_kg_new();
	kgc->kg=kg;
	wtk_kgc_set_state(kgc,WTK_KGC_INIT);
	wtk_kgc_compile_file(kgc,fn);
	wtk_kgc_reset(kgc);
	return kg;
}

wtk_kg_t* wtk_kg_new_fn(char *fn,wtk_rbin2_t *rbin)
{
	wtk_kgc_t *kgc;
	wtk_kg_t *kg;

	kgc=wtk_kgc_new();
	kgc->rbin=rbin;
	kg=wtk_kgc_compile(kgc,fn);
	wtk_kgc_delete(kgc);
	return kg;
}
