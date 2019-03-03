#include "qtk_ws.h"
#include "qtk_entry.h"
#include "qtk_socket.h"
#include "qtk/util/qtk_jwt.h"
#include "third/libwebsockets/private-libwebsockets.h"


int qtk_ws_established(struct lws *wsi) {
    qtk_entry_t *entry = lws_context_user(wsi->context);
    int id = qtk_entry_alloc_id(entry, 0);
    qtk_socket_t *cli = qtk_entry_get_socket(entry, id);
    qtk_sframe_msg_t *msg = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGCONNECT, id);
    qtk_socket_init_ws(cli, entry, wsi);
    cli->state = SOCKET_CONNECTED;
    qtk_entry_raise_msg(entry, msg);
    return id;
}


/* return non-zero will cause auth failure */
int qtk_ws_handle_jwt(struct lws *wsi, struct per_session_data__protobuf *data)
{
    int len = lws_hdr_total_length(wsi, WSI_TOKEN_HTTP_AUTHORIZATION);
    char buf[4096];
    if (!len || len > sizeof(buf) - 1) {
        wtk_debug("missing AUTHORIZATION or it is too long\n");
        return -1;
    }
    lws_hdr_copy(wsi, buf, sizeof(buf), WSI_TOKEN_HTTP_AUTHORIZATION);
    int pass = 0;
    char payload[4096];
    qtk_jwt_verify(buf, payload, sizeof(payload), &pass);
    if (data->jwt_payload) {
        wtk_debug("This Maybe a bug\n");
        wtk_string_delete(data->jwt_payload);
        data->jwt_payload = NULL;
    }
    data->jwt_payload = wtk_string_dup_data_pad0(payload, strlen(payload));
    return pass ? 0 : 1;
}


int qtk_ws_raise_jwt_payload(struct lws *wsi, struct per_session_data__protobuf *data)
{
    if (NULL == data->jwt_payload)
        return -1;
    qtk_entry_t *entry = lws_context_user(wsi->context);
    qtk_sframe_msg_t *msg = qtk_fwd_jwt_payload(entry->frame, data->sock_id,
                                                data->jwt_payload->data, data->jwt_payload->len);
    qtk_entry_raise_msg(entry, msg);
    return 0;
}


int qtk_ws_closed(struct lws *wsi, qtk_socket_t *s) {
    qtk_entry_t *entry = lws_context_user(wsi->context);
    qtk_sframe_msg_t *msg = qtk_fwd_notice_msg(entry->frame, QTK_SFRAME_SIGDISCONNECT, s->id);
    qtk_entry_raise_msg(entry, msg);
    qtk_socket_close(s);
    return 0;
}


int qtk_ws_accept_cb(struct lws_context *context, qtk_event_t *ev) {
    qtk_event2_t *ev2 = lws_container_of(ev, qtk_event2_t, ev);
    struct lws_pollfd eventfd;

    if (ev2->ev.error) return -1;

    eventfd.fd = ev2->fd;
    eventfd.events = LWS_POLLIN;
    eventfd.revents = LWS_POLLIN;

    lws_service_fd(context, &eventfd);
    return 0;
}
