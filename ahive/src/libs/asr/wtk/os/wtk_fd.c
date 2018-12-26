#include "wtk_fd.h"
#include <errno.h>
#ifdef WIN32
#include <winsock2.h>
#pragma  comment(lib,"Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#endif

int fd_read(int fd, char* data, int len, int* readed)
{
	int ret = 0, index = 0, nLeft = len, count;
	int step;

	if (!data)
	{
		return -1;
	}
	while (nLeft > 0)
	{
		step=min(4096,nLeft);
		count = read(fd, &data[index], step);
		if (count < step)
		{
			if ((count<=0) && (errno != EINTR ))
			{
				ret = -1;
				break;
			}
		}
		index += count;
		nLeft -= count;
	}
	if (readed)
	{
		*readed = index;
	}
	return ret;
}

int fd_write(int fd, const char* data, int len, int* writed)
{
	int ret = 0, index = 0, nLeft = len;
	int step;

	if (!data)
	{
		return -1;
	}
	while (nLeft > 0)
	{
		step=min(4096,nLeft);
		//step=nLeft;
		ret = write(fd, &data[index],step);
		//wtk_debug("pid=%d,ret=%d,step=%d,nLeft=%d,len=%d\n",getpid(),ret,step,nLeft,len);
		if (ret < step)
		{
			if ((ret<=0) && (errno != EINTR))
			{
				ret = -1;
				break;
			}
		}
		index += ret;
		nLeft -= ret;
	}
	if (writed)
	{
		*writed = index;
	}
	return ret;
}

int wtk_fd_read(int fd,char* buf,int len)
{
	int readed;

	readed=0;
	fd_read(fd,buf,len,&readed);
	return len==readed ? 0 :-1;
}

int wtk_fd_write(int fd,char* buf,int len)
{
	int writed=0;

	fd_write(fd,buf,len,&writed);
	return len==writed ? 0 : -1;
}

wtk_string_t* wtk_fd_read_string(int fd)
{
	int32_t read_length;
	wtk_string_t* s;
	int ret;

	s=0;
	read_length=0;
	ret=wtk_fd_read(fd,(char*)&read_length,sizeof(read_length));
	if(ret!=0 || read_length<=0)
	{
		perror(__FUNCTION__);
#ifndef WIN32
		wtk_debug("read length failed(proc=%d,ret=%d,read_length=%d).\n",getpid(),ret,read_length);
#endif
		goto end;
	}
	s=wtk_string_new(read_length);
	ret=wtk_fd_read(fd,s->data,read_length);
	if(ret!=0)
	{
#ifndef WIN32
		wtk_debug("read string failed (%d,%d-%d).\n",ret,read_length,getpid());
#endif
		wtk_string_delete(s);
		s=0;
	}
end:
	return s;
}

int wtk_fd_write_string(int fd,char* buf,int len)
{
	int32_t length;
	int ret;

	length=len;
	ret=wtk_fd_write(fd,(char*)&length,sizeof(length));
	if(ret!=0){goto end;}
	ret=wtk_fd_write(fd,buf,len);
end:
	return ret;
}

int wtk_fd_write_stack(int fd,wtk_stack_t *s)
{
	stack_block_t *b;
	int ret,count,t;
	int32_t length;

	length=s->len;
	ret=wtk_fd_write(fd,(char*)&length,sizeof(length));
	if(ret!=0){goto end;}
	count=0;
	for(b=s->pop;b;b=b->next)
	{
		t=b->push-b->pop;
		ret=wtk_fd_write(fd,b->pop,t);
		if(ret!=0){goto end;}
		count+=t;
	}
	ret=count==length?0:-1;
end:
	return ret;
}

#ifdef WIN32
wtk_fd_state_t wtk_fd_recv(int fd, char* buf, int len, int* readed)
{
    wtk_fd_state_t s;
    int ret;
    int no;

    ret = recv(fd, buf, len, 0);
    //print_data(buf,ret);
    if (ret >= 0)
    {
        if (readed){*readed = ret;}
        s=(ret>0) ? WTK_OK : ((errno == EAGAIN || errno==0) ? WTK_OK : WTK_EOF);
    }
    else
    {
        if (readed){*readed = 0;}
#ifdef WIN32
        no=WSAGetLastError();
        s=(no==WSAEWOULDBLOCK || no==0) ? WTK_AGAIN : WTK_ERR;
#else
        s = (errno == EAGAIN || errno == EINTR || errno==0) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
#endif
    }
    return s;
}

wtk_fd_state_t wtk_fd_send(int fd, char* buf, int len, int* writed)
{
    wtk_fd_state_t s=WTK_OK;
    int nLeft = len;
    int idx = 0;
    int ret;
    int err;

    //print_data(buf,len);
    //wtk_debug("send %d\n",len);
    while (nLeft > 0)
    {
        ret = send(fd, &(buf[idx]), nLeft, 0);
        if (ret <= 0)
        {
            //s = (errno == EAGAIN || errno == EINTR || errno==0) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
            err=WSAGetLastError();
            s= (err==WSAEWOULDBLOCK) ? WTK_AGAIN :  ((err==WSAESHUTDOWN) ? WTK_EOF : WTK_ERR);
            //wtk_debug("fd=%d,left=%d,again=%d,err=%d\n",fd,nLeft,s==WTK_AGAIN,err);
            break;
        }
        nLeft -= ret;
        idx += ret;
    }
    if(nLeft==0){s=WTK_OK;}
    if (writed)
    {
        *writed = idx;
    }
    return s;
}
#else

//#define DEBUG_NK

wtk_fd_state_t wtk_fd_recv2(int fd, char* buf, int len)
{
	wtk_fd_state_t state;
	char *s,*e;
	int read;

	s=buf;e=s+len;
	while(s<e)
	{
		state=wtk_fd_recv(fd,s,e-s,&read);
		if(state!=WTK_OK || read<=0){break;}
		s+=read;
	}
	if(s<e)
	{
		state=WTK_ERR;
	}
	return state;
}

wtk_fd_state_t wtk_fd_recv(int fd, char* buf, int len, int* readed)
{
	wtk_fd_state_t s;
	int ret;

	ret = recv(fd, buf, len, 0);
	//print_data(buf,ret);
	//wtk_debug("errno=%d\n",errno);
#ifdef DEBUG_NK
	print_data(buf,ret);
#endif
	if (ret >= 0)
	{
#ifdef USE_TCP_QUICKACK
        int flag = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(int));
#endif
		if (readed){*readed = ret;}
		s=(ret>0) ? WTK_OK : ( (errno == EAGAIN || errno == 0) ? WTK_OK : WTK_EOF);
		//s=(ret>0) ? WTK_OK : ( (errno == EAGAIN) ? WTK_OK : WTK_EOF);
	}
	else
	{
		if (readed){*readed = 0;}
		s = (errno == EAGAIN || errno == EINTR) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
	}
	return s;
}

wtk_fd_state_t wtk_fd_recv_blk(int fd, char* buf, int len, int* readed)
{
	wtk_fd_state_t s;
	int ret;

	ret = recv(fd, buf, len, 0);
	//print_data(buf,ret);
	//wtk_debug("errno=%d\n",errno);
#ifdef DEBUG_NK
	print_data(buf,ret);
#endif
	if (ret >= 0)
	{
#ifdef USE_TCP_QUICKACK
        int flag = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &flag, sizeof(int));
#endif
		if (readed){*readed = ret;}
		//change by pfc123
		if(ret==0)
		{
			wtk_debug("errno=%d %s ret=%d\n",errno,strerror(errno),ret);
		}
		s=(ret>0) ? WTK_OK : ( (errno == EAGAIN || errno == 0 || errno==115) ? WTK_OK : WTK_EOF);
		//s=(ret>0) ? WTK_OK : WTK_EOF;
	}
	else
	{
		if (readed){*readed = 0;}
		//wtk_debug("errno=%d %s\n",errno,strerror(errno));
		s = (errno == EINTR || errno==115 || errno == EAGAIN) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
	}
	return s;
}

wtk_fd_state_t wtk_fd_send(int fd, char* buf, int len, int* writed)
{
	wtk_fd_state_t s=WTK_OK;
	int nLeft = len;
	int idx = 0;
	int ret;

	//print_data(buf,len);
	//wtk_debug("%*.*s\n",len,len,buf);
#ifdef DEBUG_NK
	print_data(buf,len);
#endif
	while (nLeft > 0)
	{
		ret = send(fd, &(buf[idx]), nLeft, 0);
		//wtk_debug("ret=%d/%d\n",ret,nLeft);
		if (ret <= 0)
		{
			//wtk_debug("%d,%d\n",ret,errno);
			s = (errno == EAGAIN || errno == EINTR ||errno==0) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
			break;
		}
//		if(ret>0)
//		{
//			static char hdr[4]={0,};
//			static int hdr_pos=0;
//			static int len=0;
//			static int is_hdr=1;
//			static int pos=0;
//			char *s,*e;
//			int i;
//
//			if(idx==0)
//			{
//				wtk_debug("================= is_hdr=%d pos=%d len=%d\n",is_hdr,pos,len);
//			}
//			s=buf+idx;
//			e=s+ret;
//			while(s<e)
//			{
//				if(is_hdr)
//				{
//					hdr[hdr_pos++]=*s;
//					++s;
//					if(hdr_pos==4)
//					{
//						len=*(int*)(hdr);
//						wtk_debug("=======================> len=%d\n",len);
//						if(len<0)
//						{
//							exit(0);
//						}
//						is_hdr=0;
//						pos=0;
//					}
//				}else
//				{
//					i=min(e-s,len-pos);
//					pos+=i;
//					if(pos==len)
//					{
//						is_hdr=1;
//						hdr_pos=0;
//					}
//					s+=i;
//				}
//			}
//
//		}
//		if(ret>0)
//		{
//			print_hex(buf+idx,ret);
//		}
		nLeft -= ret;
		idx += ret;
	}
	if(nLeft==0){s=WTK_OK;}
	if (writed)
	{
		*writed = idx;
	}
	//if(s==WTK_EOF){wtk_debug("state=%d,errno=%d\n",s,errno);}
	return s;
}
#endif

wtk_fd_state_t wtk_fd_write_nonblock(int fd,const char *data,int bytes,int *writed)
{
	wtk_fd_state_t s;
	int ret;

	//wtk_debug("%d: [%.*s]\n",getpid(),bytes,data);
	ret=write(fd,data,bytes);
	//wtk_debug("%d: fd=%d,write=%d,errno=%d\n",getpid(),fd,ret,errno);
	if(ret<0)
	{
		//perror(__FUNCTION__);
		//wtk_debug("errno=%d\n",errno);
		s = (errno == EAGAIN || errno == EINTR ||errno==0) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
	}else if(ret==0)
	{
		s=WTK_EOF;
	}else
	{
		s=WTK_OK;
	}
	if(writed)
	{
		*writed=ret>0?ret:0;
	}
	//wtk_debug("s=%d\n",s);
	return s;
}

wtk_fd_state_t wtk_fd_read_nonblock(int fd,char *data,int bytes,int *readed)
{
	wtk_fd_state_t s;
	int ret;

	ret=read(fd,data,bytes);
	//wtk_debug("%d: fd=%d,ret=%d,errno=%d\n",getpid(),fd,ret,errno);
	if(ret<0)
	{
		s = (errno == EAGAIN || errno == EINTR ||errno==0) ? WTK_AGAIN : ((errno==EPIPE) ? WTK_EOF : WTK_ERR);
	}else if(ret==0)
	{
		s=WTK_EOF;
	}else
	{
		//wtk_debug("%d: [%.*s]\n",getpid(),bytes,data);
		s=WTK_OK;
	}
	if(readed)
	{
		*readed=ret>0?ret:0;
	}
	return s;
}


wtk_fd_state_t wtk_fd_flush_send_stack(int fd,wtk_stack_t *s)
{
	wtk_fd_state_t t;
	stack_block_t *b;
	int count,writed;
    int len,ret;

	t=WTK_OK;
	for(b=s->pop,count=0;b;b=b->next)
	{
        len=b->push - b->pop;
        if(len<=0){continue;}
		t=wtk_fd_send(fd,(char*)b->pop,len,&writed);
		count+=writed;
		if(t!=WTK_OK){break;}
	}
	ret=wtk_stack_pop(s,0,count);
	if(ret!=0)
	{
		t=WTK_ERR;
	}
	return t;
}


#ifdef WIN32

int wtk_fd_set_nonblock(int fd)
{
    u_long iMode = 1;

    return  ioctlsocket(fd,FIONBIO,&iMode);
}
#else
int wtk_fd_set_nonblock(int fd)
{
    int opts,ret;

    ret=-1;
    opts = fcntl(fd, F_GETFL);
    if (opts == -1){goto end;}
    opts = opts | O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, opts);
end:
    return ret;
}

int wtk_fd_set_block(int fd)
{
    int opts,ret;

    ret=-1;
    opts = fcntl(fd, F_GETFL);
    if (opts == -1){goto end;}
    opts = opts ^ O_NONBLOCK;
    ret = fcntl(fd, F_SETFL, opts);
end:
    return ret;
}

int wtk_fd_is_block(int fd)
{
    int opts;
    int ret;

    opts = fcntl(fd, F_GETFL);
    ret=opts & O_NONBLOCK;
    return ret?0:1;
}
#endif


int wtk_fd_set_tcp_client_opt(int fd)
{
	int keepalive = 1, keepalive_time = 3, keepalive_intvl = 3,
			keepalive_probes = 2;
	int reuse = 1;
	int ret;

	ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char*) &reuse,
			sizeof(reuse));
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (char*) (&keepalive),
			(socklen_t) sizeof(keepalive));
#ifndef WIN32
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (void*) (&keepalive_time),
			(socklen_t) sizeof(keepalive_time));
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL,
			(void*) (&keepalive_intvl), (socklen_t) sizeof(keepalive_intvl));
	if (ret != 0) {
		goto end;
	}
	ret = setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*) (&keepalive_probes),
			(socklen_t) sizeof(keepalive_probes));

#ifdef USE_TCP_NODELAY
    {
        int tcp_nodelay = 1;
        ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&tcp_nodelay, sizeof(int));
    }
#endif

#endif
end:
	return ret;
}
