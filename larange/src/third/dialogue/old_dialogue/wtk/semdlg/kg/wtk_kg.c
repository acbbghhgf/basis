#include "wtk_kg.h" 

wtk_kg_t* wtk_kg_new()
{
	wtk_kg_t *kg;

	kg=(wtk_kg_t*)wtk_malloc(sizeof(wtk_kg_t));
	kg->heap=wtk_heap_new(4096);
	kg->_class=NULL;
	wtk_queue3_init(&(kg->inst_q));
	return kg;
}

void wtk_kg_delete(wtk_kg_t *kg)
{
	wtk_heap_delete(kg->heap);
	wtk_free(kg);
}

wtk_kg_class_t* wtk_kg_new_class(wtk_kg_t *kg,char *nm,int nm_len)
{
	wtk_heap_t *heap=kg->heap;
	wtk_kg_class_t *cls;

	cls=(wtk_kg_class_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_class_t));
	cls->name=wtk_heap_dup_string(heap,nm,nm_len);
	wtk_queue_init(&(cls->item_q));
	cls->freeask=NULL;
	cls->freetalk=NULL;
	return cls;
}

wtk_kg_inst_t* wtk_kg_get_inst(wtk_kg_t *kg,char *nm,int nm_len,int insert)
{
	wtk_kg_inst_t *inst;
	wtk_heap_t *heap=kg->heap;
	wtk_queue_node_t *qn;

	for(qn=kg->inst_q.pop;qn;qn=qn->next)
	{
		inst=data_offset2(qn,wtk_kg_inst_t,q_n);
		if(wtk_string_cmp(inst->name,nm,nm_len)==0)
		{
			return inst;
		}
	}
	if(!insert){return NULL;}
	inst=(wtk_kg_inst_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_inst_t));
	inst->name=wtk_heap_dup_string(heap,nm,nm_len);
	wtk_queue3_init(&(inst->item_q));
	wtk_queue3_push(&(kg->inst_q),&(inst->q_n));
	return inst;
}

wtk_kg_item_t* wtk_kg_new_item(wtk_kg_t *kg,char *nm,int nm_len)
{
	wtk_kg_item_t *item;
	wtk_heap_t *heap=kg->heap;

	item=(wtk_kg_item_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_item_t));
	item->name=wtk_heap_dup_string(heap,nm,nm_len);
	item->nlg_net=NULL;
	item->attr=NULL;
	item->next=NULL;
	item->_virtual=0;
	item->use_last_best=0;
	item->vtype=WTK_KG_ITEM_STR;
	return item;
}

wtk_kg_item_t* wtk_kg_class_get_item(wtk_kg_t *kg,wtk_kg_class_t *cls,char *nm,int nm_len,int insert)
{
	wtk_queue_node_t *qn;
	wtk_kg_item_t *item;
	wtk_string_t *v;

	if(wtk_str_equal_s(nm,nm_len,"freeask"))
	{
		if(cls->freeask)
		{
			return cls->freeask;
		}
		if(!insert){return NULL;}
		item=wtk_kg_new_item(kg,nm,nm_len);
		cls->freeask=item;
	}else if(wtk_str_equal_s(nm,nm_len,"freetalk"))
	{
		if(cls->freetalk)
		{
			return cls->freetalk;
		}
		if(!insert){return NULL;}
		item=wtk_kg_new_item(kg,nm,nm_len);
		cls->freetalk=item;
	}else
	{
		for(qn=cls->item_q.pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_kg_item_t,q_n);
			v=item->name;
			if(wtk_string_cmp(v,nm,nm_len)==0)
			{
				return item;
			}
		}
		if(!insert){return NULL;}
		item=wtk_kg_new_item(kg,nm,nm_len);
		wtk_queue3_push(&(cls->item_q),&(item->q_n));
	}
	return item;
}

wtk_kg_class_t* wtk_kg_find_class(wtk_kg_t *kg,char *nm,int nm_len,int insert)
{
	wtk_kg_class_t *cls=NULL;

	if(kg->_class){return kg->_class;}
	if(insert==0){goto end;}
	cls=wtk_kg_new_class(kg,nm,nm_len);
	kg->_class=cls;
end:
	return cls;
}



void wtk_kg_str_to_array_split(wtk_larray_t *a,char *item,int len,int index)
{
	wtk_string_t *v;

	v=(wtk_string_t*)wtk_malloc(sizeof(wtk_string_t));
	wtk_string_set(v,item,len);
	wtk_larray_push2(a,&v);
}

//void wtk_kg_item_set_array_value(wtk_kg_t *kg,wtk_kg_item_t *item,char *data,int len)
//{
//	wtk_heap_t *heap=kg->heap;
//	wtk_larray_t *a;
//	wtk_strbuf_t *buf;
//	wtk_string_t **strs;
//	int i;
//
//	//wtk_debug("[%.*s]\n",len,data);
//	buf=wtk_strbuf_new(256,1);
//	a=wtk_larray_new(10,sizeof(void*));
//	wtk_str_split(data,len,'\n',a,(wtk_str_split_f)wtk_kg_str_to_array_split);
//	item->vtype=WTK_KG_ITEM_ARRAY;
//	item->v.strs=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*a->nslot);
//	strs=(wtk_string_t**)a->slot;
//	for(i=0;i<a->nslot;++i)
//	{
//		wtk_strbuf_reset(buf);
//		wtk_strbuf_push(buf,strs[i]->data,strs[i]->len);
//		wtk_free(strs[i]);
//		wtk_strbuf_strip(buf);
//		item->v.strs[i]=wtk_heap_dup_string(heap,buf->data,buf->pos);
//		//wtk_debug("v[%d]=[%.*s]\n",i,item->v.strs[i]->len,item->v.strs[i]->data);
//	}
//	wtk_larray_delete(a);
//	wtk_strbuf_delete(buf);
//	//exit(0);
//}



wtk_kg_item_attr_t* wtk_kg_item_attr_new(wtk_heap_t *heap)
{
	wtk_kg_item_attr_t *a;

	a=(wtk_kg_item_attr_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_item_attr_t));
	a->answer=NULL;
	a->ask=NULL;
	a->set=NULL;
	a->ask2=NULL;
	a->post_v=NULL;
	a->attr_q=NULL;
	a->confirm=NULL;
	a->check=NULL;
	a->args=NULL;
	return a;
}

wtk_kg_item_attr_kv_t* wtk_kg_item_attr_kv_new(wtk_heap_t *heap,wtk_string_t *k,wtk_string_t *v)
{
	wtk_kg_item_attr_kv_t *kv;

	kv=(wtk_kg_item_attr_kv_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_item_attr_kv_t));
	kv->k=wtk_heap_dup_string(heap,k->data,k->len);
	kv->v=wtk_heap_dup_string(heap,v->data,v->len);
	return kv;
}

int wtk_kg_item_set_attr(wtk_kg_t *kg,wtk_kg_item_t *ki,char *data,int len)
{
	wtk_queue_node_t *qn;
	wtk_cfg_file_t *cfg;
	wtk_heap_t *heap=kg->heap;
	int ret;
	wtk_cfg_item_t *item;
	wtk_kg_item_attr_kv_t *kv;

	//wtk_debug("set [%.*s] attr\n",ki->name->len,ki->name->data);
	//wtk_debug("[%.*s]\n",len,data);
	cfg=wtk_cfg_file_new();
	ret=wtk_cfg_file_feed(cfg,data,len);
	if(ret!=0){goto end;}
	//wtk_local_cfg_print(cfg->main);
	ki->attr=wtk_kg_item_attr_new(heap);
	for(qn=cfg->main->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		if(item->type!=WTK_CFG_STRING){continue;}
		if(wtk_string_cmp_s(item->key,"ask")==0)
		{
			ki->attr->ask=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"set")==0)
		{
			ki->attr->set=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"answer")==0)
		{
			ki->attr->answer=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"ask2")==0)
		{
			ki->attr->ask2=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"post_v")==0)
		{
			ki->attr->post_v=wtk_heap_dup_str2(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"confirm")==0)
		{
			ki->attr->confirm=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"check")==0)
		{
			ki->attr->check=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else if(wtk_string_cmp_s(item->key,"args")==0)
		{
			ki->attr->args=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
		}else
		{
			if(!ki->attr->attr_q)
			{
				ki->attr->attr_q=(wtk_queue3_t*)wtk_heap_malloc(heap,sizeof(wtk_queue3_t));
				wtk_queue3_init(ki->attr->attr_q);
			}
			kv=wtk_kg_item_attr_kv_new(heap,item->key,item->value.str);
			wtk_queue3_push(ki->attr->attr_q,&(kv->q_n));
		}
	}
end:
	wtk_cfg_file_delete(cfg);
	return ret;
}

wtk_string_t* wtk_kg_item_get_attr(wtk_kg_item_t *ki,char *p,int len)
{
	wtk_queue_node_t *qn;
	wtk_kg_item_attr_kv_t *kv;

	//wtk_debug("%p=[%.*s] ask=%p\n",ki,len,p,ki->attr->ask);
	//wtk_debug("[%.*s]=%p\n",len,p,ki->attr->args);
	if(wtk_str_equal_s(p,len,"ask"))
	{
		return ki->attr->ask;
	}else if(wtk_str_equal_s(p,len,"set"))
	{
		return ki->attr->set;
	}else if(wtk_str_equal_s(p,len,"answer"))
	{
		return ki->attr->answer;
	}else if(wtk_str_equal_s(p,len,"ask2"))
	{
		return ki->attr->ask2;
	}else if(wtk_str_equal_s(p,len,"args"))
	{
		return ki->attr->args;
	}
//	else if(wtk_str_equal_s(p,len,"post_v"))
//	{
//		return ki->attr->post_v;
//	}
	if(ki->attr->attr_q)
	{
		for(qn=ki->attr->attr_q->pop;qn;qn=qn->next)
		{
			kv=data_offset2(qn,wtk_kg_item_attr_kv_t,q_n);
			if(wtk_string_cmp(kv->k,p,len)==0)
			{
				return kv->k;
			}
		}
	}
	return NULL;
}

wtk_kg_item_next_item_t* wtk_kg_item_next_item_new(wtk_heap_t *heap)
{
	wtk_kg_item_next_item_t *item;

	item=(wtk_kg_item_next_item_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_item_next_item_t));
	item->_if=NULL;
	item->v.item=NULL;
	item->is_item=1;
	return item;
}

wtk_kg_item_next_t* wtk_kg_item_next_new(wtk_heap_t *heap)
{
	wtk_kg_item_next_t *n;

	n=(wtk_kg_item_next_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_item_next_t));
	wtk_queue3_init(&(n->next_q));
	return n;
}



int wtk_kg_item_set_next(wtk_kg_t *kg,wtk_kg_item_t *ki,char *data,int len)
{
	wtk_source_t src;
	wtk_strbuf_t *buf;
	int ret,eof;
	wtk_kg_item_next_item_t *ni;
	wtk_heap_t *heap=kg->heap;
	char *p;
	wtk_string_t v;

	buf=wtk_strbuf_new(256,1);
	wtk_source_init_str(&(src),data,len);
	ki->next=wtk_kg_item_next_new(heap);
	while(1)
	{
		ret=wtk_source_read_line2(&(src),buf,&eof);
		if(ret!=0){break;}
		wtk_strbuf_strip(buf);
		if(buf->pos>0)
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			ni=wtk_kg_item_next_item_new(heap);
			if(buf->data[0]=='/')
			{
				p=wtk_str_rchr(buf->data,buf->pos,'/');
				if(p)
				{
					//wtk_debug("[%.*s]\n",(int)(p-buf->data-1),buf->data+1);
					ni->_if=wtk_if_new(heap,buf->data+1,(int)(p-buf->data-1));
					//wtk_debug("[%.*s]\n",(int)(buf->data+buf->pos-p-1),p+1);
					//ni->v.name=wtk_heap_dup_string(heap,p+1,(int)(buf->data+buf->pos-p-1));
					wtk_string_set(&(v),p+1,(int)(buf->data+buf->pos-p-1));

					//exit(0);
				}else
				{
					wtk_string_set(&(v),buf->data,buf->pos);
					//ni->v.name=wtk_heap_dup_string(heap,buf->data,buf->pos);
				}
			}else
			{
				wtk_string_set(&(v),buf->data,buf->pos);
				//ni->name=wtk_heap_dup_string(heap,buf->data,buf->pos);
			}
			if(v.data[0]=='#')
			{
				ni->v.lua=wtk_heap_dup_str2(heap,v.data+1,v.len-1);
				ni->is_item=0;
			}else
			{
				ni->v.item=wtk_kg_class_get_item(kg,kg->_class,v.data,v.len,1);
				ni->is_item=1;
			}
			wtk_queue3_push(&(ki->next->next_q),&(ni->q_n));
		}
		if(eof)
		{
			break;
		}
	}
	wtk_strbuf_delete(buf);
	wtk_source_clean_str(&(src));
	//wtk_kg_item_print(ki);
	return 0;
}

wtk_kg_inst_value_t* wtk_kg_inst_value_new(wtk_heap_t *heap)
{
	wtk_kg_inst_value_t *v;

	v=(wtk_kg_inst_value_t*)wtk_heap_malloc(heap,sizeof(wtk_kg_inst_value_t));
	v->item=NULL;
	v->v.str=NULL;
	return v;
}

wtk_kg_inst_value_t* wtk_kg_inst_get_value_item(wtk_kg_inst_t *inst,char *k,int k_len)
{
	wtk_queue_node_t *qn;
	wtk_kg_inst_value_t *vi;

	for(qn=inst->item_q.pop;qn;qn=qn->next)
	{
		vi=data_offset2(qn,wtk_kg_inst_value_t,q_n);
		if(wtk_string_cmp(vi->item->name,k,k_len)==0)
		{
			return vi;
		}
	}
	return NULL;
}

void wtk_kg_set_inst_value_item_str(wtk_kg_inst_t *inst,char *k,int k_len,wtk_string_t *v)
{
	wtk_kg_inst_value_t *vi;

	vi=wtk_kg_inst_get_value_item(inst,k,k_len);
	if(vi)
	{
		vi->v.str=v;
	}
}

void wtk_kg_set_inst_value_item_number(wtk_kg_inst_t *inst,char *k,int k_len,int i)
{
	wtk_kg_inst_value_t *vi;

	vi=wtk_kg_inst_get_value_item(inst,k,k_len);
	if(vi)
	{
		vi->v.i=i;
	}
}


void wtk_kg_set_inst_value(wtk_kg_t *kg,wtk_kg_inst_t *inst,char *data,int len)
{
	wtk_cfg_file_t *cfg;
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;
	wtk_kg_item_t *ki;
	wtk_heap_t *heap=kg->heap;
	wtk_kg_inst_value_t *vi;

	//wtk_debug("[%.*s]\n",len,data);
	cfg=wtk_cfg_file_new();
	wtk_cfg_file_feed(cfg,data,len);
	//wtk_local_cfg_print(cfg->main);
	for(qn=cfg->main->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_cfg_item_t,n);
		ki=wtk_kg_class_get_item(kg,kg->_class,item->key->data,item->key->len,0);
		if(!ki)
		{
			continue;
		}
		vi=wtk_kg_inst_value_new(heap);
		vi->item=ki;
		wtk_queue3_push(&(inst->item_q),&(vi->q_n));
		switch(item->type)
		{
		case WTK_CFG_STRING:
			switch(ki->vtype)
			{
			case WTK_KG_ITEM_STR:
				vi->v.str=wtk_heap_dup_string(heap,item->value.str->data,item->value.str->len);
				break;
			case WTK_KG_ITEM_INT:
				vi->v.i=wtk_str_atoi(item->value.str->data,item->value.str->len);
				break;
			case WTK_KG_ITEM_ARRAY:
				wtk_debug("unmatch [%.*s] type\n",ki->name->len,ki->name->data);
				break;
			}
			break;
		case WTK_CFG_ARRAY:
			if(ki->vtype==WTK_KG_ITEM_ARRAY)
			{
				wtk_string_t **strs;
				int i;

				strs=(wtk_string_t**)(item->value.array->slot);
				vi->v.a.strs=(wtk_string_t**)wtk_heap_malloc(heap,sizeof(wtk_string_t*)*item->value.array->nslot);
				for(i=0;i<item->value.array->nslot;++i)
				{
					vi->v.a.strs[i]=wtk_heap_dup_string(heap,strs[i]->data,strs[i]->len);
				}
				vi->v.a.n=item->value.array->nslot;
			}else
			{
				wtk_debug("unmatch [%.*s] type\n",ki->name->len,ki->name->data);
			}
			break;
		case WTK_CFG_LC:
			wtk_debug("unmatch [%.*s] type\n",ki->name->len,ki->name->data);
			break;
		}
	}
	wtk_cfg_file_delete(cfg);
	//exit(0);
}

void wtk_kg_update(wtk_kg_t *kg)
{
//	wtk_queue_node_t *qn,*qn2;
//	wtk_kg_item_t *item;
//	wtk_kg_item_next_item_t *ni;
//
//	for(qn=kg->_class->item_q.pop;qn;qn=qn->next)
//	{
//		item=data_offset2(qn,wtk_kg_item_t,q_n);
//		//printf("%.*s\n",item->name->len,item->name->data);
//		if(item->next)
//		{
//			for(qn2=item->next->next_q.pop;qn2;qn2=qn2->next)
//			{
//				ni=data_offset2(qn2,wtk_kg_item_next_item_t,q_n);
//				ni->v.item=wtk_kg_class_get_item(kg,kg->_class,ni->v.name->data,ni->v.name->len,0);
//			}
//		}
//	}
}

void wtk_kg_item_next_print(wtk_kg_item_next_t *n)
{
	wtk_queue_node_t *qn;
	wtk_kg_item_next_item_t *item;

	for(qn=n->next_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_kg_item_next_item_t,q_n);
		if(item->_if)
		{
			printf("/");
			wtk_if_print(item->_if);
			printf("/");
		}
		if(item->is_item)
		{
			printf("%.*s\n",item->v.item->name->len,item->v.item->name->data);
		}else
		{
			printf("#%s\n",item->v.lua);
		}
	}
}

void wtk_kg_item_attr_print(wtk_kg_item_attr_t *a)
{
	printf("[attr]\n");
	printf("{{{\n");
	if(a->ask)
	{
		printf("ask=%.*s;\n",a->ask->len,a->ask->data);
	}
	if(a->set)
	{
		printf("set=%.*s;\n",a->set->len,a->set->data);
	}
	if(a->answer)
	{
		printf("answer=%.*s;\n",a->answer->len,a->answer->data);
	}
	printf("}}}\n");
}

void wtk_kg_item_print(wtk_kg_item_t *item)
{
	printf("[item=%.*s",item->name->len,item->name->data);
	if(item->_virtual)
	{
		printf(",virtual=1");
	}
	printf("]\n");
	if(item->attr)
	{
		wtk_kg_item_attr_print(item->attr);
	}
	if(item->next)
	{
		printf("[next]\n");
		printf("{{{\n");
		wtk_kg_item_next_print(item->next);
		printf("}}}\n");
	}
	printf("\n");
}

void wtk_kg_class_print(wtk_kg_class_t *cls)
{
	wtk_queue_node_t *qn;
	wtk_kg_item_t *item;

	printf("[class=%.*s]\n",cls->name->len,cls->name->data);
	for(qn=cls->item_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_kg_item_t,q_n);
		//printf("%.*s\n",item->name->len,item->name->data);
		wtk_kg_item_print(item);
	}
}

void wtk_kg_inst_print(wtk_kg_inst_t *inst)
{
	wtk_queue_node_t *qn;
	wtk_kg_inst_value_t *v;
	int i;

	printf("[inst=%.*s]\n",inst->name->len,inst->name->data);
	printf("[value]\n");
	printf("{{{\n");
	for(qn=inst->item_q.pop;qn;qn=qn->next)
	{
		v=data_offset2(qn,wtk_kg_inst_value_t,q_n);
		printf("%.*s=",v->item->name->len,v->item->name->data);
		switch(v->item->vtype)
		{
		case WTK_KG_ITEM_STR:
			printf("%.*s",v->v.str->len,v->v.str->data);
			break;
		case WTK_KG_ITEM_ARRAY:
			printf("[");
			for(i=0;i<v->v.a.n;++i)
			{
				if(i>0)
				{
					printf(",");
				}
				printf("\"%.*s\"",v->v.a.strs[i]->len,v->v.a.strs[i]->data);
			}
			printf("]");
			break;
		case WTK_KG_ITEM_INT:
			printf("%d",v->v.i);
			break;
		}
		printf(";\n");
	}
	printf("}}}\n");
}

void wtk_kg_print(wtk_kg_t *kg)
{
	wtk_queue_node_t *qn;
	wtk_kg_inst_t *inst;

	if(kg->_class)
	{
		wtk_kg_class_print(kg->_class);
		printf("\n");
	}
	for(qn=kg->inst_q.pop;qn;qn=qn->next)
	{
		inst=data_offset2(qn,wtk_kg_inst_t,q_n);
		wtk_kg_inst_print(inst);
	}
}


