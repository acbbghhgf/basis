#include "qtk_mqtt_parser.h"
#include <ctype.h>
#include "qtk/entry/qtk_socket.h"
#include "qtk_mqtt_buffer.h"
#include "mqtt.h"


static int qtk_mqtt_parser_claim_buffer(qtk_mqtt_parser_t *p, qtk_socket_t *s) {
    if (!p->pack) {
        if (p->pack_handler.empty) {
            if (p->pack_handler.empty(p->pack_handler.layer, p)) return -1;
            qtk_mqttbuf_init(p->pack, s);
        }
    }
    return 0;
}


static int qtk_mqtt_parser_raise(qtk_mqtt_parser_t *p) {
    if (p->pack) {
        if (p->pack_handler.ready) {
            if (p->pack_handler.ready(p->pack_handler.layer, p)) return -1;
        }
        p->pack = NULL;
    }
    qtk_mqtt_parser_reset_state(p);
    return 0;
}


static int qtk_mqtt_parser_unlink(qtk_mqtt_parser_t *p) {
    if (p->pack) {
        if (MQTT_DONE == p->state && p->pack_handler.ready) {

        }
        if (p->pack_handler.unlink) {
            p->pack_handler.unlink(p->pack_handler.layer, p);
        }
        p->pack = NULL;
    }
    return 0;
}


static int qtk_mqtt_head_valid(uint8_t b) {
    switch (b>>4) {
    case MQTT_PUBREL:
    case MQTT_SUBSCRIBE:
    case MQTT_UNSUBSCRIBE:
        return (b&0x0f) == 0x02;
    case MQTT_PUBLISH:
        return 1;
    case MQTT_CONNECT:
    case MQTT_CONNACK:
    case MQTT_PUBACK:
    case MQTT_PUBREC:
    case MQTT_PUBCOMP:
    case MQTT_SUBACK:
    case MQTT_UNSUBACK:
    case MQTT_PINGREQ:
    case MQTT_PINGRESP:
    case MQTT_DISCONNECT:
        return (b&0x0f) == 0;
    default:
        return 0;
    }
}


static int qtk_mqtt_parser_feed(qtk_mqtt_parser_t *p, qtk_socket_t *sock, char *buf, int len) {
    char *s = buf, *e = buf + len;
    int ret = 0;
    uint32_t u;                 /* for length calc */
    while (s < e) {
        char ch = *s;
        switch (p->state) {
        case MQTT_START:
            if (qtk_mqtt_head_valid(ch)) {
                qtk_mqtt_parser_claim_buffer(p, sock);
                p->pack->headbyte = ch;
                p->state = MQTT_LENGTH;
            } else {
            }
            break;
        case MQTT_LENGTH:
            u = ch & 0x7f;
            p->pack->length += u << (7*p->tmpbuf->pos);
            wtk_strbuf_push_c(p->tmpbuf, ch);
            if (!(ch & 0x80)) {
                qtk_mqttbuf_alloc_body(p->pack);
                p->state = MQTT_BODY;
            } else if (p->tmpbuf->pos >= 4) {
                wtk_debug("invalid mqtt length\r\n");
                ret = -1;
                goto end;
            }
            break;
        case MQTT_BODY:
        {
            int r;
            r = qtk_mqttbuf_push(p->pack, s, e-s);
            if (r > 0) {
                s += r;
            } else {
                ret = -1;
            }
            if (qtk_mqttbuf_full(p->pack)) {
                qtk_mqtt_parser_raise(p);
            }
            continue;
        }
        default:
            wtk_debug("unexcepted state %d.\n", p->state);
            ret = -1;
            goto end;
        }
        ++s;
    }
end:
    return ret == 0 ? e - s : ret;
}


static int qtk_mqtt_parser_close(qtk_mqtt_parser_t *p) {
    wtk_strbuf_delete(p->tmpbuf);
    qtk_mqtt_parser_reset(p);
    return 0;
}


qtk_mqtt_parser_t* qtk_mqtt_parser_new() {
    qtk_mqtt_parser_t *p;

    p = (qtk_mqtt_parser_t*)wtk_calloc(1, sizeof(*p));
    p->tmpbuf = wtk_strbuf_new(4, 1);
    p->reset_handler = (qtk_parser_reset_handler)qtk_mqtt_parser_reset_state;
    p->close_handler = (qtk_parser_reset_handler)qtk_mqtt_parser_close;
    p->clone_handler = (qtk_parser_clone_handler)qtk_mqtt_parser_clone;
    qtk_mqtt_parser_init(p);

    return p;
}


qtk_mqtt_parser_t* qtk_mqtt_parser_clone(qtk_mqtt_parser_t *p) {
    qtk_mqtt_parser_t *parser;
    parser = (qtk_mqtt_parser_t*)wtk_malloc(sizeof(*p));
    *parser = *p;
    parser->tmpbuf = wtk_strbuf_new(4, 1);
    parser->pack = NULL;
    qtk_mqtt_parser_reset_state(parser);

    return parser;
}


int qtk_mqtt_parser_delete(qtk_mqtt_parser_t *p) {
    qtk_mqtt_parser_close(p);
    wtk_free(p);
    return 0;
}


int qtk_mqtt_parser_init(qtk_mqtt_parser_t *p) {
    p->handler = (qtk_parser_handler)qtk_mqtt_parser_feed;
    p->pack = NULL;
    memset(&p->pack_handler, 0, sizeof(p->pack_handler));

    return 0;
}


int qtk_mqtt_parser_reset(qtk_mqtt_parser_t *p) {
    qtk_mqtt_parser_unlink(p);
    qtk_mqtt_parser_init(p);

    return 0;
}


int qtk_mqtt_parser_reset_state(qtk_mqtt_parser_t *p) {
    qtk_mqtt_parser_unlink(p);
    p->state = MQTT_START;
    wtk_strbuf_reset(p->tmpbuf);

    return 0;
}


void qtk_mqtt_parser_set_handlers(qtk_mqtt_parser_t *p,
                                  qtk_mqtt_parser_pack_f empty,
                                  qtk_mqtt_parser_pack_f ready,
                                  qtk_mqtt_parser_pack_f unlink,
                                  void *layer) {
    p->pack_handler.empty = empty;
    p->pack_handler.ready = ready;
    p->pack_handler.unlink = unlink;
    p->pack_handler.layer = layer;
}
