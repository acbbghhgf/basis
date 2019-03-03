#ifndef _QTK_EVENT_QTK_EVENT_H
#define _QTK_EVENT_QTK_EVENT_H
#include "qtk/event/qtk_event_cfg.h"

#define QTK_EVENT_READ   1
#define QTK_EVENT_WRITE  2

struct qtk_connection;
struct qtk_event_timer;

typedef struct qtk_event qtk_event_t;
typedef struct qtk_event2 qtk_event2_t;
typedef struct qtk_event_module qtk_event_module_t;

typedef int(*qtk_event_handler)(void* data, qtk_event_t *e);
typedef int(*qtk_eventctl_handler)(void *ths, int fd, qtk_event_t*e);
typedef int(*qtk_eventctl_handler2)(void *ths, int fd);
typedef int(*qtk_connctl_handler)(void *ths, struct qtk_connection *c);
typedef int(*qtk_eventproc_handler)(void *ths, int timeout, int flag);
typedef int(*qtk_eventimpl_handler)(void *ths);


struct qtk_event {
    void *data;
    qtk_event_handler read_handler;
    qtk_event_handler write_handler;
    uint32_t events;
    unsigned want_read:1;
    unsigned want_write:1;
    unsigned read:1;
    unsigned write:1;
    unsigned error:1;
    unsigned eof:1;
    unsigned reof:1;
    unsigned et:1;
};


struct qtk_event2 {
    qtk_event_t ev;
    int fd;
};


struct qtk_event_module {
    qtk_event_cfg_t *cfg;
    int (*add_ev)(void *ths, int fd, qtk_event_t *event);
    int (*mod_ev)(void *ths, int fd, qtk_event_t *event);
    int (*del_ev)(void *ths, int fd);
    int (*add_conn)(void *ths, struct qtk_connection *c);
    int (*mod_conn)(void *ths, struct qtk_connection *c);
    int (*del_conn)(void *ths, struct qtk_connection *c);
    int (*process)(void *ths, int timeout, int flag);
    int (*init)(void *ths);
    int (*clean)(void *ths);
    int (*deleter)(void *ths);
    void *impl_d;                      /* currently, qtk_epoll_t */
    struct qtk_event_timer *timer;
};


qtk_event_module_t* qtk_event_module_new(qtk_event_cfg_t *cfg);
qtk_event_module_t* qtk_event_module_new2(qtk_event_cfg_t *cfg);
int qtk_event_module_delete(qtk_event_module_t *em);
int qtk_event_module_delete2(qtk_event_module_t *em);
int qtk_event_module_init(qtk_event_module_t *em);
int qtk_event_module_run(qtk_event_module_t *em);
int qtk_event_module_process(qtk_event_module_t *em, int timeout, int flag);
qtk_event_module_t* qtk_event_get_module();

#define qtk_event_add_event(em, fd, event) ((em)->add_ev((em)->impl_d, fd, event))
#define qtk_event_del_event(em, fd) ((em)->del_ev((em)->impl_d, fd))
#define qtk_event_mod_event(em, fd, event) ((em)->mod_ev((em)->impl_d, fd, event))
#define qtk_event_process(em, timeout, flag) ((em)->process((em)->impl_d, timeout, flag))

qtk_event_t* qtk_event_new(wtk_heap_t *h);
int qtk_event_init(qtk_event_t *ev, int flag, qtk_event_handler rhander, qtk_event_handler whandler, void *d);
int qtk_event_reset(qtk_event_t *ev);
int qtk_event_clean(qtk_event_t *ev);

#endif
