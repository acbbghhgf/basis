#ifndef WTK_OS_WTK_PID_H_
#define WTK_OS_WTK_PID_H_
#include "wtk/core/wtk_type.h"
#include "wtk/os/wtk_proc.h"
#include "wtk/core/wtk_os.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
#else
#include <sys/types.h>
#include <sys/wait.h>
int pid_kill(pid_t pid);
int pid_setup_signal();
void pid_daemonize();
void pid_save_file(pid_t pid,char *fn);
void pid_delete_file(pid_t pid,char* pf);
int pid_from_file(char *fn);
#endif
#ifdef __cplusplus
};
#endif
#endif
