#include "wtk_chnpos.h" 
#include <ctype.h>

wtk_chnpos_t* wtk_chnpos_new(wtk_chnpos_cfg_t *cfg)
{
	wtk_chnpos_t *pos;

	pos=(wtk_chnpos_t*)wtk_malloc(sizeof(wtk_chnpos_t));
	pos->cfg=cfg;
	pos->heap=wtk_heap_new(4096);
	if(cfg->use_prune)
	{
		pos->prune=wtk_prune_new(&(cfg->prune));
	}else
	{
		pos->prune=NULL;
	}
	//wtk_debug("prune=%p\n",pos->prune);
	//exit(0);
	wtk_chnpos_reset(pos);
	return pos;
}

void wtk_chnpos_delete(wtk_chnpos_t *pos)
{
	if(pos->prune)
	{
		wtk_prune_delete(pos->prune);
	}
	wtk_heap_delete(pos->heap);
	wtk_free(pos);
}


void wtk_chnpos_reset(wtk_chnpos_t *pos)
{
	if(pos->prune)
	{
		wtk_prune_reset(pos->prune);
	}
	wtk_heap_reset(pos->heap);
	pos->frame=0;
	pos->max_inst=NULL;
	wtk_queue_init(&(pos->inst_q));
}

wtk_chnpos_inst_t* wtk_chnpos_new_inst(wtk_chnpos_t *pos,wtk_chnpos_state_t *state)
{
	wtk_heap_t *heap=pos->heap;
	wtk_chnpos_inst_t *inst;

	inst=(wtk_chnpos_inst_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_inst_t));
	inst->state=state;
	inst->pth=NULL;
	//wtk_queue_push(&(pos->inst_q),&(inst->q_n));
	return inst;
}

wtk_chnpos_pth_t* wtk_chnpos_new_pth(wtk_chnpos_t *pos,wtk_string_t *v,int vpos)
{
	wtk_heap_t *heap=pos->heap;
	wtk_chnpos_pth_t *pth;

	//wtk_debug("vpos=%d\n",vpos);
	pth=(wtk_chnpos_pth_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_pth_t));
	pth->prev=NULL;
	pth->v=wtk_heap_dup_string(heap,v->data,v->len);
	pth->pos=vpos;
	return pth;
}


void wtk_chnpos_init_network(wtk_chnpos_t *pos,wtk_string_t *obs)
{
	wtk_chnpos_wrd_t wrd;
	wtk_chnpos_inst_t *inst;
	int i;
	float f;

	wrd=wtk_chnpos_model_get_wrd2(pos->cfg->model,obs->data,obs->len);
	//wtk_debug("n=%d\n",n);
	//pth=wtk_chnpos_new_pth(pos,obs);
	for(i=0;i<wrd.nstate;++i)
	{
		f=wrd.states[i]->start+wtk_chnpos_state_get_emit_prob(wrd.states[i],obs);
		if(f<-1e10)
		{
			continue;
		}
		inst=wtk_chnpos_new_inst(pos,wrd.states[i]);
		inst->like=f;
		inst->pth=wtk_chnpos_new_pth(pos,obs,wrd.states[i]->pos);
		if(!pos->max_inst || inst->like>pos->max_inst->like)
		{
			pos->max_inst=inst;
		}
		//wtk_chnpos_state_print(pos->cfg->model,wrd.states[i]);
		//wtk_debug("inst->like=%f/%f/%f\n",wrd.states[i]->start,inst->like,wtk_chnpos_state_get_emit_prob(wrd.states[i],obs));
		wtk_queue_push(&(pos->inst_q),&(inst->q_n));
	}
	//wtk_debug("inst_q=%d\n",pos->inst_q.length);
	//exit(0);
//	if(pos->inst_q.length==0)
//	{
//		inst=wtk_chnpos_new_inst(pos,NULL);
//		inst->like=f;
//		inst->pth=wtk_chnpos_new_pth(pos,obs,-1);
//		wtk_queue_push(&(pos->inst_q),&(inst->q_n));
//	}
}

int wtk_chnpos_has_path(wtk_queue_t *q,wtk_chnpos_pth_t *pth,int pos)
{
	wtk_queue_node_t *qn;
	wtk_chnpos_inst_t *inst;

	for(qn=q->push;qn;qn=qn->prev)
	{
		inst=data_offset2(qn,wtk_chnpos_inst_t,q_n);
		if(inst->pth->prev==pth)
		{
			if(inst->pth->pos==pos)
			{
				return 1;
			}
		}else
		{
			break;
		}
	}
	return 0;
}

void wtk_chnpos_feed(wtk_chnpos_t *pos,wtk_string_t *obs)
{
	wtk_prune_t *prune=pos->prune;
	wtk_queue_node_t *qn;
	wtk_chnpos_inst_t *inst,*inst2;
	wtk_chnpos_wrd_t wrd;
	wtk_chnpos_arc_t *arc;
	wtk_queue_t q;
	int nprev;
	int nsub;
	int i;
	int b;
	float last_max;
	float f;

	if(pos->max_inst)
	{
		last_max=pos->max_inst->like;
	}else
	{
		last_max=0;
	}
	//wtk_debug("[%.*s] pos=%d prune=%p\n",obs->len,obs->data,pos->inst_q.length,pos->prune);
//	if(pos->cfg->use_prune && pos->inst_q.length>pos->cfg->prune.count)
//	{
//		if(pos->max_inst)
//		{
//			last_max=pos->max_inst->like;
//			wtk_debug("last_max=%f\n",last_max);
//			if(last_max<-1e5)
//			{
//				prune=NULL;
//			}
//		}else
//		{
//			last_max=0;
//		}
//	}else
//	{
//		last_max=0;
//		prune=NULL;
//	}
	pos->max_inst=NULL;
	obs=wtk_heap_dup_string(pos->heap,obs->data,obs->len);
	//wtk_debug("[%.*s] len=%d\n",obs->len,obs->data,pos->inst_q.length);
	++pos->frame;
	if(pos->frame==1)
	{
		wtk_chnpos_init_network(pos,obs);
		return;
	}
	//wtk_debug("len=%d\n",pos->inst_q.length);
	wtk_queue_init(&(q));
	nprev=0;
	nsub=0;
	//wtk_debug("last_max=%f\n",last_max);
	wrd=wtk_chnpos_model_get_wrd2(pos->cfg->model,obs->data,obs->len);
	for(qn=pos->inst_q.pop;qn;qn=qn->next)
	{
		inst=data_offset2(qn,wtk_chnpos_inst_t,q_n);
		if(inst->state->narc==0)
		{
			continue;
		}
		nprev+=inst->state->narc;
		//wtk_debug("state=%p/%p narc=%d pos=%d\n",inst->state,arc->to,inst->state->narc,pos->inst_q.length);
		for(i=0;i<inst->state->narc;++i)
		{
			arc=inst->state->arcs+i;
			b=wtk_chnpos_wrd_has_state(&wrd,arc->to);
			if(b)
			{
//				if(inst->pth)
//				{
//					b=wtk_chnpos_has_path(&(q),inst->pth,arc->to->pos);
//					if(b)
//					{
//						continue;
//					}
//				}
				f=wtk_chnpos_state_get_emit_prob(arc->to,obs);
				f=inst->like+arc->prob+f;
				if(prune && f-last_max<prune->cfg->min_score)
				{
					continue;
				}
				++nsub;
				inst2=wtk_chnpos_new_inst(pos,arc->to);
				inst2->like=f;
				//wtk_debug("%f %f/%f/%f last=%f\n",inst2->like,inst->like,arc->prob,f,last_max);
				if(prune)// && inst2->like>-1e5)
				{
					wtk_prune_add(prune,inst2->like-last_max);
				}
				inst2->pth=wtk_chnpos_new_pth(pos,obs,arc->to->pos);
				inst2->pth->prev=inst->pth;
				if(!pos->max_inst || inst2->like>pos->max_inst->like)
				{
					//wtk_debug("like=%f/%f/%f/%f\n",inst->like,inst2->like-inst->like,inst2->like,pos->max_inst?pos->max_inst->like:0);
					//wtk_chnpos_state_print(pos->cfg->model,arc->to);
					pos->max_inst=inst2;
				}
				wtk_queue_push(&(q),&(inst2->q_n));
				//wtk_chnpos_state_print(pos->cfg->model,arc->to);
				//wtk_debug("[%d]\n",q.length);
			}
		}
	}
	//wtk_debug("nsub=%d\n",nsub);
	//wtk_debug("len=%d like=%f\n",q.length,pos->max_inst->like);
	//wtk_debug("len=%d like=%f %.*s\n",q.length,pos->max_inst?pos->max_inst->like:0,obs->len,obs->data);
	if(nsub==0)
	{
		if(nprev==0)
		{
			wtk_chnpos_state_t **states;
			int n;

			states=(wtk_chnpos_state_t**)(pos->cfg->model->state_robin->r);
			n=pos->cfg->model->state_robin->nslot;
			for(qn=pos->inst_q.pop;qn;qn=qn->next)
			{
				inst=data_offset2(qn,wtk_chnpos_inst_t,q_n);
				for(i=0;i<n;++i)
				{
					inst2=wtk_chnpos_new_inst(pos,states[i]);
					inst2->like=inst->like+MIN_FLOAT+wtk_chnpos_state_get_emit_prob(states[i],obs);
					inst2->pth=wtk_chnpos_new_pth(pos,obs,states[i]->pos);
					inst2->pth->prev=inst->pth;
					if(!pos->max_inst || inst2->like>pos->max_inst->like)
					{
						pos->max_inst=inst2;
					}
					//wtk_debug("inst2_like=%f\n",inst2->like);
					wtk_queue_push(&(q),&(inst2->q_n));
				}
			}
		}else
		{
			for(qn=pos->inst_q.pop;qn;qn=qn->next)
			{
				inst=data_offset2(qn,wtk_chnpos_inst_t,q_n);
				if(inst->state->narc==0)
				{
					continue;
				}
				for(i=0;i<inst->state->narc;++i)
				{
					arc=inst->state->arcs+i;
					f=inst->like+arc->prob+wtk_chnpos_state_get_emit_prob(arc->to,obs);
					if(pos->max_inst && f<pos->max_inst->like-1e5)
					{
						continue;
					}
					inst2=wtk_chnpos_new_inst(pos,arc->to);
					inst2->like=f;
					inst2->pth=wtk_chnpos_new_pth(pos,obs,arc->to->pos);
					inst2->pth->prev=inst->pth;
					if(!pos->max_inst || inst2->like>pos->max_inst->like)
					{
						pos->max_inst=inst2;
					}
//					wtk_debug("inst2_like=%f %f/%f [%.*s]\n",inst2->like,inst->like,arc->prob,
//							(((wtk_string_t**)pos->cfg->model->pos_a->slot)[arc->to->pos])->len,
//							(((wtk_string_t**)pos->cfg->model->pos_a->slot)[arc->to->pos])->data);
					//exit(0);
					wtk_queue_push(&(q),&(inst2->q_n));
				}
			}
		}
	}
	//wtk_debug("prune=-%p %d/%d\n",prune,prune->count,prune->cfg->count);
	if(prune)
	{
		if(wtk_prune_want_prune(prune))
		{
			wtk_queue_t qx;
			float f;
			//float f1;


			f=wtk_prune_get_thresh(prune);
			//wtk_debug("prrune thresh=%f\n",f);
			//f1=f;
			f+=last_max;
			wtk_queue_init(&(qx));
			while(1)
			{
				qn=wtk_queue_pop(&(q));
				if(!qn){break;}
				inst=data_offset2(qn,wtk_chnpos_inst_t,q_n);
				if(inst->like>f)
				{
					//wtk_debug("push[%d/%d] %f/%f\n",qx.length,q.length,inst->like,f);
					wtk_queue_push(&(qx),qn);
				}
			}
			q=qx;
		}
		wtk_prune_reset(prune);
	}
	//wtk_debug("len=%d like=%f\n",q.length,pos->max_inst->like);
	pos->inst_q=q;
	//wtk_debug("[%.*s] pos=%d\n",obs->len,obs->data,pos->inst_q.length);
}

void wtk_chnpos_pth_print(wtk_chnpos_t *pos,wtk_chnpos_pth_t *pth)
{
	wtk_string_t *v;

	if(pth->prev)
	{
		wtk_chnpos_pth_print(pos,pth->prev);
	}
	//printf("%.*s/%d",pth->v->len,pth->v->data,pth->pos);
	printf("%.*s",pth->v->len,pth->v->data);
	v=wtk_chnpos_model_get_pos_str(pos->cfg->model,pth->pos);
	printf("/%.*s ",v->len,v->data);
}

void wtk_chnpos_print(wtk_chnpos_t *pos)
{
	printf("like: %f\n",pos->max_inst->like);
	wtk_chnpos_pth_print(pos,pos->max_inst->pth);
	printf("\n");
}

int wtk_chnpos_parse(wtk_chnpos_t *pos,wtk_string_t **strs,int n)
{
	int i;

	for(i=0;i<n;++i)
	{
		//wtk_debug("v[%d]=[%.*s]\n",i,strs[i]->len,strs[i]->data);
		wtk_chnpos_feed(pos,strs[i]);
		if(pos->inst_q.length==0)
		{
			return -1;
		}
	}
	//wtk_chnpos_print(pos);
	return 0;
}

void wtk_chnpos_test(wtk_chnpos_t *pos,char *data,int bytes)
{
	int init;
	wtk_string_t k;
	char *s,*e;
	int n;

	s=data;
	e=s+bytes;
	init=1;
	k.data=NULL;
	while(s<e)
	{
		n=wtk_utf8_bytes(*s);
		if(init==1)
		{
			if(n>1 || !isspace(*s))
			{
				k.data=s;
				init=0;
			}
		}else
		{
			if(n==1 && isspace(*s))
			{
				k.len=s-k.data;
				//wtk_debug("%.*s\n",k.len,k.data);
				wtk_chnpos_feed(pos,&k);
				k.data=NULL;
				init=1;
			}
		}
		s+=n;
	}
	if(init==0 && k.data)
	{
		k.len=e-k.data;
		//wtk_debug("%.*s\n",k.len,k.data);
		wtk_chnpos_feed(pos,&k);
	}
	wtk_debug("inst=%d %p\n",pos->inst_q.length,pos->max_inst);
	wtk_chnpos_print(pos);
}


