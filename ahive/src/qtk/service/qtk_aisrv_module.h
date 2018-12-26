#ifndef QTK_SERVICE_QTK_AISRV_MODULE_H
#define QTK_SERVICE_QTK_AISRV_MODULE_H
#include "wtk/core/wtk_str.h"
#include "wtk/core/cfg/wtk_cfg_file.h"

#ifdef __cplusplus
extern "C" {
#endif

enum qtk_aisrv_cmd{
    QTK_AISRV_CMD_START = 0,
    QTK_AISRV_CMD_FEED,
    QTK_AISRV_CMD_STOP,
    QTK_AISRV_CMD_GET_RESULT,
};

enum qtk_aisrv_errno {
    QTK_AISRV_FAILURE = -1,
    QTK_AISRV_SUCCESS = 0,
    QTK_AISRV_SUCCESS_EOF = 1,
    QTK_AISRV_EMPTY_RESULT = 2,
    QTK_AISRV_START_ERROR = 61000,
    QTK_AISRV_FEED_ERROR,
    QTK_AISRV_STOP_ERROR,
    QTK_AISRV_INVALID_PARAM,
};

typedef struct qtk_aisrv_mod qtk_aisrv_mod_t;
typedef int (*qtk_aisrv_openmod_f)(qtk_aisrv_mod_t*);
typedef void* (*qtk_aisrv_init_f)(wtk_local_cfg_t*);
typedef void (*qtk_aisrv_clean_f)(void*);
typedef void* (*qtk_aisrv_create_f)(void*);
typedef int (*qtk_aisrv_release_f)(void*);
typedef int (*qtk_aisrv_start_f)(void*, wtk_string_t*);
typedef int (*qtk_aisrv_feed_f)(void*, wtk_string_t*);
typedef int (*qtk_aisrv_stop_f)(void*);
typedef int (*qtk_aisrv_get_result_f)(void*, wtk_string_t*);



struct qtk_aisrv_mod {
    wtk_string_t *name;
    void *handle;
    qtk_aisrv_init_f init;
    qtk_aisrv_clean_f clean;
    qtk_aisrv_create_f create;
    qtk_aisrv_start_f start;
    qtk_aisrv_feed_f feed;
    qtk_aisrv_get_result_f get_result;
    qtk_aisrv_stop_f stop;
    qtk_aisrv_release_f release;
};


void qtk_aisrv_set_auth_port(int port);
void qtk_aisrv_set_auth_host(const char *host);
void qtk_aisrv_set_auth_url(const char *url);

int qtk_aisrv_get_auth_port();
const char* qtk_aisrv_get_auth_host();
const char* qtk_aisrv_get_auth_url();


#ifdef __cplusplus
}
#endif

#endif
