#ifndef WTK_OS_TCP_WTK_TCP_LISTEN
#define WTK_OS_TCP_WTK_TCP_LISTEN
#include "wtk/core/wtk_type.h" 
#include "wtk_tcp_listen_cfg.h"
#ifdef WIN32
#include <WinSock2.h>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_tcp_listen wtk_tcp_listen_t;

typedef void (*wtk_tcp_listen_notify_f)(void *ths,int fd);

struct wtk_tcp_listen
{
	wtk_tcp_listen_cfg_t *cfg;
	struct sockaddr_in addr;
	int fd;
	wtk_tcp_listen_notify_f notify;
	void *notify_ths;
	unsigned run:1;
};

wtk_tcp_listen_t* wtk_tcp_listen_new(wtk_tcp_listen_cfg_t *cfg);
void wtk_tcp_listen_delete(wtk_tcp_listen_t *l);
void wtk_tcp_listen_set_notify(wtk_tcp_listen_t *l,void *ths,wtk_tcp_listen_notify_f notify);
int wtk_tcp_listen_run(wtk_tcp_listen_t *l);
#ifdef __cplusplus
};
#endif
#endif
