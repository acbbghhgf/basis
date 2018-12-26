#include "wtk_owlkv_cfg.h" 

int wtk_owlkv_cfg_init(wtk_owlkv_cfg_t *cfg)
{
	cfg->hash=NULL;
	cfg->class_dn=NULL;
	cfg->inst_dn=NULL;
	cfg->owl_fn=NULL;
	cfg->class_lex_fn=NULL;
	cfg->net_class=NULL;
	cfg->lex=NULL;
	cfg->nlg_fn=NULL;
	cfg->rel_lex_fn=NULL;
	cfg->net_rel=NULL;
	cfg->use_random=0;
	return 0;
}

int wtk_owlkv_cfg_clean(wtk_owlkv_cfg_t *cfg)
{
	if(cfg->hash)
	{
		wtk_str_hash_it_t it;
		hash_str_node_t *node;
		wtk_lex_net_t *net;

		it=wtk_str_hash_iterator(cfg->hash);
		while(1)
		{
			node=wtk_str_hash_it_next(&(it));
			if(!node){break;}
			net=(wtk_lex_net_t*)node->value;
			wtk_lex_net_delete(net);
		}
		wtk_str_hash_delete(cfg->hash);
	}
	if(cfg->net_class)
	{
		wtk_lex_net_delete(cfg->net_class);
	}
	if(cfg->net_rel)
	{
		wtk_lex_net_delete(cfg->net_rel);
	}
	return 0;
}

int wtk_owlkv_cfg_update_local(wtk_owlkv_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	//wtk_local_cfg_print(lc);
	wtk_local_cfg_update_cfg_str(lc,cfg,class_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,rel_lex_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,class_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,inst_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,owl_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,nlg_fn,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_random,v);
	cfg->lex=wtk_local_cfg_find_lc_s(lc,"lex");
	return 0;
}

int wtk_owlkv_cfg_update(wtk_owlkv_cfg_t *cfg)
{
	return 0;
}

int wtk_owlkv_cfg_update_lex(wtk_owlkv_cfg_t *cfg,wtk_lexc_t *lex)
{
	int ret=-1;

	if(cfg->class_lex_fn)
	{
		cfg->net_class=wtk_lexc_compile_file2(lex,cfg->class_lex_fn);
		if(!cfg->net_class)
		{
			wtk_debug("compile %s failed.\n",cfg->class_lex_fn);
			goto end;
		}
	}
	if(cfg->rel_lex_fn)
	{
		cfg->net_rel=wtk_lexc_compile_file2(lex,cfg->rel_lex_fn);
		if(!cfg->net_rel)
		{
			wtk_debug("compile %s failed.\n",cfg->rel_lex_fn);
			goto end;
		}
	}
	if(cfg->lex)
	{
		wtk_queue_node_t *qn;
		wtk_cfg_item_t *item;
		wtk_lex_net_t *net;

		cfg->hash=wtk_str_hash_new(cfg->lex->cfg->queue.length*2+2);
		for(qn=cfg->lex->cfg->queue.pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_cfg_item_t,n);
			if(item->type==WTK_CFG_STRING)
			{
				net=wtk_lexc_compile_file2(lex,item->value.str->data);
				if(!net)
				{
					wtk_debug("compile [%.*s]=[%.*s] failed\n",item->key->len,item->key->data,item->value.str->len,item->value.str->data);
					ret=-1;
					goto end;
				}
				wtk_str_hash_add(cfg->hash,item->key->data,item->key->len,net);
			}
		}
	}
	ret=0;
end:
	//exit(0);
	return ret;
}
