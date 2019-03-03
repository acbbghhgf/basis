#ifndef QTK_ENTRY_QTK_WS_H
#define QTK_ENTRY_QTK_WS_H
#include "third/libwebsockets/libwebsockets.h"
#include "qtk_socket.h"
#include "qtk/websocket/qtk_ws_protobuf.h"

#ifdef __cplusplus
extern "C" {
#endif

struct qtk_event;

int qtk_ws_established(struct lws *wsi);
int qtk_ws_closed(struct lws *wsi, qtk_socket_t *s);
int qtk_ws_accept_cb(struct lws_context *context, struct qtk_event *ev);
int qtk_ws_handle_jwt(struct lws *wsi, struct per_session_data__protobuf *data);
int qtk_ws_raise_jwt_payload(struct lws *wsi, struct per_session_data__protobuf *data);

#ifdef __cplusplus
}
#endif

#endif
