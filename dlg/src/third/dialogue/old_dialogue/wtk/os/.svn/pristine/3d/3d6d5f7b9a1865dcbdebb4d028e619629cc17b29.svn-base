#ifndef WTK_CORE_WTK_BLOCKQUEUE_H_
#define WTK_CORE_WTK_BLOCKQUEUE_H_
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_lockqueue.h"
#include "wtk/os/wtk_sem.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_blockqueue wtk_blockqueue_t;

struct wtk_blockqueue
{
	WTK_LOCK_QUEUE
	wtk_sem_t sem;
};

int wtk_blockqueue_init(wtk_blockqueue_t *q);
int wtk_blockqueue_clean(wtk_blockqueue_t *q);
int wtk_blockqueue_push(wtk_blockqueue_t *q,wtk_queue_node_t *n);
wtk_queue_node_t* wtk_blockqueue_pop(wtk_blockqueue_t *q,int ms,int *is_timeout);
int wtk_blockqueue_wake(wtk_blockqueue_t *q);
int wtk_blockqueue_push_front(wtk_blockqueue_t *q,wtk_queue_node_t *n);
#ifdef __cplusplus
};
#endif
#endif
