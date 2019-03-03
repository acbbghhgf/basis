#ifndef QTK_DLG_MAIN_H_
#define QTK_DLG_MAIN_H_

#include "qtk_dlg_worker.h"
#include "qtk_dlg_cfg.h"
#include "qtk/sframe/qtk_sframe.h"

typedef struct qtk_dlg qtk_dlg_t;
typedef struct qtk_dlg_event qtk_dlg_event_t;

struct qtk_dlg {
    qtk_dlg_worker_t *worker;
    qtk_dlg_cfg_t *cfg;
    qtk_sframe_method_t *gate;
    qtk_thread_t route;
    unsigned run : 1;
};

struct qtk_dlg_event {
    union {
        wtk_string_t data;
        int sig;
    } d;
    wtk_queue_node_t node;
    int id;
    unsigned is_sig : 1;
};

qtk_dlg_event_t *qtk_dlg_event_new_proto(const char *data, int len);
qtk_dlg_event_t *qtk_dlg_event_new_sig(int sig);
void qtk_dlg_event_delete(qtk_dlg_event_t *ev);

#endif
