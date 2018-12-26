#ifndef WTK_OS_DTP_WTK_DTP_H_
#define WTK_OS_DTP_WTK_DTP_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/os/wtk_blockqueue.h"
#include "wtk_dtp_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dtp wtk_dtp_t;
typedef void*(*wtk_dtp_inst_new_f)(void *ths);
typedef void(*wtk_dtp_inst_delete_f)(void *inst);
typedef void(*wtk_dtp_inst_process_f)(void *ths,void *inst,wtk_queue_node_t *n);

typedef struct
{
	void *ths;
	wtk_dtp_inst_new_f inst_new;
	wtk_dtp_inst_delete_f inst_delete;
	wtk_dtp_inst_process_f	inst_process;
}wtk_dtp_handler_t;

typedef struct
{
	wtk_thread_t thread;
	wtk_queue_node_t q_n;
	void *inst;
	wtk_dtp_t *dtp;
}wtk_dtp_route_t;

struct wtk_dtp
{
	wtk_dtp_cfg_t *cfg;
	wtk_dtp_handler_t *handler;
	wtk_heap_t *heap;
	wtk_queue_t thread_q;  //wtk_dtp_route_t
	wtk_blockqueue_t input_q;
	unsigned run:1;
};

wtk_dtp_t* wtk_dtp_new(wtk_dtp_cfg_t *cfg,wtk_dtp_handler_t *h);
void wtk_dtp_delete(wtk_dtp_t *dtp);
void wtk_dtp_push(wtk_dtp_t *dtp,wtk_queue_node_t *n);
void wtk_dtp_join(wtk_dtp_t *dtp);
void wtk_dtp_stop(wtk_dtp_t *dtp);
#ifdef __cplusplus
};
#endif
#endif
