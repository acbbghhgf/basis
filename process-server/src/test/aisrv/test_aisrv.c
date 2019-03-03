#include <dlfcn.h>
#include <sys/wait.h>
#include "qtk/sframe/qtk_sframe.h"
#include "wtk/os/wtk_thread.h"
#include "qtk/service/qtk_aisrv_module.h"
#include "qtk/service/qtk_aisrv_cfg.h"


typedef struct test_engine test_engine_t;


struct test_engine {
    wtk_thread_t thread;
    int pid;
    void *frame;
    qtk_aisrv_mod_t mod;
    qtk_aisrv_cfg_t *cfg;
    char *test_file;
    char *engine_name;
    char *engine_res;
    char *engine_cfg;
    unsigned run:1;
};


qtk_aisrv_cfg_t* _aisrv_conf_new(const char *name, const char *res);
void _aisrv_conf_delete(qtk_aisrv_cfg_t *cfg);
int _aisrv_module_init(qtk_aisrv_mod_t *mod);

int qtk_zsend(void *ctx, uint32_t src, uint32_t dst, int type, int session, void *data, size_t sz) {return 0;}

const char* qtk_zcommand(void *ctx, const char *cmd, const char *param) { return NULL;}

void qtk_zcallback(void *ctx, void *ud, void * cb) {}

void qtk_zerror(void *ctx, const char *msg, ...) {}

static void engine_conf(test_engine_t *engine, wtk_local_cfg_t *main) {
    wtk_local_cfg_t *lc = main;
    wtk_string_t *v;
    wtk_local_cfg_update_cfg_str(lc, engine, test_file, v);
    wtk_local_cfg_update_cfg_str(lc, engine, engine_cfg, v);
    wtk_local_cfg_update_cfg_str(lc, engine, engine_name, v);
    wtk_local_cfg_update_cfg_str(lc, engine, engine_res, v);
}


static int thread_route(void *data, wtk_thread_t *t) {
    test_engine_t *engine = data;
    qtk_aisrv_mod_t *module = &engine->mod;
    _aisrv_module_init(module);
    void *srv_data = module->init(engine->cfg->lc);

    int pid = fork();
    if (pid > 0) {
        waitpid(pid, NULL, 0);
        goto end;
    }

    void *inst = module->create(srv_data);

    while (engine->run) {
        int fd = open("../data/tee.wav", O_RDONLY);
        char buf[4097];
        wtk_string_t arg = wtk_string("{\"text\":\"tee\"}");
        int n;
        module->start(inst, &arg);
        while ((n = read(fd, buf, sizeof(buf)-1)) > 0) {
            wtk_debug("%d\r\n", n);
            wtk_string_set(&arg, buf, n);
            module->feed(inst, &arg);
        }
        module->stop(inst);
        module->get_result(inst, &arg);
        wtk_debug("%s\n", arg.data);
        engine->run = 0;
    }
    module->release(inst);
end:
    module->clean(srv_data);

    return 0;
}


test_engine_t* test_aisrv_new(void * frame) {
    test_engine_t *demo = wtk_malloc(sizeof(*demo));
    qtk_sframe_method_t *method = frame;
    wtk_local_cfg_t *lc = method->get_cfg(frame);

    engine_conf(demo, lc);
    demo->mod.name = wtk_string_dup_data(demo->engine_name, strlen(demo->engine_name));
    demo->cfg = _aisrv_conf_new(demo->engine_name, demo->engine_res);
    /* wtk_thread_init(&demo->thread, thread_route, demo); */
    demo->frame = frame;
    demo->run = 0;

    return demo;
}


int test_aisrv_start(test_engine_t *demo) {
    demo->run = 1;
    demo->pid = fork();
    /* wtk_thread_start(&demo->thread); */
    if (demo->pid == 0) {
        thread_route(demo, NULL);
        exit(0);
    }

    return 0;
}


int test_aisrv_stop(test_engine_t *demo) {
    /* wtk_thread_join(&demo->thread); */
    waitpid(demo->pid, NULL, 0);
    demo->run = 0;
    return 0;
}


int test_aisrv_delete(test_engine_t *demo) {
    /* wtk_thread_join(&demo->thread); */
    /* wtk_thread_clean(&demo->thread); */
    if (demo->mod.name) {
        wtk_string_delete(demo->mod.name);
        // dlclose(demo->mod.handle); // for valgrind output
    }
    if (demo->cfg) {
        _aisrv_conf_delete(demo->cfg);
        demo->cfg = NULL;
    }
    wtk_free(demo);
    return 0;
}
