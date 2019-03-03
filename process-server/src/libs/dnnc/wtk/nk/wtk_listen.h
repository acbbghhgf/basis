#ifndef WTK_NK_WTK_LISTEN
#define WTK_NK_WTK_LISTEN
#include "wtk/core/wtk_type.h" 
#include "wtk_listen_cfg.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_listen wtk_listen_t;
struct wtk_listen
{
	wtk_listen_cfg_t *cfg;
	int fd;
};

wtk_listen_t* wtk_listen_new(wtk_listen_cfg_t *cfg);
void wtk_listen_delete(wtk_listen_t *l);
int wtk_listen_start(wtk_listen_t *l);
void wtk_listen_stop(wtk_listen_t *l);
#ifdef __cplusplus
};
#endif
#endif
