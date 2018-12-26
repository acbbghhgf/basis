#ifdef WIN32
#ifndef WTK_NK_MODULE_WTK_SELECT_H_
#define WTK_NK_MODULE_WTK_SELECT_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/http/nk/wtk_event.h"
#include "wtk/http/nk/wtk_connection.h"
#include <Winsock2.h>
#ifdef __cplusplus
extern "C" {
#endif
#define SHUT_RD SD_RECEIVE
typedef struct  wtk_select wtk_select_t;
typedef wtk_select_t wtk_epoll_t;

struct wtk_select
{
    int fd;
    int size;
    int montier_events;
    fd_set *r_set;
    fd_set *w_set;
    fd_set *e_set;
    fd_set *tmpr_set;
    fd_set *tmpw_set;
    fd_set *tmpe_set;
    wtk_queue_t event_queue;
};

int wtk_epoll_create(wtk_epoll_t *epoll);
int wtk_epoll_init(wtk_epoll_t *epoll,int event_num);
int wtk_epoll_clean(wtk_epoll_t *epoll);
int wtk_epoll_close(wtk_epoll_t *epoll);
int wtk_epoll_remove_connection(wtk_epoll_t *epoll,wtk_connection_t *c);
int wtk_epoll_remove_event(wtk_epoll_t* epoll,int fd, wtk_event_t *event);
int wtk_epoll_add_event(wtk_epoll_t *epoll,int fd,wtk_event_t* event);
int wtk_epoll_event_add_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_event_remove_write(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_event_remove_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_event_add_read(wtk_epoll_t *epoll,int fd,wtk_event_t *event);
int wtk_epoll_add_connection(wtk_epoll_t *epoll,wtk_connection_t *c);
int wtk_epoll_process(wtk_epoll_t *epoll,int timeout);
#endif
#ifdef __cplusplus
};
#endif
#endif
