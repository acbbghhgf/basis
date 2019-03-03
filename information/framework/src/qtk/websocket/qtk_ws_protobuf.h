#ifndef QTK_WEBSOCKET_QTK_WS_PROTOBUF_H
#define QTK_WEBSOCKET_QTK_WS_PROTOBUF_H
#include "third/libwebsockets/libwebsockets.h"
#include "qtk/protobuf/qtk_wire_format.h"
#include "qtk/core/qtk_array.h"
#include "wtk/core/wtk_str.h"
#include "qtk/core/qtk_deque.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct qtk_protobuf qtk_protobuf_t;
typedef struct qpb_variant qpb_variant_t;
typedef struct qpb_field qpb_field_t;

struct qtk_entry;
struct qtk_deque;


struct qpb_variant {
    union {
        uint32_t u32;
        int32_t i32;
        uint64_t u64;
        int64_t i64;
        double df;
        float f;
        int b;
        int e;
        wtk_string_t *s;
        qtk_array_t *a;
    } v;
    qtk_ctype_t type;
};


typedef enum {
    QPB_LABEL_OPTIONAL = 1,
    QPB_LABEL_REQUIRED = 2,
    QPB_LABEL_REPEATED = 3,
} qpb_label_t;


typedef enum {
    QPB_DESCTYPE_DOUBLE = 1,
    QPB_DESCTYPE_FLOAT = 2,
    QPB_DESCTYPE_INT64 = 3,
    QPB_DESCTYPE_UINT64 = 4,
    QPB_DESCTYPE_INT32 = 5,
    QPB_DESCTYPE_FIXED64 = 6,
    QPB_DESCTYPE_FIXED32 = 7,
    QPB_DESCTYPE_BOOL = 8,
    QPB_DESCTYPE_STRING = 9,
    QPB_DESCTYPE_GROUP = 10, /* not support */
    QPB_DESCTYPE_MESSAGE = 11,
    QPB_DESCTYPE_BYTES = 12,
    QPB_DESCTYPE_UINT32 = 13,
    QPB_DESCTYPE_ENUM = 14,
    QPB_DESCTYPE_SFIXED32 = 15,
    QPB_DESCTYPE_SFIXED64 = 16,
    QPB_DESCTYPE_SINT32 = 17,
    QPB_DESCTYPE_SINT64 = 18,
} qpb_desctype_t;


struct qpb_field {
    uint8_t label;
    uint8_t desctype;
};


qpb_variant_t* qpb_variant_new();
void qpb_variant_init(qpb_variant_t*);
qpb_variant_t* qpb_var_new_array(int len);
qpb_variant_t* qpb_var_new_array2(qtk_array_t *a);
qpb_variant_t* qpb_var_new_msg(int len);
qpb_variant_t* qpb_var_new_msg2(qtk_array_t *a);
qpb_variant_t* qpb_var_new_uint32(uint32_t u32);
qpb_variant_t* qpb_var_new_int32(int32_t i32);
qpb_variant_t* qpb_var_new_uint64(uint64_t u64);
qpb_variant_t* qpb_var_new_int64(int64_t i64);
qpb_variant_t* qpb_var_new_double(double df);
qpb_variant_t* qpb_var_new_float(float f);
qpb_variant_t* qpb_var_new_bool(int b);
qpb_variant_t* qpb_var_new_enum(int e);
qpb_variant_t* qpb_var_new_str(const char *p, int len);
qpb_variant_t* qpb_var_new_str2(wtk_string_t *str);
int qpb_variant_clean(qpb_variant_t* v);
int qpb_variant_delete(qpb_variant_t* v);
qpb_variant_t* qpb_var_array_get(qpb_variant_t *v, int idx);
int qpb_var_to_bool(qpb_variant_t *v);
double qpb_var_to_double(qpb_variant_t *v);
float qpb_var_to_float(qpb_variant_t *v);
uint32_t qpb_var_to_fixed32(qpb_variant_t *v);
uint64_t qpb_var_to_fixed64(qpb_variant_t *v);
int32_t qpb_var_to_sfixed32(qpb_variant_t *v);
int64_t qpb_var_to_sfixed64(qpb_variant_t *v);
int32_t qpb_var_to_int32(qpb_variant_t *v);
int64_t qpb_var_to_int64(qpb_variant_t *v);
uint64_t qpb_var_to_uint64(qpb_variant_t *v);
uint32_t qpb_var_to_uint32(qpb_variant_t *v);
int32_t qpb_var_to_sint32(qpb_variant_t *v);
int64_t qpb_var_to_sint64(qpb_variant_t *v);
#define qpb_var_to_enum qpb_var_to_int32
void qpb_var_set_string(qpb_variant_t *v, wtk_string_t *s);
void qpb_var_set_string2(qpb_variant_t *v, const char *p, int len);
void qpb_var_set_bool(qpb_variant_t *v, int b);
void qpb_var_set_double(qpb_variant_t *v, double df);
void qpb_var_set_float(qpb_variant_t *v, float f);
void qpb_var_set_fixed32(qpb_variant_t *v, uint32_t u);
void qpb_var_set_fixed64(qpb_variant_t *v, uint64_t u);
void qpb_var_set_sfixed32(qpb_variant_t *v, int32_t i);
void qpb_var_set_sfixed64(qpb_variant_t *v, int64_t i);
void qpb_var_set_int32(qpb_variant_t *v, int32_t i);
void qpb_var_set_int64(qpb_variant_t *v, int64_t i);
void qpb_var_set_uint64(qpb_variant_t *v, uint64_t u);
void qpb_var_set_uint32(qpb_variant_t *v, uint32_t u);
void qpb_var_set_sint32(qpb_variant_t *v, int32_t i);
void qpb_var_set_sint64(qpb_variant_t *v, int64_t i);
#define qpb_var_set_enum qpb_var_set_int32
void qpb_var_set_array(qpb_variant_t *v, int len);

struct qtk_protobuf {
    qpb_variant_t *v;
    struct lws *wsi;
    char buf[1024];
    char *buf_s;
    char *buf_e;
    wtk_heap_t *heap;
};


struct per_session_data__protobuf {
    struct qtk_deque *send_dq;
    struct qtk_deque *recv_dq;
    struct qtk_deque *tmp_dq;
    wtk_string_t *jwt_payload;
    uint32_t send_left;
    int sock_id;
};


qtk_protobuf_t *qtk_protobuf_new();
int qtk_protobuf_init2(qtk_protobuf_t *pb, wtk_heap_t *heap);
int qtk_protobuf_init(qtk_protobuf_t *pb);
int qtk_protobuf_clean(qtk_protobuf_t *pb);
int qtk_protobuf_delete(qtk_protobuf_t *pb);
int ws_callback_protobuf(struct lws *wsi, enum lws_callback_reasons reason, void *user,
                         void *in, size_t len);

qpb_variant_t* qtk_pb_decode(qtk_protobuf_t *pb, qtk_deque_t *dq);
int qtk_pb_decode_variant(qtk_protobuf_t *pb, qpb_variant_t *v, qpb_field_t *field);
int qtk_pb_decode_array(qtk_protobuf_t *pb, qpb_variant_t *v, qpb_field_t *field);
int qtk_pb_decode_message(qtk_protobuf_t *pb, qpb_variant_t *v);
int qtk_pb_encode_variant(qtk_protobuf_t *pb, qpb_variant_t *v, qtk_deque_t *dq);

#ifdef __cplusplus
}
#endif

#endif
