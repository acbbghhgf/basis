#include <assert.h>
#include <ctype.h>
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/os/qtk_rlimit.h"
#include "wtk/os/wtk_thread.h"
#include "qtk/event/qtk_event_timer.h"
#include "qtk/core/qtk_abstruct_hashtable.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "qtk/core/qtk_int_hashtable.h"
#include "qtk/core/cJSON.h"
#include "qtk/mqtt/mqtt.h"


typedef struct test_bench test_bench_t;
typedef struct mqtt_waitack  mqtt_waitack_t;
typedef struct test_proc  test_proc_t;
typedef struct test_item test_item_t;

static int proc_read_test_file(test_proc_t *proc);


struct test_bench  {
    wtk_thread_t thread;
    qtk_sframe_method_t *method;
    qtk_event_timer_t *timer;
    const char *ip;
    const char *fn;
    const char *appid;
    const char *secretKey;
    test_proc_t *procs;
    qtk_abstruct_hashtable_t *proc_hash;
    int port;
    int device_start_id;
    int nloop;
    int nproc;
    int pre_time;
    int sleep;
    int durian_time;
    qtk_event_timer_cfg_t timer_cfg;
    uint32_t pub_count;
    uint32_t msg_count;
    uint32_t acked_count;
    unsigned run:1;
};

struct test_proc {
    test_bench_t *bench;
    test_item_t *cur_item;
    qtk_abstruct_hashtable_t* waits;
    wtk_queue_t item_q;
    qtk_hashnode_t sock_id;
    uint32_t pack_id;
    int pos;
    int loop;
    wtk_string_t *device;
    int recv_t;
    unsigned int waitack:1;
    unsigned int connected:1;
    unsigned int finished:1;
};

struct mqtt_waitack {
    qtk_hashnode_t pack_id;
    void *timer;
    test_proc_t *proc;
    MQTT_Package_t *pack;
};

struct test_item {
    wtk_queue_node_t node;
    MQTT_Package_t *pack;
    int loop;
    int sleep;
};


static int test_proc_start_item(test_proc_t *proc);
static int test_proc_handle(test_proc_t *proc);


uint32_t test_get_ms() {
    static uint64_t start_time = 0;
    if (0 == start_time) start_time = time_get_ms();
    return (uint64_t)time_get_ms() - start_time;
}


static wtk_string_t* test_string_dup(const char *p) {
    wtk_string_t *s;
    s = wtk_string_new(strlen(p)+1);
    strcpy(s->data, p);
    s->len--;
    return s;
}


static mqtt_waitack_t* mqtt_waitack_new(int pack_id, test_proc_t *proc) {
    mqtt_waitack_t *wait = wtk_malloc(sizeof(*wait));
    wait->pack_id.key._number = pack_id;
    wait->proc = proc;
    wait->pack = proc->cur_item->pack;
    return wait;
}


static int mqtt_waitack_delete(mqtt_waitack_t *wait) {
    if (wait->timer) {
        qtk_event_timer_remove(wait->proc->bench->timer, wait->timer);
    }
    wtk_free(wait);
    return 0;
}


static test_item_t* test_item_new() {
    test_item_t *item = wtk_malloc(sizeof(*item));
    wtk_queue_node_init(&item->node);
    item->pack = NULL;
    item->loop = 0;
    item->sleep = 0;
    return item;
}


static int test_item_delete(test_item_t *item) {
    if (item->pack && item->loop > 0) {
        mqtt_package_delete(item->pack);
    }
    wtk_free(item);
    return 0;
}


static int test_proc_init(test_proc_t *proc, test_bench_t *test, int device_id) {
    char tmp[32];
    proc->bench = test;
    sprintf(tmp, "%x", device_id);
    proc->device = test_string_dup(tmp);
    proc->waits = qtk_int_growhash_new(100, offsetof(mqtt_waitack_t, pack_id));
    proc->loop = 0;
    proc->pack_id = 0;
    proc->connected = 0;
    proc->finished = 0;
    wtk_queue_init(&proc->item_q);
    proc_read_test_file(proc);

    return 0;
}


static int test_wait_cleaner(void *udata, void *data) {
    return mqtt_waitack_delete(data);
}


static int test_proc_clean_waits(test_proc_t *proc) {
    qtk_hashtable_walk(proc->waits, test_wait_cleaner, proc);
    qtk_hashtable_reset(proc->waits);
    return 0;
}


static int test_proc_connect(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    qtk_sframe_connect_param_t *con_param;
    int sock_id = method->socket(method);
    qtk_hashtable_remove(test->proc_hash, &proc->sock_id.key);
    proc->sock_id.key._number = sock_id;
    proc->cur_item = NULL;
    qtk_hashtable_add(test->proc_hash, proc);
    test_proc_clean_waits(proc);

    msg = method->new(method, QTK_SFRAME_MSG_CMD, sock_id);
    con_param = MQTT_CONNECT_PARAM_NEW(test->ip, test->port, QTK_SFRAME_NOT_RECONNECT);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, con_param);
    method->push(method, msg);

    return 0;
}


static int test_proc_disconnect(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    int sock_id = proc->sock_id.key._number;

    msg = method->new(method, QTK_SFRAME_MSG_CMD, sock_id);
    method->set_cmd(msg, QTK_SFRAME_CMD_CLOSE, NULL);
    method->push(method, msg);
    proc->connected = 0;

    return 0;
}


static int test_proc_start(test_proc_t *proc, int delay) {
    test_bench_t *test = proc->bench;
    proc->connected = 0;
    qtk_event_timer_add(test->timer, delay,
                        (qtk_event_timer_handler)test_proc_connect, proc);

    return 0;
}


static int test_proc_clean(test_proc_t *proc) {
    wtk_queue_node_t *node;
    while ((node = wtk_queue_pop(&proc->item_q))) {
        test_item_delete(data_offset(node, test_item_t, node));
    }
    wtk_string_delete(proc->device);
    qtk_hashtable_delete(proc->waits);
    return 0;
}


static wtk_strbuf_t* test_read_file(const char *fn) {
    FILE *file = fopen(fn, "r");
    int fsize = file_length(file);
    wtk_strbuf_t *buf = wtk_strbuf_new(fsize+1, 1);
    assert(fsize == fread(buf->data, 1, fsize, file));
    buf->pos = fsize;
    wtk_strbuf_push_c(buf, '\0');
    return buf;
}


static void test_item_param_parse(void *udata, char *data, int len, int index) {
    cJSON *params = udata;
    char old = data[len];
    data[len] = '\0';
    char *p = strchr(data, '=');
    assert(p);
    *p++ = '\0';
    if ('"' == *p) {
        assert('"' == p[strlen(p)-1]);
        p[strlen(p)-1] = '\0';
        cJSON_AddStringToObject(params, data, p+1);
    } else if ('-' == *p || isdigit(*p)) {
        cJSON_AddNumberToObject(params, data, atoi(p));
    } else {
        assert(*p);
    }
    data[len] = old;
}


static int test_type_str_to_id(const char *type) {
    if (0 == strcmp(type, "connect")) return MQTT_CONNECT;
    else if (0 == strcmp(type, "connack")) return MQTT_CONNACK;
    else if (0 == strcmp(type, "publish")) return MQTT_PUBLISH;
    else if (0 == strcmp(type, "puback")) return MQTT_PUBACK;
    else if (0 == strcmp(type, "pubrec")) return MQTT_PUBREC;
    else if (0 == strcmp(type, "pubrel")) return MQTT_PUBREL;
    else if (0 == strcmp(type, "pubcomp")) return MQTT_PUBCOMP;
    else if (0 == strcmp(type, "subscribe")) return MQTT_SUBSCRIBE;
    else if (0 == strcmp(type, "suback")) return MQTT_SUBACK;
    else if (0 == strcmp(type, "unsubscribe")) return MQTT_UNSUBSCRIBE;
    else if (0 == strcmp(type, "unsuback")) return MQTT_UNSUBACK;
    else if (0 == strcmp(type, "pingreq")) return MQTT_PINGREQ;
    else if (0 == strcmp(type, "pingresp")) return MQTT_PINGRESP;
    else if (0 == strcmp(type, "disconnect")) return MQTT_DISCONNECT;
    return MQTT_Reserved;
}


static test_item_t* proc_read_item(test_proc_t *proc, const char *line, int len) {
    test_item_t *item = test_item_new();
    cJSON *obj, *param;
    int type;
    param = cJSON_CreateObject();
    wtk_str_split((char*)line, len, ' ', param, test_item_param_parse);
    obj = cJSON_GetObjectItem(param, "type");
    type = test_type_str_to_id(obj->valuestring);
    if (type >= MQTT_CONNECT && type <= MQTT_DISCONNECT) {
        MQTT_Package_t *pack = mqtt_package_new(type);
        switch (type) {
        case MQTT_CONNECT:
            obj = cJSON_GetObjectItem(param, "willflag");
            strcpy(pack->headers.connect.proto_name, "MQIsdp");
            pack->headers.connect.proto_level = 3;
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                pack->headers.connect.willflag = 1;
                obj = cJSON_GetObjectItem(param, "willretain");
                if (obj && 0 == strcmp(obj->valuestring, "1")) {
                    pack->headers.connect.willretn = 1;
                }
                if (obj) {
                    pack->headers.connect.willqos = atoi(obj->valuestring);
                }
            }
            obj = cJSON_GetObjectItem(param, "cleansession");
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                pack->headers.connect.cleanses = 1;
            }
            obj = cJSON_GetObjectItem(param, "username");
            if (obj && strlen(obj->valuestring)) {
                pack->headers.connect.username = 1;
            }
            obj = cJSON_GetObjectItem(param, "password");
            if (obj && strlen(obj->valuestring)) {
                pack->headers.connect.password = 1;
            }
            obj = cJSON_GetObjectItem(param, "keepalive");
            if (obj) {
                pack->headers.connect.keepalive = atoi(obj->valuestring);
            }
            obj = cJSON_GetObjectItem(param, "clientid");
            if (obj) {
                int len = strlen(obj->valuestring);
                pack->payload.connect.clientid = wtk_string_new(len+proc->device->len);
                memcpy(pack->payload.connect.clientid->data, obj->valuestring, len);
                memcpy(pack->payload.connect.clientid->data+len, proc->device->data, proc->device->len);
            } else {
                pack->payload.connect.clientid = wtk_string_dup_data(proc->device->data,
                                                                     proc->device->len);
            }
            if (pack->headers.connect.willflag) {
                int len;
                obj = cJSON_GetObjectItem(param, "willtopic");
                assert(obj);
                len = strlen(obj->valuestring);
                pack->payload.connect.willtopic = wtk_string_new(len);
                memcpy(pack->payload.connect.willtopic->data, obj->valuestring, len);
                obj = cJSON_GetObjectItem(param, "willmsg");
                assert(obj);
                len = strlen(obj->valuestring);
                pack->payload.connect.willmsg = wtk_string_new(len);
                memcpy(pack->payload.connect.willmsg->data, obj->valuestring, len);
            }
            if (pack->headers.connect.username) {
                obj = cJSON_GetObjectItem(param, "username");
                assert(obj);
                int len = strlen(obj->valuestring);
                pack->payload.connect.username = wtk_string_new(len);
                memcpy(pack->payload.connect.username->data, obj->valuestring, len);
            }
            if (pack->headers.connect.password) {
                obj = cJSON_GetObjectItem(param, "password");
                assert(obj);
                int len = strlen(obj->valuestring);
                pack->payload.connect.password = wtk_string_new(len);
                memcpy(pack->payload.connect.password->data, obj->valuestring, len);
            }
            break;
        case MQTT_CONNACK:
            obj = cJSON_GetObjectItem(param, "session_present");
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                pack->headers.connack.sess_present = 1;
            }
            if (obj) {
                pack->headers.connack.code = atoi(obj->valuestring);
            }
            break;
        case MQTT_PUBLISH:
        {
            int len;
            obj = cJSON_GetObjectItem(param, "topic");
            if (obj) {
                len = strlen(obj->valuestring);
                pack->headers.publish.topic = wtk_string_new(len);
                memcpy(pack->headers.publish.topic->data, obj->valuestring, len);
            } else {
                pack->headers.publish.topic = wtk_string_dup_data(proc->device->data, proc->device->len);
            }
            obj = cJSON_GetObjectItem(param, "dup");
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                pack->headers.publish.dup = 1;
            }
            obj = cJSON_GetObjectItem(param, "retain");
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                pack->headers.publish.retain = 1;
            }
            obj = cJSON_GetObjectItem(param, "qos");
            if (obj && strlen(obj->valuestring)) {
                pack->headers.publish.qos = atoi(obj->valuestring);
            }
            obj = cJSON_GetObjectItem(param, "message");
            if (obj) {
                int len = strlen(obj->valuestring);
                if (NULL == pack->payload.msg) {
                    pack->payload.msg = wtk_strbuf_new(len, 1);
                } else {
                    wtk_strbuf_reset(pack->payload.msg);
                }
                wtk_strbuf_push(pack->payload.msg, obj->valuestring, len);
            }
        }
        case MQTT_PUBACK:
        case MQTT_PUBREC:
        case MQTT_PUBREL:
        case MQTT_PUBCOMP:
        case MQTT_UNSUBACK:
            break;
        case MQTT_SUBSCRIBE:
        case MQTT_UNSUBSCRIBE:
        {
            MQTT_TopicFilter_t *tf;
            int len;
            char *p;
            /* only support one topic */
            obj = cJSON_GetObjectItem(param, "topic");
            tf = mqtt_topicfilter_new();
            if (obj){
                len = strlen(obj->valuestring);
                p = wtk_malloc(len);
                memcpy(p, obj->valuestring, len);
            } else {
                len = proc->device->len;
                p = wtk_malloc(len);
                memcpy(p, proc->device->data, len);
            }
            wtk_string_set(&tf->name, p, len);
            obj = cJSON_GetObjectItem(param, "qos");
            if (obj && strlen(obj->valuestring)) {
                tf->qos = atoi(obj->valuestring);
            }
            wtk_queue_push(&pack->payload.subscribe.topics, &tf->node);
            break;
        }
        case MQTT_SUBACK:
        {
            uint8_t code;
            /* only support one topic */
            obj = cJSON_GetObjectItem(param, "qos");
            if (obj && strlen(obj->valuestring)) {
                code = atoi(obj->valuestring);
            }
            obj = cJSON_GetObjectItem(param, "failure");
            if (obj && 0 == strcmp(obj->valuestring, "1")) {
                code |= 0x80;
            }
            break;
        }
        case MQTT_PINGREQ:
        case MQTT_PINGRESP:
        case MQTT_DISCONNECT:
            break;
        }
        obj = cJSON_GetObjectItem(param, "_loop");
        if (obj) {
            item->loop = atoi(obj->valuestring);
        } else {
            item->loop = 1;
        }
        obj = cJSON_GetObjectItem(param, "_sleep");
        if (obj) {
            item->sleep = atoi(obj->valuestring);
        } else {
            item->sleep = 0;
        }
        item->pack = pack;
    }
    cJSON_Delete(param);
    return item;
}


static int proc_read_test_file(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    wtk_strbuf_t *buf = test_read_file(test->fn);
    char *p, *s;
    test_item_t *item;
    if ('\n' != buf->data[buf->pos-1]) {
        wtk_strbuf_push_c(buf, '\n');
    }
    s = buf->data;
    while (*s) {
        int i;
        p = strchr(s, '\n');
        if (p) {
            *p = '\0';
            item = proc_read_item(proc, s, p-s);
        } else {
            p = s + strlen(s);
            item = proc_read_item(proc, s, p-s);
            p--;
        }
        wtk_queue_push(&proc->item_q, &item->node);
        for (i = 1; i < item->loop; i++) {
            test_item_t *item2 = test_item_new();
            item2->pack = item->pack;
            item2->sleep = item->sleep;
            wtk_queue_push(&proc->item_q, &item2->node);
        }
        s = p+1;
    }
    wtk_strbuf_delete(buf);
    return 0;
}

static int test_update_cfg(test_bench_t *test, wtk_local_cfg_t *lc) {
    wtk_string_t *v;

    wtk_local_cfg_update_cfg_str(lc, test, ip, v);
    wtk_local_cfg_update_cfg_i(lc, test, port, v);
    wtk_local_cfg_update_cfg_i(lc, test, device_start_id, v);
    wtk_local_cfg_update_cfg_i(lc, test, nproc, v);
    wtk_local_cfg_update_cfg_i(lc, test, nloop, v);
    wtk_local_cfg_update_cfg_i(lc, test, pre_time, v);
    wtk_local_cfg_update_cfg_i(lc, test, sleep, v);
    wtk_local_cfg_update_cfg_str(lc, test, fn, v);
    wtk_local_cfg_update_cfg_str(lc, test, appid, v);
    wtk_local_cfg_update_cfg_str(lc, test, secretKey, v);

    return 0;
}


static int test_proc_start_item(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    test_item_t *item = proc->cur_item;
    wtk_queue_node_t *node;
    int sleep = 0;

    if (NULL == item) {
        node = wtk_queue_peek(&proc->item_q, 0);
        if (NULL == node) return -1;
    } else {
        sleep = item->sleep;
        node = item->node.next;
    }
    if (NULL == node) {
        /* no more item */
        if (++proc->loop >= test->nloop) {
            test->durian_time = test_get_ms();
            proc->finished = 1;
            wtk_debug("*******finish\r\n");
        } else {
            test_proc_disconnect(proc);
            test_proc_connect(proc);
        }
        return 0;
    }
    item = data_offset(node, test_item_t, node);
    proc->cur_item = item;
    proc->pos = -1;
    proc->recv_t = 0;
    qtk_event_timer_add(test->timer, sleep,
                        (qtk_event_timer_handler)test_proc_handle, proc);
    return 0;
}


static void test_print_connect(MQTT_Package_t *pack) {
    printf(">> CONNECT\t(%s %d) clientid=%.*s keepalive=%d", pack->headers.connect.proto_name,
           pack->headers.connect.proto_level,
           pack->payload.connect.clientid->len,
           pack->payload.connect.clientid->data,
           pack->headers.connect.keepalive);
    if (pack->headers.connect.username) {
        printf(" username=%.*s", pack->payload.connect.username->len,
               pack->payload.connect.username->data);
    }
    if (pack->headers.connect.password) {
        printf(" password=%.*s", pack->payload.connect.password->len,
               pack->payload.connect.password->data);
    }
    if (pack->headers.connect.willflag) {
        printf(" willmessage=([%.*s]%.*s qos=%d retain=%d)",
               pack->payload.connect.willtopic->len,
               pack->payload.connect.willtopic->data,
               pack->payload.connect.willmsg->len,
               pack->payload.connect.willmsg->data,
               pack->headers.connect.willqos,
               pack->headers.connect.willretn);
    }
    if (pack->headers.connect.cleanses) {
        printf(" CLEANSESSION");
    }
    printf("\r\n");
}


static void test_print_publish(MQTT_Package_t *pack) {
    printf(">> PUBLISH\ttopic=%.*s message=%.*s",
           pack->headers.publish.topic->len,
           pack->headers.publish.topic->data,
           pack->payload.msg->pos, pack->payload.msg->data);
    if (pack->headers.publish.qos > 0) {
        printf(" id=%d qos=%d", pack->headers.publish.id,
               pack->headers.publish.qos);
    }
    if (pack->headers.publish.dup) {
        printf(" DUP");
    }
    if (pack->headers.publish.retain) {
        printf(" RETAIN");
    }
    printf("\r\n");
}


static int test_proc_repub(mqtt_waitack_t *wait) {
    test_proc_t *proc = wait->proc;
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    MQTT_Package_t *pack = wait->pack;
    int pack_id = wait->pack_id.key._number;
    qtk_sframe_msg_t *msg;
    msg = method->new(method, QTK_SFRAME_MSG_MQTT, proc->sock_id.key._number);
    pack->headers.publish.id = pack_id;
    pack->headers.publish.dup = 1;
    method->set_mqtt(msg, pack);
    pack->headers.publish.dup = 0;
    method->push(method, msg);

    return 1;
}


static int test_proc_handle(test_proc_t *proc) {
    test_item_t *item = proc->cur_item;
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    uint8_t type;
    qtk_sframe_msg_t *msg;
    mqtt_waitack_t *wait = NULL;

    if (item == NULL && !proc->connected) {
        return 0;
    }
    /*
      TODO: parse item->pack to msg
     */
    type = item->pack->type;
    msg = method->new(method, QTK_SFRAME_MSG_MQTT, proc->sock_id.key._number);
    if ((MQTT_PUBLISH == type && item->pack->headers.publish.qos > 0)
        || MQTT_SUBSCRIBE == type
        || MQTT_UNSUBSCRIBE == type) {
        item->pack->headers.publish.id = proc->pack_id++;
        wait = mqtt_waitack_new(item->pack->headers.publish.id, proc);
    }
    method->set_mqtt(msg, item->pack);

    if (wait) {
        wait->timer = qtk_event_timer_add(test->timer, 5000,
                                          (qtk_event_timer_handler)test_proc_repub, wait);
        qtk_hashtable_add(proc->waits, wait);
    } else if (type != MQTT_CONNECT) {
        qtk_event_timer_add(test->timer, 0,
                            (qtk_event_timer_handler)test_proc_start_item, proc);
    }

    switch (type) {
    case MQTT_CONNECT:
        test_print_connect(item->pack);
        break;
    case MQTT_PUBLISH:
        test->pub_count++;
        test_print_publish(item->pack);
        break;
    case MQTT_SUBSCRIBE:
    {
        wtk_queue_node_t *n;
        printf(">> SUBSCRIBE\tid=%d\r\n", item->pack->headers.subscribe.id);
        for (n = item->pack->payload.subscribe.topics.pop; n; n = n->next) {
            MQTT_TopicFilter_t *tf = data_offset(n, MQTT_TopicFilter_t, node);
            printf("\t\ttopicfilter=%.*s qos=%d\r\n", tf->name.len, tf->name.data, tf->qos);
        }
        break;
    }
    case MQTT_UNSUBSCRIBE:
    {
        wtk_queue_node_t *n;
        printf(">> UNSUBSCRIBE\tid=%d", item->pack->headers.unsubscribe.id);
        for (n = item->pack->payload.unsubscribe.topics.pop; n; n = n->next) {
            MQTT_TopicFilter_t *tf = data_offset(n, MQTT_TopicFilter_t, node);
            printf(" %.*s", tf->name.len, tf->name.data);
        }
        printf("\r\n");
        break;
    }
    case MQTT_DISCONNECT:
        printf(">> DISCONNECT\r\n");
        break;
    }
    return method->push(method, msg);
}


static int test_proc_puback(test_proc_t *proc, MQTT_Package_t *publish) {
    if (publish->headers.publish.qos == 1) {
        MQTT_Package_t ack;
        test_bench_t *test = proc->bench;
        qtk_sframe_method_t *method = test->method;
        qtk_sframe_msg_t *msg;
        ack.type = MQTT_PUBACK;
        ack.flag = 0;
        ack.length = 2;
        ack.headers.puback.id = publish->headers.publish.id;
        msg = method->new(method, QTK_SFRAME_MSG_MQTT, proc->sock_id.key._number);
        method->set_mqtt(msg, &ack);
        printf(">> PUBACK\tid=%d\r\n", ack.headers.puback.id);
        method->push(method, msg);
	return QTK_TIMER_AGAIN;
    }
    return 0;
}


static int test_proc_response(test_proc_t *proc, MQTT_Package_t *pack) {
    test_bench_t *test = proc->bench;
    int isack = 0;
    int pack_id = 0;
    switch (pack->type) {
    case MQTT_CONNACK:
        printf("<< CONNACK\tsp=%d, code=%d\r\n",
               pack->headers.connack.sess_present,
               pack->headers.connack.code);
        qtk_event_timer_add(test->timer, 0,
                            (qtk_event_timer_handler)test_proc_start_item, proc);
        break;
    case MQTT_PUBLISH:
        printf("<< Message from [%.*s]: %.*s\r\n",
               pack->headers.publish.topic->len,
               pack->headers.publish.topic->data,
               pack->payload.msg->pos, pack->payload.msg->data);
        test->msg_count++;
        test_proc_puback(proc, pack);
        break;
    case MQTT_PUBACK:
        isack = 1;
        pack_id = pack->headers.puback.id;
        printf("<< PUBACK\tid=%d\r\n", pack_id);
        break;
    case MQTT_SUBACK:
        isack = 1;
        pack_id = pack->headers.suback.id;
        printf("<< SUBACK\tid=%d\r\n", pack_id);
        break;
    case MQTT_UNSUBACK:
        isack = 1;
        pack_id = pack->headers.unsuback.id;
        printf("<< UNSUBACK\tid=%d\r\n", pack_id);
        break;
    case MQTT_PINGRESP:
        printf("<< PINGRESP\r\n");
        break;
    default:
        printf("response %d\r\n", pack->type);
        break;
    }
    if (isack) {
        qtk_hashable_t key;
        key._number = pack_id;
        mqtt_waitack_t *wait = qtk_hashtable_remove(proc->waits, &key);
        if (wait) {
            if (MQTT_PUBACK == pack->type) {
                test->acked_count++;
            }
            mqtt_waitack_delete(wait);
            qtk_event_timer_add(test->timer, 0,
                                (qtk_event_timer_handler)test_proc_start_item, proc);
        }
    }
    return 0;
}


static int test_process_notice(test_proc_t *proc, qtk_sframe_msg_t *msg) {
    qtk_sframe_method_t *method = proc->bench->method;
    int signal = method->get_signal(msg);

    switch (signal) {
    case QTK_SFRAME_SIGCONNECT:
        wtk_debug("connected\r\n");
        test_proc_start_item(proc);
        test_get_ms();
        break;
    case QTK_SFRAME_SIGDISCONNECT:
        wtk_debug("===============disconnect\r\n");
        test_proc_disconnect(proc);
        test_proc_connect(proc);
        break;
    case QTK_SFRAME_SIGECONNECT:
        wtk_debug("connect failed\r\n");
        test_proc_disconnect(proc);
        test_proc_connect(proc);
        break;
    default:
        break;
    }
    return 0;
}


static int test_route(void *data, wtk_thread_t *thread) {
    test_bench_t *test = data;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    test_proc_t *proc;
    while (test->run) {
        msg = method->pop(method, 10);
        if (msg) {
            int msg_type = method->get_type(msg);
            int sock_id = method->get_id(msg);
            proc = qtk_hashtable_find(test->proc_hash, (qtk_hashable_t*)&sock_id);
            if (proc && !proc->finished) {
                if (msg_type == QTK_SFRAME_MSG_NOTICE) {
                    test_process_notice(proc, msg);
                } else if (msg_type == QTK_SFRAME_MSG_MQTT) {
                    if (!method->get_signal(msg)) {
                        MQTT_Package_t *mqtt = mqtt_package_new();
                        method->get_mqtt(msg, mqtt);
                        test_proc_response(proc, mqtt);
                        mqtt_package_delete(mqtt);
                    }
                }
            }
            method->delete(method, msg);
        }
        qtk_event_timer_process(test->timer);
    }
    return 0;
}


void* mqtt_cli_new(void *frame) {
    test_bench_t *test = wtk_malloc(sizeof(*test));
    test->nloop = 1;
    test->nproc = 1;
    test->method = frame;
    test->run = 0;
    wtk_thread_init(&test->thread, test_route, test);
    wtk_local_cfg_t *lc = test->method->get_cfg(test->method);
    test_update_cfg(test, lc);
    test->procs = wtk_calloc(test->nproc, sizeof(test->procs[0]));
    test->proc_hash = qtk_int_hashtable_new(test->nproc * 1.2, offsetof(test_proc_t, sock_id));
    qtk_event_timer_cfg_init(&test->timer_cfg);
    test->timer = qtk_event_timer_new(&test->timer_cfg);
    qtk_event_timer_init(test->timer, &test->timer_cfg);
    qtk_rlimit_set_nofile(1024*1024);

    return test;
}


int mqtt_cli_start(test_bench_t *test) {
    int i;
    int gap = (test->pre_time + test->nproc) / test->nproc;
    int delay = 0;
    test->pub_count = 0;
    test->msg_count = 0;
    test->acked_count = 0;
    test->durian_time = 0;
    for (i = 0; i < test->nproc; i++) {
        test_proc_init(test->procs + i, test, test->device_start_id + i);
        test_proc_start(test->procs + i, delay);
        delay += gap;
    }
    test->run = 1;
    wtk_thread_start(&test->thread);
    return 0;
}


int mqtt_cli_stop(test_bench_t *test) {
    int i;
    if (0 == test->durian_time) {
        test->durian_time = test_get_ms();
    }
    printf("##############################\r\n");
    printf("publish: %d\r\n", test->pub_count);
    printf("acked publish: %d\r\n", test->acked_count);
    printf("message: %d\r\n", test->msg_count);
    printf("time: %d\r\n", test->durian_time);
    printf("##############################\r\n");
    test->run = 0;
    wtk_thread_join(&test->thread);
    for (i = 0; i < test->nproc; i++) {
        test_proc_clean(test->procs + i);
    }
    return 0;
}


int mqtt_cli_delete(test_bench_t *test) {
    qtk_event_timer_delete(test->timer);
    qtk_hashtable_delete(test->proc_hash);
    wtk_free(test->procs);
    wtk_free(test);
    return 0;
}
