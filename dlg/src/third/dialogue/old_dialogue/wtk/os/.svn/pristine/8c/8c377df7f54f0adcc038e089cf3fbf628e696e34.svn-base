#include "wtk_thread2.h" 
int wtk_thread2_run(wtk_thread2_t *t2,wtk_thread_t *t);

wtk_thread2_t* wtk_thread2_new(void *ths,
		wtk_thread2_start_f start,
		wtk_thread2_stop_f stop,
		wtk_thread2_process_f process,
		wtk_thread2_err_f err)
{
	wtk_thread2_t *t;

	t=(wtk_thread2_t*)wtk_malloc(sizeof(wtk_thread2_t));
	t->ths=ths;
	t->start=start;
	t->stop=stop;
	t->process=process;
	t->err=err;
	t->run=0;
	wtk_sem_init(&(t->notify_sem),0);
	wtk_blockqueue_init(&(t->input_q));
	wtk_queue_init(&(t->time_q));
	wtk_thread_init(&(t->thread),(thread_route_handler)wtk_thread2_run,t);
	return t;
}

void wtk_thread2_delete(wtk_thread2_t *t)
{
	wtk_thread_clean(&(t->thread));
	wtk_sem_clean(&(t->notify_sem));
	wtk_free(t);
}

int wtk_thread2_start(wtk_thread2_t *t)
{
	t->run=1;
	return wtk_thread_start(&(t->thread));
}

int wtk_thread2_stop(wtk_thread2_t *t)
{
	t->run=0;
	wtk_blockqueue_wake(&(t->input_q));
	return wtk_thread_join(&(t->thread));
}

wtk_thread2_msg_t* wtk_thread2_msg_new(wtk_thread2_msg_type_t type,void *data)
{
	wtk_thread2_msg_t *msg;

	msg=(wtk_thread2_msg_t*)wtk_malloc(sizeof(wtk_thread2_msg_t));
	msg->type=type;
	msg->data=data;
	return msg;
}

void wtk_thread2_msg_delete(wtk_thread2_msg_t *msg)
{
	wtk_free(msg);
}

void wtk_thread2_msg_start(wtk_thread2_t *t)
{
	wtk_thread2_msg_t *msg;

	msg=wtk_thread2_msg_new(WTK_THREAD2_START,NULL);
	wtk_blockqueue_push(&(t->input_q),&(msg->q_n));
}

void wtk_thread2_msg_cancel(wtk_thread2_t *t2)
{
	wtk_queue_node_t *qn;
	wtk_thread2_msg_t *msg;

	while(1)
	{
		qn=wtk_blockqueue_pop(&(t2->input_q),0,NULL);
		if(!qn){break;}
		msg=data_offset2(qn,wtk_thread2_msg_t,q_n);
		if(msg->type==WTK_THREAD2_STOP)
		{
			wtk_blockqueue_push(&(t2->input_q),qn);
			break;
		}
		if(t2->err)
		{
			t2->err(t2->ths,msg->data);
		}
		wtk_thread2_msg_delete(msg);
	}
}

void wtk_thread2_msg_process(wtk_thread2_t *t,void *data)
{
	wtk_thread2_msg_t *msg;

	msg=wtk_thread2_msg_new(WTK_THREAD2_PROCESS,data);
	wtk_blockqueue_push(&(t->input_q),&(msg->q_n));
}

int wtk_thread2_msg_stop(wtk_thread2_t *t)
{
	return wtk_thread2_msg_stop2(t,1);
}

int wtk_thread2_msg_stop2(wtk_thread2_t *t,int wait_eof)
{
	wtk_thread2_msg_t *msg;

	msg=wtk_thread2_msg_new(WTK_THREAD2_STOP,NULL);
	wtk_blockqueue_push(&(t->input_q),&(msg->q_n));
	//wtk_debug("wait end1 %p....\n",t);
	if(wait_eof)
	{
		wtk_thread2_msg_wait_stop(t,-1);
	}
	//wtk_debug("=============> wait end....\n");
	return 0;
}

int wtk_thread2_msg_wait_stop(wtk_thread2_t *t,int timeout)
{
	return  wtk_sem_acquire(&(t->notify_sem),timeout);
}

wtk_thread2_timer_t* wtk_thread2_timer_new(double delay,wtk_thread2_timer_func_t func,void *ths,void *hook)
{
	wtk_thread2_timer_t *t;

	t=(wtk_thread2_timer_t*)wtk_malloc(sizeof(wtk_thread2_timer_t));
	t->t=time_get_ms()+delay;
	t->func=func;
	t->ths=ths;
	t->hook=hook;
	return t;
}

void wtk_thread2_timer_delete(wtk_thread2_timer_t *t)
{
	wtk_free(t);
}

void wtk_thread2_touch_timer(wtk_thread2_t *dlg)
{
	wtk_queue_node_t *qn;
	wtk_queue_t *q=&(dlg->time_q);
	wtk_thread2_timer_t *timer;
	double t;

	//wtk_debug("====================> touch timer %d\n",q->length);
	while(1)
	{
		qn=q->pop;
		if(!qn){break;}
		timer=data_offset2(qn,wtk_thread2_timer_t,q_n);
		t=time_get_ms();
		//wtk_debug("===========> touch timer %f\n",t-timer->t);
		//wtk_debug("time=%f,%f\n",t,timer->t);
		if(timer->t<=t)
		{
			wtk_queue_pop(q);
			timer->func(timer->ths,timer->hook);
			wtk_thread2_timer_delete(timer);
		}else
		{
			break;
		}
	}
}

void wtk_thread2_add_timer(wtk_thread2_t *dlg,double delay,wtk_thread2_timer_func_t func,void *ths,void *hook)
{
	wtk_thread2_timer_t *timer,*tx;
	wtk_queue_node_t *qn,*suc;

	timer=wtk_thread2_timer_new(delay,func,ths,hook);
	//wtk_debug("time_q=%d\n",dlg->time_q.length);
	if(dlg->time_q.length>0)
	{
		suc=NULL;
		for(qn=dlg->time_q.pop;qn;qn=qn->next)
		{
			tx=data_offset2(qn,wtk_thread2_timer_t,q_n);
			if(tx->t>timer->t)
			{
				suc=qn;
				break;
			}
		}
		if(suc)
		{
			wtk_queue_insert_before(&(dlg->time_q),suc,&(timer->q_n));
		}else
		{
			wtk_queue_push(&(dlg->time_q),&(timer->q_n));
		}
	}else
	{
		wtk_queue_push(&(dlg->time_q),&(timer->q_n));
	}
}

int wtk_thread2_run(wtk_thread2_t *t2,wtk_thread_t *t)
{
typedef enum
{
	WTK_THREAD2_INIT,
	WTK_THREAD2_DATA,
}wtk_thread2_state_t;
	wtk_queue_node_t *qn;
	wtk_thread2_msg_t *msg;
	wtk_thread2_state_t state;
	int ret;

	state=WTK_THREAD2_INIT;
	while(t2->run)
	{
		qn=wtk_blockqueue_pop(&(t2->input_q),100,NULL);
		if(!qn)
		{
			wtk_thread2_touch_timer(t2);
			continue;
		}
		msg=data_offset2(qn,wtk_thread2_msg_t,q_n);
		switch(state)
		{
		case WTK_THREAD2_INIT:
			if(msg->type==WTK_THREAD2_START)
			{
				if(t2->start)
				{
					ret=t2->start(t2->ths);
					if(ret==0)
					{
						state=WTK_THREAD2_DATA;
					}else
					{
						wtk_debug("err[%s] start failed\n",t->name);
					}
				}else
				{
					state=WTK_THREAD2_DATA;
				}
			}else
			{
				//wtk_debug("err[%s] msg: %d %p\n",t->name,msg->type,t2->err);
				if(t2->err)
				{
					t2->err(t2->ths,msg->data);
				}
			}
			break;
		case WTK_THREAD2_DATA:
			switch(msg->type)
			{
			case WTK_THREAD2_START:
				wtk_debug("err[%s] msg: %d\n",t->name,msg->type);
				break;
			case WTK_THREAD2_STOP:
				if(t2->stop)
				{
					t2->stop(t2->ths);
				}
				//wtk_debug("notify end....\n");
				//wtk_debug("notify end2 %p....\n",t);
				state=WTK_THREAD2_INIT;
				break;
			case WTK_THREAD2_PROCESS:
				if(t2->process)
				{
					t2->process(t2->ths,msg->data);
				}
				break;
			}
			break;
		}
		if(msg->type==WTK_THREAD2_STOP)
		{
			wtk_sem_release(&(t2->notify_sem),1);
		}
		wtk_thread2_msg_delete(msg);
	}
	return 0;
}


