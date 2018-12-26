#include "wtk_blockqueue.h"

int wtk_blockqueue_init(wtk_blockqueue_t *q)
{
	int ret;

	ret=wtk_lockqueue_init((wtk_lockqueue_t*)q);
	if(ret!=0){goto end;}
	ret=wtk_sem_init(&(q->sem),0);
end:
	return ret;
}

int wtk_blockqueue_clean(wtk_blockqueue_t *q)
{
	wtk_lockqueue_clean((wtk_lockqueue_t*)q);
	wtk_sem_clean(&(q->sem));
	return 0;
}

int wtk_blockqueue_push(wtk_blockqueue_t *q,wtk_queue_node_t *n)
{
	int ret;

	ret=wtk_lockqueue_push((wtk_lockqueue_t*)q,n);
	if(ret!=0){goto end;}
    //wtk_debug("push time,%d\n",q->length);
	ret=wtk_sem_inc(&(q->sem));
end:
	return ret;
}

int wtk_blockqueue_push_front(wtk_blockqueue_t *q,wtk_queue_node_t *n)
{
	int ret;

	ret=wtk_lockqueue_push_front((wtk_lockqueue_t*)q,n);
	if(ret!=0){goto end;}
    //wtk_debug("push time,%d\n",q->length);
	ret=wtk_sem_inc(&(q->sem));
end:
	return ret;
}

wtk_queue_node_t* wtk_blockqueue_pop(wtk_blockqueue_t *q,int ms,int *is_timeout)
{
	wtk_queue_node_t *n;
	int ret;

	n=0;
    //wtk_debug("pop time,%d\n",ms);
	ret=wtk_sem_acquire(&(q->sem),ms);
    //wtk_debug("pop ret,%d\n",ret);
	if(ret==0)
	{
		n=wtk_lockqueue_pop((wtk_lockqueue_t*)q);
	}
	if(is_timeout)
	{
		*is_timeout= ret==0 ? 0 :1;
	}
	return n;
}

int wtk_blockqueue_wake(wtk_blockqueue_t *q)
{
	return wtk_sem_inc(&(q->sem));
}
