#include "qtk_inst.h"
#include "third/uuid/uuid4.h"
#include "qtk_inst_cfg.h"
#include "qtk/ipc/qtk_unsocket.h"
#include "wtk/semdlg/wtk_semdlg.h"
#include "qtk/os/qtk_base.h"
#include "qtk/os/qtk_alloc.h"
#include "qtk/core/cJSON.h"
#include "mod/qtk_cmd.h"

#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <malloc.h>
#include <sys/prctl.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <hiredis/hiredis.h>

struct ZYGOTE;

typedef int (*F)(struct ZYGOTE *z, wtk_string_t *req);

#define CoreHeader              \
    qtk_inst_cfg_t *cfg;        \
    qtk_unsocket_t *s;          \
    wtk_semdlg_t *inst;         \
    int fd;                     \
    F f;

struct ZYGOTE {
    CoreHeader
};

struct INST {
    CoreHeader
    redisContext *rctx;
    wtk_string_t result; // pointed mem which inside wtk_semdlg_t
};

static struct {
    void *dl;
    wtk_semdlg_cfg_t *(*semdlg_cfg_new)(char *);
    void (*semdlg_cfg_delete)(wtk_semdlg_cfg_t *);
    wtk_semdlg_t *(*semdlg_new)(wtk_semdlg_cfg_t *);
    int (*semdlg_lua_state_set)(wtk_semdlg_t *, const char *, int);
    int (*semdlg_inter_state_set)(wtk_semdlg_t *, char *, int);
    int (*semdlg_lua_state_get)(wtk_semdlg_t *, wtk_strbuf_t *);
    int (*semdlg_inter_state_get)(wtk_semdlg_t *, wtk_strbuf_t *);
    void (*semdlg_set_env)(wtk_semdlg_t *, char *, int);
    wtk_string_t (*semdlg_process2)(wtk_semdlg_t *, char *, int , int);
    void (*semdlg_delete)(wtk_semdlg_t *);
    int (*semdlg_reset)(wtk_semdlg_t *);
    int (*semdlg_reset_history)(wtk_semdlg_t *);
} M;

static void _inst_run(struct INST *I, const char *path, size_t len, sem_t *sem);

static void _reopen_fds()
{
    const char *fd_dir = "/proc/self/fd";
    const char *fd_fmt = "/proc/self/fd/%d";
    wtk_queue_t q;
    wtk_queue_init(&q);

    DIR *d = opendir(fd_dir);
    if (!d) {
        qtk_debug("opendir failed: %s\n", strerror(errno));
        return;
    }
    while (1) {
        struct dirent *ent = readdir(d);
        if (!ent) break;
        if (ent->d_type != DT_DIR) {
            int fd = wtk_str_atoi(ent->d_name, strlen(ent->d_name));
            if (fd < 3) continue;
            struct stat st;
            if (fstat(fd, &st) < 0) {
                perror("Get File Stat failed");
                continue;
            }
            if (!S_ISREG(st.st_mode)) {
                close(fd);
                continue;
            }
            wtk_queue_item_t *item = wtk_malloc(sizeof(*item));
            item->hook = (void *)(long long)fd;
            wtk_queue_push(&q, &item->q_n);
        }
    }
    closedir(d);
    d = NULL;

    wtk_queue_node_t *node, *next;
    for (node = q.pop; node; node = next) {
        wtk_queue_item_t *item = data_offset2(node, wtk_queue_item_t, q_n);
        int fd = (long long)item->hook;
        char fd_name[108];
        snprintf(fd_name, sizeof(fd_name), fd_fmt, fd);
        int flag = fcntl(fd, F_GETFL);
        flag &= ~O_CREAT;
        int fd2 = open(fd_name, flag);
        dup2(fd2, fd);
        close(fd2);
        next = node->next;
        qtk_free(item);
    }
}

static void _signal_handler(int sig)
{
    switch (sig) {
    case SIGUSR1:
        exit(0);
        break;
    case SIGCHLD:
        while(waitpid(-1, NULL, WNOHANG)>0);
        break;
    default:
        ;
    }
}

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

static void _setup_signal()
{
    int sigs[] = {SIGUSR1, SIGCHLD};
    int i;
    for (i=0; cast(unsigned long, i)<ARRAY_SIZE(sigs); i++)
        signal(sigs[i], _signal_handler);
}

static redisContext *_get_redis(struct INST *I)
{
    char tmp[128];
    strcpy(tmp, I->cfg->redisAddr);
    char *colon = strchr(tmp, ':');
    *colon = '\0';
    int port = atoi(colon + 1);
    redisContext *rctx = redisConnect(tmp, port);
    if (rctx && !rctx->err) return rctx;
    qtk_debug("error connect redis %s:%d\n", tmp, port);
    if (rctx) redisFree(rctx);
    return NULL;
}

#define IsJsonString(item)      (item && item->type == cJSON_String && item->valuestring)
#define IsRedisReplyString(r)   (r->type == REDIS_REPLY_STRING && r->str)

static void _inst_resume_state(struct INST *I, const char *clientId)
{
    M.semdlg_reset(I->inst);
    M.semdlg_reset_history(I->inst);
    redisReply *r = redisCommand(I->rctx, "HMGET %s lua inter", clientId);
    if (r->type == REDIS_REPLY_ARRAY) {
        redisReply *lua = r->element[0];
        redisReply *inter = r->element[1];
        if (IsRedisReplyString(lua))
            M.semdlg_lua_state_set(I->inst, lua->str, strlen(lua->str));
        if (IsRedisReplyString(inter))
            M.semdlg_inter_state_set(I->inst, inter->str, strlen(inter->str));
    }
    freeReplyObject(r);
}

static void _inst_store_state(struct INST *I, const char *clientId)
{
    wtk_strbuf_t *state = wtk_strbuf_new(2048, 1);
    M.semdlg_lua_state_get(I->inst, state);
    wtk_strbuf_push_c(state, '\0');

    redisReply *r = redisCommand(I->rctx, "HMSET %s lua %s", clientId, state->data);
    if (r->type == REDIS_REPLY_ERROR)
        qtk_debug("error ouccured %s\n", I->rctx->errstr);
    freeReplyObject(r);

    wtk_strbuf_reset(state);
    M.semdlg_inter_state_get(I->inst, state);
    wtk_strbuf_push_c(state, '\0');

    r = redisCommand(I->rctx, "HMSET %s inter %s", clientId, state->data);
    if (r->type == REDIS_REPLY_ERROR)
        qtk_debug("error ouccured %s\n", I->rctx->errstr);
    freeReplyObject(r);

    r = redisCommand(I->rctx, "EXPIRE %s %d", clientId, I->cfg->stateExpire);
    if (r->type == REDIS_REPLY_ERROR)
        qtk_debug("error ouccured %s\n", I->rctx->errstr);
    freeReplyObject(r);

    wtk_strbuf_delete(state);
}

static int _raise_result(struct ZYGOTE *I, wtk_string_t *res, const char *dst)
{
    // TODO simplify protocol
    qtk_pack_t *pack = qtk_pack_new();
    qtk_pack_add_s(pack, "dst", dst, strlen(dst));
    qtk_pack_add_s(pack, "res", res->data, res->len);
    qtk_pack_encode(pack);
    if (wtk_fd_write(I->fd, pack->buf->data, pack->buf->pos)) {
        qtk_debug("write failed\n");
    }
    qtk_pack_delete(pack);
    return 0;
}

static void _inst_calc(struct INST *I, const char *param)
{
    cJSON *json, *item;
    json = cJSON_Parse(param);
    if (NULL == json) {
        qtk_debug("ERROR: param not a json [%s]\n", param);
        goto end;
    }

    item = cJSON_GetObjectItem(json, "clientId");
    if (!IsJsonString(item)) {
        qtk_debug("ERROR: param missing clientId\n");
        goto end;
    }
    const char *clientId = item->valuestring;

    item = cJSON_GetObjectItem(json, "input");
    if (!IsJsonString(item)) {
        qtk_debug("ERROR: param missing input\n");
        goto end;
    }
    const char *input = item->valuestring;

    _inst_resume_state(I, clientId);

    char env[1024];
    int elen = snprintf(env, sizeof(env), "client=\"%s\";", clientId);
    M.semdlg_set_env(I->inst, env, elen);

    item = cJSON_GetObjectItem(json, "useJson");
    int useJson = (item && item->type == cJSON_True);

    I->result = M.semdlg_process2(I->inst, cast(char *, input), strlen(input), useJson);
    if (I->result.len <= 0) {
        qtk_debug("inst process error\n");
        goto end;
    }

    _inst_store_state(I, clientId);

end:
    if (json) cJSON_Delete(json);
}

static void _inst_getRes(struct INST *I, const char *param)
{
    _raise_result(cast(struct ZYGOTE *, I), &I->result, param);
}

static void _zygote_clean_env(struct ZYGOTE *Z)
{
    close(Z->fd);
    close(Z->s->fd);
    qtk_free(Z->s);
    Z->s = NULL;
}

static void _get_inst(struct ZYGOTE *Z, const char *param)
{
    unuse(param);
    char tmp[UUID4_LEN];
    uuid4_generate(tmp);
    char path[108];
    int len = snprintf(path, sizeof(path), "/tmp/dlgd/%s", tmp);
    sem_t *sem = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE,
                               MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sem_init(sem, 1, 0);
    pid_t pid = fork();    // parent process is zygote process
    if (pid == 0) {
        _zygote_clean_env(Z);
        _inst_run(cast(struct INST *, Z), path, len, sem);
        exit(0);
    }
    sem_wait(sem);
    char buf[1024];
    // path + : + pid + : + name
    if (Z->cfg->tmp) {
        len = snprintf(buf, sizeof(buf), "%s:%d:%s", path, pid, Z->cfg->cfn);
    } else {
        len = snprintf(buf, sizeof(buf), "%s:%d:%.*s", path, pid,
                       Z->cfg->res->len, Z->cfg->res->data);
    }
    wtk_string_t res = {buf, len};
    _raise_result(Z, &res, param);
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));
}

static int _zygote_process(struct ZYGOTE *Z, wtk_string_t *req)
{
    char *param;
    long cmd = strtol(req->data, &param, 0);
    switch (cmd) {
    case GETINST:
        _get_inst(Z, param + 1); // skip separator
        break;
    case EXIT:
        return -1;
    default:
        qtk_debug("unknown cmd %ld\n", cmd);
    }
    return 0;
}

static void _inst_itoz(struct INST *I)
{
    I->f = _zygote_process;
    if (I->rctx) {
        redisFree(I->rctx);
        I->rctx = NULL;
    }
    I->result.len = 0;
    I->result.data = NULL;
}

static int _inst_process(struct ZYGOTE *Z, wtk_string_t *req)
{
    char *param;
    long cmd = strtol(req->data, &param, 0);
    struct INST *I = cast(struct INST *, Z);
    switch (cmd) {
    case CALC:
        _inst_calc(I, param + 1); // skip separator
        break;
    case GETRES:
        _inst_getRes(I, param + 1);
        break;
    case ITOZ:
        _inst_itoz(I);
        break;
    case EXIT:
        return -1;
    default:
        qtk_debug("unknown cmd %ld\n", cmd);
    }
    return 0;
}

static int _inst_prepare(const char *path, size_t len, sem_t *sem, struct INST *I)
{
    malloc_trim(0);
    _setup_signal();
    prctl(PR_SET_PDEATHSIG, SIGUSR1);
    _reopen_fds();
    I->s = qtk_unsocket_serve(path, len, 10);
    sem_post(sem);
    if (NULL == I->inst) {
        wtk_semdlg_cfg_t *core_cfg = M.semdlg_cfg_new(cast(char *, I->cfg->cfn));
        if (NULL == core_cfg) return -1;
        I->inst = M.semdlg_new(core_cfg);
        if (NULL == I->inst) return -1;
    }
    if (NULL == I->rctx) I->rctx = _get_redis(I);
    I->f = _inst_process;
    I->fd = accept(I->s->fd, NULL, NULL);
    return 0;
}

static void _inst_clean(struct INST *I)
{
    if (I->s) qtk_unsocket_delete(I->s);
    if (I->rctx) redisFree(I->rctx);
    if (I->inst) {
        wtk_semdlg_cfg_t *cfg = I->inst->cfg;
        M.semdlg_delete(I->inst);
        M.semdlg_cfg_delete(cfg);
    }
    close(I->fd);
}

static void _inst_run(struct INST *I, const char *path, size_t len, sem_t *sem)
{
    if (_inst_prepare(path, len, sem, I)) return ;

    while (1) {
        // req->data always has '\0' after it's last character
        wtk_string_t *req = wtk_fd_read_string(I->fd);
        if (req == NULL) break;         // we think inst broken, so exit
        int ret = I->f(cast(struct ZYGOTE *, I), req);
        wtk_string_delete(req);
        if (ret) break;
    }

    qtk_debug("inst exiting ... \n");
    _inst_clean(I);
}

qtk_variant_t *qtk_inst_new(qtk_inst_cfg_t *cfg, const char *path, size_t len)
{

    sem_t *sem = (sem_t *)mmap(NULL, sizeof(sem_t), PROT_READ|PROT_WRITE,
                               MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    sem_init(sem, 1, 0);

    pid_t pid = fork();    // parent process is main process
    if (pid == 0) {
        struct INST I;
        memset(&I, 0, sizeof(I));
        I.cfg = cfg;
        _inst_run(&I, path, len, sem);
        exit(0);
    }

    qtk_variant_t *v = cfg->tmp ? qtk_variant_new(pid, cfg->cfn, strlen(cfg->cfn)) :
                                  qtk_variant_new(pid, cfg->res->data, cfg->res->len);
    sem_wait(sem);
    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));
    return v;
}

int qtk_inst_mod_init(const char *fn)
{
    void *dl = dlopen(fn, RTLD_LAZY|RTLD_DEEPBIND);
    if (NULL == dl) {
        printf("%s\n", dlerror());
        return -1;
    }
    _setup_signal();
    M.semdlg_new = dlsym(dl, "wtk_semdlg_new");
    M.semdlg_cfg_new = dlsym(dl, "wtk_semdlg_cfg_new");
    M.semdlg_cfg_delete = dlsym(dl, "wtk_semdlg_cfg_delete");
    M.semdlg_delete = dlsym(dl, "wtk_semdlg_delete");
    M.semdlg_lua_state_get = dlsym(dl, "qtk_lua_local_get");
    M.semdlg_lua_state_set = dlsym(dl, "qtk_lua_local_set");
    M.semdlg_inter_state_get = dlsym(dl, "qtk_semdlg_get_inter_state");
    M.semdlg_inter_state_set = dlsym(dl, "qtk_semdlg_set_inter_state");
    M.semdlg_process2 = dlsym(dl, "wtk_semdlg_process2");
    M.semdlg_set_env = dlsym(dl, "wtk_semdlg_set_env");
    M.semdlg_reset = dlsym(dl, "wtk_semdlg_reset");
    M.semdlg_reset_history = dlsym(dl, "wtk_semdlg_reset_history");
    return 0;
}

void qtk_inst_mod_clean()
{
    dlclose(M.dl);
    memset(&M, 0, sizeof(M));
}
