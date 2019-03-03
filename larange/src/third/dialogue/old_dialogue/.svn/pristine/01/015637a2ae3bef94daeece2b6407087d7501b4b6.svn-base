#include "wtk_semdlg_cfg.h" 

int wtk_semdlg_assist_cfg_init(wtk_semdlg_assist_cfg_t *cfg)
{
	wtk_string_set(&(cfg->name),0,0);
	wtk_string_set(&(cfg->sex),0,0);
	cfg->age=-1;
	return 0;
}

int wtk_semdlg_assist_cfg_clean(wtk_semdlg_assist_cfg_t *cfg)
{
	return 0;
}

int wtk_semdlg_assist_cfg_update_local(wtk_semdlg_assist_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	wtk_local_cfg_update_cfg_string_v(lc,cfg,name,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,sex,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,age,v);
	return 0;
}


int wtk_semdlg_cfg_init(wtk_semdlg_cfg_t *cfg)
{
	wtk_kgkv_cfg_init(&(cfg->kgkv));
	wtk_semdlg_assist_cfg_init(&(cfg->assist));
	wtk_wrdvec_cfg_init(&(cfg->wrdvec));
	wtk_lex_cfg_init(&(cfg->lex));
	wtk_semfld_cfg_init(&(cfg->domain));
	wtk_lua2_cfg_init(&(cfg->lua));
	wtk_faq_cfg_init(&(cfg->faq));
	wtk_queue_init(&(cfg->fld_q));
	wtk_string_set_s(&(cfg->failed_output),"我现在还不知道如何回答。");
	wtk_string_set_s(&(cfg->empty_output),"你在说什么啊?");
	cfg->nhistory=2;
	cfg->grammar_conf_thresh=1.0;
	cfg->debug=0;
	cfg->brain_dn="./brain";
	cfg->lua_init_func=NULL;
	wtk_string_set(&(cfg->pre_fld),0,0);
	wtk_string_set(&(cfg->post_fld),0,0);
	wtk_string_set(&(cfg->bg_fld),0,0);
	wtk_string_set(&(cfg->interrupt_fld),0,0);

	wtk_semfstr_cfg_init(&(cfg->semfst));
	cfg->semfst_net=NULL;
	cfg->semfst_net_fn=NULL;

	cfg->lua_robot_init_func=NULL;
	cfg->lua_robot_msg_func=NULL;
	wtk_lua2_cfg_init(&(cfg->robot_lua));

	cfg->def_dn=NULL;

	cfg->use_rbin=0;
	cfg->lua_feed_json_func=NULL;
	cfg->lua_usr_get="usr_get";
	cfg->lua_feed_rec_hint=NULL;
	cfg->lua_on_empty_output=NULL;
	cfg->lua_reset=NULL;

	wtk_chnlike_cfg_init(&(cfg->chnlike));
	cfg->use_chnlike=0;
	cfg->lua_rec_choose_func=NULL;
	cfg->use_wrdvec=1;
	cfg->use_faq=1;

	cfg->dat_fn=NULL;

	wtk_nlpemot_cfg_init(&(cfg->nlpemot));
	cfg->use_nlpemot=0;
	cfg->cfile=NULL;
	cfg->rbin = NULL;


	wtk_faqc_cfg_init(&(cfg->faqc));
	cfg->use_faqc=0;

	cfg->lua_process_init_func=NULL;
	cfg->lua_set_func=NULL;
	cfg->lua_syn_semdlg_state=NULL;
	cfg->lua_int_lua_local_set = NULL;
	cfg->lua_int_lua_local_get = NULL;
	cfg->else_field = NULL;
	return 0;
}

int wtk_semdlg_cfg_clean(wtk_semdlg_cfg_t *cfg)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;

	wtk_faqc_cfg_clean(&(cfg->faqc));
	while(1)
	{
		qn=wtk_queue_pop(&(cfg->fld_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		wtk_semfld_cfg_clean(&(item->fld));
	}
	if(cfg->semfst_net)
	{
		wtk_semfst_net_delete(cfg->semfst_net);
	}
	if(cfg->use_chnlike)
	{
		wtk_chnlike_cfg_clean(&(cfg->chnlike));
	}
	if(cfg->use_wrdvec)
	{
		wtk_wrdvec_cfg_clean(&(cfg->wrdvec));
	}
	if(cfg->use_nlpemot)
	{
		wtk_nlpemot_cfg_clean(&(cfg->nlpemot));
	}
	wtk_semfstr_cfg_clean(&(cfg->semfst));
	if(cfg->use_faq)
	{
		wtk_faq_cfg_clean(&(cfg->faq));
	}
	wtk_kgkv_cfg_clean(&(cfg->kgkv));
	wtk_lua2_cfg_clean(&(cfg->lua));
	wtk_lua2_cfg_clean(&(cfg->robot_lua));
	wtk_lex_cfg_clean(&(cfg->lex));
	wtk_semfld_cfg_clean(&(cfg->domain));
	if(cfg->cfile)
	{
		wtk_cfg_file_delete(cfg->cfile);
	}
	if(cfg->rbin) {
		wtk_rbin2_delete(cfg->rbin);
	}
	return 0;
}

int wtk_semdlg_cfg_add_fld(wtk_semdlg_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_heap_t *heap=lc->heap;
	wtk_semfld_item_t *item;
	int ret;

	//wtk_debug("[%.*s]\n",lc->name.len,lc->name.data);
	item=(wtk_semfld_item_t*)wtk_heap_malloc(heap,sizeof(wtk_semfld_item_t));
	wtk_semfld_cfg_init(&(item->fld));
	ret=wtk_semfld_cfg_update_local(&(item->fld),lc);
	if(ret==0)
	{
		wtk_queue_push(&(cfg->fld_q),&(item->q_n));
	}
	return ret;
}


int wtk_semdlg_cfg_update_flds(wtk_semdlg_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_queue_node_t *qn;
	wtk_cfg_item_t *item;
	int ret;

	for(qn=lc->cfg->queue.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_cfg_item_t,n);
		if(item->type==WTK_CFG_LC)
		{
			ret=wtk_semdlg_cfg_add_fld(cfg,item->value.cfg);
			if(ret!=0){goto end;}
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_semdlg_cfg_update_local(wtk_semdlg_cfg_t *cfg,wtk_local_cfg_t *main)
{
	wtk_local_cfg_t *lc;
	wtk_string_t *v;
	int ret=-1;

	//wtk_local_cfg_print(main);
	lc=main;
	wtk_local_cfg_update_cfg_string_v(lc,cfg,failed_output,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,empty_output,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,pre_fld,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,post_fld,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,bg_fld,v);
	wtk_local_cfg_update_cfg_string_v(lc,cfg,interrupt_fld,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_syn_semdlg_state,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_set_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_feed_rec_hint,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_usr_get,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,brain_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_init_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_process_init_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,semfst_net_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_robot_msg_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_robot_init_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_feed_json_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_rec_choose_func,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_int_lua_local_get,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_int_lua_local_set,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,def_dn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,dat_fn,v);
	//wtk_local_cfg_update_cfg_str(lc,cfg,kv_fn,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_on_empty_output,v);
	wtk_local_cfg_update_cfg_str(lc,cfg,lua_reset,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,nhistory,v);
	wtk_local_cfg_update_cfg_f(lc,cfg,grammar_conf_thresh,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,debug,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_chnlike,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_wrdvec,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_faq,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_nlpemot,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,use_faqc,v);
	cfg->else_field = wtk_local_cfg_find_array_s(lc,"else_field");
	if(cfg->use_nlpemot)
	{
		lc=wtk_local_cfg_find_lc_s(main,"nlpemot");
		if(lc)
		{
			ret=wtk_nlpemot_cfg_update_local(&(cfg->nlpemot),lc);
			if(ret!=0){goto end;}
		}
	}
	if(cfg->use_faq)
	{
		lc=wtk_local_cfg_find_lc_s(main,"faq");
		if(lc)
		{
			ret=wtk_faq_cfg_update_local(&(cfg->faq),lc);
			if(ret!=0){goto end;}
		}
	}
	if(cfg->use_faqc)
	{
		lc=wtk_local_cfg_find_lc_s(main,"faqc");
		if(lc)
		{
			ret=wtk_faqc_cfg_update_local(&(cfg->faqc),lc);
			if(ret!=0){goto end;}
		}
	}
	lc=wtk_local_cfg_find_lc_s(main,"lex");
	if(lc)
	{
		ret=wtk_lex_cfg_update_local(&(cfg->lex),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"lua");
	if(lc)
	{
		ret=wtk_lua2_cfg_update_local(&(cfg->lua),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"robot_lua");
	if(lc)
	{
		//wtk_local_cfg_print(lc);
		ret=wtk_lua2_cfg_update_local(&(cfg->robot_lua),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"domain");
	//wtk_local_cfg_print(lc);
	if(lc)
	{
		ret=wtk_semfld_cfg_update_local(&(cfg->domain),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"flds");
	if(lc)
	{
		ret=wtk_semdlg_cfg_update_flds(cfg,lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"semfst");
	if(lc)
	{
		//wtk_local_cfg_print(lc);
		ret=wtk_semfstr_cfg_update_local(&(cfg->semfst),lc);
		if(ret!=0){goto end;}
	}
	lc=wtk_local_cfg_find_lc_s(main,"assist");
	if(lc)
	{
		ret=wtk_semdlg_assist_cfg_update_local(&(cfg->assist),lc);
		if(ret!=0){goto end;}
	}
	if(cfg->use_wrdvec)
	{
		lc=wtk_local_cfg_find_lc_s(main,"wrdvec");
		if(lc)
		{
			ret=wtk_wrdvec_cfg_update_local(&(cfg->wrdvec),lc);
			if(ret!=0){goto end;}
		}
	}
	if(cfg->use_chnlike)
	{
		lc=wtk_local_cfg_find_lc_s(main,"chnlike");
		if(lc)
		{
			ret=wtk_chnlike_cfg_update_local(&(cfg->chnlike),lc);
			if(ret!=0){goto end;}
		}
	}
	lc=wtk_local_cfg_find_lc_s(main,"kgkv");
	if(lc)
	{
		ret=wtk_kgkv_cfg_update_local(&(cfg->kgkv),lc);
		if(ret!=0){goto end;}
	}
	if(!cfg->kgkv.kv_fn)
	{
		v=wtk_local_cfg_find_string_s(main,"kv_fn");
		if(v)
		{
			cfg->kgkv.kv_fn=v->data;
		}
	}
	ret=0;
end:
	return ret;
}

int wtk_semdlg_cfg_update_fld(wtk_semdlg_cfg_t *cfg,wtk_lexc_t *lexc)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;
	int ret;

	for(qn=cfg->fld_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		ret=wtk_semfld_cfg_update(&(item->fld),lexc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}

typedef struct
{
	wtk_strbuf_t *buf;
	int i;
}wtk_semdlg_dat_item_t;


void wtk_semdlg_cfg_update_dat_item(wtk_semdlg_dat_item_t *item,char *name,int len)
{
	wtk_string_t *v;
	char *p;

	v=wtk_basename(name,'/');
	p=wtk_str_rchr(v->data,v->len,'.');
	name=v->data;
	len=(int)(p-v->data);
	if(item->i>0)
	{
		wtk_strbuf_push_s(item->buf,"|");
	}
	++item->i;
	wtk_strbuf_push(item->buf,name,len);
	wtk_string_delete(v);
}


wtk_string_t* wtk_semdlg_cfg_update_dat(wtk_semdlg_cfg_t *cfg)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;
	wtk_strbuf_t *buf;
	wtk_semdlg_dat_item_t vi;
	wtk_string_t *v;
	int pos;

	buf=wtk_strbuf_new(256,1);
	vi.buf=buf;
	for(qn=cfg->fld_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		if(item->fld.dat_dn && item->fld.use_dat_env)
		{
			//wtk_debug("%s\n",item->fld.dat_dn);
			if(buf->pos>0)
			{
				wtk_strbuf_push_s(buf,"\n");
			}
			pos=buf->pos;
			wtk_strbuf_push_s(buf,"L");
			wtk_strbuf_push(buf,item->fld.name.data,item->fld.name.len);
			wtk_strbuf_push_s(buf,"=");
			vi.i=0;
			wtk_os_dir_walk(item->fld.dat_dn,&(vi),(wtk_os_dir_walk_notify_f)wtk_semdlg_cfg_update_dat_item);
			wtk_strbuf_push_s(buf,";");
			item->fld.lex_pre_dat=wtk_string_dup_data(buf->data+pos,buf->pos-pos);
			//wtk_debug("[%.*s]\n",item->fld.lex_pre_dat->len,item->fld.lex_pre_dat->data);
			//exit(0);
		}else if(cfg->assist.name.len>0 && (wtk_string_cmp_s(&(item->fld.name),"usr")==0||wtk_string_cmp_s(&(item->fld.name),"bg")==0))
		{
			wtk_strbuf_t *xbuf;

			xbuf=wtk_strbuf_new(256,1);
			wtk_strbuf_push_s(xbuf,"Lusr=");
			wtk_strbuf_push(xbuf,cfg->assist.name.data,cfg->assist.name.len);
			wtk_strbuf_push_s(xbuf,";");
			item->fld.lex_pre_dat=wtk_string_dup_data(xbuf->data,xbuf->pos);
			//wtk_debug("[%.*s]\n",item->fld.lex_pre_dat->len,item->fld.lex_pre_dat->data);
			wtk_strbuf_delete(xbuf);
		}
	}
	//exit(0);
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	if(buf->pos>0)
	{
		v=wtk_string_dup_data(buf->data,buf->pos);
	}else
	{
		v=NULL;
	}
	//exit(0);
	//wtk_debug("pre_dat=%p\n",cfg->lex.lexc.pre_dat);
	wtk_strbuf_delete(buf);
	return v;
}

#define wtk_semdlg_cfg_find_fld_s(cfg,name) wtk_semdlg_cfg_find_fld(cfg,name,sizeof(name)-1)

wtk_semfld_cfg_t* wtk_semdlg_cfg_find_fld(wtk_semdlg_cfg_t *cfg,char *name,int len)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;

	for(qn=cfg->fld_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		//wtk_debug("[%.*s]\n",item->fld.name.len,item->fld.name.data);
		if(wtk_string_cmp(&(item->fld.name),name,len)==0)
		{
			return &(item->fld);
		}
	}
	return NULL;
}

void wtk_semdlg_cfg_update_info(wtk_semdlg_cfg_t *cfg)
{
	wtk_semfld_cfg_t *fld;
	wtk_kg_inst_t *inst;

	//wtk_debug("============== update info ===============\n");
	if(cfg->assist.name.len==0 && cfg->assist.sex.len==0 && cfg->assist.age<0)
	{
		return;
	}
	fld=wtk_semdlg_cfg_find_fld_s(cfg,"assist");
	//wtk_debug("============== update info fld=%p ===============\n",fld);
	if(fld && fld->use_kg && fld->kg.kg)
	{
		inst=wtk_kg_get_inst_s(fld->kg.kg,"我");
		//wtk_debug("inst=%p\n",inst);
		if(inst)
		{
			//wtk_debug("set inst\n");
			if(cfg->assist.name.len>0)
			{
				wtk_kg_set_inst_value_item_str_s(inst,"属性.姓名",&(cfg->assist.name));
			}
			if(cfg->assist.sex.len>0)
			{
				wtk_kg_set_inst_value_item_str_s(inst,"属性.性别",&(cfg->assist.sex));
			}
			if(cfg->assist.age>=0)
			{
				wtk_kg_set_inst_value_item_number_s(inst,"属性.年龄",cfg->assist.age);
			}
		}
	}
}


int wtk_semdlg_cfg_update(wtk_semdlg_cfg_t *cfg)
{
	wtk_lexc_t *lexc=NULL;
	int ret;
	wtk_string_t *v;

	if(!cfg->wrdvec.fn)
	{
		cfg->use_wrdvec=0;
	}
	if(cfg->use_wrdvec)
	{
		ret=wtk_wrdvec_cfg_update(&(cfg->wrdvec));
		if(ret!=0){goto end;}
	}
	if(cfg->use_faq)
	{
		ret=wtk_faq_cfg_update(&(cfg->faq));
		if(ret!=0){goto end;}
	}
	if(cfg->use_faqc)
	{
		ret=wtk_faqc_cfg_update(&(cfg->faqc));
		if(ret!=0){goto end;}
	}
	ret=wtk_lua2_cfg_update(&(cfg->lua));
	if(ret!=0){goto end;}
	ret=wtk_lua2_cfg_update(&(cfg->robot_lua));
	if(ret!=0){goto end;}
	ret=wtk_lex_cfg_update(&(cfg->lex));
	if(ret!=0){goto end;}
	v=wtk_semdlg_cfg_update_dat(cfg);
	lexc=wtk_lexc_new(&(cfg->lex.lexc));
	if(cfg->use_nlpemot)
	{
		ret=wtk_nlpemot_cfg_update(&(cfg->nlpemot),lexc);
		if(ret!=0){goto end;}
	}
	if(v)
	{
		lexc->pre_dat=v;
	}
	//wtk_debug("[%.*s]\n",v->len,v->data);
	//wtk_debug("pre_data=%p\n",lexc->pre_dat);
	ret=wtk_semfld_cfg_update(&(cfg->domain),lexc);
	if(0 != ret) {
		goto end;
	}
	//exit(0);
	if(v)
	{
		lexc->pre_dat=NULL;
		wtk_string_delete(v);
	}
	if(ret!=0){goto end;}
	ret=wtk_semdlg_cfg_update_fld(cfg,lexc);
	if(ret!=0){goto end;}
	ret=wtk_semfstr_cfg_update(&(cfg->semfst));
	if(ret!=0){goto end;}
	if(cfg->semfst_net_fn)
	{
		wtk_semfstc_t *c;

		c=wtk_semfstc_new();
		cfg->semfst_net=wtk_semfst_net_new(lexc);
		ret=wtk_semfstc_compile_file(c,cfg->semfst_net,cfg->semfst_net_fn);
		wtk_semfstc_delete(c);
		if(ret!=0){goto end;}
	}
	if(cfg->use_chnlike)
	{
		ret=wtk_chnlike_cfg_update(&(cfg->chnlike));
		if(ret!=0){goto end;}
	}
	wtk_semdlg_cfg_update_info(cfg);
	ret=0;
end:
	if(lexc)
	{
		wtk_lexc_delete(lexc);
	}
	//cfg->faq.vecdb.db="./tmp/faqvec.db";
	//cfg->brain_dn="./tmp/brain";
	return ret;
}


int wtk_semdlg_cfg_update_fld2(wtk_semdlg_cfg_t *cfg,wtk_source_loader_t *sl,wtk_lexc_t *lexc)
{
	wtk_queue_node_t *qn;
	wtk_semfld_item_t *item;
	int ret;

	for(qn=cfg->fld_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_semfld_item_t,q_n);
		ret=wtk_semfld_cfg_update2(&(item->fld),sl,lexc);
		if(ret!=0){goto end;}
	}
	ret=0;
end:
	return ret;
}


int wtk_semdlg_cfg_update2(wtk_semdlg_cfg_t *cfg,wtk_source_loader_t *sl)
{
	wtk_lexc_t *lexc=NULL;
	wtk_string_t *v;
	int ret;

	if(cfg->use_faqc)
	{
		ret=wtk_faqc_cfg_update(&(cfg->faqc));
		if(ret!=0){goto end;}
	}
	ret=wtk_wrdvec_cfg_update2(&(cfg->wrdvec),sl);
	if(ret!=0){goto end;}
	if (cfg->use_faq){
	 	ret=wtk_faq_cfg_update2(&(cfg->faq),sl);
		if(ret!=0){goto end;}
	}
	ret=wtk_lua2_cfg_update(&(cfg->lua));
	if(ret!=0){goto end;}
	ret=wtk_lua2_cfg_update(&(cfg->robot_lua));
	if(ret!=0){goto end;}
	ret=wtk_lex_cfg_update2(&(cfg->lex),sl);
	if(ret!=0){goto end;}
	ret=wtk_kgkv_cfg_update2(&(cfg->kgkv),sl);
	if(ret!=0){goto end;}
	v=wtk_semdlg_cfg_update_dat(cfg);
	//wtk_debug("[%.*s]\n",v->len,v->data);
	//exit(0);
	lexc=wtk_lexc_new(&(cfg->lex.lexc));
	if(cfg->use_nlpemot)
	{
		ret=wtk_nlpemot_cfg_update2(&(cfg->nlpemot),sl,lexc);
		if(ret!=0)
		{
			wtk_debug("update nplemot failed\n");
			goto end;
		}
	}
	if(v)
	{
		lexc->pre_dat=v;
	}
	ret=wtk_semfld_cfg_update2(&(cfg->domain),sl,lexc);
	if(v)
	{
		lexc->pre_dat=NULL;
		wtk_string_delete(v);
	}
	if(ret!=0){goto end;}
	ret=wtk_semdlg_cfg_update_fld2(cfg,sl,lexc);
	if(ret!=0){goto end;}
	ret=wtk_semfstr_cfg_update(&(cfg->semfst));
	if(ret!=0){goto end;}
	if(cfg->semfst_net_fn)
	{
		wtk_semfstc_t *c;

		c=wtk_semfstc_new();
		cfg->semfst_net=wtk_semfst_net_new(lexc);
		c->rbin=lexc->rbin;
		ret=wtk_semfstc_compile_file(c,cfg->semfst_net,cfg->semfst_net_fn);
		wtk_semfstc_delete(c);
		if(ret!=0){goto end;}
	}
	wtk_semdlg_cfg_update_info(cfg);
	ret=0;
end:
	if(lexc)
	{
		wtk_lexc_delete(lexc);
	}
	return ret;
}

wtk_semdlg_cfg_t* wtk_semdlg_cfg_new(char *fn)
{
	wtk_main_cfg_t *main_cfg = NULL;
	wtk_semdlg_cfg_t *cfg = NULL;

	main_cfg=wtk_main_cfg_new_type(wtk_semdlg_cfg,fn);//wtk_semdlg_cfg_init()
	if(main_cfg == NULL) {
		goto end;
	}
	cfg=(wtk_semdlg_cfg_t*)main_cfg->cfg;
	cfg->cfg.main_cfg=main_cfg;
	cfg->use_rbin=0;
	//cfg->faq.vecdb.db="./tmp/faqvec.db";
	//cfg->brain_dn="./tmp/brain";
end:
	return cfg;
}

wtk_semdlg_cfg_t* wtk_semdlg_cfg_new_bin(char *fn)
{
	wtk_mbin_cfg_t *bin_cfg;
	wtk_semdlg_cfg_t *cfg=NULL;
	wtk_cfg_file_t *cfile;
	wtk_string_t *v;
	wtk_local_cfg_t *custom;

	cfile=wtk_cfg_file_new_fn(fn);
	if(!cfile){goto end;}
	v=wtk_local_cfg_find_string_s(cfile->main,"cfg");
	if(!v)
	{
		wtk_debug("cfg not found\n");
		goto end;
	}
	custom=wtk_local_cfg_find_lc_s(cfile->main,"main");
	//wtk_debug("custom=%p\n",custom);
	if(custom)
	{
		bin_cfg=wtk_mbin_cfg_new_type3(wtk_semdlg_cfg,v->data,"./cfg",custom);
	}else
	{
		bin_cfg=wtk_mbin_cfg_new_type(wtk_semdlg_cfg,v->data,"./cfg");
	}
	cfg=(wtk_semdlg_cfg_t*)bin_cfg->cfg;
	cfg->cfg.bin_cfg=bin_cfg;
	cfg->use_rbin=1;
	cfg->rbin = bin_cfg->rbin;
	//cfg->faq.vecdb.db="./tmp/faqvec.db";
	//cfg->brain_dn="./tmp/brain";
	cfg->cfile=cfile;
	cfile=NULL;
end:
	if(cfile)
	{
		wtk_cfg_file_delete(cfile);
	}
	//exit(0);
	return cfg;
}

void wtk_semdlg_cfg_delete(wtk_semdlg_cfg_t *cfg)
{
	if(cfg->use_rbin)
	{
		cfg->rbin = NULL;
		wtk_mbin_cfg_delete(cfg->cfg.bin_cfg);
	}else
	{
		wtk_main_cfg_delete(cfg->cfg.main_cfg);
	}
}

wtk_semdlg_cfg_t* wtk_semdlg_cfg_new_bin2(char *bin_fn,int seek_pos,wtk_local_cfg_t *custom)
{
	wtk_semdlg_cfg_t *cfg;
	wtk_rbin2_item_t *item;
	wtk_source_loader_t sl;
	char *cfg_fn = "./cfg";
	int ret;


	cfg = (wtk_semdlg_cfg_t*)wtk_malloc(sizeof(wtk_semdlg_cfg_t));
	wtk_semdlg_cfg_init(cfg);

	cfg->rbin = wtk_rbin2_new();
	ret = wtk_rbin2_read2(cfg->rbin,bin_fn,seek_pos);
	if(ret != 0) {
		goto end;
	}

	item = wtk_rbin2_get2(cfg->rbin,cfg_fn,strlen(cfg_fn));
	if(!item) {
		ret = -1;	goto end;
	}

	cfg->cfile = wtk_cfg_file_new();
	wtk_cfg_file_set_rbin(cfg->cfile,cfg->rbin);
	wtk_cfg_file_add_var_ks(cfg->cfile,"pwd",".",1);
	ret = wtk_cfg_file_feed(cfg->cfile,item->data->data,item->data->len);
	if(ret != 0) {
		goto end;
	}
	if(custom) {
		wtk_local_cfg_update(cfg->cfile->main,custom);
	}

	ret = wtk_semdlg_cfg_update_local(cfg,cfg->cfile->main);
	if(ret != 0) {
		goto end;
	}

	sl.hook = cfg->rbin;
	sl.vf = (wtk_source_loader_v_t)wtk_rbin2_load_file;
	ret = wtk_semdlg_cfg_update2(cfg,&sl);
	if(ret != 0) {
		goto end;
	}
	cfg->use_rbin = 1;

	ret = 0;

end:
	if(ret != 0) {
		wtk_semdlg_cfg_delete_bin2(cfg);
		cfg = NULL;
	}
	return cfg;

}

void wtk_semdlg_cfg_delete_bin2(wtk_semdlg_cfg_t *cfg)
{
	wtk_semdlg_cfg_clean(cfg);
	wtk_free(cfg);
}


