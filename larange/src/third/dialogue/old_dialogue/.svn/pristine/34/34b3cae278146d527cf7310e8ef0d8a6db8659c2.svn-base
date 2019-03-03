#include "wtk_clsvec2.h" 

int wtk_clsvec2_run_thread(wtk_clsvec2_t *v,wtk_thread_t *t);

wtk_clsvec2_t* wtk_clsvec2_new(int nthread)
{
	wtk_clsvec2_t *v;
	int i;
#ifdef WIN32
	SYSTEM_INFO si;
#endif

	v=(wtk_clsvec2_t*)wtk_malloc(sizeof(wtk_clsvec2_t));
	v->dat=NULL;
	v->loc_heap=wtk_heap_new(4096);
	if(nthread==-1)
	{
#ifdef __ANDROID__
		v->nthread=4;
#elif WIN32
		GetSystemInfo(&si);
		v->nthread = si.dwNumberOfProcessors;
#else
		v->nthread=wtk_get_cpus();
#endif
	}else
	{
		v->nthread=nthread;
	}
	//v->nthread=2;
	//v->nthread=1;
	if(v->nthread>1)
	{
		v->run=1;
		v->threads=(wtk_thread_t*)wtk_malloc(sizeof(wtk_thread_t)*v->nthread);
		v->thread_q=(wtk_queue_t*)wtk_malloc(sizeof(wtk_queue_t)*v->nthread);
		v->sem=(wtk_sem_t*)wtk_malloc(sizeof(wtk_sem_t)*v->nthread);
		v->notify_sem=(wtk_sem_t*)wtk_malloc(sizeof(wtk_sem_t)*v->nthread);
		for(i=0;i<v->nthread;++i)
		{
			wtk_sem_init(&(v->sem[i]),0);
			wtk_sem_init(&(v->notify_sem[i]),0);
			wtk_queue_init(&(v->thread_q[i]));
			wtk_thread_init(v->threads+i,(thread_route_handler)wtk_clsvec2_run_thread,v);
			wtk_thread_start(v->threads+i);
		}
	}
	return v;
}

void wtk_clsvec2_delete(wtk_clsvec2_t *v)
{
	int i;

	if(v->nthread>0)
	{
		wtk_free(v->thread_q);
		for(i=0;i<v->nthread;++i)
		{
			wtk_thread_clean(v->threads+i);
			wtk_sem_clean(&(v->sem[i]));
			wtk_sem_clean(&(v->notify_sem[i]));
		}
		wtk_free(v->threads);
		wtk_free(v->sem);
		wtk_free(v->notify_sem);
	}
	if(v->dat)
	{
		wtk_clsvec_dat_delete(v->dat);
	}
	wtk_heap_delete(v->loc_heap);
	wtk_free(v);
}


wtk_clsvec2_class_t* wtk_clsvec2_new_class(wtk_clsvec2_t *v)
{
	wtk_heap_t *heap=v->dat->heap;
	wtk_clsvec2_class_t *cls;
	int i;

	cls=(wtk_clsvec2_class_t*)wtk_heap_malloc(heap,sizeof(wtk_clsvec2_class_t));
	wtk_queue_init(&(cls->item_q));
	//cls->vec=wtk_vecf_new(v->vsize);
	wtk_sfvec_init(&(cls->vec));
	//wtk_vecf_zero(cls->vec);
	cls->thread_q=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t)*v->nthread);
	for(i=0;i<v->nthread;++i)
	{
		wtk_queue_init(&(cls->thread_q[i]));
	}
	return cls;
}

void wtk_clsvec2_init_random_class(wtk_clsvec2_t *v,wtk_clsvec2_class_t *cls)
{
	int idx;
	wtk_queue_node_t *qn;
	wtk_queue_t *q=&(v->dat->item_q);
	wtk_clsvec2_item_t *item;
	wtk_svec_item_t *vi,*vi2;
	wtk_heap_t *heap=v->loc_heap;

	idx=wtk_random(0,q->length-1);
	//wtk_debug("idx=%d/%d\n",idx,v->item_q.length);
	qn=wtk_queue_peek(q,idx);
	//wtk_queue_remove(q,qn);
	item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
	//wtk_queue_push(&(cls->item_q),qn);
	for(qn=item->vec.item_q.pop;qn;qn=qn->next)
	{
		vi=data_offset2(qn,wtk_svec_item_t,q_n);

		vi2=wtk_heap_malloc(heap,sizeof(wtk_svec_item_t));
		vi2->i=vi->i;
		vi2->v=vi->v;
		wtk_sfvec_add_value(&(cls->vec),vi2);
	}
}

void wtk_clsvec2_init_class(wtk_clsvec2_t *v)
{
	wtk_heap_t *heap=v->dat->heap;
	int i;

	v->clses=(wtk_clsvec2_class_t**)wtk_heap_malloc(heap,sizeof(wtk_clsvec2_class_t*)*v->ncls);
	for(i=0;i<v->ncls;++i)
	{
		v->clses[i]=wtk_clsvec2_new_class(v);
		wtk_clsvec2_init_random_class(v,v->clses[i]);
	}

	v->list_q=(wtk_queue_t**)wtk_heap_malloc(heap,sizeof(wtk_queue_t*)*v->ncls);
	for(i=0;i<v->ncls;++i)
	{
		v->list_q[i]=(wtk_queue_t*)wtk_heap_malloc(heap,sizeof(wtk_queue_t));
		wtk_queue_init(v->list_q[i]);
	}
}


int wtk_clsvec2_reclass(wtk_clsvec2_t *v)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_clsvec2_item_t *item,*item2;
	double f;
	double min=1000;
	int idx;
	int i,j;
	wtk_clsvec2_class_t *cls;
	wtk_svec_item_t *si;
	int ki=0;
	int b=0;
	wtk_vecf_t *vf;
	int xki=0;

	for(qn=v->dat->item_q.pop;qn;qn=qn->next)
	{
		++ki;
		if(ki%1000==0)
		{
			wtk_debug("iter=%d, ki=%d\n",v->iter,ki);
		}
		item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
		if(v->iter!=1)
		{
			if(item->index!=xki)
			{
				wtk_debug("found bug %d/%d\n",item->index,xki);
				exit(0);
			}
			++xki;
		}
		idx=-1;
		for(i=0;i<v->ncls;++i)
		{
			f=wtk_sfvec_dist(&(v->clses[i]->vec),&(item->vec));
			//wtk_debug("v[%d]=%f\n",i,f);
			//exit(0);
			if(idx==-1 || f<min)
			{
				min=f;
				idx=i;
			}
		}
		//wtk_debug("idx=%d,min=%f\n",idx,min);
		wtk_queue_push(&(v->clses[idx]->item_q),&(item->item_n));
		//exit(0);
	}
	b=1;
	for(i=0;i<v->ncls;++i)
	{
		if(v->clses[i]->item_q.length==v->list_q[i]->length)
		{
			for(qn=v->clses[i]->item_q.pop,qn2=v->list_q[i]->pop;qn;qn=qn->next,qn2=qn2->next)
			{
				item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
				item2=data_offset2(qn2,wtk_clsvec2_item_t,list_n);
				if(item!=item2)
				{
					wtk_debug("v[%d]=%d/%d\n",i,item->index,item2->index);
					b=0;
					break;
				}
			}
			if(b==0)
			{
				break;
			}
		}else
		{
			wtk_debug("item not equal v[%d]=%d/%d\n",i,v->clses[i]->item_q.length,v->list_q[i]->length);
			b=0;
			break;
		}
	}
	if(b)
	{
		return 1;
	}
	wtk_heap_reset(v->loc_heap);
	vf=wtk_vecf_new(v->dat->vsize);
	for(i=0;i<v->ncls;++i)
	{
		wtk_svec_item_t *vi2;

		cls=v->clses[i];
		wtk_debug("class[%d]=%d\n",i,cls->item_q.length);
		wtk_sfvec_init(&(cls->vec));
		wtk_vecf_zero(vf);
		wtk_queue_init(v->list_q[i]);
		while(1)
		{
			qn=wtk_queue_pop(&(cls->item_q));
			if(!qn){break;}
			item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
			wtk_queue_push(v->list_q[i],&(item->list_n));
			for(qn2=item->vec.item_q.pop;qn2;qn2=qn2->next)
			{
				si=data_offset2(qn2,wtk_svec_item_t,q_n);
				vf->p[si->i]+=si->v;
			}
			//wtk_queue_push(&(v->item_q),qn);
		}
		for(j=0;j<vf->len;++j)
		{
			if(vf->p[j]!=0)
			{
				vi2=wtk_heap_malloc(v->loc_heap,sizeof(wtk_svec_item_t));
				vi2->i=j;
				vi2->v=vf->p[j];
				wtk_sfvec_add_value(&(cls->vec),vi2);
			}
		}
		wtk_sfvec_norm(&(cls->vec));
	}
	wtk_vecf_delete(vf);
	return 0;
}

int wtk_clsvec2_run_thread(wtk_clsvec2_t *v,wtk_thread_t *t)
{
	int tidx=0;
	int idx;
	int i;
	wtk_queue_t *q;
	wtk_queue_node_t *qn;
	wtk_clsvec2_item_t *item;
	double f;
	double min=100;
	int ki;

	for(i=0;i<v->nthread;++i)
	{
		if(v->threads+i==t)
		{
			tidx=i;
			break;
		}
	}
	q=&(v->thread_q[tidx]);
	while(v->run)
	{
		wtk_sem_acquire(&(v->sem[tidx]),-1);
		if(!v->run){break;}
		wtk_debug("get idx=%d len=%d\n",tidx,q->length);
		ki=0;
		for(qn=q->pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
			++ki;
			if(ki%1000==0)
			{
				wtk_debug("iter=%d thread=%d ki=%d\n",v->iter,tidx,ki);
			}
			idx=-1;
			for(i=0;i<v->ncls;++i)
			{
				f=wtk_sfvec_dist(&(v->clses[i]->vec),&(item->vec));
				//wtk_debug("v[%d]=%f\n",i,f);
				//exit(0);
				if(idx==-1 || f<min)
				{
					min=f;
					idx=i;
				}
			}
			//wtk_debug("idx=%d,min=%f\n",idx,min);
			wtk_queue_push(&(v->clses[idx]->thread_q[tidx]),&(item->item_n));
			//exit(0);
		}
		wtk_sem_release(&(v->notify_sem[tidx]),1);
	}
	return 0;
}

void wtk_clsvec2_start_thread(wtk_clsvec2_t *v)
{
	int i;

	for(i=0;i<v->nthread;++i)
	{
		wtk_sem_release(&(v->sem[i]),1);
	}
}

void wtk_clsvec2_join_thread(wtk_clsvec2_t *v)
{
	int i;

	for(i=0;i<v->nthread;++i)
	{
		wtk_sem_acquire(&(v->notify_sem[i]),-1);
	}
}

void wtk_clsve2_close_thread(wtk_clsvec2_t *v)
{
	int i;

	v->run=0;
	for(i=0;i<v->nthread;++i)
	{
		wtk_sem_release(&(v->sem[i]),1);
		wtk_thread_join(&(v->threads[i]));
	}
}

void wtk_clsvec2_print_item(wtk_clsvec2_t *v)
{
	int i,j;
	int nx;

	for(i=0;i<v->nthread;++i)
	{
		nx=0;
		for(j=0;j<v->ncls;++j)
		{
			//wtk_debug("i=%d/%d\n",i,j);
			nx+=v->clses[j]->thread_q[i].length;
		}
		wtk_debug("v[%d]=%d\n",i,nx);
	}
	//exit(0);
}

int wtk_clsvec2_reclass2(wtk_clsvec2_t *v)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_clsvec2_item_t *item,*item2;
	int i,j;
	wtk_clsvec2_class_t *cls;
	wtk_svec_item_t *si;
	int b;
	wtk_vecf_t *vf;
	wtk_queue_t *q,*q2;
	//int nx=0;

	wtk_clsvec2_start_thread(v);
	wtk_clsvec2_join_thread(v);
	//wtk_clsvec2_print_item(v);
	b=1;
	for(i=0;i<v->ncls;++i)
	{
		q=&(v->clses[i]->item_q);
		wtk_queue_init(q);
		for(j=0;j<v->nthread;++j)
		{
			q2=&(v->clses[i]->thread_q[j]);
			//wtk_debug("v[%d][%d]=%d\n",i,j,q2->length);
			while(1)
			{
				qn=wtk_queue_pop(q2);
				if(!qn){break;}
				wtk_queue_push(q,qn);
			}
			wtk_queue_init(q2);
		}
		//nx+=v->clses[i]->item_q.length;
		//wtk_debug("item=%d/%d\n",v->clses[i]->item_q.length,v->list_q[i]->length);
		if(b)
		{
			if(v->clses[i]->item_q.length==v->list_q[i]->length)
			{
				for(qn=v->clses[i]->item_q.pop,qn2=v->list_q[i]->pop;qn;qn=qn->next,qn2=qn2->next)
				{
					item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
					item2=data_offset2(qn2,wtk_clsvec2_item_t,list_n);
					if(item!=item2)
					{
						wtk_debug("v[%d]=%d/%d\n",i,item->index,item2->index);
						b=0;
						break;
					}
				}
			}else
			{
				wtk_debug("item not equal v[%d]=%d/%d\n",i,v->clses[i]->item_q.length,v->list_q[i]->length);
				b=0;
				//break;
			}
		}
	}
	//exit(0);
	if(b)
	{
		return 1;
	}
	wtk_clsvec2_print(v);
	wtk_heap_reset(v->loc_heap);
	vf=wtk_vecf_new(v->dat->vsize);
	for(i=0;i<v->ncls;++i)
	{
		wtk_svec_item_t *vi2;

		cls=v->clses[i];
		wtk_debug("iter=%d class[%d]=%d\n",v->iter,i,cls->item_q.length);
		wtk_sfvec_init(&(cls->vec));
		wtk_vecf_zero(vf);
		wtk_queue_init(v->list_q[i]);
		while(1)
		{
			qn=wtk_queue_pop(&(cls->item_q));
			if(!qn){break;}
			item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
			wtk_queue_push(v->list_q[i],&(item->list_n));
			for(qn2=item->vec.item_q.pop;qn2;qn2=qn2->next)
			{
				si=data_offset2(qn2,wtk_svec_item_t,q_n);
				vf->p[si->i]+=si->v;
			}
			//wtk_queue_push(&(v->item_q),qn);
		}
		for(j=0;j<vf->len;++j)
		{
			if(vf->p[j]!=0)
			{
				vi2=wtk_heap_malloc(v->loc_heap,sizeof(wtk_svec_item_t));
				vi2->i=j;
				vi2->v=vf->p[j];
				wtk_sfvec_add_value(&(cls->vec),vi2);
			}
		}
		wtk_sfvec_norm(&(cls->vec));
	}
	wtk_vecf_delete(vf);
	//exit(0);
	return 0;
}

void wtk_clsvec2_init_thread(wtk_clsvec2_t *v)
{
	wtk_queue_node_t *qn;
	//wtk_clsvec2_item_t *item;
	int step;
	int idx=0;
	wtk_queue_t *q;
	int nx=v->nthread-1;
	wtk_queue_t *q2=&(v->dat->item_q);

	step=q2->length/v->nthread;
	wtk_debug("len=%d step=%d\n",q2->length,step);
	q=&(v->thread_q[0]);
	while(1)
	{
		qn=wtk_queue_pop(q2);
		if(!qn){break;}
		//item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
		wtk_queue_push(q,qn);
		if(q->length==step && idx<nx)
		{
			++idx;
			//wtk_debug("idx=%d len=%d step=%d\n",idx,q->length,step);
			q=&(v->thread_q[idx]);
		}
	}
//	{
//		int i;
//		int nx=0;
//
//		for(i=0;i<v->nthread;++i)
//		{
//			wtk_debug("v[%d]=%d\n",i,v->thread_q[i].length);
//			nx+=v->thread_q[i].length;
//		}
//		wtk_debug("nx=%d\n",nx);
//	}
	//exit(0);
}

void wtk_clsvec2_process(wtk_clsvec2_t *v,char *fn,int ncls)
{
	int b;

	v->iter=0;
	v->dat=wtk_clsvec_dat_new(fn);
	v->ncls=ncls;
	wtk_clsvec2_init_class(v);
	if(v->nthread>1)
	{
		wtk_clsvec2_init_thread(v);
	}
	v->run=1;
	while(1)
	{
		++v->iter;
		if(v->nthread==1)
		{
			b=wtk_clsvec2_reclass(v);
		}else
		{
			b=wtk_clsvec2_reclass2(v);
		}
//		if(v->iter==3)
//		{
//			break;
//		}
		if(b)
		{
			break;
		}

	}
	if(v->nthread>1)
	{
		wtk_clsve2_close_thread(v);
	}
	wtk_clsvec2_print(v);
}

void wtk_clsvec2_print(wtk_clsvec2_t *v)
{
	wtk_queue_node_t *qn;
	wtk_clsvec2_item_t *item;
	int i;
	FILE *f;
	char buf[256];
	char *dn="tmp";

	wtk_mkdir_p(dn,'/',1);
	wtk_debug("========== save class =========\n");
	for(i=0;i<v->ncls;++i)
	{
		//printf("=============== class=%d =================\n",i);
		sprintf(buf,"%s/%d.cls.%d",dn,v->iter,i);
		f=fopen(buf,"w");
		for(qn=v->clses[i]->item_q.pop;qn;qn=qn->next)
		{
			item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
			fprintf(f,"%.*s\n",item->name->len,item->name->data);
		}
		fclose(f);
	}
}

