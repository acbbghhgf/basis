#ifndef WTK_OS_WTK_THREAD_H_
#define WTK_OS_WTK_THREAD_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_sem.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_thread wtk_thread_t;
typedef int (*thread_route_handler)(void *data,wtk_thread_t *t);

typedef enum
{
	THREAD_STATE_INIT,
	THREAD_STATE_RUN,
	THREAD_STATE_EXIT,
}wtk_thread_state;

struct wtk_thread
{
	wtk_sem_t sem;
	wtk_thread_state state;
#ifdef WIN32
    HANDLE handler;
    DWORD ppid;
#else
	pthread_t handler;
	pid_t ppid;
#endif
	thread_route_handler route;
	void *data;
	void *app_data;				//used for attach thread data;
	char *name;
};

int wtk_thread_init(wtk_thread_t *t,thread_route_handler route,void *data);
int wtk_thread_clean(wtk_thread_t *t);
int wtk_thread_start(wtk_thread_t *t);
int wtk_thread_join(wtk_thread_t *t);
int wtk_thread_kill(wtk_thread_t *t);
void wtk_thread_set_name(wtk_thread_t *t,char *name);

typedef int(*wtk_thread_route_f)(void *ths);
typedef int(*wtk_thread_create_f)(wtk_thread_route_f route,void *ths);
void wtk_thread_g_set_glb_create(wtk_thread_create_f cf);
#ifdef __cplusplus
};
#endif
#endif
