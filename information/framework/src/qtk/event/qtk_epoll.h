#ifndef _QTK_EVENT_QTK_EPOLL_H
#define _QTK_EVENT_QTK_EPOLL_H
#include <unistd.h>
#include <sys/epoll.h>
#include "qtk_event.h"
#include "qtk_epoll_cfg.h"

typedef struct qtk_epoll qtk_epoll_t;

struct qtk_epoll {
    qtk_epoll_cfg_t *cfg;
    int fd;
    struct epoll_event* events;
    int size;
    int moniter_events;
    unsigned int et: 1;
};

int qtk_epoll_module_init(qtk_event_module_t *em, qtk_epoll_cfg_t *cfg);

#endif
