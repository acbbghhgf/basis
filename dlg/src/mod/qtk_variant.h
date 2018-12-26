#ifndef _MOD_QTK_VARIANT_H_
#define _MOD_QTK_VARIANT_H_

#include "pipe/qtk_pipe.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_queue.h"

typedef struct qtk_variant qtk_variant_t;
struct qtk_variant {
    qtk_pipe_t p;
    wtk_string_t name;
    wtk_queue_node_t node;

    unsigned zygote   : 1;
    unsigned in_useq  : 1;
    unsigned in_freeq : 1;
    unsigned raw      : 1;
    unsigned used     : 1;
    unsigned closed   : 1;
};

qtk_variant_t *qtk_variant_new(pid_t pid, const char *name, size_t len);
void qtk_variant_delete(qtk_variant_t *v);
int qtk_variant_write_cmd(qtk_variant_t *v, int cmd, const char *param);
void qtk_pipe_set_close_notifier(qtk_pipe_t *p, qtk_pipe_handler f, void *ud);
int qtk_variant_prepare(qtk_variant_t *v, qtk_event_module_t *em, qtk_parser_t *ps,
                        const char *p, size_t l);
void qtk_variant_kill(qtk_variant_t *v, int sig);
int qtk_variant_exit(qtk_variant_t *v);

#endif
