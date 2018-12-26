#ifndef WTK_CONF_WTK_CONF_NK_H_
#define WTK_CONF_WTK_CONF_NK_H_
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "wtk/http/nk/listen/wtk_listen_cfg.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_nk_cfg wtk_nk_cfg_t;
struct wtk_nk_cfg
{
	wtk_listen_cfg_t listen;
	int connection_cache;
	int max_connections;
	int rw_size;
	int max_read_count;
	int cpu_update_timeout;
	int poll_hint;
	int poll_timeout;
	unsigned loop:1;	//listen: INADDR_LOOPBACK
	unsigned passive:1;	//passive=0: listen fd is closed when connection clean;else not closed;
	unsigned debug:1;
	unsigned update_time:1;
	unsigned use_cpu:1;
    unsigned log_snd:1;
    unsigned log_rcv:1;
    unsigned log_event:1;
    unsigned log_connection:1;
    unsigned attach_test:1;
    unsigned use_cfile_cpu:1;
    unsigned prompt:1;
    unsigned use_pipe:1;	//use pip queue or not;
};

int wtk_nk_cfg_init(wtk_nk_cfg_t *cfg);
int wtk_nk_cfg_clean(wtk_nk_cfg_t *cfg);
int wtk_nk_cfg_update(wtk_nk_cfg_t *cfg);
int wtk_nk_cfg_update_local(wtk_nk_cfg_t *cfg,wtk_local_cfg_t *lc);
void wtk_nk_cfg_print(wtk_nk_cfg_t *cfg);
#ifdef __cplusplus
};
#endif
#endif
