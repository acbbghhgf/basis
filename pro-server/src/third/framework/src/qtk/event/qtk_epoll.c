#include <stdio.h>
#include "qtk_epoll.h"


static int qtk_epoll_add(qtk_epoll_t *epoll, int fd, uint32_t events, void *data) {
    struct epoll_event ev = {events, {data}};
    int ret;

    ret = epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd, &ev);
    if (ret != 0) {
        perror(__FUNCTION__);
    } else {
        ++epoll->moniter_events;
    }

    return ret;
}


static int qtk_epoll_del(qtk_epoll_t *epoll, int fd) {
    int ret;

    ret = epoll_ctl(epoll->fd, EPOLL_CTL_DEL, fd, NULL);
    if (ret != 0) {
        perror(__FUNCTION__);
    } else {
        --epoll->moniter_events;
    }

    return ret;
}


static int qtk_epoll_mod(qtk_epoll_t *epoll, int fd, uint32_t events, void *data) {
    struct epoll_event ev = {events, {data}};
    int ret;

    ret = epoll_ctl(epoll->fd, EPOLL_CTL_MOD, fd, &ev);
    if (ret != 0) {
        if (epoll_ctl(epoll->fd, EPOLL_CTL_ADD, fd, &ev)) {
            perror(__FUNCTION__);
        } else {
            ++epoll->moniter_events;
        }
    }

    return ret;
}


static int qtk_epoll_get_event_flags(qtk_epoll_t *epoll, qtk_event_t *event) {
    uint32_t events;

    events = EPOLLRDHUP | EPOLLHUP | EPOLLERR;
    event = (qtk_event_t*)((intptr_t)event & ~1L);
    if (event->want_read) {
        events |= EPOLLIN;
    }
    if (event->want_write) {
        events |= EPOLLOUT;
    }
    if (epoll->et && event->et) {
        events |= EPOLLET;
    }

    return events;
}


static int qtk_epoll_add_event(qtk_epoll_t* epoll, int fd, qtk_event_t *event) {
    int ret;
    uint32_t events;

    events = qtk_epoll_get_event_flags(epoll, event);
    ret = qtk_epoll_add(epoll, fd, events, event);

    return ret;
}


static int qtk_epoll_remove(qtk_epoll_t *epoll, int fd) {
    int ret;

    ret = qtk_epoll_del(epoll, fd);

    return ret;
}


static int qtk_epoll_mod_event(qtk_epoll_t* epoll, int fd, qtk_event_t *event) {
    int ret;
    uint32_t events;

    events = qtk_epoll_get_event_flags(epoll, event);
    ret = qtk_epoll_mod(epoll, fd, events, event);

    return ret;
}


static int qtk_epoll_process(qtk_epoll_t *epoll, int timeout, int flag) {
    int ret, i;
    struct epoll_event *event;
    uint32_t events;
    qtk_event_t *e;

    ret = epoll_wait(epoll->fd, epoll->events, epoll->size, timeout);
    if (ret <= 0) goto end;
    for (i = 0; i < ret; ++i) {
        event = &epoll->events[i];
        events = event->events;
        e = (qtk_event_t*)((intptr_t)event->data.ptr & ~1L);
        e->events = events;
        e->read = events & EPOLLIN ? 1 : 0;
        e->write = events & EPOLLOUT ? 1 : 0;
        e->error = events & EPOLLERR ? 1 : 0;
        if (!e->eof) {
            e->eof = events & EPOLLHUP ? 1 : 0;
        }
        if (!e->reof) {
            e->reof = events & EPOLLRDHUP ? 1 : 0;
        }
        if (e->write_handler == e->read_handler) {
            /*
              if it's not sure that close event will be processed in read_handler,
              write_handler must be same as read_handler, so that a handler

              cannot be called after close event
            */
            e->write_handler(e->data, event->data.ptr);
        } else {
            if (e->reof || e->eof || e->error) {
                e->read = 1;
            }
            if (!e->error && e->write && e->write_handler) {
                e->write_handler(e->data, event->data.ptr);
            }
            if (e->read && e->read_handler) {
                e->read_handler(e->data, event->data.ptr);
            }
        }
    }
    ret = 0;
end:
    return ret;
}


static int qtk_epoll_init(qtk_epoll_t *epoll) {
    int ret;
    qtk_epoll_cfg_t* cfg = epoll->cfg;

    epoll->size = cfg->event_num ? cfg->event_num : 100;
    if (cfg->et) {
        epoll->et = 1;
    }

    epoll->events = wtk_calloc(epoll->size, sizeof(struct epoll_event));
    if (epoll->events == NULL) {
        ret = -1;
        goto end;
    }
    epoll->fd = epoll_create(epoll->size);
    if (INVALID_FD == epoll->fd) {
        ret = -1;
        goto end;
    }
    ret = 0;
end:
    return ret;
}


static int qtk_epoll_clean(qtk_epoll_t *epoll) {
    if (INVALID_FD != epoll->fd) {
        close(epoll->fd);
        epoll->fd = INVALID_FD;
    }
    if (epoll->events) {
        wtk_free(epoll->events);
        epoll->events = NULL;
    }

    return 0;
}


static int qtk_epoll_delete(qtk_epoll_t *epoll) {
    qtk_epoll_clean(epoll);
    wtk_free(epoll);

    return 0;
}


int qtk_epoll_module_init(qtk_event_module_t *em, qtk_epoll_cfg_t *cfg) {
    qtk_epoll_t *epoll;

    epoll = (qtk_epoll_t*)wtk_malloc(sizeof(*epoll));
    epoll->cfg = cfg;
    epoll->et = 0;
    em->impl_d = epoll;
    em->add_ev = (qtk_eventctl_handler)qtk_epoll_add_event;
    em->mod_ev = (qtk_eventctl_handler)qtk_epoll_mod_event;
    em->del_ev = (qtk_eventctl_handler2)qtk_epoll_remove;
    em->process = (qtk_eventproc_handler)qtk_epoll_process;
    em->init = (qtk_eventimpl_handler)qtk_epoll_init;
    em->clean = (qtk_eventimpl_handler)qtk_epoll_clean;
    em->deleter = (qtk_eventimpl_handler)qtk_epoll_delete;

    return 0;
}
