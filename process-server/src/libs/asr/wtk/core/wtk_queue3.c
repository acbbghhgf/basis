#include "wtk_queue3.h"

/*
void wtk_queue3_init(wtk_queue3_t *q)
{
	q->pop=q->push=0;
}*/


int wtk_queue3_len(wtk_queue3_t *q)
{
	wtk_queue_node_t *n=q->pop;
	int len;

	len=0;
	while(n)
	{
		++len;
		n=n->next;
	}
	return len;
}

void wtk_queue3_push(wtk_queue3_t *q,wtk_queue_node_t *n)
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
	++q->len;
}

void wtk_queue3_push_front(wtk_queue3_t *q,wtk_queue_node_t *n)
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
	++q->len;
}

void wtk_queue3_insert_to(wtk_queue3_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2)
{
	if(n==q->push)
	{
		wtk_queue3_push(q,n2);
	}else
	{
		n2->prev=n;
		n2->next=n->next;
		n2->next->prev=n2->prev->next=n2;
		++q->len;
	}
	return;
}

/**
 *	n0->n2->n3
 *				=>	n0->n1->n2->n3
 *	   n1
 */
void wtk_queue3_insert_before(wtk_queue3_t *q,wtk_queue_node_t *n2,wtk_queue_node_t *n1)
{
	n1->prev=n2->prev;
	if(n1->prev)
	{
		n1->prev->next=n1;
	}else
	{
		q->pop=n1;
	}
	n2->prev=n1;
	n1->next=n2;
	++q->len;
}

wtk_queue_node_t* wtk_queue3_pop(wtk_queue3_t *q)
{
	wtk_queue_node_t* n;

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
	--q->len;
end:
	return n;
}

wtk_queue_node_t* wtk_queue3_pop_back(wtk_queue3_t *q)
{
	wtk_queue_node_t* n;

	if(q->len<=0 || !q->push){return 0;}
	n=q->push;
	--q->len;
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

void wtk_queue3_remove(wtk_queue3_t *q,wtk_queue_node_t *n)
{
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
	--q->len;
}

void wtk_queue3_check(wtk_queue3_t *q)
{
	wtk_queue_node_t *qn;
	int len=0;

	for(qn=q->pop;qn;qn=qn->next)
	{
		++len;
	}
	if(len!=q->len)
	{
		wtk_debug("found bug=%d/%d\n",len,q->len);
		exit(0);
	}
}

void wtk_queue_link2(wtk_queue_t *dst,wtk_queue3_t *src)
{
	wtk_queue_node_t *n=src->pop;

	if(src->len<=0){return;}
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
	dst->length+=src->len;
}

wtk_queue_node_t* wtk_queue3_peek(wtk_queue3_t *q,int index)
{
	wtk_queue_node_t *n=0;
	int i;

	if(index>=q->len){goto end;}
	n=q->pop;i=0;
	while(i<index)
	{
		n=n->next;
		++i;
	}
end:
	return n;
}
