#include "wtk_model.h" 

wtk_model_t* wtk_model_new(int nslot)
{
	wtk_model_t *m;

	m=(wtk_model_t*)wtk_malloc(sizeof(wtk_model_t));
	m->hash=wtk_str_hash_new(nslot);
	m->data_ths=NULL;
	m->get_data_f=NULL;
	m->touch_f=NULL;
	m->touch_ths=NULL;
	return m;
}

void wtk_model_delete(wtk_model_t *m)
{
	wtk_str_hash_delete(m->hash);
	wtk_free(m);
}

void wtk_model_set_data_get(wtk_model_t *m,void *ths,wtk_model_get_data_f get_data)
{
	m->data_ths=ths;
	m->get_data_f=get_data;
}

void wtk_model_set_touch(wtk_model_t *m,void *ths,wtk_model_touch_f touch)
{
	m->touch_ths=ths;
	m->touch_f=touch;
}

void* wtk_model_get_data(wtk_model_t *m,char *k,int k_bytes)
{
	return m->get_data_f(m->data_ths,k,k_bytes);
}

wtk_model_item_t* wtk_model_get_item(wtk_model_t *m,char *k,int k_bytes)
{
	hash_str_node_t *node;
	wtk_heap_t *heap=m->hash->heap;
	wtk_model_item_t *item;

	node=wtk_str_hash_find_node(m->hash,k,k_bytes,NULL);
	if(!node)
	{
		item=(wtk_model_item_t*)wtk_heap_malloc(heap,sizeof(wtk_model_item_t));
		item->k=wtk_heap_dup_string(heap,k,k_bytes);
		item->v.type=WTK_MODEL_V_NIL;
		item->v.v.i=0;
		wtk_queue2_init(&(item->listener_q));
		wtk_str_hash_add(m->hash,item->k->data,item->k->len,item);
	}else
	{
		item=node->value;
	}
	return item;
}

void wtk_model_add_listener(wtk_model_t *m,char *k,int k_bytes,void *ths,wtk_model_notify_f notify)
{
	wtk_model_item_t* item;

	item=wtk_model_get_item(m,k,k_bytes);
	wtk_model_add_listener2(m,item,ths,notify);
}

void wtk_model_add_listener2(wtk_model_t *m,wtk_model_item_t *item,void *ths,wtk_model_notify_f notify)
{
	wtk_heap_t *heap=m->hash->heap;
	wtk_model_listener_item_t *li;

	li=(wtk_model_listener_item_t*)wtk_heap_malloc(heap,sizeof(wtk_model_listener_item_t));
	li->notify=notify;
	li->ths=ths;
	wtk_queue2_push(&(item->listener_q),&(li->q_n));
}


void wtk_model_item_set(wtk_model_item_t *item,wtk_model_item_v_t *new_value)
{
	wtk_queue_node_t *qn;
	wtk_model_listener_item_t *li;
	wtk_model_item_v_t *old_value;

	if(item->v.type==WTK_MODEL_V_NIL)
	{
		old_value=NULL;
	}else
	{
		old_value=&(item->v);
	}
	for(qn=item->listener_q.pop;qn;qn=qn->next)
	{
		li=data_offset2(qn,wtk_model_listener_item_t,q_n);
		li->notify(li->ths,old_value,new_value);
	}
	item->v=*new_value;
}

void wtk_model_item_set_i(wtk_model_item_t *item,int i)
{
	wtk_model_item_v_t v = {0};

	if(item->v.v.i==i)
	{
		return;
	}
	v.type=WTK_MODEL_V_I;
	v.v.i=i;
	wtk_model_item_set(item,&v);
}

void wtk_model_item_set_f(wtk_model_item_t *item,float f)
{
	wtk_model_item_v_t v = {0};

	if(item->v.v.f==f)
	{
		return;
	}
	v.type=WTK_MODEL_V_F;
	v.v.f=f;
	wtk_model_item_set(item,&v);
}

void wtk_model_item_set_p(wtk_model_item_t *item,void *p)
{
	wtk_model_item_v_t v = {0};

	v.type=WTK_MODEL_V_I;
	v.v.p=p;
	wtk_model_item_set(item,&v);
}


wtk_model_item_t* wtk_model_set_i(wtk_model_t *m,char *k,int k_bytes,int i)
{
	wtk_model_item_t *item;
	wtk_model_item_v_t v;

	v.type=WTK_MODEL_V_I;
	v.v.i=i;
	item=wtk_model_get_item(m,k,k_bytes);
	wtk_model_item_set(item,&v);
	return item;
}

wtk_model_item_t* wtk_model_set_f(wtk_model_t *m,char *k,int k_bytes,float f)
{
	wtk_model_item_t *item;
	wtk_model_item_v_t v;

	v.type=WTK_MODEL_V_F;
	v.v.f=f;
	item=wtk_model_get_item(m,k,k_bytes);
	wtk_model_item_set(item,&v);
	return item;
}

wtk_model_item_t* wtk_model_set_p(wtk_model_t *m,char *k,int k_bytes,void *p)
{
	wtk_model_item_t *item;
	wtk_model_item_v_t v;

	v.type=WTK_MODEL_V_P;
	v.v.p=p;
	item=wtk_model_get_item(m,k,k_bytes);
	wtk_model_item_set(item,&v);
	return item;
}


