#ifndef WTK_OS_WTK_SOCKETPROC_H_
#define WTK_OS_WTK_SOCKETPROC_H_
#include "wtk/core/wtk_type.h"
#include "wtk_pipeproc.h"
#include "wtk_fd.h"
#include "wtk_pid.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_socketproc wtk_socketproc_t;
#define wtk_socketproc_fd(s) ((s)->v[(s)->vi])

struct wtk_socketproc
{
	char *name;
	pid_t pid;
	int v[2];
	int vi;
	pipeproc_init_handler init;
	pipeproc_clean_handler clean;
	pipeproc_handler handler;
	void *user_data;
};

wtk_socketproc_t* wtk_socketproc_new(char* name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *data);
int wtk_socketproc_delete(wtk_socketproc_t* p);
int wtk_socketproc_start(wtk_socketproc_t* proc);
#ifdef __cplusplus
};
#endif
#endif
