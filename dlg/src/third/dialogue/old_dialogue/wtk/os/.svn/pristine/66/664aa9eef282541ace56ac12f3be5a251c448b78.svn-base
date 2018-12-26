#ifndef WTK_OS_WTK_THREAD2
#define WTK_OS_WTK_THREAD2
#include "wtk/core/wtk_type.h" 
#include "wtk/os/wtk_blockqueue.h"
#include "wtk/core/wtk_os.h"
#include "wtk_thread.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_thread2 wtk_thread2_t;
typedef int(*wtk_thread2_start_f)(void *ths);
typedef int(*wtk_thread2_stop_f)(void *ths);
typedef int(*wtk_thread2_process_f)(void *ths,void *data);
typedef int(*wtk_thread2_err_f)(void *ths,void *data);

typedef enum
{
	WTK_THREAD2_START,
	WTK_THREAD2_STOP,
	WTK_THREAD2_PROCESS
}wtk_thread2_msg_type_t;

typedef struct
{
	wtk_queue_node_t q_n;
	wtk_thread2_msg_type_t type;
	void *data;
}wtk_thread2_msg_t;

typedef void(*wtk_thread2_timer_func_t)(void *ths,void *hook);

typedef struct
{
	wtk_queue_node_t q_n;
	double t;
	wtk_thread2_timer_func_t func;
	void *ths;
	void *hook;
}wtk_thread2_timer_t;

struct wtk_thread2
{
	wtk_thread_t thread;
	void *ths;
	wtk_thread2_start_f start;
	wtk_thread2_stop_f stop;
	wtk_thread2_process_f process;
	wtk_thread2_err_f err;
	wtk_blockqueue_t input_q;
	wtk_queue_t time_q;
	wtk_sem_t notify_sem;
	unsigned run:1;
};

wtk_thread2_t* wtk_thread2_new(void *ths,
		wtk_thread2_start_f start,
		wtk_thread2_stop_f stop,
		wtk_thread2_process_f process,
		wtk_thread2_err_f err);
void wtk_thread2_delete(wtk_thread2_t *thread);
int wtk_thread2_start(wtk_thread2_t *t);
int wtk_thread2_stop(wtk_thread2_t *t);

void wtk_thread2_msg_cancel(wtk_thread2_t *t);
void wtk_thread2_msg_start(wtk_thread2_t *t);
void wtk_thread2_msg_process(wtk_thread2_t *t,void *data);
int wtk_thread2_msg_stop(wtk_thread2_t *t);
int wtk_thread2_msg_stop2(wtk_thread2_t *t,int wait_eof);
int wtk_thread2_msg_wait_stop(wtk_thread2_t *t,int timeout);
void wtk_thread2_add_timer(wtk_thread2_t *dlg,double delay,wtk_thread2_timer_func_t func,void *ths,void *hook);
#ifdef __cplusplus
};
#endif
#endif
