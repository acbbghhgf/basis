#include "wtk_listen_cfg.h"
#include "wtk/os/wtk_socket.h"
#ifdef WIN32
#include <windows.h>
#include <ws2tcpip.h>
#endif

int wtk_listen_cfg_init(wtk_listen_cfg_t *cfg)
{
#ifdef WIN32
    cfg->backlog=SOMAXCONN;
#else
	cfg->backlog=511;
#endif
	cfg->reuse=1;
	cfg->loop_back=0;
	cfg->defer_accept=1;
	cfg->defer_accept_timeout=30;
	return 0;
}

int wtk_listen_cfg_clean(wtk_listen_cfg_t *cfg)
{
	return 0;
}

int wtk_listen_cfg_update_local(wtk_listen_cfg_t *cfg,wtk_local_cfg_t *lc)
{
	wtk_string_t *v;

	//wtk_local_cfg_update_cfg_i(lc,cfg,port,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,backlog,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,reuse,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,loop_back,v);
	wtk_local_cfg_update_cfg_b(lc,cfg,defer_accept,v);
	wtk_local_cfg_update_cfg_i(lc,cfg,defer_accept_timeout,v);
	return 0;
}

int wtk_listen_cfg_update(wtk_listen_cfg_t *cfg)
{
	return 0;
}
