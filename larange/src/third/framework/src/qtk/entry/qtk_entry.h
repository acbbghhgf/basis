#ifndef _QTK_ACCESS_ENTRY_QTK_ENTRY_H
#define _QTK_ACCESS_ENTRY_QTK_ENTRY_H
#include "qtk/entry/qtk_entry_cfg.h"
#include "qtk/event/qtk_event.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/os/wtk_pipequeue.h"
#include "wtk/os/wtk_blockqueue.h"
#include "wtk/core/wtk_hoard.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/os/wtk_lock.h"
#include "qtk/event/qtk_event_timer.h"
#include "wtk/os/wtk_log.h"
#include "qtk/http/qtk_request.h"
#include "qtk/entry/qtk_socket.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/frame/qtk_frame.h"
#ifdef __cplusplus
extern "C" {
#endif

#define qtk_entry_event_set_cb(evt, callback) (evt->cb = callback)

struct qtk_fwd;
struct qtk_request_pool;

typedef struct qtk_entry qtk_entry_t;
typedef struct qtk_entry_event qtk_entry_event_t;
typedef int (*qtk_loc_process_f)(void* userdata, qtk_response_t *rep);

typedef struct qtk_loc_meta_data {
    wtk_string_t key;
    qtk_loc_process_f process;
    void* udata;
} qtk_loc_meta_data_t;


struct qtk_entry {
    qtk_entry_cfg_t *cfg;
    wtk_log_t *log;
    wtk_blockqueue_t *output_q;        /* entry to uplayer */
    wtk_blockqueue_t cmd_q;

    wtk_thread_t cmd_thread;

    wtk_heap_t *heap;
    wtk_str_hash_t *loc_hash;
    qtk_socket_t *slot;
    int slot_cap;
    int alloc_id;

    /* for request_pool */
    struct qtk_request_pool *pool_slot;
    int pool_cap;
    int alloc_pid;

    int nworker;

    wtk_array_t *workers;
    struct qtk_fwd *frame;

    unsigned run:1;
};


qtk_entry_t *qtk_entry_new(qtk_entry_cfg_t *cfg, struct qtk_fwd *frame, wtk_log_t *log);
int qtk_entry_init(qtk_entry_t *entry);
int qtk_entry_delete(qtk_entry_t *entry);
int qtk_entry_start(qtk_entry_t *entry);
int qtk_entry_stop(qtk_entry_t *entry);
int qtk_entry_bytes(qtk_entry_t *entry);
int qtk_entry_loc_dispatch(qtk_entry_t *entry,
                           const char *key, int keybytes,
                           qtk_response_t *rep);

/* this function is not thread-safe */
int qtk_entry_do_statistic(qtk_entry_t *entry, qtk_response_t *rep);
int qtk_entry_init_loc(qtk_entry_t *entry, qtk_loc_meta_data_t *locs, int sz);
int qtk_entry_push_msg(qtk_entry_t *entry, qtk_sframe_msg_t *msg);
int qtk_entry_alloc_id(qtk_entry_t *entry, int pid);
int qtk_entry_alloc_pool(qtk_entry_t *entry);
qtk_socket_t *qtk_entry_get_socket(qtk_entry_t *entry, int id);
struct qtk_request_pool* qtk_entry_get_request_pool(qtk_entry_t *entry, int id);
int qtk_entry_raise_msg(qtk_entry_t *entry, qtk_sframe_msg_t *msg);

#ifdef __cplusplus
};
#endif
#endif
