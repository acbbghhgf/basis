#include "qtk_zeus_module.h"
#include <dlfcn.h>


static void* _try_open(qtk_zeus_modules_t *zmods, const char *name) {
    const char *ext = ".so";
    size_t ext_size = strlen(ext);
    const char *path = zmods->path;
    size_t path_size = strlen(path);
    size_t name_size = strlen(name);
    size_t sz = path_size + name_size + ext_size + 1;
    void *dl = NULL;
    char tmp[sz];
    do {
        while (*path == ';') path++;
        if (*path == 0) break;
        const char *l = strchr(path, ';');
        if (l == NULL) {
            path_size = strlen(path);
            l = path + path_size;
        } else {
            path_size = l - path;
        }
        memcpy(tmp, path, path_size);
        memcpy(tmp+path_size, name, name_size);
        memcpy(tmp+path_size+name_size, ext, ext_size+1);
        dl = dlopen(tmp, RTLD_NOW | RTLD_GLOBAL);
        path = l;
    } while (dl == NULL);

    if (dl == NULL) {
        fprintf(stderr, "try open %s failed : %s\r\n", name, dlerror());
    }

    return dl;
}


static int _insert(qtk_zeus_modules_t* zmods, qtk_zmodule_t *mod) {
    if (zmods->count >= QTK_ZMODULE_MAX) return -1;
    zmods->m[zmods->count] = *mod;
    ++zmods->count;
    return 0;
}


static qtk_zmodule_t* _query(qtk_zeus_modules_t *zmods, const char *name) {
    int i;
    for (i = 0; i < zmods->count; ++i) {
        if (strcmp(zmods->m[i].name, name) == 0) {
            return zmods->m + i;
        }
    }
    return NULL;
}


static void* _get_api(qtk_zmodule_t *mod, const char *api_name) {
    size_t name_size = strlen(mod->name);
    size_t api_size = strlen(api_name);
    char tmp[name_size + api_size + 1];
    memcpy(tmp, mod->name, name_size);
    memcpy(tmp+name_size, api_name, api_size+1);
    char *p = strrchr(tmp, '.');
    if (p) {
        ++p;
    } else {
        p = tmp;
    }
    return dlsym(mod->module, p);
}


static int _open_sym(qtk_zmodule_t *mod) {
    mod->create = _get_api(mod, "_create");
    mod->init = _get_api(mod, "_init");
    mod->release = _get_api(mod, "_release");
    return !(mod->create && mod->init && mod->release);
}


qtk_zeus_modules_t* qtk_zmodule_new(const char *path) {
    int sz = strlen(path) + 1;
    qtk_zeus_modules_t *zmods = wtk_malloc(sizeof(*zmods) + sz);
    wtk_lock_init(&zmods->lock);
    zmods->count = 0;
    memcpy(zmods+1, path, sz);
    zmods->path = (const char*)(zmods + 1);
    memset(zmods->m, 0, sizeof(zmods->m));
    return zmods;
}


int qtk_zmodule_delete(qtk_zeus_modules_t* zmods) {
    int i;
    for (i = 0; i < zmods->count; ++i) {
        wtk_free((void*)zmods->m[i].name);
        dlclose(zmods->m[i].module);
    }
    wtk_lock_clean(&zmods->lock);
    wtk_free(zmods);
    return 0;
}


int qtk_zmodule_insert(qtk_zeus_modules_t* zmods, qtk_zmodule_t *mod) {
    wtk_lock_lock(&zmods->lock);
    if (NULL == _query(zmods, mod->name)) {
        _insert(zmods, mod);
    }
    wtk_lock_unlock(&zmods->lock);
    return 0;
}


qtk_zmodule_t *qtk_zmodule_query(qtk_zeus_modules_t *zmods, const char *name) {
    qtk_zmodule_t *mod = _query(zmods, name);
    if (mod) return mod;
    wtk_lock_lock(&zmods->lock);
    mod = _query(zmods, name);
    if (NULL == mod && zmods->count < QTK_ZMODULE_MAX) {
        int index = zmods->count;
        void *dl = _try_open(zmods, name);
        if (dl) {
            zmods->m[index].name = name;
            zmods->m[index].module = dl;
            if (!_open_sym(&zmods->m[index])) {
                int name_size = strlen(name);
                char *p;
                mod = zmods->m + index;
                p = wtk_malloc(name_size + 1);
                memcpy(p, name, name_size+1);
                mod->name = p;
                ++zmods->count;
            }
        }
    }
    wtk_lock_unlock(&zmods->lock);
    return mod;
}


void *qtk_zmodule_instance_create(qtk_zmodule_t *mod) {
    if (mod->create) {
        return mod->create();
    } else {
        return NULL;
    }
}


int qtk_zmodule_instance_init(qtk_zmodule_t *mod, void *inst, struct qtk_zcontext *ctx, const char *param) {
    if (mod->init) {
        return mod->init(inst, ctx, param);
    } else {
        return -1;
    }
}


void qtk_zmodule_instance_release(qtk_zmodule_t *mod, void *inst) {
    if (mod->release) {
        mod->release(inst);
    }
}
