#include "wtk_kgkv_cfg.h" 

int wtk_kgkv_cfg_init(wtk_kgkv_cfg_t *cfg)
{
	cfg->kv_fn=NULL;
	wtk_queue_init(&(cfg->item_q));
	return 0;
}

int wtk_kgkv_cfg_clean(wtk_kgkv_cfg_t *cfg)
{
	wtk_queue_node_t *qn;
	wtk_kgkv_item_t *item;

	while(1)
	{
		qn=wtk_queue_pop(&(cfg->item_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_kgkv_item_t,q_n);
		wtk_free(item);
	}
	return 0;
}

void wtk_kgkv_cfg_update_kv(wtk_kgkv_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;
	wtk_kgkv_item_t *kv;

	//wtk_local_cfg_print(lc);
	for(qn=lc->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		if(item->type!=WTK_CFG_STRING){continue;}
		kv=(wtk_kgkv_item_t*)wtk_malloc(sizeof(wtk_kgkv_item_t));
		kv->k=item->key;
		kv->fn=item->value.str->data;
		wtk_queue_push(&(cfg->item_q),&(kv->q_n));
	}
	//exit(0);
}

int wtk_kgkv_cfg_update_local(wtk_kgkv_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_string_t *v;
	wtk_local_cfg_t *lc;

	lc=main;
	wtk_local_cfg_update_cfg_str(lc,cfg,kv_fn,v);
	lc=wtk_local_cfg_find_lc_s(main,"kv");
	if(lc)
	{
		wtk_kgkv_cfg_update_kv(cfg,lc);
	}
	return 0;
}

int wtk_kgkv_cfg_update(wtk_kgkv_cfg_t *cfg)
{
	return 0;
}

int wtk_kgkv_cfg_update2(wtk_kgkv_cfg_t *cfg,wtk_source_loader_t *sl)
{
	return 0;
}
