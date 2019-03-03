#ifndef _WTK_NK_WTK_EVENT_H_
#define _WTK_NK_WTK_EVENT_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_queue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_event wtk_event_t;
typedef int (*wtk_event_handler)(void* data,wtk_event_t *e);
//#define wtk_event_epolled(e) ((e)->errepolled || (e)->readepolled || (e)->writeepolled )
#define wtk_event_epolled(e) ((e)->in_queue)
#define wtk_event_reset_sig(e) (e)->in_queue=(e)->read=(e)->write=(e)->eof=(e)->reof=(e)->error=(e)->writepending=(e)->writeepolled=(e)->readepolled=(e)->errepolled=0;

struct wtk_event
{
	void *data;
    wtk_event_handler handler;
#ifdef WIN32
    wtk_queue_node_t select_node_q;
    int fd;
#else
    uint32_t events;
    wtk_queue_node_t epoll_n;
#endif
    unsigned  in_queue:1;
	unsigned want_read:1;
	unsigned want_write:1;
	unsigned want_accept:1;
	unsigned read:1;
	unsigned write:1;
	unsigned eof:1;
	unsigned reof:1;
	unsigned error:1;
    unsigned writepending:1;
    unsigned writeepolled:1;
    unsigned readepolled:1;
    unsigned errepolled:1;
    //unsigned processed:1;			//event is processed or not;
};

void wtk_event_print(wtk_event_t *e);
//void wtk_event_reset_sig(wtk_event_t *e);
#ifdef __cplusplus
};
#endif
#endif
