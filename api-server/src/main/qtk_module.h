#ifndef MAIN_QTK_MODULE_H_
#define MAIN_QTK_MODULE_H_

struct qtk_context;

typedef void * (*qtk_dl_create)(void);
typedef int (*qtk_dl_init)(void * inst, struct qtk_context *, const char * parm);
typedef void (*qtk_dl_release)(void * inst);
typedef void (*qtk_dl_signal)(void * inst, int signal);
typedef struct qtk_module qtk_module_t;

struct qtk_module {
	const char *name;
	void *module;
	qtk_dl_create create;
	qtk_dl_init init;
	qtk_dl_release release;
	qtk_dl_signal signal;
};

void qtk_module_insert(struct qtk_module *mod);
struct qtk_module * qtk_module_query(const char * name);
void * qtk_module_instance_create(struct qtk_module *);
int qtk_module_instance_init(struct qtk_module *, void * inst, struct qtk_context *ctx, const char * parm);
void qtk_module_instance_release(struct qtk_module *, void *inst);
void qtk_module_instance_signal(struct qtk_module *, void *inst, int signal);

void qtk_module_init(const char *path);
void qtk_module_clean();

#endif
