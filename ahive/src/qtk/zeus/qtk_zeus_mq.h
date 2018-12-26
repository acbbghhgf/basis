#ifndef QTK_ZEUS_QTK_ZEUS_MQ_H
#define QTK_ZEUS_QTK_ZEUS_MQ_H
#include <stdint.h>
#include "wtk/os/wtk_lock.h"
#include "wtk/core/wtk_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_zserver;

#define qtk_zeus_in_mq(msgq) ((msgq)->in_mq)

typedef struct qtk_zeus_mq qtk_zeus_mq_t;
typedef struct qtk_zmesg qtk_zmesg_t;
typedef struct qtk_zmesg_queue qtk_zmesg_queue_t;
typedef void (*qtk_zmesg_drop_f)(qtk_zmesg_t*, void*);


struct qtk_zmesg {
    uint32_t source;
    int session;
    void *data;
    int sz;
};


struct qtk_zmesg_queue {
    wtk_queue_node_t node;
    wtk_lock_t lock;
    wtk_queue_t q;
    uint32_t handle;
    unsigned in_mq:1;
    unsigned release:1;
};


struct qtk_zeus_mq {
    wtk_lock_t lock;
    wtk_queue_t q;
};


qtk_zeus_mq_t* qtk_zmq_new();
int qtk_zmq_delete(qtk_zeus_mq_t *mq);
qtk_zmesg_queue_t* qtk_zmq_newq(uint32_t handle);
int qtk_zmq_release(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q, qtk_zmesg_drop_f drop_func, void* ud);
int qtk_zmq_mark_release(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q);
int qtk_zmq_pushq(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q);
qtk_zmesg_queue_t* qtk_zmq_popq(qtk_zeus_mq_t *mq);
int qtk_zmq_push(qtk_zmesg_queue_t *q, qtk_zmesg_t *msg, struct qtk_zserver* zserver);
int qtk_zmq_pop(qtk_zmesg_queue_t *q, qtk_zmesg_t* msg);
uint32_t qtk_zmq_handle(qtk_zmesg_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif
