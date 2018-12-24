#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "os/qtk_alloc.h"
#include "os/qtk_base.h"
#include "os/qtk_thread.h"
#include "main/qtk_context.h"

struct logger {
	FILE * handle;
	char * filename;
	int close;
};


struct logger *logger_create(void)
{
	struct logger * inst = qtk_malloc(sizeof(*inst));
	inst->handle = NULL;
	inst->close = 0;
	inst->filename = NULL;

	return inst;
}


void logger_release(struct logger *inst)
{
	if (inst->close) {
		fclose(inst->handle);
	}
	qtk_free(inst->filename);
	qtk_free(inst);
}


static int logger_cb(qtk_context_t *context, void *ud, int type, int session, uint32_t source, const void * msg, size_t sz)
{
	struct logger * inst = ud;
    fprintf(inst->handle, "%.*s\n", (int)sz, (char *)msg);
    fflush(inst->handle);
	return 0;
}


int logger_init(struct logger * inst, struct qtk_context *ctx, const char * parm)
{
	if (parm) {
		inst->handle = fopen(parm,"a");
		if (inst->handle == NULL) {
			return 1;
		}
		inst->filename = qtk_malloc(strlen(parm)+1);
		strcpy(inst->filename, parm);
		inst->close = 1;
	} else {
		inst->handle = stdout;
	}
	if (inst->handle) {
        ctx->cb = logger_cb;
        ctx->ud = inst;
		return 0;
	}
	return 1;
}
