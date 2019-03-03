#ifndef WTK_NK_POLL_WTK_POLL
#define WTK_NK_POLL_WTK_POLL
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_poll_cfg.h"
#include "wtk_epoll.h"
#include "wtk/nk/wtk_connection.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_poll wtk_poll_t;

typedef void(*wtk_poll_process_f)(void *ths,wtk_poll_evt_t *evt);

typedef struct
{
	wtk_poll_evt_t evt;
	void *ths;
	wtk_poll_process_f process;
}wtk_poll_process_evt_t;

struct wtk_poll
{
	wtk_poll_cfg_t *cfg;
	union
	{
		wtk_epoll_t *epoll;
	}v;
};

wtk_poll_t* wtk_poll_new(wtk_poll_cfg_t *cfg);
void wtk_poll_delete(wtk_poll_t *poll);
int wtk_poll_start(wtk_poll_t *poll);
void wtk_poll_stop(wtk_poll_t *poll);
void wtk_poll_add_process_evt(wtk_poll_t *poll,int fd,wtk_poll_process_evt_t *evt);
int wtk_poll_add_connection(wtk_poll_t *poll,wtk_connection_t *c);
void wtk_poll_remove_connection(wtk_poll_t *poll,wtk_connection_t *c);
void wtk_poll_connection_add_write(wtk_poll_t *poll,wtk_connection_t *c);
void wtk_poll_connection_remove_write(wtk_poll_t *poll,wtk_connection_t *c);
int wtk_poll_process(wtk_poll_t *poll);
#ifdef __cplusplus
};
#endif
#endif
