#ifndef WTK_NK_MODULE_WTK_EPOLL_H_
#define WTK_NK_MODULE_WTK_EPOLL_H_
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_alloc.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/http/nk/wtk_event.h"
#include "wtk/http/nk/wtk_connection.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_epoll wtk_epoll_t;

struct wtk_epoll
{
	int fd;
	int size;
	int montier_events;
	struct epoll_event* events;
	wtk_queue_t accept_queue;
	wtk_queue_t client_queue;
	unsigned et:1;
	unsigned bw_on_sig:1; //break from wait on signal.
};

int wtk_epoll_init(wtk_epoll_t *epoll,int event_num);
int wtk_epoll_bytes(wtk_epoll_t *epoll);
int wtk_epoll_clean(wtk_epoll_t *epoll);
int wtk_epoll_create(wtk_epoll_t *epoll);
int wtk_epoll_close(wtk_epoll_t *epoll);
int wtk_epoll_add(wtk_epoll_t* epoll, int fd, uint32_t events,void *data);
int wtk_epoll_remove(wtk_epoll_t* epoll, int fd);
int wtk_epoll_mod(wtk_epoll_t* epoll, int fd, uint32_t events, void *data);
int wtk_epoll_event_add_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_event_remove_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_add_event(wtk_epoll_t *epoll,int fd,wtk_event_t* event);
int wtk_epoll_add_connection(wtk_epoll_t *epoll,wtk_connection_t *c);
int wtk_epoll_remove_connection(wtk_epoll_t *epoll,wtk_connection_t *c);
int wtk_epoll_wait(wtk_epoll_t* epoll,int timeout);
int wtk_epoll_process(wtk_epoll_t *epoll,int has_accept,int timeout,sem_t *sem);
int wtk_epoll_process2(wtk_epoll_t *epoll,int timeout,int *pret);
int wtk_epoll_event_remove_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_event_add_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
#ifdef __cplusplus
};
#endif
#endif
