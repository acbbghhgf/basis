#include "qtk_zeus_mq.h"
#include "wtk/core/wtk_alloc.h"
#include "qtk_zeus_server.h"
#include <assert.h>


typedef struct qtk_zmesg_node qtk_zmesg_node_t;

struct qtk_zmesg_node {
    wtk_queue_node_t node;
    qtk_zmesg_t msg;
};


static int _release(qtk_zmesg_queue_t *q) {
    wtk_lock_clean(&q->lock);
    wtk_free(q);
    return 0;
}


/* static int _remove(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q) { */
/*     wtk_queue_remove(&mq->q, &q->node); */
/*     return 0; */
/* } */


qtk_zeus_mq_t* qtk_zmq_new() {
    qtk_zeus_mq_t *mq = (qtk_zeus_mq_t*)wtk_malloc(sizeof(qtk_zeus_mq_t));
    wtk_queue_init(&mq->q);
    wtk_lock_init(&mq->lock);
    return mq;
}


int qtk_zmq_delete(qtk_zeus_mq_t *mq) {
    wtk_lock_clean(&mq->lock);
    wtk_free(mq);
    return 0;
}


qtk_zmesg_queue_t* qtk_zmq_newq(uint32_t handle) {
    qtk_zmesg_queue_t *q = (qtk_zmesg_queue_t*)wtk_malloc(sizeof(*q));
    wtk_queue_node_init(&q->node);
    wtk_queue_init(&q->q);
    q->handle = handle;
    q->in_mq = 1;
    q->release = 0;
    wtk_lock_init(&q->lock);
    return q;
}


int qtk_zmq_mark_release(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q) {
    wtk_lock_lock(&q->lock);
    assert(0 == q->release);
    q->release = 1;
    if (!q->in_mq) {
        qtk_zmq_pushq(mq, q);
    }
    wtk_lock_unlock(&q->lock);
    return 0;
}


int qtk_zmq_release(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q, qtk_zmesg_drop_f drop_func, void* ud) {
    wtk_lock_t *lock = &q->lock;
    wtk_lock_lock(lock);
    if (!q->release) {
        qtk_zmq_pushq(mq, q);
        q = NULL;
    }
    wtk_lock_unlock(lock);
    if (q) {
        wtk_queue_node_t *n;
        while ((n = wtk_queue_pop(&q->q))) {
            qtk_zmesg_node_t *msg_node = data_offset(n, qtk_zmesg_node_t, node);
            drop_func(&msg_node->msg, ud);
        }
        return _release(q);
    }
    return 0;
}


int qtk_zmq_pushq(qtk_zeus_mq_t *mq, qtk_zmesg_queue_t *q) {
    wtk_lock_lock(&mq->lock);
    wtk_queue_push(&mq->q, &q->node);
    wtk_lock_unlock(&mq->lock);
    return 0;
}


qtk_zmesg_queue_t* qtk_zmq_popq(qtk_zeus_mq_t *mq) {
    wtk_queue_node_t *n;
    qtk_zmesg_queue_t *q;
    wtk_lock_lock(&mq->lock);
    n = wtk_queue_pop(&mq->q);
    q = data_offset(n, qtk_zmesg_queue_t, node);
    wtk_lock_unlock(&mq->lock);
    return q;
}


int qtk_zmq_push(qtk_zmesg_queue_t *q, qtk_zmesg_t* msg, qtk_zserver_t *zserver) {
    qtk_zmesg_node_t *msg_n = wtk_malloc(sizeof(*msg_n));
    wtk_queue_node_init(&msg_n->node);
    msg_n->msg = *msg;
    wtk_lock_lock(&q->lock);
    wtk_queue_push(&q->q, &msg_n->node);
    if (!q->in_mq) {
        q->in_mq = 1;
        qtk_zmq_pushq(zserver->mq, q);
        pthread_mutex_lock(&zserver->msg_mutex);
        if (zserver->nsleep > 0) {
            pthread_cond_signal(&zserver->msg_cond);
        }
        pthread_mutex_unlock(&zserver->msg_mutex);
    }
    wtk_lock_unlock(&q->lock);
    return 0;
}


int qtk_zmq_pop(qtk_zmesg_queue_t *q, qtk_zmesg_t *msg) {
    wtk_queue_node_t *n;
    wtk_lock_lock(&q->lock);
    n = wtk_queue_pop(&q->q);
    if (NULL == n) {
        q->in_mq = 0;
    }
    wtk_lock_unlock(&q->lock);
    if (n) {
        qtk_zmesg_node_t *msg_node = data_offset(n, qtk_zmesg_node_t, node);
        *msg = msg_node->msg;
        wtk_free(msg_node);
        return 0;
    } else {
        return -1;
    }
}


uint32_t qtk_zmq_handle(qtk_zmesg_queue_t *q) {
    return q->handle;
}
