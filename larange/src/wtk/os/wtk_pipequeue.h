#ifndef WTK_CORE_WTK_PIPEQUEUE_H_
#define WTK_CORE_WTK_PIPEQUEUE_H_
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_lockqueue.h"
#include "wtk/os/wtk_fd.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pipequeue wtk_pipequeue_t;

struct wtk_pipequeue
{
	WTK_LOCK_QUEUE
	/*
	 * 	pipe_fd 0 for read, 1 for write;read is non-blocked;
	 * this is used for single sender and single receiver,
	 * it's not safe with multi-sender and multi-receiver for not lock fd;
	 */
	int pipe_fd[2];
};

int wtk_pipequeue_init(wtk_pipequeue_t *q);
int wtk_pipequeue_clean(wtk_pipequeue_t *q);
int wtk_pipequeue_push(wtk_pipequeue_t *q,wtk_queue_node_t *n);
wtk_queue_node_t* wtk_pipequeue_pop(wtk_pipequeue_t *q);
int wtk_pipequeue_touch_write(wtk_pipequeue_t* q);
int wtk_pipequeue_touch_read(wtk_pipequeue_t* q);
#ifdef __cplusplus
};
#endif
#endif
