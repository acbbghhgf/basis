#include <assert.h>
#include <sys/prctl.h>
#include <sys/wait.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <sys/un.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "wtk/core/wtk_str.h"
#include "wtk/os/wtk_lock.h"
#include "wtk/os/wtk_fd.h"
#include "wtk/os/wtk_pipeproc.h"
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "wtk/core/wtk_hash.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/param/wtk_module_param.h"
#include "wtk/core/param/param_serialize.h"
#include "qtk/audio/qtk_audio_decoder.h"
#include "qtk/audio/qtk_encode.h"
#include "wtk/os/wtk_fd.h"
//#include "qtk/util/qtk_fifo.h"
#include "qtk/core/qtk_atomic.h"
#include "qtk/zeus/qtk_zeus.h"
#include "qtk/zeus/qtk_zeus_server.h"
#include "qtk_aisrv_module.h"
#include "qtk_aisrv_cfg.h"


typedef struct qtk_aisrv qtk_aisrv_t;
typedef struct qtk_aisrv_instance qtk_aisrv_instance_t;


struct qtk_aisrv {
    qtk_aisrv_cfg_t *cfg;
    wtk_string_t *name;
    struct qtk_zcontext *ctx;
    wtk_pipeproc_t *proc;
    qtk_aisrv_mod_t *module;
    void *data;
    int inst_cnt;
};


struct qtk_aisrv_instance {
    int pid;
    int srv_fd;
    int fifo_fd;
    int run_cnt;
    wtk_string_t *fifo_path;
    unsigned run:1;
};

static char rep_ok[] = "OK";
static char req_create_instance[] = "CREATE INSTANCE";
static char rep_init_err[] = "ERROR INIT";
static char rep_fork_err[] = "ERROR FORK";
static char rep_too_many[] = "ERROR TOO MANY INSTANCE";
static char rep_invalid_req[] = "ERROR INVALID REQUEST";

static qtk_aisrv_t *ths_service;
static void *ths_idata;
static wtk_hash_t *pid_inst;
static int need_sweep_child;
/* static qtk_audio_decoder_t *decoder; */
static int nservice = 0;
static qtk_aisrv_mod_t mods[16];
static int nmod = 0;


static int auth_port = 80;
static char auth_host[64] = "cloud.qdreamer.com";
static char auth_url[128] = "/authApp/api/v1/serverDeviceVerify/";

int aisrv_init(qtk_aisrv_t *engine, struct qtk_zcontext *ctx, const char *param);

void qtk_aisrv_set_auth_port(int port) {
    auth_port = port;
}

void qtk_aisrv_set_auth_host(const char *host) {
    strncpy(auth_host, host, sizeof(auth_host));
}
void qtk_aisrv_set_auth_url(const char *url) {
    strncpy(auth_url, url, sizeof(auth_url));
}

int qtk_aisrv_get_auth_port() {
    return auth_port;
}

const char* qtk_aisrv_get_auth_host() {
    return auth_host;
}

const char* qtk_aisrv_get_auth_url() {
    return auth_url;
}


static void _inst_handle_signal(int sig) {
    switch (sig) {
    case SIGHUP:
        exit(0);
        break;
    default:
        break;
    }
}


static int _inst_set_signal() {
    int signals[] = {SIGHUP};
    int i;
    prctl(PR_SET_PDEATHSIG, SIGHUP);

    for (i = 0; i < sizeof(signals)/sizeof(signals[0]); ++i) {
        signal(signals[i], _inst_handle_signal);
    }

    signal(SIGSEGV, SIG_DFL);
    signal(SIGFPE, SIG_DFL);

    return 0;
}


static unsigned int _inst_hash_by_pid(qtk_aisrv_instance_t *inst, unsigned int nslot) {
    return (inst->pid) % nslot;
}


static int _inst_cmp_by_pid(qtk_aisrv_instance_t *inst1, qtk_aisrv_instance_t *inst2) {
    return inst1->pid - inst2->pid;
}


static qtk_aisrv_instance_t* _inst_new() {
    qtk_aisrv_instance_t *inst = wtk_malloc(sizeof(*inst));
    inst->srv_fd = INVALID_FD;
    inst->fifo_fd = INVALID_FD;
    inst->run_cnt = 0;
    return inst;
}


static int _inst_delete(qtk_aisrv_instance_t *inst) {
    if (inst->fifo_fd != INVALID_FD) {
        close(inst->fifo_fd);
    }
    if (inst->srv_fd != INVALID_FD) {
        close(inst->srv_fd);
    }
    if (inst->fifo_path) {
        char tmp[128];
        memcpy(tmp, inst->fifo_path->data, inst->fifo_path->len);
        tmp[inst->fifo_path->len] = '\0';
        unlink(tmp);
        wtk_string_delete(inst->fifo_path);
    }
    wtk_free(inst);
    return 0;
}


static void _inst_reopen_fds(int start_fd) {
    const char *fd_dir = "/proc/self/fd";
    const char *fd_fmt = "/proc/self/fd/%d";
    DIR *d;
    struct dirent *ent;
    wtk_queue_t queue;
    wtk_queue_node_t *node;
    wtk_queue_item_t *item;
    int fd;
    int fd2;
    int flag;
    char fdname[128];

    wtk_queue_init(&queue);
    d = opendir(fd_dir);
    if (!d) {
        const char *es = strerror(errno);
        wtk_debug("opendir failed: %s\r\n", es);
        qtk_zserver_log("opendir failed: %s", es);
        return;
    }
    while (1) {
        ent = readdir(d);
        if (!ent) {
            break;
        }
        if (ent->d_type != DT_DIR) {
            fd = wtk_str_atoi(ent->d_name, strlen(ent->d_name));
            if (fd < start_fd) continue;
            struct stat st;
            if (fstat(fd, &st) < 0) {
                const char *es = strerror(errno);
                wtk_debug("Get File Stat failed: %s\r\n", es);
                qtk_zserver_log("Get File Stat failed: %s", es);
                continue;
            }
            if (!S_ISREG(st.st_mode)) {
                continue;
            }
            item = (wtk_queue_item_t*)wtk_calloc(1, sizeof(wtk_queue_item_t));
            item->hook = (void*)(long long)fd;
            wtk_queue_push(&queue, &item->q_n);
        }
    }

    if (d) {
        closedir(d);
        d = NULL;
    }

    while ((node = wtk_queue_pop(&queue))) {
        item = data_offset2(node, wtk_queue_item_t, q_n);
        fd = (long long)item->hook;
        sprintf(fdname, fd_fmt, fd);
        /* printf("%s\r\n", fdname); */
        flag = fcntl(fd, F_GETFL);
        flag &= ~O_CREAT;
        fd2 = open(fdname, flag);
        dup2(fd2, fd);
        close(fd2);
        wtk_free(item);
    }
}


static int _gen_fifo_name(qtk_aisrv_instance_t *inst) {
    static uint32_t cnt = 0;
    char fifo_path[256];
    struct sockaddr_un addr_un;
    int addr_len;
    int fd;

    int ret;
    ret = sprintf(fifo_path, "fifo/%.*s.%u", ths_service->name->len, ths_service->name->data, cnt++);
    inst->fifo_path = wtk_string_dup_data(fifo_path, ret);
    addr_un.sun_family = AF_UNIX;
    strcpy(addr_un.sun_path, fifo_path);
    ret = -1;
    /* set input channel block */
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == INVALID_FD) {
        const char *es = strerror(errno);
        wtk_debug("socket failed : %s\r\n", es);
        qtk_zserver_log("create unix socket failed : %s", es);
        goto end;
    }
    unlink(fifo_path);
    addr_len = offsetof(struct sockaddr_un, sun_path) + inst->fifo_path->len;
    if (bind(fd, (struct sockaddr*)&addr_un, addr_len)) {
        const char *es = strerror(errno);
        wtk_debug("bind failed : %s\r\n", es);
        qtk_zserver_log("socket bind to \"%.*s\" failed: %s", inst->fifo_path->len, inst->fifo_path->data, es);
        goto end;
    }
    if (listen(fd, 5)) {
        const char *es = strerror(errno);
        wtk_debug("listen failed : %s\r\n", es);
        qtk_zserver_log(NULL, "socket listen failed : %s", es);
        goto end;
    }
    inst->srv_fd = fd;
    fd = INVALID_FD;
    ret = 0;
end:
    if (fd != INVALID_FD) {
        close(fd);
        unlink(fifo_path);
    }
    return ret;
}


static int _inst_response(qtk_aisrv_instance_t *inst, int code, wtk_string_t *result) {
    int len = result->len + sizeof(code);
    wtk_string_t *rep = wtk_string_new(len);
    memcpy(rep->data, &code, sizeof(code));
    memcpy(rep->data+sizeof(code), result->data, result->len);
    wtk_fd_write_string(inst->fifo_fd, rep->data, rep->len);
    wtk_string_delete(rep);
    return 0;
}


static int _inst_process(qtk_aisrv_instance_t *inst, wtk_string_t *req) {
    int ret = 0;
    /* int need_pre_audio = 0; */
    unsigned char cmd = req->data[0];
    wtk_string_t arg;
    wtk_string_set(&arg, req->data+sizeof(int), req->len-sizeof(int));
    //wtk_string_set(&res, NULL, 0);
    wtk_string_t res = {NULL, 0};

    switch (cmd) {
    case QTK_AISRV_CMD_START:
        inst->run_cnt++;
        ret = ths_service->module->start(ths_idata, &arg);
        break;
    case QTK_AISRV_CMD_FEED:
        ret = ths_service->module->feed(ths_idata, &arg);
        break;
    case QTK_AISRV_CMD_STOP:
        ret = ths_service->module->stop(ths_idata);
        break;
    case QTK_AISRV_CMD_GET_RESULT:
        wtk_string_set(&arg, NULL, 0);
        ret = ths_service->module->get_result(ths_idata, &res);
        break;
    default:
        ret = -1;
        break;
    }

    _inst_response(inst, ret, &res);
    return ret;
}


static void _inst_run(qtk_aisrv_instance_t *inst) {
    wtk_string_t *req;
    int ret;

    wtk_debug("open fifo %.*s\r\n", inst->fifo_path->len, inst->fifo_path->data);
    wtk_debug("INSTANCE [%d] is ready\r\n", inst->pid);
    qtk_zserver_log("INSTANCE [%.*s/%d] is ready",
                    ths_service->name->len, ths_service->name->data, inst->pid);

    inst->run = 1;

    while (inst->run) {
        req = wtk_fd_read_string(inst->fifo_fd);
        if (NULL == req) {
            wtk_debug("stopping...\r\n");
            inst->run = 0;
            break;
        }
        ret = _inst_process(inst, req);
        if (ret == -1) {
            /* exit if any error occur */
            wtk_debug("stopping...\r\n");
            inst->run = 0;
        }
        wtk_string_delete(req);
    }

    wtk_debug("INSTANCE clean fifo: %.*s\r\n",
              inst->fifo_path->len, inst->fifo_path->data);
    qtk_zserver_log("INSTANCE [%.*s/%d] exit",
                    ths_service->name->len, ths_service->name->data);
    _inst_delete(inst);
}


static void _aisrv_handle_signal(int sig) {
    switch (sig) {
    case SIGCHLD:
        need_sweep_child = 1;
        break;
    default:
        wtk_debug("receive unexpect signal: %d\r\n", sig);
        break;
    }
}


static void _aisrv_setup_signal() {
    int signals[] = {SIGCHLD,};
    int i;
    for (i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
        signal(signals[i], _aisrv_handle_signal);
    }
    signal(SIGSEGV, SIG_DFL);
}


static void _aisrv_pidhash_remove(int pid) {
    wtk_hash_node_t *n;
    qtk_aisrv_instance_t instance;
    qtk_aisrv_instance_t *inst = &instance;
    inst->pid = pid;
    if ((n = wtk_hash_remove(pid_inst, inst,
                             (wtk_hash_f)_inst_hash_by_pid,
                             (wtk_cmp_f)_inst_cmp_by_pid))) {
        ths_service->inst_cnt--;
        inst = (qtk_aisrv_instance_t *)n->v;
        _inst_delete(inst);
    }
}


static void _aisrv_sweep_children() {
    int pid;
    while ((pid = waitpid(0, NULL, WNOHANG)) > 0) {
        wtk_debug("sweep child. pid: %d\r\n", pid);
        qtk_zserver_log("SWEEP instance [%.*s/%d]", ths_service->name->len, ths_service->name->data, pid);
        _aisrv_pidhash_remove(pid);
    }
}


static void _aisrv_pidhash_add(qtk_aisrv_t *srv, qtk_aisrv_instance_t *inst) {
    _aisrv_pidhash_remove(inst->pid);
    wtk_hash_node_t *n = wtk_malloc(sizeof(*n));
    wtk_queue_node_init(&n->q_n);
    wtk_hash_add_node(pid_inst, inst, n, (wtk_hash_f)_inst_hash_by_pid);
    srv->inst_cnt++;
}


static int _inst_open_srv_sock(qtk_aisrv_instance_t *inst) {
    int fd;
    struct sockaddr_un addr_cli;
    unsigned int len;
    len = sizeof(addr_cli);
    fd = accept(inst->srv_fd, (struct sockaddr*)&addr_cli, &len);
    if (fd == INVALID_FD) {
        const char *es = strerror(errno);
        wtk_debug("accept failed : %s\r\n", es);
        qtk_zserver_log("INSTANCE [%.*s/%d] accept failed: %s",
                        ths_service->name->len, ths_service->name->data, inst->pid, es);
        return -1;
    }
    wtk_fd_set_tcp_client_opt(fd);
    inst->fifo_fd = fd;
    return 0;
}


static wtk_string_t* _aisrv_fork_instance(qtk_aisrv_t *srv) {
    qtk_aisrv_instance_t *inst = _inst_new();

    if (_gen_fifo_name(inst)) {
        _inst_delete(inst);
        return NULL;
    }

    inst->pid = fork();

    if (inst->pid < 0) {
        _inst_delete(inst);
        return NULL;
    } else if (0 == inst->pid) {
        char buf[128];
        const char *p;
        qtk_log_reopen(qtk_zserver_get_log());
        _inst_reopen_fds(3);
        inst->pid = getpid();
        _inst_set_signal();
        if (_inst_open_srv_sock(inst)) exit(0);
        sprintf(buf, "%.*s", (int)min(sizeof(buf)-1, inst->fifo_path->len), inst->fifo_path->data);
        p = strrchr(buf, '/');
        if (p) {
            prctl(PR_SET_NAME, p + 1);
        } else {
            prctl(PR_SET_NAME, buf);
        }
        ths_idata = srv->module->create(srv->data);
        _inst_run(inst);
        if (ths_idata) {
            ths_service->module->release(ths_idata);
        }
        exit(0);
    } else {
        close(inst->srv_fd);
        inst->srv_fd = INVALID_FD;
        _aisrv_pidhash_add(srv, inst);
    }

    return inst->fifo_path;
}


/* static void _aisrv_del_instance(qtk_aisrv_t *srv) { */
/*     uint32_t handle = qtk_zcontext_handle(srv->ctx); */
/*     qtk_zsend(srv->ctx, 0, handle, 0, 0, "D", 1); */
/* } */


static int _aisrv_init_engine(qtk_aisrv_t *srv) {
    srv->data = srv->module->init(srv->cfg->lc);
    return 0;
}


static int _pid_hash_walker(void *user_data, void *data) {
    wtk_hash_node_t *hnode = (wtk_hash_node_t*)data;
    _inst_delete(hnode->v);
    wtk_debug("_inst_delete\r\n");
    return 0;
}


static int _aisrv_proc_init(qtk_aisrv_t *srv, wtk_pipeproc_t *proc) {
    int ret = -1;
    qtk_log_reopen(qtk_zserver_get_log());
    _aisrv_setup_signal();
    pid_inst = wtk_hash_new(100);
    if (_aisrv_init_engine(srv)) {
        wtk_debug("engine init failed in child process\r\n");
        qtk_zserver_log("ENGINE [%.*s] init failed", srv->name->len, srv->name->data);
        wtk_pipeproc_write_string(proc, rep_init_err, sizeof(rep_init_err)-1);
    } else {
        qtk_zserver_log("ENGINE [%.*s] init", srv->name->len, srv->name->data);
        ret = wtk_pipeproc_write_string(proc, rep_ok, sizeof(rep_ok)-1);
    }
    return ret;
}


static int _aisrv_proc_clean(qtk_aisrv_t *srv) {
    if (srv->data) {
        srv->module->clean(srv->data);
    }
    wtk_hash_walk(pid_inst, _pid_hash_walker, NULL);
    wtk_hash_delete(pid_inst);
    pid_inst = NULL;
    qtk_zerror(srv->ctx, "engine [%s] exited.", srv->name->data);
    return 0;
}


static wtk_string_t* _aisrv_proc_process(qtk_aisrv_t *srv, wtk_string_t *req) {
    wtk_string_t *rep;
    wtk_string_t *fifo;

    if (need_sweep_child) {
        _aisrv_sweep_children();
        need_sweep_child = 0;
    }

    if (wtk_string_equal_s(req, req_create_instance)) {
        if (srv->cfg->max_instance && srv->inst_cnt >= srv->cfg->max_instance) {
            rep = wtk_string_dup(rep_too_many);
            goto end;
        }
        fifo = _aisrv_fork_instance(srv);
        if (NULL == fifo) {
            qtk_zserver_log("create \"%.*s\" instance failed", srv->name->len, srv->name->data);
            rep = wtk_string_dup(rep_fork_err);
            goto end;
        }
        rep = wtk_string_dup_data(fifo->data, fifo->len);
        wtk_debug("new_instance %.*s\r\n", rep->len, rep->data);
    } else {
        wtk_debug("invalid req %.*s\r\n", req->len, req->data);
        qtk_zserver_log("invalid engine request: \"%.*s\"", req->len, req->data);
        rep = wtk_string_dup(rep_invalid_req);
    }
end:
    return rep;
}


int _aisrv_module_init(qtk_aisrv_mod_t *mod) {
    char name[256];
    qtk_aisrv_openmod_f open;
    int i;
    sprintf(name, "./%.*s.so", mod->name->len, mod->name->data);
    void *dl = NULL;
    dl = dlopen(name, RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    if (NULL == dl) {
        wtk_debug("try open %s failed : %s\r\n", name, dlerror());
        qtk_zserver_log("aisrv: try open %s failed: %s", name, dlerror());
        return -1;
    }
    mod->handle = dl;
    i = sprintf(name, "%.*s_open", mod->name->len, mod->name->data);
    /* replace '.' with '_' */
    while (--i >= 0) {
        if ('.' == name[i]) name[i] = '_';
    }
    open = dlsym(dl, name);
    if (NULL == open) {
        wtk_debug("cannot find entry function[%s] in %.*s.so\r\n",
                  name, mod->name->len, mod->name->data);
        qtk_zserver_log("aisrv: cannot find entry function[%s] in %.*s.so",
                  name, mod->name->len, mod->name->data);
        return -1;
    }
    if (open(mod)) return -1;
    assert(mod->init != NULL);
    return 0;
}


void _aisrv_conf_delete(qtk_aisrv_cfg_t *cfg) {
    if (cfg->cfg_file) {
        wtk_cfg_file_delete(cfg->cfg_file);
    }
    wtk_free(cfg);
}


static void _aisrv_conf_init(qtk_aisrv_cfg_t *cfg) {
    cfg->max_instance = 100;
    cfg->max_loop = 0;         /* unlimited */
}


static void _aisrv_conf_update_local(qtk_aisrv_cfg_t *cfg, wtk_local_cfg_t *main) {
    wtk_string_t *v = NULL;

    wtk_local_cfg_update_cfg_i(main, cfg, max_instance, v);
    wtk_local_cfg_update_cfg_i(main, cfg, max_loop, v);
}


qtk_aisrv_cfg_t* _aisrv_conf_new(const char *name, const char *res) {
    char fpath[2048];
    char dpath[2048];
    char *text = NULL;
    int len;
    wtk_local_cfg_t *mcfg;
    qtk_aisrv_cfg_t *cfg = wtk_calloc(1, sizeof(*cfg));
    int ret = -1;
    wtk_proc_get_dir(dpath, sizeof(dpath));
    strcpy(&dpath[strlen(dpath)], "/../ext");
    sprintf(fpath, "%s/%s.cfg", dpath, name);
    if (!wtk_file_exist(fpath)) {
        text = file_read_buf(fpath, &len);
    }
    if (NULL == text) {
        wtk_debug("%s not exist.\n", fpath);
        qtk_zserver_log("config file \"%s\" not exist", fpath);
        goto end;
    }
    cfg->cfg_file = wtk_cfg_file_new(fpath);
    cfg->heap = cfg->cfg_file->heap;
    wtk_cfg_file_add_var_ks(cfg->cfg_file, "pwd", dpath, strlen(dpath));
    ret = wtk_cfg_file_feed(cfg->cfg_file, text, len);
    if (ret) goto end;
    mcfg = cfg->cfg_file->main;
    cfg->lc = wtk_local_cfg_find_lc(mcfg, (char*)res, strlen(res));
    if (!cfg->lc) {
        wtk_debug("engine entry \"%s\" not found in %s\r\n", res, fpath);
        qtk_zserver_log("engine entry \"%s\" not found in %s", res, fpath);
        goto end;
    }
    _aisrv_conf_init(cfg);
    _aisrv_conf_update_local(cfg, mcfg);
    ret = 0;
end:
    if (text) {
        wtk_free(text);
    }
    if (ret) {
        if (cfg) {
            _aisrv_conf_delete(cfg);
            cfg = NULL;
        }
    }
    return cfg;
}


static qtk_aisrv_mod_t* _aisrv_query_module(wtk_string_t *name) {
    int i;
    qtk_aisrv_mod_t *mod;
    for (i = 0; i < nmod; ++i) {
        if (wtk_string_equal(mods[i].name, name)) {
            mod = mods + i;
            return mod;
        }
    }
    if (nmod >= sizeof(mods)/sizeof(mods[0])) {
        return NULL;
    }
    mod = &mods[nmod];
    mod->name = name;
    if (0 == _aisrv_module_init(mod)) {
        ++nmod;
    }
    mod->name = wtk_string_dup_data(name->data, name->len);
    return mod;
}


static void _aisrv_clean() {
    int i;
    assert(NULL == pid_inst);
    for (i = 0; i < nmod; ++i) {
        wtk_string_delete(mods[i].name);
        dlclose(mods[i].handle);
    }
    nmod = 0;
}


qtk_aisrv_t* aisrv_create(void) {
    qtk_aisrv_t *inst = wtk_calloc(1, sizeof(*inst));
    if (nservice == 0) {
        qtk_aisrv_set_auth_host(qtk_zserver_g()->cfg->hw_auth_host);
        qtk_aisrv_set_auth_url(qtk_zserver_g()->cfg->hw_auth_url);
        qtk_aisrv_set_auth_port(qtk_zserver_g()->cfg->hw_auth_port);
    }
    QTK_ATOM_INC(&nservice);
    return inst;
}


void aisrv_release(qtk_aisrv_t *engine) {
    assert(NULL == engine->data);
    if (engine->proc) {
        wtk_pipeproc_clean(engine->proc);
        wtk_pipeproc_join(engine->proc);
        wtk_free(engine->proc);
    }
    if (engine->name) {
        wtk_string_delete(engine->name);
    }
    if (engine->cfg) {
        _aisrv_conf_delete(engine->cfg);
    }

    wtk_free(engine);
    if (0 == QTK_ATOM_DEC(&nservice)) {
        _aisrv_clean();
    }
}


static int aisrv_cb(struct qtk_zcontext *ctx, void *ud, int type, int session, uint32_t src,
                    const char *data, size_t sz) {
    qtk_aisrv_t *engine = (qtk_aisrv_t*)ud;
    int ret;
    wtk_string_t *rep = NULL;
    int retry = 5;
    if (type == ZEUS_PTYPE_ERROR) goto end;
    assert(engine->ctx == ctx);
    while (--retry) {
        ret = wtk_pipeproc_write_string(engine->proc, req_create_instance,
                                        sizeof(req_create_instance)-1);
        if (ret) {
            aisrv_init(engine, ctx, NULL);
            continue;
        }
        rep = wtk_pipeproc_read_string(engine->proc);
        if (NULL == rep) {
            aisrv_init(engine, ctx, NULL);
            continue;
        }
        qtk_zsend(ctx, 0, src, ZEUS_PTYPE_RESPONSE, session, rep->data, rep->len);
        break;
    }
    if (0 == retry) {
        char tmp[128];
        int n = snprintf(tmp, sizeof(tmp), "ERROR : %s is shutdown", engine->name->data);
        qtk_zsend(ctx, 0, src, ZEUS_PTYPE_RESPONSE, session, tmp, n);
    }
end:
    if (rep) {
        wtk_string_delete(rep);
    }
    return 0;
}


int aisrv_init(qtk_aisrv_t *engine, struct qtk_zcontext *ctx, const char *param) {
    assert(param || engine->name);
    size_t sz = 0;
    if (param) {
        sz = strlen(param);
        engine->name = wtk_string_new(sz + 1);
        strcpy(engine->name->data, param);
    } else {
        param = (const char*)engine->name->data;
        sz = strlen(param);
    }
    char method[sz+2];
    strcpy(method, param);
    char *args = strrchr(method, '.');
    if (args) *args = '\0';
    size_t med_sz = strlen(method);
    wtk_string_t name;
    wtk_string_set(&name, method, med_sz);
    qtk_aisrv_mod_t *mod = _aisrv_query_module(&name);
    wtk_string_t *rep;
    int ret = -1;
    if (NULL == mod) {
        qtk_zerror(ctx, "engine [%.*s] open module failed",
                   engine->name->len, engine->name->data);
        goto end;
    }
    engine->module = mod;
    if (engine->cfg) {
        _aisrv_conf_delete(engine->cfg);
    }
    engine->cfg = _aisrv_conf_new(method, method+med_sz+1);
    if (NULL == engine->cfg) {
        qtk_zerror(ctx, "engine [%.*s] open conf file failed",
                   engine->name->len, engine->name->data);
        goto end;
    }
    qtk_zerror(ctx, "engine [%.*s] starting...", sz, param);
    ths_service = engine;
    if (engine->proc) {
        wtk_pipeproc_clean(engine->proc);
        wtk_pipeproc_join(engine->proc);
        wtk_free(engine->proc);
    }
    engine->proc = wtk_pipeproc_new(engine->name->data,
                                    (pipeproc_init_handler)_aisrv_proc_init,
                                    (pipeproc_clean_handler)_aisrv_proc_clean,
                                    (pipeproc_handler)_aisrv_proc_process,
                                    engine);
    /*
      ignore SIGPIPE
     */
    signal(SIGPIPE, SIG_IGN);
    if (NULL == engine->proc) goto end;
    rep = wtk_pipeproc_read_string(engine->proc);
    if (NULL == rep) goto end;
    if (wtk_string_cmp_s(rep, rep_ok)) {
        qtk_zerror(ctx, "engine [%.*s] start failed, %.*s",
                   engine->name->len, engine->name->data, rep->len, rep->data);
    } else {
        qtk_zerror(ctx, "engine [%.*s] started",
                   engine->name->len, engine->name->data);
        ret = 0;
    }
    wtk_string_delete(rep);

    engine->ctx = ctx;
    qtk_zcallback(ctx, engine, aisrv_cb);
    sprintf(method, ".%s", param);
    qtk_zcommand(ctx, "REG", method);
    ret = 0;
end:
    return ret;
}
