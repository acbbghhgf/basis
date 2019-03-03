#include "wtk_dnnc.h" 
#include "wtk/os/wtk_socket.h"
#include "wtk/os/wtk_fd.h"
int wtk_dnnc_run(wtk_dnnc_t *c,wtk_thread_t *t);

wtk_dnnc_t* wtk_dnnc_new(wtk_dnnc_cfg_t *cfg)
{
	wtk_dnnc_t *c;

	c=(wtk_dnnc_t*)wtk_malloc(sizeof(wtk_dnnc_t));
	c->cfg=cfg;
	c->fd=-1;
	//wtk_dnnc_connect(c);
	c->state=WTK_DNNC_STATE_INIT;
	c->rstate=WTK_DNNC_PARSER_INIT;
	c->run=1;
	c->rbuf=(char*)wtk_malloc(cfg->rw_size);
	c->wbuf=wtk_buf_new(1024);
	c->rxbuf=wtk_strbuf_new(10240,1);
	c->raise=NULL;
	c->ths=NULL;
	wtk_pipequeue_init(&(c->pipeq));
	wtk_thread_init(&(c->thread),(thread_route_handler)wtk_dnnc_run,c);
	wtk_thread_start(&(c->thread));
	return c;
}

void wtk_dnnc_delete(wtk_dnnc_t *c)
{
	c->run=0;
	wtk_thread_join(&(c->thread));
	wtk_thread_clean(&(c->thread));
	wtk_free(c->rbuf);
	wtk_buf_delete(c->wbuf);
	wtk_strbuf_delete(c->rxbuf);
	wtk_pipequeue_clean(&(c->pipeq));
	if(c->fd>=0)
	{
		close(c->fd);
	}
	wtk_free(c);
}


void wtk_dnnc_set_raise(wtk_dnnc_t *dnnc,void *ths,wtk_dnnc_raise_f raise)
{
	dnnc->raise=raise;
	dnnc->ths=ths;
}



wtk_dnnc_msg_t* wtk_dnnc_msg_new(float *f,int len,int is_end)
{
	wtk_dnnc_msg_t *msg;

	msg=(wtk_dnnc_msg_t*)wtk_malloc(sizeof(wtk_dnnc_msg_t));
	if(len>0)
	{
		msg->data=wtk_string_dup_data((char*)f,len*sizeof(float));
	}else
	{
		msg->data=NULL;
	}
	msg->is_end=is_end;
	return msg;
}

void wtk_dnnc_msg_delete(wtk_dnnc_msg_t *msg)
{
	if(msg->data)
	{
		wtk_string_delete(msg->data);
	}
	wtk_free(msg);
}

void wtk_dnnc_feed(wtk_dnnc_t *dnnc,float *f,int len,int is_end)
{
	wtk_dnnc_msg_t *msg;

	msg=wtk_dnnc_msg_new(f,len,is_end);
	wtk_pipequeue_push(&(dnnc->pipeq),&(msg->q_n));
//	if(is_end)
//	{
//		wtk_debug("=========== wait message ========\n");
//		wtk_sem_acquire(&(dnnc->end_sem),-1);
//		wtk_debug("=========== wait message end ========\n");
//	}
}

void wtk_dnnc_reset(wtk_dnnc_t *dnnc)
{
}

void wtk_dnnc_raise_msg(wtk_dnnc_t *c,char *data,int len)
{
	char cmd;

	cmd=c->cmd;
	//wtk_debug("============== cmd=%d ==============\n",cmd);
	if(c->cfg->debug)
	{
		wtk_debug("raise len=%d cmd=%d\n",len,cmd);
	}
	if(cmd==1)
	{
		//raise data;
		if(c->raise)
		{
			if(c->cfg->debug)
			{
				wtk_debug("raise idx=%d row=%d col=%d\n",c->v[0],c->v[1],c->v[2]);
			}
			c->raise(c->ths,cmd,c->v[1],c->v[2],(float*)(data));
		}
	}else if(cmd==2)
	{
		//raise msg;
		//end;
		//wtk_debug("========= end =========\n");
		if(c->raise)
		{
			c->raise(c->ths,cmd,0,0,NULL);
		}
		c->state=WTK_DNNC_STATE_INIT;
	}
}

int wtk_dnnc_feed_read_data(wtk_dnnc_t *c,char *data,int len)
{
	wtk_strbuf_t *buf=c->rxbuf;
	int i;

	//print_data(data,len);
	switch(c->rstate)
	{
	case WTK_DNNC_PARSER_INIT:
		if(len>=4)
		{
			c->msg_size.v=*((uint32_t*)(data));
			//wtk_debug("size=%d\n",c->msg_size.v);
			i=c->msg_size.v;
			if(i<=0)
			{
				return -1;
			}
			//print_hex(data,min(len,17));
			c->rstate=WTK_DNNC_PARSER_DATA_HDR;
			wtk_strbuf_reset(buf);
			if(len>4)
			{
				return wtk_dnnc_feed_read_data(c,data+4,len-4);
			}else
			{
				return 0;
			}
		}else
		{
			c->msg_size_pos=len;
			for(i=0;i<len;++i)
			{
				c->msg_size.data[i]=data[i];
			}
			c->rstate=WTK_DNNC_PARSER_HDR;
		}
		break;
	case WTK_DNNC_PARSER_HDR:
		if(len==0){return 0;}
		i=min(4-c->msg_size_pos,len);
		memcpy(c->msg_size.data+c->msg_size_pos,data,i);
		c->msg_size_pos+=i;
		if(c->msg_size_pos==4)
		{
			//wtk_debug("size=%d\n",c->msg_size.v);
			wtk_strbuf_reset(buf);
			c->rstate=WTK_DNNC_PARSER_DATA_HDR;
			if(len>i)
			{
				return wtk_dnnc_feed_read_data(c,data+i,len-i);
			}
		}
		break;
	case WTK_DNNC_PARSER_DATA_HDR:
		if(len==0){return 0;}
		c->cmd=data[0];
		c->msg_size.v-=1;
		if(c->cmd==1)
		{
			c->rstate=WTK_DNNC_PARSER_DATA_HDR2;
		}else
		{
			wtk_dnnc_raise_msg(c,buf->data,buf->pos);
			wtk_strbuf_reset(buf);
			c->rstate=WTK_DNNC_PARSER_INIT;
		}
		if(len>1)
		{
			return wtk_dnnc_feed_read_data(c,data+1,len-1);
		}
		break;
	case WTK_DNNC_PARSER_DATA_HDR2:
		if(len==0){return 0;}
		i=min(12-buf->pos,len);
		//wtk_debug("size=%d/%d/%d i=%d\n",c->msg_size.v,buf->pos,len,i);
		wtk_strbuf_push(buf,data,i);
		if(buf->pos>=12)
		{
			c->msg_size.v-=12;
			memcpy((char*)c->v,buf->data,12);
			//wtk_debug("%d/%d/%d\n",c->v[0],c->v[1],c->v[2]);
			c->rstate=WTK_DNNC_PARSER_DATA;
			wtk_strbuf_reset(buf);
			if(len>i)
			{
				return wtk_dnnc_feed_read_data(c,data+i,len-i);
			}
		}
		break;
	case WTK_DNNC_PARSER_DATA:
		//wtk_debug("msg_size=%d\n",p->msg_size.v);
		i=min(c->msg_size.v-buf->pos,len);
		//wtk_debug("size=%d/%d/%d i=%d\n",c->msg_size.v,buf->pos,len,i);
		wtk_strbuf_push(buf,data,i);
		if(buf->pos>=c->msg_size.v)
		{
			//print_data(p->buf->data,p->buf->pos);
			//wtk_debug("pos=%d\n",buf->pos);
			wtk_dnnc_raise_msg(c,buf->data,buf->pos);
			wtk_strbuf_reset(buf);
			c->rstate=WTK_DNNC_PARSER_INIT;
			if(len>i)
			{
				return wtk_dnnc_feed_read_data(c,data+i,len-i);
			}
		}
		break;
	}
	return 0;
}

int wtk_dnnc_read_svr(wtk_dnnc_t *c)
{
	wtk_fd_state_t s;
	int ret, readed;
	char *buf;
	int len;
	//int lac,max_active_count;
    int tot_readed;

	len = c->cfg->rw_size;
	buf = c->rbuf;
	ret = 0;
	s = WTK_OK;
	tot_readed=0;
	while(1)
	{
		s = wtk_fd_recv(c->fd, buf, len, &readed);
		if (s == WTK_EOF||(readed==0 && tot_readed==0 && s==WTK_OK))
		{
            //if read available, but read 0 bytes, for the remote connection is closed. this is for windows.
            s=WTK_EOF;
			break;
		}
		tot_readed+=readed;
		if (readed > 0)
		{
			//wtk_debug("read =%d\n",readed);
			ret=wtk_dnnc_feed_read_data(c,buf,readed);
			//wtk_debug("msg_size=%d \n",c->msg_size.v,c->rstate);
			if(ret!=0)
			{
				s=WTK_ERR;
				break;
			}
		}else
		{
			break;
		}
	}
	if(tot_readed==0)
	{
		return -1;
	}else
	{
		return (s==WTK_OK||s==WTK_AGAIN)?0:-1;
	}
}

#ifdef DEBUG_P
static int w1=0;
static int w2=0;
wtk_strbuf_t *xbuf=NULL;

void wtk_dnnc_check_write(wtk_buf_t *buf)
{
	int ret;
	int i;
	wtk_buf_it_t it,it2;
	char *p;
	int j;

	if(!xbuf){return;}
	if(w2+wtk_buf_len(buf)!=w1)
	{
		wtk_debug("======== found bug %d/%d/%d\n",w1,w2,wtk_buf_len(buf));
		exit(0);
	}
	it=wtk_buf_get_it(buf);
	j=0;
	///wtk_buf_print(buf);
	wtk_debug("========> %d/%d/%d item=%p\n",w1,w2,wtk_buf_len(buf),it.item);
	for(i=w2;i<xbuf->pos;++i)
	{
		++j;
		it2=it;
		p=wtk_buf_it_next(&it);
//		if(i+1<xbuf->pos && it.item==NULL)
//		{
//			exit(0);
//		}
		if(*p!=xbuf->data[i])
		{
			wtk_debug("found bug i=%d %#x %#x j=%d\n",i,*p,xbuf->data[i],j);
			wtk_debug("item pos=%d len=%d\n",it.pos,it.item->len);
			print_hex(xbuf->data+i,20);
			print_hex(p,20);
			//wtk_buf_print(buf);
			exit(0);
		}
	}
	wtk_debug("================ check j=%d \n",j);
}
#endif

int wtk_dnnc_write_flush(wtk_dnnc_t *c)
{
	wtk_buf_t *buf=c->wbuf;
	int ret;
	wtk_buf_item_t *item;
	wtk_fd_state_t s;
	int writed;

#ifdef DEBUG_P
	wtk_debug("============== flush len=%d ==============\n",wtk_buf_len(buf));
	wtk_dnnc_check_write(buf);
#endif
	item=buf->front;
	//wtk_debug("========> check buf %d/%d/%d\n",w1,w2,wtk_buf_len(buf));
	if(item->len>0)
	{
		s=WTK_OK;
		while(item->len>0)
		{
			//wtk_debug("pos=%d len=%d\n",item->pos,item->len);
			s=wtk_fd_send(c->fd,item->data+item->pos,item->len,&writed);
			//wtk_debug("pos=%d len=%d writed=%d s=%d\n",item->pos,item->len,writed,s);
			//perror(__FUNCTION__);
			//wtk_dnnc_check_write(buf);
#ifdef DEBUG_P
			w2+=writed;
#endif
			if(writed>0)
			{
				item->pos+=writed;
				item->len-=writed;
				if(item->len<=0)
				{
					if(item->next)
					{
						buf->front=item->next;
						wtk_buf_item_delete(item);
						item=buf->front;
					}else
					{
						item->pos=0;
						item->len=0;
						break;
					}
				}
			}
			if(s!=WTK_OK){break;}
		}
		ret=(s==WTK_OK||s==WTK_AGAIN)?0:-1;
	}else
	{
		ret=0;
	}
//end:
#ifdef DEBUG_P
	wtk_dnnc_check_write(buf);
#endif
	return ret;
}

int wtk_dnnc_write(wtk_dnnc_t *c,char *buf,int len)
{
	wtk_fd_state_t s;
	int writed, left;
	int ret;
	//static int ki=0;

#ifdef DEBUG_P
	if(xbuf==NULL)
	{
		xbuf=wtk_strbuf_new(10240,1);
	}
	//wtk_debug("pos=%d len=%d\n",xbuf->pos,len);
	wtk_strbuf_push(xbuf,buf,len);
	w1+=len;
#endif
	//wtk_debug("pos=%d len=%d\n",xbuf->pos,len);
	//ki+=len;
	//wtk_debug("============== flush=%d/%d ==============\n",len,ki);

	if(c->wbuf && c->wbuf->front->len>0)
	{
		//wtk_debug("========= check len=%d add %d =========\n",wtk_buf_len(c->wbuf),len);
		//print_hex(buf,min(len,10));
		//print_hex(xbuf->data+xbuf->pos-len,min(len,10));
		wtk_buf_push(c->wbuf,buf,len);
		//wtk_debug("========= added len=%d =========\n",wtk_buf_len(c->wbuf));
		//wtk_dnnc_check_write(c->wbuf);
		ret=wtk_dnnc_write_flush(c);
		if(ret!=0){goto end;}
	}else
	{
		writed = 0;
		//wtk_debug("len=%d\n",len);
		s=wtk_fd_send(c->fd, buf, len, &writed);
#ifdef DEBUG_P
		w2+=writed;
#endif
		left=len-writed;
		if(left>0)
		{
//			wtk_debug("w1=%d w2=%d pos=%d\n",w1,w2,xbuf->pos);
//			wtk_debug("len=%d\n",wtk_buf_len(c->wbuf));
//			wtk_debug("================ check x ====================\n");
//			print_hex(buf+writed,min(left,10));
//			print_hex(xbuf->data+xbuf->pos-left,min(left,10));
//			wtk_debug("================ check y ====================\n");
			wtk_buf_push(c->wbuf,buf+writed,left);
//			wtk_debug("len=%d\n",wtk_buf_len(c->wbuf));
//			wtk_dnnc_check_write(c->wbuf);
		}
		ret=(s == WTK_OK || s == WTK_AGAIN) ? 0 : -1;
	}
end:
#ifdef DEBUG_P
	wtk_dnnc_check_write(c->wbuf);
#endif
	return ret;
}

void wtk_dnnc_write_msg(wtk_dnnc_t *c,char cmd,char *data,int len)
{
	char buf[5];
	uint32_t *n;

	n=(uint32_t*)buf;
	*n=len+1;
	buf[4]=cmd;
	//wtk_debug("n=%d\n",*n);
	//wtk_debug("===============\n");
	//print_hex(buf,5);
	wtk_dnnc_write(c,(char*)buf,5);
	if(len>0)
	{
		wtk_dnnc_write(c,data,len);
	}
	//getchar();
}

void wtk_dnnc_read_msg(wtk_dnnc_t *c)
{
	wtk_pipequeue_t *q=&(c->pipeq);
	wtk_queue_node_t *qn;
	wtk_dnnc_msg_t *msg;

	//wtk_debug("======== len=%d ===========\n",q->length);
	while(q->length>0)
	{
		qn=wtk_pipequeue_pop(q);
		if(!qn){break;}
		msg=data_offset2(qn,wtk_dnnc_msg_t,q_n);
		if(c->cfg->debug)
		{
			wtk_debug("read msg=%p len=%d\n",msg,msg->data?msg->data->len:0);
		}
		//wtk_debug("read msg=%p ki=%d\n",msg,++ki);
		if(c->cfg->debug)
		{
			wtk_debug("read msg=%p\n",msg);
		}
		switch(c->state)
		{
		case WTK_DNNC_STATE_INIT:
			wtk_dnnc_write_msg(c,0,NULL,0);
			//wtk_debug("=========== start =======\n");
			c->state=WTK_DNNC_STATE_DAT;
			break;
		default:
			break;
		}
		if(msg->data)
		{
			//wtk_debug("============= len=%d ========\n",msg->data->len);
			//print_float((float*)msg->data->data,10);
			//exit(0);
			wtk_dnnc_write_msg(c,1,msg->data->data,msg->data->len);
		}
		if(msg->is_end)
		{
			wtk_dnnc_write_msg(c,2,NULL,0);
			c->state=WTK_DNNC_STATE_WAIT_END;
		}
		wtk_dnnc_msg_delete(msg);
	}
}

void wtk_dnnc_disconnect(wtk_dnnc_t *c)
{
	if(c->fd>=0)
	{
		close(c->fd);
		c->fd=-1;
	}
	switch(c->state)
	{
	case WTK_DNNC_STATE_INIT:
		break;
	case WTK_DNNC_STATE_DAT:
		c->state=WTK_DNNC_STATE_INIT;
		break;
	case WTK_DNNC_STATE_WAIT_END:
		if(c->raise)
		{
			c->raise(c->ths,3,0,0,NULL);
		}
		c->state=WTK_DNNC_STATE_INIT;
		break;
	}
}

int wtk_dnnc_run(wtk_dnnc_t *c,wtk_thread_t *t)
{
	wtk_dnnc_cfg_t *cfg=c->cfg;
	struct timeval tv,tv1;
	int ret;
	fd_set rset,wset;
	int maxfd;
	int b;
	int pfd=c->pipeq.pipe_fd[0];

	tv1.tv_sec=cfg->timeout/1000;
	tv1.tv_usec=(cfg->timeout%1000)*1e3;
	ret=wtk_socket_connect4(cfg->ip,cfg->port,&c->fd);
	if(cfg->debug)
	{
		wtk_debug("connect %s:%s ret=%d.\n",cfg->ip,cfg->port,ret);
		perror(__FUNCTION__);
	}
	if(ret==0 || c->fd>=0)
	{
		wtk_fd_set_nonblock(c->fd);
	}
	while(c->run)
	{
		if(c->fd>=0)
		{
//			FD_ZERO(&eset);
//			FD_SET(c->fd, &eset);
			FD_ZERO(&rset);
			FD_SET(c->fd, &rset);
			FD_SET(pfd, &rset);
			maxfd=max(c->fd,pfd)+1;
			tv=tv1;
			if(c->wbuf->front->len>0)
			{
				//wtk_debug("want write\n");
				FD_ZERO(&wset);
				FD_SET(c->fd, &wset);
				ret=select(maxfd,&rset,&wset,NULL,&tv);
			}else
			{
				ret=select(maxfd,&rset,NULL,NULL,&tv);
			}
			//wtk_debug("============= ret=%d fd=%d state=%d/%d ===============\n",ret,c->fd,c->state,c->rstate);
			if(ret>0)
			{
//				b=FD_ISSET(c->fd,&eset);
//				if(b)
//				{
//					wtk_debug("================= err =============\n");
//					wtk_dnnc_disconnect(c);
//					continue;
//				}
				b=FD_ISSET(c->fd,&rset);
				if(b)
				{
					//wtk_debug("================= read =============\n");
					ret=wtk_dnnc_read_svr(c);
					if(ret!=0)
					{
						//wtk_debug("================= disconnect =============\n");
						wtk_dnnc_disconnect(c);
						continue;
					}
				}
				if(c->wbuf->front->len>0)
				{
					b=FD_ISSET(c->fd,&wset);
					if(b)
					{
						ret=wtk_dnnc_write_flush(c);
						if(ret!=0)
						{
							//wtk_debug("================= disconnect =============\n");
							wtk_dnnc_disconnect(c);
							continue;
						}
					}
				}
				b=FD_ISSET(pfd,&rset);
				if(b)
				{
					wtk_dnnc_read_msg(c);
				}
			}
		}else
		{
			//wtk_debug("============ connect ==============\n");
			ret=wtk_socket_connect4(cfg->ip,cfg->port,&c->fd);
			if(cfg->debug)
			{
				wtk_debug("connect %s:%s ret=%d.\n",cfg->ip,cfg->port,ret);
				perror(__FUNCTION__);
			}
			if(ret==0 || c->fd>=0)
			{
				wtk_fd_set_nonblock(c->fd);
			}else
			{
				//wtk_debug("============ sleep ==============\n");
				wtk_msleep(cfg->try_sleep_time);
			}
		}
	}
	return 0;
}
