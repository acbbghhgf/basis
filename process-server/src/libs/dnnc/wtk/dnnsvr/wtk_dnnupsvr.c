#include "wtk_dnnupsvr.h" 
#include "wtk/os/wtk_cpu.h"
void wtk_dnnupsvr_delete_cudnn(wtk_dnnupsvr_t *up,wtk_cudnn_t *cudnn);

wtk_dnnupsvr_msg_t* wtk_dnnupsvr_msg_new(void *hook,char *data,int len)
{
	wtk_dnnupsvr_msg_t *msg;

	msg=(wtk_dnnupsvr_msg_t*)wtk_malloc(sizeof(wtk_dnnupsvr_msg_t));
	msg->hook.parser=hook;
	if(data)
	{
		msg->type=WTK_DNNUPSVR_MSG_DAT;
		msg->dat=wtk_string_dup_data(data,len);
	}else
	{
		msg->type=WTK_DNNUPSVR_MSG_STOP;
		msg->dat=NULL;
	}
	return msg;
}


void wtk_dnnupsvr_msg_delete(wtk_dnnupsvr_msg_t *msg)
{
	if(msg->dat)
	{
		wtk_string_delete(msg->dat);
	}
	wtk_free(msg);
}

wtk_dnn_calc_msg_t* wtk_dnnupsvr_new_calc_msg(wtk_dnnupsvr_t *up)
{
	wtk_dnn_calc_msg_t *msg;

	msg=(wtk_dnn_calc_msg_t*)wtk_malloc(sizeof(wtk_dnn_calc_msg_t));
	msg->input=wtk_cudnn_matrix_new(up->cfg->dnn.cache_size,up->input_size);
	msg->output=wtk_cudnn_matrix_new(up->cfg->dnn.cache_size,up->output_size);
	return msg;
}

int wtk_dnn_calc_msg_delete(wtk_dnn_calc_msg_t *msg)
{
	wtk_cudnn_matrix_delete(msg->output);
	wtk_cudnn_matrix_delete(msg->input);
	wtk_free(msg);
	return 0;
}

wtk_dnn_calc_msg_t* wtk_dnnupsvr_pop_calc_msg(wtk_dnnupsvr_t *up)
{
	return wtk_hoard_pop(&(up->calc_msg_hoard));
}

void wtk_dnnupsvr_push_calc_msg(wtk_dnnupsvr_t *up,wtk_dnn_calc_msg_t *msg)
{
	wtk_hoard_push(&(up->calc_msg_hoard),msg);
}

int wtk_dnnupsvr_process_dnn_env_init(wtk_dnnupsvr_t *upsvr,wtk_thread_t *thread)
{
	wtk_cudnn_cfg_t *cfg;
	wtk_cudnn_gpu_cfg_t *gpu;
	int i;

	cfg=&(upsvr->cfg->dnn);
	if(cfg->use_cuda)
	{
		i=(thread-upsvr->pool->threads)/upsvr->cfg->thread_per_gpu;//sizeof(wtk_thread_t);
		//i=i/upsvr->cfg->thread_per_gpu;
		//wtk_debug("thread used=%d\n",i);
		wtk_log_log(upsvr->log,"use gpu=%d.",i);
		gpu=cfg->gpu[i];
		thread->app_data=wtk_cudnn_env_new(&(upsvr->cfg->dnn),gpu);
	}else
	{
		thread->app_data=wtk_cudnn_env_new(&(upsvr->cfg->dnn),NULL);
	}
	//wtk_debug("thread=%p env=%p\n",thread,thread->app_data);
	return 0;
}

int wtk_dnnupsvr_process_dnn_env_clean(wtk_dnnupsvr_t *upsvr,wtk_thread_t *thread)
{
	wtk_cudnn_env_delete((wtk_cudnn_env_t*)thread->app_data);
	return 0;
}

int wtk_dnnupsvr_process_dnn_env(wtk_dnnupsvr_t *upsvr,wtk_queue_node_t *qn,wtk_thread_t *thread)
{
	wtk_dnn_calc_msg_t *msg;
	wtk_cudnn_env_t *env=(wtk_cudnn_env_t*)thread->app_data;
	wtk_cudnn_matrix_t *output;
	int ret;
	wtk_dnnupsvr_msg_t *xmsg;

	msg=data_offset2(qn,wtk_dnn_calc_msg_t,q_n);
	//wtk_debug("==================== idx=%d ====================\n",msg->idx);
	//wtk_debug("thread=%p env=%p\n",thread,thread->app_data);
	if(upsvr->cfg->debug)
	{
		wtk_debug("%s:%d process msg=%d/%p matrix=%d/%d\n",msg->parser->c->peer_name,msg->parser->c->peer_port,msg->idx,msg,
				msg->input->row,msg->input->col);
	}
	if(!msg->parser->c->evt.want_close)
	{
		if(upsvr->cfg->dnn.use_cuda)
		{
			ret=wtk_cudnn_env_process(env,msg->input);
			if(ret==0)
			{
				output=env->host_output;
				env->host_output=msg->output;
				msg->output=output;
			}else
			{
				msg->output->row=0;
			}
		}else
		{
			//print_float(msg->input->v,10);
			ret=wtk_cudnn_env_process_cpu(env,msg->input);
			if(ret==0)
			{
				output=env->output[upsvr->cfg->dnn.nlayer-1];
				env->output[upsvr->cfg->dnn.nlayer-1]=msg->output;
				msg->output=output;
			}else
			{
				msg->output->row=0;
			}
			//print_float(msg->input->v,10);
			//print_float(output->v,10);
			//exit(0);
		}
	}
	xmsg=wtk_dnnupsvr_msg_new(msg,NULL,0);
	xmsg->type=WTK_DNNUPSVR_MSG_CALC;
	wtk_blockqueue_push(&(upsvr->input_q),&(xmsg->q_n));
	return 0;
}

wtk_dnnupsvr_t* wtk_dnnupsvr_new(wtk_dnnupsvr_cfg_t *cfg)
{
	wtk_dnnupsvr_t *up;
	int n;

	up=(wtk_dnnupsvr_t*)wtk_malloc(sizeof(wtk_dnnupsvr_t));
	up->cfg=cfg;
	up->run=0;
	up->log=NULL;
        up->input_size = 0;
        up->output_size = 0;
        up->pool = NULL;
	up->pipeq=NULL;
	return up;
}

void wtk_dnnupsvr_delete(wtk_dnnupsvr_t *up)
{
	wtk_thread_pool_delete(up->pool);
	wtk_blockqueue_clean(&(up->thread_q));
	wtk_blockqueue_clean(&(up->input_q));
	wtk_hoard_clean(&(up->calc_msg_hoard));
	wtk_free(up);
}

void wtk_dnnupsvr_init(wtk_dnnupsvr_t *up) {
    int n;
    wtk_dnnupsvr_cfg_t* cfg = up->cfg;

  	if(cfg->dnn.use_cuda)
	{
        if (cfg->dnn.gpu) {
            up->input_size=cfg->dnn.gpu[0]->trans->b->len;
            up->output_size=cfg->dnn.gpu[0]->layer[cfg->dnn.nlayer-1]->b->len;
        }
	}else
	{
		up->input_size=cfg->dnn.trans->b->len;
		up->output_size=cfg->dnn.layer[cfg->dnn.nlayer-1]->b->len;
	}
	wtk_blockqueue_init(&(up->input_q));
	wtk_hoard_init(&(up->calc_msg_hoard),offsetof(wtk_dnn_calc_msg_t,hoard_n),cfg->cache_size,
                   (wtk_new_handler_t)wtk_dnnupsvr_new_calc_msg,(wtk_delete_handler_t)wtk_dnn_calc_msg_delete,up);
	wtk_blockqueue_init(&(up->thread_q));
	if(cfg->dnn.use_cuda)
	{
		n=cfg->dnn.ngpu*cfg->thread_per_gpu;
	}else
	{
		n=wtk_get_cpus();
	}
	up->pool=wtk_thread_pool_new2(n,&(up->thread_q),(wtk_thread_pool_init_f)wtk_dnnupsvr_process_dnn_env_init,
                                  (wtk_thread_pool_clean_f)wtk_dnnupsvr_process_dnn_env_clean,
                                  (thread_pool_handler)wtk_dnnupsvr_process_dnn_env,up);
}

void wtk_dnnupsvr_start(wtk_dnnupsvr_t *up)
{
	up->run=1;
	wtk_thread_pool_start(up->pool);
}

void wtk_dnnupsvr_stop(wtk_dnnupsvr_t *up)
{
	up->run=0;
	wtk_thread_pool_stop(up->pool);
	wtk_blockqueue_wake(&(up->input_q));
}

void wtk_dnnupsvr_raise_cudnn_input(wtk_dnnupsvr_t *up,wtk_cudnn_t *cudnn,wtk_cudnn_matrix_t *input)
{
	wtk_dnn_calc_msg_t *msg;
	wtk_msg_parser_t *p;

	p=(wtk_msg_parser_t*)(cudnn->hook);
	if(p->c->evt.want_close)
	{

	}else
	{
		++cudnn->calc_cnt;
		msg=wtk_dnnupsvr_pop_calc_msg(up);
		cudnn->input=msg->input;
		msg->input=input;
		msg->parser=p;
		msg->idx=cudnn->idx;
		//wtk_debug("=============== raise idx=%d ================\n",msg->idx);
		if(up->cfg->debug)
		{
			wtk_debug("%s:%d raise msg=%d/%p matrix=%d/%d\n",msg->parser->c->peer_name,msg->parser->c->peer_port,
					msg->idx,msg,input->row,input->col);
		}
		wtk_blockqueue_push(&(up->thread_q),&(msg->q_n));
	}
}

void wtk_dnnupsvr_process_calc_msg(wtk_dnnupsvr_t *up,wtk_dnnupsvr_msg_t *msg)
{
	wtk_dnn_calc_msg_t *xmsg;
	wtk_cudnn_t *cudnn;
	wtk_queue_node_t *qn;
	wtk_queue_t *q;
	wtk_dnnupsvr_msg_t *msg2;

	xmsg=msg->hook.calc_msg;
	if(up->cfg->debug)
	{
		wtk_debug("%s:%d recv msg=%d/%p matrix=%d/%d\n",xmsg->parser->c->peer_name,xmsg->parser->c->peer_port,xmsg->idx,xmsg,
				xmsg->input->row,xmsg->input->col);
	}
	cudnn=(wtk_cudnn_t*)(xmsg->parser->hook);
	q=&(cudnn->recv_q);
	--cudnn->calc_cnt;
	//wtk_debug("idx=%d row=%d/%d idx=%d/%d calc_cnt=%d\n",xmsg->idx,xmsg->input->row,xmsg->output->row,xmsg->idx,cudnn->recv_idx,cudnn->calc_cnt);
	if(cudnn->want_delete)
	{
		wtk_queue_push(q,&(msg->q_n));
		if(cudnn->calc_cnt==0)
		{
			wtk_msg_parser_t *parser;

			//wtk_debug("============== q=%d ============\n",q->length);
			parser=(wtk_msg_parser_t*)(cudnn->hook);
			parser->hook=NULL;
			wtk_dnnupsvr_delete_cudnn(up,cudnn);
			msg=wtk_dnnupsvr_msg_new(parser,NULL,0);
			msg->type=WTK_DNNUPSVR_MSG_STOP_END;
			//wtk_debug("====================== push close use=%d free=%d ================\n",up->calc_msg_hoard.use_length,up->calc_msg_hoard.cur_free);
			wtk_pipequeue_push(up->pipeq,&(msg->q_n));
		}
	}else
	{
		if(xmsg->idx==cudnn->recv_idx)
		{
			//wtk_debug("========= use idx=%d ======\n",xmsg->idx);
			//wtk_debug("=========== push msg ============\n");
			if(up->cfg->debug)
			{
				wtk_debug("%s:%d sendnk msg=%d/%p matrix=%d/%d\n",msg->hook.calc_msg->parser->c->peer_name,msg->hook.calc_msg->parser->c->peer_port,
						msg->hook.calc_msg->idx,msg->hook.calc_msg,
						msg->hook.calc_msg->input->row,msg->hook.calc_msg->input->col);
			}
			wtk_pipequeue_push(up->pipeq,&(msg->q_n));
			++cudnn->recv_idx;
			while(q->pop)
			{
				msg=data_offset2(q->pop,wtk_dnnupsvr_msg_t,q_n);
				//wtk_debug("idx=%d/%d\n",msg->hook.calc_msg->idx,cudnn->recv_idx)
				if(msg->hook.calc_msg->idx==cudnn->recv_idx)
				{
					//wtk_debug("========= push idx=%d q=%d ======\n",msg->hook.calc_msg->idx,q->length);
					//wtk_debug("=========== push msg ============\n");
					if(up->cfg->debug)
					{
						wtk_debug("%s:%d sendnk msg=%d/%p matrix=%d/%d\n",msg->hook.calc_msg->parser->c->peer_name,msg->hook.calc_msg->parser->c->peer_port,
								msg->hook.calc_msg->idx,msg->hook.calc_msg,
								msg->hook.calc_msg->input->row,msg->hook.calc_msg->input->col);
					}
					wtk_queue_pop(q);
					wtk_pipequeue_push(up->pipeq,&(msg->q_n));
					++cudnn->recv_idx;
				}else
				{
					break;
				}
			}
			//wtk_debug("============================== pop=%p  len=%d\n",q->pop,q->length);
			if(cudnn->is_end && cudnn->recv_idx==cudnn->idx)
			{
				if(up->cfg->debug)
				{
					wtk_debug("want end idx=%d/%d\n",cudnn->recv_idx,cudnn->idx);
				}
				wtk_cudnn_reset(cudnn);
				msg=wtk_dnnupsvr_msg_new(cudnn->hook,NULL,0);
				msg->type=WTK_DNNUPSVR_MSG_CALC_END;
				wtk_pipequeue_push(up->pipeq,&(msg->q_n));
			}
		}else
		{
			for(qn=q->pop;qn;qn=qn->next)
			{
				msg2=data_offset2(qn,wtk_dnnupsvr_msg_t,q_n);
				//wtk_debug("idx=%d\n",msg2->idx);
				if(xmsg->idx<msg2->hook.calc_msg->idx)
				{
					wtk_queue_insert_before(q,qn,&(msg->q_n));
					msg=NULL;
					break;
				}
			}
			if(msg)
			{
				wtk_queue_push(q,&(msg->q_n));
			}
		}
	}
}

void wtk_dnnupsvr_process_msg(wtk_dnnupsvr_t *up,wtk_dnnupsvr_msg_t *msg)
{
	wtk_msg_parser_t *parser;
	wtk_cudnn_t *cudnn;
	char cmd;

	parser=msg->hook.parser;
	cmd=msg->dat->data[0];
	//wtk_debug("====================== cmd=%d =====================\n",cmd);
	switch(cmd)
	{
	case 0: //start
		//wtk_debug("======================= start hook=%p ===========================\n",parser->hook);
		if(!parser->hook)
		{
			parser->hook=cudnn=wtk_cudnn_new(&(up->cfg->dnn));
			cudnn->hook=parser;
			wtk_cudnn_set_raise(cudnn,up,(wtk_cudnn_raise_f)wtk_dnnupsvr_raise_cudnn_input);
		}
		if(up->cfg->debug)
		{
			wtk_debug("%s:%d start\n",parser->c->peer_name,parser->c->peer_port);
		}
		break;
	case 1: //dat;
		cudnn=(wtk_cudnn_t*)(parser->hook);
		if(up->cfg->debug)
		{
			wtk_debug("%s:%d recv %d.\n",parser->c->peer_name,parser->c->peer_port,(int)((msg->dat->len-1)/sizeof(float)));
		}
		if(cudnn)
		{
			wtk_cudnn_feed2(cudnn,(float*)(msg->dat->data+1),(msg->dat->len-1)/sizeof(float),0);
		}
		break;
	case 2://stop
		cudnn=(wtk_cudnn_t*)(parser->hook);
		if(up->cfg->debug)
		{
			wtk_debug("%s:%d recv stop.\n",parser->c->peer_name,parser->c->peer_port);
		}
		if(cudnn)
		{
			wtk_cudnn_feed2(cudnn,NULL,0,1);
			if(cudnn->idx==0)
			{
				msg=wtk_dnnupsvr_msg_new(cudnn->hook,NULL,0);
				msg->type=WTK_DNNUPSVR_MSG_CALC_END;
				wtk_pipequeue_push(up->pipeq,&(msg->q_n));
			}
		}
		break;
	}
//	{
//		cudnn=(wtk_cudnn_t*)(parser->hook);
//		wtk_debug("========== idx=%d ==============\n",cudnn->cnt);
//	}
}

void wtk_dnnupsvr_delete_cudnn(wtk_dnnupsvr_t *up,wtk_cudnn_t *cudnn)
{
	wtk_dnnupsvr_msg_t *msg;
	wtk_queue_node_t *qn;

	while(1)
	{
		qn=wtk_queue_pop(&(cudnn->recv_q));
		if(!qn){break;}
		msg=data_offset2(qn,wtk_dnnupsvr_msg_t,q_n);
		if(msg->hook.calc_msg)
		{
			wtk_dnnupsvr_push_calc_msg(up,msg->hook.calc_msg);
		}
		wtk_dnnupsvr_msg_delete(msg);
	}
	wtk_cudnn_delete(cudnn);
}

void wtk_dnnupsvr_process_msg_stop(wtk_dnnupsvr_t *up,wtk_dnnupsvr_msg_t *msg)
{
	wtk_msg_parser_t *parser;
	wtk_cudnn_t *cudnn;

	//wtk_debug("================== want stop ===============\n");
	parser=msg->hook.parser;
	cudnn=(wtk_cudnn_t*)(parser->hook);
	if(!cudnn->is_end)
	{
		wtk_cudnn_feed2(cudnn,NULL,0,1);
	}
	if(cudnn->calc_cnt==0)
	{
		if(cudnn)
		{
			wtk_dnnupsvr_delete_cudnn(up,cudnn);
			parser->hook=NULL;
		}
		msg->type=WTK_DNNUPSVR_MSG_STOP_END;
		//wtk_debug("====================== push close ================\n");
		wtk_pipequeue_push(up->pipeq,&(msg->q_n));
	}else
	{
		//wtk_debug("====================== delay close ================\n");
		cudnn->want_delete=1;
		wtk_dnnupsvr_msg_delete(msg);
	}
}

int wtk_dnnupsvr_run(wtk_dnnupsvr_t *up)
{
	wtk_queue_node_t *qn;
	wtk_dnnupsvr_msg_t *msg;
	int ki=0;

	//wtk_debug("=========== calc=%d/%d =========\n",up->calc_msg_hoard.use_length,up->calc_msg_hoard.cur_free);
	while(up->run)
	{
		qn=wtk_blockqueue_pop(&(up->input_q),-1,NULL);
		if(!qn){continue;}
		msg=data_offset2(qn,wtk_dnnupsvr_msg_t,q_n);
		//wtk_debug("=================== ki=%d ======================\n",++ki);
		switch(msg->type)
		{
		case WTK_DNNUPSVR_MSG_DAT:
			if(!msg->hook.parser->c->evt.want_close && msg->dat)
			{
				wtk_dnnupsvr_process_msg(up,msg);
			}
			wtk_dnnupsvr_msg_delete(msg);
			break;
		case WTK_DNNUPSVR_MSG_STOP:
			wtk_dnnupsvr_process_msg_stop(up,msg);
			//wtk_dnnupsvr_msg_delete(msg);
			break;
		case WTK_DNNUPSVR_MSG_CALC:
			//msg send to network to send dat
			wtk_dnnupsvr_process_calc_msg(up,msg);
			break;
		case WTK_DNNUPSVR_MSG_REUSE:
			if(msg->hook.calc_msg)
			{
				wtk_dnnupsvr_push_calc_msg(up,msg->hook.calc_msg);
			}
			wtk_dnnupsvr_msg_delete(msg);
			break;
		}
	}
	//wtk_debug("run end\n");
	return 0;
}


void wtk_dnnupsvr_feed(wtk_dnnupsvr_t *up,wtk_msg_parser_t *parser,char *data,int len)
{
	wtk_dnnupsvr_msg_t *msg;

	//wtk_debug("feed message len=%d\n",len);
	msg=wtk_dnnupsvr_msg_new(parser,data,len);
	wtk_blockqueue_push(&(up->input_q),&(msg->q_n));
}

void wtk_dnnupsvr_connection_want_close(wtk_dnnupsvr_t *up,wtk_msg_parser_t *parser)
{
	wtk_dnnupsvr_msg_t *msg;

	//wtk_debug("============ want close =============\n");
	//wtk_debug("============== paser=%p c=%p ==========\n",parser,parser->c);
	msg=wtk_dnnupsvr_msg_new(parser,NULL,0);
	wtk_blockqueue_push(&(up->input_q),&(msg->q_n));
}
