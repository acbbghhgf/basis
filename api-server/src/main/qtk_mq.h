#ifndef MAIN_QTK_MESSAGE_QUEUE_H_
#define MAIN_QTK_MESSAGE_QUEUE_H_

#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>

#include "os/qtk_spinlock.h"

typedef struct qtk_message qtk_message_t;
typedef struct qtk_message_queue qtk_message_queue_t;
typedef struct qtk_global_queue qtk_global_queue_t;

// type is encoding in skynet_message.sz high 8bit
#define MESSAGE_TYPE_MASK    (SIZE_MAX >> 8)
#define MESSAGE_TYPE_SHIFT   ((sizeof(size_t)-1) * 8)

#define MQ_GET_HANDLE(q)     ((q)->handle)

struct qtk_message {
	uint32_t source;
    /* session == 0 means we need no response */
	int session;
	void * data;
	size_t sz;
};

struct qtk_message_queue {
	qtk_spinlock_t lock;
	uint32_t handle;
	int cap;
	int head;
	int tail;
	int release;
	int in_global;
	int overload;
	int overload_threshold;
	qtk_message_t *queue;
	qtk_message_queue_t *next;
};

struct qtk_global_queue {
	qtk_message_queue_t *head;
	qtk_message_queue_t *tail;
    qtk_spinlock_t lock;
    sem_t sem;
};

void qtk_globalmq_push(qtk_message_queue_t *queue);
qtk_message_queue_t *qtk_globalmq_pop(void);
void qtk_globalmq_touch(void);

qtk_message_queue_t *qtk_mq_create(uint32_t handle);
void qtk_mq_mark_release(qtk_message_queue_t *q);

typedef void (*message_drop)(qtk_message_t *, void *);

void qtk_mq_release(qtk_message_queue_t *q, message_drop drop_func, void *ud);

// 0 for success
int qtk_mq_pop(qtk_message_queue_t *q, qtk_message_t *message);
void qtk_mq_push(qtk_message_queue_t *q, qtk_message_t *message);

// return the length of message queue, for debug
int qtk_mq_length(qtk_message_queue_t *q);
int qtk_mq_overload(qtk_message_queue_t *q);

void qtk_globalmq_init();
void qtk_globalmq_clean();

#endif
