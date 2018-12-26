#include "wtk_pipequeue.h"
#ifdef WIN32
#include <io.h>
# include <winsock2.h>
# include <windows.h>
# include <io.h>
#include <ws2tcpip.h>
#include "wtk/os/wtk_socket.h"
#define close closesocket
#else
#include <sys/socket.h>
#include <sys/types.h>
#endif

int wtk_pipequeue_init(wtk_pipequeue_t *q)
{
	int ret;

	memset(q,0,sizeof(wtk_pipequeue_t));
	wtk_lockqueue_init((wtk_lockqueue_t*)q);
#ifdef WIN32
   //ret=_pipe( q->pipe_fd, 0,  0 );
   ret=wtk_socketpair(q->pipe_fd,0);
#else
	ret=pipe(q->pipe_fd);
	//ret=socketpair(AF_INET,SOCK_STREAM,0,q->pipe_fd);
#endif
	if(ret!=0){goto end;}
	ret=wtk_fd_set_nonblock(q->pipe_fd[0]);
end:
	return ret;
}

int wtk_pipequeue_clean(wtk_pipequeue_t *q)
{
	wtk_lockqueue_clean((wtk_lockqueue_t*)q);
#ifdef WIN32
	closesocket(q->pipe_fd[0]);
	closesocket(q->pipe_fd[1]);
#else
	close(q->pipe_fd[0]);
	close(q->pipe_fd[1]);
#endif
	return 0;
}

int wtk_pipequeue_push(wtk_pipequeue_t *q,wtk_queue_node_t *n)
{
    int ret;

	wtk_lockqueue_push((wtk_lockqueue_t *)q,n);
	ret=wtk_pipequeue_touch_write(q);
    return ret? 0 : -1;
}

int wtk_pipequeue_touch_write(wtk_pipequeue_t* q)
{
#ifdef WIN32
    return send(q->pipe_fd[1],"m",1,0);
#else
	return write(q->pipe_fd[1],"m",1);
#endif
}

int wtk_pipequeue_touch_read(wtk_pipequeue_t* q)
{
	char b;

#ifdef WIN32
    return recv(q->pipe_fd[0],&b,1,0);
#else
    return read(q->pipe_fd[0],&b,1);
#endif
}

wtk_queue_node_t* wtk_pipequeue_pop(wtk_pipequeue_t *q)
{
	wtk_queue_node_t* n;
	int ret;

	n=0;
	ret=wtk_pipequeue_touch_read(q);
	if(ret!=1)
    {
        //wtk_debug("%d\n",WSAGetLastError ());
        //perror(__FUNCTION__);
        goto end;
    }
	n=wtk_lockqueue_pop((wtk_lockqueue_t *)q);
end:
	return n;
}
