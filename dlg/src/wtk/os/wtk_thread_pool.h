#ifndef WTK_OS_WTK_THREAD_POOL_H_
#define WTK_OS_WTK_THREAD_POOL_H_
#include "wtk/os/wtk_thread.h"
#include "wtk/os/wtk_blockqueue.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_thread_pool wtk_thread_pool_t;
typedef int (*thread_pool_handler)(void *user_data,wtk_queue_node_t *n,wtk_thread_t *thread);
typedef int (*wtk_thread_pool_init_f)(void *user_data,wtk_thread_t *thread);
typedef int (*wtk_thread_pool_clean_f)(void *user_data,wtk_thread_t *thread);
typedef void (*wtk_thread_pool_timeout_f)(void *user_data,wtk_thread_t *thread);

struct wtk_thread_pool
{
	int thread_num;
	wtk_thread_t *threads;
	wtk_blockqueue_t *queue;
	thread_pool_handler handler;
	wtk_thread_pool_init_f init;
	wtk_thread_pool_clean_f clean;
	wtk_thread_pool_timeout_f timeout_f;
	int timeout;
	void *user_data;
	unsigned run:1;
};

wtk_thread_pool_t *wtk_thread_pool_new(int threads,wtk_blockqueue_t *queue,	thread_pool_handler handler,void *user_data);
wtk_thread_pool_t *wtk_thread_pool_new2(int threads,wtk_blockqueue_t *queue,wtk_thread_pool_init_f init,
		wtk_thread_pool_clean_f clean,thread_pool_handler handler,void *user_data);
int wtk_thread_pool_bytes(wtk_thread_pool_t *t);
int wtk_thread_pool_delete(wtk_thread_pool_t *p);
void wtk_thread_pool_set_timeout(wtk_thread_pool_t *p,wtk_thread_pool_timeout_f timeout_f,int timeout);
int wtk_thread_pool_start(wtk_thread_pool_t *p);
int wtk_thread_pool_join(wtk_thread_pool_t *p);
int wtk_thread_pool_stop(wtk_thread_pool_t *p);
int wtk_thread_pool_kill(wtk_thread_pool_t *p);
#ifdef __cplusplus
};
#endif
#endif
