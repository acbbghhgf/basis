#ifndef _QTK_TEST_MQTT_H_
#define _QTK_TEST_MQTT_H_
#include <stdint.h>
#include <wtk/core/wtk_queue.h>
#include <wtk/core/wtk_strbuf.h>

#ifdef __cplusplus
#extern "C" {
#endif


struct qtk_deque;


enum MQTT_Type{
    MQTT_Reserved = 0,
    MQTT_CONNECT,
    MQTT_CONNACK,
    MQTT_PUBLISH,
    MQTT_PUBACK,
    MQTT_PUBREC,
    MQTT_PUBREL,
    MQTT_PUBCOMP,
    MQTT_SUBSCRIBE,
    MQTT_SUBACK,
    MQTT_UNSUBSCRIBE,
    MQTT_UNSUBACK,
    MQTT_PINGREQ,
    MQTT_PINGRESP,
    MQTT_DISCONNECT,
};

enum MQTT_ConnCode {
    MQTT_Conn_Accept = 0,
    MQTT_Invalid_Proto,
    MQTT_Invalid_ID,
    MQTT_No_Service,
    MQTT_Invalid_Password,
    MQTT_Forbidden,
};


struct MQTT_TopicFilter {
    wtk_queue_node_t node;
    wtk_string_t name;
    uint8_t qos:2;
};


struct MQTT_Package {
    uint8_t type:4;
    uint8_t flag:4;
    uint32_t length;
    union {
        struct {
            char proto_name[7]; /* B0-B5 */
            uint8_t proto_level; /* B6 */
            uint8_t username:1;  /* B7.7 */
            uint8_t password:1;  /* B7.6 */
            uint8_t willretn:1;  /* B7.5 */
            uint8_t willqos:2;   /* B7.4-3 */
            uint8_t willflag:1;  /* B7.2 */
            uint8_t cleanses:1;  /* B7.1 */
            uint8_t reserved:1;
            uint16_t keepalive; /* B8-9 */
        } connect;
        struct {
            uint8_t sess_present:1; /* B0.0 */
            enum MQTT_ConnCode code;
        } connack;
        struct {
            uint16_t id;        /* qos=1,2 */
            uint8_t dup:1;
            uint8_t qos:2;
            uint8_t retain:1;
            wtk_string_t *topic;
        } publish;
        struct {
            uint16_t id;
        } puback;
        struct {
            uint16_t id;
        } pubrec;
        struct {
            uint16_t id;
        } pubrel;
        struct {
            uint16_t id;
        } pubcomp;
        struct {
            uint16_t id;
        } subscribe;
        struct {
            uint16_t id;
        } suback;
        struct {
            uint16_t id;
        } unsubscribe;
        struct {
            uint16_t id;
        } unsuback;
    } headers;
    union {
        struct {
            wtk_string_t *clientid;
            wtk_string_t *willtopic;
            wtk_string_t *willmsg;
            wtk_string_t *username;
            wtk_string_t *password;
        } connect;
        struct {
            wtk_queue_t topics;
        } subscribe;
        struct {
            wtk_string_t *codes;
        } suback;
        struct {
            wtk_queue_t topics;
        } unsubscribe;
        wtk_strbuf_t *msg;
    } payload;
};


typedef struct MQTT_TopicFilter MQTT_TopicFilter_t;
typedef struct MQTT_Package MQTT_Package_t;


int mqtt_atoi(const uint8_t *buf, int len, uint32_t *n);
int mqtt_itoa(uint32_t n, uint8_t *buf);
int mqtt_unpack_var(const uint8_t *buf, int n, uint8_t type, MQTT_Package_t *pack);
int mqtt_pack_var(MQTT_Package_t *pack, uint8_t type, struct qtk_deque *dbuf);
int mqtt_pack(MQTT_Package_t *pack, struct qtk_deque *dbuf);
int mqtt_unpack(const uint8_t *buf, int n, MQTT_Package_t *pack);

MQTT_TopicFilter_t* mqtt_topicfilter_new();
int mqtt_topicfilter_delete(MQTT_TopicFilter_t* tf);

MQTT_Package_t *mqtt_package_new();
int mqtt_package_delete(MQTT_Package_t *pack);


#ifdef __cplusplus
}
#endif

#endif
