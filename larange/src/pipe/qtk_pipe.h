#ifndef _PIPE_QTK_PIPE_H_
#define _PIPE_QTK_PIPE_H_

#include "qtk/ipc/qtk_unsocket.h"
#include "wtk/core/wtk_stack.h"
#include "qtk/event/qtk_event.h"
#include "pack/qtk_parser.h"

typedef struct qtk_pipe qtk_pipe_t;
typedef int (*qtk_pipe_handler)(void *ud, qtk_pipe_t *p);

struct qtk_pipe {
    qtk_unsocket_t s;
    pid_t pid;
    wtk_stack_t *ws;
    qtk_event_t ev;
    qtk_event_module_t *em;         // from caller
    qtk_parser_t ps;                // from caller
    qtk_pipe_handler close_notifier;
    void *ud;
    unsigned connected : 1;
};

qtk_pipe_t *qtk_pipe_new();
void qtk_pipe_init(qtk_pipe_t *p);
void qtk_pipe_clean(qtk_pipe_t *p);
void qtk_pipe_delete(qtk_pipe_t *p);
void qtk_pipe_set_close_notifier(qtk_pipe_t *p, qtk_pipe_handler f, void *ud);
int qtk_pipe_send(qtk_pipe_t *p, const char *d, int l);
int qtk_pipe_send_string(qtk_pipe_t *p, const char *d, int l);
int qtk_pipe_connect(qtk_pipe_t *p, qtk_event_module_t *em, qtk_parser_t *ps,
                     const char *path, size_t len);
void qtk_pipe_kill(qtk_pipe_t *p, int sig);

#endif
