#ifndef WTK_HTTP_PP_WTK_PP_CFG_H_
#define WTK_HTTP_PP_WTK_PP_CFG_H_
#include "wtk/core/cfg/wtk_cfg_file.h"
#include "wtk/os/wtk_socket.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_pp_cfg wtk_pp_cfg_t;
struct wtk_pp_cfg
{
	float cpu_frequency;	//MHZ
	int cpus;
	int time_relink;	//ms
	wtk_string_t ip;
	wtk_string_t port;
	wtk_string_t url;
	wtk_addrinfo_t *addr;
	unsigned use_touch:1;
	unsigned ht_enable:1;
};

int wtk_pp_cfg_init(wtk_pp_cfg_t *cfg);
int wtk_pp_cfg_clean(wtk_pp_cfg_t *cfg);
int wtk_pp_cfg_update(wtk_pp_cfg_t *cfg);
int wtk_pp_cfg_update_local(wtk_pp_cfg_t *cfg,wtk_local_cfg_t *lc);
#ifdef __cplusplus
};
#endif
#endif
