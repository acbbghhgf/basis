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

int qtk_xfer_send(qtk_xfer_msg_t *xmsg)
{
    qtk_sframe_method_t *method = xmsg->gate;
    qtk_sframe_msg_t *msg = NULL;
    if (NULL == method) return -1;
    msg = method->new(method, xmsg->type, xmsg->id);
    switch (xmsg->type) {
    case QTK_SFRAME_MSG_REQUEST:
        method->set_method(msg, xmsg->param);
        method->set_url(msg, xmsg->url, strlen(xmsg->url));
        break;
    case QTK_SFRAME_MSG_RESPONSE:
        method->set_status(msg, xmsg->param);
        break;
    default:
        goto err;
    }
    if (xmsg->body) method->add_body(msg, xmsg->body->data, xmsg->body->len);
    if (xmsg->content_type) method->add_header(msg, "Content-Type", xmsg->content_type);
    method->push(method, msg);
    return 0;
err:
    if (msg && method) method->delete(method, msg);
    return -1;
}
