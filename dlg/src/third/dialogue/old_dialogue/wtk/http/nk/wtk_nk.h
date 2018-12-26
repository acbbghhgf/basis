#ifndef WTK_NK_WTK_NET_H_
#define WTK_NK_WTK_NET_H_
#include "wtk/core/wtk_type.h"
//#include "wtk/core/wtk_bit_heap.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wtk_array.h"
#include "wtk_connection.h"
#include "wtk/http/nk/listen/wtk_listen.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_hoard.h"
#include "wtk/os/wtk_time.h"
#include "wtk/os/wtk_timer.h"
//#include "wtk/conf/wtk_conf_file.h"
#include "wtk/os/wtk_log.h"
#include "wtk/core/wtk_rbtree.h"
#include "wtk_nk_cfg.h"
#include "wtk/os/wtk_pipequeue.h"
#ifdef WIN32
#include "wtk/http/nk/module/wtk_select.h"
#else
#include "wtk/http/nk/module/wtk_epoll.h"
#include "wtk/os/wtk_cpu.h"
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nk wtk_nk_t;

typedef struct
{
	wtk_pipequeue_t pipe_queue;			//event that to be read from uplayer;
	wtk_event_t pipe_event;
}wtk_nk_pipe_t;

struct wtk_nk
{
	wtk_nk_cfg_t *cfg;
	wtk_array_t	*listening;
	wtk_heap_t* heap;
	wtk_hoard_t *con_hoard;
	wtk_hoard_t  *stack_hoard;
	wtk_epoll_t *epoll;
	wtk_stack_t *tmp_stk;				//used for merge response to string;
	wtk_strbuf_t *tmp_buf;				//used for temp buffer;
	char* rw_buf;						//used for read-write buffer;
	wtk_log_t *log;
#ifdef	USE_TREE
	wtk_rbtree_t *timer_tree;
#else
	wtk_queue_t timer_queue;			//used for timer;
#endif
	//---------------- pipe queue use for uplayer --------------------
	wtk_nk_pipe_t *pipe;

	wtk_string_t *nk_test;
#ifndef WIN32
	wtk_cpu_t *cpu;
    sem_t *accept_sem;
#endif
    unsigned int run:1;
    unsigned int read_delay:1;
};

wtk_nk_t* wtk_nk_new(wtk_nk_cfg_t *cfg,wtk_log_t *log);
int wtk_nk_delete(wtk_nk_t *nk);
int wtk_nk_bytes(wtk_nk_t *nk);
int wtk_nk_init(wtk_nk_t *net,wtk_nk_cfg_t *cfg,wtk_log_t *log);
int wtk_nk_clean(wtk_nk_t *net);
int wtk_nk_run(wtk_nk_t *net);
int wtk_nk_stop(wtk_nk_t *net);
int wtk_nk_start_listen(wtk_nk_t* net);
int wtk_nk_start_epoll(wtk_nk_t* net);

/**
 * @brief start listen and create epoll;
 */
int wtk_nk_prepare(wtk_nk_t *nk);
//========================== private section =====================
void wtk_nk_add_listen(wtk_nk_t *net,wtk_listen_t *l);
int wtk_nk_open_listen_fd(wtk_nk_t *net);
int wtk_nk_run(wtk_nk_t *net);
wtk_stack_t* wtk_nk_new_stack(wtk_nk_t *net);
wtk_stack_t* wtk_nk_pop_stack(wtk_nk_t *net);
int wtk_nk_push_stack(wtk_nk_t *net,wtk_stack_t* s);
wtk_connection_t* wtk_nk_new_connection(wtk_nk_t *net);
wtk_connection_t* wtk_nk_pop_connection(wtk_nk_t *net);
int wtk_nk_push_connection(wtk_nk_t *net,wtk_connection_t *c);
int wtk_nk_get_port(wtk_nk_t *net);
int wtk_nk_close_fd(wtk_nk_t *n);
void wtk_nk_clean_mem(wtk_nk_t *n);

/**
 *	delay: ms
 */
int wtk_nk_add_timer(wtk_nk_t *net,double delay,wtk_timer_t *timer,wtk_timer_handler handler,void *data);
void wtk_nk_print(wtk_nk_t *n);
#ifndef WIN32
void wtk_nk_add_accept_sem(wtk_nk_t *nk);
#endif

/**
 * @brief set client fd
 */
wtk_connection_t* wtk_nk_add_client_fd(wtk_nk_t *nk,int fd);

void wtk_nk_set_pipe_handler(wtk_nk_t *nk,void *ths,wtk_event_handler handler);
void wtk_nk_send_pipe_event(wtk_nk_t *nk,wtk_queue_node_t *n);
#ifdef __cplusplus
};
#endif
#endif
