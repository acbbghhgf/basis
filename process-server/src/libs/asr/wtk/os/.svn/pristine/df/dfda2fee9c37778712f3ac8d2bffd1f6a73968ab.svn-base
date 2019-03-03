#include "wtk_dtp.h"

int wtk_dtp_route_process(wtk_dtp_route_t *r,wtk_thread_t *t)
{
	wtk_dtp_t *dtp=r->dtp;
	wtk_dtp_cfg_t *cfg=dtp->cfg;
	wtk_blockqueue_t *q=&(dtp->input_q);
	wtk_queue_node_t *n;

	while(dtp->run)
	{
		//wtk_debug("wait pop\n");
		n=wtk_blockqueue_pop(q,cfg->timeout,0);
		//wtk_debug("pop node=%p\n",n);
		if(!n){continue;}
		dtp->handler->inst_process(dtp->handler->ths,r->inst,n);
	}
	return 0;
}

int wtk_dtp_add_route(wtk_dtp_t *dtp)
{
	wtk_dtp_route_t *r;
	wtk_dtp_handler_t *handler=dtp->handler;
	int ret=-1;

	r=(wtk_dtp_route_t*)wtk_heap_malloc(dtp->heap,sizeof(*r));
	r->dtp=dtp;
	r->inst=handler->inst_new(handler->ths);
	if(!r->inst){goto end;}
	ret=wtk_thread_init(&(r->thread),(thread_route_handler)wtk_dtp_route_process,r);
	if(ret!=0){goto end;}
	ret=wtk_thread_start(&(r->thread));
	if(ret!=0){goto end;}
	//wtk_debug("add route ppid=%d\n",r->thread.ppid);
	wtk_queue_push(&(dtp->thread_q),&(r->q_n));
end:
	return ret;
}

wtk_dtp_t* wtk_dtp_new(wtk_dtp_cfg_t *cfg,wtk_dtp_handler_t *h)
{
	wtk_dtp_t *dtp;
	int i;
	int ret=0;

	dtp=(wtk_dtp_t*)wtk_malloc(sizeof(*dtp));
	dtp->cfg=cfg;
	dtp->handler=h;
	dtp->heap=wtk_heap_new(4096);
	wtk_queue_init(&(dtp->thread_q));
	wtk_blockqueue_init(&(dtp->input_q));
	dtp->run=1;
	for(i=0;i<cfg->min;++i)
	{
		ret=wtk_dtp_add_route(dtp);
		if(ret!=0){goto end;}
	}
end:
	if(ret!=0)
	{
		wtk_dtp_delete(dtp);
		dtp=0;
	}
	return dtp;
}

void wtk_dtp_delete(wtk_dtp_t *dtp)
{
	wtk_dtp_stop(dtp);
	wtk_blockqueue_clean(&(dtp->input_q));
	wtk_heap_delete(dtp->heap);
	wtk_free(dtp);
}

void wtk_dtp_push(wtk_dtp_t *dtp,wtk_queue_node_t *n)
{
	wtk_blockqueue_t *q=&(dtp->input_q);

	if(q->length>dtp->thread_q.length)
	{
		if(dtp->cfg->max<0 || dtp->thread_q.length<dtp->cfg->max)
		{
			//wtk_debug("add route: input=%d,thread=%d\n",q->length,dtp->thread_q.length);
			wtk_dtp_add_route(dtp);
		}
	}
	wtk_blockqueue_push(q,n);
}

void wtk_dtp_join(wtk_dtp_t *dtp)
{
	wtk_queue_node_t *n;
	wtk_dtp_route_t *r;

	for(n=dtp->thread_q.pop;n;n=n->next)
	{
		r=data_offset(n,wtk_dtp_route_t,q_n);
		wtk_thread_join(&(r->thread));
		wtk_thread_clean(&(r->thread));
		dtp->handler->inst_delete(r->inst);
	}
	wtk_queue_init(&(dtp->thread_q));
	wtk_heap_reset(dtp->heap);
}

void wtk_dtp_stop(wtk_dtp_t *dtp)
{
	dtp->run=0;
	wtk_dtp_join(dtp);
}


