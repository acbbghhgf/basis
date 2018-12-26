#include "wtk_semdlg.h" 
#include "wtk/semdlg/semfld/wtk_act_lua.h"
#include "wtk/semdlg/wtk_semlua.h"

typedef struct
{
	wtk_string_t *msg;
	wtk_string_t *v;
}wtk_semdlg_robot_msg_t;

int wtk_semdlg_process_robot(wtk_semdlg_t *dlg,wtk_semdlg_robot_msg_t *msg);
int wtk_semdlg_robot_start(wtk_semdlg_t *dlg);

wtk_semdlg_robot_msg_t* wtk_semdlg_robot_msg_new(char *msg,int msg_len)
{
	wtk_semdlg_robot_msg_t *x;

	x=(wtk_semdlg_robot_msg_t*)wtk_malloc(sizeof(wtk_semdlg_robot_msg_t));
	x->msg=wtk_string_dup_data(msg,msg_len);
	x->v=NULL;
	return x;
}

wtk_semdlg_robot_msg_t* wtk_semdlg_robot_msg_new2(char *msg,int msg_len,char *v,int v_len)
{
	wtk_semdlg_robot_msg_t *x;

	x=(wtk_semdlg_robot_msg_t*)wtk_malloc(sizeof(wtk_semdlg_robot_msg_t));
	x->msg=wtk_string_dup_data(msg,msg_len);
	x->v=wtk_string_dup_data(v,v_len);
	return x;
}

void wtk_semdlg_robot_msg_delete(wtk_semdlg_robot_msg_t *msg)
{
	wtk_string_delete(msg->msg);
	if(msg->v)
	{
		wtk_string_delete(msg->v);
	}
	wtk_free(msg);
}

void wtk_semdlg_update_flds(wtk_semdlg_t *dlg)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;
	wtk_semfld_t *fld;

	for(qn=dlg->cfg->fld_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		fld=wtk_semfld_new(&(item->fld),dlg);
		wtk_queue_push(&(dlg->fld_q),&(fld->q_n));
		wtk_str_hash_add(dlg->fld_hash,fld->cfg->name.data,fld->cfg->name.len,fld);
		//wtk_debug("%.*s bytes %d\n", fld->cfg->name.len, fld->cfg->name.data, wtk_heap_bytes(fld->cfg->semfi.net->heap));
	}
}

wtk_semdlg_t* wtk_semdlg_new(wtk_semdlg_cfg_t *cfg)
{
	wtk_semdlg_t *dlg = NULL;
	wtk_rbin2_t *rbin = NULL;
	int ret = 0;

	dlg=(wtk_semdlg_t*)wtk_malloc(sizeof(wtk_semdlg_t));
	if (dlg == NULL) {
		wtk_debug("semdlg mem err\n");
		ret = -1;
		goto end;
	}
	dlg->cfg=cfg;
	wtk_semdlg_env_init(&(dlg->env));
	dlg->jsonkv=wtk_jsonkv_new(cfg->brain_dn);
	if(cfg->use_rbin)
	{
		//rbin=cfg->cfg.bin_cfg->rbin;
		rbin=cfg->rbin;
		//wtk_debug("rbin=%p\n",rbin);
	}else
	{
		rbin=NULL;
	}
	dlg->str_output=wtk_strbuf_new(256,1);
	dlg->kgkv=wtk_kgkv_new(&(cfg->kgkv),rbin);
	if(cfg->use_chnlike)
	{
		dlg->chnlike=wtk_chnlike_new(&(cfg->chnlike),rbin);
	}else
	{
		dlg->chnlike=NULL;
	}
	if(cfg->use_faq)
	{
		dlg->faq=wtk_faq_new(&(cfg->faq),rbin);
	}else
	{
		dlg->faq=NULL;
	}
	if(cfg->use_faqc)
	{
		dlg->faqc=wtk_faqc_new(&(cfg->faqc));
	}else
	{
		dlg->faqc=NULL;
	}
	dlg->lua=wtk_lua2_new(&(cfg->lua),rbin);
	dlg->lexr=wtk_lexr_new(&(cfg->lex.lexr),rbin);
	if(cfg->use_wrdvec)
	{
		dlg->wrdvec=wtk_wrdvec_new(&(cfg->wrdvec),rbin);
		if (dlg->faq){
			wtk_faq_set_wrdvec(dlg->faq,dlg->wrdvec);
		}
		wtk_lexr_set_wrdvec(dlg->lexr,dlg->wrdvec);
	}else
	{
		dlg->wrdvec=NULL;
	}
	if(cfg->use_nlpemot)
	{
		dlg->nlpemot=wtk_nlpemot_new(&(cfg->nlpemot),rbin,dlg->lexr);
	}else
	{
		dlg->nlpemot=NULL;
	}
	if(cfg->dat_fn)
	{
		dlg->db=wtk_sqlite_new(cfg->dat_fn);
	}else
	{
		dlg->db=NULL;
	}
	dlg->buf=wtk_strbuf_new(256,1);
	dlg->glb_heap=wtk_heap_new(4096);
	dlg->loc_heap=wtk_heap_new(4096);
	dlg->fld_hash=wtk_str_hash_new(cfg->fld_q.length*2+1);
	//得到设置lua的函数
	wtk_act_lua_link(dlg->lua);
	wtk_lua_semfld_link(dlg->lua);
	wtk_kgr_link_lua(dlg->lua);
	//设置request
	if (dlg->cfg->else_field){
		lua_pushstring(dlg->lua->state, "else_field");
		lua_pushlightuserdata(dlg->lua->state,dlg->cfg->else_field);
		lua_settable(dlg->lua->state,LUA_REGISTRYINDEX);
	}
	dlg->last_fld=NULL;
	dlg->count=0;
	if(cfg->domain.semfi.lexfn)
	{
		dlg->domain=wtk_semfld_new(&(cfg->domain),dlg);
		//wtk_debug("domain bytes is %d\n", wtk_heap_bytes(dlg->domain->cfg->semfi.net->heap));
	}else
	{
		dlg->domain=NULL;
	}
	dlg->json=wtk_json_new();
	dlg->output=NULL;
	dlg->input=NULL;
	dlg->nxt_fld=NULL;
	dlg->cur_fld=NULL;
	dlg->syn_fld=NULL;
	dlg->json_parser=wtk_json_parser_new();
	dlg->ext_item=NULL;
	dlg->playing=0;
	wtk_queue_init(&(dlg->fld_q));
	wtk_queue_init(&(dlg->hist_q));
	wtk_semdlg_update_flds(dlg);
	dlg->pre_fld=wtk_semdlg_get_fld(dlg,cfg->pre_fld.data,cfg->pre_fld.len);
	dlg->post_fld=wtk_semdlg_get_fld(dlg,cfg->post_fld.data,cfg->post_fld.len);
	dlg->bg_fld=wtk_semdlg_get_fld(dlg,cfg->bg_fld.data,cfg->bg_fld.len);
	dlg->interupt_fld=wtk_semdlg_get_fld(dlg,cfg->interrupt_fld.data,cfg->interrupt_fld.len);
	dlg->msg_func=NULL;
	dlg->msg_ths=NULL;
	dlg->conf=0;
	dlg->last_input=wtk_strbuf_new(256,1);
	dlg->last_output=wtk_strbuf_new(256,1);
	dlg->use_faq=0;
	dlg->keep_fld=0;
	dlg->faq_get_ths=NULL;
	dlg->faq_get=NULL;
	dlg->output_filter_ths=NULL;
	dlg->output_filter=NULL;
	if(cfg->semfst_net)
	{
		dlg->semfst=wtk_semfstr_new(&(cfg->semfst),cfg->semfst_net,dlg->lexr,dlg->lua);
		dlg->semfst->lua_hook=dlg;
	}else
	{
		dlg->semfst=NULL;
	}
	dlg->env_parser=wtk_cfg_file_new();
	dlg->nxt_json_fld=NULL;
	dlg->nxt_json_func=wtk_strbuf_new(256,1);
	wtk_semdlg_reset(dlg);
	if(dlg->cfg->lua_init_func)
	{
		wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_init_func,dlg->buf,NULL);
	}
	if(dlg->cfg->lua_robot_msg_func)
	{
		dlg->robot_route=wtk_thread2_new(dlg,
				(wtk_thread2_start_f)wtk_semdlg_robot_start,
				NULL,
				(wtk_thread2_process_f)wtk_semdlg_process_robot,
				NULL);
		dlg->robot_lua=wtk_lua2_new(&(cfg->robot_lua),rbin);
		wtk_lua_semfld_link_wtk(dlg->robot_lua);
		dlg->robot_buf=wtk_strbuf_new(256,1);
		wtk_thread2_start(dlg->robot_route);
		wtk_thread2_msg_start(dlg->robot_route);
	}else
	{
		dlg->robot_buf=NULL;
		dlg->robot_route=NULL;
		dlg->robot_lua=NULL;
	}
	//wtk_debug("dlg=%p/%p\n",dlg,dlg->robot_route);
	//wtk_debug("new dlg=%p\n",dlg);
end:
	if(ret == -1) {

	}
	return dlg;
}

void wtk_semdlg_set_random(wtk_semdlg_t *dlg,int random)
{
	wtk_queue_node_t *qn;
	wtk_semfld_t *fld;

	for(qn=dlg->fld_q.pop;qn;qn=qn->next)
	{
		fld=data_offset2(qn,wtk_semfld_t,q_n);
		fld->cfg->kg.use_random=random;
	}
}

void wtk_semdlg_delete(wtk_semdlg_t *dlg)
{
	wtk_queue_node_t *qn;
	wtk_semfld_t *fld;

	wtk_cfg_file_delete(dlg->env_parser);
	wtk_strbuf_delete(dlg->last_output);
	if(dlg->nlpemot)
	{
		wtk_nlpemot_delete(dlg->nlpemot);
	}
	if(dlg->str_output)
	{
		wtk_strbuf_delete(dlg->str_output);
	}
	if(dlg->faqc)
	{
		wtk_faqc_delete(dlg->faqc);
	}
	if(dlg->kgkv)
	{
		wtk_kgkv_delete(dlg->kgkv);
	}
	if(dlg->chnlike)
	{
		wtk_chnlike_delete(dlg->chnlike);
	}
	if(dlg->robot_route)
	{
		wtk_thread2_stop(dlg->robot_route);
		wtk_thread2_delete(dlg->robot_route);
		wtk_strbuf_delete(dlg->robot_buf);
	}
	if(dlg->robot_lua)
	{
		wtk_lua2_delete(dlg->robot_lua);
	}
	if(dlg->wrdvec)
	{
		wtk_wrdvec_delete(dlg->wrdvec);
	}
	if(dlg->db)
	{
		wtk_sqlite_delete(dlg->db);
	}
	while(1)
	{
		qn=wtk_queue_pop(&(dlg->fld_q));
		if(!qn){break;}
		fld=data_offset2(qn,wtk_semfld_t,q_n);
		wtk_semfld_delete(fld);
	}
	if(dlg->semfst)
	{
		wtk_semfstr_delete(dlg->semfst);
	}
	wtk_strbuf_delete(dlg->nxt_json_func);
	wtk_strbuf_delete(dlg->last_input);
	wtk_jsonkv_delete(dlg->jsonkv);
	wtk_json_parser_delete(dlg->json_parser);
	if(dlg->faq)
	{
		wtk_faq_delete(dlg->faq);
	}
	wtk_json_delete(dlg->json);
	wtk_str_hash_delete(dlg->fld_hash);
	wtk_lua2_delete(dlg->lua);
	wtk_heap_delete(dlg->glb_heap);
	wtk_heap_delete(dlg->loc_heap);
	wtk_strbuf_delete(dlg->buf);
	if(dlg->domain)
	{
		wtk_semfld_delete(dlg->domain);
	}
	wtk_lexr_delete(dlg->lexr);
	wtk_free(dlg);
}

void wtk_semdlg_reset_history(wtk_semdlg_t *dlg)
{
	//wtk_debug("reset history\n");
	dlg->nxt_json_fld=NULL;
	wtk_strbuf_reset(dlg->nxt_json_func);
	dlg->count=0;
	if(dlg->last_fld)
	{
		dlg->last_fld->is_finish=1;
		wtk_semfld_reset(dlg->last_fld);
	}
	if(dlg->pre_fld)
	{
		dlg->pre_fld->is_finish=1;
		wtk_semfld_reset(dlg->pre_fld);
	}
	wtk_semdlg_set_next_fld(dlg,NULL);
	wtk_semdlg_set_last_fld(dlg,NULL);
	if(dlg->cfg->lua_reset)
	{
		wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_reset,dlg->buf,NULL);
	}
}

void wtk_semdlg_reset(wtk_semdlg_t *dlg)
{
	//wtk_strbuf_reset(dlg->last_input);
	dlg->ext_item=NULL;
	if(dlg->cur_fld)
	{
		wtk_semfld_reset(dlg->cur_fld);
	}
	if(dlg->pre_fld)
	{
		wtk_semfld_reset(dlg->pre_fld);
	}
	wtk_json_reset(dlg->json);
	dlg->input=NULL;
	dlg->output=NULL;
	dlg->cur_fld=NULL;
	dlg->out=0;
	dlg->vad_time=0;
	wtk_heap_reset(dlg->loc_heap);
	wtk_lexr_reset(dlg->lexr);
}

void wtk_semdlg_update_env(wtk_semdlg_t *dlg)
{
	wtk_queue_node_t *qn;
	wtk_semfld_t *fld;

	for(qn=dlg->fld_q.pop;qn;qn=qn->next)
	{
		fld=data_offset2(qn,wtk_semfld_t,q_n);
		wtk_semfld_update_env(fld);
	}
}

void wtk_semdlg_set_env(wtk_semdlg_t *dlg,char *env,int bytes)
{
	// wtk_cfg_file_t *cf=r->env_parser;

	// //wtk_debug("[%.*s]\n",bytes,env);
	// wtk_cfg_file_reset(cf);
	// if(bytes>0)
	// {
	// 	wtk_cfg_file_feed(cf,env,bytes);
	// 	wtk_semdlg_env_reset(&(r->env));
	// 	if(cf->main)
	// 	{
	// 		wtk_semdlg_env_update_local(&(r->env),cf->main);
	// 		wtk_semdlg_update_env(r);
	// 	}
	// }
	wtk_json_reset(dlg->env);
    wtk_json_parser_reset(dlg->json_parser);
    wtk_json_parser_parse2(dlg->json_parser, dlg->env, env, bytes);
	return;
	return;
}

wtk_semfld_t* wtk_semdlg_get_fld(wtk_semdlg_t *dlg,char *data,int bytes)
{
	return (wtk_semfld_t*)wtk_str_hash_find(dlg->fld_hash,data,bytes);
}

wtk_semfld_t* wtk_semdlg_get_fld2(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_semfld_t *fld=NULL;
	wtk_string_t *domain;
	int ret;

	if(dlg->syn_fld)
	{
		//wtk_debug("checvk1111111 ...........\n");
		fld=dlg->syn_fld;
		dlg->syn_fld=NULL;
		goto end;
	}
	if(dlg->nxt_fld)
	{
		//wtk_debug("checvk222222222222 ...........\n");
		fld=dlg->nxt_fld;
		dlg->nxt_fld=NULL;
		goto end;
	}
	if(!dlg->domain)
	{
		goto end;
	}
	//wtk_debug("checvk3333333333 ...........\n");
	ret=wtk_semfld_process(dlg->domain,data,bytes);
	if(ret!=0)
	{
		//wtk_debug("domain process failed\n");
		goto end;
	}
	domain=dlg->domain->output;
	if(!domain)
	{
		//wtk_debug("domain process failed\n");
		goto end;
	}
	dlg->domained=1;
	fld=(wtk_semfld_t*)wtk_str_hash_find(dlg->fld_hash,domain->data,domain->len);

	if(!fld)
	{
		wtk_debug("domain [%.*s] not found.\n",domain->len,domain->data);
		wtk_semfld_reset(dlg->domain); //使用聊天
		return NULL;
		goto end;
	}
	ret=0;
end:
	if(dlg->domain)
	{
		wtk_semfld_reset(dlg->domain);
	}
	//wtk_debug("fld=%p last=%p\n",fld,dlg->last_fld);
	if(!fld)
	{
		fld=dlg->last_fld?dlg->last_fld:NULL;
	}
	//wtk_debug("fld=%p last=%p\n",fld,dlg->last_fld);
	//exit(0);
	return fld;
}

#include "wtk/os/wtk_log.h"

void wtk_semdlg_add_nlpemot_output(wtk_semdlg_t *dlg,char *txt,int len,wtk_json_t *json)
{
	wtk_strbuf_t *buf;
	char *s,*e;
	int n;
	wtk_string_t **strs;
	int i;
	int nslot;
	int b;
	wtk_json_item_t *item,*oi,*ti;

	//wtk_debug("[%.*s]\n",len,txt);
	buf=wtk_strbuf_new(256,1);
	s=txt;e=s+len;
	strs=(wtk_string_t**)(dlg->nlpemot->cfg->sep->slot);
	nslot=dlg->nlpemot->cfg->sep->nslot;
	item=wtk_json_new_array(json);
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		b=0;
		for(i=0;i<nslot;++i)
		{
			if(wtk_string_cmp(strs[i],s,n)==0)
			{
				b=1;
				break;
			}
		}
		wtk_strbuf_push(buf,s,n);
		s+=n;
		if(b || s>=e)
		{
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			wtk_nlpemot_process(dlg->nlpemot,buf->data,buf->pos);
			//wtk_nlpemot_vec_print(&(dlg->nlpemot->vec));
			oi=wtk_json_new_object(json);
			wtk_json_obj_add_str2_s(json,oi,"text",buf->data,buf->pos);
			ti=NULL;
			if(dlg->nlpemot->type->pos>0)
			{
				ti=wtk_json_new_object(json);
				wtk_json_obj_add_str2_s(json,ti,"t",dlg->nlpemot->type->data,dlg->nlpemot->type->pos);
				wtk_json_obj_add_ref_number_s(json,ti,"v",dlg->nlpemot->value);
			}
			if(ti)
			{
				wtk_json_obj_add_item2_s(json,oi,"emotion",ti);
			}
			wtk_json_array_add_item(json,item,oi);
			wtk_strbuf_reset(buf);
		}
	}
	//wtk_json_item_print3(item);
	if(item->v.array->length>0)
	{
		wtk_json_obj_add_item2_s(json,dlg->output,"output2",item);
	}
	//wtk_json_item_print3(dlg->output);
	wtk_strbuf_delete(buf);
	//exit(0);
}

void wtk_semdlg_add_faq_id(wtk_semdlg_t *dlg,wtk_string_t *v,wtk_json_t *json,wtk_json_item_t *item)
{
	char *s;
	char *e;

	s=v->data+2;
	e=v->data+v->len;
	while(s<e)
	{
		if(*s==')')
		{
			++s;
			break;
		}
		++s;
	}
	if(s!=v->data)
	{
		v->data+=2;
		//wtk_debug("[%.*s]\n",(int)(s-v.data-1),v.data);
		wtk_json_obj_add_str2_s(json,item,"id",v->data,(int)(s-v->data-1));
		v->data=s;
		v->len=e-s;
		//wtk_debug("[%.*s]\n",v.len,v.data);
		//exit(0);
	}
}

void wtk_semdlg_output(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_json_t *json=(dlg->json);
	wtk_json_item_t *item,*vi;
	wtk_semfld_t *fld;
	wtk_queue_node_t *qn;
	wtk_semslot_item_t *si;
	wtk_string_t *k;
	wtk_string_t *output=NULL;
	wtk_string_t v;
	int use_fld=0;
	wtk_string_t v2;
	float faqc_conf;

	if(dlg->out)
	{
		return;
	}
	dlg->out=1;
	if(dlg->output)
	{
		item=dlg->output;
	}else
	{
		item=wtk_json_new_object(json);
		dlg->output=item;
	}
	if(data && bytes>0)
	{
		wtk_json_obj_add_str2_s(json,item,"input",data,bytes);
	}else{
		wtk_json_obj_add_str2_s(json,item,"input",NULL,0);
	}
	if(dlg->pre_fld && dlg->pre_fld->output)
	{
		output=dlg->pre_fld->output;
		fld=dlg->pre_fld;
		use_fld=1;
		goto end;
	}
	dlg->use_faq=0;
	fld=dlg->cur_fld;
	if(dlg->str_output->pos>0)
	{
		wtk_string_set(&(v),dlg->str_output->data,dlg->str_output->pos);
		output=&(v);
		goto end;
	}
	if(dlg->faqc)
	{
		faqc_conf=0;
		v=wtk_faqc_get_query2(dlg->faqc, &faqc_conf);
		if(v.len>0 && faqc_conf > dlg->faqc->cfg->conf)
		{
			output=&(v);
			goto end;
		}
	}
	if(fld && fld->output && fld->output->len > 0)
	{
		output=fld->output;
		use_fld=1;
	}else
	{
		dlg->use_faq=1;
		if(dlg->post_fld)
		{
			wtk_semfld_process(dlg->post_fld,dlg->input->data,dlg->input->len);
			//wtk_debug("[%p]\n",dlg->post_fld->output);
			if(dlg->post_fld->output && dlg->post_fld->output->len > 0)
			{
				output=dlg->post_fld->output;
				fld=dlg->post_fld;
				use_fld=1;
			}
		}
		if(!output && dlg->semfst)
		{
			if(dlg->cfg->debug)
			{
				wtk_debug("=== process semfst\n");
			}
			v=wtk_semfstr_process(dlg->semfst,dlg->input->data,dlg->input->len);
			if(v.len>0 && v.data[0]!='/')
			{
				vi=wtk_json_item_dup(dlg->semfst->act_req,json->heap);
				wtk_json_obj_add_item2_s(json,item,"semfst",vi);
				output=&(v);
			}
			wtk_semfstr_reset(dlg->semfst);
		}
		if(!output || output->len <= 0)
		{
			if(dlg->faq_get && dlg->input && dlg->input->len>0)
			{
				v=dlg->faq_get(dlg->faq_get_ths,dlg->input->data,dlg->input->len);
			}else
			{
				wtk_string_set(&(v),0,0);
			}
			if(v.len<=0 && dlg->faq)
			{
				v=wtk_faq_get(dlg->faq,data,bytes);
				if(dlg->cfg->debug)
				{
					wtk_debug("faq=[%.*s]\n",v.len,v.data);
				}
				//$(40bcttpvkhhs) 力气超级大，可是死得有点搞笑，自己把自己锤死了
				if(v.len>3 && v.data[0]=='$' && v.data[1]=='(')
				{
					wtk_semdlg_add_faq_id(dlg,&v,json,item);
				}
				///wtk_debug("prob=%f\n",dlg->faq->v.vecfaq->prob);
				wtk_json_obj_add_ref_number_s(json,item,"ctx",0);
			}
			//wtk_debug("[%.*s]\n",v.len,v.data);
			if(v.len>0)
			{
				output=&(v);
			}else
			{
				if(dlg->cfg->failed_output.len>0)
				{
					output=&(dlg->cfg->failed_output);
				}else
				{
					output=NULL;
				}
			}
		}
	}
	if(!output || output->len<=0)
	{
		//wtk_debug("emtpyt=%s\n",dlg->cfg->lua_on_empty_output);
		if(dlg->cfg->lua_on_empty_output)
		{
			wtk_lua2_arg_t arg[2];

			arg[0].type=WTK_LUA2_THS;
			arg[0].v.ths=dlg;
			wtk_strbuf_reset(dlg->buf);
			//wtk_debug("emtpyt=%s\n",dlg->cfg->lua_on_empty_output);
			wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_on_empty_output,dlg->buf,arg,NULL);
			wtk_string_set(&(v),dlg->buf->data,dlg->buf->pos);
			output=&(v);
		}else
		{
			output=&(dlg->cfg->empty_output);
		}
	}
end:
	if(dlg->output_filter && output && output->len>0)
	{
		v2=dlg->output_filter(dlg->output_filter_ths,output->data,output->len);
		if(v2.len>0)
		{
			//wtk_debug("output is %d [%.*s]\n",v.len,v.len,v.data)
			output=&v2;
		}
	}
	if(fld && fld->kg && fld->kg->fst && fld->kg->fst->cur_state)
	{
		wtk_json_obj_add_str2_s(json,item,"kg",fld->kg->fst->cur_state->name->data,fld->kg->fst->cur_state->name->len);
	}
	if(output)
	{
		//wtk_debug("[%.*s]\n",output->len,output->data);
		wtk_strbuf_reset(dlg->last_output);
		wtk_strbuf_push(dlg->last_output,output->data,output->len);
		wtk_json_obj_add_str2_s(json,item,"output",output->data,output->len);
		if(dlg->nlpemot)
		{
			wtk_semdlg_add_nlpemot_output(dlg,output->data,output->len,json);
		}
		if(dlg->has_post_audio)
		{
			wtk_semdlg_syn(dlg,output->data,output->len,1);
		}
	}
	//wtk_json_obj_add_ref_number_s(json,item,"count",dlg->count);
	if(use_fld && fld)
	{
		wtk_json_obj_add_str2_s(json,item,"fld",fld->cfg->name.data,fld->cfg->name.len);
		if(fld->slot->slot_q.length>0)
		{
			vi=wtk_json_new_array(json);
			wtk_json_obj_add_item2_s(json,item,"action",vi);
			wtk_json_array_add_str(json,vi,fld->cfg->name.data,fld->cfg->name.len);
			vi=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,item,fld->cfg->name.data,fld->cfg->name.len,vi);
			for(qn=fld->slot->slot_q.pop;qn;qn=qn->next)
			{
				si=data_offset2(qn,wtk_semslot_item_t,q_n);
				k=wtk_semslot_item_get_key(si);
				//wtk_debug("%.*s\n",k->len,k->data);
				if(k && si->v)
				{
					wtk_json_obj_add_str2(json,vi,k->data,k->len,si->v->data,si->v->len);
				}
			}
		}
		if(fld->fst && fld->fst->cur_state)
		{
			//wtk_debug("=========> output");
			wtk_json_obj_add_str_s(json,item,"state",fld->fst->cur_state->name);
		}
	}
}

void wtk_semdlg_set_next_fld(wtk_semdlg_t *dlg,wtk_semfld_t *next_fld)
{
	dlg->nxt_fld=next_fld;
}

void wtk_semdlg_set_last_fld(wtk_semdlg_t *dlg,wtk_semfld_t *fld)
{
	dlg->last_fld=fld;
}

void wtk_semdlg_semdlg_quit_fld(wtk_semdlg_t *dlg,wtk_semfld_t *fld)
{
	dlg->last_fld = 0;
	dlg->nxt_fld = 0;
	fld->want_add_hist = 0;
}

int wtk_semdlg_re_process(wtk_semdlg_t *dlg)
{
	wtk_string_t *domain;
	wtk_semfld_t *fld;
	int ret;
	wtk_string_t *v;

	v=dlg->input;
	dlg->cur_fld=NULL;
	ret=wtk_semfld_process(dlg->domain,v->data,v->len);
	if(ret!=0)
	{
		goto end;
	}
	ret=-1;
	domain=dlg->domain->output;
	if(!domain){goto end;}
	//wtk_debug("[%.*s]\n",domain->len,domain->data);
	fld=(wtk_semfld_t*)wtk_str_hash_find(dlg->fld_hash,domain->data,domain->len);
	if(!fld)
	{
		//wtk_debug("domain [%.*s] not found.\n",domain->len,domain->data);
		goto end;
	}
	dlg->cur_fld=fld;
	ret=wtk_semfld_process(fld,v->data,v->len);
	if(ret<0){goto end;}
	if(ret==0 && fld->output)
	{
		wtk_semdlg_set_last_fld(dlg, fld);
//		dlg->last_fld=fld;
	}
end:
	wtk_semfld_reset(dlg->domain);
	return ret;
}

void wtk_semdlg_update_lua(wtk_semdlg_t *dlg,char *s,int bytes)
{
	wtk_lua2_load_str(dlg->lua,s,bytes);
}

void wtk_semdlg_remove_history(wtk_semdlg_t *dlg,wtk_semfld_t *fld)
{
	//wtk_debug("====================>semdlg history length = %d.\n",dlg->hist_q.length);
	wtk_queue_remove(&(dlg->hist_q),&(fld->hist_n));
	//wtk_debug("semdlg history length = %d.\n",dlg->hist_q.length);
}

void wtk_semdlg_add_history(wtk_semdlg_t *dlg,wtk_semfld_t *fld)
{
	wtk_queue_t *q=&(dlg->hist_q);
	wtk_queue_node_t *qn;

	if(dlg->cfg->nhistory<=1){return;}
	if(fld->remember)
	{
		if(q->push && q->push==&(fld->hist_n))
		{

		}else
		{
			wtk_queue_remove(q,&(fld->hist_n));
			wtk_queue_push(q,&(fld->hist_n));
		}
	}else
	{
		wtk_queue_push(q,&(fld->hist_n));//所有的历史领域加入之前并没有进行更新
		fld->remember=1;
		while(q->length>dlg->cfg->nhistory)
		{
			qn=wtk_queue_pop(q);
			if(!qn){break;}
			fld=data_offset2(qn,wtk_semfld_t,hist_n);
			fld->remember=0;
			wtk_semfld_set_end(fld);
			wtk_semfld_reset(fld);
		}
	}
}

int wtk_semdlg_process_history(wtk_semdlg_t *dlg,wtk_string_t *v,wtk_semfld_t *fld)
{
	wtk_queue_t *q=&(dlg->hist_q);
	wtk_queue_node_t *qn;
	wtk_semfld_t *fldx;
	int ret=1;

	if(dlg->cfg->nhistory<=1)
	{
		wtk_semfld_set_end(fld);
		wtk_semfld_reset(fld);
		return ret;
	}
	for(qn=q->push;qn;qn=qn->prev)
	{
		fldx=data_offset2(qn,wtk_semfld_t,hist_n);
		if(dlg->cfg->debug)
		{
			wtk_debug("use history domain [%d:%.*s]\n",q->length,fldx->cfg->name.len,fldx->cfg->name.data);
		}
		if(fldx==fld || fldx->cfg->nlg_close.len>0)
		{
			continue;
		}
		ret=wtk_semfld_process(fldx,v->data,v->len);
		//wtk_debug("===============> ret=%d\n",ret);
		if(ret==0)
		{
			//wtk_debug("output=%p\n",fld->output);
			if(fldx->output)
			{
				dlg->cur_fld=fldx;
//				dlg->last_fld=fldx;
				wtk_semdlg_set_last_fld(dlg, fldx);
				wtk_semdlg_add_history(dlg,fldx);
			}
			break;
		}
	}
	return ret;
}

//让外部掉用的接口函数
wtk_string_t wtk_semdlg_process2(wtk_semdlg_t *dlg,char *data,int bytes,int use_json)
{
	wtk_string_t v;

	if(use_json)
	{
		wtk_semdlg_feed_json(dlg,data,bytes);
	}else
	{
		wtk_semdlg_process(dlg,data,bytes);
	}
	v=wtk_semdlg_get_result(dlg);
	wtk_semdlg_reset(dlg);
	return v;
}

int wtk_semdlg_process(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_strbuf_t *buf;
	wtk_string_t *v;
	wtk_heap_t *heap=dlg->loc_heap;
	int ret=-1;
	wtk_semfld_t *fld;

	++dlg->count;
	dlg->has_post_audio=0;
	dlg->domained=0;
	dlg->output=NULL;
	buf=dlg->buf;
	dlg->cur_fld=NULL;
	wtk_strbuf_reset(dlg->str_output);
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_skip_utf8_ws(buf,data,bytes);
	if(dlg->cfg->debug)
	{
		wtk_debug("pre=%p next=%p\n",dlg->pre_fld,dlg->nxt_fld);
	}
	dlg->input=v=wtk_heap_dup_string(heap,buf->data,buf->pos);

	if(dlg->faqc)
	{
		wtk_faqc_query_start(dlg->faqc,dlg->input->data,dlg->input->len);
	}
	if(dlg->pre_fld)
	{
		if(dlg->input->len==0)
		{
			ret=0;
			goto end;
		}
		wtk_semfld_process(dlg->pre_fld,dlg->input->data,dlg->input->len);
		if(dlg->pre_fld->output || dlg->str_output->pos>0)
		{
			ret=0;
			goto end;
		}
	}
	if(dlg->cfg->lua_process_init_func)
	{
		wtk_lua2_arg_t arg[2];

		arg[0].type=WTK_LUA2_THS;
		arg[0].v.ths=dlg;
		arg[1].type=WTK_LUA2_STRING;
		arg[1].v.str=*(dlg->input);
		wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_process_init_func,dlg->buf,arg,arg+1,NULL);
		if(dlg->str_output->pos>0)
		{
			goto end;
		}
	}
	fld=wtk_semdlg_get_fld2(dlg,v->data,v->len);
	if(!fld)
	{
		if(dlg->cfg->debug)
		{
			wtk_debug("get fld failed\n");
		}
		goto end;
	}
	v=dlg->input;
	dlg->cur_fld=fld;
	ret=wtk_semfld_process(fld,v->data,v->len);
	if(ret<0 || dlg->keep_fld)
	{
		goto end;
	}
	if(ret==1)
	{
		if(dlg->cfg->nhistory>0)
		{
			ret=wtk_semdlg_process_history(dlg,v,fld);
		}
		if(ret==1)
		{
			ret=wtk_semdlg_re_process(dlg);
		}
	}else
	{
		if(fld && fld->want_add_hist && fld->output)
		{
			wtk_semdlg_add_history(dlg,fld);
			wtk_semdlg_set_last_fld(dlg, dlg->cur_fld);
		}
		ret=0;
	}
end:
	wtk_semdlg_output(dlg,data,bytes);
	if(ret==0 && dlg->last_fld)
	{
		wtk_semfld_touch_end(dlg->last_fld);
	}
	if(dlg->input)
	{
		wtk_strbuf_reset(dlg->last_input);
		wtk_strbuf_push(dlg->last_input,data,bytes);
	}
	return ret;
}

wtk_string_t wtk_semdlg_get_choose_rec(wtk_semdlg_t *dlg,char *json,int len)
{
	wtk_json_parser_t *parser=dlg->json_parser;
	wtk_json_item_t *man,*item;
	wtk_string_t v;
	int ret;
	wtk_strbuf_t *buf=dlg->buf;

	//wtk_debug("dlg=%p [%.*s]\n",dlg,bytes,data);
	wtk_strbuf_reset(buf);
	wtk_string_set(&(v),0,0);
	wtk_json_parser_reset(parser);
	ret=wtk_json_parser_parse(parser,json,len);
	if(ret!=0){goto end;}
	man=parser->json->main;
	if(dlg->cfg->lua_rec_choose_func)
	{
		wtk_lua2_arg_t arg[2];

		arg[0].type=WTK_LUA2_THS;
		arg[0].v.ths=dlg;
		arg[1].type=WTK_LUA2_THS;
		arg[1].v.ths=parser->json;
		wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_rec_choose_func,buf,arg,arg+1,NULL);
		if(buf->pos>0)
		{
			wtk_string_set(&(v),buf->data,buf->pos);
			goto end;
		}
	}
	item=wtk_json_obj_get_s(man,"rec");
	if(item && item->type==WTK_JSON_STRING && item->v.str)
	{
		wtk_strbuf_push(buf,item->v.str->data,item->v.str->len);
		wtk_string_set(&(v),buf->data,buf->pos);
	}else
	{
		wtk_string_set(&(v),0,0);
	}
end:
	wtk_json_parser_reset(parser);
	return v;
}

void wtk_semdlg_set_next_json_handler(wtk_semdlg_t *dlg,wtk_semfld_t *fld,wtk_string_t *f)
{
	dlg->nxt_json_fld=fld;
	wtk_strbuf_reset(dlg->nxt_json_func);
	if(f)
	{
		wtk_strbuf_push(dlg->nxt_json_func,f->data,f->len);
		wtk_strbuf_push_c(dlg->nxt_json_func,0);
	}
}

int wtk_semdlg_feed_json(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_json_parser_t *parser=dlg->json_parser;
	wtk_json_item_t *man,*item;
	wtk_json_item_t *subitem;
	wtk_semfld_t *fld;
	wtk_string_t v;
	wtk_faqc_t *faqc;
	int state;
	int ret;

	//wtk_debug("dlg=%p [%.*s]\n",dlg,bytes,data);
	if(dlg->nxt_json_fld && dlg->nxt_json_func->pos>0)
	{
		wtk_lua2_arg_t arg[3];

		arg[0].type=WTK_LUA2_THS;
		arg[0].v.ths=dlg;
		arg[1].type=WTK_LUA2_THS;
		arg[1].v.ths=dlg->nxt_json_fld;
		arg[2].type=WTK_LUA2_STRING;
		wtk_string_set(&(arg[2].v.str),data,bytes);
		dlg->cur_fld=dlg->nxt_json_fld;
		wtk_lua2_process_arg(dlg->lua,dlg->nxt_json_func->data,dlg->buf,arg,arg+1,arg+2,NULL);
		faqc=dlg->faqc;
		dlg->faqc=NULL;
		wtk_semdlg_output(dlg,NULL,0);
		dlg->faqc=faqc;
		ret=0;
	}else
	{
		wtk_json_parser_reset(parser);
		ret=wtk_json_parser_parse(parser,data,bytes);
		if(ret!=0){goto end;}
		man=parser->json->main;
		if(!man){ret=-1;goto end;}
		item=wtk_json_obj_get_s(man,"syn");
		if(item && item->type==WTK_JSON_OBJECT)
		{
			state=1;
			subitem=wtk_json_obj_get_s(item,"fld");
			if(subitem && subitem->type==WTK_JSON_STRING)
			{
				if(wtk_string_cmp_s(subitem->v.str,".usr")==0)
				{
					subitem=wtk_json_obj_get_s(item,"state");
					if(subitem && subitem->type==WTK_JSON_NUMBER && subitem->v.number==0)
					{
						state=0;
					}
				}else{
					fld=wtk_semdlg_get_fld(dlg,subitem->v.str->data,subitem->v.str->len);
					if(fld)
					{
						dlg->syn_fld=fld;
						fld->is_finish=1;
						wtk_semfld_reset(fld);
						subitem=wtk_json_obj_get_s(item,"state");
						if(subitem && subitem->type==WTK_JSON_STRING)
						{
							wtk_semfld_set_fst_state(fld,subitem->v.str->data,subitem->v.str->len);
						}
					}
				}
			}
			if(dlg->cfg->lua_syn_semdlg_state && dlg->pre_fld)
			{
				wtk_lua2_arg_t arg[3];
				arg[0].type=WTK_LUA2_THS;
				arg[0].v.ths=dlg;
				arg[1].type=WTK_LUA2_THS;
				arg[1].v.ths=dlg->pre_fld;
				arg[2].type=WTK_LUA2_NUMBER;
				arg[2].v.number=state;
				wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_syn_semdlg_state,dlg->buf,arg,arg+1,arg+2,NULL);
			}
		}
		if(dlg->cfg->lua_feed_json_func)//现在是得到识别结果
		{
			wtk_lua2_arg_t arg[2];

			arg[0].type=WTK_LUA2_THS;
			arg[0].v.ths=dlg;
			arg[1].type=WTK_LUA2_THS;
			arg[1].v.ths=parser->json;
			wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_feed_json_func,dlg->buf,arg,arg+1,NULL);
		}
		item=wtk_json_obj_get_s(man,"conf");
		if(item && item->type==WTK_JSON_NUMBER)
		{
			dlg->conf=item->v.number;
		}else
		{
			dlg->conf=0;
		}
		item=wtk_json_obj_get_s(man,"vadtime");
		if(item && item->type==WTK_JSON_NUMBER)
		{
			dlg->vad_time=item->v.number;
		}else
		{
			dlg->vad_time=0;
		}
		item=wtk_json_obj_get_s(man,"err");
		//wtk_debug("item=%p\n",item);
		if(item && item->v.str)
		{
			wtk_json_t *json=(dlg->json);
			wtk_json_item_t *vi;

			vi=wtk_json_new_object(json);
			dlg->output=vi;
			wtk_json_obj_add_str2_s(json,vi,"output",item->v.str->data,item->v.str->len);
			//wtk_debug("[%.*s]\n",item->v.str->len,item->v.str->data);
			//wtk_debug("add output\n");
		}else
		{
			item=wtk_json_obj_get_s(man,"rec");
			if(item  && item->v.str)
			{
				v=*item->v.str;
			}else
			{
				wtk_string_set(&(v),0,0);
			}
			//wtk_debug("[%.*s]\n",v.len,v.data);
			dlg->ext_item=wtk_json_obj_get_s(man,"ext");
			ret=wtk_semdlg_process(dlg,v.data,v.len);
		}
	}
end:
	return ret;
}

void wtk_semdlg_output_talking(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_json_t *json=(dlg->json);
	wtk_json_item_t *item,*vi;
	wtk_semfld_t *fld;
	wtk_queue_node_t *qn;
	wtk_semslot_item_t *si;
	wtk_string_t *k;

	if(dlg->out)
	{
		return;
	}
	dlg->out=1;
	if(dlg->output)
	{
		item=dlg->output;
	}else
	{
		item=wtk_json_new_object(json);
		dlg->output=item;
	}
	wtk_json_obj_add_str2_s(json,item,"input",data,bytes);
	fld=dlg->interupt_fld;
	if(fld)
	{
		wtk_json_obj_add_str2_s(json,item,"fld",fld->cfg->name.data,fld->cfg->name.len);
		if(fld->slot->slot_q.length>0)
		{
			vi=wtk_json_new_array(json);
			wtk_json_obj_add_item2_s(json,item,"action",vi);
			wtk_json_array_add_str(json,vi,fld->cfg->name.data,fld->cfg->name.len);
			vi=wtk_json_new_object(json);
			wtk_json_obj_add_item2(json,item,fld->cfg->name.data,fld->cfg->name.len,vi);
			for(qn=fld->slot->slot_q.pop;qn;qn=qn->next)
			{
				si=data_offset2(qn,wtk_semslot_item_t,q_n);
				k=wtk_semslot_item_get_key(si);
				if(k && si->v)
				{
					wtk_json_obj_add_str2(json,vi,k->data,k->len,si->v->data,si->v->len);
				}
			}
		}
		if(fld->fst && fld->fst->cur_state)
		{
			//wtk_debug("=========> output");
			wtk_json_obj_add_str_s(json,item,"state",fld->fst->cur_state->name);
		}
	}
}

int wtk_semdlg_process_talking(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_strbuf_t *buf;
	wtk_string_t *v;
	wtk_heap_t *heap=dlg->loc_heap;
	int ret=-1;

	buf=dlg->buf;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_skip_utf8_ws(buf,data,bytes);
	dlg->input=v=wtk_heap_dup_string(heap,buf->data,buf->pos);
	if(dlg->interupt_fld)
	{
		ret=wtk_semfld_process(dlg->interupt_fld,v->data,v->len);
	}
	wtk_semdlg_output_talking(dlg,data,bytes);
	return ret;
}

int wtk_semdlg_feed_talking_json(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_json_parser_t *parser=dlg->json_parser;
	wtk_json_item_t *man,*item;
	wtk_string_t v;
	int ret;

	//wtk_debug("dlg=%p [%.*s]\n",dlg,bytes,data);
	wtk_json_parser_reset(parser);
	ret=wtk_json_parser_parse(parser,data,bytes);
	if(ret!=0){goto end;}
	man=parser->json->main;
	item=wtk_json_obj_get_s(man,"conf");
	if(item && item->type==WTK_JSON_NUMBER)
	{
		dlg->conf=item->v.number;
	}else
	{
		dlg->conf=0;
	}
	item=wtk_json_obj_get_s(man,"vadtime");
	if(item && item->type==WTK_JSON_NUMBER)
	{
		dlg->vad_time=item->v.number;
	}else
	{
		dlg->vad_time=0;
	}
	item=wtk_json_obj_get_s(man,"rec");
	if(item  && item->v.str)
	{
		v=*item->v.str;
	}else
	{
		wtk_string_set(&(v),0,0);
	}
	//wtk_debug("[%.*s]\n",v.len,v.data);
	dlg->ext_item=wtk_json_obj_get_s(man,"ext");
	ret=wtk_semdlg_process_talking(dlg,v.data,v.len);
end:
	return ret;
}


void wtk_semdlg_output2(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_json_t *json=(dlg->json);
	wtk_json_item_t *item,*vi;
	wtk_semfld_t *fld;
	wtk_string_t *v;

	item=wtk_json_new_object(json);
	dlg->output=item;
	wtk_json_obj_add_str2_s(json,item,"input",data,bytes);
	fld=dlg->cur_fld;
	if(fld)
	{
		vi=fld->fi->act.v.json.json;
		v=dlg->domain->output;
		if(vi && v)
		{
			vi=wtk_json_item_dup(vi,json->heap);
			//wtk_json_copy_obj_dict(json,item,vi);
			wtk_json_obj_add_item2(json,item,v->data,v->len,vi);
			wtk_json_obj_add_str2_s(json,item,"domain",v->data,v->len);
		}
		//wtk_json_item_print3(vi);
	}
}

int wtk_semdlg_process_fld_slot(wtk_semdlg_t *dlg,char *data,int bytes)
{
	wtk_strbuf_t *buf;
	wtk_string_t *v;
	wtk_heap_t *heap=dlg->loc_heap;
	int ret=-1;
	wtk_semfld_t *fld=NULL;

	//wtk_debug("[%.*s]\n",bytes,data);
	dlg->domained=0;
	buf=dlg->buf;
	dlg->cur_fld=NULL;
	wtk_strbuf_reset(buf);
	wtk_strbuf_push_skip_utf8_ws(buf,data,bytes);
	if(buf->pos<=0)
	{
		dlg->input=NULL;
		goto end;
	}
	dlg->input=v=wtk_heap_dup_string(heap,buf->data,buf->pos);
	v=dlg->input;
	ret=wtk_semfld_process(dlg->domain,v->data,v->len);
	if(ret!=0)
	{
		goto end;
	}
	ret=-1;
	v=dlg->domain->output;
	if(!v){goto end;}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	fld=(wtk_semfld_t*)wtk_str_hash_find(dlg->fld_hash,v->data,v->len);
	if(!fld)
	{
		//wtk_debug("get fld failed\n");
		goto end;
	}
	v=dlg->input;
	//wtk_debug("fld=%d\n",dlg->domained);
	dlg->cur_fld=fld;
	ret=wtk_semfi_process(fld->fi,v->data,v->len);
	//wtk_debug("ret=%d\n",ret);
	if(ret<0)
	{
		goto end;
	}
end:
	wtk_semdlg_output2(dlg,data,bytes);
	if(fld)
	{
		wtk_semfld_set_end(fld);
		wtk_semfld_reset(fld);
	}
	return ret;
}

void wtk_semdlg_print(wtk_semdlg_t *dlg)
{
	wtk_json_item_print3(dlg->output);
}

wtk_string_t wtk_semdlg_get_result(wtk_semdlg_t *dlg)
{
	wtk_strbuf_t *buf=dlg->buf;
	wtk_string_t v;

	wtk_strbuf_reset(buf);
	if(dlg->output)
	{
		wtk_json_item_print(dlg->output,buf);
	}
	wtk_string_set(&(v),buf->data,buf->pos);
	return v;
}

void wtk_semdlg_set_ext_json(wtk_semdlg_t *dlg,char *k,int k_len,wtk_json_item_t *item)
{
	wtk_json_t *json=dlg->json;
	wtk_json_item_t *obj;

	//wtk_json_item_print3(item);
	if(!item)
	{
		wtk_debug("found bug\n");
		return;
	}
	if(!dlg->output)
	{
		dlg->output=wtk_json_new_object(json);
	}
	obj=dlg->output;
	//wtk_json_item_print4(dlg->output);
	wtk_json_obj_add_item2(json,obj,k,k_len,item);
}

void wtk_semdlg_syn(wtk_semdlg_t *dlg,char *v,int v_len,int syn)
{
	wtk_semdlg_sys_msg_t msg;

	//wtk_debug("play %p...\n",dlg->play);
	if(dlg->msg_func)
	{
		msg.type=WTK_SEMDLG_TTS;
		wtk_string_set(&(msg.v),v,v_len);
		msg.syn=syn;
		dlg->msg_func(dlg->msg_ths,&(msg));
	}
}

void wtk_semdlg_play_file(wtk_semdlg_t *dlg,char *v,int v_len,int syn)
{
	wtk_semdlg_sys_msg_t msg;

	if(dlg->msg_func)
	{
		msg.type=WTK_SEMDLG_PLAY_FILE;
		wtk_string_set(&(msg.v),v,v_len);
		msg.syn=syn;
		dlg->msg_func(dlg->msg_ths,&(msg));
	}
}

void wtk_semdlg_set_rec_grammar(wtk_semdlg_t *dlg,char *v,int v_len)
{
	wtk_semdlg_sys_msg_t msg;

	if(dlg->msg_func)
	{
		msg.type=WTK_SEMDLG_REC_SET_GRAMMAR;
		wtk_string_set(&(msg.v),v,v_len);
		msg.syn=1;
		dlg->msg_func(dlg->msg_ths,&(msg));
	}
}

void wtk_semdlg_exe(wtk_semdlg_t *dlg,wtk_json_item_t *item)
{
	wtk_semdlg_sys_msg_t msg;

	wtk_string_set(&(dlg->exe_ret),0,0);
	if(dlg->msg_func)
	{
		msg.type=WTK_SEMDLG_DLG_CMD;
		msg.syn=1;
		msg.cmd=item;
		dlg->msg_func(dlg->msg_ths,&(msg));
	}
	//wtk_debug("[%.*s]\n",dlg->exe_ret.len,dlg->exe_ret.data);
}

void wtk_semdlg_skip_session(wtk_semdlg_t *dlg)
{
	//wtk_debug("set skip session\n");
	wtk_semdlg_set_next_fld(dlg,NULL);
	wtk_semdlg_set_last_fld(dlg,NULL);
}

void wtk_semdlg_set_playing(wtk_semdlg_t *dlg,int talking)
{
	//wtk_debug("playing talking=%d...\n",talking);
	dlg->playing=talking;
}

void wtk_semdlg_feed_timer(wtk_semdlg_t *dlg,char *func,int func_bytes)
{
	if(func)
	{
		wtk_strbuf_t *tx;

		tx=wtk_strbuf_new(256,1);
		wtk_strbuf_push(tx,func,func_bytes);
		wtk_strbuf_push_c(tx,0);
		wtk_lua2_process_arg(dlg->lua,tx->data,dlg->buf,NULL);
		wtk_strbuf_delete(tx);
	}
}

int wtk_semdlg_is_bg(wtk_semdlg_t *dlg)
{
	if(dlg->bg_fld && dlg->nxt_fld==dlg->bg_fld && dlg->keep_fld)
	{
		return 1;
	}
	return 0;
}

void wtk_semdlg_set_output(wtk_semdlg_t *dlg,char *data,int len)
{
	wtk_strbuf_reset(dlg->str_output);
	wtk_strbuf_push(dlg->str_output,data,len);
}

void wtk_semdlg_set_bg(wtk_semdlg_t *dlg,int bg)
{
	if(!dlg->bg_fld)
	{
		return;
	}
	if(bg)
	{
		wtk_semdlg_set_next_fld(dlg,NULL);
		wtk_semdlg_set_last_fld(dlg,NULL);
		dlg->nxt_fld=dlg->bg_fld;
		dlg->keep_fld=1;
	}else
	{
		dlg->keep_fld=0;
		wtk_semdlg_set_next_fld(dlg,NULL);
		wtk_semdlg_set_last_fld(dlg,NULL);
	}
}

void wtk_semdlg_set_fld(wtk_semdlg_t *dlg,char *fld,int fld_bytes)
{
	wtk_semfld_t *f;

	f=wtk_semdlg_get_fld(dlg,fld,fld_bytes);
	wtk_semdlg_set_next_fld(dlg,NULL);
	wtk_semdlg_set_last_fld(dlg,NULL);
	dlg->nxt_fld=f;
	dlg->keep_fld=f?1:0;
}

void wtk_semdlg_set_faq_get(wtk_semdlg_t *dlg,void *ths,wtk_semdlg_get_faq_f faq_get)
{
	dlg->faq_get_ths=ths;
	dlg->faq_get=faq_get;
}

void wtk_semdlg_set_output_filter(wtk_semdlg_t *dlg,void *ths,wtk_semdlg_filter_output_f output_filter)
{
	dlg->output_filter_ths=ths;
	dlg->output_filter=output_filter;
}

void wtk_semdlg_feed_robot_msg(wtk_semdlg_t *dlg,char *msg,int msg_len)
{
	wtk_semdlg_robot_msg_t *x;

	//wtk_debug("dlg=%p msg=[%.*s]\n",dlg->robot_route,msg_len,msg);
	if(dlg->robot_route)
	{
		x=wtk_semdlg_robot_msg_new(msg,msg_len);
		wtk_thread2_msg_process(dlg->robot_route,x);
	}
}

void wtk_semdlg_feed_robot_msg2(wtk_semdlg_t *dlg,char *msg,int msg_len,char* v,int v_len)
{
	wtk_semdlg_robot_msg_t *x;

	//wtk_debug("dlg=%p msg=[%.*s]\n",dlg,msg_len,msg);
	if(dlg->robot_route)
	{
		x=wtk_semdlg_robot_msg_new2(msg,msg_len,v,v_len);
		wtk_thread2_msg_process(dlg->robot_route,x);
	}
}

int wtk_semdlg_process_robot_lua(wtk_semdlg_t *dlg,char *msg,int msg_len,char *v,int v_len)
{
	wtk_lua2_arg_t arg[3];

	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=dlg;
	arg[1].type=WTK_LUA2_STRING;
	wtk_string_set(&(arg[1].v.str),msg,msg_len);
	arg[2].type=WTK_LUA2_STRING;
	wtk_string_set(&(arg[2].v.str),v,v_len);
	//wtk_debug("[%s]=[%.*s]\n",dlg->cfg->lua_robot_msg_func,msg_len,msg);
	return wtk_lua2_process_arg(dlg->robot_lua,dlg->cfg->lua_robot_msg_func,dlg->robot_buf,arg,arg+1,arg+2,NULL);
}

void wtk_semdlg_set_lua_dat(wtk_semdlg_t *dlg,char *k,char *v)
{
	if(dlg->cfg->lua_set_func)
	{
		wtk_lua2_arg_t arg[3];

		arg[0].type=WTK_LUA2_THS;
		arg[0].v.ths=dlg;
		arg[1].type=WTK_LUA2_STRING;
		wtk_string_set(&(arg[1].v.str),k,strlen(k));
		arg[2].type=WTK_LUA2_STRING;
		wtk_string_set(&(arg[2].v.str),v,strlen(v));
		//wtk_debug("[%s]=[%.*s]\n",dlg->cfg->lua_robot_msg_func,msg_len,msg);
		//wtk_debug("set [%s]=[%s]\n",k,v);
		wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_set_func,
				dlg->buf,arg,arg+1,arg+2,NULL);

	}
}

void wtk_semdlg_robot_process_timer(wtk_semdlg_t *dlg,char *func)
{
	wtk_lua2_arg_t arg[2];

	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=dlg;
	wtk_lua2_process_arg(dlg->robot_lua,func,dlg->robot_buf,arg,NULL);
	wtk_free(func);
}

void wtk_semdlg_add_robot_timer(wtk_semdlg_t *dlg,double delay,char *func,int len)
{
	char *t;

	t=wtk_data_to_str(func,len);
	wtk_thread2_add_timer(dlg->robot_route,delay,(wtk_thread2_timer_func_t)wtk_semdlg_robot_process_timer,dlg,t);
}

int wtk_lua_semdlg_add_robot_timer(lua_State *l)
{
	wtk_semdlg_t *sem;
	wtk_string_t v;
	size_t len;
	int i;
	double t;

	i=1;
	if(!lua_isuserdata(l,i)){goto end;}
	sem=(wtk_semdlg_t*)lua_touserdata(l,i);
	++i;
	if(!lua_isnumber(l,i)){goto end;}
	t=lua_tonumber(l,i);
	++i;
	if(!lua_isstring(l,i)){goto end;}
	v.data=(char*)lua_tolstring(l,i,&len);
	v.len=len;
	wtk_semdlg_add_robot_timer(sem,t,v.data,v.len);
end:
	return 0;
}

int wtk_semdlg_robot_start(wtk_semdlg_t *dlg)
{
	wtk_lua2_t *lua=dlg->robot_lua;

	wtk_lua2_link_function(lua,wtk_lua_semdlg_add_robot_timer,"wtk_semdlg_add_robot_timer");
	if(dlg->cfg->lua_robot_init_func)
	{
		//wtk_debug("===================> int\n");
		wtk_lua2_process_arg(lua,dlg->cfg->lua_robot_init_func,dlg->robot_buf,NULL);
		//exit(0);
	}
	return 0;
}


int wtk_semdlg_process_robot(wtk_semdlg_t *dlg,wtk_semdlg_robot_msg_t *msg)
{
	//wtk_debug("msg=[%.*s]\n",msg->msg->len,msg->msg->data);
	wtk_semdlg_process_robot_lua(dlg,msg->msg->data,msg->msg->len,msg->v?msg->v->data:NULL,msg->v?msg->v->len:0);
	wtk_semdlg_robot_msg_delete(msg);
	//wtk_debug("msg process end\n");
	return 0;
}


wtk_string_t wtk_semdlg_flush(wtk_semdlg_t *dlg,wtk_string_t *fld,wtk_string_t *v)
{
	wtk_semfld_t *xfld;
	wtk_string_t vx;
	wtk_act_t *act;
	wtk_strbuf_t *buf=dlg->buf;

	//wtk_debug("get fild[%.*s]\n",fld->len,fld->data);
	wtk_string_set(&(vx),0,0);
	xfld=wtk_semdlg_get_fld(dlg,fld->data,fld->len);
	if(!xfld || !xfld->cfg->lua_flush){goto end;}
	//wtk_debug("[%.*s]=[%s]\n",v->len,v->data,xfld->cfg->lua_flush);
	act=wtk_act_new_str(v->data,v->len);
	//wtk_act_print(act);
	//wtk_debug("flush %s\n",xfld->cfg->lua_flush);
	wtk_semfld_feed_lua(xfld,act,xfld->cfg->lua_flush,NULL,buf);
	wtk_string_set(&(vx),buf->data,buf->pos);
//	wtk_debug("nexgt_fld=%p\n",dlg->nxt_fld);
//	if(!dlg->nxt_fld && vx.len>0)
//	{
//		wtk_debug("set next fild=%p\n",xfld);
//		dlg->nxt_fld=xfld;
//	}
	wtk_act_delete(act);
end:
	//exit(0);
	return vx;
}

wtk_string_t wtk_semdlg_get_usr(wtk_semdlg_t *dlg)
{
	wtk_strbuf_t *buf=dlg->buf;
	wtk_string_t v;

	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_usr_get,buf,NULL);
	wtk_string_set(&(v),buf->data,buf->pos);
	return v;
}

void wtk_semdlg_feed_hint(wtk_semdlg_t *dlg,char *hint,int hint_bytes)
{
	wtk_strbuf_t *buf=dlg->buf;
	wtk_lua2_arg_t arg[3];

	//wtk_debug("[%.*s] %s\n",hint_bytes,hint,dlg->cfg->lua_feed_rec_hint);
	if(!dlg->cfg->lua_feed_rec_hint){return;}
	arg[0].type=WTK_LUA2_THS;
	arg[0].v.ths=dlg;
	arg[1].type=WTK_LUA2_STRING;
	wtk_string_set(&(arg[1].v.str),hint,hint_bytes);
	wtk_strbuf_reset(buf);
	wtk_lua2_process_arg(dlg->lua,dlg->cfg->lua_feed_rec_hint,buf,arg,arg+1,NULL);
}

//根据传入的fld 得到状态,的item
wtk_json_item_t * qtk_semdlg_get_inter_state_item(wtk_semfld_t *fld, wtk_json_t *json)
{
	wtk_json_item_t *item;
	wtk_json_item_t *kg_item;


	item = wtk_json_new_object(json);
	wtk_json_obj_add_str(json, item, "name", strlen("name"), &(fld->cfg->name));

	if(fld->kg){
		kg_item = wtk_json_new_object(json);
		wtk_json_obj_add_str(json, kg_item, "last_item", strlen("last_item"), fld->kg->last->name);
		if( fld->kg->last && fld->kg->last->nlg_net){
			wtk_json_obj_add_str(json, kg_item, "cur_state", strlen("cur_state"), fld->kg->fst->cur_state->name);
			wtk_json_obj_add_ref_number_s(json, kg_item, "cur_state_round", (double)(fld->kg->fst->cur_state_round));
		}
		if(fld->kg->next){
			wtk_json_obj_add_str(json, kg_item, "next_item", strlen("next_item"), fld->kg->next->name);
		}
		wtk_json_obj_add_item2(json, item, "kg", strlen("kg"), kg_item);
	}else if (fld->fst){
		wtk_json_obj_add_str(json, item, "state", strlen("state"), fld->fst->cur_state->name);
		wtk_json_obj_add_ref_number_s(json, item, "cur_state_round", (double)(fld->fst->cur_state_round));
	}

	return item;
}

//放在整体对话系统跑完之后,提取出整个中间状态,
//主要有 syn_fld??, nxt_fld, last_fld, kg:item cur_state, fst cur_state ,history, cur_state_round

int qtk_semdlg_get_inter_state(wtk_semdlg_t *dlg, wtk_strbuf_t* buf)
{
	wtk_semfld_t *fld = NULL;
	wtk_queue_node_t* qn;
	wtk_json_t *json = NULL;
	wtk_json_item_t *item = NULL;
	wtk_json_item_t *array = NULL;
	int ret = 0;

	json = wtk_json_new();
	json->main = wtk_json_new_object(json);


	if(!dlg){
		wtk_debug("dlg is NULL error\n");
		ret = -1;
		goto end;
	}
	if(dlg->syn_fld){
		//wtk_debug("syn_fld is %.*s\n", dlg->syn_fld->cfg->name.len, dlg->syn_fld->cfg->name.data);
		fld = dlg->syn_fld;
		item = qtk_semdlg_get_inter_state_item(fld, json);
		wtk_json_obj_add_item2(json, json->main, "syn_fld", strlen("syn_fld"), item);
	}
	if (dlg->nxt_fld){
		//wtk_debug("nxt_fld is %.*s\n", dlg->nxt_fld->cfg->name.len, dlg->nxt_fld->cfg->name.data);
		fld = dlg->nxt_fld;
		item = qtk_semdlg_get_inter_state_item(fld, json);
		wtk_json_obj_add_item2(json, json->main, "nxt_fld", strlen("nxt_fld"), item);
	}
	if (dlg->pre_fld){
		fld = dlg->pre_fld;
		//wtk_debug("pre fld is %.*s\n", fld->cfg->name.len, fld->cfg->name.data);
		item = qtk_semdlg_get_inter_state_item(fld, json);
		wtk_json_obj_add_item2(json, json->main, "pre_fld", strlen("pre_fld"), item);
	}
	if (dlg->post_fld){
		fld = dlg->post_fld;
		//wtk_debug("post fld is %.*s\n", fld->cfg->name.len, fld->cfg->name.data);
		item = qtk_semdlg_get_inter_state_item(fld, json);
		wtk_json_obj_add_item2(json, json->main, "post_fld", strlen("post_fld"), item);
	}
	if(dlg->last_fld){
		fld = dlg->last_fld;
		//wtk_debug("last fld is %.*s\n", fld->cfg->name.len, fld->cfg->name.data);
		item = qtk_semdlg_get_inter_state_item(fld, json);
		wtk_json_obj_add_item2(json, json->main, "last_fld", strlen("last_fld"), item);
	}
	if (dlg->cfg->nhistory > 0){
		array = wtk_json_new_array(json);
		for(qn=dlg->hist_q.pop;qn;qn = qn->next){	//队列 先入先出 保证前后顺序
			fld = data_offset2(qn, wtk_semfld_t, hist_n);
			//wtk_debug("history fld is %.*s\n", fld->cfg->name.len, fld->cfg->name.data);
			item = qtk_semdlg_get_inter_state_item(fld, json);
			wtk_json_array_add_item(json, array, item);
		}
		wtk_json_obj_add_item2(json, json->main, "history", strlen("history"), array);
	}
	wtk_json_print(json, buf);

end:
	wtk_json_delete(json);
	return ret;
}

//设置领域, 使领域回到以前的状态
wtk_semfld_t* qtk_semdlg_json_set_fld_get(wtk_semdlg_t *dlg, char *str, int len, wtk_json_item_t *item)
{
	wtk_json_item_t *v = NULL, *kg_v = NULL;
	wtk_semfld_t *fld = NULL;

	if(!dlg) {
		wtk_debug("dlg arg is NULL error\n");
		goto end;
	}

	fld = wtk_semdlg_get_fld(dlg, str, len);
	if(!fld) {
		wtk_debug("[%.*s] fld is fild error\n", len, str);
		goto end;
	}
	//先设置kg
	v = wtk_json_obj_get_s(item, "kg");
	if(v && v->type == WTK_JSON_OBJECT) {
		if (!fld->kg){
			wtk_debug("[%.*s] fld dont have kg start\n",len,str);
			fld = NULL;
			goto end;
		}
		kg_v = wtk_json_obj_get_s(v, "last_item");
		if(kg_v && kg_v->type == WTK_JSON_STRING) {
			fld->kg->last = wtk_kg_class_get_item(fld->kg->cfg->kg,fld->kg->cfg->kg->_class,kg_v->v.str->data,kg_v->v.str->len,0);
		}
		kg_v = wtk_json_obj_get_s(v, "cur_state");
		if (kg_v && kg_v->type == WTK_JSON_STRING) {
			//wtk_debug("nlg_net %p\n", fld->kg->cur->nlg_net);
			//wtk_nlgfst2_set_net(fld->kg->fst, fld->kg->cur->nlg_net);
			if (!fld->kg->last || !fld->kg->last->nlg_net){
				wtk_debug("kg fst->last item is lost error\n");
				fld = NULL;
				goto end;
			}
			fld->kg->fst->net = fld->kg->last->nlg_net;
			fld->kg->fst->cur_state = wtk_nlgnet_get_state(fld->kg->fst->net, kg_v->v.str->data, kg_v->v.str->len, 0);//0 不创建新状态
		}
		kg_v = wtk_json_obj_get_s(v, "cur_state_round");
		if (kg_v && kg_v->type == WTK_JSON_NUMBER) {
			fld->kg->fst->cur_state_round = (int)(kg_v->v.number);
		}
		kg_v = wtk_json_obj_get_s(v,"next_item");
		if (kg_v && kg_v->type == WTK_JSON_STRING) {
			fld->kg->next = wtk_kg_class_get_item(fld->kg->cfg->kg,fld->kg->cfg->kg->_class,kg_v->v.str->data,kg_v->v.str->len,0);
		}
		goto end;
	}

	//另外一种可能
	v = wtk_json_obj_get_s(item, "state");
	if(v && v->type == WTK_JSON_STRING) {
		wtk_nlgfst_set_state(fld->fst, v->v.str->data, v->v.str->len);
	}
	v = wtk_json_obj_get_s(item, "cur_state_round");
	if(v && v->type == WTK_JSON_NUMBER) {
		fld->fst->cur_state_round = (int)(v->v.number);
	}

end:

	return fld;
}
//0 success else error
int qtk_semdlg_set_inter_state(wtk_semdlg_t *dlg, char *str, int len)
{
	wtk_json_parser_t* perser = wtk_json_parser_new();
	wtk_json_item_t *man = NULL, *item = NULL, *v = NULL, *history_q = NULL;
	wtk_semfld_t *fld = NULL;
	int ret = 0, i = 0;

	ret = wtk_json_parser_parse(perser,str,len);
	if (ret != 0){
		wtk_debug("=========== err ===========\n");
		wtk_debug("invalid json.\n");
		goto end;
	}

	man = perser->json->main;
	if (!man){
		ret = -1;
		goto end;
	}

	item = wtk_json_obj_get_s(man, "syn_fld");
	if(item && item->type == WTK_JSON_OBJECT) {
		v = wtk_json_obj_get_s(item, "name");
		if(v && v->type == WTK_JSON_STRING) {
			dlg->syn_fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, item);
		}
	}

	item = wtk_json_obj_get_s(man, "nxt_fld");
	if(item && item->type == WTK_JSON_OBJECT) {
		v = wtk_json_obj_get_s(item, "name");
		if(v && v->type == WTK_JSON_STRING) {
			dlg->nxt_fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, item);
		}
	}
	item = wtk_json_obj_get_s(man, "pre_fld");
	if(item && item->type == WTK_JSON_OBJECT) {
		v = wtk_json_obj_get_s(item, "name");
		if(v && v->type == WTK_JSON_STRING) {
			dlg->pre_fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, item);
		}
	}
	item = wtk_json_obj_get_s(man, "post_fld");
	if(item && item->type == WTK_JSON_OBJECT) {
		v = wtk_json_obj_get_s(item, "name");
		if(v && v->type == WTK_JSON_STRING) {
			dlg->post_fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, item);
		}
	}
	item = wtk_json_obj_get_s(man, "last_fld");
	if(item && item->type == WTK_JSON_OBJECT) {
		v = wtk_json_obj_get_s(item, "name");
		if(v && v->type == WTK_JSON_STRING) {
			dlg->last_fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, item);
		}
	}

	item = wtk_json_obj_get_s(man, "history");
	if (item && item->type == WTK_JSON_ARRAY) {
		i = 0;
		while(1) {	//从0开始计数的
			history_q = wtk_json_array_get(item, i);
			if (!history_q) {
				break;
			}
			++i;
			v = wtk_json_obj_get_s(history_q, "name");
			if(v && v->type == WTK_JSON_STRING) {
				fld = qtk_semdlg_json_set_fld_get(dlg, v->v.str->data,v->v.str->len, history_q);
				if(fld) {
					wtk_semdlg_add_history(dlg, fld);
				}
			}
		}
	}

end:

	wtk_json_parser_delete(perser);
	return ret;
}
/*
 * 得到全部的lua局部变量
 * */
int qtk_lua_local_get(wtk_semdlg_t *semdlg, wtk_strbuf_t *buf)
{
	int ret = 0;
	lua_State* L = semdlg->lua->state;
	int len = 0;
	const char* str = NULL;

	if(semdlg->cfg->lua_int_lua_local_get == NULL)
	{
		ret = -1;
		//wtk_debug("error function is unknow\n");
		goto end;
	}

	lua_getglobal(L, semdlg->cfg->lua_int_lua_local_get);

	if (lua_pcall(L, 0, 1, 0)){
		ret = -1;
		wtk_debug("err is %s\n", lua_tostring(L, -1));
		goto end;
	}
	if (!lua_isstring(L, -1)){
		ret = -1;
		goto end;
	}
	len = lua_strlen(L, -1);
	str = lua_tostring(L, -1);
	wtk_strbuf_push(buf, str, len);
end:
	return ret;
}
/*
 * 设置全部的lua中间变量
 * */
int qtk_lua_local_set(wtk_semdlg_t *semdlg, const char *str, int len)
{
	int ret = 0;
	lua_State* L = semdlg->lua->state;

	if(semdlg->cfg->lua_int_lua_local_set == NULL)
	{
		ret = -1;
		//wtk_debug("error function is unknow\n");
		goto end;
	}

	lua_getglobal(L, semdlg->cfg->lua_int_lua_local_set);
	lua_pushlstring(L, str, len);

	//有一个返回值 保证是否成功
	if (lua_pcall(L, 1, 1, 0)){
			ret = -1;
			wtk_debug("err is %s\n", lua_tostring(L, -1));
			goto end;
		}
	if (!lua_isnumber(L, -1)){
			ret = -1;
			goto end;
		}
	ret = lua_tonumber(L, -1);
end:
	return ret;
}
