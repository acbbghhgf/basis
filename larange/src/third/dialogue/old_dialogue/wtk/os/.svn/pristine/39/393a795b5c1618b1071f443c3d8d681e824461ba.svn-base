#ifndef WTK_OS_WTK_PIPEPROC_H_
#define WTK_OS_WTK_PIPEPROC_H_
#include <unistd.h>
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_stack.h"
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk_pid.h"
#include "wtk_fd.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pipeproc wtk_pipeproc_t;
typedef int (*pipeproc_init_handler)(void *user_data,wtk_pipeproc_t *proc);
typedef int (*pipeproc_clean_handler)(void *user_data);
typedef wtk_string_t* (*pipeproc_handler)(void *user_data,wtk_string_t *req);

struct wtk_pipeproc
{
	char *name;
	pid_t pid;
	int pipe_fds[2][2];	//[1][1] for child write,[0][1] for child read;[0][1] for parent write,[1][0] for parent read;
	int pipe_wfd_index;
	pipeproc_init_handler init;
	pipeproc_clean_handler clean;
	pipeproc_handler handler;
	void *user_data;
	unsigned sync:1;
	volatile unsigned run:1;
};

wtk_pipeproc_t* wtk_pipeproc_new(char *name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data);
wtk_pipeproc_t* wtk_pipeproc_new2(char *name,pipeproc_init_handler init,
		pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data,
		int interactive);
int wtk_pipeproc_delete(wtk_pipeproc_t *p);
int wtk_pipeproc_clean(wtk_pipeproc_t *p);
int wtk_pipeproc_init(wtk_pipeproc_t *p,char *name,pipeproc_init_handler init,pipeproc_clean_handler clean,pipeproc_handler handler,void *user_data);
int wtk_pipeproc_start(wtk_pipeproc_t* proc);
int wtk_pipeproc_join(wtk_pipeproc_t *proc);
int wtk_pipeproc_wait(wtk_pipeproc_t *proc, int timeout); /* timeout in ms */
int wtk_pipeproc_kill(wtk_pipeproc_t* proc);
int wtk_pipeproc_write_string(wtk_pipeproc_t* proc,char* buf,int len);
wtk_string_t* wtk_pipeproc_read_string(wtk_pipeproc_t *p);
int wtk_pipeproc_read_string2(wtk_pipeproc_t *p,wtk_strbuf_t *buf);
int wtk_pipeproc_write_stack(wtk_pipeproc_t* proc,wtk_stack_t *s);
int wtk_pipeproc_read_stack(wtk_pipeproc_t *p,wtk_stack_t *stack);
void wtk_pipeproc_set_process_init_handler(pipeproc_init_handler handler,void *data);
#ifdef __cplusplus
};
#endif
#endif
