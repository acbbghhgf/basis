#include "wtk_queue2.h"

/*
void wtk_queue2_init(wtk_queue2_t *q)
{
	q->pop=q->push=0;
}*/

int wtk_queue2_len(wtk_queue2_t *q)
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

void wtk_queue2_push(wtk_queue2_t *q,wtk_queue_node_t *n)
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
}

void wtk_queue2_touch_node(wtk_queue2_t *q,wtk_queue_node_t *n)
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
	n->prev=q->push;
	if(q->push)
	{
		q->push->next=n;
	}
	q->push=n;
	n->next=NULL;
	if(!q->pop)
	{
		q->pop=n;
	}
}

void wtk_queue2_push_front(wtk_queue2_t *q,wtk_queue_node_t *n)
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
}

void wtk_queue2_insert_to(wtk_queue2_t *q,wtk_queue_node_t *n,wtk_queue_node_t* n2)
{
	if(n==q->push)
	{
		wtk_queue2_push(q,n2);
	}else
	{
		n2->prev=n;
		n2->next=n->next;
		n2->next->prev=n2->prev->next=n2;
	}
	return;
}

/**
 *	n0->n2->n3
 *				=>	n0->n1->n2->n3
 *	   n1
 */
void wtk_queue2_insert_before(wtk_queue2_t *q,wtk_queue_node_t *n2,wtk_queue_node_t *n1)
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
}

wtk_queue_node_t* wtk_queue2_pop(wtk_queue2_t *q)
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
end:
	return n;
}


void wtk_queue2_remove(wtk_queue2_t *q,wtk_queue_node_t *n)
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
}
