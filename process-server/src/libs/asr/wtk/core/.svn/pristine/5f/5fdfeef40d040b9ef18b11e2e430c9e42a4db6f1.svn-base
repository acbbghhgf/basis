#include "wtk_sfvec.h" 


void wtk_sfvec_init(wtk_sfvec_t *v)
{
	wtk_queue3_init(&(v->item_q));
}

void wtk_sfvec_add_value(wtk_sfvec_t *v,wtk_svec_item_t *item)
{
	wtk_queue_node_t *qn;
	wtk_svec_item_t *si;
	wtk_queue_node_t *prev=NULL;

	//wtk_debug("%d:%.0f\n",item->i,item->v);
	for(qn=v->item_q.push;qn;qn=qn->prev)
	{
		si=data_offset2(qn,wtk_svec_item_t,q_n);
		if(si->i<item->i)
		{
			prev=qn;
			break;
		}
	}
	//wtk_debug("prev=%p\n",prev);
	if(prev)
	{
		wtk_queue3_insert_to(&(v->item_q),prev,&(item->q_n));
	}else
	{
		if(qn)
		{
			wtk_queue3_push(&(v->item_q),&(item->q_n));
		}else
		{
			wtk_queue3_push_front(&(v->item_q),&(item->q_n));
		}
	}
	//wtk_sfvec_print(v);
}

void wtk_sfvec_norm(wtk_sfvec_t *v)
{
	wtk_queue_node_t *qn;
	wtk_svec_item_t *si;
	float f;

	f=0;
	for(qn=v->item_q.pop;qn;qn=qn->next)
	{
		si=data_offset2(qn,wtk_svec_item_t,q_n);
		f+=si->v*si->v;
	}
	if(f==0){return;}
	f=1.0/sqrt(f);
	for(qn=v->item_q.pop;qn;qn=qn->next)
	{
		si=data_offset2(qn,wtk_svec_item_t,q_n);
		si->v*=f;
	}
}

double wtk_sfvec_dist(wtk_sfvec_t *sf1,wtk_sfvec_t *sf)
{
	wtk_queue_node_t *qn,*qn2;
	wtk_svec_item_t *si,*si2;
	double f,t;
	int b;

	//wtk_sfvec_print(sf1);
	//wtk_sfvec_print(sf);
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
	for(qn2=sf->item_q.pop;qn2;qn2=qn2->next)
	{
		si2=data_offset2(qn2,wtk_svec_item_t,q_n);
		t=0;
		b=0;
		for(qn=sf1->item_q.pop;qn;qn=qn->next)
		{
			si=data_offset2(qn,wtk_svec_item_t,q_n);
			//wtk_debug("v[%d]=%d\n",i,si->i);
			if(si2->i==si->i)
			{
				b=1;
				break;
			}
		}
		if(b)
		{
			continue;
		}
		t=si2->v;
		//wtk_debug("v[%d]=%f\n",si2->i,si2->v);
		f+=t*t;
	}
	//wtk_debug("f=%f\n",f);
	//exit(0);
	return f;
}

void wtk_sfvec_dup(wtk_sfvec_t *src,wtk_sfvec_t *dst,wtk_heap_t *heap)
{
	wtk_queue_node_t *qn;
	wtk_svec_item_t *si,*ti;

	wtk_sfvec_init(dst);
	for(qn=src->item_q.pop;qn;qn=qn->next)
	{
		si=data_offset2(qn,wtk_svec_item_t,q_n);
		//printf("%d:%.3f\n",si->i,si->v);
		ti=(wtk_svec_item_t*)wtk_heap_malloc(heap,sizeof(wtk_svec_item_t));
		ti->i=si->i;
		ti->v=si->v;
		wtk_sfvec_add_value(dst,ti);
	}
}

void wtk_sfvec_print(wtk_sfvec_t *v)
{
	wtk_queue_node_t *qn;
	wtk_svec_item_t *si;

	wtk_debug("============ class %d ====================\n",v->item_q.len);
	for(qn=v->item_q.pop;qn;qn=qn->next)
	{
		si=data_offset2(qn,wtk_svec_item_t,q_n);
		printf("%d:%.3f\n",si->i,si->v);
	}
}
