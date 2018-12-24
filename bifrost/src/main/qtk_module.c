#include "qtk_module.h"
#include "os/qtk_spinlock.h"
#include "os/qtk_alloc.h"
#include "os/qtk_base.h"

#include <assert.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define MAX_MODULE_TYPE 32

struct modules {
	int count;
	struct qtk_spinlock lock;
	char *path;
	struct qtk_module m[MAX_MODULE_TYPE];
};

static struct modules *M = NULL;

static void *_try_open(struct modules *m, const char * name)
{
	const char *l;
	const char * path = m->path;
	size_t path_size = strlen(path);
	size_t name_size = strlen(name);

	int sz = path_size + name_size;
	//search path
	void * dl = NULL;
	char tmp[sz];
	do
	{
		memset(tmp,0,sz);
		while (*path == ';') path++;
		if (*path == '\0') break;
		l = strchr(path, ';');
		if (l == NULL) l = path + strlen(path);
		int len = l - path;
		int i;
		for (i=0;path[i]!='?' && i < len ;i++) {
			tmp[i] = path[i];
		}
		memcpy(tmp+i,name,name_size);
		if (path[i] == '?') {
			strncpy(tmp+i+name_size,path+i+1,len - i - 1);
		} else {
			qtk_debug("Invalid C service path\n");
			exit(1);
		}
        dl = dlopen(tmp, RTLD_LAZY | RTLD_GLOBAL);
		path = l;
	}while(dl == NULL);

	if (dl == NULL) {
		qtk_debug("try open %s failed : %s\n",name,dlerror());
	}

	return dl;
}


static struct qtk_module *_query(const char * name)
{
	int i;
	for (i=0;i<M->count;i++) {
		if (strcmp(M->m[i].name,name)==0) {
			return &M->m[i];
		}
	}
	return NULL;
}


static void *get_api(struct qtk_module *mod, const char *api_name)
{
	size_t name_size = strlen(mod->name);
	size_t api_size = strlen(api_name);
	char tmp[name_size + api_size + 1];
	memcpy(tmp, mod->name, name_size);
	memcpy(tmp+name_size, api_name, api_size+1);
	char *ptr = strrchr(tmp, '.');
	if (ptr == NULL) {
		ptr = tmp;
	} else {
		ptr = ptr + 1;
	}
	return dlsym(mod->module, ptr);
}


static int open_sym(struct qtk_module *mod)
{
	mod->create = get_api(mod, "_create");
	mod->init = get_api(mod, "_init");
	mod->release = get_api(mod, "_release");
	mod->signal = get_api(mod, "_signal");

	return mod->init == NULL;
}


struct qtk_module *qtk_module_query(const char * name)
{
	struct qtk_module * result = _query(name);
	if (result)
		return result;

	QTK_SPIN_LOCK(M)

	result = _query(name); // double check

	if (result == NULL && M->count < MAX_MODULE_TYPE) {
		int index = M->count;
		void * dl = _try_open(M,name);
		if (dl) {
			M->m[index].name = name;
			M->m[index].module = dl;

			if (open_sym(&M->m[index]) == 0) {
				M->m[index].name = strdup(name);
				M->count ++;
				result = &M->m[index];
			}
		}
	}

	QTK_SPIN_UNLOCK(M)

	return result;
}


void qtk_module_insert(struct qtk_module *mod)
{
	QTK_SPIN_LOCK(M)

	struct qtk_module * m = _query(mod->name);
	assert(m == NULL && M->count < MAX_MODULE_TYPE);
	int index = M->count;
	M->m[index] = *mod;
	++M->count;

	QTK_SPIN_UNLOCK(M)
}


void *qtk_module_instance_create(struct qtk_module *m)
{
	if (m->create) {
		return m->create();
	} else {
		return (void *)(intptr_t)(~0);
	}
}


int qtk_module_instance_init(struct qtk_module *m, void * inst,
                             struct qtk_context *ctx, const char * parm)
{
	return m->init(inst, ctx, parm);
}


void qtk_module_instance_release(struct qtk_module *m, void *inst)
{
	if (m->release) {
		m->release(inst);
	}
}


void qtk_module_instance_signal(struct qtk_module *m, void *inst, int signal)
{
	if (m->signal) {
		m->signal(inst, signal);
	}
}


void qtk_module_init(const char *path)
{
	struct modules *m = qtk_malloc(sizeof(*m));
	m->count = 0;
	m->path = strdup(path);

	QTK_SPIN_INIT(m)

	M = m;
}


void qtk_module_clean()
{
    struct modules *m = M;

    if (m) {
        QTK_SPIN_DESTROY(m);
        int i;
        for (i=0; i<m->count; i++) {
            qtk_free((char *)m->m[i].name);
            dlclose(m->m[i].module);
        }
        qtk_free(m->path);
        qtk_free(m);
        M = NULL;
    }
}
