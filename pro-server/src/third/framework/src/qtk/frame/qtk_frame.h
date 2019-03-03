#ifndef _QTK_FWD_QTK_FWD_H
#define _QTK_FWD_QTK_FWD_H
#include "qtk/conf/qtk_conf_file.h"
#include "wtk/os/wtk_blockqueue.h"
#include "wtk/os/wtk_thread.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "wtk/os/wtk_sem.h"
#include "wtk/os/wtk_proc.h"
#include "qtk/event/qtk_event.h"
#include "qtk/entry/qtk_entry.h"
#include "wtk/os/wtk_cpu.h"
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/mqtt/qtk_mqtt_buffer.h"
#include "qtk/websocket/qtk_ws_protobuf.h"

#ifdef __cplusplus
extern "C" {
#endif


struct qtk_entry;

typedef struct qtk_fwd qtk_fwd_t;


struct qtk_fwd {
    /* the method struct must located first */
    qtk_sframe_method_t method;
    qtk_conf_file_t *cf;
    qtk_event_module_t *event_module;
    wtk_blockqueue_t entry_to_uplayer;
    wtk_pipequeue_t uplayer_to_entry;
    volatile double time;
    struct qtk_entry *entry;
    qtk_sframe_module_t uplayer;

    pid_t worker_pid;
    pid_t master_pid;
    volatile unsigned run:1;
};

int qtk_fwd_init(qtk_fwd_t *h, qtk_conf_file_t *cfg);
int qtk_fwd_clean(qtk_fwd_t *h);
int qtk_fwd_run(qtk_fwd_t *s);
int qtk_fwd_stop_proc(qtk_conf_file_t *cfg);
int qtk_fwd_is_running(qtk_conf_file_t *cfg);
int qtk_fwd_set_stop();
void qtk_fwd_g_get_cpuinfo(wtk_strbuf_t *buf);
void qtk_fwd_g_get_meminfo(wtk_strbuf_t *buf);
void* qtk_fwd_g_add_timer(int ex_time, qtk_event_timer_handler handler, void *data);
int qtk_fwd_g_remove_timer(void *handle);
double qtk_fwd_g_get_time();
int qtk_fwd_clean_env();
qtk_sframe_msg_t *qtk_fwd_request_msg(qtk_fwd_t *frame, qtk_request_t *request);
qtk_sframe_msg_t *qtk_fwd_response_msg(qtk_fwd_t *frame, qtk_response_t *response);
qtk_sframe_msg_t *qtk_fwd_mqtt_msg(qtk_fwd_t *frame, qtk_mqtt_buffer_t *mqtt);
qtk_sframe_msg_t *qtk_fwd_notice_msg(qtk_fwd_t *frame, int signal, int id);
qtk_sframe_msg_t *qtk_fwd_jwt_payload(qtk_fwd_t *frame, int id, const char *payload, size_t len);
qtk_sframe_msg_t *qtk_fwd_cmd_msg(qtk_fwd_t *frame, int cmd, int id);
qtk_sframe_msg_t *qtk_fwd_protobuf_msg(qtk_fwd_t *frame, qtk_protobuf_t *pb, int sock_id);

int qtk_uplayer_init(qtk_sframe_module_t *uplayer, qtk_fwd_t *frame);
int qtk_uplayer_start(qtk_sframe_module_t *uplayer);
int qtk_uplayer_stop(qtk_sframe_module_t *uplayer);
int qtk_uplayer_clean(qtk_sframe_module_t *uplayer);


#ifdef __cplusplus
};
#endif
#endif
