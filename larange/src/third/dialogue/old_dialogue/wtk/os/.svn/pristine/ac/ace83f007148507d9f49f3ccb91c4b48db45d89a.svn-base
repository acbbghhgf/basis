#ifndef WTK_CORE_WTK_LOCKQUEUE_H_
#define WTK_CORE_WTK_LOCKQUEUE_H_
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_lock.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_lockqueue wtk_lockqueue_t;

#define WTK_LOCK_QUEUE \
	WTK_QUEUE \
	wtk_lock_t l;

struct wtk_lockqueue
{
	WTK_LOCK_QUEUE
};

int wtk_lockqueue_init(wtk_lockqueue_t *q);
int wtk_lockqueue_clean(wtk_lockqueue_t *q);
int wtk_lockqueue_push(wtk_lockqueue_t *q,wtk_queue_node_t *n);
int wtk_lockqueue_push_front(wtk_lockqueue_t *q,wtk_queue_node_t *n);
wtk_queue_node_t* wtk_lockqueue_pop(wtk_lockqueue_t *q);
#ifdef __cplusplus
};
#endif
#endif
