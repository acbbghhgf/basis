#ifndef _QTK_MQTT_QTK_MQTT_BUFFER_H
#define _QTK_MQTT_QTK_MQTT_BUFFER_H
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"

#ifdef __cplusplus
#extern "C" {
#endif

struct qtk_socket;

typedef struct qtk_mqtt_buffer qtk_mqtt_buffer_t;

struct qtk_mqtt_buffer {
    wtk_queue_node_t buf_n;
    struct qtk_socket* s;
    uint8_t headbyte;
    uint32_t length;
    wtk_strbuf_t *buf;
};

qtk_mqtt_buffer_t* qtk_mqttbuf_new();
int qtk_mqttbuf_init(qtk_mqtt_buffer_t *buf, struct qtk_socket *s);
int qtk_mqttbuf_delete(qtk_mqtt_buffer_t *buf);
int qtk_mqttbuf_alloc_body(qtk_mqtt_buffer_t *buf);
int qtk_mqttbuf_push(qtk_mqtt_buffer_t *mbuf, const char *buf, int len);
int qtk_mqttbuf_full(qtk_mqtt_buffer_t *buf);
int qtk_mqttbuf_send(qtk_mqtt_buffer_t *mbuf, struct qtk_socket *s);


#ifdef __cplusplus
}
#endif


#endif
