#include "wtk_semfi.h"
#include "wtk/semdlg/wtk_semdlg.h"

wtk_semfi_t* wtk_semfi_new(wtk_semfi_cfg_t *cfg,wtk_semfld_t *fld)
{
	wtk_semfi_t *f;

	f=(wtk_semfi_t*)wtk_malloc(sizeof(wtk_semfi_t));
	f->cfg=cfg;
	f->fld=fld;
#ifdef USE_CRF
	if(cfg->crffn)
	{
		f->crf=wtk_crf_new(cfg->crffn);
	}else
	{
		f->crf=NULL;
	}
#endif
	f->crf_hash=NULL;
	wtk_act_init_json(&(f->act),fld->dlg->loc_heap);
	wtk_semfi_reset(f);
	return f;
}

void wtk_semfi_delete(wtk_semfi_t *semfi)
{
	if(semfi->crf_hash)
	{
		wtk_str_hash_delete(semfi->crf_hash);
	}
#ifdef USE_CRF
	if(semfi->crf)
	{
		wtk_crf_delete(semfi->crf);
	}
#endif
	wtk_free(semfi);
}

void wtk_semfi_reset(wtk_semfi_t *semfi)
{
#ifdef USE_CRF
	if(semfi->crf)
	{
		wtk_crf_reset(semfi->crf);
	}
#endif
	wtk_act_reset(&(semfi->act));
	if(semfi->crf_hash)
	{
		wtk_str_hash_delete(semfi->crf_hash);
		semfi->crf_hash=NULL;
	}
}

wtk_string_t* wtk_semfld_get_var(wtk_semfld_t *fld,char *act,int act_bytes,char *name,int name_bytes)
{
	//wtk_debug("[%.*s]\n",name_bytes,name);
	if(wtk_str_equal_s(name,name_bytes,".state"))
	{
		if(fld->fst && fld->fst->cur_state)
		{
			return fld->fst->cur_state->name;
		}else
		{
			return NULL;
		}
	}if(wtk_str_equal_s(name,name_bytes,".kg"))
	{
		//wtk_debug("kg=%p next=%p\n",fld->kg,fld->kg?fld->kg->next:NULL);
		if(fld->kg && fld->kg->next)
		{
			//wtk_debug("[%.*s]\n",fld->kg->next->name->len,fld->kg->next->name->data);
			return fld->kg->next->name;
		}else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


int wtk_semfld_has_var(wtk_semfld_t *fld,char *act,int act_bytes,char *name,int name_bytes)
{
	wtk_string_t *v;
	int ret;

	//wtk_debug("[%.*s]:[%.*s]\n",act_bytes,act,name_bytes,name);
	if(wtk_str_equal_s(act,act_bytes,"want") && fld->ask_slot->pos>0)
	{
		//wtk_debug("[%.*s]\n",fld->ask_slot->pos,fld->ask_slot->data);
		if(fld->ask_slot && wtk_str_equal(name,name_bytes,fld->ask_slot->data,fld->ask_slot->pos))
		{
			ret=1;
			goto end;
		}
	}else if(wtk_str_equal_s(act,act_bytes,"slot"))
	{
		v=wtk_semfld_get(fld,name,name_bytes);
		if(v)
		{
			ret=1;
			goto end;
		}
	}
	if(wtk_str_equal_s(name,name_bytes,"want"))
	{
		//wtk_debug("ask[%.*s]\n",fld->ask_slot->pos,fld->ask_slot->data);
		if(fld->ask_slot->pos>0)
		{
			ret=1;
		}else
		{
			ret=0;
		}
		goto end;
	}
	ret=0;
end:
	//wtk_debug("[%.*s]=[%.*s] ret=%d\n",act_bytes,act,name_bytes,name,ret);
	return ret;
}

#ifdef USE_CRF
int wtk_semfi_process_crf_act(wtk_semfi_t *semfi,char *s,int len,wtk_str_hash_t *hash)
{
	static wtk_string_t xi=wtk_string("i");
	wtk_poseg_t *seg=semfi->fld->dlg->lexr->poseg;
	wtk_crf_t *crf=semfi->crf;
	int i,n;
	const char *ts;
	wtk_strbuf_t *buf;
	wtk_string_t *v;
	hash_str_node_t *node;
	int ret=-1;
	int add=1;

	buf=wtk_strbuf_new(256,1);
	//wtk_debug("[%.*s]\n",len,s);
	wtk_poseg_process(seg,s,len);
	for(i=0;i<seg->nwrd;++i)
	{
		//wtk_debug("v[%d]=%.*s %.*s\n",i,seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data);
		wtk_strbuf_reset(buf);
		wtk_strbuf_push(buf,seg->wrds[i]->data,seg->wrds[i]->len);
		wtk_strbuf_push_s(buf," ");
		wtk_strbuf_push(buf,seg->pos[i]->data,seg->pos[i]->len);
		wtk_strbuf_push_c(buf,0);
		ret=wtk_crf_add(crf,buf->data);
		if(ret!=0){goto end;}
	}
	//exit(0);
	ret=wtk_crf_process(crf);
	if(ret!=0){goto end;}
	n=wtk_crf_nresult(crf);
	for(i=0;i<n;++i)
	{
		ts=wtk_crf_get(crf,i);
		if(!ts){continue;}
		//wtk_debug("v[%d]=%.*s %.*s %s\n",i,seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts);
		//printf("%.*s %.*s %s f=%f\n",seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts,wtk_crf_get_prob(crf,i));
		//printf("%.*s %.*s %s\n",seg->wrds[i]->len,seg->wrds[i]->data,seg->pos[i]->len,seg->pos[i]->data,ts);
		if(strcmp(ts,"n")==0)
		{
			continue;
		}
		if(ts[0]=='.')
		{
			add=0;
			wtk_str_hash_remove(hash,"act",3);
			v=wtk_heap_dup_string(hash->heap,(char*)(ts+1),strlen(ts)-1);
			wtk_str_hash_add2(hash,"act",3,v);
		}else
		{
			wtk_strbuf_reset(buf);
			node=wtk_str_hash_remove(hash,(char*)ts,strlen(ts));
			if(node)
			{
				v=(wtk_string_t*)node->value;
				wtk_strbuf_push(buf,v->data,v->len);
			}
			wtk_strbuf_push(buf,seg->wrds[i]->data,seg->wrds[i]->len);
			v=wtk_heap_dup_string(hash->heap,buf->data,buf->pos);
			wtk_str_hash_add2(hash,(char*)ts,strlen(ts),v);
		}
	}
	if(add)
	{
		wtk_str_hash_add2(hash,"act",3,&xi);
	}
	ret=0;
end:
	wtk_poseg_reset(seg);
	wtk_strbuf_delete(buf);
	wtk_crf_reset(crf);
	return ret;
}


int  wtk_semfi_process_crf(wtk_semfi_t *semfi,char *data,int bytes)
{
	wtk_lexr_t *l=semfi->fld->dlg->lexr;
	wtk_json_item_t *item;
	wtk_json_obj_item_t *ji;
	wtk_queue_node_t *qn;
	wtk_string_t *v;
	int ret;
	wtk_str_hash_t *hash;

	hash=wtk_str_hash_new(7);
	semfi->crf_hash=hash;
	wtk_act_init_hash(&(semfi->act),hash);
	if(semfi->cfg->net)
	{
		wtk_lexr_set_has_var_function(l,semfi->fld,(wtk_lexr_has_var_f)wtk_semfld_has_var);
		wtk_lexr_set_get_var_function(l,semfi->fld,(wtk_lexr_get_var_f)wtk_semfld_get_var);
		ret=wtk_lexr_process(l,semfi->cfg->net,data,bytes);
		if(ret!=0){ret=-1;goto end;}
		//wtk_lexr_print(l);
		//crf to normalize
		if(l->action)
		{
			item=wtk_json_obj_get_s(l->action,"request");
			if(item && item->type==WTK_JSON_OBJECT)
			{
				for(qn=item->v.object->pop;qn;qn=qn->next)
				{
					ji=data_offset(qn,wtk_json_obj_item_t,q_n);
					v=wtk_json_item_get_str_value(ji->item);
					if(ji && v)
					{
						//wtk_debug("[%.*s]=[%.*s]\n",ji->k.len,ji->k.data,v->len,v->data);
						v=wtk_heap_dup_string(hash->heap,v->data,v->len);
						wtk_str_hash_add2(hash,ji->k.data,ji->k.len,v);
					}
				}
				ret=0;
				goto end;
			}
		}
	}
	ret=wtk_semfi_process_crf_act(semfi,data,bytes,hash);
	//exit(0);
end:
	return ret;
}
#endif

int wtk_semfi_process(wtk_semfi_t *semfi,char *data,int bytes)
{
	wtk_lexr_t *l=semfi->fld->dlg->lexr;
	wtk_heap_t *heap=semfi->fld->dlg->loc_heap;
	int ret=-1;

	//wtk_debug("[%.*s]=%p %p\n",bytes,data,semfi->crf,semfi->cfg->crffn);
#ifdef USE_CRF
	if(semfi->crf)
	{
		ret=wtk_semfi_process_crf(semfi,data,bytes);
	}else if(semfi->cfg->net)
#endif
	{
		wtk_lexr_set_has_var_function(l,semfi->fld,(wtk_lexr_has_var_f)wtk_semfld_has_var);
		wtk_lexr_set_get_var_function(l,semfi->fld,(wtk_lexr_get_var_f)wtk_semfld_get_var);
		ret=wtk_lexr_process(l,semfi->cfg->net,data,bytes);
		if(ret!=0 || !l->action){ret=-1;goto end;}
		//wtk_lexr_print(l);
		wtk_act_init_json(&(semfi->act),heap);
		semfi->act.v.json.json=wtk_json_item_dup(l->action,heap);
		wtk_act_update(&(semfi->act));
	}
	//wtk_json_item_print3(semfi->act);
	ret=0;
end:
	wtk_lexr_reset(l);
	return ret;
}
