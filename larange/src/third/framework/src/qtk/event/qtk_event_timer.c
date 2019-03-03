#include "qtk/event/qtk_event_timer.h"
#include "qtk/core/qtk_bheap.h"


qtk_event_timer_item_t* qtk_event_timer_item_new(wtk_heap_t *h) {
    qtk_event_timer_item_t *ti;

    ti = (qtk_event_timer_item_t*)wtk_heap_zalloc(h, sizeof(qtk_event_timer_item_t));

    return ti;
}


int qtk_event_timer_item_delete(qtk_event_timer_item_t *ti) {
    return 0;
}


int qtk_event_timer_item_init(qtk_event_timer_item_t *ti,
                              long long key,
                              qtk_event_timer_handler handler,
                              void *data) {
    ti->key = key;
    ti->handler = handler;
    ti->data = data;
    return 0;
}


int qtk_event_timer_item_clean(qtk_event_timer_item_t *ti) {
    ti->key = 0;
    ti->handler = NULL;
    ti->data = NULL;

    return 0;
}


static qtk_event_timer_item_t* qtk_new_timer(qtk_event_timer_t *timer) {
    return qtk_event_timer_item_new(timer->heap);
}


static qtk_event_timer_item_t* qtk_timer_acquire_item(qtk_event_timer_t *timer) {
    return wtk_hoard_pop(timer->item_hoard);
}


static int qtk_timer_release_item(qtk_event_timer_t *timer, qtk_event_timer_item_t *ti) {
    qtk_event_timer_item_clean(ti);
    return wtk_hoard_push(timer->item_hoard, ti);
}


static int qtk_timer_item_cmp(void *l, void *r) {
    qtk_event_timer_item_t *li = l, *ri = r;
    return li->key - ri->key;
}


static int qtk_timer_item_del(void *udata, void *data) {
    return qtk_timer_release_item(udata, data);
}


qtk_event_timer_t* qtk_event_timer_new(qtk_event_timer_cfg_t *cfg) {
    qtk_event_timer_t *timer = (qtk_event_timer_t*)wtk_calloc(1, sizeof(qtk_event_timer_t));

    timer->cfg = cfg;
    timer->size = cfg->timer_size;
    timer->heap = wtk_heap_new(4096);
    timer->timer_q = qtk_bheap_new(256, qtk_timer_item_cmp, NULL);
    timer->item_hoard = (wtk_hoard_t*)wtk_heap_malloc(timer->heap, sizeof(wtk_lockhoard_t));

    return timer;
}


int qtk_event_timer_init(qtk_event_timer_t *timer, qtk_event_timer_cfg_t *cfg) {
    timer->cfg = cfg;
    timer->size = cfg->timer_size;
    wtk_hoard_init(timer->item_hoard, offsetof(qtk_event_timer_item_t, hoard_n),
                   timer->size, (wtk_new_handler_t)qtk_new_timer,
                   (wtk_delete_handler_t)qtk_event_timer_item_delete, timer);
    wtk_time_init(&timer->time);
    wtk_lock_init(&timer->lock);
    return 0;
}


int qtk_event_timer_process(qtk_event_timer_t *timer) {
    qtk_event_timer_item_t *ti;
    wtk_queue_t tmp_q;
    double ms;

    wtk_queue_init(&tmp_q);
    ms = wtk_time_ms(&timer->time);
    wtk_time_update(&timer->time);
    ms = wtk_time_ms(&timer->time) - ms;
    if (ms > 20) {
        /*wtk_debug("WARNING: last process take too much time, %lf\r\n", ms);*/
    }

    if (qtk_bheap_head(timer->timer_q)) {
        ti = qtk_bheap_head(timer->timer_q)->data;
        if (ti->key > (long long)timer->time.wtk_cached_time) {
            goto end;
        }
    }

    while (qtk_bheap_head(timer->timer_q)) {
        wtk_lock_lock(&timer->lock);
        ti = qtk_bheap_head(timer->timer_q)->data;
        if (ti->key > (long long)timer->time.wtk_cached_time) {
            wtk_lock_unlock(&timer->lock);
            break;
        }
        qtk_bheap_pop(timer->timer_q);
        wtk_lock_unlock(&timer->lock);
        ti->handler(ti->data);
    }
end:
    return 0;
}


int qtk_event_timer_clean(qtk_event_timer_t *timer) {
    wtk_lock_lock(&timer->lock);
    qtk_bheap_delete2(timer->timer_q, qtk_timer_item_del, timer);
    wtk_hoard_clean(timer->item_hoard);
    wtk_lock_unlock(&timer->lock);
    wtk_time_clean(&timer->time);
    wtk_lock_clean(&timer->lock);
    return 0;
}


int qtk_event_timer_delete(qtk_event_timer_t *timer) {
    qtk_event_timer_clean(timer);
    wtk_heap_delete(timer->heap);
    wtk_free(timer);
    return 0;
}


void* qtk_event_timer_add(qtk_event_timer_t *timer,
                          long long delay,
                          qtk_event_timer_handler handler,
                          void* data) {
    int ret;
    long long key;
    void *handle = NULL;
    qtk_event_timer_item_t *ti;

    wtk_lock_lock(&timer->lock);
    ti = qtk_timer_acquire_item(timer);

    if (NULL == ti) {
        goto end;
    }
    key = (long long)timer->time.wtk_cached_time + delay;
    ret = qtk_event_timer_item_init(ti, key, handler, data);
    if (ret) {
        qtk_timer_release_item(timer, ti);
        goto end;
    }
    handle = qtk_bheap_insert(timer->timer_q, ti);
end:
    wtk_lock_unlock(&timer->lock);
    return handle;
}


int qtk_event_timer_remove(qtk_event_timer_t *timer, void *handle) {
    int ret;

    qtk_event_timer_item_t *ti = ((qtk_bheap_node_t *)handle)->data;
    wtk_lock_lock(&timer->lock);
    ret = qtk_bheap_remove(timer->timer_q, handle);
    if (ret) {
        wtk_debug("timer may be triggered\r\n");
        goto end;
    }
    ret = qtk_timer_release_item(timer, ti);
    if (ret) {
        wtk_debug("terrible bug\r\n");
        goto end;
    }
end:
    wtk_lock_unlock(&timer->lock);
    return ret;
}
