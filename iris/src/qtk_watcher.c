#include "qtk_watcher.h"
#include "qtk/sframe/qtk_sframe.h"
#include "wtk/core/wtk_strbuf.h"


qtk_watcher_node_t* qtk_watcher_node_new(qtk_watcher_t *w, qtk_watch_handler_t handler, void* data) {
    qtk_watcher_node_t* wn = wtk_malloc(sizeof(*wn));
    wn->watcher = w;
    wn->handler = handler;
    wn->data = data;
    wtk_queue_node_init(&wn->node);
    wtk_queue_node_init(&wn->cnode);
    wn->buildin = 0;
    wn->unique = 0;
    return wn;
}


int qtk_watcher_node_delete(qtk_watcher_node_t *wn) {
    wtk_free(wn);
    return 0;
}


qtk_watcher_t *qtk_watcher_new(wtk_string_t *topic) {
    qtk_watcher_t *watcher;
    wtk_string_t *key;
    char *p;
    watcher = wtk_malloc(sizeof(*watcher) + topic->len);
    p = (char*)(watcher+1);
    wtk_queue_node_init(&watcher->node.n);
    key = &watcher->node.key._str;
    memcpy(p, topic->data, topic->len);
    wtk_string_set(key, p, topic->len);
    wtk_queue_init(&watcher->notifier_q);
    return watcher;
}


int qtk_watcher_add(qtk_watcher_t *watcher, qtk_watcher_node_t *wn) {
    return wtk_queue_push(&watcher->notifier_q, &wn->node);
}


/* qtk_watcher_node_t* qtk_watcher_add_local(qtk_watcher_t *watcher, */
/*                                           qtk_watch_handler_t handler, */
/*                                           void* data) { */
/*     qtk_watcher_node_t *wn; */
/*     wn = qtk_watcher_node_new(watcher, handler, data); */
/*     if (qtk_watcher_add(watcher, wn)) { */
/*         qtk_watcher_node_delete(wn); */
/*         wn = NULL; */
/*     } */
/*     return wn; */
/* } */

qtk_watcher_node_t* qtk_watcher_find(qtk_watcher_t *watcher, qtk_watch_handler_t handler, void *data) {
    wtk_queue_node_t *qn;
    qtk_watcher_node_t *wn = NULL;
    for (qn = watcher->notifier_q.pop; qn; qn = qn->next) {
        wn = data_offset(qn, qtk_watcher_node_t, node);
        if (wn->handler == handler && wn->data == data) {
            break;
        }
    }
    return wn;
}


qtk_watcher_node_t* qtk_watcher_get_uniq(qtk_watcher_t *watcher) {
    wtk_queue_node_t *n, *p;
    qtk_watcher_node_t *data;

    for (n=watcher->notifier_q.pop; n; n=p) {
        p = n->next;
        data = data_offset(n, qtk_watcher_node_t, node);
        if (data->unique) {
            return data;
        }
    }
    return NULL;
}


int qtk_watcher_delnode(qtk_watcher_t *watcher, qtk_watch_handler_t handler, void *data) {
    qtk_watcher_node_t *wn = qtk_watcher_find(watcher, handler, data);

    if (wn) {
        if (wn->buildin) {
            wtk_debug("cannot remove buildin watcher\r\n");
            return -1;
        }
        qtk_watcher_remove(watcher, wn);
        return 0;
    } else {
        return -1;
    }
}


int qtk_watcher_handle(qtk_watcher_t *watcher, qtk_sframe_msg_t *msg) {
    wtk_queue_node_t *qn;
    qtk_watcher_node_t *wn;
    int unique_cnt = 0;

    for (qn = watcher->notifier_q.pop; qn; qn = qn->next) {
        wn = data_offset(qn, qtk_watcher_node_t, node);
        if (wn->unique && unique_cnt++) continue;
        /* if msg is forwarded, dont call handle behind */
        if (1 == wn->handler(wn->data, msg)) return 1;
    }

    return 0;
}


int qtk_watcher_clean(qtk_watcher_t *watcher) {
    wtk_queue_node_t *qn;
    qtk_watcher_node_t *wn;
    while ((qn = wtk_queue_pop(&watcher->notifier_q))) {
        wn = data_offset(qn, qtk_watcher_node_t, node);
        if (!wn->buildin) {
            qtk_watcher_node_delete(wn);
        }
    }
    return 0;
}


int qtk_watcher_delete(qtk_watcher_t *watcher) {
    qtk_watcher_clean(watcher);
    wtk_free(watcher);
    return 0;
}


int qtk_watcher_remove(qtk_watcher_t *watcher, qtk_watcher_node_t *wn) {
    wtk_queue_remove(&watcher->notifier_q, &wn->node);
    qtk_watcher_node_delete(wn);
    return 0;
}
