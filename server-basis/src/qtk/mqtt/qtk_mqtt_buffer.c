#include "qtk_mqtt_buffer.h"
#include "mqtt.h"
#include <assert.h>
#include "qtk/entry/qtk_socket.h"


qtk_mqtt_buffer_t* qtk_mqttbuf_new() {
    qtk_mqtt_buffer_t *mbuf = (qtk_mqtt_buffer_t*)wtk_calloc(sizeof(*mbuf), 1);
    return mbuf;
}


int qtk_mqttbuf_init(qtk_mqtt_buffer_t *buf, qtk_socket_t *s) {
    buf->s = s;
    return 0;
}


int qtk_mqttbuf_delete(qtk_mqtt_buffer_t *buf) {
    if (buf->buf) {
        wtk_strbuf_delete(buf->buf);
    }
    wtk_free(buf);
    return 0;
}


int qtk_mqttbuf_alloc_body(qtk_mqtt_buffer_t *buf) {
    if (buf->buf) {
        wtk_strbuf_reset(buf->buf);
        wtk_strbuf_expand(buf->buf, buf->length);
    } else {
        buf->buf = wtk_strbuf_new(buf->length, 1);
    }
    return 0;
}


int qtk_mqttbuf_full(qtk_mqtt_buffer_t *buf) {
    if (buf->buf) {
        assert(buf->buf->pos <= buf->length);
        return buf->buf->pos == buf->length;
    } else {
        return buf->length == 0;
    }
}


int qtk_mqttbuf_push(qtk_mqtt_buffer_t *mbuf, const char *buf, int len) {
    if (mbuf->buf) {
        if (len > mbuf->length) len = mbuf->length;
        wtk_strbuf_push(mbuf->buf, buf, len);
        return len;
    } else {
        return -1;
    }
}


int qtk_mqttbuf_send(qtk_mqtt_buffer_t *mbuf, qtk_socket_t *s) {
    int ret;
    uint8_t fixhdr[5];
    fixhdr[0] = mbuf->headbyte;
    ret = mqtt_itoa(mbuf->length, fixhdr+1);
    assert(ret >= 0);
    if (qtk_socket_send(s, (const char*)fixhdr, ret+1)) {
        return -1;
    }
    if (mbuf->buf && mbuf->buf->pos) {
        return qtk_socket_send(s, mbuf->buf->data, mbuf->buf->pos);
    }
    return 0;
}
