#include <dlfcn.h>
#include <malloc.h>
#include <assert.h>
#include <ctype.h>
#include "qtk_frame.h"
#include "wtk/os/wtk_pipeproc.h"
#include "qtk_mail.h"
#include "string.h"
#include "qtk/util/qtk_mail.h"
#include "qtk/entry/qtk_entry.h"
#include "qtk/entry/qtk_socket_server.h"
#include "qtk/entry/qtk_request_pool.h"
#include "qtk/core/qtk_string.h"
#include "qtk/mqtt/qtk_mqtt_buffer.h"
#include "qtk/mqtt/mqtt.h"
#include "qtk/core/qtk_deque.h"
#include "qtk/websocket/qtk_ws_protobuf.h"


static qtk_fwd_t *fwd;

static char entry_name[] = "entry";
static char uplayer_name[] = "uplayer";

static qtk_loc_meta_data_t stat_locs[] = {
    {wtk_string("entry"), (qtk_loc_process_f)qtk_entry_do_statistic, entry_name},
};

static qtk_log_t* qtk_frame_get_base_log(qtk_fwd_t *frame);
static void* qtk_frame_get_uplayer_cfg(qtk_fwd_t *frame);
static int qtk_frame_alloc_socket(qtk_fwd_t *frame);
static int qtk_frame_alloc_request_pool(qtk_fwd_t *frame);
static wtk_string_t* qtk_frame_get_local_addr(qtk_fwd_t *frame, qtk_sframe_msg_t *msg);
static wtk_string_t* qtk_frame_get_remote_addr(qtk_fwd_t *frame, qtk_sframe_msg_t *msg);
static qtk_sframe_msg_t* qtk_frame_pop_msg(qtk_fwd_t *frame, int timeout);
static int qtk_frame_push_msg(qtk_fwd_t *frame, qtk_sframe_msg_t *msg);
static qtk_sframe_msg_t *qtk_frame_msg_new(qtk_fwd_t *frame, int type, int id);
static int qtk_frame_msg_delete(qtk_fwd_t *frame, qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_type(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_signal(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_method(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_status(qtk_sframe_msg_t *msg);
static wtk_string_t* qtk_frame_msg_get_url(qtk_sframe_msg_t *msg);
static wtk_string_t* qtk_frame_msg_get_header(qtk_sframe_msg_t *msg, const char *key);
static wtk_strbuf_t* qtk_frame_msg_get_body(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_req_method(qtk_sframe_msg_t *msg);
static wtk_string_t* qtk_frame_msg_get_req_url(qtk_sframe_msg_t *msg);
static wtk_string_t* qtk_frame_msg_get_req_header(qtk_sframe_msg_t *msg, const char *key);
static wtk_strbuf_t* qtk_frame_msg_get_req_body(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_id(qtk_sframe_msg_t *msg);
static int qtk_frame_msg_get_mqtt(qtk_sframe_msg_t *msg, MQTT_Package_t *mqtt);
static qpb_variant_t* qtk_frame_msg_get_pbvar(qtk_sframe_msg_t *msg);
static struct lws* qtk_frame_msg_get_ws(qtk_sframe_msg_t *msg);
static void qtk_frame_msg_set_body(qtk_sframe_msg_t *msg, wtk_strbuf_t *body);
static void qtk_frame_msg_add_body(qtk_sframe_msg_t *msg, const char *data, int len);
static void qtk_frame_msg_add_header(qtk_sframe_msg_t *msg, const char *key, const char *val);
static void qtk_frame_msg_set_signal(qtk_sframe_msg_t *msg, int sig);
static void qtk_frame_msg_set_method(qtk_sframe_msg_t *msg, int meth);
static void qtk_frame_msg_set_status(qtk_sframe_msg_t *msg, int sc);
static void qtk_frame_msg_set_url(qtk_sframe_msg_t *msg, const char *url, int len);
static void qtk_frame_msg_set_cmd(qtk_sframe_msg_t *msg, int cmd, void *param);
static int qtk_frame_msg_set_mqtt(qtk_sframe_msg_t *msg, MQTT_Package_t *mqtt);
static int qtk_frame_msg_set_protobuf(qtk_sframe_msg_t *msg, qpb_variant_t *v, struct lws *wsi);


static int qtk_fwd_init_entry_loc(qtk_fwd_t *h) {
    int i;
    int n = sizeof(stat_locs)/sizeof(stat_locs[0]);
    for (i = 0; i < n; ++i) {
        char *name = stat_locs[i].udata;
        if (NULL == name) continue;
        if (strcmp(name, entry_name)) {
            stat_locs[i].udata = h->entry;
        } else if (strcmp(name, uplayer_name)) {
            stat_locs[i].udata = &h->uplayer;
        }
    }
    return qtk_entry_init_loc(h->entry, stat_locs, n);
}

static int qtk_malloc_trim(qtk_fwd_t *h){
    qtk_fwd_cfg_t *cfg;
    cfg = &h->cf->framework;
    qtk_log_log(h->cf->log,"malloc_trim pid=%#x.",getpid());
    malloc_trim(0);
    qtk_event_timer_add(h->event_module->timer, cfg->malloc_trim_time,
                        (qtk_event_timer_handler)qtk_malloc_trim, h);
    return 0;
}

static int qtk_fwd_static(qtk_fwd_t *h) {
    qtk_fwd_cfg_t *cfg;

    cfg = &h->cf->framework;
    qtk_event_timer_add(h->event_module->timer, cfg->statis_time,
                        (qtk_event_timer_handler)qtk_fwd_static, h);
    return 0;
}


void* qtk_fwd_g_add_timer(int ex_time, qtk_event_timer_handler handler, void *data) {
    assert(fwd);
    return qtk_event_timer_add(fwd->event_module->timer, ex_time, handler, data);
}


int qtk_fwd_g_remove_timer(void *handle) {
    assert(fwd);
    return qtk_event_timer_remove(fwd->event_module->timer, handle);
}


double qtk_fwd_g_get_time() {
    return fwd ? fwd->event_module->timer->time.wtk_cached_time : 0;
}


char* qtk_fwd_g_get_pidfn(qtk_conf_file_t *cfg)
{
    static char fn[2048];
    wtk_string_t *pid_fn = &cfg->framework.pid_fn;
    int n;

    n = wtk_proc_get_dir(fn,sizeof(fn));
    fn[n] = '/';
    fn[n+1] = '\0';
    strncat(fn, pid_fn->data, pid_fn->len);
    return fn;
}

void qtk_fwd_save_pidfile(qtk_fwd_t *s)
{
    char *fn;

    fn=qtk_fwd_g_get_pidfn(s->cf);
    pid_save_file(s->master_pid,fn);
}

void qtk_fwd_delete_pidfile(qtk_fwd_t *s)
{
    char *fn;

    fn=qtk_fwd_g_get_pidfn(s->cf);
    pid_delete_file(s->master_pid,fn);
}

int qtk_fwd_stop_proc(qtk_conf_file_t *cfg)
{
    char *fn;
    int n;
    char *p;
    int id;//,status;
    int i;

    fn=qtk_fwd_g_get_pidfn(cfg);
    if(wtk_file_exist(fn)==0)
    {
        p=file_read_buf(fn,&n);
        id=wtk_str_atoi(p,n);
        kill(id,SIGQUIT);
        for (i = 0; i < 100; i++) {
            usleep(100*1000);
            if (kill(id, 0)) break;
        }
        if (i == 100) {
            wtk_debug("stop proc timeout.\r\n");
        }
    }else
    {
        printf("info: %s not exist.\n",fn);
    }
    exit(0);
    return 0;
}

int qtk_fwd_is_running(qtk_conf_file_t *cfg)
{
    char *fn;
    int n;
    char *p;
    int id;//,status;

    fn=qtk_fwd_g_get_pidfn(cfg);
    if(wtk_file_exist(fn)==0)
    {
        p=file_read_buf(fn,&n);
        id=wtk_str_atoi(p,n);
        free(p);
        if(id>0)
        {
            if(kill(id,0)==0)
            {
                return 1;
            }
        }
    }
    return 0;
}


static void qtk_fwd_on_signal(int sig)
{
    switch(sig)
    {
        /* case SIGSEGV: */
        /*  wtk_proc_dump_stack(); */
        /*     qtk_fwd_exit(); */
        /*  break; */
    case SIGINT:
        if(fwd->master_pid>0)
        {
            kill(0,SIGKILL);
        }
        qtk_fwd_set_stop();
        break;
    case SIGQUIT:
        if (fwd->master_pid == getpid()) {
            kill(fwd->worker_pid,SIGQUIT);
            waitpid(fwd->worker_pid, NULL, 0);
        }
        qtk_fwd_set_stop();
        break;
    case SIGUSR1:
        wtk_debug("wait ...\n");
        break;
    case SIGPIPE:
        //dump_stack();
        break;
    default:
        wtk_debug("%d: receive %d signal.\n",getpid(),sig);
        break;
    }
}

static int qtk_fwd_setup_signal()
{
    /* int signals[]={SIGSEGV,SIGPIPE,SIGQUIT,SIGINT,SIGUSR1}; */
    int signals[]={SIGPIPE,SIGQUIT,SIGINT,SIGUSR1};
    int i,count;

    count=sizeof(signals)/sizeof(int);
    for(i=0;i<count;++i)
    {
        signal(signals[i],qtk_fwd_on_signal);
    }
    return 0;
}


static void qtk_fwd_init_method(qtk_fwd_t *frame) {
    qtk_sframe_method_t *method = &frame->method;
    method->get_log = (qtk_sframe_get_log_f)qtk_frame_get_base_log;
    method->get_cfg = (qtk_sframe_get_cfg_f)qtk_frame_get_uplayer_cfg;
    method->socket = (qtk_sframe_socket_f)qtk_frame_alloc_socket;
    method->request_pool = (qtk_sframe_socket_f)qtk_frame_alloc_request_pool;
    method->get_local_addr = (qtk_sframe_address_f)qtk_frame_get_local_addr;
    method->get_remote_addr = (qtk_sframe_address_f)qtk_frame_get_remote_addr;
    method->pop = (qtk_sframe_msg_pop_f)qtk_frame_pop_msg;
    method->push = (qtk_sframe_msg_push_f)qtk_frame_push_msg;
    method->new = (qtk_sframe_msg_new_f)qtk_frame_msg_new;
    method->delete = (qtk_sframe_msg_delete_f)qtk_frame_msg_delete;
    method->get_type = (qtk_sframe_msg_get_type_f)qtk_frame_msg_get_type;
    method->get_signal = (qtk_sframe_msg_get_signal_f)qtk_frame_msg_get_signal;
    method->get_method = (qtk_sframe_msg_get_method_f)qtk_frame_msg_get_method;
    method->get_url = (qtk_sframe_msg_get_url_f)qtk_frame_msg_get_url;
    method->req_method = (qtk_sframe_msg_get_method_f)qtk_frame_msg_get_req_method;
    method->req_url = (qtk_sframe_msg_get_url_f)qtk_frame_msg_get_req_url;
    method->req_body = (qtk_sframe_msg_proto_body_find_f)qtk_frame_msg_get_req_body;
    method->req_header = (qtk_sframe_msg_proto_field_value_find_f)qtk_frame_msg_get_req_header;
    method->get_status = (qtk_sframe_msg_get_status_code_f)qtk_frame_msg_get_status;
    method->get_id = (qtk_sframe_msg_get_id_f)qtk_frame_msg_get_id;
    method->set_signal = (qtk_sframe_msg_add_notice_signal_f)qtk_frame_msg_set_signal;
    method->set_body = (qtk_sframe_msg_proto_set_body_f)qtk_frame_msg_set_body;
    method->get_body = (qtk_sframe_msg_proto_body_find_f)qtk_frame_msg_get_body;
    method->get_header = (qtk_sframe_msg_proto_field_value_find_f)qtk_frame_msg_get_header;
    method->add_body = (qtk_sframe_msg_proto_add_body_f)qtk_frame_msg_add_body;
    method->add_header = (qtk_sframe_msg_proto_add_field_f)qtk_frame_msg_add_header;
    method->set_method = (qtk_sframe_msg_set_method_f)qtk_frame_msg_set_method;
    method->set_url = (qtk_sframe_msg_set_url_f)qtk_frame_msg_set_url;
    method->set_status = (qtk_sframe_msg_set_status_f)qtk_frame_msg_set_status;
    method->set_cmd = (qtk_sframe_msg_set_cmd_f)qtk_frame_msg_set_cmd;
    method->get_mqtt = (qtk_sframe_msg_get_mqtt_f)qtk_frame_msg_get_mqtt;
    method->set_mqtt = (qtk_sframe_msg_set_mqtt_f)qtk_frame_msg_set_mqtt;
    method->get_pb_var = (qtk_sframe_msg_get_pbvar_f)qtk_frame_msg_get_pbvar;
    method->get_ws = (qtk_sframe_msg_get_ws_f)qtk_frame_msg_get_ws;
    method->set_protobuf = qtk_frame_msg_set_protobuf;
}


int qtk_fwd_init(qtk_fwd_t *h,qtk_conf_file_t *cf)
{
    qtk_fwd_cfg_t *cfg;
    qtk_log_t *log = cf->log;
    int ret = -1;

    cfg=&(cf->framework);
    fwd=h;
    if(qtk_fwd_is_running(cf)) {
        printf("process is already running.\nexit...\n");
        exit(0);
    }
    qtk_fwd_setup_signal();
    h->worker_pid=h->master_pid=0;
    h->cf=cf;
    h->event_module = qtk_event_module_new(&(cfg->event));
    wtk_blockqueue_init(&h->entry_to_uplayer);
    wtk_pipequeue_init(&h->uplayer_to_entry);
    if (NULL == h->event_module) {
        goto end;
    }
    h->entry = qtk_entry_new(&cfg->entry, h, log);
    if (NULL == h->entry) goto end;
    qtk_fwd_init_entry_loc(h);
    qtk_fwd_init_method(h);
    if (cfg->send_mail) {
        qtk_mail_init(cfg->mail_buf);
    }
    ret = 0;
end:
    return ret;
}


int qtk_fwd_clean_env() {
    if (fwd->entry) {
        qtk_entry_delete(fwd->entry);
        fwd->entry = NULL;
    }
    qtk_event_module_delete(fwd->event_module);
    if (fwd->cf->framework.send_mail) {
        qtk_mail_clean();
    }
    return 0;
}

int qtk_fwd_clean(qtk_fwd_t *s)
{
    qtk_fwd_clean_env();
    if (fwd->master_pid) {
        kill(0, SIGKILL);
    }

    return 0;
}

int qtk_fwd_start(qtk_fwd_t *s)
{
    if (qtk_uplayer_init(&s->uplayer, s)) return -1;
    qtk_entry_start(s->entry);
    qtk_uplayer_start(&s->uplayer);
    qtk_fwd_static(s);
    qtk_malloc_trim(s);
    return 0;
}


int qtk_fwd_stop(qtk_fwd_t *s) {
    wtk_debug("waiting for exit...\r\n");
    qtk_uplayer_stop(&s->uplayer);
    /* wake msg queue for uplayer thread exit */
    wtk_blockqueue_wake(&s->entry_to_uplayer);
    qtk_entry_stop(s->entry);
    qtk_uplayer_clean(&fwd->uplayer);
    wtk_debug("entry stopped\r\n");
    return 0;
}


int qtk_fwd_run_proc(qtk_fwd_t* s)
{
    int ret;

    ret=qtk_fwd_start(s);
    if(ret!=0){goto end;}
    s->run=1;
    while(s->run)
    {
        qtk_event_module_process(s->event_module, 10, 0);
        wtk_time_update(s->cf->log->t);
    }
    wtk_debug("start stop server ...\n");
    qtk_fwd_stop(s);
    wtk_debug("end server ...\n");
end:
    return ret;
}

int qtk_fwd_run_daemon(qtk_fwd_t *s)
{
    int  count = 0;
    pid_t pid;
    pid_t wpid;
    int status;
    //int ret;

    pid_daemonize();
    setpgid(0,0);
    s->master_pid=getpid();
    qtk_fwd_save_pidfile(s);
    s->run = 1;
    while (s->run) {
        pid = fork();
        if (pid == 0) {
            qtk_log_log(s->cf->log,"fork pid=%#x,count=%d.",getpid(),count);
            qtk_fwd_run_proc(s);
            break;
        } else if (pid > 0) {
            if (s->cf->framework.send_mail) {
                char mail_content[256];
                int n;
                n = snprintf(mail_content, sizeof(mail_content), "%s started",
                             s->cf->framework.uplayer_name);
                qtk_mail_send(mail_content, n);
            }
            s->worker_pid=pid;
            wpid=waitpid(pid, &status, 0);
            if (wpid==-1) {
                perror(__FUNCTION__);
            }
            ++count;
            if (s->cf->framework.send_mail) {
                char mail_content[256];
                int n;
                n = snprintf(mail_content, sizeof(mail_content), "%s started",
                             s->cf->framework.uplayer_name);
                qtk_mail_send(mail_content, n);
            }
        }
    }
    qtk_fwd_delete_pidfile(s);
    return 0;
}

int qtk_fwd_run(qtk_fwd_t *s)
{
    if(s->cf->framework.daemon)
    {
        return qtk_fwd_run_daemon(s);
    }else
    {
        return qtk_fwd_run_proc(s);
    }
}


int qtk_fwd_set_stop() {
    int pid;
    pid=getpid();
    if(pid==fwd->master_pid)
    {
        qtk_fwd_delete_pidfile(fwd);
        //kill(0,SIGKILL);
    }
    fwd->run = 0;
    return 0;
}


static void* _get_api(qtk_sframe_module_t *uplayer, const char *api_name) {
    void* ret;
    size_t name_size = strlen(uplayer->name);
    size_t api_size = strlen(api_name);
    char tmp[name_size + api_size + 1];
    memcpy(tmp, uplayer->name, name_size);
    memcpy(tmp+name_size, api_name, api_size+1);
    char *p = strrchr(tmp, '/');
    if (p) {
        ++p;
    } else {
        p = tmp;
    }
    ret = dlsym(uplayer->handle, p);
    if (NULL == ret) {
        wtk_debug("cannot find symbol '%s' in '%s'\r\n", tmp, uplayer->name);
    }
    return ret;
}


int qtk_uplayer_init(qtk_sframe_module_t *uplayer, qtk_fwd_t *frame) {
    int ret = -1;
    const char *name = frame->cf->framework.uplayer_name;
    size_t name_size = name ? strlen(name) : 0;
    size_t sz = name_size + 6;
    char tmp[sz];
    int ld_flags = RTLD_NOW | RTLD_GLOBAL;

    if (NULL == name) {
        wtk_debug("lost name field in uplayer's cfg\r\n");
        goto end;
    }

    snprintf(tmp, sizeof(tmp), "./%s.so", name);
    if (frame->cf->framework.ld_deepbind) {
        ld_flags |= RTLD_DEEPBIND;
    }
    uplayer->handle = dlopen(tmp, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if (NULL == uplayer->handle) {
        wtk_debug("try open %s failed : %s\r\n", name, dlerror());
        goto end;
    }
    uplayer->name = name;
    uplayer->new = _get_api(uplayer, "_new");
    uplayer->start = _get_api(uplayer, "_start");
    uplayer->stop = _get_api(uplayer, "_stop");
    uplayer->delete = _get_api(uplayer, "_delete");
    uplayer->module = uplayer->new((qtk_sframe_method_t *)frame);
    if (NULL == uplayer->module) {
        wtk_debug("uplayer new failed\r\n");
        goto end;
    }
    ret = 0;
end:
    return ret;
}


int qtk_uplayer_start(qtk_sframe_module_t *uplayer) {
    if (uplayer->module && uplayer->start) {
        return uplayer->start(uplayer->module);
    } else {
        wtk_debug("uplayer module not init\r\n");
        return -1;
    }
}


int qtk_uplayer_stop(qtk_sframe_module_t *uplayer) {
    if (uplayer->module && uplayer->stop) {
        return uplayer->stop(uplayer->module);
    } else {
        wtk_debug("uplayer module not init\r\n");
        return -1;
    }
}


int qtk_uplayer_clean(qtk_sframe_module_t *uplayer) {
    if (uplayer->module && uplayer->delete) {
        return uplayer->delete(uplayer->module);
    } else {
        wtk_debug("uplayer module not init\r\n");
        return -1;
    }
}


static qtk_log_t* qtk_frame_get_base_log(qtk_fwd_t *frame) {
    return frame->cf->log;
}


static void *qtk_frame_get_uplayer_cfg(qtk_fwd_t *frame) {
    return frame->cf->framework.uplayer;
}


static int qtk_frame_alloc_socket(qtk_fwd_t *frame) {
    return qtk_entry_alloc_id(frame->entry, 0);
}


static int qtk_frame_alloc_request_pool(qtk_fwd_t *frame) {
    return qtk_entry_alloc_pool(frame->entry);
}


static wtk_string_t* qtk_frame_get_local_addr(qtk_fwd_t *frame, qtk_sframe_msg_t *msg) {
    qtk_socket_t *s = qtk_entry_get_socket(frame->entry, msg->id);
    if (s) {
        return s->local_addr;
    } else {
        return NULL;
    }
}


static wtk_string_t* qtk_frame_get_remote_addr(qtk_fwd_t *frame, qtk_sframe_msg_t *msg) {
    if (msg->id < 0) {
        qtk_request_pool_t *pool = qtk_entry_get_request_pool(frame->entry, -msg->id);
        return pool->remote_addr;
    }
    qtk_socket_t *s = qtk_entry_get_socket(frame->entry, msg->id);
    if (s) {
        return s->remote_addr;
    } else {
        return NULL;
    }
}


static qtk_sframe_msg_t* qtk_frame_pop_msg(qtk_fwd_t *frame, int timeout) {
    wtk_queue_node_t* node = wtk_blockqueue_pop(&frame->entry_to_uplayer, timeout, NULL);
    return data_offset(node, qtk_sframe_msg_t, q_n);
}


static int qtk_frame_push_msg(qtk_fwd_t *frame, qtk_sframe_msg_t *msg) {
    return qtk_entry_push_msg(frame->entry, msg);
}


static qtk_sframe_msg_t *qtk_frame_msg_new_base(qtk_fwd_t *frame, int type, int id) {
    qtk_sframe_msg_t *msg = wtk_malloc(sizeof(*msg));
    wtk_queue_node_init(&msg->q_n);
    msg->id = id;
    msg->signal = 0;
    msg->type = type;
    memset(&msg->info, 0, sizeof(msg->info));
    return msg;
}


static qtk_sframe_msg_t *qtk_frame_msg_new(qtk_fwd_t *frame, int type, int id) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, type, id);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        msg->info.request = qtk_request_new();
        qtk_request_init(msg->info.request, id);
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        msg->info.response = qtk_response_new();
        qtk_response_init(msg->info.response, id);
    } else if (type == QTK_SFRAME_MSG_MQTT) {
        qtk_socket_t *s = qtk_entry_get_socket(frame->entry, id);
        msg->info.mqttbuf = qtk_mqttbuf_new();
        qtk_mqttbuf_init(msg->info.mqttbuf, s);
    } else if (type == QTK_SFRAME_MSG_PROTOBUF) {
        msg->info.protobuf = qtk_protobuf_new();
    }
    return msg;
}


static int qtk_frame_msg_delete(qtk_fwd_t *frame, qtk_sframe_msg_t *msg) {
    switch (msg->type) {
    case QTK_SFRAME_MSG_CMD:
        if (msg->info.lis_param) {
            wtk_free(msg->info.lis_param);
            msg->info.lis_param = NULL;
        }
        break;
    case QTK_SFRAME_MSG_REQUEST:
        if (msg->info.request) {
            if (!(msg->signal & QTK_SFRAME_SIG_DNOT_DEL_REQ)) {
                qtk_request_delete(msg->info.request);
            }
            msg->info.request = NULL;
        }
        break;
    case QTK_SFRAME_MSG_RESPONSE:
        if (msg->info.response) {
            qtk_response_delete(msg->info.response);
            msg->info.response = NULL;
        }
        break;
    case QTK_SFRAME_MSG_MQTT:
        if (msg->info.mqttbuf) {
            qtk_mqttbuf_delete(msg->info.mqttbuf);
            msg->info.mqttbuf = NULL;
        }
        break;
    case QTK_SFRAME_MSG_PROTOBUF:
        if (msg->info.protobuf) {
            qtk_protobuf_delete(msg->info.protobuf);
            msg->info.protobuf = NULL;
        }
        break;
    case QTK_SFRAME_MSG_JWT_PAYLOAD:
        if (msg->info.jwt_payload) {
            wtk_string_delete(msg->info.jwt_payload);
        }
    default:
        break;
    }
    wtk_free(msg);
    return 0;
}


static int qtk_frame_msg_get_type(qtk_sframe_msg_t *msg) {return msg->type;}
static int qtk_frame_msg_get_signal(qtk_sframe_msg_t *msg) {return msg->signal;}


static int qtk_frame_msg_get_method(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_REQUEST) {
        return msg->info.request->method;
    } else {
        return HTTP_UNKNOWN;
    }
}


static int qtk_frame_msg_get_status(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE) {
        return msg->info.response->status;
    } else {
        return 0;
    }
}


static int qtk_frame_msg_get_req_method(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE
        && msg->info.response->request) {
        return msg->info.response->request->method;
    } else {
        return HTTP_UNKNOWN;
    }
}


static wtk_string_t* qtk_frame_msg_get_req_url(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE
        && msg->info.response->request) {
        return &msg->info.response->request->url;
    } else {
        return NULL;
    }
}


static wtk_string_t* qtk_frame_msg_get_req_header(qtk_sframe_msg_t *msg, const char *key) {
    int klen = strlen(key);
    if (klen > 100) {
        wtk_debug("header %s is too long\r\n", key);
        return NULL;
    }
    char key2[klen];
    qtk_str_tolower2(key2, key, klen);
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE
        && msg->info.response->request) {
        return qtk_request_get_hdr(msg->info.response->request, key2, klen);
    }
    return NULL;
}


static wtk_strbuf_t* qtk_frame_msg_get_req_body(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE
        && msg->info.response->request) {
        return msg->info.response->request->body;
    }
    return NULL;
}


static wtk_string_t* qtk_frame_msg_get_url(qtk_sframe_msg_t *msg) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_REQUEST) {
        return &msg->info.request->url;
    } else {
        return NULL;
    }
}


static wtk_string_t* qtk_frame_msg_get_header(qtk_sframe_msg_t *msg, const char *key) {
    int klen = strlen(key);
    if (klen > 100) {
        wtk_debug("header %s is too long\r\n", key);
        return NULL;
    }
    char key2[klen];
    qtk_str_tolower2(key2, key, klen);
    int type = qtk_frame_msg_get_type(msg);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        return qtk_request_get_hdr(msg->info.request, key2, klen);
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        return qtk_response_get_hdr(msg->info.response, key2, klen);
    }
    return NULL;
}


static wtk_strbuf_t* qtk_frame_msg_get_body(qtk_sframe_msg_t *msg) {
    int type = qtk_frame_msg_get_type(msg);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        return msg->info.request->body;
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        return msg->info.response->body;
    }
    return NULL;
}


static int qtk_frame_msg_get_id(qtk_sframe_msg_t *msg) {return msg->id;}

static void qtk_frame_msg_set_body(qtk_sframe_msg_t *msg, wtk_strbuf_t *body) {
    int type = qtk_frame_msg_get_type(msg);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        if (body != msg->info.request->body) {
            if (msg->info.request->body) {
                wtk_strbuf_delete(msg->info.request->body);
            }
            msg->info.request->body = body;
        }
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        if (body != msg->info.response->body) {
            if (msg->info.response->body) {
                wtk_strbuf_delete(msg->info.response->body);
            }
            msg->info.response->body = body;
        }
    }
}


static void qtk_frame_msg_add_body(qtk_sframe_msg_t *msg, const char *data, int len) {
    int type = qtk_frame_msg_get_type(msg);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        qtk_request_set_body(msg->info.request, data, len);
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        qtk_response_set_body(msg->info.response, data, len);
    }
}


static void qtk_frame_msg_add_header(qtk_sframe_msg_t *msg, const char *key, const char *val) {
    int type = qtk_frame_msg_get_type(msg);
    if (type == QTK_SFRAME_MSG_REQUEST) {
        qtk_request_add_hdr(msg->info.request, (char*)key, strlen(key),
                            wtk_heap_dup_string(msg->info.request->heap, (char*)val, strlen(val)));
    } else if (type == QTK_SFRAME_MSG_RESPONSE) {
        qtk_response_add_hdr(msg->info.response, (char*)key, strlen(key),
                             wtk_heap_dup_string(msg->info.response->heap, (char*)val, strlen(val)));
    }
}


static void qtk_frame_msg_set_method(qtk_sframe_msg_t *msg, int meth) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_REQUEST) {
        msg->info.request->method = meth;
    }
}


static void qtk_frame_msg_set_signal(qtk_sframe_msg_t *msg, int sig) { msg->signal = sig; }


static void qtk_frame_msg_set_status(qtk_sframe_msg_t *msg, int sc) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_RESPONSE) {
        msg->info.response->status = sc;
    }
}


static void qtk_frame_msg_set_url(qtk_sframe_msg_t *msg, const char *url, int len) {
    if (qtk_frame_msg_get_type(msg) == QTK_SFRAME_MSG_REQUEST) {
        qtk_request_t *req = msg->info.request;
        char *p = wtk_heap_malloc(req->heap, len);
        memcpy(p, url, len);
        wtk_string_set(&req->url, p, len);
    }
}


static void qtk_frame_msg_set_cmd(qtk_sframe_msg_t *msg, int cmd, void *param) {
    if (qtk_frame_msg_get_type(msg)== QTK_SFRAME_MSG_CMD) {
        qtk_frame_msg_set_signal(msg, cmd);
        switch (cmd) {
        case QTK_SFRAME_CMD_OPEN:
        case QTK_SFRAME_CMD_OPEN_POOL:
            msg->info.con_param = param;
            break;
        case QTK_SFRAME_CMD_LISTEN:
            msg->info.lis_param = param;
            break;
        default:
            break;
        }
    }
}


static qpb_variant_t *qtk_frame_msg_get_pbvar(qtk_sframe_msg_t *msg) {
    qpb_variant_t *v = NULL;
    if (QTK_SFRAME_MSG_PROTOBUF == qtk_frame_msg_get_type(msg)) {
        v = msg->info.protobuf->v;
    }
    return v;
}


static struct lws *qtk_frame_msg_get_ws(qtk_sframe_msg_t *msg) {
    struct lws *wsi = NULL;
    if (QTK_SFRAME_MSG_PROTOBUF == qtk_frame_msg_get_type(msg)) {
        wsi = msg->info.protobuf->wsi;
    }
    return wsi;
}


static int qtk_frame_msg_set_protobuf(qtk_sframe_msg_t *msg, qpb_variant_t *v, struct lws *wsi) {
    int ret = -1;
    if (QTK_SFRAME_MSG_PROTOBUF == qtk_frame_msg_get_type(msg)) {
        msg->info.protobuf->v = v;
        msg->info.protobuf->wsi = wsi;
        ret = 0;
    }
    return ret;
}


static int qtk_frame_msg_get_mqtt(qtk_sframe_msg_t *msg, MQTT_Package_t *mqtt) {
    if (QTK_SFRAME_MSG_MQTT == qtk_frame_msg_get_type(msg)) {
        qtk_mqtt_buffer_t *mbuf = msg->info.mqttbuf;
        mqtt->type = mbuf->headbyte >> 4;
        mqtt->flag = mbuf->headbyte & 0x0f;
        mqtt->length = mbuf->length;
        if (mqtt_unpack_var((const uint8_t*)mbuf->buf->data, mbuf->buf->pos, mqtt->type, mqtt) < 0) {
            return -1;
        }
        return 0;
    } else {
        return -1;
    }
}


static int qtk_frame_msg_set_mqtt(qtk_sframe_msg_t *msg, MQTT_Package_t *mqtt) {
    int ret = -1;
    if (QTK_SFRAME_MSG_MQTT == qtk_frame_msg_get_type(msg)) {
        qtk_mqtt_buffer_t *mbuf = msg->info.mqttbuf;
        qtk_deque_t *dbuf = qtk_deque_new(1024, 10240, 1);
        if (NULL == mbuf) {
            mbuf = qtk_mqttbuf_new();
            msg->info.mqttbuf = mbuf;
        }
        if (mqtt_pack_var(mqtt, mqtt->type, dbuf) >= 0) {
            mbuf->headbyte = mqtt->type;
            mbuf->headbyte = (mbuf->headbyte<<4)|mqtt->flag;
            if (dbuf->len) {
                mbuf->length = dbuf->len;
                qtk_mqttbuf_alloc_body(mbuf);
                qtk_deque_copy(dbuf, mbuf->buf);
            } else {
                mbuf->length = 0;
            }
            msg->info.mqttbuf = mbuf;
            ret = 0;
        }
        qtk_deque_delete(dbuf);
    }
    return ret;
}


qtk_sframe_msg_t *qtk_fwd_protobuf_msg(qtk_fwd_t *frame, qtk_protobuf_t *pb, int sock_id) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_PROTOBUF, sock_id);
    msg->info.protobuf = pb;
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_mqtt_msg(qtk_fwd_t *frame, qtk_mqtt_buffer_t *mbuf) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_MQTT, mbuf->s->id);
    msg->info.mqttbuf = mbuf;
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_request_msg(qtk_fwd_t *frame, qtk_request_t *request) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_REQUEST, request->sock_id);
    msg->info.request = request;
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_response_msg(qtk_fwd_t *frame, qtk_response_t *response) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_RESPONSE, response->sock_id);
    msg->info.response = response;
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_notice_msg(qtk_fwd_t *frame, int signal, int id) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_NOTICE, id);
    qtk_frame_msg_set_signal(msg, signal);
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_cmd_msg(qtk_fwd_t *frame, int cmd, int id) {
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_CMD, id);
    qtk_frame_msg_set_signal(msg, cmd);
    return msg;
}


qtk_sframe_msg_t *qtk_fwd_jwt_payload(qtk_fwd_t *frame, int id, const char *payload, size_t len)
{
    qtk_sframe_msg_t *msg = qtk_frame_msg_new_base(frame, QTK_SFRAME_MSG_JWT_PAYLOAD, id);
    msg->info.jwt_payload = wtk_string_dup_data_pad0((char *)payload, len);
    return msg;
}
