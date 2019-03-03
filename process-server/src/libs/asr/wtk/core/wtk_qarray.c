#include "wtk_qarray.h"

wtk_qarray_t* wtk_qarray_new(wtk_heap_t *heap,int use_sort)
{
	wtk_qarray_t *a;

	a=(wtk_qarray_t*)wtk_heap_malloc(heap,sizeof(wtk_qarray_t));
	a->heap=heap;
	a->use_sort=use_sort;
	wtk_queue_init(&(a->q));
	return a;
}

wtk_qarray_item_t* wtk_qarray_add_item(wtk_qarray_t *a,int idx)
{
	wtk_qarray_item_t *item,*t;
	wtk_queue_node_t *qn,*nxt;

	item=(wtk_qarray_item_t*)wtk_heap_malloc(a->heap,sizeof(wtk_qarray_item_t));
	item->idx=idx;
	if(a->use_sort)
	{
		for(nxt=NULL,qn=a->q.pop;qn;qn=qn->next)
		{
			t=data_offset(qn,wtk_qarray_item_t,q_n);
			if(idx>=t->idx)
			{
				nxt=qn;
				break;
			}
		}
		if(nxt)
		{
			wtk_queue_insert_before(&(a->q),nxt,&(item->q_n));
		}else
		{
			wtk_queue_push(&(a->q),&(item->q_n));
		}
	}else
	{
		wtk_queue_push(&(a->q),&(item->q_n));
	}
	return item;
}

wtk_qarray_item_t* wtk_qarray_find(wtk_qarray_t *a,int idx)
{
	wtk_queue_node_t *qn;
	wtk_qarray_item_t *item;

	for(qn=a->q.pop;qn;qn=qn->next)
	{
		item=data_offset(qn,wtk_qarray_item_t,q_n);
		if(item->idx==idx)
		{
			return item;
		}
	}
	return NULL;
}

void wtk_qarray_add_p(wtk_qarray_t *a,int idx,void *ths)
{
	wtk_qarray_item_t *item;

	item=wtk_qarray_add_item(a,idx);
	item->v.p=ths;
}

void wtk_qarray_append_p(wtk_qarray_t *a,void *ths)
{
	wtk_qarray_add_p(a,a->q.length,ths);
}

void* wtk_qarray_get_p(wtk_qarray_t *a,int idx)
{
	wtk_qarray_item_t *item;

	item=wtk_qarray_find(a,idx);
	return item->v.p;
}
