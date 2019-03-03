#ifndef WTK_DNNSVR_WTK_DNNUPSVR
#define WTK_DNNSVR_WTK_DNNUPSVR
#include "wtk/core/cfg/wtk_local_cfg.h" 
#include "wtk_dnnupsvr_cfg.h"
#include "wtk/nk/parser/wtk_msg_parser.h"
#include "wtk/os/wtk_blockqueue.h"
#include "wtk/os/wtk_thread_pool.h"
#include "wtk/core/wtk_hoard.h"
#include "wtk/os/wtk_log.h"
#include "wtk/os/wtk_pipequeue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnupsvr wtk_dnnupsvr_t;

typedef enum
{
	WTK_DNNUPSVR_MSG_DAT,
	WTK_DNNUPSVR_MSG_STOP,
	WTK_DNNUPSVR_MSG_CALC,
	WTK_DNNUPSVR_MSG_REUSE,
	WTK_DNNUPSVR_MSG_CALC_END,
	WTK_DNNUPSVR_MSG_STOP_END,
}wtk_dnnupsvr_msg_type_t;

typedef struct
{
	wtk_queue_node_t hoard_n;
	wtk_queue_node_t q_n;
	wtk_cudnn_matrix_t *input;
	wtk_cudnn_matrix_t *output;
	wtk_msg_parser_t *parser;
	int idx;
}wtk_dnn_calc_msg_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_dnnupsvr_msg_type_t type;
	union
	{
		wtk_msg_parser_t *parser;
		wtk_dnn_calc_msg_t *calc_msg;
	}hook;
	wtk_string_t *dat;
}wtk_dnnupsvr_msg_t;



struct wtk_dnnupsvr
{
	wtk_dnnupsvr_cfg_t *cfg;
	wtk_blockqueue_t input_q;
	wtk_blockqueue_t thread_q;
	wtk_hoard_t calc_msg_hoard;
	wtk_thread_pool_t *pool;
	wtk_log_t *log;
	wtk_pipequeue_t *pipeq;
	int input_size;
	int output_size;
	unsigned run:1;
};

wtk_dnnupsvr_t* wtk_dnnupsvr_new(wtk_dnnupsvr_cfg_t *cfg);
void wtk_dnnupsvr_delete(wtk_dnnupsvr_t *up);
void wtk_dnnupsvr_start(wtk_dnnupsvr_t *up);
void wtk_dnnupsvr_stop(wtk_dnnupsvr_t *up);
int wtk_dnnupsvr_run(wtk_dnnupsvr_t *up);

void wtk_dnnupsvr_feed(wtk_dnnupsvr_t *up,wtk_msg_parser_t *parser,char *data,int len);
void wtk_dnnupsvr_connection_want_close(wtk_dnnupsvr_t *up,wtk_msg_parser_t *parser);
#ifdef __cplusplus
};
#endif
#endif
