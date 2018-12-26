#include "wtk_httpc_request.h"
#include "wtk/os/wtk_socket.h"

int wtk_httpc_request_connect2(char *ip,char *port,int *pfd,int timeout,int use_hack);
static wtk_string_t *client=NULL;

void wtk_httpc_request_set_client(wtk_string_t *v)
{
	client=v;
}

int wtk_httpc_request_connect(char *ip,char *port,int *pfd,int timeout)
{
	return wtk_httpc_request_connect2(ip,port,pfd,timeout,0);
}


int wtk_httpc_request_connect2(char *ip,char *port,int *pfd,int timeout,int use_hack)
{
	wtk_addrinfo_t* info=NULL;
	int ret=-1;
	int fd=-1;
	struct timeval tv;
	fd_set set;

	if(!ip){goto end;}
#ifdef WIN32
    info=wtk_addrinfo_get(ip,port);
#else
    if(use_hack)
    {
    	info=wtk_addrinfo_get3(ip,port,timeout);
    }else
    {
    	info=wtk_addrinfo_get2(ip,port);
    }
#endif
	if(!info)
	{
		wtk_debug("%s:%s not found.\n",ip,port);
		goto end;
	}
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	if(timeout<0)
	{
		ret=connect(fd,info->addr,info->addrlen);
	}else
	{
		ret=wtk_fd_set_nonblock(fd);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0){goto end;}
		ret=connect(fd,info->addr,info->addrlen);
		//wtk_debug("ret=%d\n",ret);
		if(ret!=0)
		{
			tv.tv_sec=timeout/1000;
			tv.tv_usec=(timeout%1000)*1e3;
			FD_ZERO(&set);
			FD_SET(fd, &set);
			if(select(fd+1,NULL,&set,NULL,&tv)>0)
			{
				wtk_fd_set_block(fd);
				ret=0;
			}else
			{
				ret=-1;
				goto end;
			}
		}else
		{
			wtk_fd_set_block(fd);
		}
	}
	if(ret!=0){goto end;}
	if(timeout>0)
	{
		tv.tv_sec=timeout/1000;
		tv.tv_usec=(timeout%1000)*1e3;
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
		setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
	}
	*pfd=fd;
#ifndef WIN32
	//wtk_debug("connect %s:%s\n",ip,port);
#endif
end:
	if(info){wtk_addrinfo_delete(info);}
	if((ret!=0) && (fd>0))
	{
		wtk_socket_close_fd(fd);
	}
	return ret;
}

wtk_httpc_request_t* wtk_httpc_request_new(wtk_httpc_cfg_t *cfg)
{
	wtk_httpc_request_t *req=0;
	int fd=0,ret;

	ret=wtk_httpc_request_connect2(cfg->ip.data,cfg->port.data,&fd,cfg->timeout,cfg->use_hack);
	if(ret!=0)
	{
		fd=-1;
		//goto end;
	}
	req=wtk_httpc_request_new_fd(fd,&(cfg->url));
	req->ip=req->port=NULL;
	if(cfg->ip.len>0)
	{
		req->ip=cfg->ip.data;
	}
	if(cfg->port.len>0)
	{
		req->port=cfg->port.data;
	}
	req->add_hdr_f=0;
	req->cfg=cfg;
	return req;
}

wtk_httpc_request_t* wtk_httpc_request_new_addr(wtk_string_t *url,struct sockaddr* addr,socklen_t len)
{
	wtk_httpc_request_t *req=0;
	int fd,ret=-1;

	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	ret=connect(fd,addr,len);
	if(ret!=0){goto end;}
	req=wtk_httpc_request_new_fd(fd,url);
end:
	if((ret!=0) && (fd>0))
	{
		wtk_socket_close_fd(fd);
	}
	return req;
}

wtk_httpc_request_t* wtk_httpc_request_new_fd(int fd,wtk_string_t *url)
{
	wtk_httpc_request_t *req=0;

	req=(wtk_httpc_request_t*)wtk_malloc(sizeof(*req));
	req->cfg=0;
	req->ip=0;req->port=0;
	req->fd=fd;
	req->url=url;
	req->buf=wtk_strbuf_new(4096,1);
	req->response=wtk_http_response_new();
	req->body_notify_f=NULL;
	req->body_notify_ths=NULL;
	req->log=NULL;
	return req;
}

void wtk_httpc_request_set_notify(wtk_httpc_request_t *req,void *ths,wtk_httpc_response_notify_f notify)
{
	req->body_notify_f=notify;
	req->body_notify_ths=ths;
}

void wtk_httpc_request_close(wtk_httpc_request_t *req)
{
	//perror(__FUNCTION__);
	//wtk_debug("close ....\n");
	if(req->fd>0)
	{
		wtk_socket_close_fd(req->fd);
		req->fd=-1;
	}
}

int wtk_httpc_request_reconnect(wtk_httpc_request_t *req)
{
	wtk_httpc_request_close(req);
	//wtk_debug("=============> ip=%p/%p\n",req->ip,req->cfg->ip.data);
	if(req->ip)
	{
		return wtk_httpc_request_connect2(req->ip,req->port,&(req->fd),req->cfg->timeout,req->cfg->use_hack);
	}else if(req->cfg && req->cfg->ip.len>0)
	{
		if(req->cfg->port.len>0)
		{
			return wtk_httpc_request_connect2(req->cfg->ip.data,req->cfg->port.data,&(req->fd),req->cfg->timeout,req->cfg->use_hack);
		}else
		{
			return wtk_httpc_request_connect2(req->cfg->ip.data,"80",&(req->fd),req->cfg->timeout,req->cfg->use_hack);
		}
	}else
	{
		return -1;
	}
}

int wtk_httpc_request_delete(wtk_httpc_request_t *req)
{
	wtk_httpc_request_close(req);
	wtk_strbuf_delete(req->buf);
	wtk_http_response_delete(req->response);
	wtk_free(req);
	return 0;
}

int wtk_httpc_reqeust_preapre(wtk_httpc_request_t *req)
{
	int ret=0;

	if(req->fd<0)
	{
		ret=wtk_httpc_request_reconnect(req);
	}
	return ret;
}

int wtk_httpc_request_req(wtk_httpc_request_t *req,char *param,int param_bytes,int is_post,int use_1_1)
{
	int run=1;

	return wtk_httpc_request_req2(req,param,param_bytes,is_post,use_1_1,&run);
}

int wtk_httpc_request_req2(wtk_httpc_request_t *req,char *param,int param_bytes,int is_post,int use_1_1,int *run)
{
//#define DEBUG_HTTPC
//#define DEBUG_ERR
	wtk_strbuf_t *buf=req->buf;
	wtk_http_response_t *rep=req->response;
	wtk_fd_state_t s;
	int ret,writed,readed,left;
	wtk_string_t *v;
	int close;
	int last_pos;
	double t;

#ifdef DEBUG_ERR
	wtk_debug("================== test round url=[%s:%s %.*s]\n",req->ip,req->port,req->url->len,req->url->data);
#endif
	//wtk_debug("use=%d\n",use_1_1);
	wtk_http_response_reset(rep);
	ret=wtk_httpc_reqeust_preapre(req);
	if(ret!=0)
	{
		if(req->log)
		{
			wtk_log_log(req->log,"prepare request failed ip=[%.*s] timeout=%d.",req->cfg->ip.len,req->cfg->ip.data,req->cfg->timeout);
		}else
		{
			wtk_debug("prepare request failed ip=[%.*s] timeout=%d.\n",req->cfg->ip.len,req->cfg->ip.data,req->cfg->timeout);
		}
		goto end;
	}
	wtk_strbuf_reset(buf);
	if(is_post)
	{
		char tmp[64];
		int n;

		wtk_strbuf_push_s(buf,"POST ");
		wtk_strbuf_push(buf,req->url->data,req->url->len);
		if(use_1_1)
		{
			wtk_strbuf_push_s(buf," HTTP/1.1\r\n");
		}else
		{
			wtk_strbuf_push_s(buf," HTTP/1.0\r\n");
		}
		wtk_strbuf_push_s(buf,"Content-Length: ");
		n=sprintf(tmp,"%d",param_bytes);
		wtk_strbuf_push(buf,tmp,n);
		wtk_strbuf_push_s(buf,"\r\n");
		//wtk_debug("add hdr=%p\n",req->add_hdr_f);
		if(req->add_hdr_f)
		{
			req->add_hdr_f(req->add_hdr_ths,buf);
			//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		}else
		{
			wtk_strbuf_push_s(buf,"Content-Type: application/x-www-form-urlencoded\r\n");
		}
	}else
	{
		wtk_strbuf_push_s(buf,"GET ");
		wtk_strbuf_push(buf,req->url->data,req->url->len);
		if(param_bytes>0)
		{
			wtk_strbuf_push_s(buf,"?");
			wtk_strbuf_push(buf,param,param_bytes);
		}
		if(use_1_1)
		{
			wtk_strbuf_push_s(buf," HTTP/1.1\r\nContent-Length: 0\r\n");
		}else
		{
			//wtk_strbuf_push_s(buf," HTTP/1.0\r\n");
			wtk_strbuf_push_s(buf," HTTP/1.0\r\nContent-Length: 0\r\n");
		}
		if(req->cfg->ip.len>0)
		{
			wtk_strbuf_push_s(buf,"Host:");
			wtk_strbuf_push(buf,req->cfg->ip.data,req->cfg->ip.len);
			wtk_strbuf_push_s(buf,"\r\n");
		}
		if(req->add_hdr_f)
		{
			req->add_hdr_f(req->add_hdr_ths,buf);
		}
	}
	if(req->cfg && req->cfg->cookie.cookie && req->cfg->cookie.cookie->pos>0)
	{
		wtk_strbuf_push_s(buf,"Cookie: ");
		wtk_strbuf_push(buf,req->cfg->cookie.cookie->data,req->cfg->cookie.cookie->pos);
		wtk_strbuf_push_s(buf,"\r\n");
	}
	if(client)
	{
		wtk_strbuf_push_s(buf,"Client: ");
		wtk_strbuf_push(buf,client->data,client->len);
		wtk_strbuf_push_s(buf,"\r\n");
	}
	wtk_strbuf_push_s(buf,"\r\n");
	//wtk_debug("%.*s\n",buf->pos,buf->data);
	//exit(0);
	if(is_post && param_bytes>0)
	{
		wtk_strbuf_push(buf,param,param_bytes);
	}
	writed=0;
#ifdef DEBUG_HTTPC
	print_data(buf->data,buf->pos);
	//exit(0);
#endif
	ret=wtk_fd_send(req->fd,buf->data,buf->pos,&writed);
	if(ret!=0)
	{
		//if socket is disconnected.
		ret=wtk_httpc_request_reconnect(req);
		if(ret==0)
		{
			ret=wtk_fd_send(req->fd,buf->data,buf->pos,&writed);
		}
	}
	if(ret!=0 && writed!=buf->pos)
	{
		//perror(__FUNCTION__);
		//wtk_debug("ret=%d\n",ret);
		wtk_debug("send data failed ret=%d\n",ret);
		ret=-1;
		goto end;
	}
	last_pos=0;
	while(*run)
	{
		t=time_get_ms();
		s=wtk_fd_recv_blk(req->fd,buf->data,buf->length,&readed);
		if(s==WTK_AGAIN && readed==0)
		{
			//wtk_debug("continue...\n");
			continue;
		}
		//print_data(buf->data,readed);
		if(s!=WTK_OK)
		{
			t=time_get_ms()-t;
			perror(__FUNCTION__);
			wtk_debug("============================> recv failed readed=%d time=%f.\n",readed,t);
			//wtk_debug("is blocking=%d\n",wtk_fd_is_block(req->fd));
			//exit(0);
//			print_data(buf->data,readed);
//			exit(0);
			ret=-1;
			break;
		}
		//wtk_debug("readed=%d\n",readed);
		if(readed==0)
		{
			ret=rep->unknown_content_length?0:-1;
			wtk_debug("recv failed s=%d.\n",s);
#ifdef DEBUG_ERR
			wtk_debug("url=[%s:%s %.*s]\n",req->ip,req->port,req->url->len,req->url->data);
			print_data(param,param_bytes);
			perror(__FUNCTION__);
			wtk_debug("ret=%d,unknwo=%d,state=%d,%d,%d\n",ret,rep->unknown_content_length,rep->state,RESPONSE_CONTENT_DONE,rep->content_length);
#endif
			break;
		}
		buf->pos=readed;
		//print_data(buf->data,buf->pos);
		//wtk_debug("[%.*s]\n",buf->pos,buf->data);
		ret=wtk_http_response_feed(rep,buf->data,buf->pos,&left);
		//wtk_debug("ret=%d\n",ret);
		if((ret!=0) || (left!=0))
		{
			//wtk_debug("recv failed.\n");
			ret=-1;
			break;
		}
		if(rep->body->pos>last_pos)
		{
			if(req->body_notify_f)
			{
				req->body_notify_f(req->body_notify_ths,rep->body->data+last_pos,rep->body->pos-last_pos);
			}
			last_pos=rep->body->pos;
		}
		//wtk_debug("pos=%d\n",rep->body->pos);
		//wtk_debug("state=%d,%d,%d\n",rep->state,RESPONSE_CONTENT_DONE,rep->content_length);
		if(rep->state==RESPONSE_CONTENT_DONE){ret=0;break;}
		if(readed<=0)
		{
			//wtk_debug("recv failed.\n");
			ret=-1;break;
		}
	}
end:
	if(ret!=0 || !use_1_1)
	{
		close=1;
	}else
	{
		v=wtk_str_hash_find_s(rep->hash,"connection");
		if(v && (wtk_string_cmp_s(v,"close")==0))
		{
			close=1;
		}else
		{
			close=0;
		}
	}
	if(req->cfg && req->cfg->cookie.update_cookie)
	{
		v=wtk_str_hash_find_s(rep->hash,"set-cookie");
		if(v)
		{
			wtk_cookie_cfg_update_cookie(&(req->cfg->cookie),v->data,v->len);
		}
	}
	//wtk_http_response_print(rep);
#ifdef DEBUG_HTTPC
	wtk_http_response_print(rep);
#endif
	//wtk_debug("ret=%d\n",ret);
#ifdef DEBUG_ERR
	wtk_http_response_print(rep);
	wtk_debug("=======================================\n");
#endif
	if(close)
	{
#ifdef DEBUG_ERR
		wtk_debug("================== test round url=[%s:%s %.*s]\n",req->ip,req->port,req->url->len,req->url->data);
		wtk_debug("close request ret=%d....\n",ret);
		//exit(0);
		wtk_http_response_print(rep);
#endif
		wtk_httpc_request_close(req);
	}
	//wtk_debug("body-=%d\n",req->response->body->pos);
	return ret;
}

