#ifndef WTK_NK_WTK_NK
#define WTK_NK_WTK_NK
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_hoard.h"
#include "wtk_connection.h"
#include "wtk_nk_cfg.h"
#include "wtk_listen.h"
#include "wtk/os/wtk_pipequeue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nk wtk_nk_t;

typedef void* (*wtk_nk_parser_new_f)(void *data,wtk_connection_t *c);
typedef void(*wtk_nk_parser_init_f)(void *ths);
typedef int(*wtk_nk_parser_feed_f)(void *ths,char *buf,int len);
typedef void(*wtk_nk_parser_want_close_f)(void *ths);
typedef void(*wtk_nk_parser_clean_f)(void *ths);
typedef void(*wtk_nk_parser_delete_f)(void *ths);

typedef struct
{
	void *ths;
	wtk_nk_parser_new_f newx;
	wtk_nk_parser_init_f init;
	wtk_nk_parser_feed_f feed;
	wtk_nk_parser_want_close_f want_close;
	wtk_nk_parser_clean_f clean;
	wtk_nk_parser_delete_f deletex;
}wtk_nk_parser_factory_t;

typedef void(*wtk_nk_process_pipe_f)(void *ths);

struct wtk_nk
{
	wtk_nk_cfg_t *cfg;
	wtk_log_t *log;
	wtk_poll_t *poll;
	wtk_listen_t *listen;
	char *rw_buf;
	wtk_hoard_t con_hoard;
	wtk_nk_parser_factory_t parser_factory;
	wtk_pipequeue_t pipeq;
	wtk_poll_process_evt_t pipe_evt;
	void *process_pipe_ths;
	wtk_nk_process_pipe_f process_pipe;
	unsigned run:1;
};

wtk_nk_t* wtk_nk_new(wtk_nk_cfg_t *cfg);
void wtk_nk_delete(wtk_nk_t *nk);
int wtk_nk_start(wtk_nk_t *nk);
int wtk_nk_stop(wtk_nk_t *nk);
int wtk_nk_run(wtk_nk_t *nk);

void wtk_nk_set_process_pipe(wtk_nk_t *nk,void *ths,wtk_nk_process_pipe_f p);
//-------------------- module reference api -------------------
wtk_connection_t* wtk_nk_pop_connection(wtk_nk_t *nk);
void wtk_nk_push_connection(wtk_nk_t *nk,wtk_connection_t *c);
#ifdef __cplusplus
};
#endif
#endif
