#ifndef _MAIN_QTK_XFER_H_
#define _MAIN_QTK_XFER_H_

#include "qtk/sframe/qtk_sframe.h"

typedef struct qtk_xfer_msg qtk_xfer_msg_t;
struct qtk_xfer_msg {
    int type;           // response or request
    int param;          // method or status, depend on type
    char *url;          // url
    int id;
    wtk_string_t *body;
    char *content_type;
    qtk_sframe_method_t *gate;
};

int qtk_xfer_parse_addr(const char *addr, char *ip, int *port);
int qtk_xfer_estab(qtk_sframe_method_t *method, const char *addr, int recon);
int qtk_xfer_destroy(qtk_sframe_method_t *method, int id);
int qtk_xfer_listen(qtk_sframe_method_t *method, const char *addr, int backlog);
int qtk_xfer_newPool(qtk_sframe_method_t *method, const char *addr, int nslot);
int qtk_xfer_send(qtk_xfer_msg_t *xmsg);

#endif
