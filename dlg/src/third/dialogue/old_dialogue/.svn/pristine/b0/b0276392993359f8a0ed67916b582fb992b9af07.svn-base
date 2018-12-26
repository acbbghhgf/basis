#include "wtk_queue.h"
#include "wtk_heap.h"

wtk_queue_t* wtk_queue_new()
{
	wtk_queue_t *q;

	q=(wtk_queue_t*)wtk_malloc(sizeof(*q));
	wtk_queue_init(q);
	return q;
}

wtk_queue_t* wtk_queue_new_h(wtk_heap_t *h)
{
	wtk_queue_t *q;

	q=(wtk_queue_t*)wtk_heap_malloc(h,sizeof(*q));
	wtk_queue_init(q);
	return q;
}

int wtk_queue_delete(wtk_queue_t *q)
{
	free(q);
	return 0;
}

int wtk_queue_init2(wtk_queue_t *q)
{
	q->pop=q->push=0;
	q->listener=0;q->data=0;
	q->length=0;
	return 0;
}

void wtk_queue_set_push_listener(wtk_queue_t *q,queue_push_listener l,void *data)
{
	q->listener=l;
	q->data=data;
}

int wtk_queue_push(wtk_queue_t *q,wtk_queue_node_t *n)
{
	n->prev=q->push;
	if(q->push)
	{
		q->push->next=n;
	}
	n->next=0;
	q->push=n;
	if(!q->pop)
	{
		q->pop=n;
	}
	if(q->listener)
	{
		q->listener(q->data);
	}
	++q->length;
	return 0;
}

void wtk_queue_link(wtk_queue_t *dst,wtk_queue_t *src)
{
	wtk_queue_node_t *n=src->pop;

	if(src->length<=0){return;}
	if(dst->length==0)
	{
		dst->pop=dst->push=0;
	}
	n->prev=dst->push;
	if(dst->push)
	{
		dst->push->next=n;
	}
	dst->push=src->push;
	if(!dst->pop)
	{
		dst->pop=n;
	}
	if(dst->listener)
	{
		dst->listener(dst->data);
	}
	dst->length+=src->length;
}

int wtk_queue_push_front(wtk_queue_t *q,wtk_queue_node_t *n)
{
	n->next=q->pop;
	if(q->pop)
	{
		q->pop->prev=n;
	}
	n->prev=0;
	q->pop=n;
	if(!q->push)
	{
		q->push=n;
	}
	if(q->listener)
	{
		q->listener(q->data);
	}
	++q->length;
	return 0;
}

void wtk_queue_insert_to(wtk_queue_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2)
{
	if(n==q->push)
	{
		wtk_queue_push(q,n2);
	}else
	{
		n2->prev=n;
		n2->next=n->next;
		n2->next->prev=n2->prev->next=n2;
		++q->length;
	}
	return;
}

void wtk_queue_insert_before(wtk_queue_t *q,wtk_queue_node_t *n,wtk_queue_node_t *n2)
{
	if(n==q->pop)
	{
		wtk_queue_push_front(q,n2);
	}else
	{
		n2->next=n;
		n2->prev=n->prev;
		n->prev->next=n2;
		n->prev=n2;
		++q->length;
	}
}

wtk_queue_node_t* wtk_queue_pop_back(wtk_queue_t *q)
{
	wtk_queue_node_t* n;

	if(q->length<=0 || !q->push){return 0;}
	n=q->push;
	--q->length;
	q->push=q->push->prev;
	if(q->push)
	{
		q->push->next=NULL;
	}else
	{
		q->pop=NULL;
	}
	return n;
}

wtk_queue_node_t* wtk_queue_pop(wtk_queue_t *q)
{
	wtk_queue_node_t* n;

	if(q->length<=0){return 0;}
	n=0;
	if(!q->pop){goto end;}
	n=q->pop;
	q->pop=q->pop->next;
	if(q->pop)
	{
		q->pop->prev=0;
	}else
	{
		q->push=0;
	}
	--q->length;
end:
	return n;
}

int wtk_queue_remove(wtk_queue_t *q,wtk_queue_node_t *n)
{
	if(q->length<=0){return 0;}
	if(n->prev)
	{
		n->prev->next=n->next;
	}else
	{
		q->pop=n->next;
	}
	if(n->next)
	{
		n->next->prev=n->prev;
	}else
	{
		q->push=n->prev;
	}
	n->prev=n->next=0;
    --q->length;
	return 0;
}

void wtk_queue_swap(wtk_queue_t *q,wtk_queue_node_t *n1,wtk_queue_node_t *n2)
{
	wtk_queue_node_t tmp;

	if(q->pop==n1)
	{
		q->pop=n2;
	}
	if(q->push==n1)
	{
		q->push=n2;
	}
	if(q->pop==n2)
	{
		q->pop=n1;
	}
	if(q->push==n2)
	{
		q->push=n1;
	}
	if(n1->prev)
	{
		n1->prev->next=n2;
	}
	if(n1->next)
	{
		n1->next->prev=n2;
	}
	if(n2->prev)
	{
		n2->prev->next=n1;
	}
	if(n2->next)
	{
		n2->next->prev=n1;
	}
	tmp=*n1;
	*n1=*n2;
	*n2=tmp;
}

void wtk_queue_touch_node(wtk_queue_t *q,wtk_queue_node_t *n)
{
	wtk_queue_remove(q,n);
	wtk_queue_push(q,n);
}

void wtk_queue_touch_front(wtk_queue_t *q,wtk_queue_node_t *n)
{
	wtk_queue_remove(q,n);
	wtk_queue_push_front(q,n);
}

void* wtk_queue_find(wtk_queue_t *q,int of,wtk_cmp_handler_t cmp,void *user_data)
{
	wtk_queue_node_t *n,*p;
	void *data;
	void *v;

	v=0;
	for(n=q->pop;n;n=p)
	{
		p=n->next;
		data=(void*)wtk_queue_node_data_byof(n,of);
		if(cmp(user_data,data)==0)
		{
			v=data;
			break;
		}
	}
	return v;
}

int wtk_queue_walk(wtk_queue_t *q,int of,wtk_walk_handler_t walk,void *user_data)
{
	wtk_queue_node_t *n,*p;
	void *data;
	int ret=0;

	for(n=q->pop;n;n=p)
	{
		p=n->next;
		data=(void*)wtk_queue_node_data_byof(n,of);
		ret=walk(user_data,data);
		if(ret!=0){goto end;}
	}
end:
	return ret;
}

wtk_queue_node_t* wtk_queue_peek(wtk_queue_t *q,int index)
{
	wtk_queue_node_t *n=0;
	int i;

	if(index>=q->length){goto end;}
	n=q->pop;i=0;
	while(i<index)
	{
		n=n->next;
		++i;
	}
end:
	return n;
}


/**
 *	node 0,1,2,3
 */
void wtk_queue_insert(wtk_queue_t *q,wtk_queue_node_t *n,wtk_cmp_handler_t cmp)
{
	wtk_queue_node_t *xn,*prev=0;
	int ret;

	for(xn=q->pop;xn;xn=xn->next)
	{
		ret=cmp(n,xn);
		if(ret>0)
		{
			prev=xn->prev;
			break;
		}
		if(!xn->next)
		{
			prev=xn;
		}
	}
	if(prev)
	{
		wtk_queue_insert_to(q,prev,n);
	}else
	{
		wtk_queue_push_front(q,n);
	}
}

void wtk_queue_print(wtk_queue_t *q,int of,wtk_print_handler_t print)
{
	wtk_queue_node_t *n,*p;
	void *data;

	for(n=q->pop;n;n=p)
	{
		p=n->next;
		data=(void*)wtk_queue_node_data_byof(n,of);
		print(data);
	}
}

int wtk_queue_node_len(wtk_queue_node_t *qn)
{
	int cnt=0;

	while(qn)
	{
		++cnt;
		qn=qn->next;
	}
	return cnt;
}

wtk_queue_node_t* wtk_queue_find_node(wtk_queue_t *q,wtk_queue_node_t *xn)
{
	wtk_queue_node_t *qn;

	for(qn=q->pop;qn;qn=qn->next)
	{
		if(qn==xn)
		{
			return qn;
		}
	}
	return NULL;
}


int wtk_queue_check(wtk_queue_t *q)
{
    wtk_queue_node_t *n;
    int count;

    count=0;
    for(n=q->pop;n;++count,n=n->next);
    if(count!=q->length)
    {
    	wtk_debug("real count is %d but length is %d\n",count,q->length);
    	exit(0);
    }
    return count-q->length;
}


void wtk_queue_sort_insert(wtk_queue_t *q,wtk_queue_node_t *qn1,wtk_queue_node_cmp_f cmp)
{
	wtk_queue_node_t *qn,*nxt;
	int ret;

	nxt=NULL;
	for(qn=q->pop;qn;qn=qn->next)
	{
		ret=cmp(qn1,qn);
		if(ret>=0)
		{
			nxt=qn;
			break;
		}
	}
	if(nxt)
	{
		wtk_queue_insert_before(q,nxt,qn1);
	}else
	{
		wtk_queue_push(q,qn1);
	}
}

void wtk_queue_sort(wtk_queue_t *src,wtk_queue_node_cmp_f cmp)
{
	wtk_queue_t dst;
	wtk_queue_node_t *qn;

	wtk_queue_init(&(dst));
	while(1)
	{
		qn=wtk_queue_pop(src);
		if(!qn){break;}
		wtk_queue_sort_insert(&(dst),qn,cmp);
	}
	*src=dst;
}

//------------------------ test/example section ------------
typedef struct
{
	wtk_queue_node_t q_n;
	int i;
}wtk_foo_n_t;

void wtk_queue_test_g()
{
	wtk_foo_n_t v[10],*vp;
	wtk_queue_t q;
	wtk_queue_node_t *qn;
	int i,n;

	wtk_queue_init(&(q));
	n=sizeof(v)/sizeof(wtk_foo_n_t);
	for(i=0;i<n;++i)
	{
		v[i].i=i;
	}
	for(i=0;i<n;++i)
	{
		wtk_queue_push(&(q),&(v[i].q_n));
	}
	while(1)
	{
		qn=wtk_queue_pop(&(q));
		if(!qn){break;}
		vp=data_offset(qn,wtk_foo_n_t,q_n);
		wtk_debug("vp=%d\n",vp->i);
	}
}
