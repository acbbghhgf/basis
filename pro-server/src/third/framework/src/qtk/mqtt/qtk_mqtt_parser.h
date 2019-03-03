#ifndef _QTK_HTTP_QTK_MQTT_PARSER_H
#define _QTK_HTTP_QTK_MQTT_PARSER_H
#include "wtk/core/wtk_queue.h"
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wtk_str.h"
#include "qtk/entry/qtk_parser.h"
#include "qtk_mqtt_buffer.h"

#ifdef __cplusplus
#extern "C" {
#endif

struct qtk_connection;

typedef struct qtk_mqtt_parser qtk_mqtt_parser_t;
typedef struct qtk_mqtt_parser_pack_handler qtk_mqtt_parser_pack_handler_t;
typedef int(*qtk_mqtt_parser_pack_f)(void *layer, qtk_mqtt_parser_t* parser);

typedef enum {
    MQTT_START = 0,
    MQTT_LENGTH,
    MQTT_BODY,
    MQTT_DONE,
} qtk_mqtt_parser_state_t;

struct qtk_mqtt_parser_pack_handler {
    qtk_mqtt_parser_pack_f empty;
    qtk_mqtt_parser_pack_f ready;
    qtk_mqtt_parser_pack_f unlink;
    void *layer;
};

struct qtk_mqtt_parser {
    QTK_PARSER
    wtk_queue_node_t n;
    wtk_strbuf_t *tmpbuf;
    qtk_mqtt_buffer_t *pack;
    qtk_mqtt_parser_state_t state;
    qtk_mqtt_parser_pack_handler_t pack_handler;
};

qtk_mqtt_parser_t* qtk_mqtt_parser_new();
int qtk_mqtt_parser_delete(qtk_mqtt_parser_t *p);
int qtk_mqtt_parser_init(qtk_mqtt_parser_t *p);
qtk_mqtt_parser_t* qtk_mqtt_parser_clone(qtk_mqtt_parser_t *p);
int qtk_mqtt_parser_reset(qtk_mqtt_parser_t *p);
int qtk_mqtt_parser_reset_state(qtk_mqtt_parser_t *p);
void qtk_mqtt_parser_set_handlers(qtk_mqtt_parser_t *p,
                                  qtk_mqtt_parser_pack_f empty,
                                  qtk_mqtt_parser_pack_f ready,
                                  qtk_mqtt_parser_pack_f unlink,
                                  void *layer);

#ifdef __cplusplus
}
#endif

#endif
