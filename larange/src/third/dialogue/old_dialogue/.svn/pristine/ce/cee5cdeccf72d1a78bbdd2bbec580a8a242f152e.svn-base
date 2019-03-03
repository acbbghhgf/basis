#include "wtk_semfst_net.h" 

wtk_semfst_net_t* wtk_semfst_net_new(wtk_lexc_t *lexc)
{
	wtk_semfst_net_t *r;

	r=(wtk_semfst_net_t*)wtk_malloc(sizeof(wtk_semfst_net_t));
	r->heap=wtk_heap_new(4096);
	r->buf=wtk_strbuf_new(256,1);
	r->lexc=lexc;
	wtk_queue_init(&(r->sceen_q));
	return r;
}

void wtk_semfst_delete_sceen(wtk_semfst_net_t *r)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_semfst_sceen_t *sceen;
	wtk_semfst_script_t *script;

	for(qn=r->sceen_q.pop;qn;qn=qn->next)
	{
		sceen=data_offset2(qn,wtk_semfst_sceen_t,q_n);
		for(qn2=sceen->script_q.pop;qn2;qn2=qn2->next)
		{
			script=data_offset2(qn2,wtk_semfst_script_t,q_n);
			wtk_lex_net_delete(script->lexnet);
			wtk_lex_script_delete(script->lexscript);
		}
	}
}

void wtk_semfst_net_delete(wtk_semfst_net_t *r)
{
	wtk_semfst_delete_sceen(r);
	wtk_lex_script_delete(r->lexlib);
	wtk_strbuf_delete(r->buf);
	wtk_heap_delete(r->heap);
	wtk_free(r);
}

int wtk_semfst_net_set_lexlib(wtk_semfst_net_t *r,char *fn,int fn_bytes)
{
	wtk_strbuf_t *buf=r->buf;

	wtk_strbuf_reset(buf);
	wtk_strbuf_push(buf,fn,fn_bytes);
	wtk_strbuf_push_c(buf,0);
	r->lexlib=wtk_lexc_compile_file(r->lexc,buf->data);
	wtk_lexc_reset(r->lexc);
	return r->lexlib?0:-1;
}

wtk_semfst_sceen_t* wtk_semfst_net_pop_sceen(wtk_semfst_net_t *r,char *name,int bytes)
{
	wtk_semfst_sceen_t *s;

	s=(wtk_semfst_sceen_t*)wtk_heap_malloc(r->heap,sizeof(wtk_semfst_sceen_t));
	wtk_queue_init(&(s->script_q));
	wtk_queue_push(&(r->sceen_q),&(s->q_n));
	if(bytes>0)
	{
		s->name=wtk_heap_dup_string(r->heap,name,bytes);
	}else
	{
		s->name=NULL;
	}
	s->init=NULL;
	return s;
}

wtk_semfst_script_t* wtk_semfst_net_find_script(wtk_semfst_net_t *r,wtk_semfst_sceen_t *sceen,char *nm,int nm_bytes,int insert)
{
	wtk_semfst_script_t *s;
	wtk_heap_t *heap=r->heap;
	wtk_queue_node_t *qn;

	if(nm_bytes>0)
	{
		for(qn=sceen->script_q.pop;qn;qn=qn->next)
		{
			s=data_offset2(qn,wtk_semfst_script_t,q_n);
			if(wtk_string_cmp(s->name,nm,nm_bytes)==0)
			{
				goto end;
			}
		}
	}
	if(!insert){s=NULL;goto end;}
	s=(wtk_semfst_script_t*)wtk_heap_malloc(heap,sizeof(wtk_semfst_script_t));
	if(nm_bytes>0)
	{
		s->name=wtk_heap_dup_string(heap,nm,nm_bytes);
	}else
	{
		s->name=NULL;
	}
	wtk_queue3_init(&(s->output_q));
	s->other=NULL;
	s->lexnet=NULL;
	s->lexscript=NULL;
	if(!sceen->init)
	{
		sceen->init=s;
	}
	wtk_queue3_push(&(sceen->script_q),&(s->q_n));
end:
	return s;
}

int wtk_semfst_net_set_script_lex(wtk_semfst_net_t *r,wtk_semfst_script_t *script,char *lex,int lex_len)
{
	wtk_lexc_t *lexc=r->lexc;

	//wtk_debug("[%.*s]\n",lex_len,lex);
	wtk_lexc_set_get_expr(lexc,r->lexlib,(wtk_lexc_get_expr_f)wtk_lex_script_get_expr);
	script->lexscript=wtk_lexc_compile(lexc,lex,lex_len);
	if(!script->lexscript)
	{
		return -1;
	}
	//wtk_lex_script_print(script->lexscript);
	wtk_lexc_reset(lexc);
	script->lexnet=wtk_lex_net_new(script->lexscript);
	return 0;
}

wtk_semfst_output_t* wtk_semfst_net_pop_script_output(wtk_semfst_net_t *r,wtk_semfst_script_t *script)
{
	wtk_semfst_output_t *output;

	output=(wtk_semfst_output_t*)wtk_heap_malloc(r->heap,sizeof(wtk_semfst_output_t));
	output->get_answer=0;
	output->reset=0;
	output->skip=0;
	wtk_queue3_init(&(output->slot_q));
	output->n_output_item=0;
	output->output_items=NULL;
	output->next=NULL;
	return output;
}

void wtk_semfst_net_set_script_output_slot(wtk_semfst_net_t *r,wtk_semfst_output_t *output,wtk_string_t *k,wtk_string_t *v)
{
	wtk_heap_t *heap=r->heap;
	wtk_semfst_slot_t *slot;

	slot=(wtk_semfst_slot_t*)wtk_heap_malloc(heap,sizeof(wtk_semfst_slot_t));
	slot->k=wtk_heap_dup_string(heap,k->data,k->len);
	if(v && v->len>0)
	{
		slot->v=wtk_heap_dup_string(heap,v->data,v->len);
	}else
	{
		slot->v=NULL;
	}
	wtk_queue3_push(&(output->slot_q),&(slot->q_n));
}

wtk_semfst_output_str_item_t* wtk_semfst_net_output_new_output_item_str(wtk_semfst_net_t *r,char* data,int len)
{
	wtk_heap_t *heap=r->heap;
	wtk_semfst_output_str_item_t* item;

	item=(wtk_semfst_output_str_item_t*)wtk_heap_malloc(heap,sizeof(wtk_semfst_output_str_item_t));
	item->is_var=0;
	item->v.str=wtk_heap_dup_string(heap,data,len);
	return item;
}

wtk_semfst_output_str_item_t* wtk_semfst_net_output_new_output_item_str_var(wtk_semfst_net_t *r,char* data,int len)
{
	wtk_heap_t *heap=r->heap;
	wtk_semfst_output_str_item_t* item;

	item=(wtk_semfst_output_str_item_t*)wtk_heap_malloc(heap,sizeof(wtk_semfst_output_str_item_t));
	item->is_var=1;
	item->v.var=wtk_heap_dup_string(heap,data,len);
	return item;
}

void wtk_semfst_output_str_item_print(wtk_semfst_output_str_item_t *si)
{
	if(si->is_var)
	{
		printf("${%.*s}",si->v.var->len,si->v.var->data);
	}else
	{
		printf("%.*s",si->v.str->len,si->v.str->data);
	}
}

void wtk_semfst_output_item_print(wtk_semfst_output_item_t *item)
{
	wtk_queue_node_t *qn;
	wtk_semfst_output_str_item_t *si;

	if(item->type==WTK_SEMFSTSTR_OUTPUT_LUA)
	{
		printf("#%s\n",item->v.lua);
	}else
	{
		for(qn=item->v.strq->pop;qn;qn=qn->next)
		{
			si=data_offset2(qn,wtk_semfst_output_str_item_t,q_n);
			wtk_semfst_output_str_item_print(si);
		}
		printf("\n");
	}
}

wtk_semfst_output_item_t* wtk_semfst_net_output_new_output_item(wtk_semfst_net_t *r,char *data,int len)
{
	wtk_heap_t *heap=r->heap;
	wtk_semfst_output_item_t *item;

	item=(wtk_semfst_output_item_t*)wtk_heap_malloc(heap,sizeof(wtk_semfst_output_item_t));
	if(data[0]=='#')
	{
		item->type=WTK_SEMFSTSTR_OUTPUT_LUA;
		item->v.lua=wtk_heap_dup_str2(heap,data+1,len-1);
	}else
	{
		wtk_var_parse_t *vp;
		wtk_strbuf_t *buf;
		char *s,*e;
		int n;
		int ret;
		wtk_semfst_output_str_item_t *si;

		item->type=WTK_SEMFSTSTR_OUTPUT_STR;
		item->v.strq=(wtk_queue3_t*)wtk_heap_malloc(heap,sizeof(wtk_queue3_t));
		wtk_queue3_init(item->v.strq);
		buf=wtk_strbuf_new(256,1);
		vp=wtk_var_parse_new();
		s=data;e=s+len;
		while(s<e)
		{
			n=wtk_utf8_bytes(*s);
			//wtk_debug("[%.*s]\n",n,s);
			ret=wtk_var_parse2(vp,s,n);
			if(ret==1)
			{
				//wtk_debug("[%.*s]\n",buf->pos,buf->data);
				if(buf->pos>0)
				{
					si=wtk_semfst_net_output_new_output_item_str(r,buf->data,buf->pos);
					wtk_queue3_push(item->v.strq,&(si->q_n));
					wtk_strbuf_reset(buf);
				}
				si=wtk_semfst_net_output_new_output_item_str_var(r,vp->buf->data,vp->buf->pos);
				wtk_queue3_push(item->v.strq,&(si->q_n));
				wtk_var_parse_reset(vp);
			}else if(vp->sub_state==0)
			{
				wtk_strbuf_push(buf,s,n);
			}
			s+=n;
		}
		if(buf->pos>0)
		{
			si=wtk_semfst_net_output_new_output_item_str(r,buf->data,buf->pos);
			wtk_queue3_push(item->v.strq,&(si->q_n));
		}
		//wtk_semfst_output_item_print(item);
		wtk_var_parse_delete(vp);
		wtk_strbuf_delete(buf);
	}
	return item;
}

void wtk_semfst_net_set_output_items(wtk_semfst_net_t *r,wtk_semfst_output_t *output,wtk_semfst_output_item_t **items,int n)
{
	wtk_heap_t *heap=r->heap;

	//wtk_debug("n=%d\n",n);
	output->output_items=(wtk_semfst_output_item_t**)wtk_heap_malloc(heap,sizeof(wtk_semfst_output_item_t*)*n);
	output->n_output_item=n;
	memcpy(output->output_items,items,sizeof(wtk_semfst_output_item_t*)*n);
}


wtk_semfst_output_item_t* wtk_semfst_output_get(wtk_semfst_output_t *output)
{
	int i;

	i=wtk_random(0,output->n_output_item-1);
	//wtk_debug("=================================> i=%d/%d\n",i,output->n_output_item);
	return output->output_items[i];
}



void wtk_semfst_output_print(wtk_semfst_output_t *output)
{
	wtk_semfst_slot_t *slot;
	wtk_queue_node_t *qn;
	int i;

	printf("[output,get_ask=%d,reset=%d,skip=%d",output->get_answer,output->reset,output->skip);
	if(output->next)
	{
		printf(",next=%.*s",output->next->name->len,output->next->name->data);
	}
	for(qn=output->slot_q.pop;qn;qn=qn->next)
	{
		slot=data_offset2(qn,wtk_semfst_slot_t,q_n);
		if(slot->k && slot->v)
		{
			printf(",%.*s=%.*s",slot->k->len,slot->k->data,slot->v->len,slot->v->data);
		}else
		{
			printf(",%.*s",slot->k->len,slot->k->data);
		}
	}
	printf("]\n");
	for(i=0;i<output->n_output_item;++i)
	{
		wtk_semfst_output_item_print(output->output_items[i]);
	}
}

void wtk_semfst_script_print(wtk_semfst_script_t *s)
{
	wtk_queue_node_t *qn;
	wtk_semfst_output_t *output;

	if(s->name)
	{
		printf("[script=%.*s]\n",s->name->len,s->name->data);
	}
	printf("[lex]\n");
	if(s->lexscript)
	{
		printf("{{{\n");
		wtk_lex_script_print(s->lexscript);
		//exit(0);
		printf("}}}\n");
	}
	for(qn=s->output_q.pop;qn;qn=qn->next)
	{
		output=data_offset2(qn,wtk_semfst_output_t,q_n);
		wtk_semfst_output_print(output);
	}
	if(s->other)
	{
		wtk_semfst_output_print(s->other);
	}
//	if(s->fail)
//	{
//		wtk_semfst_output_print(s->fail);
//	}
	printf("\n");
}

void wtk_semfst_sceen_print(wtk_semfst_sceen_t *s)
{
	wtk_queue_node_t *qn;
	wtk_semfst_script_t *script;

	if(s->name)
	{
		printf("[sceen=%.*s]\n",s->name->len,s->name->data);
	}else
	{
		printf("[sceen]\n");
	}
	for(qn=s->script_q.pop;qn;qn=qn->next)
	{
		script=data_offset2(qn,wtk_semfst_script_t,q_n);
		wtk_semfst_script_print(script);
	}
}

void wtk_semfst_net_print(wtk_semfst_net_t *r)
{
	wtk_queue_node_t *qn;
	wtk_semfst_sceen_t *s;

	for(qn=r->sceen_q.pop;qn;qn=qn->next)
	{
		s=data_offset2(qn,wtk_semfst_sceen_t,q_n);
		wtk_semfst_sceen_print(s);
	}
}
