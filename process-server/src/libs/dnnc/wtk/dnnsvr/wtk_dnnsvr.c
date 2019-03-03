#include "wtk_dnnsvr.h" 

void wtk_dnnsvr_raise_msg(wtk_dnnsvr_t *svr,wtk_msg_parser_t *parser,char *data,int len)
{
	//static int ki=0;

	//wtk_debug("ki=%d len=%d\n",++ki,len);
	if(parser->c->evt.ref==0)
	{
		parser->c->evt.ref=1;
	}
	//wtk_debug("========== len=%d ============\n",len);
	wtk_dnnupsvr_feed(svr->upsvr,parser,data,len);
	//wtk_debug("============== calc=%d/%d ============\n",svr->upsvr->calc_msg_hoard.use_length,svr->upsvr->calc_msg_hoard.cur_free);
}

void wtk_dnnsvr_parser_want_close(wtk_msg_parser_t *parser)
{
	wtk_dnnsvr_t *svr;

	if(parser->c->evt.ref==1)
	{
		svr=(wtk_dnnsvr_t*)parser->ths;
		wtk_dnnupsvr_connection_want_close(svr->upsvr,parser);
	}
}

void wtk_dnnsvr_process_pipe(wtk_dnnsvr_t *svr)
{
	wtk_pipequeue_t *q=&(svr->nk->pipeq);
	wtk_queue_node_t *qn;
	wtk_dnnupsvr_msg_t *msg;
	wtk_msg_parser_t *parser;
	wtk_connection_t *c;
	wtk_dnn_calc_msg_t *calc_msg;
	wtk_cudnn_t *cudnn;
	//uint32_t n;
	char buf[32];
	uint32_t *v;
	int nx;
	int skip_frame=svr->upsvr->cfg->dnn.skip_frame;
	int i,j;
	float *f;

	//wtk_debug("======== len=%d ===========\n",q->length);
	while(q->length>0)
	{
		qn=wtk_pipequeue_pop(q);
		if(!qn){break;}
		msg=data_offset2(qn,wtk_dnnupsvr_msg_t,q_n);
		switch(msg->type)
		{
		case WTK_DNNUPSVR_MSG_CALC:
			calc_msg=msg->hook.calc_msg;
			//wtk_debug("========= idx=%d want_close=%d ============\n",calc_msg->idx,c->evt.want_close);
			parser=calc_msg->parser;
			c=parser->c;
			if(svr->upsvr->cfg->debug)
			{
				wtk_debug("network %s:%d msg=%d/%p matrix=%d/%d\n",msg->hook.calc_msg->parser->c->peer_name,msg->hook.calc_msg->parser->c->peer_port,
						msg->hook.calc_msg->idx,msg->hook.calc_msg,
						msg->hook.calc_msg->input->row,msg->hook.calc_msg->input->col);
			}
			if(!c->evt.want_close)
			{
				//wtk_debug("output=%d/%d\n",calc_msg->output->row,calc_msg->output->col);
				if(skip_frame==0)
				{
					nx=calc_msg->output->row*calc_msg->output->col*sizeof(float);
					v=(uint32_t*)buf;
					v[0]=nx+13;
					//wtk_debug("v=%d\n",v[0]);
					buf[4]=1;
					v=(uint32_t*)(buf+5);
					v[0]=calc_msg->idx;
					v[1]=calc_msg->output->row;
					v[2]=calc_msg->output->col;
					//print_hex(buf,17);
					//wtk_debug("nx=%d\n",nx+17);
					wtk_connection_write(c,buf,17);
					//wtk_debug("============ send ==============\n");
					//print_float(calc_msg->output->v,10);
					wtk_connection_write(c,(char*)(calc_msg->output->v),nx);
				}else
				{
					nx=calc_msg->output->row*skip_frame*calc_msg->output->col*sizeof(float);
					v=(uint32_t*)buf;
					v[0]=nx+13;
					//wtk_debug("v=%d\n",v[0]);
					buf[4]=1;
					v=(uint32_t*)(buf+5);
					v[0]=calc_msg->idx;
					v[1]=calc_msg->output->row*skip_frame;
					v[2]=calc_msg->output->col;
					//print_hex(buf,17);
					//wtk_debug("nx=%d\n",nx+17);
					wtk_connection_write(c,buf,17);
					//wtk_debug("============ send ==============\n");
					//print_float(calc_msg->output->v,10);
					f=calc_msg->output->v;
					nx=calc_msg->output->col*sizeof(float);
					for(i=0;i<calc_msg->output->row;++i,f+=calc_msg->output->col)
					{
						for(j=0;j<skip_frame;++j)
						{
							wtk_connection_write(c,(char*)f,nx);
						}
					}
				}
			}
			break;
		case WTK_DNNUPSVR_MSG_CALC_END:
			if(svr->upsvr->cfg->debug)
			{
				wtk_debug("network %s:%d end\n",msg->hook.parser->c->peer_name,msg->hook.parser->c->peer_port);
			}
			v=(uint32_t*)buf;
			v[0]=1;
			buf[4]=2;
			c=msg->hook.parser->c;
			msg->hook.parser=NULL;
			wtk_connection_write(c,buf,5);
			break;
		case WTK_DNNUPSVR_MSG_STOP_END:
			//wtk_debug("=============== close connection ===============\n");
			if(svr->upsvr->cfg->debug)
			{
				wtk_debug("network %s:%d close\n",msg->hook.parser->c->peer_name,msg->hook.parser->c->peer_port);
			}
			parser=msg->hook.parser;
			c=parser->c;
			msg->hook.parser=NULL;
			c->evt.ref=0;
			wtk_connection_clean(c);
			wtk_nk_push_connection(c->nk, c);
			break;
		default:
			break;
		}
		msg->type=WTK_DNNUPSVR_MSG_REUSE;
		wtk_blockqueue_push(&(svr->upsvr->input_q),qn);
	}
	//wtk_debug("============== calc= use=%d free=%d ============\n",svr->upsvr->calc_msg_hoard.use_length,svr->upsvr->calc_msg_hoard.cur_free);
}

wtk_msg_parser_t* wtk_dnnsvr_new_parser(wtk_dnnsvr_t *svr,wtk_connection_t *c)
{
	wtk_msg_parser_t *p;

	p=wtk_msg_parser_new(c);
	wtk_msg_parser_set_raise(p,svr,(wtk_msg_parser_raise_f)wtk_dnnsvr_raise_msg);
	return p;
}

void wtk_dnnsvr_init_nk(wtk_dnnsvr_t *svr)
{
	wtk_nk_t *nk=svr->nk;

	nk->parser_factory.ths=svr;
	nk->parser_factory.newx=(wtk_nk_parser_new_f)wtk_dnnsvr_new_parser;
	nk->parser_factory.deletex=(wtk_nk_parser_delete_f)wtk_msg_parser_delete;
	nk->parser_factory.feed=(wtk_nk_parser_feed_f)wtk_msg_parser_feed;
	nk->parser_factory.want_close=(wtk_nk_parser_want_close_f)wtk_dnnsvr_parser_want_close;
	nk->parser_factory.clean=(wtk_nk_parser_clean_f)wtk_msg_parser_reset;
}


wtk_dnnsvr_t *wtk_dnnsvr_new(wtk_dnnsvr_cfg_t *cfg)
{
	wtk_dnnsvr_t *svr;

	svr=(wtk_dnnsvr_t*)wtk_malloc(sizeof(wtk_dnnsvr_t));
	svr->cfg=cfg;
	svr->nk=wtk_nk_new(&(cfg->nk));
	svr->upsvr=wtk_dnnupsvr_new(&(cfg->upsvr));
	svr->upsvr->log=svr->nk->log;
	svr->upsvr->pipeq=&(svr->nk->pipeq);
	wtk_nk_set_process_pipe(svr->nk,svr,(wtk_nk_process_pipe_f)wtk_dnnsvr_process_pipe);
	wtk_dnnsvr_init_nk(svr);
	return svr;
}

void wtk_dnnsvr_delete(wtk_dnnsvr_t *svr)
{
	int i;

	for(i=0;i<2;++i)
	{
		wtk_thread_clean(svr->thread+i);
	}
	wtk_nk_delete(svr->nk);
	wtk_dnnupsvr_delete(svr->upsvr);
	wtk_free(svr);
}

int wtk_dnnsvr_start(wtk_dnnsvr_t *svr)
{
	int i;

	wtk_nk_start(svr->nk);
	wtk_dnnupsvr_start(svr->upsvr);
	wtk_thread_init(svr->thread+0,(thread_route_handler)wtk_nk_run,svr->nk);
	wtk_thread_init(svr->thread+1,(thread_route_handler)wtk_dnnupsvr_run,svr->upsvr);
	for(i=0;i<2;++i)
	{
		wtk_thread_start(svr->thread+i);
	}
	return 0;
}

int wtk_dnnsvr_stop(wtk_dnnsvr_t *svr)
{
	wtk_dnnupsvr_stop(svr->upsvr);
	wtk_nk_stop(svr->nk);
	return 0;
}

int wtk_dnnsvr_join(wtk_dnnsvr_t *svr)
{
	int i;

	for(i=0;i<2;++i)
	{
		wtk_thread_join(svr->thread+i);
	}
	return 0;
}
