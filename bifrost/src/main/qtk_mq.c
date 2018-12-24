#include "os/qtk_spinlock.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"
#include "qtk_mq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_QUEUE_SIZE 64
#define MAX_GLOBAL_MQ 0x10000

// 0 means mq is not in global mq.
// 1 means mq is in global mq , or the message is dispatching.

#define MQ_IN_GLOBAL 1
#define MQ_OVERLOAD 1024

qtk_global_queue_t *Q = NULL;


void qtk_globalmq_push(qtk_message_queue_t *queue)
{
	qtk_global_queue_t *q = Q;

	QTK_SPIN_LOCK(q)
	assert(queue->next == NULL);
	if(q->tail) {
		q->tail->next = queue;
		q->tail = queue;
	} else {
		q->head = q->tail = queue;
	}
	QTK_SPIN_UNLOCK(q)
    sem_post(&q->sem);
}


qtk_message_queue_t *qtk_globalmq_pop(void)
{
	qtk_global_queue_t *q = Q;

    sem_wait(&q->sem);
	QTK_SPIN_LOCK(q)
	qtk_message_queue_t *mq = q->head;
	if(mq) {
		q->head = mq->next;
		if(q->head == NULL) {
			assert(mq == q->tail);
			q->tail = NULL;
		}
		mq->next = NULL;
	}
	QTK_SPIN_UNLOCK(q)

	return mq;
}


void qtk_globalmq_touch(void)
{
    qtk_global_queue_t *q = Q;
    sem_post(&q->sem);
}


qtk_message_queue_t *qtk_mq_create(uint32_t handle)
{
	qtk_message_queue_t *q = qtk_malloc(sizeof(*q));
	q->handle = handle;
	q->cap = DEFAULT_QUEUE_SIZE;
	q->head = 0;
	q->tail = 0;
	QTK_SPIN_INIT(q)
	// When the queue is create (always between service create and service init) ,
	// set in_global flag to avoid push it to global queue .
	// If the service init success, skynet_context_new will call skynet_mq_push to push it to global queue.
	q->in_global = MQ_IN_GLOBAL;
	q->release = 0;
	q->overload = 0;
	q->overload_threshold = MQ_OVERLOAD;
	q->queue = qtk_malloc(sizeof(qtk_message_t) * q->cap);
	q->next = NULL;

	return q;
}


static void _release(qtk_message_queue_t *q)
{
	assert(q->next == NULL);
	QTK_SPIN_DESTROY(q)
	qtk_free(q->queue);
	qtk_free(q);
}


int qtk_mq_length(qtk_message_queue_t *q)
{
	int head, tail,cap;

	QTK_SPIN_LOCK(q)
	head = q->head;
	tail = q->tail;
	cap = q->cap;
	QTK_SPIN_UNLOCK(q)

	if (head <= tail) {
		return tail - head;
	}
	return tail + cap - head;
}


int qtk_mq_overload(qtk_message_queue_t *q)
{
	if (q->overload) {
		int overload = q->overload;
		q->overload = 0;
		return overload;
	}
	return 0;
}


int qtk_mq_pop(qtk_message_queue_t *q, qtk_message_t *message)
{
	int ret = 1;
	QTK_SPIN_LOCK(q)

	if (q->head != q->tail) {
		*message = q->queue[q->head++];
		ret = 0;
		int head = q->head;
		int tail = q->tail;
		int cap = q->cap;

		if (head >= cap) {
			q->head = head = 0;
		}
		int length = tail - head;
		if (length < 0) {
			length += cap;
		}
		while (length > q->overload_threshold) {
			q->overload = length;
			q->overload_threshold *= 2;
		}
	} else {
		// reset overload_threshold when queue is empty
		q->overload_threshold = MQ_OVERLOAD;
	}

	if (ret) {
		q->in_global = 0;
	}

	QTK_SPIN_UNLOCK(q)

	return ret;
}


static void expand_queue(qtk_message_queue_t *q)
{
	qtk_message_t *new_queue = qtk_malloc(sizeof(qtk_message_t) * q->cap * 2);
	int i;
	for (i=0; i<q->cap; i++) {
		new_queue[i] = q->queue[(q->head + i) % q->cap];
	}
	q->head = 0;
	q->tail = q->cap;
	q->cap *= 2;

	qtk_free(q->queue);
	q->queue = new_queue;
}


void qtk_mq_push(qtk_message_queue_t *q, qtk_message_t *message)
{
	assert(message);
    QTK_SPIN_LOCK(q)

	q->queue[q->tail] = *message;
	if (++ q->tail >= q->cap) {
		q->tail = 0;
	}

	if (q->head == q->tail) {
		expand_queue(q);
	}

	if (q->in_global == 0) {
		q->in_global = MQ_IN_GLOBAL;
		qtk_globalmq_push(q);
	}

	QTK_SPIN_UNLOCK(q)
}


void qtk_globalmq_init()
{
	qtk_global_queue_t *q = qtk_malloc(sizeof(*q));
	memset(q,0,sizeof(*q));
	QTK_SPIN_INIT(q);
	Q=q;
}

void qtk_globalmq_clean()
{
    qtk_global_queue_t *q = Q;
    if (q) {
        QTK_SPIN_DESTROY(q);
        qtk_free(q);
        Q = NULL;
    }
}


void qtk_mq_mark_release(qtk_message_queue_t *q) {
	QTK_SPIN_LOCK(q)
	assert(q->release == 0);
	q->release = 1;
	if (q->in_global != MQ_IN_GLOBAL) {
		qtk_globalmq_push(q);
	}
	QTK_SPIN_UNLOCK(q)
}


static void _drop_queue(qtk_message_queue_t *q, message_drop drop_func, void *ud) {
	qtk_message_t msg;
	while(!qtk_mq_pop(q, &msg)) {
        if (drop_func) { drop_func(&msg, ud); }
	}
	_release(q);
}


void qtk_mq_release(qtk_message_queue_t *q, message_drop drop_func, void *ud) {
	QTK_SPIN_LOCK(q)

	if (q->release) {
		QTK_SPIN_UNLOCK(q)
		_drop_queue(q, drop_func, ud);
	} else {
		qtk_globalmq_push(q);
		QTK_SPIN_UNLOCK(q)
	}
}
