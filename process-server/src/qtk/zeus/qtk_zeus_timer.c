#include "qtk_zeus_timer.h"
#include "qtk_zeus.h"
#include "qtk_zeus_mq.h"
#include "qtk_zeus_server.h"
#include <time.h>

typedef struct qtk_ztimer_event qtk_ztimer_event_t;

struct qtk_ztimer_event {
    uint32_t handle;
    int session;
};

static uint64_t qtk_ztimer_get_ms() {
    struct timeval tv;
    uint64_t ret = 0;
    int err;

    err = gettimeofday(&tv,0);
    if (err == 0) {
        ret = (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
    return ret;
}


static int qtk_ztimer_cmp(qtk_ztimer_node_t *d1, qtk_ztimer_node_t *d2) {
    if (d1->expire < d2->expire) return 1;
    else if (d1->expire > d2->expire) return -1;
    return 0;
}


int qtk_ztimer_emit(qtk_zeus_timer_t *timer, uint32_t handle, int session) {
    qtk_zmesg_t msg;
    msg.source = 0;
    msg.session = session;
    msg.data = NULL;
    msg.sz = (size_t)(ZEUS_PTYPE_RESPONSE) << ZEUS_TYPE_SHIFT;
    return qtk_zcontext_push(handle, &msg);
}


qtk_zeus_timer_t *qtk_ztimer_new() {
    qtk_zeus_timer_t *timer = wtk_malloc(sizeof(*timer));
    wtk_lock_init(&timer->lock);
    timer->cached_time = qtk_ztimer_get_ms();
    wtk_queue_init(&timer->q);
    return timer;
}


int qtk_ztimer_delete(qtk_zeus_timer_t *timer) {
    wtk_lock_clean(&timer->lock);
    wtk_free(timer);
    return 0;
}


int qtk_ztimer_update(qtk_zeus_timer_t *timer) {
    wtk_lock_lock(&timer->lock);
    timer->cached_time = qtk_ztimer_get_ms();
    wtk_queue_node_t *n;
    while ((n = wtk_queue_peek(&timer->q, 0))) {
        qtk_ztimer_node_t *tn = data_offset(n, qtk_ztimer_node_t, node);
        if (tn->expire <= timer->cached_time) {
            qtk_ztimer_event_t *ev = (qtk_ztimer_event_t*)(tn+1);
            qtk_ztimer_emit(timer, ev->handle, ev->session);
            wtk_queue_pop(&timer->q);
            wtk_free(tn);
        } else {
            break;
        }
    }
    wtk_lock_unlock(&timer->lock);
    return 0;
}


int qtk_ztimer_add(qtk_zeus_timer_t *timer, void *arg, size_t sz, uint64_t time) {
    qtk_ztimer_node_t *node = wtk_malloc(sizeof(*node) + sz);
    wtk_queue_node_init(&node->node);
    node->expire = timer->cached_time + time;
    memcpy(node+1, arg, sz);
    wtk_lock_lock(&timer->lock);
    wtk_queue_insert(&timer->q, &node->node, (wtk_cmp_handler_t)qtk_ztimer_cmp);
    wtk_lock_unlock(&timer->lock);
    return 0;
}


int qtk_ztimer_timeout(qtk_zeus_timer_t *timer, uint32_t handle, int time, int session) {
    if (time <= 0) {
        return qtk_ztimer_emit(timer, handle, session);
    } else {
        qtk_ztimer_event_t event;
        event.handle = handle;
        event.session = session;
        qtk_ztimer_add(timer, &event, sizeof(event), time);
    }

    return session;
}


uint64_t qtk_ztimer_now(qtk_zeus_timer_t *timer) {
    return timer->cached_time;
}
