#include "wtk_slist.h"

void wtk_slist_init(wtk_slist_t *list)
{
	list->prev=0;
}

void wtk_slist_push(wtk_slist_t *list,wtk_slist_node_t *node)
{
	node->prev=list->prev;
	list->prev=node;
}

void wtk_slist_push_front(wtk_slist_t *list,wtk_slist_node_t *node)
{
	while(list->prev)
	{
		list=list->prev;
	}
	node->prev=NULL;
	list->prev=node;
}

int wtk_slist_remove(wtk_slist_t *list,wtk_slist_node_t *n)
{
	wtk_slist_node_t *tmp,*prev;

	for(prev=NULL,tmp=list->prev;tmp;tmp=tmp->prev)
	{
		if(tmp==n)
		{
			if(prev)
			{
				prev->prev=tmp->prev;
			}else
			{
				list->prev=tmp->prev;
			}
			return 1;
		}
		prev=tmp;
	}
	return 0;
}

wtk_slist_node_t* wtk_slist_pop(wtk_slist_t *list)
{
	wtk_slist_node_t *node;

	if(list->prev)
	{
		node=list->prev;
		list->prev=node->prev;
	}else
	{
		node=0;
	}
	return node;
}

int wtk_slist_len(wtk_slist_t *l)
{
	wtk_slist_node_t *n;
	int len=0;

	for(n=l->prev;n;n=n->prev)
	{
		++len;
	}
	return len;
}


void wtk_slist2_init(wtk_slist2_t *l)
{
	l->pop=l->push=NULL;
}

void wtk_slist2_push(wtk_slist2_t *l,wtk_slist_node_t *n)
{
	n->prev=NULL;
	if(l->push)
	{
		l->push->prev=n;
		l->push=n;
	}else
	{
		l->pop=l->push=n;
	}
}

wtk_slist_node_t* wtk_slist2_pop(wtk_slist2_t *l)
{
	wtk_slist_node_t *n;

	if(l->pop)
	{
		n=l->pop;
		if(n==l->push)
		{
			l->pop=l->push=NULL;
		}else
		{
			l->pop=n->prev;
		}
		return n;
	}else
	{
		return NULL;
	}
}
