#ifndef WTK_NK_POLL_WTK_EPOLL
#define WTK_NK_POLL_WTK_EPOLL
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include "wtk/core/wtk_type.h" 
#include "wtk_epoll_cfg.h"
#include "wtk_poll_evt.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_epoll wtk_epoll_t;

typedef void(*wtk_epoll_process_evt_f)(wtk_poll_evt_t *evt);

struct wtk_epoll
{
	wtk_epoll_cfg_t *cfg;
	struct epoll_event* events;
	int fd;
};

wtk_epoll_t* wtk_epoll_new(wtk_epoll_cfg_t *cfg);
void wtk_epoll_delete(wtk_epoll_t *poll);
int wtk_epoll_start(wtk_epoll_t *poll);
void wtk_epoll_stop(wtk_epoll_t *poll);
int wtk_epoll_add_evt(wtk_epoll_t *epoll,int fd,wtk_poll_evt_t *evt);
int wtk_epoll_remove_evt(wtk_epoll_t *epoll,int fd,wtk_poll_evt_t *evt);
int wtk_epoll_process(wtk_epoll_t *epoll,int timeout,wtk_epoll_process_evt_f process_evt);
#ifdef __cplusplus
};
#endif
#endif
