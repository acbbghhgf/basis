#include "wtk_clsvec.h" 

wtk_clsvec_t* wtk_clsvec_new(wtk_clsvec_cfg_t *cfg)
{
	wtk_clsvec_t *v;

	v=(wtk_clsvec_t*)wtk_malloc(sizeof(wtk_clsvec_t));
	v->cfg=cfg;
	v->heap=wtk_heap_new(4096);
	v->vsize=50;
	v->sse_thresh=10;
	v->idx_name=NULL;
	v->glb_heap=wtk_heap_new(4096);
	wtk_queue_init(&(v->output_q));
	wtk_queue_init(&(v->cls_q));
	wtk_queue_init(&(v->item_q));
	return v;
}

void wtk_clsvec_delete(wtk_clsvec_t *v)
{
	if(v->glb_heap)
	{
		wtk_heap_delete(v->glb_heap);
	}
	wtk_heap_delete(v->heap);
	wtk_free(v);
}

wtk_clsvec_item_t* wtk_clsvec_item_new(wtk_heap_t *heap,char *nm,int len,int vlen)
{
	wtk_clsvec_item_t *item;

	item=(wtk_clsvec_item_t*)wtk_heap_malloc(heap,sizeof(wtk_clsvec_item_t));
	item->v=wtk_vecf_new(vlen);
	item->str=wtk_heap_dup_string(heap,nm,len);
	return item;
}

#include "wtk/core/wtk_str_parser.h"

int wtk_clsvec_load_idx(wtk_clsvec_t *v,wtk_source_t *src)
{
	int ret;
	wtk_strbuf_t *buf;
	wtk_clsvec_item_t *item;
	wtk_heap_t *heap=v->glb_heap;
	wtk_strkv_parser_t kp;
	int i,nl;
	float xv;
	char *p;

	wtk_queue_init(&(v->item_q));
	buf=wtk_strbuf_new(256,1);
	ret=wtk_source_read_line(src,buf);
	if(ret!=0){goto end;}
	//wtk_debug("[%.*s]\n",buf->pos,buf->data);
	wtk_strkv_parser_init(&(kp),buf->data,buf->pos);
	while(1)
	{
		ret=wtk_strkv_parser_next(&(kp));
		if(ret!=0){break;}
		if(wtk_str_equal_s(kp.k.data,kp.k.len,"vec"))
		{
			v->vsize=wtk_str_atoi(kp.v.data,kp.v.len);
		}else
		{
			wtk_debug("unsported [%.*s]=[%.*s]\n",kp.k.len,kp.k.data,kp.v.len,kp.v.data);
			exit(0);
		}
	}
	//wtk_debug("v=%d\n",v->vsize);
	v->idx_name=(wtk_string_t**)wtk_heap_malloc(heap,v->vsize*sizeof(wtk_string_t*));
	for(i=0;i<v->vsize;++i)
	{
		ret=wtk_source_read_line(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		v->idx_name[i]=wtk_heap_dup_string(heap,buf->data,buf->pos);
	}
	while(1)
	{
		ret=wtk_source_read_line(src,buf);
		if(ret!=0){goto end;}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		item=wtk_clsvec_item_new(heap,buf->data,buf->pos,v->vsize);
		wtk_vecf_zero(item->v);
		while(1)
		{
			ret=wtk_source_skip_sp(src,&nl);
			if(nl){break;}
			ret=wtk_source_read_string(src,buf);
			if(ret!=0){goto end;}
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
			p=wtk_str_chr(buf->data,buf->pos,':');
			//wtk_debug("[%.*s]\n",(int)(p-buf->data),buf->data);
			i=wtk_str_atoi(buf->data,(int)(p-buf->data));
			//wtk_debug("i=%d\n",i);
			++p;
			//wtk_debug("[%.*s]\n",(int)(buf->data+buf->pos-p),p);
			xv=wtk_str_atof(p,(int)(buf->data+buf->pos-p));
			item->v->p[i]=xv;
			//exit(0);
		}
		//exit(0);
	}
	ret=0;
end:
	wtk_debug("ret=%d\n",ret);
	exit(0);
	wtk_strbuf_delete(buf);
	return ret;
}

int wtk_clsvec_load(wtk_clsvec_t *v,wtk_source_t *src)
{
	int ret;
	wtk_strbuf_t *buf;
	wtk_clsvec_item_t *item;
	wtk_heap_t *heap=v->heap;

	wtk_queue_init(&(v->item_q));
	buf=wtk_strbuf_new(256,1);
	while(1)
	{
		ret=wtk_source_skip_sp(src,NULL);
		if(ret!=0)
		{
			goto end;
		}
		ret=wtk_source_read_line(src,buf);
		if(ret!=0 || buf->pos==0)
		{
			ret=0;goto end;
		}
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		wtk_strbuf_strip(buf);
		item=wtk_clsvec_item_new(heap,buf->data,buf->pos,v->vsize);
		ret=wtk_source_read_float(src,item->v->p,v->vsize,0);
		if(ret!=0)
		{
			wtk_debug("read float failed\n");
			goto end;
		}
		wtk_queue_push(&(v->item_q),&(item->q_n));
	}
	ret=0;
end:
	wtk_strbuf_delete(buf);
	return ret;
}

wtk_clsvec_cls_item_t* wtk_clsvec_cls_item_new(wtk_heap_t *heap,int vsize)
{
	wtk_clsvec_cls_item_t *item;

	item=(wtk_clsvec_cls_item_t*)wtk_heap_malloc(heap,sizeof(wtk_clsvec_cls_item_t));
	wtk_queue_init(&(item->item_q));
	item->v=wtk_vecf_new(vsize);
	item->sse=0;
	return item;
}

wtk_clsvec_cls_item_t* wtk_clsvec_init_class(wtk_clsvec_t *v)
{
	wtk_clsvec_cls_item_t *cls_item;
	wtk_clsvec_item_t *item;
	wtk_queue_node_t *qn;

	cls_item=wtk_clsvec_cls_item_new(v->heap,v->vsize);
	while(1)
	{
		qn=wtk_queue_pop(&(v->item_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_clsvec_item_t,q_n);
		wtk_queue_push(&(cls_item->item_q),&(item->q_n));
	}
	return cls_item;
}

wtk_clsvec_cls_item_t* wtk_clsvec_cls_get_random_cls(wtk_clsvec_t *v,wtk_clsvec_cls_item_t *cls2)
{
	wtk_clsvec_cls_item_t *cls;
	wtk_queue_node_t *qn;
	wtk_clsvec_item_t *item;
	int i;

	i=wtk_random(0,cls2->item_q.length-1);
	cls=wtk_clsvec_cls_item_new(v->heap,v->vsize);
	qn=wtk_queue_peek(&(cls2->item_q),i);
	wtk_queue_remove(&(cls2->item_q),qn);

	item=data_offset2(qn,wtk_clsvec_item_t,q_n);
	wtk_queue_push(&(cls->item_q),qn);
	wtk_vecf_cpy(cls->v,item->v);
	return cls;
}

void wtk_clsvec_cls_item_add_item(wtk_clsvec_cls_item_t *cls,wtk_clsvec_item_t *item)
{
	int n=cls->item_q.length;
	int i;
	float *fp1,*fp2;
	double s;

	fp1=cls->v->p;
	fp2=item->v->p;
	s=1.0/(n+1);
	for(i=0;i<cls->v->len;++i)
	{
		fp1[i]=(fp1[i]*n+fp2[i])*s;
	}
	wtk_queue_push(&(cls->item_q),&(item->q_n));
	wtk_vecf_norm(item->v);
}

void wtk_clsvec_cls_item_update_sse(wtk_clsvec_cls_item_t *cls)
{
	wtk_queue_node_t *qn;
	wtk_clsvec_item_t *item;
	double f;

	f=0;
	for(qn=cls->item_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_clsvec_item_t,q_n);
		//wtk_vecf_print(qa->v);
		//wtk_debug("len=%d\n",qa->v->len)
		f+=wtk_vecf_dist(cls->v,item->v);
		//wtk_debug("f=%f\n",f);
		//exit(0);
	}
	cls->sse=f;
}

void wtk_clsvec_add_cls(wtk_queue_t *q,wtk_clsvec_cls_item_t *cls)
{
	wtk_queue_node_t *qn,*prev;
	wtk_clsvec_cls_item_t *item;

	prev=NULL;
	for(qn=q->push;qn;qn=qn->prev)
	{
		item=data_offset2(qn,wtk_clsvec_cls_item_t,q_n);
		//wtk_debug("sse=%f/%f\n",item->sse,cls->sse);
		if(item->sse<=cls->sse)
		{
			prev=qn;
			break;
		}
	}
	//wtk_debug("========== sse=%f ==========\n",cls->sse);
	if(prev)
	{
		wtk_queue_insert_to(q,prev,&(cls->q_n));
	}else
	{
		wtk_queue_push_front(q,&(cls->q_n));
	}
}

void wtk_clsvec_process_cls(wtk_clsvec_t *v,wtk_clsvec_cls_item_t *cls)
{
	float thresh=v->sse_thresh;

	wtk_clsvec_cls_item_update_sse(cls);
	//wtk_debug("item=%d sse=%f\n",cls->item_q.length,cls->sse);
	if(cls->sse>thresh)
	{
		wtk_clsvec_add_cls(&(v->cls_q),cls);
	}else
	{
		wtk_clsvec_add_cls(&(v->output_q),cls);
	}
}

void  wtk_clsvec_process_kmeans(wtk_clsvec_t *v,wtk_clsvec_cls_item_t *cls)
{
	wtk_clsvec_cls_item_t *cls1,*cls2;
	wtk_queue_node_t *qn;
	wtk_clsvec_item_t *item;
	double f1,f2;
	wtk_queue_t q;

	if(cls->item_q.length<2)
	{
		wtk_debug("found bug\n");
		exit(0);
		return;
	}
	cls1=wtk_clsvec_cls_get_random_cls(v,cls);
	cls2=wtk_clsvec_cls_get_random_cls(v,cls);
	while(1)
	{
		qn=wtk_queue_pop(&(cls->item_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_clsvec_item_t,q_n);
		f1=wtk_vecf_dist(item->v,cls1->v);
		f2=wtk_vecf_dist(item->v,cls2->v);
		//wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
		if(f1<f2)
		{
			wtk_clsvec_cls_item_add_item(cls1,item);
		}else
		{
			wtk_clsvec_cls_item_add_item(cls2,item);
		}
	}

	wtk_queue_init(&(q));
	q=cls1->item_q;
	wtk_queue_init(&(cls1->item_q));
	while(1)
	{
		qn=wtk_queue_pop(&(q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_clsvec_item_t,q_n);
		f1=wtk_vecf_dist(item->v,cls1->v);
		f2=wtk_vecf_dist(item->v,cls2->v);
		//wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
		if(f1<f2)
		{
			wtk_queue_push(&(cls1->item_q),&(item->q_n));
			//wtk_faq_cls_item_add(cls1,item);
		}else
		{
			wtk_queue_push(&(cls2->item_q),&(item->q_n));
		}
	}
	q=cls2->item_q;
	wtk_queue_init(&(cls2->item_q));
	while(1)
	{
		qn=wtk_queue_pop(&(q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_clsvec_item_t,q_n);
		f1=wtk_vecf_dist(item->v,cls1->v);
		f2=wtk_vecf_dist(item->v,cls2->v);
		//wtk_debug("f1=%f f2=%f [%.*s]\n",f1,f2,item->q->len,item->q->data);
		if(f1<f2)
		{
			wtk_queue_push(&(cls1->item_q),&(item->q_n));
			//wtk_faq_cls_item_add(cls1,item);
		}else
		{
			wtk_queue_push(&(cls2->item_q),&(item->q_n));
		}
	}

	wtk_clsvec_process_cls(v,cls1);
	wtk_clsvec_process_cls(v,cls2);
}

void wtk_clsvec_process(wtk_clsvec_t *v,char *fn)
{
	wtk_clsvec_cls_item_t *cls;
	wtk_queue_node_t *qn;

	wtk_debug("load %s\n",fn);
	if(v->cfg->use_idx)
	{
		wtk_source_load_file(v,(wtk_source_load_handler_t)wtk_clsvec_load_idx,fn);
	}else
	{
		wtk_source_load_file(v,(wtk_source_load_handler_t)wtk_clsvec_load,fn);
	}
	wtk_debug("load item=%d\n",v->item_q.length);
	wtk_queue_init(&(v->cls_q));
	cls=wtk_clsvec_init_class(v);
	wtk_clsvec_process_kmeans(v,cls);
	while(1)
	{
		qn=wtk_queue_pop_back(&(v->cls_q));
		if(!qn){break;}
		cls=data_offset2(qn,wtk_clsvec_cls_item_t,q_n);
		wtk_clsvec_process_kmeans(v,cls);
	}
}

void wtk_clsvec_print(wtk_clsvec_t *v)
{
	wtk_queue_t *q=&(v->output_q);
	wtk_queue_node_t *qn,*qn2;
	wtk_clsvec_cls_item_t *cls;
	wtk_clsvec_item_t *item;

	printf("cls=%d\n",v->output_q.length);
	for(qn=q->pop;qn;qn=qn->next)
	{
		cls=data_offset2(qn,wtk_clsvec_cls_item_t,q_n);
		printf("%f %d\n",cls->sse,cls->item_q.length);
		for(qn2=cls->item_q.pop;qn2;qn2=qn2->next)
		{
			item=data_offset2(qn2,wtk_clsvec_item_t,q_n);
			printf("%.*s\n",item->str->len,item->str->data);
		}
	}
}
