#include "wtk_thread_pool.h"

wtk_thread_pool_t *wtk_thread_pool_new(int threads,wtk_blockqueue_t *queue,	thread_pool_handler handler,void *user_data)
{
	return wtk_thread_pool_new2(threads,queue,NULL,NULL,handler,user_data);
}

wtk_thread_pool_t *wtk_thread_pool_new2(int threads,wtk_blockqueue_t *queue,wtk_thread_pool_init_f init,
		wtk_thread_pool_clean_f clean,thread_pool_handler handler,void *user_data)
{
	wtk_thread_pool_t *t;

	t=(wtk_thread_pool_t*)wtk_malloc(sizeof(*t));
	t->thread_num=threads;
	t->threads=(wtk_thread_t*)wtk_calloc(threads,sizeof(wtk_thread_t));
	t->queue=queue;
	t->handler=handler;
	t->init=init;
	t->clean=clean;
	t->user_data=user_data;
	t->timeout=-1;
	t->timeout_f=0;
	return t;
}

int wtk_thread_pool_bytes(wtk_thread_pool_t *t)
{
	return sizeof(*t)+sizeof(wtk_thread_t)*t->thread_num;
}

int wtk_thread_pool_delete(wtk_thread_pool_t *p)
{
	int i;

	for(i=0;i<p->thread_num;++i)
	{
		//wtk_thread_join(&p->threads[i]);
		wtk_thread_clean((&p->threads[i]));
	}
	wtk_free(p->threads);
	wtk_free(p);
	return 0;
}

void wtk_thread_pool_set_timeout(wtk_thread_pool_t *p,wtk_thread_pool_timeout_f timeout_f,int timeout)
{
	p->timeout_f=timeout_f;
	p->timeout=timeout;
}

int wtk_thread_pool_run(wtk_thread_pool_t *p,wtk_thread_t *t)
{
	wtk_blockqueue_t* q;
	wtk_queue_node_t *n;
	thread_pool_handler handler;
	void *user_data;
	int ret;
	int is_timeout;

	q=(p->queue);
	handler=p->handler;
	user_data=p->user_data;
    //wtk_debug("thread[%d] is on.\n",t->ppid);
	if(p->init)
	{
		p->init(p->user_data,t);
	}
	while(p->run)
	{
		n=wtk_blockqueue_pop(q,p->timeout,&is_timeout);
		//wtk_debug("pid: %d,tid=%d\n",getpid(),t->ppid);
		if(!n)
		{
			if(is_timeout)
			{
				if(p->timeout_f)
				{
					p->timeout_f(p->user_data,t);
				}
				continue;
			}
			break;
		}
		ret=handler(user_data,n,t);
		if(ret!=0){break;}
	}
#ifndef WIN32
	//wtk_debug("thread %d run=%d goto exit.\n",t->ppid,p->run);
#endif
	if(p->clean)
	{
		p->clean(p->user_data,t);
	}
	return 0;
}

int wtk_thread_pool_start(wtk_thread_pool_t *p)
{
	wtk_thread_t *t;
	int i,ret;

	p->run=1;ret=0;
	for(i=0;i<p->thread_num;++i)
	{
		t=&(p->threads[i]);
		ret=wtk_thread_init(t,(thread_route_handler)wtk_thread_pool_run,p);
		if(ret!=0)
		{
			perror(__FUNCTION__);
			wtk_debug("create thread pool failed.\n");
			break;
		}
		ret=wtk_thread_start(t);
		if(ret!=0)
		{
			perror(__FUNCTION__);
			wtk_debug("create thread pool failed.\n");
			break;
		}
	}
	return ret;
}

int wtk_thread_pool_join(wtk_thread_pool_t *p)
{
	wtk_thread_t *t;
	int i,ret;

	ret=0;
	for(i=0;i<p->thread_num;++i)
	{
		t=&(p->threads[i]);
		ret=wtk_thread_join(t);
	}
	return ret;
}

int wtk_thread_pool_stop(wtk_thread_pool_t *p)
{
	int i;

	p->run=0;
	for(i=0;i<p->thread_num;++i)
	{
		wtk_blockqueue_wake((p->queue));
	}
    for(i=0;i<p->thread_num;++i)
    {
        wtk_thread_join(&(p->threads[i]));
    }
	return 0;
}

int wtk_thread_pool_kill(wtk_thread_pool_t *p)
{
	int i;

	p->run=0;
    for(i=0;i<p->thread_num;++i)
    {
    	wtk_thread_kill(&(p->threads[i]));
    }
	return 0;
}

