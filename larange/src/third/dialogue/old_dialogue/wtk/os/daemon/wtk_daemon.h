#ifndef WTK_OS_DAEMON_WTK_DAEMON_H_
#define WTK_OS_DAEMON_WTK_DAEMON_H_
#include "wtk/os/wtk_proc.h"
#include "wtk/os/wtk_pid.h"
#include "wtk/os/wtk_sem.h"
#include "wtk/os/wtk_log.h"
#include "wtk_daemon_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_daemon wtk_daemon_t;
typedef int(*wtk_daemon_start_f)(void *usr_data);
typedef int(*wtk_daemon_stop_f)(void *usr_data);
typedef int(*wtk_daemon_join_f)(void *usr_data);
#define wtk_daemon_new_type(cfg,log,usr_data,type) wtk_daemon_new(cfg,log,usr_data,\
		(wtk_daemon_start_f)CAT(type,_start),\
		(wtk_daemon_stop_f)CAT(type,_stop),\
		(wtk_daemon_join_f)CAT(type,_join))


struct wtk_daemon
{
	wtk_daemon_cfg_t *cfg;
	wtk_log_t *log;
	wtk_sem_t stop_sem;
	pid_t master_pid;
	pid_t worker_pid;
	wtk_daemon_start_f start;
	wtk_daemon_stop_f stop;
	wtk_daemon_join_f join;
	void *usr_data;
	volatile unsigned run:1;
};

/**
 *	@brief:
 *		start: usually start server thread route;
 *		stop:  send stop hint to server thread route;
 *		join: wait server thread route to be end;
 */
wtk_daemon_t* wtk_daemon_new(wtk_daemon_cfg_t *cfg,
		wtk_log_t *log,
		void *usr_data,
		wtk_daemon_start_f start,
		wtk_daemon_stop_f stop,
		wtk_daemon_join_f join);
void wtk_daemon_delete(wtk_daemon_t *d);
int wtk_daemon_run(wtk_daemon_t *d);

//==================== Test Section ==================
int wtk_daemon_test(int argc,char **argv);
#ifdef __cplusplus
};
#endif
#endif
