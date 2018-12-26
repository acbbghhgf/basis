#ifndef _QTK_CACHED_QTK_WATCHER_H
#define _QTK_CACHED_QTK_WATCHER_H
#include "wtk/core/wtk_str.h"
#include "qtk/core/qtk_abstruct_hashtable.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_sframe_msg;
struct qtk_sframe;

#define qtk_buildin_watcher(s, h, d) {.topic = wtk_string(s), \
    .watcher.node = {NULL, NULL},                             \
    .watcher.cnode = {NULL, NULL},                            \
    .watcher.handler = (qtk_watch_handler_t)h,                \
    .watcher.data = d,                                        \
    .watcher.buildin = 1}

typedef struct qtk_watcher qtk_watcher_t;
typedef struct qtk_watcher_node qtk_watcher_node_t;
typedef int (*qtk_watch_handler_t)(void*, struct qtk_sframe_msg *msg);


struct qtk_watcher_node {
    wtk_queue_node_t node;
    wtk_queue_node_t cnode;
    qtk_watch_handler_t handler;
    void *data;
    qtk_watcher_t *watcher;
    unsigned buildin:1;
    unsigned unique:1;
};


typedef struct {
    wtk_string_t topic;
    qtk_watcher_node_t watcher;
} qtk_buildin_watcher_t;


struct qtk_watcher {
    qtk_hashnode_t node;
    wtk_queue_t notifier_q;
};

qtk_watcher_node_t *qtk_watcher_node_new(qtk_watcher_t *watcher,
                                         qtk_watch_handler_t handler, void* data);
int qtk_watcher_node_delete(qtk_watcher_node_t *wn);
qtk_watcher_t *qtk_watcher_new(wtk_string_t *topic);
int qtk_watcher_add(qtk_watcher_t *watcher, qtk_watcher_node_t *wn);
qtk_watcher_node_t* qtk_watcher_find(qtk_watcher_t *watcher, qtk_watch_handler_t handler, void *data);
qtk_watcher_node_t* qtk_watcher_get_uniq(qtk_watcher_t *watcher);
int qtk_watcher_delnode(qtk_watcher_t *watcher, qtk_watch_handler_t handler, void* data);
int qtk_watcher_handle(qtk_watcher_t *watcher, struct qtk_sframe_msg *msg);
int qtk_watcher_clean(qtk_watcher_t *watcher);
int qtk_watcher_delete(qtk_watcher_t *watcher);
int qtk_watcher_remove(qtk_watcher_t *watcher, qtk_watcher_node_t *wn);


#ifdef __cplusplus
}
#endif

#endif
