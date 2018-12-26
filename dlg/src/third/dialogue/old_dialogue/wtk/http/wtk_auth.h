#ifndef WTK_AUTH_H_
#define WTK_AUTH_H_

#include "wtk/http/misc/httpc/wtk_httpc.h"
#include "wtk/http/misc/httpc/wtk_httpc_request.h"

#ifdef __cplusplus
extern "C" {
#endif

struct wtk_http_auth_cfg {
    char *usrid;
    char *appid;
    char *seckey;
    char *tm;
    char *sig;
    char *dev;
};
typedef struct wtk_http_auth_cfg wtk_http_auth_cfg_t;

struct wtk_http_auth {
    char *ip;
    char *port;
    char *usrid;
    char *appid;
    char *seckey;
};
typedef struct wtk_http_auth wtk_http_auth_t;

int wtk_http_auth(wtk_http_auth_t *cfg);

#ifdef __cplusplus
};
#endif

#endif

