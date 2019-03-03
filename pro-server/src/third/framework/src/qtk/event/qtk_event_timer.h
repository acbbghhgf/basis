#ifndef _QTK_EVENT_QTK_EVENT_TIMER_H
#define _QTK_EVENT_QTK_EVENT_TIMER_H
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/os/wtk_lockhoard.h"
#include "wtk/os/wtk_time.h"
#include "qtk/event/qtk_event_timer_cfg.h"

#define QTK_TIMER_SUCCESS 0
#define QTK_TIMER_AGAIN   1
#define QTK_TIMER_FAILED  -1

struct qtk_bheap;

typedef struct qtk_event_timer qtk_event_timer_t;
typedef struct qtk_event_timer_item qtk_event_timer_item_t;
typedef int (*qtk_event_timer_handler)(void* data);

struct qtk_event_timer_item {
    wtk_queue_node_t timer_node_q;
    wtk_queue_node_t hoard_n;
    uint64_t key;    // ms
    uint32_t delay;
    qtk_event_timer_handler handler;
    void *data;
};

struct qtk_event_timer {
    struct qtk_bheap *timer_q;
    qtk_event_timer_cfg_t *cfg;
    wtk_heap_t *heap;
    wtk_hoard_t *item_hoard;
    int size;
    wtk_time_t time;
    wtk_lock_t lock;
};

qtk_event_timer_item_t* qtk_event_timer_item_new(wtk_heap_t *h);
int qtk_event_timer_item_init(qtk_event_timer_item_t *ti, long long key, qtk_event_timer_handler h, void *d);
int qtk_event_timer_item_reset(qtk_event_timer_item_t *ti);
int qtk_event_timer_item_delete(qtk_event_timer_item_t *ti);

qtk_event_timer_t* qtk_event_timer_new(qtk_event_timer_cfg_t *cfg);
int qtk_event_timer_init(qtk_event_timer_t *timer, qtk_event_timer_cfg_t *cfg);
int qtk_event_timer_process(qtk_event_timer_t *timer);
int qtk_event_timer_clean(qtk_event_timer_t *timer);
int qtk_event_timer_delete(qtk_event_timer_t *timer);
void* qtk_event_timer_add(qtk_event_timer_t *timer, long long delay, qtk_event_timer_handler handler, void* data);
int qtk_event_timer_remove(qtk_event_timer_t *timer, void *handle);

#endif
