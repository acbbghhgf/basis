#include "qtk_ws_protobuf.h"
#include "qtk/entry/qtk_ws.h"
#include "qtk/entry/qtk_entry.h"
#include "private-libwebsockets.h"
#include "qtk/core/qtk_deque.h"
#include "qtk/core/qtk_array.h"
#include "qtk/frame/qtk_frame.h"


#define QTK_PREDICT_TRUE(x) (__builtin_expect(!!(x), 1))

#define QTK_UTF8LEN(x) (kUTF8LenTbl[(uint8_t)(x)])

#define ZIGZAG32(x) (((x)<<1) ^ ((x)>>31))
#define ZIGZAG64(x) (((x)<<1) ^ ((x)>>63))
#define UNZIGZAG(x) (((x)&1) ? -((x)+1)/2 : (x)/2)

#define QTK_PB_IN_BUF(pb, x) (((x) >= (pb)->buf) && ((x) < (pb)->buf+sizeof((pb)->buf)))


// Table of UTF-8 character lengths, based on first byte
static const unsigned char kUTF8LenTbl[256] = {
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,

    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2, 2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3, 3,3,3,3,3,3,3,3, 4,4,4,4,4,4,4,4, 4,4,4,4,4,4,4,4
};


qpb_variant_t* qtk_pb_decode_frame(qtk_protobuf_t *pb, qtk_deque_t *dq, int group_number);


void qpb_variant_init(qpb_variant_t *v) {
    memset(&v->v, 0, sizeof(v->v));
    v->type = 0;
}


qpb_variant_t* qpb_variant_new() {
    qpb_variant_t* v = wtk_malloc(sizeof(*v));
    qpb_variant_init(v);
    return v;
}


qpb_variant_t* qpb_var_new_array(int len) {
    qtk_array_t *a = qtk_array_new2(sizeof(qpb_variant_t), len);
    return qpb_var_new_array2(a);
}


qpb_variant_t* qpb_var_new_array2(qtk_array_t *a) {
    qpb_variant_t *var = qpb_variant_new();
    var->v.a = a;
    var->type = QPB_CTYPE_ARRAY;
    return var;
}


qpb_variant_t* qpb_var_new_msg(int len) {
    qtk_array_t *a = qtk_array_new2(sizeof(qpb_variant_t), len);
    return qpb_var_new_msg2(a);
}


qpb_variant_t* qpb_var_new_msg2(qtk_array_t *a) {
    qpb_variant_t *var = qpb_variant_new();
    var->v.a = a;
    var->type = QPB_CTYPE_MESSAGE;
    return var;
}


qpb_variant_t* qpb_var_new_uint32(uint32_t u32) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.u32 = u32;
    v->type = QPB_CTYPE_UINT32;
    return v;
}


qpb_variant_t* qpb_var_new_int32(int32_t i32) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.i32 = i32;
    v->type = QPB_CTYPE_INT32;
    return v;
}


qpb_variant_t* qpb_var_new_uint64(uint64_t u64) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.u64 = u64;
    v->type = QPB_CTYPE_UINT64;
    return v;
}


qpb_variant_t* qpb_var_new_int64(int64_t i64) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.i64 = i64;
    v->type = QPB_CTYPE_INT64;
    return v;
}


qpb_variant_t* qpb_var_new_double(double df) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.df = df;
    v->type = QPB_CTYPE_DOUBLE;
    return v;
}


qpb_variant_t* qpb_var_new_float(float f) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.f = f;
    v->type = QPB_CTYPE_FLOAT;
    return v;
}


qpb_variant_t* qpb_var_new_bool(int b) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.b = b;
    v->type = QPB_CTYPE_BOOL;
    return v;
}


qpb_variant_t* qpb_var_new_enum(int e) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.e = e;
    v->type = QPB_CTYPE_ENUM;
    return v;
}


qpb_variant_t* qpb_var_new_str(const char *p, int len) {
    wtk_string_t *str = wtk_string_new(len);
    memcpy(str->data, p, len);
    return qpb_var_new_str2(str);
}


qpb_variant_t* qpb_var_new_str2(wtk_string_t *str) {
    qpb_variant_t *v = qpb_variant_new();
    v->v.s = str;
    v->type = QPB_CTYPE_STRING;
    return v;
}


qpb_variant_t* qpb_var_array_get(qpb_variant_t *v, int idx) {
    assert(v->type == QPB_CTYPE_MESSAGE || v->type == QPB_CTYPE_ARRAY);
    return qtk_array_get(v->v.a, idx);
}


wtk_string_t* qpb_var_to_string(qpb_variant_t *v) {
    assert(v->type == QPB_CTYPE_STRING);
    return v->v.s;
}


#define qpb_var_need_clean(v) 1


void qpb_var_set_string(qpb_variant_t *v, wtk_string_t *s) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.s = s;
    v->type = QPB_CTYPE_STRING;
}


void qpb_var_set_string2(qpb_variant_t *v, const char *p, int len) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.s = wtk_string_new(len);
    memcpy(v->v.s->data, p, len);
    v->type = QPB_CTYPE_STRING;
}


int qpb_var_to_bool(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_BOOL:
        return v->v.b;
    case QPB_CTYPE_VARINT:
        return v->v.u64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_bool(qpb_variant_t *v, int b) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.b = b;
    v->type = QPB_CTYPE_BOOL;
}


double qpb_var_to_double(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_DOUBLE:
        return v->v.df;
    case QPB_CTYPE_FIXED64:
        return v->v.df;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_double(qpb_variant_t *v, double df) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.df = df;
    v->type = QPB_CTYPE_DOUBLE;
}


float qpb_var_to_float(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_FLOAT:
        return v->v.f;
    case QPB_CTYPE_FIXED32:
        return v->v.f;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_float(qpb_variant_t *v, float f) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.f = f;
    v->type = QPB_CTYPE_FLOAT;
}


uint32_t qpb_var_to_fixed32(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_FIXED32:
        return v->v.u32;
    case QPB_CTYPE_UINT32:
        return v->v.u32;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_fixed32(qpb_variant_t *v, uint32_t u) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u32 = u;
    v->type = QPB_CTYPE_FIXED32;
}


uint64_t qpb_var_to_fixed64(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_FIXED64:
        return v->v.u64;
    case QPB_CTYPE_UINT64:
        return v->v.u64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_fixed64(qpb_variant_t *v, uint64_t u) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u64 = u;
    v->type = QPB_CTYPE_FIXED64;
}


int32_t qpb_var_to_sfixed32(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_FIXED32:
        return v->v.i32;
    case QPB_CTYPE_INT32:
        return v->v.i32;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_sfixed32(qpb_variant_t *v, int32_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.i32 = i;
    v->type = QPB_CTYPE_FIXED32;
}


int64_t qpb_var_to_sfixed64(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_FIXED64:
        return v->v.i64;
    case QPB_CTYPE_INT64:
        return v->v.i64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_sfixed64(qpb_variant_t *v, int64_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.i64 = i;
    v->type = QPB_CTYPE_FIXED64;
}


int32_t qpb_var_to_int32(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return (v->v.u64 & 0xffffffff);
    case QPB_CTYPE_INT32:
        return v->v.i32;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_int32(qpb_variant_t *v, int32_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.i32 = i;
    v->type = QPB_CTYPE_INT32;
}


int64_t qpb_var_to_int64(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return v->v.i64;
    case QPB_CTYPE_INT64:
        return v->v.i64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_int64(qpb_variant_t *v, int64_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.i64 = i;
    v->type = QPB_CTYPE_INT64;
}


uint64_t qpb_var_to_uint64(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return v->v.u64;
    case QPB_CTYPE_UINT64:
        return v->v.u64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_uint64(qpb_variant_t *v, uint64_t u) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u64 = u;
    v->type = QPB_CTYPE_UINT64;
}


uint32_t qpb_var_to_uint32(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return v->v.u64 & 0xffffffff;
    case QPB_CTYPE_UINT32:
        return v->v.u32;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_uint32(qpb_variant_t *v, uint32_t u) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u32 = u;
    v->type = QPB_CTYPE_UINT32;
}


int32_t qpb_var_to_sint32(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return UNZIGZAG(v->v.u64);
    case QPB_CTYPE_INT32:
        return v->v.i32;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_sint32(qpb_variant_t *v, int32_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u32 = ZIGZAG32(i);
    v->type = QPB_CTYPE_UINT32;
}


int64_t qpb_var_to_sint64(qpb_variant_t *v) {
    switch (v->type) {
    case QPB_CTYPE_VARINT:
        return UNZIGZAG(v->v.u64);
    case QPB_CTYPE_INT64:
        return v->v.i64;
    default:
        assert(0);
    }
    return 0;
}


void qpb_var_set_sint64(qpb_variant_t *v, int64_t i) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.u64 = ZIGZAG64(i);
    v->type = QPB_CTYPE_UINT64;
}


void qpb_var_set_array(qpb_variant_t *v, int len) {
    if (qpb_var_need_clean(v)) {
        qpb_variant_clean(v);
    }
    v->v.a = qtk_array_new2(sizeof(qpb_variant_t), len);
    v->type = QPB_CTYPE_ARRAY;
}


int qpb_variant_clean(qpb_variant_t* v) {
    if (0 == v->type) return 0;
    switch (v->type) {
    case QPB_CTYPE_STRING:
    case QPB_CTYPE_DELIMITED:
        wtk_string_delete(v->v.s);
        v->v.s = NULL;
        break;
    case QPB_CTYPE_ARRAY:
    case QPB_CTYPE_PACK:
    case QPB_CTYPE_MESSAGE: {
        qtk_array_t *a = v->v.a;
        qpb_variant_t *s = a->data;
        qpb_variant_t *e = s + a->len;
        for (; s < e; s++) {
            if (v->type) {
                qpb_variant_clean(s);
            }
        }
        qtk_array_delete(a);
        v->v.a = NULL;
        break;
    }
    default:
        break;
    }
    v->type = 0;
    return 0;
}


int qpb_variant_delete(qpb_variant_t *v) {
    qpb_variant_clean(v);
    wtk_free(v);
    return 0;
}


typedef struct {
    qpb_variant_t *slots;
    int group_number;
    unsigned closed:1;
} qtk_decframe_t;


void qtk_frame_init2(qtk_decframe_t *frame, int group_number, qpb_variant_t *v) {
    frame->group_number = group_number;
    frame->closed = 0;
    frame->slots = v;
}


void qtk_frame_init(qtk_decframe_t *frame, int group_number) {
    qtk_frame_init2(frame, group_number, qpb_var_new_msg(8));
}


qpb_variant_t* qtk_frame_get_slots(qtk_decframe_t *frame) {
    if (frame->slots) {
        qpb_variant_t *arr = frame->slots;
        frame->slots = NULL;
        return arr;
    }
    return NULL;
}


void qtk_frame_clean(qtk_decframe_t *frame) {
    if (frame->slots) {
        qpb_variant_delete(frame->slots);
    }
}


void dump_handshake_info(struct lws *wsi);

void ws_session_data_clean(struct per_session_data__protobuf *session) {
    if (session->send_dq) {
        qtk_deque_delete(session->send_dq);
        session->send_dq = NULL;
    }
    if (session->recv_dq) {
        qtk_deque_delete(session->recv_dq);
        session->recv_dq = NULL;
    }
    if (session->tmp_dq) {
        qtk_deque_delete(session->tmp_dq);
        session->tmp_dq = NULL;
    }
    if (session->jwt_payload) {
        wtk_string_delete(session->jwt_payload);
        session->jwt_payload = NULL;
    }
}


int ws_callback_protobuf(struct lws *wsi, enum lws_callback_reasons reason, void *user,
                         void *in, size_t len) {
    unsigned char buf[LWS_PRE + 4096];
    struct per_session_data__protobuf *data = (struct per_session_data__protobuf*)user;
    unsigned char *p = &buf[LWS_PRE];
    int m, n;
    int ret = 0;

    switch (reason) {
    case LWS_CALLBACK_ESTABLISHED:
        data->sock_id = qtk_ws_established(wsi);
        qtk_ws_raise_jwt_payload(wsi, data);
        break;

    case LWS_CALLBACK_CLOSED: {
        qtk_socket_t *s = qtk_entry_get_socket(lws_context_user(wsi->context), data->sock_id);
        if (s && s->state != SOCKET_CLOSED) {
            qtk_ws_closed(wsi, s);
        }
        break;
    }

    case LWS_CALLBACK_WSI_DESTROY: {
        qtk_entry_t *entry = lws_context_user(wsi->context);
        qtk_socket_t *s = qtk_entry_get_socket(entry, data->sock_id);
        if (s && s->state != SOCKET_CLOSED) {
            qtk_ws_closed(wsi, s);
        }
        ws_session_data_clean(data);
        break;
    }

    case LWS_CALLBACK_SERVER_WRITEABLE: {
        if (NULL == data->send_dq || 0 == data->send_dq->len) break;
        assert(data->send_left || data->send_dq->len > sizeof(data->send_left));
        enum lws_write_protocol wp = 0;
        if (0 == data->send_left) {
            qtk_deque_pop(data->send_dq, (char*)&data->send_left, sizeof(data->send_left));
            wp |= LWS_WRITE_BINARY;
        } else {
            wp|= LWS_WRITE_CONTINUATION;
        }
        n = sizeof(buf) - LWS_PRE;
        if (data->send_left > n) {
            wp |= LWS_WRITE_NO_FIN;
        } else {
            n = data->send_left;
        }
        assert(data->send_dq->len >= n);
        qtk_deque_front(data->send_dq, (char*)p, n);
        m = lws_write(wsi, p, n, wp);
        if (m < 0) {
            wtk_debug("ERROR %d writing to di socket\n", n);
            return -1;
        } else {
            qtk_deque_pop(data->send_dq, NULL, m);
            data->send_left -= m;
        }
        if (data->send_dq->len > 0) {
            lws_callback_on_writable(wsi);
        }
        break;
    }
    case LWS_CALLBACK_RECEIVE:
        if (NULL == data->recv_dq) {
            data->recv_dq = qtk_deque_new(4096, 40960000, 1);
        }
        qtk_deque_push(data->recv_dq, in, len);
        if (lws_is_final_fragment(wsi)) {
            qtk_entry_t *entry = lws_context_user(wsi->context);
            qtk_protobuf_t *pb = qtk_protobuf_new();
            qtk_pb_decode(pb, data->recv_dq);
            if (pb->v) {
                pb->wsi = wsi;
                qtk_sframe_msg_t *msg = qtk_fwd_protobuf_msg(entry->frame, pb, data->sock_id);
                qtk_entry_raise_msg(entry, msg);
            } else {
                qtk_protobuf_delete(pb);
            }
        }
        break;

    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        /* do jwt auth and store jwt payload
         * return non-zero to reject client conn */
        ret = qtk_ws_handle_jwt(wsi, data);
        break;

    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        /* wtk_debug("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: len %lu\n", */
        /*           (unsigned long)len); */
        break;

    default:
        break;
    }
    return ret;
}


qtk_protobuf_t *qtk_protobuf_new() {
    qtk_protobuf_t *pb = (qtk_protobuf_t*)malloc(sizeof(*pb));
    qtk_protobuf_init(pb);
    return pb;
}


int qtk_protobuf_init2(qtk_protobuf_t *pb, wtk_heap_t *heap) {
    pb->v = NULL;
    pb->wsi = NULL;
    pb->buf_s = pb->buf;
    pb->buf_e = pb->buf;
    pb->heap = heap;
    return 0;
}


int qtk_protobuf_init(qtk_protobuf_t *pb) {
    return qtk_protobuf_init2(pb, wtk_heap_new(4096));
}


int qtk_protobuf_clean(qtk_protobuf_t *pb) {
    if (pb->v) {
        qpb_variant_delete(pb->v);
        pb->v = NULL;
    }
    if (pb->heap) {
        wtk_heap_delete(pb->heap);
        pb->heap = NULL;
    }
    return 0;
}


int qtk_protobuf_delete(qtk_protobuf_t *pb) {
    qtk_protobuf_clean(pb);
    wtk_free(pb);
    return 0;
}


int qtk_pb_read_varint64(qtk_protobuf_t *pb, uint64_t *val) {
    (void)pb;
    int bitpos = 0;
    uint8_t byte;
    *val = 0;

    do {
        byte = *pb->buf_s++;
        *val |= (uint64_t)(byte&0x7f) << bitpos;
        bitpos += 7;
    } while (byte & 0x80 && pb->buf_e > pb->buf_s);

    return 0;
}


int qtk_pb_write_varint64(qtk_protobuf_t *pb, qtk_deque_t *dq, uint64_t val) {
    (void)pb;
    uint8_t byte = 0;
    while (1) {
        byte = val & 0x7f;
        val >>= 7;
        if (val) {
            byte |= 0x80;
            qtk_deque_push(dq, (char*)&byte, 1);
        } else {
            qtk_deque_push(dq, (char*)&byte, 1);
            break;
        }
    }
    return 0;
}


int qtk_pb_read_varint32(qtk_protobuf_t *pb, uint32_t *val) {
    uint64_t v64;
    if (qtk_pb_read_varint64(pb, &v64) < 0) return -1;
    assert(v64 >> 32);
    *val = v64;
    return 0;
}


void qtk_pb_fill_buffer(qtk_protobuf_t *pb, qtk_deque_t *dq) {
    if (pb->buf_s == pb->buf_e) {
        pb->buf_s = pb->buf;
        pb->buf_e = pb->buf_s + qtk_deque_pop2(dq, pb->buf, sizeof(pb->buf));
    } else if (pb->buf_e - pb->buf_s < 10 && dq) {
        assert(QTK_PB_IN_BUF(pb, pb->buf_s));
        assert(QTK_PB_IN_BUF(pb, pb->buf_e));
        int n = pb->buf_e - pb->buf_s;
        memmove(pb->buf, pb->buf_s, n);
        pb->buf_s = pb->buf;
        pb->buf_e = pb->buf + n;
        pb->buf_e += qtk_deque_pop2(dq, pb->buf_e, sizeof(pb->buf)-n);
    }
}


int qtk_pb_read_tag(qtk_protobuf_t *pb, qtk_deque_t *dq, uint32_t *tag) {
    uint32_t v = 0;
    uint8_t data = 0;
    if (dq) {
        qtk_pb_fill_buffer(pb, dq);
    }
    if (QTK_PREDICT_TRUE(pb->buf_e > pb->buf_s)) {
        v = *pb->buf_s;
        if (v < 0x80) {
            pb->buf_s++;
            *tag = v;
            return 0;
        }
    }
    if (QTK_PREDICT_TRUE(pb->buf_e > pb->buf_s)) {
        if (data == 0) {
            *tag = 0;
            return 0;
        }
        if (qtk_pb_read_varint32(pb, tag) < 0) {
            *tag = 0;
            return -1;
        }
        return 0;
    } else {
        return -1;
    }
}

int qtk_pb_write_tag(qtk_protobuf_t *pb, qtk_deque_t *dq, uint32_t tag) {
    return qtk_pb_write_varint64(pb, dq, tag);
}

wtk_string_t* qtk_pb_read_string(qtk_protobuf_t *pb, qtk_deque_t *dq) {
    uint64_t len;
    qtk_pb_fill_buffer(pb, dq);
    if (qtk_pb_read_varint64(pb, &len) < 0) /* here only read var32 actually */ {
        return NULL;
    }
    wtk_string_t *str = wtk_string_new(len);
    char *p = str->data;
    int n = pb->buf_e - pb->buf_s;
    if (n) {
        if (n >= len) {
            memcpy(p, pb->buf_s, len);
            pb->buf_s += len;
            return str;
        } else {
            memcpy(p, pb->buf_s, n);
            pb->buf_s = pb->buf_e;
        }
        p += n;
        len -= n;
    }
    if (dq->len < len) {
        wtk_string_delete(str);
        return NULL;
    }
    qtk_deque_pop2(dq, p, len);
    return str;
}


wtk_string_t* qtk_pb_read_string_ref(qtk_protobuf_t *pb, qtk_deque_t *dq) {
    uint64_t len;
    if (qtk_pb_read_varint64(pb, &len) < 0) {
        return NULL;
    }
    wtk_string_t *str = wtk_string_new(0);
    if (dq || QTK_PB_IN_BUF(pb, pb->buf_s)) {
        char *p = wtk_heap_malloc(pb->heap, len);
        int n = pb->buf_e - pb->buf_s;
        wtk_string_set(str, p, len);
        if (n) {
            if (n >= len) {
                memcpy(p, pb->buf_s, len);
                pb->buf_s += len;
                return str;
            }  else {
                memcpy(p, pb->buf_s, n);
                pb->buf_s = pb->buf_e;
            }
            p += n;
            len -= n;
        }
        if (dq->len < len) {
            wtk_string_delete(str);
            return NULL;
        }
        qtk_deque_pop2(dq, p, len);
    } else {
        assert(pb->buf_e - pb->buf_s >= len);
        str->data = pb->buf_s;
        str->len = len;
        pb->buf_s += len;
    }
    return str;
}


int qtk_pb_decode_string(qtk_protobuf_t *pb, qtk_deque_t *dq, qpb_variant_t *v) {
    assert(0 == v->type);
    wtk_string_t *str = qtk_pb_read_string(pb, dq);
    if (NULL == str) return -1;
    v->type = QPB_CTYPE_STRING;
    v->v.s = str;
    return 0;
}


int qtk_pb_decode_varint(qtk_protobuf_t *pb, qpb_variant_t *v) {
    assert(0 == v->type);
    assert(pb->buf_e > pb->buf_s);
    v->type = QPB_CTYPE_VARINT;
    return qtk_pb_read_varint64(pb, &v->v.u64);
}


int qtk_pb_decode_fixed32(qtk_protobuf_t *pb, qpb_variant_t *v) {
    assert(0 == v->type);
    assert(pb->buf_e >= pb->buf_s + sizeof(v->v.u32));
    v->type = QPB_CTYPE_FIXED32;
    memcpy(&v->v.u32, pb->buf_s, sizeof(v->v.u32));
    return 0;
}


int qtk_pb_decode_fixed64(qtk_protobuf_t *pb, qpb_variant_t *v) {
    assert(0 == v->type);
    assert(pb->buf_e >= pb->buf_s + sizeof(v->v.u64));
    v->type = QPB_CTYPE_FIXED64;
    memcpy(&v->v.u64, pb->buf_s, sizeof(v->v.u64));
    return 0;
}


int qtk_pb_decode_delimited(qtk_protobuf_t *pb, qtk_deque_t *dq, qpb_variant_t *v) {
    assert(0 == v->type);
    wtk_string_t *str = qtk_pb_read_string_ref(pb, dq);
    if (NULL == str) return -1;
    v->type = QPB_CTYPE_DELIMITED;
    v->v.s = str;
    return 0;
}


qpb_variant_t* qtk_pb_frame_get_var(qtk_decframe_t *frame, int idx) {
    assert(frame->slots->type == QPB_CTYPE_ARRAY || frame->slots->type == QPB_CTYPE_MESSAGE);
    qtk_array_t *arr = frame->slots->v.a;
    if (idx >= arr->len) {
        qtk_array_resize(arr, idx + 1);
    }
    qpb_variant_t *v = qtk_array_get(arr, idx);
    if (v->type == QPB_CTYPE_ARRAY) {
        v = qtk_array_push(v->v.a);
    } else if (v->type) {
        /* if variant is not empty and not array, change v to array {v}, and return the second  */
        qpb_variant_t v_arr;
        qpb_variant_init(&v_arr);
        qpb_var_set_array(&v_arr, 8);
        qpb_variant_t *v1 = qtk_array_push(v_arr.v.a);
        qpb_variant_t *v2 = qtk_array_push(v_arr.v.a);
        *v1 = *v;
        *v = v_arr;
        v = v2;
    }
    return v;
}


int qtk_pb_decode_field(qtk_protobuf_t *pb, qtk_deque_t *dq, qtk_decframe_t *frame) {
    uint32_t tag = 0;
    int field_number = 0;
    int wire_type = 0;

    qtk_pb_read_tag(pb, dq, &tag);
    field_number = tag >> 3;
    wire_type = tag & 0x07;
    switch (wire_type) {
    case QPB_WIRETYPE_VARINT:
        if (pb->buf_e - pb->buf_s < 10 && dq) {
            qtk_pb_fill_buffer(pb, dq);
        }
        return qtk_pb_decode_varint(pb, qtk_pb_frame_get_var(frame, field_number));
    case QPB_WIRETYPE_FIXED32:
        if (pb->buf_e - pb->buf_s < 4 && dq) {
            qtk_pb_fill_buffer(pb, dq);
        }
        return qtk_pb_decode_fixed32(pb, qtk_pb_frame_get_var(frame, field_number));
    case QPB_WIRETYPE_FIXED64:
        if (pb->buf_e - pb->buf_s < 8 && dq) {
            qtk_pb_fill_buffer(pb, dq);
        }
        return qtk_pb_decode_fixed64(pb, qtk_pb_frame_get_var(frame, field_number));
    case QPB_WIRETYPE_LENGTH_DELIMITED:
        return qtk_pb_decode_delimited(pb, dq, qtk_pb_frame_get_var(frame, field_number));
    case QPB_WIRETYPE_START_GROUP: {
        qpb_variant_t *v = qtk_pb_decode_frame(pb, dq, field_number);
        if (NULL == v) return -1;
        qtk_array_set(frame->slots->v.a, field_number, v);
        return 0;
    }
    case QPB_WIRETYPE_END_GROUP:
        assert(frame->group_number == field_number);
        frame->closed = 1;
        return 0;
    default:
        return -1;
    }
    return 0;
}


qpb_variant_t* qtk_pb_decode_frame(qtk_protobuf_t *pb, qtk_deque_t *dq, int group_number) {
    qtk_decframe_t frame;
    qpb_variant_t *v = NULL;
    qtk_frame_init(&frame, group_number);
    while (((dq && dq->len) || (pb->buf_e > pb->buf_s)) && !frame.closed) {
        qtk_pb_decode_field(pb, dq, &frame);
    }
    if (0 == group_number || frame.closed) {
        v = qtk_frame_get_slots(&frame);
    }
    qtk_frame_clean(&frame);
    return v;
}


qpb_variant_t* qtk_pb_decode(qtk_protobuf_t *pb, qtk_deque_t *dq) {
    qpb_variant_t *v = qtk_pb_decode_frame(pb, dq, 0);
    if (pb->v) {
        qpb_variant_delete(pb->v);
    }
    pb->v = v;
    return v;
}


int qtk_pb_decode_message(qtk_protobuf_t *pb, qpb_variant_t *v) {
    assert(v->type == QPB_CTYPE_DELIMITED);
    qtk_decframe_t frame;
    qtk_frame_init2(&frame, 0, v);
    wtk_string_t *str = v->v.s;
    assert(pb->buf_s == pb->buf_e);
    pb->buf_s = str->data;
    pb->buf_e = str->data + str->len;
    v->type = QPB_CTYPE_MESSAGE;
    v->v.a = qtk_array_new2(sizeof(qpb_variant_t), 8);
    while (pb->buf_e > pb->buf_s) {
        qtk_pb_decode_field(pb, NULL, &frame);
    }
    qtk_frame_get_slots(&frame);
    qtk_frame_clean(&frame);
    wtk_string_delete(str);
    return 0;
}


int qtk_pb_decode_array(qtk_protobuf_t *pb, qpb_variant_t *v, qpb_field_t *field) {
    if (v->type == QPB_CTYPE_DELIMITED) {
        /* repeated with packed=true */
        int ret = 0;
        wtk_string_t *str = v->v.s;
        assert(pb->buf_s == pb->buf_e);
        pb->buf_s = str->data;
        pb->buf_e = str->data + str->len;
        v->type = QPB_CTYPE_ARRAY;
        v->v.a = qtk_array_new2(sizeof(qpb_variant_t), 8);
        while (pb->buf_e > pb->buf_s) {
            qpb_variant_t *e = qtk_array_push(v->v.a);
            switch (field->desctype) {
            case QPB_DESCTYPE_INT32:
            case QPB_DESCTYPE_UINT32:
            case QPB_DESCTYPE_INT64:
            case QPB_DESCTYPE_UINT64:
            case QPB_DESCTYPE_SINT32:
            case QPB_DESCTYPE_BOOL:
            case QPB_DESCTYPE_ENUM:
                ret = qtk_pb_decode_varint(pb, e);
                break;
            case QPB_DESCTYPE_FIXED32:
            case QPB_DESCTYPE_SFIXED32:
            case QPB_DESCTYPE_FLOAT:
                ret = qtk_pb_decode_fixed32(pb, e);
                break;
            case QPB_DESCTYPE_DOUBLE:
            case QPB_DESCTYPE_FIXED64:
            case QPB_DESCTYPE_SFIXED64:
                ret = qtk_pb_decode_fixed64(pb, e);
                break;
            default:
                ret = -1;
            }
        }
        wtk_string_delete(str);
        if (ret < 0) return ret;
    }
#define QPB_FORMAT_CASE(ft, ct)                 \
    case ft:                                    \
        for (; s < e; s++) {                    \
            s->type = ct;                       \
        }                                       \
        break;
    if (v->type == QPB_CTYPE_ARRAY) {
        qtk_array_t *arr = v->v.a;
        qpb_variant_t *s = arr->data;
        qpb_variant_t *e = s + arr->len;
        switch (field->desctype) {
            QPB_FORMAT_CASE(QPB_DESCTYPE_DOUBLE, QPB_CTYPE_DOUBLE);
            QPB_FORMAT_CASE(QPB_DESCTYPE_FLOAT, QPB_CTYPE_FLOAT);
            QPB_FORMAT_CASE(QPB_DESCTYPE_INT64, QPB_CTYPE_INT64);
            QPB_FORMAT_CASE(QPB_DESCTYPE_UINT64, QPB_CTYPE_UINT64);
            QPB_FORMAT_CASE(QPB_DESCTYPE_FIXED64, QPB_CTYPE_UINT64);
            QPB_FORMAT_CASE(QPB_DESCTYPE_FIXED32, QPB_CTYPE_UINT32);
            QPB_FORMAT_CASE(QPB_DESCTYPE_STRING, QPB_CTYPE_STRING);
            QPB_FORMAT_CASE(QPB_DESCTYPE_BYTES, QPB_CTYPE_STRING);
            QPB_FORMAT_CASE(QPB_DESCTYPE_SFIXED64, QPB_CTYPE_INT64);
            QPB_FORMAT_CASE(QPB_DESCTYPE_SFIXED32, QPB_CTYPE_INT32);
        case QPB_DESCTYPE_INT32:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.i32 = s->v.u64;
                    s->type = QPB_CTYPE_INT32;
                }
            }
            break;
        case QPB_DESCTYPE_UINT32:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.u32 = s->v.u64;
                    s->type = QPB_CTYPE_UINT32;
                }
            }
            break;
        case QPB_DESCTYPE_BOOL:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.b = s->v.u64;
                    s->type = QPB_CTYPE_BOOL;
                }
            }
            break;
        case QPB_DESCTYPE_ENUM:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.e = s->v.u64;
                    s->type = QPB_CTYPE_BOOL;
                }
            }
            break;
        case QPB_DESCTYPE_SINT32:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.i32 = UNZIGZAG(s->v.u64);
                    s->type = QPB_CTYPE_INT32;
                }
            }
            break;
        case QPB_DESCTYPE_SINT64:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_VARINT) {
                    s->v.i64 = UNZIGZAG(s->v.u64);
                    s->type = QPB_CTYPE_INT64;
                }
            }
            break;
        case QPB_DESCTYPE_MESSAGE:
            for (; s < e; s++) {
                if (s->type == QPB_CTYPE_DELIMITED) {
                    qtk_pb_decode_message(pb, s);
                    s->type = QPB_CTYPE_MESSAGE;
                }
            }
            break;
        default:
            return -1;
        }
    }
#undef QPB_FORAMT_CASE
    return 0;
}


int qtk_pb_decode_variant(qtk_protobuf_t *pb, qpb_variant_t *v, qpb_field_t *field) {
    assert(v->type > QPB_MAX_CTYPE);
    if (field->label == QPB_LABEL_REPEATED) {
        field->label = QPB_LABEL_OPTIONAL;
        return qtk_pb_decode_array(pb, v, field);
    } else {
        switch (field->desctype) {
        case QPB_DESCTYPE_STRING:
        case QPB_DESCTYPE_BYTES:
            assert(v->type == QPB_CTYPE_DELIMITED);
            v->type = QPB_CTYPE_STRING;
            break;
        case QPB_DESCTYPE_MESSAGE:
            qtk_pb_decode_message(pb, v);
            break;
        default:
            return -1;
        }
        return 0;
    }
}


int qtk_pb_encode_field(qtk_protobuf_t *pb, qpb_variant_t *v, int field_number, qtk_deque_t *dq) {
    uint32_t tag;
    uint32_t wire_type;
    union {
        uint32_t fix32;
        uint64_t fix64;
        uint64_t vint;
        qtk_deque_t *dq;
    } sub;
    sub.dq = NULL;

    switch (v->type) {
    case QPB_CTYPE_INT32:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.i32;
        break;
    case QPB_CTYPE_INT64:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.i64;
        break;
    case QPB_CTYPE_UINT32:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.u32;
        break;
    case QPB_CTYPE_BOOL:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.b;
        break;
    case QPB_CTYPE_ENUM:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.e;
        break;
    case QPB_CTYPE_UINT64:
    case QPB_CTYPE_VARINT:
        wire_type = QPB_WIRETYPE_VARINT;
        sub.vint = v->v.u64;
        break;
    case QPB_CTYPE_FLOAT:
    case QPB_CTYPE_FIXED32:
        wire_type = QPB_WIRETYPE_FIXED32;
        sub.fix32 = v->v.u32;
        break;
    case QPB_CTYPE_DOUBLE:
    case QPB_CTYPE_FIXED64:
        wire_type = QPB_WIRETYPE_FIXED64;
        sub.fix64 = v->v.u64;
        break;
    case QPB_CTYPE_PACK: {
        qpb_variant_t *s = v->v.a->data;
        qpb_variant_t *e = s + v->v.a->len;
        sub.dq = qtk_deque_new(4096, 4096*1000, 1);
        for (; s < e && s->type; s++) {
            qtk_pb_encode_variant(pb, s, sub.dq);
        }
        wire_type = QPB_WIRETYPE_LENGTH_DELIMITED;
        break;
    }
    case QPB_CTYPE_DELIMITED:
    case QPB_CTYPE_STRING:
        wire_type = QPB_WIRETYPE_LENGTH_DELIMITED;
        if (field_number >= 0) {
            tag = ((uint32_t)field_number)<<3 | wire_type;
            qtk_pb_write_tag(pb, dq, tag);
        }
        qtk_pb_write_varint64(pb, dq, v->v.s->len);
        qtk_deque_push(dq, v->v.s->data,v->v.s->len);
        goto done;
    case QPB_CTYPE_ARRAY: {
        qpb_variant_t *s = v->v.a->data;
        qpb_variant_t *e = s + v->v.a->len;
        for (; s < e && s->type; s++) {
            qtk_pb_encode_field(pb, s, field_number, dq);
        }
        goto done;
    }
    case QPB_CTYPE_MESSAGE: {
        qpb_variant_t *s = v->v.a->data;
        qpb_variant_t *e = s + v->v.a->len;
        int i;
        sub.dq = qtk_deque_new(4096, 4096*1000, 1);
        for (i = 0; s < e; s++, i++) {
            if (0 == s->type) continue;
            qtk_pb_encode_field(pb, s, i, sub.dq);
        }
        wire_type = QPB_WIRETYPE_LENGTH_DELIMITED;
        break;
    }
    default:
        wire_type = QPB_WIRETYPE_VARINT;
        break;
    }

    if (field_number >= 0) {
        tag = ((uint32_t)field_number)<<3 | wire_type;
        qtk_pb_write_tag(pb, dq, tag);
    }

    switch (wire_type) {
    case QPB_WIRETYPE_VARINT:
        qtk_pb_write_varint64(pb, dq, sub.vint);
        break;
    case QPB_WIRETYPE_LENGTH_DELIMITED:
        if (sub.dq && sub.dq->len) {
            if (field_number >= 0) {
                qtk_pb_write_varint64(pb, dq, sub.dq->len);
            }
            qtk_deque_add(dq, sub.dq);
            qtk_deque_delete(sub.dq);
            sub.dq = NULL;
        }
        break;
    case QPB_WIRETYPE_FIXED32:
        qtk_deque_push(dq, (char*)&sub.fix32, sizeof(sub.fix32));
        break;
    case QPB_WIRETYPE_FIXED64:
        qtk_deque_push(dq, (char*)&sub.fix64, sizeof(sub.fix64));
        break;
    }
done:
    return 0;
}


int qtk_pb_encode_variant(qtk_protobuf_t *pb, qpb_variant_t *v, qtk_deque_t *dq) {
    return qtk_pb_encode_field(pb, v, -1, dq);
}
