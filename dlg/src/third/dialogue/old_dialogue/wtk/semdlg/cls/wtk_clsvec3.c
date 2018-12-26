#include "wtk_clsvec3.h"

wtk_clsvec3_t* wtk_clsvec3_new()
{
	wtk_clsvec3_t *v;

	v=(wtk_clsvec3_t*)wtk_malloc(sizeof(wtk_clsvec3_t));
	v->dat=NULL;
	v->heap=wtk_heap_new(4096);
	return v;
}

void wtk_clsvec3_delete(wtk_clsvec3_t *v)
{
	wtk_heap_delete(v->heap);
	if(v->dat)
	{
		wtk_clsvec_dat_delete(v->dat);
	}
	wtk_free(v);
}



//计算包含距离
double wtk_sfvec_dist2(wtk_sfvec_t *sf1,wtk_sfvec_t *sf)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_svec_item_t *si,*si2;
	double f,t;

	f=0;
	for(qn2=sf1->item_q.pop;qn2;qn2=qn2->next)
	{
		si2=data_offset2(qn2,wtk_svec_item_t,q_n);
		t=0;
		for(qn=sf->item_q.pop;qn;qn=qn->next)
		{
			si=data_offset2(qn,wtk_svec_item_t,q_n);
			//wtk_debug("v[%d]=%d\n",i,si->i);
			if(si2->i==si->i)
			{
				//wtk_debug("v[%d]=%f\n",si->i,si->v);
				t=si->v;
				break;
			}else if(si2->i<si->i)
			{
				break;
			}
		}
		//wtk_debug("v[%d]=%f-%f\n",si2->i,si2->v,t);
		t=si2->v-t;
		f+=t*t;
	}
	//exit(0);
	return f;
}

double wtk_clsvec3_add_dist(wtk_clsvec3_t *v,double dist)
{
	wtk_clsvec2_item_t *item;
	wtk_queue_node_t *qn;
	double f;
	wtk_clsvec3_class_t *cls=&(v->cls);
	double min=1e10;
	double min2=1e10;
	int i;

	wtk_queue_init(&(cls->item_q));
	i=0;
	for(qn=v->dat->item_q.pop;qn;qn=qn->next)
	{
		++i;
//		if(i%100000==0)
//		{
//			wtk_debug("process %d/%d\n",i,v->dat->item_q.length);
//		}
		item=data_offset2(qn,wtk_clsvec2_item_t,q_n);
		f=wtk_sfvec_dist2(&(cls->vec),&(item->vec));
		//wtk_debug("f=%f\n",f);
		if(f<=dist)
		{
			wtk_queue_push(&(cls->item_q),&(item->item_n));
		}
		if(f<min)
		{
			min=f;
		}
		if(f>0 && f<min2)
		{
			min2=f;
		}
	}
	v->min=min;
	v->min2=min2;
	wtk_debug("min=%f min2=%f\n",min,min2);
	return min;
}

void wtk_clsvec3_update_vec(wtk_clsvec3_t *v)
{
	wtk_heap_t *heap=v->heap;
	wtk_clsvec3_class_t *cls=&(v->cls);
	wtk_vecf_t *vf;
	wtk_queue_node_t *qn,*qn2;
	wtk_clsvec2_item_t *item;
	wtk_svec_item_t *si;
	int j;

	wtk_sfvec_init(&(cls->vec));
	vf=wtk_vecf_new(v->dat->vsize);
	wtk_vecf_zero(vf);
	while(1)
	{
		qn=wtk_queue_pop(&(cls->item_q));
		if(!qn){break;}
		item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
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
			si=wtk_heap_malloc(heap,sizeof(wtk_svec_item_t));
			si->i=j;
			si->v=vf->p[j];
			wtk_sfvec_add_value(&(cls->vec),si);
		}
	}
	wtk_sfvec_norm(&(cls->vec));
	wtk_vecf_delete(vf);
}

void wtk_clsvec3_class_init(wtk_clsvec3_class_t *cls)
{
	wtk_queue_init(&(cls->item_q));
	wtk_sfvec_init(&(cls->vec));
}

void wtk_clsvec3_init_wrd(wtk_clsvec3_t *v,char *wrd)
{
	wtk_clsvec3_class_t *cls;
	wtk_clsvec2_item_t *item;
	//wtk_heap_t *heap=v->heap;

	cls=&(v->cls);
	wtk_clsvec3_class_init(cls);
	item=wtk_clsvec_dat_get(v->dat,wrd,strlen(wrd));
	if(!item)
	{
		wtk_debug("wrd[%s] not found.\n",wrd);
		exit(0);
	}
	wtk_sfvec_dup(&(item->vec),&(cls->vec),v->heap);
	//void wtk_sfvec_dup(wtk_sfvec_t *src,wtk_sfvec_t *dst,wtk_heap_t *heap);
}

void wtk_clsvec3_init_wrd_list(wtk_clsvec3_t *v,char *wrd_fn)
{
	wtk_clsvec3_class_t *cls;
	wtk_clsvec2_item_t *item;
	//wtk_heap_t *heap=v->heap;
	wtk_source_t src;
	wtk_strbuf_t *buf;
	int ret;

	cls=&(v->cls);
	wtk_clsvec3_class_init(cls);
	buf=wtk_strbuf_new(256,1);
	wtk_source_init_file(&(src),wrd_fn);
	while(1)
	{
		ret=wtk_source_read_line(&(src),buf);
		if(ret!=0 || buf->pos==0){break;}
		wtk_strbuf_strip(buf);
		wtk_debug("[%.*s]\n",buf->pos,buf->data);
		item=wtk_clsvec_dat_get(v->dat,buf->data,buf->pos);
		if(!item)
		{
			wtk_debug("[%.*s] not found.\n",buf->pos,buf->data);
			exit(0);
		}
	}
	exit(0);
	wtk_source_clean_file(&(src));
	wtk_strbuf_delete(buf);
}

void wtk_clsvec3_process(wtk_clsvec3_t *v,char *fn,char *wrd_list)
{
	double t;
	double dist;
	double last_min;
	double max_min=0.1;

	wtk_debug("%s=%s\n",fn,wrd_list);
	v->iter=0;
	v->dat=wtk_clsvec_dat_new(fn);
	wtk_clsvec3_init_wrd_list(v,wrd_list);
	exit(0);
	//wtk_sfvec_print(&(v->cls.vec));
	dist=0;
	last_min=1;
	while(1)
	{
		++v->iter;
		wtk_debug("======== iter=%d dist=%f ==========\n",v->iter,dist);
		t=wtk_clsvec3_add_dist(v,dist);
		wtk_debug("dist1=%f class=%d\n",t,v->cls.item_q.length);
		wtk_clsvec3_print(v);
		if(v->cls.item_q.length==0)
		{
			exit(0);
		}
		dist=v->min2/last_min;
		wtk_debug("alpha=%f\n",dist);
		if(dist>10)
		{
			break;
		}
		wtk_heap_reset(v->heap);
		wtk_clsvec3_update_vec(v);
		t=wtk_clsvec3_add_dist(v,0);
		///wtk_debug("========== class=%d ============\n",v->cls.item_q.length);
		wtk_debug("dist2=%f\n",t);
		dist=v->min2;
		if(dist>max_min)
		{
			max_min=dist;
		}
		if(dist<max_min)
		{
			dist=max_min;
		}
		if(dist>0.1)
		{
			dist=0.1;
		}
		last_min=dist;
	}
	wtk_clsvec3_print(v);
	//exit(0);
}

void wtk_clsvec3_print(wtk_clsvec3_t *v)
{
	wtk_queue_node_t *qn;
	wtk_clsvec2_item_t *item;
	FILE *f;
	char buf[256];

	sprintf(buf,"cls.%d",v->iter);
	//wtk_debug("============ class %d ====================\n",v->cls.item_q.length);
	f=fopen(buf,"w");
	for(qn=v->cls.item_q.pop;qn;qn=qn->next)
	{
		item=data_offset2(qn,wtk_clsvec2_item_t,item_n);
		fprintf(f,"%.*s\n",item->name->len,item->name->data);
	}
	fclose(f);
}
