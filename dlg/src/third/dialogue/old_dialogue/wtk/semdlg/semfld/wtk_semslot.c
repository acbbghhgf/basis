#include "wtk_semslot.h"

wtk_semslot_item_t* wtk_semslot_item_new()
{
	wtk_semslot_item_t *si;

	si=(wtk_semslot_item_t*)wtk_malloc(sizeof(wtk_semslot_item_t));
	si->cfg=NULL;
	si->k=NULL;
	si->v=NULL;
	return si;
}

wtk_string_t* wtk_semslot_item_get_key(wtk_semslot_item_t *item)
{
	return item->cfg?&(item->cfg->name):item->k;
}

void wtk_semslot_item_delete(wtk_semslot_item_t *s)
{
	if(s->k)
	{
		wtk_string_delete(s->k);
	}
	if(s->v)
	{
		wtk_string_delete(s->v);
	}
	wtk_free(s);
}

int wtk_semslot_nslot(wtk_semslot_t *s)
{
	return s->slot_q.length;
}

void wtk_semslot_reset(wtk_semslot_t *s)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_t *item;

	if(s->slot_q.length==0){return;}
	while(1)
	{
		qn=wtk_queue_pop(&(s->slot_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_semslot_item_t,q_n);
		wtk_semslot_item_delete(item);
	}
}

wtk_semslot_t* wtk_semslot_new(wtk_semslot_cfg_t *cfg)
{
	wtk_semslot_t *slot;

	slot=(wtk_semslot_t*)wtk_malloc(sizeof(wtk_semslot_t));
	slot->cfg=cfg;
	wtk_queue_init(&(slot->slot_q));
	return slot;
}

void wtk_semslot_delete(wtk_semslot_t *s)
{
	wtk_semslot_reset(s);
	wtk_free(s);
}

wtk_semslot_item_t* wtk_semslot_get(wtk_semslot_t *s,char *k,int klen)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_t *item;
	wtk_string_t *v;

	for(qn=s->slot_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semslot_item_t,q_n);
		v=wtk_semslot_item_get_key(item);
		if(v && wtk_string_cmp(v,k,klen)==0)
		{
			return item;
		}
	}
	return NULL;
}

wtk_string_t* wtk_semslot_get2(wtk_semslot_t *s,char *k,int klen)
{
	wtk_semslot_item_t *item;

	item=wtk_semslot_get(s,k,klen);
	return item?item->v:NULL;
}

int wtk_semslot_is_full(wtk_semslot_t *s)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_cfg_t *item;
	wtk_semslot_item_t *si;

	for(qn=s->cfg->slot_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semslot_item_cfg_t,q_n);
		if(!item->can_be_nil)
		{
			si=wtk_semslot_get(s,item->name.data,item->name.len);
			if(!si)
			{
				return 0;
			}
		}
	}
	return 1;
}

void wtk_semslot_set(wtk_semslot_t *s,char *k,int klen,char *v,int vlen)
{
	wtk_semslot_item_t *si;

	//wtk_debug("[%.*s]=[%.*s]\n",klen,k,vlen,v);
	si=wtk_semslot_get(s,k,klen);
	if(!si)
	{
		si=wtk_semslot_item_new();
		si->cfg=wtk_semslot_cfg_get(s->cfg,k,klen);
		if(!si->cfg)
		{
			si->k=wtk_string_dup_data(k,klen);
		}
		wtk_queue_push(&(s->slot_q),&(si->q_n));
	}
	//wtk_debug("slot=%d\n",s->slot_q.length);
	if(si->v)
	{
		wtk_string_delete(si->v);
		si->v=NULL;
	}
	if(vlen>0)
	{
		si->v=wtk_string_dup_data(v,vlen);
	}
}

void wtk_semslot_del(wtk_semslot_t *s,char *k,int klen)
{
	wtk_semslot_item_t *si;

	si=wtk_semslot_get(s,k,klen);
	if(si)
	{
		wtk_queue_remove(&(s->slot_q),&(si->q_n));
	}
}

void wtk_semslot_print(wtk_semslot_t *s)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_t *item;
	wtk_string_t *v;

	for(qn=s->slot_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semslot_item_t,q_n);
		v=wtk_semslot_item_get_key(item);
		printf("[%.*s]=[%.*s]\n",v->len,v->data,item->v->len,item->v->data);
	}
}
