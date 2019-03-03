#include "wtk_semslot_cfg.h" 

int wtk_semslot_cfg_init(wtk_semslot_cfg_t *cfg)
{
	wtk_queue_init(&(cfg->slot_q));
	return 0;
}

int wtk_semslot_cfg_clean(wtk_semslot_cfg_t *cfg)
{
	return 0;
}

wtk_semslot_item_cfg_t* wtk_semslot_item_cfg_new(wtk_heap_t *heap)
{
	wtk_semslot_item_cfg_t *s;

	s=(wtk_semslot_item_cfg_t*)wtk_heap_malloc(heap,sizeof(wtk_semslot_item_cfg_t));
	s->can_be_nil=0;
	wtk_string_set(&(s->name),0,0);
	wtk_string_set(&(s->ename),0,0);
	wtk_string_set(&(s->ask),0,0);
	return s;
}

void wtk_semslot_item_update_local(wtk_semslot_item_cfg_t *item,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,item,name,v);
	wtk_local_cfg_update_cfg_string_v(lc,item,ename,v);
	wtk_local_cfg_update_cfg_string_v(lc,item,ask,v);
	wtk_local_cfg_update_cfg_b(lc,item,can_be_nil,v);
	if(item->ename.len==0)
	{
		item->ename=lc->name;
	}
}

int wtk_semslot_cfg_update_local(wtk_semslot_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;
	wtk_semslot_item_cfg_t *si;

	for(qn=lc->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		if(item->type==WTK_CFG_LC)
		{
			si=wtk_semslot_item_cfg_new(lc->heap);
			wtk_semslot_item_update_local(si,item->value.cfg);
			wtk_queue_push(&(cfg->slot_q),&(si->q_n));
		}
	}
	return 0;
}

int wtk_semslot_cfg_update(wtk_semslot_cfg_t *cfg)
{
	return 0;
}

wtk_semslot_item_cfg_t* wtk_semslot_cfg_get(wtk_semslot_cfg_t *cfg,char *k,int klen)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_cfg_t *item;

	for(qn=cfg->slot_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semslot_item_cfg_t,q_n);
		if(wtk_string_cmp(&(item->name),k,klen)==0)
		{
			return item;
		}
	}
	return NULL;
}
