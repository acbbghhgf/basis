#ifndef _MAIN_QTK_XFER_H_
#define _MAIN_QTK_XFER_H_

#include "qtk/sframe/qtk_sframe.h"

int qtk_xfer_parse_addr(const char *addr, char *ip, int *port);
int qtk_xfer_estab(qtk_sframe_method_t *method, const char *addr, int recon);
int qtk_xfer_destroy(qtk_sframe_method_t *method, int id);
int qtk_xfer_listen(qtk_sframe_method_t *method, const char *addr, int backlog);
int qtk_xfer_newPool(qtk_sframe_method_t *method, const char *addr, int nslot);

#endif
