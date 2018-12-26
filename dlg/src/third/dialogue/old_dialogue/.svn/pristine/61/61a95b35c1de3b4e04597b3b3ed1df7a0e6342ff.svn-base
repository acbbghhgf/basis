#include "wtk_kgkv.h" 


wtk_kgkv_t* wtk_kgkv_new(wtk_kgkv_cfg_t *cfg,wtk_rbin2_t *rbin)
{
	wtk_kgkv_t *kv;

	kv=(wtk_kgkv_t*)wtk_malloc(sizeof(wtk_kgkv_t));
	kv->cfg=cfg;
	kv->hash=NULL;
	if(cfg->kv_fn)
	{
		if(rbin)
		{
			kv->fkv=wtk_fkv_new4(rbin,cfg->kv_fn,17003);
		}else
		{
			kv->fkv=wtk_fkv_new3(cfg->kv_fn);
		}
	}else
	{
		kv->fkv=NULL;
	}
	if(cfg->item_q.length>0)
	{
		wtk_queue_node_t *qn;
		wtk_kgkv_item_t *item;
		wtk_fkv_t *fkv;

		kv->hash=wtk_str_hash_new(cfg->item_q.length*2+1);
		for(qn=cfg->item_q.pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_kgkv_item_t,q_n);
			if(rbin)
			{
				fkv=wtk_fkv_new4(rbin,item->fn,17003);
			}else
			{
				fkv=wtk_fkv_new3(item->fn);
			}
			if(fkv)
			{
				wtk_str_hash_add(kv->hash,item->k->data,item->k->len,fkv);
			}
		}
	}
	return kv;
}

void wtk_kgkv_delete(wtk_kgkv_t *kv)
{
	if(kv->fkv)
	{
		wtk_fkv_delete(kv->fkv);
	}
	if(kv->hash)
	{
		wtk_str_hash_it_t it;
		hash_str_node_t *node;

		it=wtk_str_hash_iterator(kv->hash);
		while(1)
		{
			node=wtk_str_hash_it_next(&(it));
			if(!node){break;}
			wtk_fkv_delete((wtk_fkv_t*)(node->value));
		}
		wtk_str_hash_delete(kv->hash);
	}
	wtk_free(kv);
}

wtk_string_t wtk_kgkv_get(wtk_kgkv_t *kv,wtk_string_t *db_name,wtk_string_t *k)
{
	wtk_fkv_t *fkv;
	wtk_string_t v;

	wtk_string_set(&(v),0,0);
	if(!db_name)
	{
		fkv=kv->fkv;
	}else
	{
		fkv=(wtk_fkv_t*)wtk_str_hash_find(kv->hash,db_name->data,db_name->len);
	}
	if(!fkv){goto end;}
	v=wtk_fkv_get_str2(fkv,k->data,k->len);
end:
	return v;
}
