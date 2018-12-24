#include "qtk_xfer.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// ip must has enough space
int qtk_xfer_parse_addr(const char *addr, char *ip, int *port)
{
    char tmp[64];
    snprintf(tmp, sizeof(tmp), "%s", addr);
    char *colon = strchr(tmp, ':');
    if (colon == NULL) return -1;
    *colon = '\0';
    strcpy(ip, tmp);
    *port = atoi(colon + 1);
    if (*port <= 0) return -1;
    return 0;
}

int qtk_xfer_estab(qtk_sframe_method_t *method, const char *addr, int reconnect)
{
    char ip[64];
    int port;
    if (qtk_xfer_parse_addr(addr, ip, &port))   return -1;
    int id = method->socket(method);
    if (id < 0) return -1;
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_CMD, id);
    qtk_sframe_connect_param_t *param = CONNECT_PARAM_NEW(ip, port, reconnect);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, param);
    method->push(method, msg);
    return id;
}

int qtk_xfer_destroy(qtk_sframe_method_t *method, int id)
{
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_CMD, id);
    method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
    method->push(method, msg);
    return 0;
}

int qtk_xfer_listen(qtk_sframe_method_t *method, const char *addr, int backlog)
{
    int id = method->socket(method);
    if (id < 0) return -1;
    char ip[64];
    int port;
    if (qtk_xfer_parse_addr(addr, ip, &port))   return -1;
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_CMD, id);
    qtk_sframe_listen_param_t *param = LISTEN_PARAM_NEW(ip, port, backlog);
    method->set_cmd(msg, QTK_SFRAME_CMD_LISTEN, param);
    method->push(method, msg);
    return id;
}

int qtk_xfer_newPool(qtk_sframe_method_t *method, const char *addr, int nslot)
{
    int id = method->request_pool(method);
    if (id < 0) return -1;
    char ip[64];
    int port;
    if (qtk_xfer_parse_addr(addr, ip, &port))   return -1;
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_CMD, -id);
    qtk_sframe_connect_param_t *param = CONNECT_PARAM_NEW(ip, port, nslot);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN_POOL, param);
    method->push(method, msg);
    return id;
}
