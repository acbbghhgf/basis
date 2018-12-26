#ifdef WIN32
#else
#include <netdb.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include "wtk_simplesvr.h" 


wtk_simplesvr_t* wtk_simplesvr_new(int port)
{
	wtk_simplesvr_t *svr;

	svr=(wtk_simplesvr_t*)wtk_malloc(sizeof(wtk_simplesvr_t));
	svr->port=port;
	svr->ths=NULL;
	svr->process=NULL;
	return svr;
}

void wtk_simplesvr_delete(wtk_simplesvr_t *svr)
{
	wtk_free(svr);
}

void wtk_simplesvr_set_process(wtk_simplesvr_t *svr,void *ths,wtk_simplesvr_process_f process)
{
	svr->ths=ths;
	svr->process=process;
}

int wtk_simplesvr_run(wtk_simplesvr_t *svr)
{
	struct sockaddr_in addr;
	socklen_t len;
	int fd=-1;
	int ret=-1;
	int reuse=1;
	int client_id;

	svr->run=1;
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0)
	{
		wtk_debug("create socket failed\n");
		goto end;
	}
	memset(&(addr),0,sizeof(addr));
	wtk_debug("listen at 127.0.0.1:%d\n",svr->port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(svr->port);
	ret=setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,(char*)&reuse,sizeof(reuse));
	if(ret!=0)
	{
		wtk_debug("create socket failed\n");
		goto end;
	}
	ret=bind(fd,(struct sockaddr*)&(addr),sizeof(addr));
	if(ret!=0)
	{
		wtk_debug("bind socket failed\n");
		perror(__FUNCTION__);
		goto end;
	}
	ret=listen(fd,1);
	if(ret!=0)
	{
		wtk_debug("listen socket failed\n");
		goto end;
	}
	while(svr->run)
	{
		len = sizeof(addr);
		wtk_debug("wait ...\n");
		client_id = accept(fd, (struct sockaddr*) &addr, &len);
		wtk_debug("get fd=%d...\n",fd);
		if(fd<0)
		{
			continue;
		}
		if(svr->process)
		{
			svr->process(svr->ths,client_id);
		}
		close(client_id);
	}
	ret=0;
end:
	wtk_debug("ret=%d\n",ret);
	return ret;
}
