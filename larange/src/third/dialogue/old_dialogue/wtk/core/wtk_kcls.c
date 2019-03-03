#include "wtk_kcls.h" 

wtk_kcls_value_t* wtk_kcls_value_new(wtk_heap_t *heap,int idx,float f)
{
	wtk_kcls_value_t *v;

	v=(wtk_kcls_value_t*)wtk_heap_malloc(heap,sizeof(wtk_kcls_value_t));
	v->idx=idx;
	v->v=f;
	return v;
}

wtk_kcls_t* wtk_kcls_new(wtk_heap_t *heap)
{
	wtk_kcls_t *cls;

	cls=(wtk_kcls_t*)wtk_heap_malloc(heap,sizeof(wtk_kcls_t));
	wtk_queue_init(&(cls->item_q));
	cls->sse=0;
	cls->mean=0;
	return cls;
}

wtk_kcls_t* wtk_kcls_pop(wtk_kcls_t *cls,wtk_heap_t *heap)
{
	wtk_kcls_t *vi;
	wtk_queue_node_t *qn;
	wtk_kcls_value_t *v;

	vi=wtk_kcls_new(heap);
	qn=wtk_queue_pop(&(cls->item_q));
	v=data_offset2(qn,wtk_kcls_value_t,q_n);
	vi->mean=v->v;
	wtk_queue_push(&(vi->item_q),qn);
	return vi;
}

void wtk_kcls_print(wtk_kcls_t *cls)
{
	wtk_queue_node_t *qn;
	wtk_kcls_value_t *vx;

	wtk_debug("============ sse=%f mean=%f ==========\n",cls->sse,cls->mean);
	for(qn=cls->item_q.pop;qn;qn=qn->next)
	{
		vx=(wtk_kcls_value_t*)data_offset2(qn,wtk_kcls_value_t,q_n);
		wtk_debug("v[%d]=%f\n",vx->idx,vx->v);
	}
}

void wtk_kcls_update_sse(wtk_kcls_t *item)
{
	wtk_queue_node_t *qn;
	wtk_kcls_value_t *vx;
	float f,f2;

	f=0;
	for(qn=item->item_q.pop;qn;qn=qn->next)
	{
		vx=(wtk_kcls_value_t*)data_offset2(qn,wtk_kcls_value_t,q_n);
		f+=vx->v;
		//wtk_debug("v[%d]=%f\n",vx->idx,vx->v);
	}
	item->mean=f/item->item_q.length;
	f=0;
	for(qn=item->item_q.pop;qn;qn=qn->next)
	{
		vx=(wtk_kcls_value_t*)data_offset2(qn,wtk_kcls_value_t,q_n);
		f2=vx->v-item->mean;
		f+=f2*f2;
	}
	item->sse=f;
	//wtk_debug("============ sse=%f ===========\n",item->sse);
}

void wtk_kcls_insert(wtk_queue_t *q,wtk_kcls_t *item)
{
	wtk_queue_node_t *qn;
	wtk_kcls_t *vi;

	for(qn=q->pop;qn;qn=qn->next)
	{
		vi=data_offset2(qn,wtk_kcls_t,q_n);
		//wtk_debug("sse=%f/%f\n",item->sse,vi->sse);
		if(vi->sse<=item->sse)
		{
			//wtk_debug("============= insert %f len=%d ===========\n",item->sse,item->item_q.length);
			wtk_queue_insert_before(q,qn,&(item->q_n));
			return;
		}
	}
	wtk_queue_push(q,&(item->q_n));
}

void wtk_kcls_cluster_process(wtk_queue_t *qt,wtk_heap_t *heap,float min_sse)
{
	wtk_kcls_t *item1,*item2,*item;
	wtk_queue_node_t *qn;
	wtk_kcls_value_t *vx;
	float f1,f2;
	float m1,m2;
	wtk_queue_t q;

	qn=wtk_queue_pop(qt);
	item=data_offset2(qn,wtk_kcls_t,q_n);
	//wtk_debug("len=%d sse=%f\n",item->item_q.length,item->sse);
	if(item->item_q.length<2 || item->sse/item->item_q.length<min_sse)
	{
		wtk_queue_push_front(qt,qn);
		return;
	}
	item1=wtk_kcls_pop(item,heap);
	item2=wtk_kcls_pop(item,heap);
	m1=item1->mean;
	m2=item2->mean;
	while(1)
	{
		qn=wtk_queue_pop(&(item->item_q));
		if(!qn){break;}
		vx=data_offset2(qn,wtk_kcls_value_t,q_n);
		f1=(vx->v-m1);//item1->mean);
		f1*=f1;
		f2=(vx->v-m2);//item2->mean);
		f2*=f2;
		if(f1<f2)
		{
			item1->mean=(item1->mean*item1->item_q.length+vx->v)/(item1->item_q.length+1);
			wtk_queue_push(&(item1->item_q),qn);
		}else
		{
			item2->mean=(item2->mean*item2->item_q.length+vx->v)/(item2->item_q.length+1);
			wtk_queue_push(&(item2->item_q),qn);
		}
	}
	q=item1->item_q;
	wtk_queue_link(&(q),&(item2->item_q));
	wtk_queue_init(&(item1->item_q));
	wtk_queue_init(&(item2->item_q));
	m1=item1->mean;
	m2=item2->mean;
	while(1)
	{
		qn=wtk_queue_pop(&(q));
		if(!qn){break;}
		vx=data_offset2(qn,wtk_kcls_value_t,q_n);
		f1=(vx->v-m1);//item1->mean);
		f1*=f1;
		f2=(vx->v-m2);//item2->mean);
		f2*=f2;
		if(f1<f2)
		{
			wtk_queue_push(&(item1->item_q),qn);
		}else
		{
			wtk_queue_push(&(item2->item_q),qn);
		}
	}
	wtk_kcls_update_sse(item1);
	wtk_kcls_update_sse(item2);
	wtk_kcls_insert(qt,item1);
	wtk_kcls_insert(qt,item2);
}

wtk_kcls_t* wtk_kcls_cluster(wtk_kcls_t *cls,wtk_heap_t *heap,float max_sse)
{
	int nx;
	wtk_queue_node_t *qn;
	wtk_queue_t q;

	wtk_queue_init(&(q));
	wtk_queue_push(&(q),&(cls->q_n));
	cls->sse=max_sse*cls->item_q.length+1;
	do
	{
		nx=q.length;
		wtk_kcls_cluster_process(&(q),heap,max_sse);
		//wtk_debug("============nx=%d =======\n",nx);
	}while(nx!=q.length);
	qn=wtk_queue_pop(&(q));
	if(qn)
	{
		cls=data_offset2(qn,wtk_kcls_t,q_n);
		return cls;
	}else
	{
		return NULL;
	}
}

void wtk_kcls_select_int(int *v,int n)
{
	wtk_heap_t *heap;
	wtk_kcls_t *cls;
	wtk_kcls_value_t *vx;
	int i;
	float max_sse=2;

	heap=wtk_heap_new(4096);
	cls=wtk_kcls_new(heap);
	for(i=0;i<n;++i)
	{
		vx=wtk_kcls_value_new(heap,i,v[i]);
		wtk_queue_push(&(cls->item_q),&(vx->q_n));
	}
	cls=wtk_kcls_cluster(cls,heap,max_sse);
	wtk_kcls_print(cls);
	wtk_heap_delete(heap);
}
