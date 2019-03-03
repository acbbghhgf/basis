#include <stdlib.h>
#include "wtk_semfld.h" 
#include "wtk/semdlg/wtk_semdlg.h"
int wtk_semfld_process2(wtk_semfld_t *fld,char *data,int bytes);
int wtk_semfld_process_fst_emit(wtk_semfld_t *fld,wtk_nlg_item_t *item);

wtk_semfld_t* wtk_semfld_new(wtk_semfld_cfg_t *cfg,wtk_semdlg_t *dlg)
{
	wtk_semfld_t *fld;

	fld=(wtk_semfld_t*)wtk_malloc(sizeof(wtk_semfld_t));
	fld->cfg=cfg;
	fld->dlg=dlg;
	fld->fi=wtk_semfi_new(&(cfg->semfi),fld);
	fld->fst=NULL;
	fld->fst2=NULL;
	if(cfg->fstfn)
	{
		if(cfg->use_nlg2)
		{
		}else
		{
			if(dlg->cfg->use_rbin)
			{
				fld->fst=wtk_nlgfst_new(dlg->cfg->rbin,cfg->nlgfn,cfg->fstfn);
			}else
			{
				fld->fst=wtk_nlgfst_new(NULL,cfg->nlgfn,cfg->fstfn);
			}
			wtk_nlgfst_set_emit(fld->fst,fld,(wtk_nlgfst_emit_f)wtk_semfld_process_fst_emit);
		}
	}
	if(cfg->use_kg)
	{
		if(dlg->cfg->use_rbin)
		{
			fld->kg=wtk_kgr_new(&(cfg->kg),fld,dlg->cfg->rbin);
		}else
		{
			fld->kg=wtk_kgr_new(&(cfg->kg),fld,NULL);
		}
	}else
	{
		fld->kg=NULL;
	}
	fld->slot=wtk_semslot_new(&(cfg->slot));
	fld->ask_slot=wtk_strbuf_new(64,1);
	fld->ask_slot_value=wtk_strbuf_new(64,1);
	fld->ask_deny_lua_func=wtk_strbuf_new(64,1);
	fld->ask_deny_lua_str=wtk_strbuf_new(256,1);
	fld->is_finish=0;
	if(cfg->luafn)
	{
		wtk_lua2_load_file2(dlg->lua,cfg->luafn);
	}
	fld->remember=0;
	fld->ntry=0;
	wtk_semfld_reset(fld);
	if(cfg->lua_env_init && cfg->dat_dn)
	{
		wtk_lua2_arg_t arg[2];

		arg[0].type=WTK_LUA2_THS;
		arg[0].v.ths=dlg;
		arg[1].type=WTK_LUA2_STRING;
		arg[1].v.str.data=cfg->dat_dn;
		arg[1].v.str.len=strlen(cfg->dat_dn);
		wtk_lua2_process_arg(fld->dlg->lua,cfg->lua_env_init,fld->dlg->buf,arg+0,arg+1,NULL);
	}
	return fld;
}

void wtk_semfld_delete(wtk_semfld_t *fld)
{
	if(fld->kg)
	{
		wtk_kgr_delete(fld->kg);
	}
	if(fld->fst)
	{
		wtk_nlgfst_delete(fld->fst);
	}
	wtk_strbuf_delete(fld->ask_deny_lua_func);
	wtk_strbuf_delete(fld->ask_deny_lua_str);
	wtk_strbuf_delete(fld->ask_slot);
	wtk_strbuf_delete(fld->ask_slot_value);
	wtk_semslot_delete(fld->slot);
	wtk_semfi_delete(fld->fi);
	wtk_free(fld);
}

void wtk_semfld_update_env(wtk_semfld_t *fld)
{
	if(fld->kg)
	{
		wtk_kgr_update_env(fld->kg);
	}
}

void wtk_semfld_reset(wtk_semfld_t *fld)
{
	fld->want_add_hist=1;
	//wtk_debug("reset fld=%p[%.*s] fini=%d\n",fld,fld->cfg->name.len,fld->cfg->name.data,fld->is_finish);
	//wtk_debug("ask=[%.*s]\n",fld->ask_slot->pos,fld->ask_slot->data);
	wtk_semfi_reset(fld->fi);
	fld->output=NULL;
	if(fld->is_finish)
	{
		//wtk_debug("=======> reset and finish [%.*s]\n",fld->cfg->name.len,fld->cfg->name.data);
		if(fld->fst)
		{
			//wtk_debug("reset fst\n");
			wtk_nlgfst_reset(fld->fst);
		}
		fld->ntry=0;
		wtk_strbuf_reset(fld->ask_slot);
		wtk_strbuf_reset(fld->ask_slot_value);
		wtk_strbuf_reset(fld->ask_deny_lua_func);
		wtk_strbuf_reset(fld->ask_deny_lua_str);
		wtk_semslot_reset(fld->slot);
		fld->is_finish=0;
	}
}

wtk_act_t* wtk_semfld_get_input_act(wtk_semfld_t *fld)
{
	return &(fld->fi->act);
}

wtk_string_t wtk_semfld_get_ask_slot(wtk_semfld_t *fld)
{
	wtk_string_t v;

	wtk_string_set(&(v),fld->ask_slot->data,fld->ask_slot->pos);
	return v;
}


wtk_string_t wtk_semfld_get_ask_slot_value(wtk_semfld_t *fld)
{
	wtk_string_t v;

	wtk_string_set(&(v),fld->ask_slot_value->data,fld->ask_slot_value->pos);
	return v;
}


void wtk_semfld_set_ask_slot(wtk_semfld_t *fld,char *data,int bytes,char *v,int v_bytes)
{
	wtk_strbuf_reset(fld->ask_slot);
	if(bytes>0)
	{
		wtk_strbuf_push(fld->ask_slot,data,bytes);
	}
	wtk_strbuf_reset(fld->ask_slot_value);
	if(v && v_bytes>0)
	{
		wtk_strbuf_push(fld->ask_slot_value,v,v_bytes);
	}
	wtk_strbuf_reset(fld->ask_deny_lua_func);
	wtk_strbuf_reset(fld->ask_deny_lua_str);
}

void wtk_semfld_set_ask_deny_func(wtk_semfld_t *fld,char *deny_func,int deny_func_bytes,char *deny_str,int deny_str_bytes)
{
	wtk_strbuf_reset(fld->ask_deny_lua_func);
	wtk_strbuf_reset(fld->ask_deny_lua_str);
	wtk_strbuf_push(fld->ask_deny_lua_func,deny_func,deny_func_bytes);
	wtk_strbuf_push(fld->ask_deny_lua_str,deny_str,deny_str_bytes);
}

void wtk_semfld_set_end(wtk_semfld_t *fld)
{
	fld->is_finish=1;
}

int wtk_semfld_can_be_end(wtk_semfld_t *fld)
{
	//wtk_debug("ask_slot=%d\n",fld->ask_slot->pos);
	if(fld->ask_slot->pos>0)
	{
		return 0;
	}
	//wtk_debug("fst=%p\n",fld->fst);
	if(fld->fst)
	{
		return  wtk_nglfst_can_be_end(fld->fst);
	}
	if(fld->kg)
	{
		return wtk_kgr_can_be_end(fld->kg);
	}
	return wtk_semslot_is_full(fld->slot);
}

void wtk_semfld_touch_end(wtk_semfld_t *fld)
{
	//wtk_debug("================= touch ======================>\n");
	if(wtk_semfld_can_be_end(fld))
	{
		wtk_semfld_set_end(fld);
		wtk_semfld_reset(fld);
	}
}



void wtk_semfld_ext_data_set_str(wtk_semfld_ext_data_t *ext,wtk_string_t *str)
{
	ext->is_str=1;
	ext->v.str=str;
}

void wtk_semfld_ext_data_set_json(wtk_semfld_ext_data_t *ext,wtk_json_item_t *json)
{
	ext->is_str=0;
	ext->v.json=json;
}

int wtk_semfld_feed_lua(wtk_semfld_t *fld,wtk_act_t *act,char *func,wtk_semfld_ext_data_t *ext,wtk_strbuf_t *buf)
{
	wtk_lua2_arg_t arg[4];

	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=fld->dlg;
	arg[1].type=WTK_LUA2_THS;
	arg[1].v.ths=fld;
	arg[2].type=WTK_LUA2_THS;
	arg[2].v.ths=act;
	if(ext)
	{
		if(ext->is_str)
		{
			arg[3].type=WTK_LUA2_STRING;
			arg[3].v.str=*(ext->v.str);
		}else
		{
			arg[3].type=WTK_LUA2_PUSH_VALUE;
			arg[3].v.push_value.ths=ext->v.json;
			arg[3].v.push_value.push=(wtk_lua2_push_value_f)wtk_lua_push_json;
		}
		return wtk_lua2_process_arg(fld->dlg->lua,func,buf,arg,arg+1,arg+2,arg+3,NULL);
	}else
	{
		arg[3].type=WTK_LUA2_THS;
		arg[3].v.ths=fld->kg;
		return wtk_lua2_process_arg(fld->dlg->lua,func,buf,arg,arg+1,arg+2,arg+3,NULL);
	}
}



void wtk_semfld_set_output(wtk_semfld_t *fld,char *data,int bytes)
{
	//wtk_debug("fld=%p [%.*s]\n",fld,bytes,data);
	fld->output=wtk_heap_dup_string(fld->dlg->loc_heap,data,bytes);
}

int wtk_semfld_process_nlg_item_str(wtk_semfld_t *fld,wtk_nlg_value_str_t *vi,wtk_act_t *act)
{
	wtk_strbuf_t *buf=fld->dlg->buf;
	wtk_queue_node_t *qn;
	wtk_nlg_value_str_slot_t *slot;
	wtk_string_t *v;

	//wtk_act_print(act);
	//wtk_debug("len=%d\n",vi->str_q.length);
	wtk_strbuf_reset(buf);
	if(fld->output)
	{
		wtk_strbuf_push(buf,fld->output->data,fld->output->len);
	}
	for(qn=vi->str_q.pop;qn;qn=qn->next)
	{
		slot=data_offset2(qn,wtk_nlg_value_str_slot_t,q_n);
		//wtk_debug("slot=%p [%.*s]\n",slot,slot->v.str->len,slot->v.str->data);
		if(slot->is_str)
		{
			wtk_strbuf_push(buf,slot->v.str->data,slot->v.str->len);
		}else
		{
			v=wtk_act_get_str_value(act,slot->v.var->data,slot->v.var->len);
			if(!v)
			{
				if(wtk_string_cmp_s(slot->v.var,".name")==0)
				{
					v=&(fld->dlg->cfg->assist.name);
				}
			}
			if(!v)
			{
				wtk_debug("[%.*s] not found\n",slot->v.var->len,slot->v.var->data);
				return -1;
			}
			wtk_strbuf_push(buf,v->data,v->len);
		}
	}
	//wtk_debug("[%.*s]=%p\n",buf->pos,buf->data,fld->output);
	wtk_semfld_set_output(fld,buf->data,buf->pos);
	return 0;
}

int wtk_semfld_process_nlg_item2(wtk_semfld_t *fld,wtk_nlg_item_t *item,wtk_act_t *act,wtk_semfld_ext_data_t *ext)
{
	wtk_nlg_value_t *v;
	wtk_strbuf_t *buf;
	int ret=-1;
	wtk_nlg_t *nlg;
	wtk_string_t *output;

	buf=wtk_strbuf_new(256,1);
	nlg=fld->fst?fld->fst->nlg:fld->cfg->nlg;
	v=wtk_nlg_get_item_value(nlg,item);
	//wtk_debug("v=%p\n",v);
	if(!v){goto end;}
	//wtk_nlg_value_print(v);
	if(item->comment)
	{
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,item->comment->data,item->comment->len);
		wtk_strbuf_push_c(buf,0);
		output=fld->output;
		ret=wtk_semfld_feed_lua(fld,act,buf->data,ext,buf);
		//wtk_debug("ret=%d %p %s\n",ret,fld->output,buf->data);
		if(ret!=0){goto end;}
		if(output)
		{
			wtk_strbuf_push_front(buf,output->data,output->len);
		}
		if(fld->output)// || (fld!=fld->dlg->post_fld && fld->dlg->cur_fld!=fld))
		{
			//wtk_debug("goto end %p\n",fld->output);
			goto end;
		}
	}
	if(v->is_str)
	{
		ret=wtk_semfld_process_nlg_item_str(fld,v->v.str,act);
	}else
	{
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,v->v.var->data,v->v.var->len);
		wtk_strbuf_push_c(buf,0);
		//wtk_debug("feed lua %s\n",buf->data);
		ret=wtk_semfld_feed_lua(fld,act,buf->data,ext,buf);
		if(buf->pos>0)
		{
			if(fld->output)
			{
				wtk_strbuf_push_front(buf,fld->output->data,fld->output->len);
			}
			wtk_semfld_set_output(fld,buf->data,buf->pos);
		}
	}
	if(ret!=0){goto end;}
	if(item->nxt.item)
	{
		//wtk_nlg_item_print(item->nxt.item);
		ret=wtk_semfld_process_nlg_item2(fld,item->nxt.item,act,ext);
		if(ret!=0){goto end;}
	}
end:
	wtk_strbuf_delete(buf);
	//wtk_debug("output=%p ret=%d\n",fld->output,ret);
	return ret;
}

int wtk_semfld_process_nlg_item(wtk_semfld_t *fld,wtk_nlg_item_t *item,wtk_act_t *act,wtk_semfld_ext_data_t *ext)
{
	//fld->output=NULL;
	return wtk_semfld_process_nlg_item2(fld,item,act,ext);
}


wtk_nlg_item_t* wtk_semfld_process_nlg_node(wtk_semfld_t *fld,wtk_nlg_node_t *node,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_nlg_node_t *n;
	wtk_string_t *v;
	wtk_nlg_item_t *item;

	//wtk_debug("len=%d %d/%d\n",node->attr_q.length,node->item->attr->nslot,wtk_act_nslot(act));
	if(node->attr_q.length==0)
	{
		//wtk_debug("len=%d match=%d item=%p\n",node->attr_q.length,node->item->match_all_sot,node->item);
		//wtk_nlg_node_print(node);
		if(node->item->match_all_sot)
		{
			return node->item->attr->nslot==wtk_act_nslot(act)?node->item:NULL;
		}else
		{
			return node->item;
		}
	}
	if(node->item)
	{
		if(node->item->attr->nslot==wtk_act_nslot(act))
		{
			return node->item;
		}
	}
	for(qn=node->attr_q.pop;qn;qn=qn->next)
	{
		n=data_offset2(qn,wtk_nlg_node_t,q_n);
		v=wtk_act_get_str_value(act,n->key->k->data,n->key->k->len);
		if(v)
		{
			if(!n->key->v || (wtk_string_cmp(n->key->v,v->data,v->len)==0))
			{
				item=wtk_semfld_process_nlg_node(fld,n,act);
				if(item){return item;}
			}
		}
	}
	if(node->item && !node->item->match_all_sot)
	{
		return node->item;
	}
	return NULL;
}

wtk_nlg_item_t* wtk_semfld_get_nlg_item(wtk_semfld_t *fld,wtk_nlg_root_t *root,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_nlg_node_t *node;
	wtk_string_t *v;
	wtk_nlg_item_t *item=NULL,*max_item=NULL;

	//wtk_debug("attr_q=%d\n",root->attr_q.length);
	if(root->attr_q.length>0)
	{
		max_item=NULL;
		for(qn=root->attr_q.pop;qn;qn=qn->next)
		{
			node=data_offset2(qn,wtk_nlg_node_t,q_n);
			v=wtk_act_get_str_value(act,node->key->k->data,node->key->k->len);
			//wtk_debug("v=%p %.*s\n",v,node->key->k->len,node->key->k->data);
			if(v)
			{
				if(!node->key->v || (wtk_string_cmp(node->key->v,v->data,v->len)==0))
				{
					item=wtk_semfld_process_nlg_node(fld,node,act);
					//wtk_act_print(&act);
					//wtk_debug("item=%p\n",item);
					//if(item){break;}
					if(item)
					{
						if(max_item)
						{
							if(item->attr->nslot>max_item->attr->nslot)
							{
								max_item=item;
							}
						}else
						{
							max_item=item;
						}
					}
				}
			}
		}
		if(max_item)
		{
			item=max_item;
			//wtk_nlg_item_print(max_item);
		}
		//wtk_debug("item=%p [%.*s]\n",item,fld->cfg->name.len,fld->cfg->name.data);
		if(!item && root->item)
		{
			if(root->item->match_all_sot==0)
			{
				item=root->item;
			}
		}
	}else
	{
		if(root->item->match_all_sot==0)
		{
			item=root->item;
		}else
		{
			if(root->item->attr->nslot==wtk_act_nslot(act))
			{
				item=root->item;
			}else
			{
				item=NULL;
			}
		}
	}
	return item;
}


int wtk_semfld_process_nlg_root(wtk_semfld_t *fld,wtk_nlg_root_t *root,wtk_act_t *act,wtk_semfld_ext_data_t *ext)
{
	wtk_nlg_item_t *item=NULL;
	int nslot;

	nslot=wtk_act_nslot(act);
	//wtk_nlg_root_print(root);
	if(nslot>0)
	{
		item=wtk_semfld_get_nlg_item(fld,root,act);
	}else
	{
		if(root->item && nslot==0)
		{
			item=root->item;
		}
	}
	//wtk_debug("item=%p ext=%p\n",item,ext);
	if(item)
	{
		//wtk_nlg_item_print(item);
		return  wtk_semfld_process_nlg_item(fld,item,act,ext);
	}
	return 0;
}

int wtk_semfld_set_str(wtk_semfld_t *fld,char *k,int klen,char *v,int vlen,int use_history)
{
	wtk_nlg_node_t *node;
	wtk_string_t x1,x2;
	wtk_semfld_ext_data_t ext;
	//int ret;

	//wtk_debug("[%.*s]=[%.*s]=%p\n",klen,k,vlen,v,fld->cfg->nlg_usr_set);
	if(fld->cfg->nlg_usr_set)
	{
		wtk_string_set(&(x1),k,klen);
		wtk_string_set(&(x2),v,vlen);
		node=wtk_nlg_root_get_node(fld->cfg->nlg,fld->cfg->nlg_usr_set,&x1,&x2,0);
		//wtk_debug("node=%p item=%p\n",node,node->item);
		if(node && node->item)
		{
			//wtk_nlg_node_print(node);
			wtk_semfld_ext_data_set_str(&(ext),&(x2));
			if(!use_history)
			{
				wtk_semfld_process_nlg_item(fld,node->item,&(fld->fi->act),&(ext));
			}else
			{
				wtk_act_t act;

				wtk_act_init_slot(&(act),fld->slot);
				wtk_semfld_process_nlg_item(fld,node->item,&(act),&(ext));
			}
//			if(ret!=0)
//			{
//				return ret;
//			}
			//return ret;
			return 0;
		}
	}
	wtk_semslot_set(fld->slot,k,klen,v,vlen);
	return 0;
}

int wtk_semfld_set_json(wtk_semfld_t *fld,char *k,int klen,wtk_json_item_t *ji)
{
	wtk_nlg_node_t *node;
	wtk_string_t x1;
	wtk_semfld_ext_data_t ext;
	int ret;
	wtk_string_t *v;

	//wtk_debug("[%.*s]=%p\n",vlen,v,fld->cfg->nlg_usr_set);
	if(fld->cfg->nlg_usr_set)
	{
		wtk_string_set(&(x1),k,klen);
		node=wtk_nlg_root_get_node(fld->cfg->nlg,fld->cfg->nlg_usr_set,&x1,NULL,0);
		//wtk_debug("node=%p item=%p\n",node,node->item);
		if(node && node->item)
		{
			wtk_semfld_ext_data_set_json(&(ext),ji);
			ret=wtk_semfld_process_nlg_item(fld,node->item,&(fld->fi->act),&(ext));
//			if(ret!=0)
//			{
//				return ret;
//			}
			return ret;
		}
	}
	v=wtk_act_json_get_str_value(ji);
	if(v)
	{
		wtk_semslot_set(fld->slot,k,klen,v->data,v->len);
	}
	return 0;
}



int wtk_semfld_del(wtk_semfld_t *fld,char *k,int klen)
{
	wtk_nlg_node_t *node;
	wtk_string_t x1;
	//int ret;

	if(fld->cfg->nlg_usr_del)
	{
		wtk_string_set(&(x1),k,klen);
		node=wtk_nlg_root_get_node(fld->cfg->nlg,fld->cfg->nlg_usr_set,&x1,NULL,0);
		if(node && node->item)
		{
			wtk_semfld_process_nlg_item(fld,node->item,&(fld->fi->act),NULL);
//			if(ret!=0)
//			{
//				return ret;
//			}
			//return ret;
		}
	}
	wtk_semslot_del(fld->slot,k,klen);
	return 0;
}

wtk_string_t* wtk_semfld_get(wtk_semfld_t *fld,char *k,int klen)
{
	wtk_semslot_item_t *item;

	item=wtk_semslot_get(fld->slot,k,klen);
	return item?item->v:NULL;
}

int wtk_semfld_update_slot(wtk_semfld_t *fld,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_json_obj_item_t *oi;
	wtk_string_t *v;
	wtk_json_item_t *vi;
	int ret;

	if(!act->v.json.req){return 0;}
	//wtk_act_print(act);
	//wtk_json_item_print3(act->v.json.req);
	for(qn=act->v.json.req->v.object->pop;qn;qn=qn->next)
	{
		oi=data_offset2(qn,wtk_json_obj_item_t,q_n);
		//wtk_debug("[%.*s]=%p\n",oi->k.len,oi->k.data,oi);
		if(wtk_act_is_str_value(act,oi->k.data,oi->k.len))
		{
			v=wtk_act_get_str_value(act,oi->k.data,oi->k.len);
			//wtk_debug("[%.*s]=%p\n",oi->k.len,oi->k.data,v);
			if(v)
			{
				//wtk_debug("[%.*s]=[%.*s]\n",oi->k.len,oi->k.data,v->len,v->data);
				ret=wtk_semfld_set_str(fld,oi->k.data,oi->k.len,v->data,v->len,0);
				if(ret!=0){return -1;}
			}
		}else
		{
			vi=wtk_json_act_get_value(&(act->v.json),oi->k.data,oi->k.len);
			if(vi)
			{
				wtk_semfld_set_json(fld,oi->k.data,oi->k.len,vi);
			}
		}
	}
	return 0;
}


int wtk_semfld_check_must_slot(wtk_semfld_t *fld,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_semslot_item_cfg_t *ci;
	wtk_semslot_item_t *si;
	wtk_nlg_node_t *node;
	wtk_string_t x1;
	int ret;
	wtk_act_t sys_act;

	wtk_act_init_slot(&(sys_act),fld->slot);
	act=&(sys_act);
	for(qn=fld->cfg->slot.slot_q.pop;qn;qn=qn->next)
	{
		ci=data_offset2(qn,wtk_semslot_item_cfg_t,q_n);
		if(!ci->can_be_nil)
		{
			si=wtk_semslot_get(fld->slot,ci->name.data,ci->name.len);
			//wtk_debug("ask %.*s\n",ci->name.len,ci->name.data);
			if(!si)
			{
				if(fld->cfg->nlg_sys_ask)
				{
					wtk_string_set(&(x1),ci->name.data,ci->name.len);
					node=wtk_nlg_root_get_node(fld->cfg->nlg,fld->cfg->nlg_sys_ask,&x1,NULL,0);
					if(node && node->item)
					{
//						wtk_nlg_item_t* item;
//
//						item=wtk_semfld_process_nlg_node(fld,node,act);
//						if(!item)
//						{
//							item=node->item;
//						}
						wtk_semfld_set_ask_slot(fld,ci->name.data,ci->name.len,0,0);
						ret=wtk_semfld_process_nlg_item(fld,node->item,act,NULL);
						//ret=wtk_semfld_process_nlg_item(fld,item,&(fld->fi->act),NULL);
						return ret;
					}
				}
				if(ci->ask.len>0)
				{
					wtk_semfld_set_ask_slot(fld,ci->name.data,ci->name.len,0,0);
					wtk_semfld_set_output(fld,ci->ask.data,ci->ask.len);
					return 0;
				}
			}
		}
	}
	return 0;
}


int wtk_semfld_answer(wtk_semfld_t *fld)
{
	wtk_nlg_item_t *item=NULL;
	wtk_nlg_root_t *root;
	wtk_act_t act;

	if(!fld->cfg->nlg_sys_answer)
	{
		return 0;
	}
	wtk_act_init_slot(&(act),fld->slot);
	root=fld->cfg->nlg_sys_answer;
	//wtk_nlg_root_print(root);
	item=wtk_semfld_get_nlg_item(fld,root,&(act));
	if(item)
	{
		//wtk_nlg_item_print(item);
		//wtk_act_print(&(act));
		return  wtk_semfld_process_nlg_item(fld,item,&act,NULL);
	}
	return 0;
}


int wtk_semfld_want_skip(wtk_semfld_t *fld,wtk_act_t *act)
{
	if(!(act->v.json.req) || !(act->v.json.req->v.object->pop))
	{
		return 1;
	}else
	{
		return 0;
	}
}

int wtk_semfld_valid_input(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_act_t *act;
	int ret;
	int valid=0;

	fld->output=NULL;
	ret=wtk_semfi_process(fld->fi,data,bytes);
	if(ret!=0){goto end;}
	act=&(fld->fi->act);
	if(wtk_act_nslot(act)>0)
	{
		valid=1;
	}
end:
	wtk_semfi_reset(fld->fi);
	return valid;
}



int wtk_semfld_process(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_act_t *act;
	int ret;
	int n;
	int b;
	int use_history=0;

	if(fld->cfg->nlg_close.len>0)
	{
		wtk_semfld_process_nlg2(fld,fld->cfg->nlg_close.data,fld->cfg->nlg_close.len);
		if(fld->output)
		{
			ret=0;
			goto end;
		}
	}
	fld->want_add_hist=1;
	if(fld->kg)
	{
		return  wtk_semfld_process3(fld,data,bytes);
	}
	if(fld->fst)
	{
		return  wtk_semfld_process2(fld,data,bytes);
	}
	if(fld->dlg->cfg->debug)
	{
		wtk_debug("domain=%.*s\n",fld->cfg->name.len,fld->cfg->name.data);
	}
	//wtk_semfld_reset(fld);
	fld->output=NULL;
	ret=wtk_semfi_process(fld->fi,data,bytes);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0){goto end;}
	ret=0;
	act=&(fld->fi->act);
	if(fld->dlg->cfg->debug)
	{
		wtk_act_print(act);
	}
	//wtk_debug("domain=%d\n",fld->dlg->domained);
	if(fld->cfg->nlg_main && fld->cfg->nlg_main->item)
	{
		ret=wtk_semfld_process_nlg_item(fld,fld->cfg->nlg_main->item,act,NULL);
		goto end;
	}
	//wtk_debug("domain=%d\n",fld->dlg->domained);
	if(!fld->dlg->domained && wtk_act_nslot(act)==0)
	{
		return 1;
	}
	//wtk_debug("req=%p\n",act->v.json.req);
	if(fld->cfg->nlg_usr_inform)
	{
		n=wtk_act_nslot(act);
		b=1;
		if(n==0 && wtk_semslot_nslot(fld->slot)>0 && fld->dlg->domained)
		{
			++fld->ntry;
			//wtk_debug("ntry=%d/%d fld=%p/%p\n",fld->ntry,fld->cfg->max_empty_input_try,fld,fld->dlg->cur_fld);
			if(fld->ntry>fld->cfg->max_empty_input_try)
			{
				fld->ntry=0;
				wtk_semslot_reset(fld->slot);
			}else
			{
				use_history=1;
				b=0;
			}
		}
		if(n>0)
		{
			fld->ntry=0;
		}
		//wtk_debug("b=%d ntry=%d\n",b,fld->ntry);
		if(b)
		{
			//fld->ntry=0;
			ret=wtk_semfld_process_nlg_root(fld,fld->cfg->nlg_usr_inform,act,NULL);
			//wtk_debug("ret=%d output=%p\n",ret,fld->output);
			if(ret!=0){goto end;}
			if(fld->output){goto end;}
		}
	}
	if(wtk_semfld_want_skip(fld,act))//  && fld->slot->cfg->slot_q.length>0)
	{
		//wtk_debug("fld=%p %p last=%p\n",fld,fld->dlg->cur_fld,fld->dlg->last_fld);
		if(fld->output || fld!=fld->dlg->cur_fld)
		{
			//wtk_debug("check yes ...\n");
			return 0;
		}else
		{
			if(wtk_semslot_nslot(fld->slot)==0)
			{
				return 1;
			}
		}
	}
	//wtk_debug("set ........\n");
	ret=wtk_semfld_update_slot(fld,act);
	if(ret!=0){goto end;}
	//wtk_act_print(act);
	if(fld->output || fld!=fld->dlg->cur_fld){goto end;}
	ret=wtk_semfld_check_must_slot(fld,act);
	if(ret!=0){goto end;}
	//wtk_debug("slot=%d\n",fld->slot->slot_q.length);
	if(fld->output || fld!=fld->dlg->cur_fld){goto end;}
	ret=wtk_semfld_answer(fld);//某个领域的固定字段
	if(ret!=0){goto end;}
	if(fld->output || fld!=fld->dlg->cur_fld){goto end;}
	wtk_act_update(act);
	if(!fld->output && use_history)
	{
		fld->ntry=fld->cfg->max_empty_input_try;
		return wtk_semfld_process(fld,data,bytes);
	}
	ret=0;
end:
	//wtk_debug("[%.*s]\n",fld->cfg->name.len,fld->cfg->name.data);
	//wtk_semslot_print(fld->slot);
	//wtk_debug("ret=%d\n",ret);
	if(fld->dlg->cfg->debug && fld->output)
	{
		wtk_debug("[%.*s]\n",fld->output->len,fld->output->data);
	}
	return ret;
}

void wtk_semfld_process_nlg(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_nlg_item_t *item;
	wtk_act_t act;

	//wtk_debug("[%.*s]\n",bytes,data);
	if(fld->fst)
	{
		item=wtk_nlg_get2(fld->fst->nlg,data,bytes,NULL);
	}else
	{
		item=wtk_nlg_get2(fld->cfg->nlg,data,bytes,NULL);
	}
	if(item)
	{
		wtk_act_init_slot(&(act),fld->slot);
		wtk_semfld_process_nlg_item(fld,item,&(act),NULL);
	}
	//exit(0);
}

int wtk_semfld_process_nlg2(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_str_hash_t *hash;
	wtk_nlg_item_t *item;
	wtk_act_t act;
	int ret;

	//wtk_debug("[%.*s]\n",bytes,data);
	if(fld->kg)
	{
		wtk_string_t t;

		//wtk_debug("process nlg[%.*s]\n",bytes,data);
		t=wtk_kgr_process_nlg(fld->kg,data,bytes);
		//wtk_debug("[%.*s]\n",t.len,t.data);
		if(t.len>0)
		{
			wtk_semfld_set_output(fld,t.data,t.len);
		}
		return 0;
	}
	hash=wtk_str_hash_new(4);
	if(fld->fst)
	{
		item=wtk_nlg_get2(fld->fst->nlg,data,bytes,hash);
	}else
	{
		item=wtk_nlg_get2(fld->cfg->nlg,data,bytes,hash);
	}
	if(item)
	{
		//wtk_nlg_item_print(item);
		wtk_act_init_hash(&(act),hash);
		ret=wtk_semfld_process_nlg_item(fld,item,&(act),NULL);
	}else
	{
		wtk_debug("[%.*s] not found.\n",bytes,data);
		//wtk_debug("========================\n");
		ret=-1;
	}
	wtk_str_hash_delete(hash);
	//exit(0);
	return ret;
}

wtk_string_t* wtk_semfld_process_owl(wtk_semfld_t *fld,wtk_act_t *act)
{
	wtk_owl_tree_t *owl=fld->cfg->owl;
	wtk_string_t *v=NULL;
	wtk_string_t *k;
	wtk_owl_item_t *item=NULL;
	wtk_owl_inst_t *inst;
	wtk_json_item_t *prop;
	wtk_queue_node_t *qn;
	wtk_json_array_item_t *ai;


//	inst=wtk_owl_tree_find_inst_s(fld->cfg->owl,"葫芦娃");
//	wtk_owl_inst_print(inst);
//	exit(0);
	//wtk_act_print(act);
	if(!fld->cfg->owl){goto end;}
	k=wtk_act_get_str_value_s(act,"inst");
	if(!k)
	{
		if(fld->dlg->cfg->debug)
		{
			wtk_debug("inst not found\n");
		}
		goto end;
	}
	inst=wtk_owl_tree_match_inst(owl,k->data,k->len);
	//wtk_debug("[%.*s]=%p\n",k->len,k->data,inst);
	if(!inst)
	{
		if(fld->dlg->cfg->debug)
		{
			wtk_debug("inst[%.*s] not found\n",k->len,k->data);
		}
		goto end;
	}
	//wtk_debug("inst=%p\n",inst);
	prop=wtk_json_act_get_value_s(&(act->v.json),"prop");
	if(!prop || prop->type!=WTK_JSON_ARRAY)
	{
		if(fld->dlg->cfg->debug)
		{
			wtk_debug("prop not found\n");
		}
		goto end;
	}
	//wtk_debug("prop=%d\n",prop->v.array->length);
	ai=data_offset2(prop->v.array->pop,wtk_json_array_item_t,q_n);
	if(ai->item->type==WTK_JSON_ARRAY)
	{
		prop=ai->item;
	}
	//wtk_debug("prop=%d\n",prop->v.array->length);
	for(qn=prop->v.array->pop;qn;qn=qn->next)
	{
		ai=data_offset2(qn,wtk_json_array_item_t,q_n);
		k=wtk_act_json_get_str_value(ai->item);
		if(!k){goto end;}
		if(fld->dlg->cfg->debug)
		{
			wtk_debug("[%.*s]\n",k->len,k->data);
		}
		//wtk_debug("[%.*s]\n",k->len,k->data);
		item=wtk_owl_inst_find_prop(inst,k);
		if(!item)
		{
			if(fld->dlg->cfg->debug)
			{
				wtk_debug("[%.*s] not found.\n",k->len,k->data);
			}
			goto end;
		}
		if(qn!=prop->v.array->push)
		{
			if(item->n_vinst==1)
			{
				inst=item->v_inst[0];
			}else
			{
				if(fld->dlg->cfg->debug)
				{
					wtk_debug("[%.*s] multi inst=%d.\n",k->len,k->data,item->n_vinst);
				}
				//多个候选者，或者没有inst;
				item=NULL;
				goto end;
			}
		};
	}
	if(!item){goto end;}
	//wtk_owl_item_print(item);
	v=wtk_owl_item_get_str_value(item);
end:
	//wtk_debug("found bug v=%p\n",v);
	//exit(0);
	return v;
}

//-------------------------------- nlg fst ---------------------------

int wtk_nlgfst_has_attr_act(wtk_nlgfst_t *fst,wtk_nlgnet_arc_attr_t *attr,wtk_act_t *act)
{
	wtk_string_t *v;

	if(attr->v)
	{
		v=wtk_act_get_str_value(act,attr->k->data,attr->k->len);
		//wtk_debug("[%.*s]=%p v=%p\n",attr->k->len,attr->k->data,v,attr->v)
		if(v && wtk_string_cmp(attr->v,v->data,v->len)==0)
		{
			//wtk_debug("[%.*s]\n",v->len,v->data);
			return 1;
		}else
		{
			return 0;
		}
	}else
	{
		return wtk_act_has_key(act,attr->k->data,attr->k->len);
	}
	return 0;
}

int wtk_nlgfst_match_arc_act(wtk_nlgfst_t *fst,wtk_nlgnet_arc_t *arc,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_nlgnet_arc_attr_t *attr;
	int ret;
	int nact;
	wtk_semfld_t *fld;

	//wtk_nlgnet_arc_print(arc);
	//wtk_debug("want_domain=%d\n",arc->domained);
	fld=(wtk_semfld_t*)fst->emit_ths;
	if(arc->playing)
	{
		if(!fld->dlg->playing)
		{
			return 0;
		}
	}
	if(!arc->domained)
	{
		if(fld->dlg->domained)
		{
			return 0;
		}
	}
	if(arc->min_conf>fld->dlg->conf)
	{
		return 0;
	}
	//wtk_debug("arc=%d\n",arc->or);
	if(!arc->or)
	{
		nact=wtk_act_nslot(act);
		//wtk_debug("act attr len %d  arc act attr len %d\n", nact,arc->attr_q.length);
		if(arc->match_all_slot)
		{
			if(nact!=arc->attr_q.length)
			{
				return 0;
			}
		}else
		{
			if(nact<arc->attr_q.length)	//match_all_slot 必须地捕获到的槽的数量大于arc上槽的数量
			{
				return 0;
			}
		}
		for(qn=arc->attr_q.pop;qn;qn=qn->next)
		{
			attr=data_offset2(qn,wtk_nlgnet_arc_attr_t,q_n);
			//wtk_debug("[%.*s]=%p\n",attr->k->len,attr->k->data,attr->v);
			ret=wtk_nlgfst_has_attr_act(fst,attr,act);
			//wtk_debug("ret=%d\n",ret);
			if(ret==0)
			{
				return 0;
			}
		}
		return 1;
	}else
	{
		for(qn=arc->attr_q.pop;qn;qn=qn->next)
		{
			attr=data_offset2(qn,wtk_nlgnet_arc_attr_t,q_n);
			ret=wtk_nlgfst_has_attr_act(fst,attr,act);
			if(ret==1)
			{
				return 1;
			}
		}
		return 0;
	}
}

int wtk_nlgfst_feed_state_act(wtk_nlgfst_t *fst,wtk_nlgnet_state_t *state,wtk_act_t *act);


int wtk_nlgfst_step_arc_act2(wtk_nlgfst_t *fst,wtk_nlgnet_arc_t *arc,wtk_act_t *act)
{
	wtk_nlgnet_state_t *state,*last_state;
	int ret;
	wtk_semfld_t *fld;

	//wtk_nlgnet_arc_print(arc);
	//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
	if(arc->skip_fld)
	{
		//fst->cur_state=wtk_nlgnet_arc_next(arc);
		ret=1;
		goto end;
	}
	fld=(wtk_semfld_t*)fst->emit_ths;
	if(arc->playing)
	{
		if(!fld->dlg->playing)
		{
			return -1;
		}
	}
	if(!arc->domained)
	{
		if(fld->dlg->domained)
		{
			return -1;
		}
	}
	//wtk_debug("================> musc=%d/%d\n",arc->must_domain,fld->dlg->domained);
	if(arc->must_domain)
	{
		if(fld->dlg->domained==0)
		{
			return 1;
		}
	}
	if(arc->min_conf>fld->dlg->conf)
	{
		return -1;
	}
	state=wtk_nlgnet_arc_next(arc);
	if(!state)
	{
		wtk_debug("next state not found.\n");
		ret=-1;goto end;
	}
	last_state=fst->cur_state;
	//wtk_debug("arc item=%p %p\n",arc->emit_item,arc->emit_func);
	if(arc->emit_item)
	{
		ret=fst->emit_process(fst->emit_ths,arc->emit_item);
		//wtk_debug("ret=%d\n",ret);
		if(ret==0)
		{
			if(fst->cur_state==last_state)
			{
				//sys 状态跳转
				if (fst->cur_state != state) {
					fst->cur_state_round=1;
					fst->cur_state=state;
				}else{
					++fst->cur_state_round;
				}
				//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
			}
			//wtk_debug("other=%p\n",fst->cur_state->other);
			if(arc->clean_ctx)
			{
				wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
			}
		}
		goto end;
	}
	if(!arc->use_emit)
	{
		ret=wtk_nlgfst_feed_state_act(fst,state,act);
		goto end;
	}else
	{
		//wtk_debug("emit=%p\n",state->emit_item);
		if(state->emit_item)
		{
			if(state!=fst->cur_state)
			{
				//if(fst->cur_state==last_state)
				//{
				fst->cur_state_round=1;
				fst->cur_state=state;
					//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
				//}
				//wtk_debug("state: %.*s\n",fst->cur_state->name->len,fst->cur_state->name->data);
			}else
			{
				++fst->cur_state_round;
			}
			//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
			//wtk_nlg_item_print(state->emit_item);
			ret=fst->emit_process(fst->emit_ths,state->emit_item);
			//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
			//wtk_debug("ret=%d\n",ret);
			if(ret==0)
			{
				//wtk_debug("other=%p\n",fst->cur_state->other);
				if(arc->clean_ctx)
				{
					wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
				}
			}
		}else
		{
			if(state!=fst->cur_state)
			{
				if(fst->cur_state==last_state)
				{
					fst->cur_state_round=1;
					fst->cur_state=state;
					//wtk_debug("[%.*s]\n",fst->cur_state->name->len,fst->cur_state->name->data);
				}
				//wtk_debug("state: %.*s\n",fst->cur_state->name->len,fst->cur_state->name->data);
			}else{
				++fst->cur_state_round;
			}
			ret=1;
		}
	}
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_nlgfst_step_arc_act(wtk_nlgfst_t *fst,wtk_nlgnet_arc_t *arc,wtk_act_t *act)
{
	int ret = 0;

	//wtk_nlgnet_arc_print(arc);
	if(arc->min_round>0 && fst->cur_state_round<arc->min_round)
	{
		ret=-1;
		goto end;
	}
	if(arc->max_round>0 && fst->cur_state_round>=arc->max_round)
	{
		ret=-1;
		goto end;
	}
	//wtk_nlgnet_arc_print(arc);
	ret=wtk_nlgfst_match_arc_act(fst,arc,act);
	//wtk_debug("ret=%d\n",ret);
	if(ret==0){ret=-1;goto end;}
	ret=wtk_nlgfst_step_arc_act2(fst,arc,act);
	//wtk_debug("ret=%d\n",ret);
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_nlgfst_feed_state_act(wtk_nlgfst_t *fst,wtk_nlgnet_state_t *state,wtk_act_t *act)
{
	wtk_queue_node_t *qn;
	wtk_nlgnet_arc_t *arc;
	int ret;

	//wtk_nlgnet_print(fst->net);
	//exit(0);
	//wtk_debug("state=[%.*s] len=%d\n",state->name->len,state->name->data,state->output_arc_q.length);
	//wtk_debug("feed %.*s output=%d other=%p\n",state->name->len,state->name->data,state->output_arc_q.length,state->other);
	//wtk_debug("other=%p\n",fst->cur_state->other);
	if(state==fst->net->root)
	{
		wtk_semslot_reset(((wtk_semfld_t*)fst->emit_ths)->slot);
	}
	//wtk_act_print(act);
	for(qn=state->output_arc_q.pop;qn;qn=qn->next)
	{
		arc=data_offset2(qn,wtk_nlgnet_arc_t,arc_n);
		//wtk_nlgnet_arc_print(arc);
		ret=wtk_nlgfst_step_arc_act(fst,arc,act);
		//wtk_debug("ret=%d\n",ret);
		if(ret>=0)
		{
			return ret;
		}
	}
	//wtk_debug("other=%p\n",fst->cur_state->other);
	//wtk_nlgnet_state_print(fst->cur_state);
	if(state->other)
	{
		ret=wtk_nlgfst_step_arc_act2(fst,state->other,act);
		//wtk_debug("ret=%d\n",ret);
		if(ret>=0)
		{
			return ret;
		}else if(state->other->fail_re_doamin)
		{
			//wtk_debug("redoamin\n");
			wtk_semfld_t *fld;

			fld=(wtk_semfld_t*)fst->emit_ths;
			fld->is_finish=1;
			wtk_semfld_reset(fld);
			return 1;
		}
		else if(state->other->other)
		{
			return wtk_nlgfst_feed_state_act(fst,state->other->other,act);
		}
	}
	return 1;
}


int wtk_nlgfst_feed_act(wtk_nlgfst_t *fst,wtk_act_t *act)
{
	wtk_semfld_t *fld;
	int ret;

	fld=(wtk_semfld_t*)(fst->emit_ths);
	if(!fst->cur_state)
	{
		return -1;
	}
	if(fst->cur_state && fst->cur_state->pre)
	{
		wtk_semfld_feed_lua(fld,act,fst->cur_state->pre,NULL,fld->dlg->buf);
		if(fld->output)
		{
			return 0;
		}
	}
	ret=wtk_nlgfst_feed_state_act(fst,fst->cur_state,act);
	//wtk_debug("================> ret=%d\n",ret);
	if(ret==0)
	{
		//wtk_debug("[%.*s] eps=%p\n",fst->cur_state->name->len,fst->cur_state->name->data,fst->cur_state->eps);
		if(fst->cur_state->eps)
		{
			fst->cur_state=fst->cur_state->eps;
		}
	}
	if(fld->dlg->cfg->debug)
	{
		wtk_debug("set state=%.*s\n",fst->cur_state?fst->cur_state->name->len:0,fst->cur_state?fst->cur_state->name->data:"")
	}
	return ret;
}

int wtk_semfld_process_fst_emit(wtk_semfld_t *fld,wtk_nlg_item_t *item)
{
	return wtk_semfld_process_nlg_item(fld,item,fld->input_act,NULL);
}

int wtk_semfld_goto_state(wtk_semfld_t *fld,char *nm,int nm_bytes)
{
	wtk_nlgnet_state_t *s;
	wtk_nlgfst_t *fst=fld->fst;

	s=wtk_nlgnet_get_state(fst->net,nm,nm_bytes,0);
	//wtk_debug("s=%p[%.*s]\n",s,s->name->len,s->name->data);
	if(s)
	{
		fst->cur_state=s;
		fst->cur_state_round=0;
		fst->emit_process(fst->emit_ths,s->emit_item);
		return 0;
	}else
	{
		return -1;
	}
}

//int wtk_semfld_process_lex(wtk_semfld_t *fld,char *data,int bytes)
//{
//	return 0;
//}

int wtk_semfld_process2(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_act_t *act;
	int ret;

	fld->input_act=NULL;
	if(fld->dlg->cfg->debug)
	{
		wtk_debug("[%.*s]\n",bytes,data);
		wtk_debug("domain=%.*s domained=%d\n",fld->cfg->name.len,fld->cfg->name.data,fld->dlg->domained);
	}
	//wtk_semfld_reset(fld);
	fld->output=NULL;
	ret=wtk_semfi_process(fld->fi,data,bytes);
	if(ret!=0){goto end;}
	ret=0;
	act=&(fld->fi->act);
	if(fld->dlg->cfg->debug)
	{
		wtk_act_print(act);
		if(fld->fst && fld->fst->cur_state)
		{
			wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
		}
	}
	//wtk_act_print(act);
	//wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
	fld->input_act=act;
	ret=wtk_nlgfst_feed_act(fld->fst,act);//使用act去fst中匹配路径
	//wtk_debug("ret=%d\n",ret);
	if(ret==0)
	{
//		if(fld->fst->cur_state==fld->fst->net->root)
//		{
//			wtk_debug("found same\n");
//			exit(0);
//		}
		//wtk_act_print(act);
		wtk_semfld_update_slot(fld,act);
		//wtk_semslot_print(fld->slot);
	}
	if(fld->fst->cur_state && fld->fst->cur_state==fld->fst->net->end)
	{
		wtk_strbuf_reset(fld->ask_slot);
	}
end:
	//wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
	//wtk_debug("[%.*s]\n",fld->cfg->name.len,fld->cfg->name.data);
	//wtk_semslot_print(fld->slot);
	//wtk_debug("ret=%d\n",ret);
	if(fld->dlg->cfg->debug && fld->output)
	{
		wtk_debug("[%.*s] ret=%d\n",fld->output->len,fld->output->data,ret);
	}
	//exit(0);
	return ret;
}

int wtk_semfld_process3(wtk_semfld_t *fld,char *data,int bytes)
{
	wtk_act_t *act;
	int ret;

	fld->input_act=NULL;
	if(fld->dlg->cfg->debug)
	{
		wtk_debug("domain=%.*s domained=%d kg=%p\n",fld->cfg->name.len,fld->cfg->name.data,fld->dlg->domained,fld->kg);
	}
	//wtk_semfld_reset(fld);
	fld->output=NULL;
	ret=wtk_semfi_process(fld->fi,data,bytes);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0){goto end;}
	ret=0;
	act=&(fld->fi->act);
	if(fld->dlg->cfg->debug)
	{
		wtk_act_print(act);
		//wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
	}
	//wtk_act_print(act);
	//wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
	fld->input_act=act;
	//ret=wtk_nlgfst_feed_act(fld->fst,act);
	//wtk_debug("ret=%d\n",ret);
	ret=wtk_kgr_feed(fld->kg,act);
	if(ret==0)
	{
		wtk_semfld_update_slot(fld,act);
		//wtk_semslot_print(fld->slot);
	}
end:
	//wtk_debug("state: %.*s\n",fld->fst->cur_state->name->len,fld->fst->cur_state->name->data);
	//wtk_debug("[%.*s]\n",fld->cfg->name.len,fld->cfg->name.data);
	//wtk_semslot_print(fld->slot);
	//wtk_debug("ret=%d\n",ret);
	if(fld->dlg->cfg->debug && fld->output)
	{
		wtk_debug("[%.*s]\n",fld->output->len,fld->output->data);
	}
	//exit(0);
	return ret;
}

void wtk_semfld_set_fst_state(wtk_semfld_t *fld,char *state,int state_bytes)
{
	wtk_nlgfst_set_state(fld->fst,state,state_bytes);
}
