#ifndef QTK_ZEUS_QTK_ZEUS_MODULE_H
#define QTK_ZEUS_QTK_ZEUS_MODULE_H
#include "wtk/os/wtk_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QTK_ZMODULE_MAX 32

struct qtk_zcontext;

typedef struct qtk_zmodule qtk_zmodule_t;
typedef struct qtk_zeus_modules qtk_zeus_modules_t;
typedef void* (*qtk_zmodule_create_f)();
typedef int (*qtk_zmodule_init_f)(void *inst, struct qtk_zcontext *ctx, const char *param);
typedef int (*qtk_zmodule_release_f)(void *inst);

struct qtk_zmodule {
    const char *name;
    void *module;
    qtk_zmodule_create_f create;
    qtk_zmodule_init_f init;
    qtk_zmodule_release_f release;
};

struct qtk_zeus_modules {
    int count;
    wtk_lock_t lock;
    const char *path;
    qtk_zmodule_t m[QTK_ZMODULE_MAX];
};


qtk_zeus_modules_t* qtk_zmodule_new(const char *path);
int qtk_zmodule_delete(qtk_zeus_modules_t* zmods);
int qtk_zmodule_insert(qtk_zeus_modules_t* zmods, qtk_zmodule_t *mod);
qtk_zmodule_t *qtk_zmodule_query(qtk_zeus_modules_t *zmods, const char *name);
void *qtk_zmodule_instance_create(qtk_zmodule_t *mod);
int qtk_zmodule_instance_init(qtk_zmodule_t *mod, void *inst, struct qtk_zcontext *ctx, const char *param);
void qtk_zmodule_instance_release(qtk_zmodule_t *mod, void *inst);


#ifdef __cplusplus
}
#endif


#endif
