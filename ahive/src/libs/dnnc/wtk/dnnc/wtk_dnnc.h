#ifndef WTK_FST_WTK_DNNC
#define WTK_FST_WTK_DNNC
#include "wtk/core/wtk_type.h" 
#include "wtk/os/wtk_thread.h"
#include "wtk_dnnc_cfg.h"
#include "wtk/os/wtk_pipequeue.h"
#include "wtk/core/wtk_buf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_dnnc wtk_dnnc_t;

typedef enum
{
	WTK_DNNC_STATE_INIT=0,
	WTK_DNNC_STATE_DAT,
	WTK_DNNC_STATE_WAIT_END,
}wtk_dnnc_state_t;

typedef enum
{
	WTK_DNNC_PARSER_INIT=0,
	WTK_DNNC_PARSER_HDR,
	WTK_DNNC_PARSER_DATA_HDR,
	WTK_DNNC_PARSER_DATA_HDR2,
	WTK_DNNC_PARSER_DATA,
}wtk_dnnc_parser_state_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_string_t *data;
	unsigned is_end:1;
}wtk_dnnc_msg_t;

/*
 * cmd:
 *    * 1 data
 *    * 2 end
 *    * 3 err
 */
typedef void(*wtk_dnnc_raise_f)(void *ths,int cmd,int row,int col,float *data);

struct wtk_dnnc
{
	wtk_dnnc_cfg_t *cfg;
	wtk_thread_t thread;
	wtk_pipequeue_t pipeq;
	wtk_buf_t *wbuf;
	wtk_strbuf_t *rxbuf;
	char *rbuf;
	int fd;
	wtk_dnnc_state_t state;
	wtk_dnnc_parser_state_t rstate;
	union
	{
		uint32_t v;
		char data[4];
	}msg_size;
	int msg_size_pos;
	int cmd;
	int v[3];//idx,row,col;
	void *ths;
	wtk_dnnc_raise_f raise;
	unsigned run:1;
};

wtk_dnnc_t* wtk_dnnc_new(wtk_dnnc_cfg_t *cfg);
void wtk_dnnc_delete(wtk_dnnc_t *dnnc);
void wtk_dnnc_set_raise(wtk_dnnc_t *dnnc,void *ths,wtk_dnnc_raise_f raise);
void wtk_dnnc_reset(wtk_dnnc_t *dnnc);
void wtk_dnnc_feed(wtk_dnnc_t *dnnc,float *f,int len,int is_end);
#ifdef __cplusplus
};
#endif
#endif
