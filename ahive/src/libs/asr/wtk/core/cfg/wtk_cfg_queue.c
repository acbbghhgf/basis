#include "wtk/core/cfg/wtk_cfg_queue.h"
#include "wtk_local_cfg.h"

wtk_cfg_queue_t* wtk_cfg_queue_new_h(wtk_heap_t *heap)
{
	wtk_cfg_queue_t *c;

	c=(wtk_cfg_queue_t*)wtk_heap_malloc(heap,sizeof(*c));
	wtk_queue_init(&(c->queue));
	c->heap=heap;
	return c;
}

wtk_cfg_item_t* wtk_cfg_queue_find(wtk_cfg_queue_t *c,char *k,int bytes)
{
	wtk_cfg_item_t *i=0,*p;
	wtk_queue_node_t *n;

	for(n=c->queue.pop;n;n=n->next)
	{
		p=data_offset(n,wtk_cfg_item_t,n);
		if(wtk_string_cmp(p->key,k,bytes)==0)
		{
			i=p;
			break;
		}
	}
	return i;
}

void wtk_cfg_queue_remove(wtk_cfg_queue_t *c,wtk_cfg_item_t *item)
{
	wtk_queue_remove(&(c->queue),&(item->n));
}

int wtk_cfg_queue_add(wtk_cfg_queue_t *c,wtk_cfg_item_t *item)
{
	return wtk_queue_push(&(c->queue),&(item->n));
}

wtk_string_t* wtk_cfg_queue_dup_string(wtk_cfg_queue_t *cfg,char *v,int vbytes)
{
	wtk_string_t *sv;

	if(vbytes<=0)
	{
		v=0;
	}
	sv=(wtk_string_t*)wtk_heap_malloc(cfg->heap,sizeof(*sv));
	if(vbytes<=0)
	{
		sv->len=0;
		sv->data=0;
	}else
	{
		sv->len=vbytes;
		sv->data=(char*)wtk_heap_malloc(cfg->heap,vbytes+1);
		memcpy(sv->data,v,vbytes);
		sv->data[vbytes]=0;
	}
	return sv;
}

wtk_cfg_item_t* wtk_cfg_queue_add_string(wtk_cfg_queue_t *cfg,char *k,int kbytes,char *v,int vbytes)
{
	wtk_heap_t *h=cfg->heap;
	wtk_cfg_item_t *item;
	wtk_string_t *sv;

	sv=wtk_cfg_queue_dup_string(cfg,v,vbytes);
	item=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg,k,kbytes);
	if(wtk_str_equal_s(v,vbytes,"nil"))
	{
		if(item)
		{
			wtk_cfg_queue_remove(cfg,item);
		}
	}else
	{
		if(!item)
		{
			item=(wtk_cfg_item_t*)wtk_heap_malloc(h,sizeof(*item));
			item->key=wtk_cfg_queue_dup_string(cfg,k,kbytes);
			wtk_cfg_queue_add(cfg,item);
		}
		item->type=WTK_CFG_STRING;
		item->value.str=sv;
	}
	return item;
}

wtk_cfg_item_t* wtk_cfg_queue_add_lc(wtk_cfg_queue_t *cfg,char *k,int kbytes,wtk_local_cfg_t *lc)
{
	wtk_heap_t *h=cfg->heap;
	wtk_cfg_item_t *item;

	item=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg,k,kbytes);
	if(!item)
	{
		item=(wtk_cfg_item_t*)wtk_heap_malloc(h,sizeof(*item));
		item->key=wtk_cfg_queue_dup_string(cfg,k,kbytes);
		wtk_string_set(&(lc->name),item->key->data,item->key->len);
		wtk_cfg_queue_add(cfg,item);
	}
	item->type=WTK_CFG_LC;
	item->value.cfg=lc;
	return item;
}

wtk_cfg_item_t* wtk_cfg_queue_add_array(wtk_cfg_queue_t *cfg,char *k,int bytes,wtk_array_t *a)
{
	wtk_heap_t *h=cfg->heap;
	wtk_cfg_item_t *item;

	item=(wtk_cfg_item_t*)wtk_cfg_queue_find(cfg,k,bytes);
	if(!item)
	{
		item=(wtk_cfg_item_t*)wtk_heap_malloc(h,sizeof(*item));
		item->key=wtk_cfg_queue_dup_string(cfg,k,bytes);
		wtk_cfg_queue_add(cfg,item);
	}
	item->type=WTK_CFG_ARRAY;
	item->value.array=a;
	return item;
}

void wtk_cfg_item_print(wtk_cfg_item_t *i)
{
	wtk_string_t *name;
	wtk_string_t **ss;
	int j;

	printf("%*.*s=",i->key->len,i->key->len,i->key->data);
	switch(i->type)
	{
	case WTK_CFG_STRING:
		name=i->value.str;
		printf("%*.*s",name->len,name->len,name->data);
		break;
	case WTK_CFG_LC:
		//printf("%*.*s={",i->key->len,i->key->len,i->key->data);
		printf("{\n");
		wtk_local_cfg_print(i->value.cfg);
		printf("}");
		//printf("{...}");
		break;
	case WTK_CFG_ARRAY:
		ss=(wtk_string_t**)(i->value.array->slot);
		printf("[");
		for(j=0;j<i->value.array->nslot;++j)
		{
			name=ss[j];
			if(j>0){printf(",");}
			printf("%*.*s",name->len,name->len,name->data);
		}
		printf("]");
		break;
	default:
		break;
	}
	printf(";\n");
}

void wtk_cfg_queue_print(wtk_cfg_queue_t *c)
{
	wtk_cfg_item_t *p;
	wtk_queue_node_t *n;

	for(n=c->queue.pop;n;n=n->next)
	{
		p=data_offset(n,wtk_cfg_item_t,n);
		wtk_cfg_item_print(p);
	}
}

void wtk_cfg_queue_to_string(wtk_cfg_queue_t *c,wtk_strbuf_t *buf)
{
	wtk_cfg_item_t *i;
	wtk_queue_node_t *n;
	wtk_string_t *v;
	wtk_string_t **ss;
	int j;

	for(n=c->queue.pop;n;n=n->next)
	{
		i=data_offset(n,wtk_cfg_item_t,n);
		wtk_strbuf_push(buf,i->key->data,i->key->len);
		wtk_strbuf_push_s(buf,"=");
		switch(i->type)
		{
		case WTK_CFG_STRING:
			v=i->value.str;
            wtk_strbuf_push_s(buf,"\"");
			wtk_strbuf_push(buf,v->data,v->len);
            wtk_strbuf_push_s(buf,"\"");
			break;
		case WTK_CFG_LC:
			wtk_strbuf_push_s(buf,"{");
			wtk_local_cfg_value_to_string(i->value.cfg,buf);
			wtk_strbuf_push_s(buf,"}");
			//printf("{...}");
			break;
		case WTK_CFG_ARRAY:
			ss=(wtk_string_t**)(i->value.array->slot);
			wtk_strbuf_push_s(buf,"[");
			for(j=0;j<i->value.array->nslot;++j)
			{
				v=ss[j];
				if(j>0){wtk_strbuf_push_s(buf,",");}
				wtk_strbuf_push(buf,v->data,v->len);
			}
			wtk_strbuf_push_s(buf,"]");
			break;
		default:
			break;
		}
		wtk_strbuf_push_s(buf,";");
	}
}
