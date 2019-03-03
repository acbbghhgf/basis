#include "wtk/core/cfg/wtk_local_cfg.h"

wtk_local_cfg_t *wtk_local_cfg_new_h(wtk_heap_t *h)
{
	wtk_local_cfg_t *c;

	c=(wtk_local_cfg_t*)wtk_heap_zalloc(h,sizeof(*c));
	c->cfg=wtk_cfg_queue_new_h(h);
	c->heap=h;
	return c;
}

wtk_local_cfg_t* wtk_local_cfg_find_lc(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	wtk_cfg_item_t *i;
	wtk_local_cfg_t *lc=0;

	if(!cfg){goto end;}
	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i && wtk_cfg_item_is_lc(i)){lc=i->value.cfg;goto end;}
	lc=wtk_local_cfg_find_lc(cfg->parent,d,bytes);
end:
	return lc;
}

wtk_cfg_item_t* wtk_local_cfg_find_local(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	return (wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
}

wtk_cfg_item_t* wtk_local_cfg_find(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	wtk_cfg_item_t *i;

	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i){goto end;}
	if(cfg->parent)
	{
		i=wtk_local_cfg_find(cfg->parent,d,bytes);
	}else
	{
		i=NULL;
	}
end:
	return i;
}


wtk_string_t* wtk_local_cfg_find_string(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	return wtk_local_cfg_find_string2(cfg,d,bytes,1);
}

wtk_string_t* wtk_local_cfg_find_string2(wtk_local_cfg_t *cfg,char *d,int bytes,int recursive)
{
	wtk_string_t* name=0;
	wtk_cfg_item_t *i;

	if(!cfg){goto end;}
	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i && wtk_cfg_item_is_string(i)){name=i->value.str;goto end;}
	if(recursive)
	{
		name=wtk_local_cfg_find_string(cfg->parent,d,bytes);
	}
end:
	return name;
}

wtk_array_t* wtk_local_cfg_find_array(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	wtk_array_t* a=0;
	wtk_cfg_item_t *i;

	if(!cfg){goto end;}
	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i && wtk_cfg_item_is_array(i)){a=i->value.array;goto end;}
	a=wtk_local_cfg_find_array(cfg->parent,d,bytes);
end:
	return a;
}

wtk_array_t* wtk_local_cfg_find_int_array(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	wtk_array_t* a=0,*b=0;
	wtk_cfg_item_t *i;
	wtk_string_t **v;
	int k,j;

	if(!cfg){goto end;}
	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i && wtk_cfg_item_is_array(i)){b=i->value.array;goto end;}
	b=wtk_local_cfg_find_array(cfg->parent,d,bytes);
	if(!b){goto end;}
end:
	if(b)
	{
		v=(wtk_string_t**)b->slot;
		a=wtk_array_new_h(cfg->heap,b->nslot,sizeof(int));
		for(k=0;k<b->nslot;++k)
		{
			j=atoi(v[k]->data);
			*((int*)wtk_array_push(a))=j;
		}
	}
	return a;
}

wtk_array_t* wtk_local_cfg_find_float_array(wtk_local_cfg_t *cfg,char *d,int bytes)
{
	wtk_array_t* a=0,*b=0;
	wtk_cfg_item_t *i;
	wtk_string_t **v;
	int k;
	float j;

	if(!cfg){goto end;}
	i=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg->cfg,d,bytes);
	if(i && wtk_cfg_item_is_array(i)){b=i->value.array;goto end;}
	b=wtk_local_cfg_find_array(cfg->parent,d,bytes);
	if(!b){goto end;}
end:
	if(b)
	{
		v=(wtk_string_t**)b->slot;
		a=wtk_array_new_h(cfg->heap,b->nslot,sizeof(float));
		for(k=0;k<b->nslot;++k)
		{
			j=atof(v[k]->data);
			*((float*)wtk_array_push(a))=j;
		}
	}
	return a;
}

wtk_cfg_item_t* wtk_local_cfg_add_item(wtk_local_cfg_t *lc,wtk_cfg_item_t *vi)
{
	wtk_cfg_item_t *item;

	switch(vi->type)
	{
	case WTK_CFG_STRING:
		item=wtk_cfg_queue_add_string(lc->cfg,vi->key->data,vi->key->len,vi->value.str->data,vi->value.str->len);
		break;
	case WTK_CFG_LC:
		item=wtk_cfg_queue_add_lc(lc->cfg,vi->key->data,vi->key->len,vi->value.cfg);
		break;
	case WTK_CFG_ARRAY:
		item=wtk_cfg_queue_add_array(lc->cfg,vi->key->data,vi->key->len,vi->value.array);
		break;
	default:
		item=NULL;
		break;
	}
	return item;
}

wtk_cfg_item_t* wtk_local_cfg_find_item(wtk_local_cfg_t *lc,wtk_cfg_item_t *vi)
{
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;

	for(qn=lc->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		if(item->type==vi->type && wtk_string_cmp(vi->key,item->key->data,item->key->len)==0)
		{
			return item;
		}
	}
	return NULL;
}

void wtk_local_cfg_update(wtk_local_cfg_t *lc,wtk_local_cfg_t *custom)
{
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;
	wtk_cfg_item_t *vi;

	for(qn=custom->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		vi=wtk_local_cfg_find_item(lc,item);
		if(vi)
		{
			//wtk_debug("set...\n");
			switch(item->type)
			{
			case WTK_CFG_STRING:
				vi->value.str=item->value.str;
				break;
			case WTK_CFG_LC:
				wtk_local_cfg_update(vi->value.cfg,item->value.cfg);
				break;
			case WTK_CFG_ARRAY:
				vi->value.array=item->value.array;
				break;
			}
		}else
		{
			wtk_local_cfg_add_item(lc,item);
		}
	}
	//exit(0);
}

/**
 * @param last_field is set when called,if section is "httpd:nk:port",last_field is set to "port";
 * @return NULL if not found;
 */
wtk_local_cfg_t* wtk_local_cfg_find_section(wtk_local_cfg_t *lc,char *section,int section_bytes,wtk_string_t *last_field)
{
	char *ks,*ke,*ls;
	int len;

	ls=ks=section;
	ke=ks+section_bytes;
	while(ks<ke)
	{
		if(*ks==':')
		{
			len=ks-ls;
			lc=wtk_local_cfg_find_lc(lc,ls,len);
			//wtk_debug("lc:%*.*s=%p\n",len,len,ls,lc);
			ls=ks+1;
			//wtk_local_cfg_print(lc);
		}
		++ks;
	}
	if(!lc){goto end;}
	len=ke-ls;
	wtk_string_set(last_field,ls,len);
end:
	//wtk_debug("%*.*s=%s\n",n->key.len,n->key.len,n->key.data,v);
	return lc;
}

wtk_local_cfg_t* wtk_local_cfg_find_section_lc(wtk_local_cfg_t* lc,char *section,int section_bytes)
{
	wtk_string_t last_field;

	lc=wtk_local_cfg_find_section(lc,section,section_bytes,&last_field);
	if(!lc){goto end;}
	lc=wtk_local_cfg_find_lc(lc,last_field.data,last_field.len);
end:
	return lc;
}


int wtk_local_cfg_update_hash(wtk_local_cfg_t* lc,wtk_arg_item_t *n,int show)
{
	wtk_string_t last_field;
	char *v;
	wtk_string_t *xv;

	if(n->v.len<=0){return 0;}
	v=(char*)n->v.data;
	lc=wtk_local_cfg_find_section(lc,n->k.data,n->k.len,&last_field);
	if(!lc){goto end;}
	xv=wtk_local_cfg_find_string(lc,last_field.data,last_field.len);
	if(xv)
	{
		wtk_string_set(xv,v,strlen(v));
		if(show)
		{
			printf("[cmd] update %.*s=%s\n",n->k.len,n->k.data,v);
		}
	}else
	{
		wtk_cfg_queue_add_string(lc->cfg,last_field.data,last_field.len,v,strlen(v));
		if(show)
		{
			printf("[cmd] set %.*s=%s\n",n->k.len,n->k.data,v);
		}
	}
end:
	//wtk_debug("%*.*s=%s\n",n->key.len,n->key.len,n->key.data,v);
	return 0;
}

int wtk_local_cfg_hook_update(void **hook,wtk_arg_item_t *item)
{
	return wtk_local_cfg_update_hash((wtk_local_cfg_t*)hook[0],item,*(int*)hook[1]);
}

void wtk_local_cfg_update_arg(wtk_local_cfg_t *lc,wtk_arg_t *arg,int show)
{
	void *hook[2]={lc,&show};

	if(show)
	{
		printf("================ update ===============\n");
	}
	//wtk_str_hash_walk(arg->hash,(wtk_walk_handler_t)wtk_local_cfg_hook_update,hook);
	wtk_queue_walk(&(arg->queue),offsetof(wtk_arg_item_t,q_n),(wtk_walk_handler_t)wtk_local_cfg_hook_update,hook);
	if(show)
	{
		printf("=======================================\n\n");
	}
}


void wtk_local_cfg_print(wtk_local_cfg_t *lc)
{
	//printf("################  %*.*s ########################\n",lc->name.len,lc->name.len,lc->name.data);
	wtk_cfg_queue_print(lc->cfg);
	//printf("################################################\n");
}

void wtk_local_cfg_value_to_string(wtk_local_cfg_t *lc,wtk_strbuf_t *buf)
{
	wtk_cfg_queue_to_string(lc->cfg,buf);
}
