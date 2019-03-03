#include "wtk/core/wtk_alloc.h"
#include "qtk_event.h"
#include "qtk_epoll.h"
#include "qtk_event_timer.h"


qtk_event_t* qtk_event_new(wtk_heap_t *h) {
    qtk_event_t *ev;

    ev = (qtk_event_t*)wtk_heap_zalloc(h, sizeof(qtk_event_t));

    return ev;
}


int qtk_event_init(qtk_event_t *ev,
                   int flag,
                   qtk_event_handler rhandler,
                   qtk_event_handler whandler, void *d) 
{
    ev->read_handler = rhandler;
    ev->write_handler = whandler;
    ev->data = d;
    ev->want_read = (flag & QTK_EVENT_READ) ? 1 : 0;
    ev->want_write = (flag & QTK_EVENT_WRITE) ? 1 : 0;
    ev->read = 0;
    ev->write = 0;
    ev->error = 0;
    ev->eof = 0;
    ev->reof = 0;
    ev->et = 0;
    ev->events = 0;

    return 0;
}


int qtk_event_reset(qtk_event_t *ev) {
    return qtk_event_init(ev, 0, NULL, NULL, NULL);
}


int qtk_event_clean(qtk_event_t *ev) {
    return qtk_event_reset(ev);
}


qtk_event_module_t* qtk_event_module_new(qtk_event_cfg_t *cfg) {
    qtk_event_module_t *em;

    em = (qtk_event_module_t*)wtk_malloc(sizeof(*em));
    em->cfg = cfg;
    if (cfg->use_epoll) {
        qtk_epoll_module_init(em, &cfg->poll.epoll);
    } else {
        // use epoll default
        cfg->use_epoll = 1;
        qtk_epoll_module_init(em, &cfg->poll.epoll);
    }
    if (cfg->use_timer) {
        em->timer = qtk_event_timer_new(&cfg->timer);
    } else {
        em->timer = NULL;
    }
    if (qtk_event_module_init(em)) {
        wtk_debug("terrible bug\r\n");
        qtk_event_module_delete(em);
        em = NULL;
    }

    return em;
}


int qtk_event_module_init(qtk_event_module_t *em) {
    int ret;
    qtk_event_cfg_t *cfg;

    cfg = em->cfg;
    ret = em->init(em->impl_d);
    if (ret) {
        goto end;
    }

    if (em->timer) {
        ret = qtk_event_timer_init(em->timer, &cfg->timer);
        if (ret) {
            goto end;
        }
    }
    ret = 0;
end:
    return 0;
}


int qtk_event_module_delete(qtk_event_module_t *em) {
    em->deleter(em->impl_d);
    if (em->timer) {
        qtk_event_timer_delete(em->timer);
    }
    wtk_free(em);

    return 0;
}


int qtk_event_module_process(qtk_event_module_t *em, int timeout, int flag) {
    qtk_event_process(em, timeout, flag);
    if (em->timer) {
        qtk_event_timer_process(em->timer);
    }

    return 0;
}
