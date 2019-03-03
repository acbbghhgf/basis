#ifndef QTK_SFRAME_QTK_SFRAME_H
#define QTK_SFRAME_QTK_SFRAME_H
#include <arpa/inet.h>
#include <netdb.h>
#include "qtk/core/qtk_network.h"
#include "wtk/core/wtk_str.h"
#include "wtk/core/wtk_strbuf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define QTK_SFRAME_ADDRSTRLEN   32

enum qtk_sframe_type_t {
    QTK_SFRAME_MSG_UNKNOWN = 0,
    QTK_SFRAME_MSG_REQUEST,
    QTK_SFRAME_MSG_RESPONSE,
    QTK_SFRAME_MSG_CMD,
    QTK_SFRAME_MSG_NOTICE,
    QTK_SFRAME_MSG_MQTT,
    QTK_SFRAME_MSG_PROTOBUF,
    QTK_SFRAME_MSG_JWT_PAYLOAD,
};


enum qtk_sframe_cmd_t {
    QTK_SFRAME_CMD_UNKNOWN = 0,
    QTK_SFRAME_CMD_OPEN,
    QTK_SFRAME_CMD_CLOSE,
    QTK_SFRAME_CMD_LISTEN,
    QTK_SFRAME_CMD_OPEN_POOL,
    QTK_SFRAME_CMD_REOPEN_POOL,
};

#define QTK_SFRAME_CLIENT_ATQA      1
#define QTK_SFRAME_CLIENT_STREAM    2

#define QTK_SFRAME_OK               0
#define QTK_SFRAME_SIGCONNECT       1
#define QTK_SFRAME_SIGDISCONNECT    2
#define QTK_SFRAME_SIGRECONNECT     3
#define QTK_SFRAME_SIGECONNECT      4
#define QTK_SFRAME_SIGESENDBUF      5
#define QTK_SFRAME_SIG_MASK         0x0fff

#define QTK_SFRAME_SIG_DNOT_DEL_REQ 0x8000


struct qtk_log;
struct qtk_fwd;
struct qtk_mqtt_buffer;
struct qtk_request;
struct qtk_response;
struct MQTT_Package;
struct qtk_protobuf;
struct lws;
struct qpb_variant;


typedef struct qtk_sframe_msg qtk_sframe_msg_t;
typedef struct qtk_sframe_method qtk_sframe_method_t;

typedef struct qtk_log *(*qtk_sframe_get_log_f)(qtk_sframe_method_t *);
typedef void *(*qtk_sframe_get_cfg_f)(qtk_sframe_method_t *);
typedef int (*qtk_sframe_socket_f)(qtk_sframe_method_t *);
typedef wtk_string_t *(*qtk_sframe_address_f)(qtk_sframe_method_t *, qtk_sframe_msg_t *);
typedef qtk_sframe_msg_t *(*qtk_sframe_msg_pop_f)(qtk_sframe_method_t *, int);
typedef int (*qtk_sframe_msg_push_f)(qtk_sframe_method_t *, qtk_sframe_msg_t *);

typedef int (*qtk_sframe_msg_delete_f)(qtk_sframe_method_t *, qtk_sframe_msg_t *);
typedef qtk_sframe_msg_t *(*qtk_sframe_msg_new_f)(qtk_sframe_method_t *, int, int);

typedef int (*qtk_sframe_msg_get_id_f)(qtk_sframe_msg_t *);
typedef int (*qtk_sframe_msg_get_type_f)(qtk_sframe_msg_t *);
typedef int (*qtk_sframe_msg_get_signal_f)(qtk_sframe_msg_t *);
typedef int (*qtk_sframe_msg_get_method_f)(qtk_sframe_msg_t *);
typedef int (*qtk_sframe_msg_get_status_code_f)(qtk_sframe_msg_t *);
typedef wtk_string_t *(*qtk_sframe_msg_get_url_f)(qtk_sframe_msg_t *);
typedef int (*qtk_sframe_msg_get_mqtt_f)(qtk_sframe_msg_t *, struct MQTT_Package*);
typedef void (*qtk_sframe_msg_set_method_f)(qtk_sframe_msg_t *, int);
typedef void (*qtk_sframe_msg_set_status_f)(qtk_sframe_msg_t *, int);
typedef void (*qtk_sframe_msg_set_url_f)(qtk_sframe_msg_t *, const char *, int);
typedef void (*qtk_sframe_msg_set_cmd_f)(qtk_sframe_msg_t *, int, void *);
typedef int (*qtk_sframe_msg_set_mqtt_f)(qtk_sframe_msg_t *, struct MQTT_Package*);
typedef void (*qtk_sframe_msg_add_notice_signal_f)(qtk_sframe_msg_t *, int);
typedef void (*qtk_sframe_msg_proto_set_body_f)(qtk_sframe_msg_t *, wtk_strbuf_t *);
typedef void (*qtk_sframe_msg_proto_add_body_f)(qtk_sframe_msg_t *, const char *, int);
typedef void (*qtk_sframe_msg_proto_add_field_f)(qtk_sframe_msg_t *, const char *, const char *);

typedef wtk_strbuf_t    *(*qtk_sframe_msg_proto_body_find_f)(qtk_sframe_msg_t *);
typedef wtk_string_t    *(*qtk_sframe_msg_proto_field_value_find_f)(qtk_sframe_msg_t *, const char *);

typedef struct qpb_variant   *(*qtk_sframe_msg_get_pbvar_f)(qtk_sframe_msg_t*);
typedef struct lws      *(*qtk_sframe_msg_get_ws_f)(qtk_sframe_msg_t*);
typedef int (*qtk_sframe_msg_set_protobuf_f)(qtk_sframe_msg_t*, struct qpb_variant*, struct lws*);

struct qtk_sframe_method{
    qtk_sframe_get_log_f    get_log;
    qtk_sframe_get_cfg_f    get_cfg;
    qtk_sframe_socket_f     socket;
    qtk_sframe_socket_f     request_pool;
    qtk_sframe_address_f    get_local_addr;
    qtk_sframe_address_f    get_remote_addr;
    qtk_sframe_msg_pop_f    pop;
    qtk_sframe_msg_push_f   push;
    qtk_sframe_msg_new_f    new;
    qtk_sframe_msg_delete_f delete;

    qtk_sframe_msg_get_id_f     get_id;
    qtk_sframe_msg_get_url_f    get_url;
    qtk_sframe_msg_get_url_f    req_url;
    qtk_sframe_msg_get_type_f   get_type;
    qtk_sframe_msg_get_signal_f get_signal;
    qtk_sframe_msg_get_method_f get_method;
    qtk_sframe_msg_get_method_f req_method;
    qtk_sframe_msg_get_status_code_f get_status;
    qtk_sframe_msg_get_mqtt_f get_mqtt;

    qtk_sframe_msg_set_method_f     set_method;
    qtk_sframe_msg_set_status_f     set_status;
    qtk_sframe_msg_set_url_f         set_url;
    qtk_sframe_msg_set_cmd_f      set_cmd;
    qtk_sframe_msg_add_notice_signal_f      set_signal;
    qtk_sframe_msg_proto_set_body_f         set_body;
    qtk_sframe_msg_proto_add_body_f         add_body;
    qtk_sframe_msg_proto_add_field_f        add_header;
    qtk_sframe_msg_proto_body_find_f        get_body;
    qtk_sframe_msg_proto_field_value_find_f get_header;
    qtk_sframe_msg_proto_body_find_f        req_body;
    qtk_sframe_msg_proto_field_value_find_f req_header;
    qtk_sframe_msg_set_mqtt_f set_mqtt;

    qtk_sframe_msg_get_pbvar_f get_pb_var;
    qtk_sframe_msg_get_ws_f get_ws;
    qtk_sframe_msg_set_protobuf_f set_protobuf;
};

typedef int (*qtk_sframe_module_f)(void *);
typedef void *(*qtk_sframe_module_new_f)(qtk_sframe_method_t *);

typedef struct qtk_sframe_module qtk_sframe_module_t;
struct qtk_sframe_module{
    const char *name;
    void *handle;               /* dlopen */
    void *module;               /* module_new */
    qtk_sframe_module_f stop;
    qtk_sframe_module_f start;
    qtk_sframe_module_f delete;
    qtk_sframe_module_new_f new;
};


typedef struct qtk_sframe_listen_param qtk_sframe_listen_param_t;
struct qtk_sframe_listen_param {
    const char *proto;
    struct sockaddr_in saddr;
    int backlog;
};


#define LISTEN_PARAM_NEW2(addr, port, bl, _proto) ({                     \
            qtk_sframe_listen_param_t *_p = wtk_malloc(sizeof(*_p));    \
            _p->saddr.sin_family = AF_INET;                             \
            inet_aton(addr, &(_p->saddr.sin_addr));                     \
            _p->saddr.sin_port = htons(port);                           \
            _p->backlog = bl;                                           \
            _p->proto = _proto;                                         \
            _p;})

#define LISTEN_PARAM_NEW(addr, port, bl)         \
    LISTEN_PARAM_NEW2(addr, port, bl, "http")

#define HTTP_LISTEN_PARAM_NEW(addr, port, bl)   \
    LISTEN_PARAM_NEW2(addr, port, bl, "http")

#define MQTT_LISTEN_PARAM_NEW(addr, port, bl)   \
    LISTEN_PARAM_NEW2(addr, port, bl, "mqtt")


typedef enum {
    QTK_SFRAME_NOT_RECONNECT = 0,
    QTK_SFRAME_RECONNECT_NOW = -1,
    QTK_SFRAME_RECONNECT_LAZY = -2,
} qtk_sframe_reconnect_t;

typedef struct qtk_sframe_connect_param qtk_sframe_connect_param_t;
struct qtk_sframe_connect_param {
    const char *proto;
    struct sockaddr_in saddr;
    qtk_sframe_reconnect_t reconnect;
};


#define CONNECT_PARAM_NEW2(addr, port, rec, _proto) ({                  \
            qtk_sframe_connect_param_t *_p = wtk_malloc(sizeof(*_p));   \
            _p->reconnect = rec;                                        \
            _p->saddr.sin_family = AF_INET;                             \
            _p->saddr.sin_port = htons(port);                           \
            _p->proto = _proto;                                         \
            if (!inet_aton(addr, &_p->saddr.sin_addr)) {                \
                if (qtk_get_ip_by_name(addr, &_p->saddr.sin_addr)) {    \
                    wtk_free(_p);                                       \
                    _p = NULL;                                          \
                }                                                       \
            }                                                           \
            _p;})

#define CONNECT_PARAM_NEW(addr, port, rec)                      \
                 CONNECT_PARAM_NEW2(addr, port, rec, "http")

#define HTTP_CONNECT_PARAM_NEW(addr, port, rec)                 \
                 CONNECT_PARAM_NEW2(addr, port, rec, "http")

#define MQTT_CONNECT_PARAM_NEW(addr, port, rec) \
                 CONNECT_PARAM_NEW2(addr, port, rec, "mqtt")


typedef struct qtk_sframe_close_param qtk_sframe_close_param_t;
struct qtk_sframe_close_param {
    wtk_string_t *local_addr;
    wtk_string_t *remote_addr;
};


struct qtk_sframe_msg{
    wtk_queue_node_t    q_n;
    uint16_t type;
    uint16_t signal;
    int id;
    union {
        struct qtk_request *request;
        struct qtk_response *response;
        struct qtk_mqtt_buffer *mqttbuf;
        struct qtk_protobuf *protobuf;
        qtk_sframe_listen_param_t *lis_param;
        qtk_sframe_connect_param_t *con_param;
        qtk_sframe_close_param_t *cls_param;
        wtk_string_t *jwt_payload;
    } info;
};


#ifdef __cplusplus
};
#endif

#endif