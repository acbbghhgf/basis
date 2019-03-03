#include <stdio.h>
#include <assert.h>
#include "mqtt.h"
#include "qtk/core/qtk_deque.h"


typedef int (*MQTT_Header_Pack)(MQTT_Package_t *, qtk_deque_t *);
typedef int (*MQTT_Header_Unpack)(const uint8_t *, int, MQTT_Package_t *);
typedef int (*MQTT_Payload_Pack)(MQTT_Package_t *, qtk_deque_t *);
typedef int (*MQTT_Payload_Unpack)(const uint8_t *, int, MQTT_Package_t *);


static int connect_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int connect_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);
static int connect_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int connect_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int connack_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int connack_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int publish_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int publish_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);
static int publish_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int publish_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int puback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int puback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int pubrec_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int pubrec_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int pubrel_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int pubrel_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int pubcomp_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int pubcomp_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int subscribe_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int subscribe_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);
static int subscribe_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int subscribe_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int suback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int suback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);
static int suback_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int suback_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int unsubscribe_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int unsubscribe_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);
static int unsubscribe_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int unsubscribe_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);

static int unsuback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq);
static int unsuback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack);


struct MQTT_Method {
    MQTT_Header_Pack hdr_pack;
    MQTT_Header_Unpack hdr_unpack;
    MQTT_Payload_Pack pld_pack;
    MQTT_Payload_Unpack pld_unpack;
};


static struct MQTT_Method methods[16] = {
    {                           /* RESERVE */
        NULL,
    },
    {                           /* CONNECT */
        connect_hdr_pack,
        connect_hdr_unpack,
        connect_pld_pack,
        connect_pld_unpack,
    },
    {                           /* CONNACK */
        connack_hdr_pack,
        connack_hdr_unpack,
        NULL,
        NULL,
    },
    {                           /* PUBLISH */
        publish_hdr_pack,
        publish_hdr_unpack,
        publish_pld_pack,
        publish_pld_unpack,
    },
    {                           /* PUBACK */
        puback_hdr_pack,
        puback_hdr_unpack,
        NULL,
        NULL,
    },
    {                           /* PUBREC */
        pubrec_hdr_pack,
        pubrec_hdr_unpack,
        NULL,
        NULL,
    },
    {                           /* PUBREL */
        pubrel_hdr_pack,
        pubrel_hdr_unpack,
        NULL,
        NULL,
    },
    {                           /* PUBCOMP */
        pubcomp_hdr_pack,
        pubcomp_hdr_unpack,
        NULL,
        NULL,
    },
    {                           /* SUBSCRIBE */
        subscribe_hdr_pack,
        subscribe_hdr_unpack,
        subscribe_pld_pack,
        subscribe_pld_unpack,
    },
    {                           /* SUBACK */
        suback_hdr_pack,
        suback_hdr_unpack,
        suback_pld_pack,
        suback_pld_unpack,
    },
    {                           /* UNSUBSCRIBE */
        unsubscribe_hdr_pack,
        unsubscribe_hdr_unpack,
        unsubscribe_pld_pack,
        unsubscribe_pld_unpack,
    },
    {                           /* UNSUBACK */
        unsuback_hdr_pack,
        unsuback_hdr_unpack,
    },
    {                           /* PINGREQ */
        NULL,
    },
    {                           /* PINGRESP */
        NULL,
    },
    {                           /* DISCONNECT */
        NULL,
    },
};


int mqtt_atoi(const uint8_t* buf, int len, uint32_t *n) {
    int i;
    int multi = 1;
    int ret = 0;
    if (len > 4) return -1;
    for (i = 0; i < len; i++) {
        ret += (buf[i] & 0x7f) * multi;
        multi *= 128;
        if (!(buf[i] & 0x80)) {
            *n = ret;
            break;
        }
    }
    return i < len ? i+1 : -1;
}


int mqtt_itoa(uint32_t n, uint8_t *buf) {
    int ret = 0;
    if (n >> 28) {
        /* n is too large */
        return -1;
    }
    if (n == 0) {
        buf[0] = 0;
        return 1;
    }
    while (n) {
        uint8_t b7 = n & 0x7f;
        n >>= 7;
        if (n) {
            buf[ret] = b7 | 0x80;
        } else {
            buf[ret] = b7;
        }
        ret++;
    }
    return ret;
}


int mqtt_pack_var(MQTT_Package_t *pack, uint8_t type, qtk_deque_t *dbuf) {
    int ret;
    int offset = 0;
    if (methods[type].hdr_pack) {
        ret = methods[type].hdr_pack(pack, dbuf);
        if (ret < 0) {
            offset = -1;
            goto end;
        }
        offset = ret;
    }
    if (methods[type].pld_pack) {
        ret = methods[type].pld_pack(pack, dbuf);
        if (ret < 0) {
            offset = -1;
            goto end;
        }
        offset += ret;
    }
end:
    return offset;
}


int mqtt_pack(MQTT_Package_t *pack, qtk_deque_t *dbuf) {
    uint32_t type = pack->type;
    int n;
    uint8_t lenbuf[4];
    uint8_t b1;
    qtk_deque_reset(dbuf);
    if (methods[type].hdr_pack) {
        n = methods[type].hdr_pack(pack, dbuf);
        if (n < 0) {
            return -1;
        }
    }
    if (methods[type].pld_pack) {
        n = methods[type].pld_pack(pack, dbuf);
        if (n < 0) {
            return -1;
        }
    }
    n = mqtt_itoa(dbuf->len, lenbuf);
    if (n < 0) {
        return -1;
    }
    qtk_deque_push_front(dbuf, (const char*)lenbuf, n);
    b1 = type<<4 | pack->flag;
    qtk_deque_push_front(dbuf, (const char*)&b1, 1);
    return dbuf->len;
}


int mqtt_unpack_var(const uint8_t *buf, int n, uint8_t type, MQTT_Package_t *pack) {
    int ret;
    int offset = 0;
    if (methods[type].hdr_unpack) {
        ret = methods[type].hdr_unpack(buf, n, pack);
        if (ret < 0) return -1;
        offset += ret;
    }
    if (methods[type].pld_unpack) {
        ret = methods[type].pld_unpack(buf+offset,
                                       n-offset,
                                       pack);
        if (ret < 0) return -1;
        offset += ret;
    }
    return offset;
}


int mqtt_unpack(const uint8_t *buf, int n, MQTT_Package_t *pack) {
    int plen;
    int ret;
    uint8_t type;
    if (n < 2) {
        return 1;               /* to again */
    }
    type = buf[0] >> 4;
    pack->type = type;
    pack->flag = buf[0] & 0x0f;
    ret = mqtt_atoi(buf+1, n-1, (uint32_t*)&plen);
    if (n > 5 && ret < 0) {
        return -1;
    } else if (ret < 0 || plen > n-1-ret) {
        return 1;
    } else {
        int offset = ret+1;
        int hdr_len = 0;
        if (methods[type].hdr_unpack) {
            ret = methods[type].hdr_unpack(buf+offset, plen, pack);
            if (ret < 0) return -1;
            hdr_len = ret;
            offset += ret;
            plen -= ret;
        }
        if (methods[type].pld_unpack) {
            ret = methods[type].pld_unpack(buf+offset,
                                           pack->length-hdr_len,
                                           pack);
            if (ret < 0) return -1;
            offset += ret;
        }
        return offset;
    }
    return 0;
}


MQTT_TopicFilter_t* mqtt_topicfilter_new() {
    MQTT_TopicFilter_t *tf = wtk_calloc(1, sizeof(*tf));
    return tf;
}


int mqtt_topicfilter_delete(MQTT_TopicFilter_t *tf) {
    if (tf->name.data) {
        wtk_free(tf->name.data);
    }
    wtk_free(tf);
    return 0;
}


MQTT_Package_t* mqtt_package_new(int type) {
    MQTT_Package_t *pack = wtk_calloc(1, sizeof(*pack));
    pack->type = type;
    return pack;
}


static int mqtt_string_delete(wtk_string_t *s) {
    if (s->data && (void*)s != s->data-sizeof(wtk_string_t)) {
        wtk_free(s->data);
    }
    wtk_free(s);
    return 0;
}


int mqtt_package_clean(MQTT_Package_t *pack) {
    switch (pack->type) {
    case MQTT_CONNECT:
#define STRING_DELETE(p) if (p) { mqtt_string_delete(p); p = NULL; }
        STRING_DELETE(pack->payload.connect.clientid)
        STRING_DELETE(pack->payload.connect.willtopic)
        STRING_DELETE(pack->payload.connect.willmsg)
        STRING_DELETE(pack->payload.connect.username)
        STRING_DELETE(pack->payload.connect.password)
        break;
    case MQTT_PUBLISH:
        STRING_DELETE(pack->headers.publish.topic);
        if (pack->payload.msg) {
            wtk_strbuf_delete(pack->payload.msg);
            pack->payload.msg = NULL;
        }
        break;
    case MQTT_SUBSCRIBE:
    case MQTT_UNSUBSCRIBE:
    {
        wtk_queue_node_t *n;
        wtk_queue_t *q = &pack->payload.subscribe.topics;
        while ((n = wtk_queue_pop(q))) {
            MQTT_TopicFilter_t *tf = data_offset(n, MQTT_TopicFilter_t, node);
            mqtt_topicfilter_delete(tf);
        }
        break;
    }
    case MQTT_SUBACK:
        STRING_DELETE(pack->payload.suback.codes);
        break;
    }
    return 0;
}


int mqtt_package_delete(MQTT_Package_t *pack) {
    mqtt_package_clean(pack);
    wtk_free(pack);
    return 0;
}


static void mqtt_word_pack(uint16_t u, qtk_deque_t *dq) {
    uint8_t b[2];
    b[1] = u & 0xff;
    b[0] = (u>>8) & 0xff;
    qtk_deque_push(dq, (const char*)b, 2);
}


static void mqtt_str_pack(const char *buf, int len, qtk_deque_t *dq) {
    mqtt_word_pack(len, dq);
    qtk_deque_push(dq, buf, len);
}


static void mqtt_word_unpack(const uint8_t *buf, int len, uint16_t *u) {
    assert(len >= 2);
    *u = buf[0];
    *u = (*u<<8) | buf[1];
}


static int mqtt_str_unpack(const uint8_t *buf, int len, wtk_string_t *s) {
    uint16_t u;
    if (len <= 2) return -1;
    mqtt_word_unpack(buf, 2, &u);
    if (u > len-2) return -1;
    if (s->data) {
        if (s->len < u) {
            wtk_free(s->data);
            s->data = (char*)wtk_malloc(u);
        }
    } else {
        s->data = (char*)wtk_malloc(u);
    }
    if (s->data == NULL) return -1;
    memcpy(s->data, buf+2, u);
    s->len = u;
    return u+2;
}


static int connect_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    const char *pn = pack->headers.connect.proto_name;
    uint8_t flag = 0;
    int ret = strlen(pn);
    mqtt_str_pack(pn, ret, dq);
    qtk_deque_push(dq, (const char*)&pack->headers.connect.proto_level, 1);
    flag = (pack->headers.connect.username<<7)              \
        | (pack->headers.connect.password<<6)               \
        | (pack->headers.connect.willretn<<5)               \
        | (pack->headers.connect.willqos<<3)                \
        | (pack->headers.connect.willflag<<2)               \
        | (pack->headers.connect.cleanses<<1);
    qtk_deque_push(dq, (const char*)&flag, 1);
    mqtt_word_pack(pack->headers.connect.keepalive, dq);
    return ret + 6;
}


static int connect_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    int ret;
    uint8_t flag;
    wtk_string_t s = {NULL, 0};
    ret = mqtt_str_unpack(buf, len, &s);
    if (s.data == NULL) return -1;
    memcpy(pack->headers.connect.proto_name, s.data, s.len);
    pack->headers.connect.proto_name[s.len] = '\0';
    wtk_free(s.data);
    if (len < ret+4) return -1;
    pack->headers.connect.proto_level = buf[ret];
    flag = buf[ret+1];
    pack->headers.connect.username = flag>>7;
    pack->headers.connect.password = (flag>>6) & 1;
    pack->headers.connect.willretn = (flag>>5) & 1;
    pack->headers.connect.willqos = (flag>>4) & 3;
    pack->headers.connect.willflag = (flag>>2) & 1;
    pack->headers.connect.cleanses = (flag>>1) & 1;
    mqtt_word_unpack(buf+ret+2, 2, &pack->headers.connect.keepalive);
    return ret+4;
}


static int connect_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_string_t *clientid = pack->payload.connect.clientid;
    wtk_string_t *willtopic = pack->payload.connect.willtopic;
    wtk_string_t *willmsg = pack->payload.connect.willmsg;
    wtk_string_t *username = pack->payload.connect.username;
    wtk_string_t *password = pack->payload.connect.password;
    int ret = clientid->len + 2;
    mqtt_str_pack(clientid->data, clientid->len, dq);
    if (pack->headers.connect.willretn) {
        mqtt_str_pack(willtopic->data, willtopic->len, dq);
        mqtt_str_pack(willmsg->data, willmsg->len, dq);
        ret += willtopic->len + willmsg->len + 4;
    }
    if (pack->headers.connect.username) {
        mqtt_str_pack(username->data, username->len, dq);
        ret += username->len + 2;
    }
    if (pack->headers.connect.password) {
        mqtt_str_pack(password->data, password->len, dq);
        ret += password->len + 2;
    }
    return ret;
}


static int connect_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    int n;
    int ret = mqtt_str_unpack(buf, len, pack->payload.connect.clientid);
    if (ret < 0) return -1;
    if (pack->headers.connect.willretn) {
        n = mqtt_str_unpack(buf+ret, len-ret, pack->payload.connect.willtopic);
        if (n < 0) return -1;
        ret += n;
        n = mqtt_str_unpack(buf+ret, len-ret, pack->payload.connect.willmsg);
        if (n < 0) return -1;
        ret += n;
    }
    if (pack->headers.connect.username) {
        n = mqtt_str_unpack(buf+ret, len-ret, pack->payload.connect.username);
        if (n < 0) return -1;
        ret += n;
    }
    if (pack->headers.connect.password) {
        n = mqtt_str_unpack(buf+ret, len-ret, pack->payload.connect.password);
        if (n < 0) return -1;
        ret += n;
    }
    return ret;
}


static int connack_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    uint8_t b[2];
    b[0] = pack->headers.connack.sess_present;
    b[1] = pack->headers.connack.code;
    qtk_deque_push(dq, (const char*)b, 2);
    return 2;
}


static int connack_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    pack->headers.connack.sess_present = buf[0] & 1;
    pack->headers.connack.code = buf[1];
    return 2;
}


static int publish_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_string_t *topic = pack->headers.publish.topic;
    pack->flag = (pack->headers.publish.dup << 3)   \
        | (pack->headers.publish.qos << 1)          \
        | pack->headers.publish.retain;
    mqtt_str_pack(topic->data, topic->len, dq);
    if (pack->headers.publish.qos > 0) {
        mqtt_word_pack(pack->headers.publish.id, dq);
    }
    return topic->len + 4;
}


static int publish_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    int ret;
    pack->headers.publish.dup = pack->flag >> 3;
    pack->headers.publish.qos = (pack->flag>>1) & 3;
    pack->headers.publish.retain = pack->flag & 1;
    if (NULL == pack->headers.publish.topic) {
        pack->headers.publish.topic = wtk_string_new(0);
    }
    ret = mqtt_str_unpack(buf, len, pack->headers.publish.topic);
    if (ret < 0) return -1;
    if (pack->headers.publish.qos > 0) {
        if (len - ret < 2) return -1;
        mqtt_word_unpack(buf+ret, 2, &pack->headers.publish.id);
        ret += 2;
    }
    return ret;
}


static int publish_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_strbuf_t *msg = pack->payload.msg;
    if (msg && msg->pos) {
        qtk_deque_push(dq, msg->data, msg->pos);
        return msg->pos;
    }
    return 0;
}


static int publish_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    wtk_strbuf_t *msg = pack->payload.msg;
    if (NULL == msg) {
        msg = wtk_strbuf_new(1024, 1);
        pack->payload.msg = msg;
    }
    wtk_strbuf_reset(msg);
    wtk_strbuf_push(msg, (char*)buf, len);
    return len;
}


static int puback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.puback.id, dq);
    return 2;
}


static int puback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.puback.id);
    return 2;
}


static int pubrec_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.pubrec.id, dq);
    return 2;
}


static int pubrec_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.pubrec.id);
    return 2;
}


static int pubrel_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.pubrel.id, dq);
    return 2;
}


static int pubrel_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.pubrel.id);
    return 2;
}


static int pubcomp_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.pubcomp.id, dq);
    return 2;
}


static int pubcomp_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.pubcomp.id);
    return 2;
}


static int subscribe_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    pack->flag = 2;
    mqtt_word_pack(pack->headers.subscribe.id, dq);
    return 2;
}


static int subscribe_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.subscribe.id);
    return 2;
}


static int subscribe_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_queue_node_t *node;
    wtk_queue_t *tf_q = &pack->payload.subscribe.topics;
    int ret = 0;
    for (node = tf_q->pop; node; node = node->next) {
        MQTT_TopicFilter_t *tf = data_offset(node, MQTT_TopicFilter_t, node);
        uint8_t qos = tf->qos;
        mqtt_str_pack(tf->name.data, tf->name.len, dq);
        qtk_deque_push(dq, (const char*)&qos, 1);
        ret += tf->name.len + 2 + 1;
    }
    return ret;
}


static int subscribe_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    int ret;
    while (len > 0) {
        MQTT_TopicFilter_t *tf = mqtt_topicfilter_new();
        ret = mqtt_str_unpack(buf, len, &tf->name);
        if (ret < 0 || len-ret>0) {
            mqtt_topicfilter_delete(tf);
            return -1;
        }
        tf->qos = buf[ret];
        buf += ret + 1;
        len -= ret + 1;
        wtk_queue_push(&pack->payload.subscribe.topics, &tf->node);
    }
    return len;
}


static int suback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.suback.id, dq);
    return 2;
}


static int suback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.suback.id);
    return 2;
}


static int suback_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_string_t *codes = pack->payload.suback.codes;
    qtk_deque_push(dq, codes->data, codes->len);
    return codes->len;
}


static int suback_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    wtk_string_t *codes = wtk_string_new(len);
    memcpy(codes->data, buf, len);
    pack->payload.suback.codes = codes;
    return len;
}


static int unsubscribe_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    pack->flag = 2;
    mqtt_word_pack(pack->headers.unsubscribe.id, dq);
    return 2;
}


static int unsubscribe_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.unsubscribe.id);
    return 2;
}


static int unsubscribe_pld_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    wtk_queue_node_t *node;
    wtk_queue_t *tf_q = &pack->payload.unsubscribe.topics;
    int ret = 0;
    for (node = tf_q->pop; node; node = node->next) {
        MQTT_TopicFilter_t *tf = data_offset(node, MQTT_TopicFilter_t, node);
        mqtt_str_pack(tf->name.data, tf->name.len, dq);
        ret += tf->name.len + 2 ;
    }
    return ret;
}


static int unsubscribe_pld_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    int ret;
    while (len > 0) {
        MQTT_TopicFilter_t *tf = mqtt_topicfilter_new();
        ret = mqtt_str_unpack(buf, len, &tf->name);
        if (ret < 0) {
            mqtt_topicfilter_delete(tf);
            return -1;
        }
        buf += ret;
        len -= ret;
        wtk_queue_push(&pack->payload.unsubscribe.topics, &tf->node);
    }
    return len;
}


static int unsuback_hdr_pack(MQTT_Package_t *pack, qtk_deque_t *dq) {
    mqtt_word_pack(pack->headers.unsuback.id, dq);
    return 2;
}


static int unsuback_hdr_unpack(const uint8_t *buf, int len, MQTT_Package_t *pack) {
    if (len < 2) return -1;
    mqtt_word_unpack(buf, 2, &pack->headers.unsuback.id);
    return 2;
}
