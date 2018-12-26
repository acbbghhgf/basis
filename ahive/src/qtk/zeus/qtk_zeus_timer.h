#ifndef QTK_ZEUS_QTK_ZEUS_TIMER_H
#define QTK_ZEUS_QTK_ZEUS_TIMER_H
#include "wtk/os/wtk_lock.h"
#include "wtk/core/wtk_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_zeus_timer qtk_zeus_timer_t;
typedef struct qtk_ztimer_node qtk_ztimer_node_t;

struct qtk_ztimer_node {
    wtk_queue_node_t node;
    uint64_t expire;
};

struct qtk_zeus_timer {
    wtk_lock_t lock;
    wtk_queue_t q;
    uint64_t cached_time;
};

qtk_zeus_timer_t* qtk_ztimer_new();
int qtk_ztimer_delete(qtk_zeus_timer_t *timer);
int qtk_ztimer_update(qtk_zeus_timer_t *timer);
int qtk_ztimer_add(qtk_zeus_timer_t *timer, void *arg, size_t sz, uint64_t time);
int qtk_ztimer_remove(qtk_zeus_timer_t *timer);
int qtk_ztimer_timeout(qtk_zeus_timer_t *timer, uint32_t handle, int time, int session);
uint64_t qtk_ztimer_now(qtk_zeus_timer_t *timer);

#ifdef __cplusplus
}
#endif

#endif
