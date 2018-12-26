#include "wtk_chnpos_model.h"



wtk_chnpos_model_t* wtk_chnpos_model_new()
{
	wtk_chnpos_model_t *m;

	m=(wtk_chnpos_model_t*)wtk_malloc(sizeof(wtk_chnpos_model_t));
	m->pos_map=wtk_str_hash_new(137);
	m->npos=0;
	m->pos_a=wtk_larray_new(64,sizeof(wtk_string_t*));
	m->state_robin=NULL;
	//wtk_queue_init(&(m->state_q));
	m->wrd_map=NULL;
	m->state_map=NULL;//wtk_str_hash_new(4703);
	return m;
}

void wtk_chnpos_model_delete(wtk_chnpos_model_t *m)
{
	wtk_larray_delete(m->pos_a);
	wtk_str_hash_delete(m->pos_map);
	if(m->wrd_map)
	{
		wtk_str_hash_delete(m->wrd_map);
	}
	if(m->state_map)
	{
		wtk_hash_delete(m->state_map);
	}
	if(m->state_robin)
	{
		wtk_robin_delete(m->state_robin);
	}
	wtk_free(m);
}


wtk_chnpos_bmes_t wtk_chnpos_bmes_parse(char *s,int bytes)
{
	if(wtk_str_equal_s(s,bytes,"B"))
	{
		return WTK_CHNPOS_B;
	}else if(wtk_str_equal_s(s,bytes,"M"))
	{
		return WTK_CHNPOS_M;
	}else if(wtk_str_equal_s(s,bytes,"E"))
	{
		return WTK_CHNPOS_E;
	}else //else if(wtk_str_equal_s(s,bytes,"S"))
	{
		return WTK_CHNPOS_S;
	}
}

unsigned char wtk_chnpos_model_pos_parse(wtk_chnpos_model_t *m,char *pos,int pos_bytes,int insert)
{
	wtk_hash_str_node_t *node;

	node=(wtk_hash_str_node_t*)wtk_str_hash_find_node3(m->pos_map,pos,pos_bytes,insert);
	if(!node)
	{
		return 0;
	}
	if(node->v.u==0)
	{
		wtk_string_t *v;

		v=wtk_heap_dup_string(m->pos_map->heap,pos,pos_bytes);
		wtk_larray_push2(m->pos_a,&v);
		node->v.u=++m->npos;
	}
	return node->v.u;
}

wtk_string_t* wtk_chnpos_model_get_pos_str(wtk_chnpos_model_t *m,int idx)
{
	wtk_string_t **strs;

	strs=(wtk_string_t**)(m->pos_a->slot);
	return strs[idx-1];
}

int wtk_chnpos_model_get_pos_walk(void *data,wtk_hash_str_node_t *node)
{
	void **p;
	int *pn;

	p=(void**)data;
	pn=p[0];
	if(node->v.u==*pn)
	{
		p[1]=&(node->key);
		return -1;
	}else
	{
		return 0;
	}
}

wtk_string_t* wtk_chnpos_model_get_pos(wtk_chnpos_model_t *m,int n)
{
	void *p[2];

	p[0]=&n;
	p[1]=NULL;
	wtk_str_hash_walk(m->pos_map,(wtk_walk_handler_t)wtk_chnpos_model_get_pos_walk,p);
	return (wtk_string_t*)p[1];
}

char* wtk_bmes_to_str(int bmes)
{
	char *s=NULL;

	switch(bmes)
	{
	case WTK_CHNPOS_B:
		s="B";
		break;
	case WTK_CHNPOS_M:
		s="M";
		break;
	case WTK_CHNPOS_E:
		s="E";
		break;
	case WTK_CHNPOS_S:
		s="S";
		break;
	}
	return s;
}

void wtk_chnpos_state_print(wtk_chnpos_model_t *m,wtk_chnpos_state_t *item)
{
	wtk_string_t *k;
	int i;
	char *s;

	s=wtk_bmes_to_str(item->bmes);
	printf("%s ",s);
	k=wtk_chnpos_model_get_pos(m,item->pos);
	printf("%.*s %f\n",k->len,k->data,item->start);
	for(i=0;i<item->narc;++i)
	{
		k=wtk_chnpos_model_get_pos(m,item->arcs[i].to->pos);
		//printf("arc[%d/%d]=%s %.*s %f\n",i,item->narc,wtk_bmes_to_str(item->arcs[i].to->bmes),k->len,k->data,item->arcs[i].prob);
		if(0)
		{
			printf("%s %.*s %f\n",wtk_bmes_to_str(item->arcs[i].to->bmes),k->len,k->data,item->arcs[i].prob);
		}
	}
}

void wtk_chnpos_wrd_print(wtk_chnpos_model_t *m,wtk_chnpos_wrd_t *wrd)
{
	int i;

	printf("%.*s %d\n",wrd->wrd.len,wrd->wrd.data,wrd->nstate);
	for(i=0;i<wrd->nstate;++i)
	{
		wtk_chnpos_state_print(m,wrd->states[i]);
	}
}

wtk_chnpos_wrd_t* wtk_chnpos_model_new_wrd(wtk_chnpos_model_t *m,char *wrd,int wrd_bytes,int nstate)
{
	wtk_heap_t *heap=m->wrd_map->heap;
	wtk_chnpos_wrd_t *w;

//	if(wtk_str_equal_s(wrd,wrd_bytes,"囧"))
//	{
//		wtk_debug("[%.*s]=%d\n",wrd_bytes,wrd,nstate);
//	}
	w=(wtk_chnpos_wrd_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_wrd_t));
	//w->wrd=wtk_heap_dup_string(heap,wrd,wrd_bytes);
	wtk_heap_fill_string(heap,&(w->wrd),wrd,wrd_bytes);
	w->nstate=nstate;
	w->states=(wtk_chnpos_state_t**)wtk_heap_malloc(heap,sizeof(wtk_chnpos_state_t*)*nstate);
	wtk_str_hash_add(m->wrd_map,w->wrd.data,w->wrd.len,w);
	return w;
}

wtk_chnpos_wrd_t* wtk_chnpos_model_get_wrd(wtk_chnpos_model_t *m,char *wrd,int wrd_bytes)
{
	return (wtk_chnpos_wrd_t*)wtk_str_hash_find(m->wrd_map,wrd,wrd_bytes);
}


wtk_chnpos_wrd_t wtk_chnpos_model_get_wrd2(wtk_chnpos_model_t *m,char *wrd,int wrd_bytes)
{
	wtk_chnpos_wrd_t *w,tw;

	w=wtk_chnpos_model_get_wrd(m,wrd,wrd_bytes);
	if(w && w->nstate>0)
	{
		tw=*w;
		goto end;
	}
	wtk_string_set(&(tw.wrd),wrd,wrd_bytes);
	tw.states=(wtk_chnpos_state_t**)(m->state_robin->r);
	tw.nstate=m->state_robin->used;
end:
	return tw;
}

unsigned int wtk_chnpos_state_hash(wtk_chnpos_state_t *state,unsigned int nslot)
{
	unsigned int v;

	v=(state->pos<<8)+state->bmes;
	return v%(nslot);
}

int wtk_chnpos_state_cmp(wtk_chnpos_state_t *src,wtk_chnpos_state_t *dst)
{
	if(src->bmes!=dst->bmes)
	{
		return src->bmes-dst->bmes;
	}
	if(src->pos!=dst->pos)
	{
		return src->pos-dst->pos;
	}
	return 0;
}


wtk_chnpos_state_t* wtk_chnpos_model_get_state(wtk_chnpos_model_t *m,unsigned char bmes,unsigned char pos,int insert)
{
	wtk_heap_t *heap=m->state_map->heap;
	wtk_hash_node_t *node;
	wtk_chnpos_state_t state;
	wtk_chnpos_state_t *s;

	state.bmes=bmes;
	state.pos=pos;
	node=wtk_hash_find_node(m->state_map,&state,(wtk_hash_f)wtk_chnpos_state_hash,(wtk_cmp_f)wtk_chnpos_state_cmp);
	if(node)
	{
		s=node->v;
		goto end;
	}
	if(!insert)
	{
		s=NULL;
		goto end;
	}
	s=(wtk_chnpos_state_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_state_t));
	s->arcs=NULL;
	s->narc=0;
	s->emits=NULL;
	s->nemit=0;
	s->start=MIN_FLOAT;
	s->bmes=bmes;
	s->pos=pos;
	//wtk_queue_push(&(m->state_q),&(s->q_n));
	wtk_hash_add(m->state_map,s,(wtk_hash_f)wtk_chnpos_state_hash);
end:
	return s;
}

int wtk_chnpos_model_load_state(wtk_chnpos_model_t *m,wtk_source_t *src)
{
	wtk_chnpos_wrd_t *wrd;
	wtk_strbuf_t *buf;
	int ret;
	int num,i,j,nx;
	unsigned char bmes;
	unsigned char pos;

	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(!wtk_str_equal_s(buf->data,buf->pos,"<state>"))
	{
		wtk_debug("[%.*s] not supported\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&nx,1,0);
	if(ret!=0)
	{
		wtk_debug("read int failed.\n");
		goto end;
	}
	m->wrd_map=wtk_str_hash_new(nx/2*2+1);
	m->state_map=wtk_hash_new(257);
	for(j=0;j<nx;++j)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0)
		{
			wtk_debug("read buf failed.\n");
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		ret=wtk_source_read_int(src,&num,1,0);
		if(ret!=0)
		{
			wtk_debug("read buf failed.\n");
			goto end;
		}
		//wtk_debug("num=%d\n",num);
		wrd=wtk_chnpos_model_new_wrd(m,buf->data,buf->pos,num);
		for(i=0;i<num;++i)
		{
			ret=wtk_source_read_string(src,buf);
			if(ret!=0)
			{
				wtk_debug("read buf failed.\n");
				goto end;
			}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			bmes=wtk_chnpos_bmes_parse(buf->data,buf->pos);
			ret=wtk_source_read_string(src,buf);
			if(ret!=0)
			{
				wtk_debug("read buf failed.\n");
				goto end;
			}
			pos=wtk_chnpos_model_pos_parse(m,buf->data,buf->pos,1);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			//wtk_chnpos_state_item_print(m,item);
			wrd->states[i]=wtk_chnpos_model_get_state(m,bmes,pos,1);
		}
		//wtk_chnpos_state_print(m,state);
		//exit(0);
		//wtk_chnpos_wrd_print(m,wrd);
		//exit(0);
	}
	ret=wtk_source_read_string(src,buf);
	if(ret!=0)
	{
		wtk_debug("read buf failed.\n");
		goto end;
	}
	if(wtk_str_equal_s(buf->data,buf->pos,"</state>"))
	{
		ret=0;
		goto end;
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_chnpos_model_load_start(wtk_chnpos_model_t *m,wtk_source_t *src)
{
	wtk_strbuf_t *buf;
	int num,i;
	int ret;
	float f;
	unsigned char bmes;
	unsigned char pos;
	wtk_chnpos_state_t *state;

	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(!wtk_str_equal_s(buf->data,buf->pos,"<start>"))
	{
		wtk_debug("[%.*s] not supported\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&num,1,0);
	if(ret!=0)
	{
		wtk_debug("read int failed.\n");
		goto end;
	}
	for(i=0;i<num;++i)
	{
		ret=wtk_source_read_string(src,buf);
		if(ret!=0)
		{
			wtk_debug("read buf failed.\n");
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		bmes=wtk_chnpos_bmes_parse(buf->data,buf->pos);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0)
		{
			wtk_debug("read buf failed.\n");
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		pos=wtk_chnpos_model_pos_parse(m,buf->data,buf->pos,0);
		ret=wtk_source_read_float(src,&f,1,0);
		if(ret!=0){goto end;}
		//wtk_debug("f=%f\n",f);
		state=wtk_chnpos_model_get_state(m,bmes,pos,1);
		if(!state)
		{
			//wtk_debug("%d pos=%.*s f=%f\n",bmes,buf->pos,buf->data,f);
			//continue;
			ret=-1;
			wtk_debug("bmes=%d pos=%.*s unspported\n",bmes,wtk_chnpos_model_get_pos(m,pos)->len,wtk_chnpos_model_get_pos(m,pos)->data);
			goto end;
		}
		state->start=f;
	}
	ret=wtk_source_read_string(src,buf);
	if(ret!=0)
	{
		wtk_debug("read buf failed.\n");
		goto end;
	}
	if(wtk_str_equal_s(buf->data,buf->pos,"</start>"))
	{
		ret=0;
		goto end;
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_chnpos_model_load_trans(wtk_chnpos_model_t *m,wtk_source_t *src)
{
	wtk_heap_t *heap=m->state_map->heap;
	wtk_chnpos_arc_t *arc;
	wtk_chnpos_state_t *state;
	wtk_strbuf_t *buf;
	float f;
	int num,n;
	int i,j,ret;
	unsigned char bmes;
	unsigned char pos;

	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(!wtk_str_equal_s(buf->data,buf->pos,"<trans>"))
	{
		wtk_debug("[%.*s] not supported\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&num,1,0);
	if(ret!=0)
	{
		wtk_debug("read int failed.\n");
		goto end;
	}
	m->state_robin=wtk_robin_new(num);
	//wtk_debug("[%.*s]=%d\n",buf->pos,buf->data,num);
	for(i=0;i<num;++i)
	{
		//wtk_debug("i=%d/%d\n",i,num);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){goto end;}
		if(wtk_str_equal_s(buf->data,buf->pos,"</trans>"))
		{
			ret=0;
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		bmes=wtk_chnpos_bmes_parse(buf->data,buf->pos);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		pos=wtk_chnpos_model_pos_parse(m,buf->data,buf->pos,0);
		ret=wtk_source_read_int(src,&n,1,0);
		if(ret!=0){goto end;}
		//wtk_debug("bmes=%d pos=%d n=%d\n",bmes,pos,n);
		state=wtk_chnpos_model_get_state(m,bmes,pos,0);
		if(!state)
		{
			wtk_debug("bmes=%d pos=[%.*s] not supported\n",bmes,buf->pos,buf->data);
			ret=-1;
			goto end;
		}
		state->narc=n;
		state->arcs=(wtk_chnpos_arc_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_arc_t)*n);
		for(j=0;j<n;++j)
		{
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			bmes=wtk_chnpos_bmes_parse(buf->data,buf->pos);
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			pos=wtk_chnpos_model_pos_parse(m,buf->data,buf->pos,0);
			ret=wtk_source_read_float(src,&f,1,0);
			if(ret!=0){goto end;}
			//wtk_debug("bmes=%d pos=%d n=%f\n",bmes,pos,f);
			arc=state->arcs+j;
			arc->to=wtk_chnpos_model_get_state(m,bmes,pos,0);
			if(!state)
			{
				wtk_debug("bmes=%d pos=[%.*s] not supported\n",bmes,buf->pos,buf->data);
				ret=-1;
				goto end;
			}
			arc->prob=f;
		}
		wtk_robin_push(m->state_robin,state);
		//wtk_chnpos_state_print(m,state);
		//exit(0);
	}
	ret=wtk_source_read_string(src,buf);
	if(ret!=0)
	{
		wtk_debug("read buf failed.\n");
		goto end;
	}
	if(wtk_str_equal_s(buf->data,buf->pos,"</trans>"))
	{
		ret=0;
		goto end;
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_chnpos_model_load_emit(wtk_chnpos_model_t *m,wtk_source_t *src)
{
	wtk_heap_t *heap=m->state_map->heap;
	wtk_chnpos_emit_t *emit;
	wtk_chnpos_state_t *state;
	wtk_strbuf_t *buf;
	float f;
	int num,n;
	int i,j,ret;
	unsigned char bmes;
	unsigned char pos;

	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_string(src,buf);
	if(ret!=0){goto end;}
	if(!wtk_str_equal_s(buf->data,buf->pos,"<emit>"))
	{
		wtk_debug("[%.*s] not supported\n",buf->pos,buf->data);
		ret=-1;
		goto end;
	}
	ret=wtk_source_read_int(src,&num,1,0);
	if(ret!=0)
	{
		wtk_debug("read int failed.\n");
		goto end;
	}
	//wtk_debug("[%.*s]=%d\n",buf->pos,buf->data,num);
	for(i=0;i<num;++i)
	{
		//wtk_debug("i=%d/%d\n",i,num);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){goto end;}
		if(wtk_str_equal_s(buf->data,buf->pos,"</emit>"))
		{
			ret=0;
			goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		bmes=wtk_chnpos_bmes_parse(buf->data,buf->pos);
		ret=wtk_source_read_string(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		pos=wtk_chnpos_model_pos_parse(m,buf->data,buf->pos,0);
		ret=wtk_source_read_int(src,&n,1,0);
		if(ret!=0){goto end;}
		//wtk_debug("bmes=%d pos=%d n=%d\n",bmes,pos,n);
		state=wtk_chnpos_model_get_state(m,bmes,pos,0);
		if(!state)
		{
			wtk_debug("bmes=%d pos=[%.*s] not supported\n",bmes,buf->pos,buf->data);
			ret=-1;
			goto end;
		}
		state->nemit=n;
		state->emits=(wtk_chnpos_emit_t*)wtk_heap_malloc(heap,sizeof(wtk_chnpos_emit_t)*n);
		for(j=0;j<n;++j)
		{
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			ret=wtk_source_read_float(src,&f,1,0);
			if(ret!=0){goto end;}
			//wtk_debug("bmes=%d pos=%d n=%f\n",bmes,pos,f);
			emit=state->emits+j;
			//emit->wrd=wtk_chnpos_model_get_wrd(m,buf->data,buf->pos);
			emit->wrd=wtk_heap_dup_string(heap,buf->data,buf->pos);
			if(!emit->wrd)
			{
				wtk_debug("%.*s %f not supported\n",buf->pos,buf->data,f);
				ret=-1;
				goto end;
				//continue;
			}
			emit->prob=f;
		}
		//wtk_chnpos_state_print(m,state);
		//exit(0);
	}
	ret=wtk_source_read_string(src,buf);
	if(ret!=0)
	{
		wtk_debug("read buf failed.\n");
		goto end;
	}
	if(wtk_str_equal_s(buf->data,buf->pos,"</emit>"))
	{
		ret=0;
		goto end;
	}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	//exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}


int wtk_chnpos_model_load(wtk_chnpos_model_t *m,wtk_source_t *src)
{
	int ret;

	ret=wtk_chnpos_model_load_state(m,src);
	if(ret!=0){goto end;}
	ret=wtk_chnpos_model_load_start(m,src);
	if(ret!=0){goto end;}
	ret=wtk_chnpos_model_load_trans(m,src);
	if(ret!=0){goto end;}
	ret=wtk_chnpos_model_load_emit(m,src);
	if(ret!=0){goto end;}
	ret=0;
end:
	//wtk_debug("ret=%d\n",ret);
	return ret;
}

int wtk_chnpos_wrd_has_state(wtk_chnpos_wrd_t *wrd,wtk_chnpos_state_t *state)
{
	int i;

	for(i=0;i<wrd->nstate;++i)
	{
		if(wrd->states[i]==state)
		{
			return 1;
		}
	}
	return 0;
}

float wtk_chnpos_state_get_emit_prob(wtk_chnpos_state_t *state,wtk_string_t *wrd)
{
	int i;

	for(i=0;i<state->nemit;++i)
	{
		if(wtk_string_cmp(state->emits[i].wrd,wrd->data,wrd->len)==0)
		{
			return state->emits[i].prob;
		}
	}
	return MIN_FLOAT;

}

float wtk_chnpos_state_get_trans_prob(wtk_chnpos_state_t *state,wtk_chnpos_state_t *nxt)
{
	int i;

	for(i=0;i<state->narc;++i)
	{
		if(state->arcs[i].to==nxt)
		{
			return state->arcs[i].prob;
		}
	}
	return MIN_FLOAT;
}
