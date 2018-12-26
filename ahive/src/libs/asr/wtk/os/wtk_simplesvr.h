#ifndef WTK_OS_WTK_SIMPLESVR
#define WTK_OS_WTK_SIMPLESVR
#include "wtk/os/wtk_socket.h" 
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_simplesvr wtk_simplesvr_t;
typedef void (* wtk_simplesvr_process_f)(void *ths,int fd);

struct wtk_simplesvr
{
	void *ths;
	wtk_simplesvr_process_f process;
	int port;
	unsigned run:1;
};

wtk_simplesvr_t* wtk_simplesvr_new(int port);
void wtk_simplesvr_delete(wtk_simplesvr_t *svr);
void wtk_simplesvr_set_process(wtk_simplesvr_t *svr,void *ths,wtk_simplesvr_process_f process);
int wtk_simplesvr_run(wtk_simplesvr_t *svr);
#ifdef __cplusplus
};
#endif
#endif
