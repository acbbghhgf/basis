#ifdef WIN32
#else
#include <netdb.h>
#endif
#include "wtk_socket.h"
#include "wtk_fd.h"
#include "wtk_thread.h"

struct addrinfo* wtk_socket_get_addrinfo(char* server,char* port)
{
    struct addrinfo hints,*paddr;

    paddr=0;
    memset(&hints,0,sizeof(hints));
    hints.ai_flags = AI_CANONNAME;
    hints.ai_family = AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    getaddrinfo(server,port,&hints,&paddr);
    return paddr;
}

int wtk_socket_free_addr(struct addrinfo* info)
{
	freeaddrinfo(info);
	return 0;
}

wtk_addrinfo_t* wtk_addrinfo_get(char* server,char *port)
{
	wtk_addrinfo_t *wa=0;
	struct addrinfo* i=0;

	i=wtk_socket_get_addrinfo(server,port);
	if(!i){goto end;}
	wa=(wtk_addrinfo_t*)wtk_malloc(sizeof(*wa));
	wa->addrlen=i->ai_addrlen;
	wa->addr=(struct sockaddr *)wtk_malloc(i->ai_addrlen);
	memcpy(wa->addr,i->ai_addr,i->ai_addrlen);
end:
	if(i){wtk_socket_free_addr(i);}
	return wa;
}

#ifdef WIN32
#else
#include <errno.h>

wtk_addrinfo_t* wtk_addrinfo_get2(char *server,char *port)
{
	wtk_addrinfo_t *info=0;
	struct sockaddr_in *addr;
	struct in_addr *tx;

#ifdef __ANDROID__
	 struct hostent *ent;
 	 ent=gethostbyname(server);
 	 if(!ent)
 	 {
 		 perror(__FUNCTION__);
 		 wtk_debug("xxxxget host by name[%s:%s] not found ent=%p %s\n",server,port,ent,strerror(errno));
 		 goto end;
 	 }
 	 tx=(struct in_addr*)(ent->h_addr_list[0]);
#else
 	struct hostent host_buf;
 	struct hostent *result;
	char buf[4096];
	int err;
	int ret;

	result=0;
	ret=gethostbyname_r(server,&host_buf,buf,sizeof(buf),&result,&err);
	if(ret!=0 || !result || result->h_length<=0)
	{
		wtk_debug("get host by name[%s:%s] not found\n",server,port);
		goto end;
	}
	tx=(struct in_addr*)(result->h_addr_list[0]);
#endif
	info=(wtk_addrinfo_t*)wtk_malloc(sizeof(wtk_addrinfo_t));
	info->addrlen=sizeof(*addr);
	addr=(struct sockaddr_in*)wtk_calloc(1,info->addrlen);
	info->addr=(struct sockaddr *)addr;
	addr->sin_addr=*(tx);
	addr->sin_port=htons(atoi(port));
	addr->sin_family=AF_INET;
end:
	return info;
}

typedef struct
{
	wtk_thread_t thread;
	wtk_sem_t sem;
	char *server;
	char *port;
	wtk_addrinfo_t *addrinfo;
	unsigned end:1;
}wtk_socket_route_t;

int wtk_socket_route_run(wtk_socket_route_t *route,wtk_thread_t *t)
{
	wtk_sem_t *wait_sem=&(route->sem);

	pthread_detach(t->handler);
	route->addrinfo=wtk_addrinfo_get2(route->server,route->port);
	if(route->end)
	{
		wtk_debug("=================================> end = %d.\n",route->end);
		//wtk_thread_clean(t);
		if(route->addrinfo){
			wtk_addrinfo_delete(route->addrinfo);
		}
		wtk_sem_clean(wait_sem);
		//wtk_free(route);
	}else
	{
		wtk_sem_release(wait_sem,1);
	}
	return 0;
}

wtk_addrinfo_t* wtk_addrinfo_get3(char *server,char *port,int timeout)
{
	wtk_socket_route_t *route;
	wtk_sem_t *wait_sem;
	wtk_addrinfo_t *info=0;
	int ret=-1;

	//wtk_debug("=======================> get addrinfo3.\n");
	route=wtk_malloc(sizeof(wtk_socket_route_t));
	route->server=server;
	route->port=port;
	route->addrinfo=0;
	route->end=0;
	wait_sem=&(route->sem);
	wtk_sem_init(wait_sem,0);
	wtk_thread_init(&(route->thread),(thread_route_handler)wtk_socket_route_run,route);
	wtk_thread_set_name(&(route->thread),"addrinfo2_get");
	wtk_thread_start(&(route->thread));
	//wtk_debug("=======================> timeout = %d.\n",timeout);
	ret=wtk_sem_acquire(wait_sem,timeout);
	//wtk_debug("=======================> ret = %d.\n",ret)
	if(ret==0)
	{
		//wtk_thread_clean(&(route->thread));
		//wtk_thread_join(&(route->thread));
		info=route->addrinfo;
		wtk_sem_clean(wait_sem);
		wtk_free(route);
	}else
	{
		//wtk_debug("=======================> ret = %d.\n",ret)
		route->end=1;
	}
	//wtk_debug("=======================>addrinfo = %p.\n",info);
	return info;
}
#endif

int wtk_addrinfo_delete(wtk_addrinfo_t *i)
{
	wtk_free(i->addr);
	wtk_free(i);
	return 0;
}

#ifdef WIN32
int wtk_socketpair(int socks[2], int make_overlapped)
{
    union {
        struct sockaddr_in inaddr;
        struct sockaddr addr;
    } a;
    SOCKET listener;
    int e;
    socklen_t addrlen = sizeof(a.inaddr);
    DWORD flags = (make_overlapped ? WSA_FLAG_OVERLAPPED : 0);
    int reuse = 1;

    if (socks == 0) {
        WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
    }

    listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == INVALID_SOCKET)
        return SOCKET_ERROR;

    memset(&a, 0, sizeof(a));
    a.inaddr.sin_family = AF_INET;
    a.inaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    a.inaddr.sin_port = 0;

    socks[0] = socks[1] = INVALID_SOCKET;
    do {
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
            (char*) &reuse, (socklen_t) sizeof(reuse)) == -1)
            break;
        if  (bind(listener, &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        if  (getsockname(listener, &a.addr, &addrlen) == SOCKET_ERROR)
            break;
        if (listen(listener, 1) == SOCKET_ERROR)
            break;
        socks[0] = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, flags);
        if (socks[0] == INVALID_SOCKET)
            break;
        if (connect(socks[0], &a.addr, sizeof(a.inaddr)) == SOCKET_ERROR)
            break;
        socks[1] = accept(listener, NULL, NULL);
        if (socks[1] == INVALID_SOCKET)
            break;

        closesocket(listener);
        return 0;

    } while (0);

    e = WSAGetLastError();
    closesocket(listener);
    closesocket(socks[0]);
    closesocket(socks[1]);
    WSASetLastError(e);
    return SOCKET_ERROR;
}

#endif

#ifndef WIN32
#include <net/if.h>
#include <sys/ioctl.h>

char* wtk_get_host_ip(char *iface)
{
	struct in_addr addr;
	char *name=0;
	char buf[256];
	const char *p;
	int ret;

	ret=wtk_get_host_addr(iface,&addr);
	if(ret!=0){goto end;}
	p=inet_ntop(AF_INET,&(addr),buf,sizeof(buf));
	if(!p){goto end;}
	name=strdup(p);
end:
	return name;
}

int  wtk_get_host_addr(char *iface,struct in_addr *addr)
{
	struct ifconf ic;
	struct ifreq *req;
	char tmp[4096];
	int fd,ret;
	int i,n;

	ret=-1;
	ic.ifc_len=sizeof(tmp);
	ic.ifc_ifcu.ifcu_buf=tmp;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	ret=ioctl(fd,SIOCGIFCONF,&ic);
	if(ret!=0){goto end;}
	n=ic.ifc_len/sizeof(struct ifreq);
	req=(struct ifreq*)tmp;
	for(i=0;i<n;++i)
	{
		if(strcmp(req[i].ifr_ifrn.ifrn_name,iface)==0)
		{
			*addr=((struct sockaddr_in*)&(req[i].ifr_ifru.ifru_addr))->sin_addr;
			ret=0;
			break;
		}
	}
end:
	return ret;
}
#endif

int wtk_socket_connect(struct sockaddr *ai_addr,socklen_t ai_addrlen,int *pfd)
{
	return wtk_socket_connect3(ai_addr,ai_addrlen,pfd,-1);
}

void wtk_socket_set_timeout(int fd,int timeout)
{
	struct timeval tv;

	if(timeout>0)
	{
		tv.tv_sec=timeout/1000;
		tv.tv_usec=(timeout%1000)*1e3;
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
		setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
	}
}

int wtk_socket_connect3(struct sockaddr *ai_addr,socklen_t ai_addrlen,int *pfd,int timeout)
{
	int fd,ret;
	struct timeval tv;

	ret=-1;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	if(timeout>0)
	{
		tv.tv_sec=timeout/1000;
		tv.tv_usec=(timeout%1000)*1e3;
		setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(tv));
		setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
	}
	ret=connect(fd,ai_addr,ai_addrlen);
	if(ret!=0){goto end;}
	ret=wtk_fd_set_tcp_client_opt(fd);
	if(ret!=0){goto end;}
	*pfd=fd;
end:
	if(ret!=0)
	{
		if(fd>0)
		{
#ifdef WIN32
			closesocket(fd);
#else
			close(fd);
#endif
			fd=0;
		}
		*pfd=INVALID_FD;
	}
	return ret;
}

int wtk_socket_get_addr(char *ip,int port,struct sockaddr_in *addr)
{
	int ret;

	addr->sin_family=AF_INET;
	addr->sin_port=htons(port);
#ifdef WIN32
	ret = inet_addr(ip);
	if (ret == -1 && strcmp(ip, "255.255.255.255"))
	{
		goto end;
	}
	addr->sin_addr.S_un.S_addr = ret;
#else
	ret=inet_aton(ip,&(addr->sin_addr));
	ret=ret!=0?0:-1;
#endif
	//wtk_debug("ip=%s\n",ip);
	//print_hex((char*)&(addr->sin_addr),sizeof(addr->sin_addr));
	//print_hex((char*)&(addr->sin_port),sizeof(addr->sin_port));
#ifdef WIN32
end:
#endif
	return ret;
}

int wtk_socket_connect2(char *ip,int port,int *pfd)
{
	struct sockaddr_in addr={0};
	int ret;

	ret=wtk_socket_get_addr(ip,port,&addr);
	if(ret!=0)
	{
		wtk_debug("get addrress failed\n");
		goto end;
	}
	ret=wtk_socket_connect((struct sockaddr*)&(addr),sizeof(addr),pfd);
end:
	return ret;
}

int wtk_socket_connect4(char *ip,char *port,int *pfd)
{
	wtk_addrinfo_t* info;
	int ret=-1;

	info=wtk_addrinfo_get(ip,port);
	if(!info)
	{
		wtk_debug("get address failed\n");
		goto end;
	}
	ret=wtk_socket_connect(info->addr,info->addrlen,pfd);
end:
	if(info)
	{
		wtk_addrinfo_delete(info);
	}
	return ret;
}

int wtk_socket_close_fd(int fd)
{
	int ret;

#ifdef WIN32
	ret=closesocket(fd);
#else
	ret=close(fd);
#endif
	return ret;
}

int wtk_socket_get_port(int fd,int *port)
{
	struct sockaddr_in addr;
	socklen_t len;
	int ret;

	len=sizeof(addr);
	ret=getsockname(fd,(struct sockaddr*)&addr,&len);
	if(ret!=0){goto end;}
	*port=ntohs(addr.sin_port);
end:
	return ret;
}

void wtk_socket_print(int fd)
{
#ifndef WIN32
	char buf[256];
	struct sockaddr_in addr;
	socklen_t len;
	int ret;
	int port;
	const char *p;

	len=sizeof(addr);
	ret=getsockname(fd,(struct sockaddr*)&addr,&len);
	if(ret!=0){goto end;}
	port=ntohs(addr.sin_port);
	p=inet_ntop(AF_INET,(struct sockaddr*)&(addr.sin_addr),buf,sizeof(buf)-1);
	wtk_debug("local=[%s:%d]\n",p,port);

	ret=getpeername(fd,(struct sockaddr*)&addr,&len);
	//wtk_debug("ret=%d\n",ret);
	if(ret!=0){goto end;}
	port=ntohs(addr.sin_port);
	p=inet_ntop(AF_INET,(struct sockaddr*)&(addr.sin_addr),buf,sizeof(buf)-1);
	wtk_debug("peer=[%s:%d]\n",p,port);
end:
#endif
	return;
}

int wtk_socket_set_reuse(int fd)
{
	int reuse=1;
	int ret;

	ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));
	if(ret!=0)
	{
		perror(__FUNCTION__);
	}
	return ret;
}

int wtk_socket_create_udp_fd(int port)
{
	struct sockaddr_in addr={0};
	int ret=-1;
	int fd=-1;

	fd=socket(AF_INET,SOCK_DGRAM,0);
	if(fd<0){goto end;}
	addr.sin_family=AF_INET;
	if(port<0){port=0;}
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
end:
	if(ret!=0)
	{
		perror(__FUNCTION__);
		if(fd>0)
		{
			close(fd);
			fd=-1;
		}
	}
	return fd;
}

int wtk_socket_create_tcp_listen_fd(int port)
{
	struct sockaddr_in addr={0};
	int fd;
	int ret=0;

	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){goto end;}
	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	ret=bind(fd,(struct sockaddr*)&addr,sizeof(addr));
	if(ret!=0){goto end;}
	ret=listen(fd,5);
end:
	if(ret!=0)
	{
		wtk_socket_close_fd(fd);
		fd=-1;
	}
	return fd;
}


#ifdef WIN32
#else
int wtk_socket_sendto_host(int fd,const char *data,int bytes,char *host,int port)
{
	struct sockaddr_in addr={0};
	int ret;

	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
#ifdef WIN32
	ret=inet_aton(host,&(addr.sin_addr));
#else
	ret=inet_pton(AF_INET,host,(void *)&(addr.sin_addr));
#endif
	wtk_debug("ret=%d\n",ret);
	if(ret!=1){ret=-1;goto end;}
	ret=wtk_socket_sendto(fd,data,bytes,(struct sockaddr*)&addr,sizeof(addr));
end:
	return ret;
}

int wtk_socket_sendto(int fd,const char *data,int bytes,const struct sockaddr* addr,socklen_t len)
{
	int ret;

	wtk_debug("[\n%.*s\n]\n",bytes,data);
	ret=sendto(fd,data,bytes,0,addr,len);
	if(ret==bytes)
	{
		ret=0;
	}else
	{
		perror(__FUNCTION__);
		ret=-1;
	}
	return ret;
}

int wtk_socket_recvfrom(int fd,char *buf,int len,struct sockaddr *addr,socklen_t *addrlen)
{
	int ret;

	ret=recvfrom(fd,buf,len,0,addr,addrlen);
	wtk_debug("[\n%.*s\n]\n",ret,buf);
	return ret;
}
#endif

int wtk_socket_readable(int fd)
{
	fd_set r_set;
	struct timeval timeout;
	int ret;

	timeout.tv_sec=0;
	timeout.tv_usec=0;
	FD_ZERO(&(r_set));
	FD_SET(fd,&(r_set));
	ret=select(fd+1,&(r_set),0,0,&timeout);
	return ret>0?1:0;
}


int wtk_socket_send_strmsg(int fd,char *data,int bytes)
{
	wtk_fd_state_t s;
	int w;
	int ret=-1;

	s=wtk_fd_send(fd,(char*)&(bytes),4,&w);
	if(s!=WTK_OK || w!=4){goto end;}
	s=wtk_fd_send(fd,data,bytes,&w);
	if(s!=WTK_OK || w!=bytes){goto end;}
	ret=0;
end:
	return ret;
}

int wtk_socket_read_strmsg(int fd,wtk_strbuf_t *buf)
{
	wtk_fd_state_t s;
	int v;
	int r;
	int ret=-1;

	wtk_strbuf_reset(buf);
	s=wtk_fd_recv(fd,(char*)&v,4,&r);
	if(s!=WTK_OK || r!=4){goto end;}
	//wtk_debug("v=%d\n",v);
	if(v>10420 || v<=0){goto end;}
	wtk_strbuf_expand(buf,v);
	s=wtk_fd_recv(fd,buf->data,v,&r);
	if(s!=WTK_OK || r!=v){goto end;}
	buf->pos=v;
	ret=0;
end:
	return ret;
}
