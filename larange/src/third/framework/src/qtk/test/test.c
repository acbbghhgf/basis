#include <assert.h>
#include <ctype.h>
#include "qtk/sframe/qtk_sframe.h"
#include "qtk/core/cJSON.h"
#include "wtk/os/wtk_thread.h"
#include "wtk/core/wtk_os.h"
#include "wtk/core/cfg/wtk_local_cfg.h"
#include "qtk/event/qtk_event_timer.h"
#include "qtk/core/qtk_str_hashtable.h"
#include "qtk/core/qtk_int_hashtable.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/engine.h>
#include "qtk/os/qtk_rlimit.h"


static char pub_key[] = "-----BEGIN RSA PUBLIC KEY-----\n\
MIGJAoGBALduScYh/dyHQm5uwZd0h/Wn9iqp7iczKehh2y93oWY8GHVdoFKxFddm\n\
reMrJKZmPzXAN9UfxLbOZagDfa0yZAycUBy6l5v/Avm2b+S0zIm+14T2MGg5MFTv\n\
A8DuIGO/lYyJOgD/vze03YGo3bXXVD+mmZuuXn8LOdlhNnntXV8pAgMBAAE=\n\
-----END RSA PUBLIC KEY-----\n\
";
static const char phrase[] = "qdreamer";


static const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


static char* base64_encode( const unsigned char * bindata, char * base64, int binlength ) {
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;

        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}


static char *my_sha1_hex(const char *str) {
    static const char *ptab = "0123456789abcdef";
    char buf[50];
    char *p_hex;
    int i,j;
    unsigned int sha_len;
    EVP_MD_CTX c;
    EVP_MD_CTX_init(&c);
    EVP_Digest(str, strlen(str), (unsigned char*)buf, &sha_len, EVP_sha1(), NULL);
    p_hex = malloc(sha_len * 2 + 4);
    for (i = 0, j = 0; i < sha_len; i++, j+=2) {
        p_hex[j] = ptab[(buf[i]>>4) & 0x0f];
        p_hex[j+1] = ptab[buf[i] & 0x0f];
    }
    p_hex[j] = 0;
    EVP_MD_CTX_cleanup(&c);
    return p_hex;
}


static RSA* rsa_new(char *pubkey, const char *passwd) {
    RSA *p_rsa = NULL;

    BIO *bio = BIO_new_mem_buf(pubkey, -1);
    if (NULL == bio) {
        ERR_print_errors_fp(stdout);
        return NULL;
    }
    p_rsa = RSA_new();
    PEM_read_bio_RSAPublicKey(bio,&p_rsa,NULL,"qdreamer");
    if (p_rsa == NULL) {
        ERR_print_errors_fp(stdout);
    }
    BIO_free_all(bio);
    return p_rsa;
}


static unsigned char *rsa_encrypt(char *str, int *enc_len, RSA *p_rsa) {
    unsigned char *p_en;
    int flen,rsa_len;
    char *sha1;
    sha1 = my_sha1_hex(str);
    //printf("sha1: %s\n", sha1);
    str = sha1;
    flen=strlen(str);
    rsa_len=RSA_size(p_rsa);
    p_en=(unsigned char *)malloc(rsa_len+1);
    *enc_len = rsa_len;
    memset(p_en,0,rsa_len+1);
    if (RSA_public_encrypt(flen, (unsigned char *)str,
                           (unsigned char*)p_en, p_rsa,
                           RSA_PKCS1_PADDING) < 0) {
        ERR_print_errors_fp(stdout);
        free(sha1);
        return NULL;
    }
    free(sha1);
    return p_en;
}


uint32_t test_get_ms() {
    static uint64_t start_time = 0;
    if (0 == start_time) start_time = time_get_ms();
    return (uint64_t)time_get_ms() - start_time;
}


typedef struct test_bench test_bench_t;
typedef struct test_stream  test_stream_t;
typedef struct test_proc  test_proc_t;
typedef struct test_item test_item_t;

struct test_bench  {
    wtk_thread_t thread;
    qtk_sframe_method_t *method;
    wtk_queue_t item_q;
    qtk_event_timer_t *timer;
    RSA *rsa;
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
    int timeslice;
    int pre_time;
    int rand_sleep;
    qtk_event_timer_cfg_t timer_cfg;
    uint64_t dly_sum;
    uint32_t dly_ave;
    uint32_t count;
    unsigned run:1;
};

struct test_stream {
    qtk_hashnode_t hook;
    int send_t;
};

struct test_proc {
    test_bench_t *bench;
    qtk_abstruct_hashtable_t *waits;
    test_item_t *cur_item;
    qtk_hashnode_t sock_id;
    int pos;
    int loop;
    wtk_string_t *device;
    int hook;
    int sleep;                  /* -1: send next stream without response
                                   0: immediately send next stream if recv response
                                   >0: sleep x ms after recv response (recv_t is set when first response)
                                 */
    int recv_t;
    unsigned int auth:1;
};

struct test_item {
    wtk_queue_node_t node;
    const char *param;
    wtk_string_t audio;
    const char *audio_type;
    int chunk;
};


static int test_proc_start_item(test_proc_t *proc);
static int test_proc_handle(test_proc_t *proc);


static wtk_string_t* test_string_dup(const char *p) {
    wtk_string_t *s;
    s = wtk_string_new(strlen(p)+1);
    strcpy(s->data, p);
    s->len--;
    return s;
}


static test_stream_t* test_stream_new(unsigned int hook) {
    char tmp[32];
    int n = sprintf(tmp, "%x", hook);
    char *p = wtk_malloc(n+1);
    strcpy(p, tmp);
    test_stream_t *stream = wtk_malloc(sizeof(*stream));
    wtk_string_set(&stream->hook.key._str, p, n);
    stream->send_t = test_get_ms();
    return stream;
}


static int test_stream_delete(test_stream_t *stream) {
    wtk_free(stream->hook.key._str.data);
    wtk_free(stream);
    return 0;
}


static test_item_t* test_item_new() {
    test_item_t *item = wtk_malloc(sizeof(*item));
    wtk_queue_node_init(&item->node);
    wtk_string_set(&item->audio, NULL, 0);
    item->param = NULL;
    return item;
}


static int test_item_delete(test_item_t *item) {
    if (item->audio.data) {
        wtk_free(item->audio.data);
    }
    if (item->param) {
        wtk_free((char*)item->param);
    }
    wtk_free(item);
    return 0;
}


static int test_proc_init(test_proc_t *proc, test_bench_t *test, int device_id) {
    char tmp[32];
    proc->bench = test;
    sprintf(tmp, "%x", device_id);
    proc->device = test_string_dup(tmp);
    proc->hook = 0;
    proc->waits = qtk_str_hashtable_new(10, offsetof(test_stream_t, hook));
    proc->auth = 0;
    proc->sleep = 0;
    proc->loop = 0;
    return 0;
}


static int test_proc_connect(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    qtk_sframe_connect_param_t *con_param;
    int sock_id = proc->sock_id.key._number;

    msg = method->new(method, QTK_SFRAME_MSG_CMD, sock_id);
    con_param = CONNECT_PARAM_NEW(test->ip, test->port, QTK_SFRAME_NOT_RECONNECT);
    method->set_cmd(msg, QTK_SFRAME_CMD_OPEN, con_param);
    method->push(method, msg);

    return 0;
}


static int test_proc_start(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    int sock_id = method->socket(method);

    proc->sock_id.key._number = sock_id;
    qtk_hashtable_add(test->proc_hash, proc);
    qtk_event_timer_add(test->timer, rand()%test->pre_time,
                        (qtk_event_timer_handler)test_proc_connect, proc);

    return 0;
}


static int test_proc_auth(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg;
    char signbuf[512];
    char b64buf[512+256];
    char *ptr_en, *ptr_req;
    int enc_len;
    long cur_time = time(NULL);
    char str_time[16];

    msg = method->new(method, QTK_SFRAME_MSG_REQUEST, proc->sock_id.key._number);
    method->set_method(msg, HTTP_POST);
    method->set_url(msg, "/auth", 5);
    method->add_header(msg, "Content-Type", "application/x-www-form-urlencoded");
    snprintf(str_time, sizeof(str_time), "%ld", cur_time);
    snprintf(signbuf, sizeof(signbuf), "%s%.*s%s%s", test->appid, proc->device->len,
             proc->device->data, str_time, test->secretKey);
    ptr_en = (char*)rsa_encrypt(signbuf, &enc_len, test->rsa);
    base64_encode((unsigned char*)ptr_en, b64buf, enc_len);
    cJSON *jp = cJSON_CreateObject();
    cJSON_AddStringToObject(jp, "appId", test->appid);
    cJSON_AddStringToObject(jp, "deviceId", proc->device->data);
    cJSON_AddStringToObject(jp, "time", str_time);
    cJSON_AddStringToObject(jp, "sign", b64buf);
    ptr_req = cJSON_PrintUnformatted(jp);
    cJSON_Delete(jp);
    method->add_body(msg, ptr_req, strlen(ptr_req));
    method->push(method, msg);
    wtk_free(ptr_req);
    wtk_free(ptr_en);
    return 0;
}


static int test_proc_clean(test_proc_t *proc) {
    wtk_string_delete(proc->device);
    qtk_hashtable_delete(proc->waits);
    return 0;
}


/*
  0 : wav
  1 : ogg
  -1 : unrecognized audio type
 */
#define WAV 0
#define OGG 1
static int get_audio_type(const char *fn) {
    int len = strlen(fn);
    if (0 == strcmp(fn+len-4, ".wav")) {
        return WAV;
    } else if (0 == strcmp(fn+len-4, ".ogg")) {
        return OGG;
    }
    return -1;
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


static int test_read_audio(test_item_t *item, const char *fn, int timeslice) {
    FILE *file = fopen(fn, "rb");
    int fsize = file_length(file);
    char *p = NULL;
    switch (get_audio_type(fn)) {
    case WAV:
        fsize -= 44;
        p = wtk_malloc(max(fsize, 44));
        assert(44 == fread(p, 1, 44, file));
        item->chunk = timeslice * 32000 / 1000;
        item->audio_type = "audio/x-wav";
        break;
    case OGG:
        p = wtk_malloc(fsize);
        item->chunk = timeslice * 6592 / 1000;
        item->audio_type = "audio/ogg";
        break;
    default:
        assert(fn);
    }
    assert(fsize == fread(p, 1, fsize, file));
    wtk_string_set(&item->audio, p, fsize);
    fclose(file);
    return 0;
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


static test_item_t* test_read_item(test_bench_t *test, const char *line, int len) {
    test_item_t *item = test_item_new();
    cJSON *obj, *param, *request;
    param = cJSON_CreateObject();
    request = cJSON_CreateObject();
    wtk_str_split((char*)line, len, ' ', request, test_item_param_parse);
    obj = cJSON_GetObjectItem(request, "audio");
    if (obj) {
        assert(obj->type==cJSON_String);
        const char *fn = obj->valuestring;
        test_read_audio(item, fn, test->timeslice);
        int type = get_audio_type(fn);
        cJSON_DeleteItemFromObject(request, "audio");
        obj = cJSON_CreateObject();
        cJSON_AddNumberToObject(obj, "sampleRate", 16000);
        cJSON_AddNumberToObject(obj, "sampleBytes", 2);
        cJSON_AddNumberToObject(obj, "channel", 1);
        switch (type) {
        case WAV:
            cJSON_AddStringToObject(obj, "audioType", "audio/x-wav");
            break;
        case OGG:
            cJSON_AddStringToObject(obj, "audioType", "ogg");
            break;
        default:
            break;
        }
        cJSON_AddItemToObject(param, "audio", obj);
    }
    cJSON_AddItemToObject(param, "request", request);
    item->param = cJSON_PrintUnformatted(param);
    cJSON_Delete(param);
    return item;
}


static int test_read_test_file(test_bench_t *test) {
    wtk_queue_init(&test->item_q);
    wtk_strbuf_t *buf = test_read_file(test->fn);
    char *p, *s;
    test_item_t *item;
    if ('\n' != buf->data[buf->pos-1]) {
        wtk_strbuf_push_c(buf, '\n');
    }
    s = buf->data;
    while ((p = strchr(s, '\n'))) {
        *p = '\0';
        item = test_read_item(test, s, p-s);
        wtk_queue_push(&test->item_q, &item->node);
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
    wtk_local_cfg_update_cfg_i(lc, test, timeslice, v);
    wtk_local_cfg_update_cfg_i(lc, test, pre_time, v);
    wtk_local_cfg_update_cfg_i(lc, test, rand_sleep, v);
    wtk_local_cfg_update_cfg_str(lc, test, fn, v);
    wtk_local_cfg_update_cfg_str(lc, test, appid, v);
    wtk_local_cfg_update_cfg_str(lc, test, secretKey, v);

    return 0;
}


static int test_proc_start_item(test_proc_t *proc) {
    test_bench_t *test = proc->bench;
    test_item_t *item = proc->cur_item;
    wtk_queue_node_t *node;
    if (proc->sleep > 0) {
        /* delay proc->sleep ms */
        qtk_event_timer_add(test->timer, proc->sleep,
                            (qtk_event_timer_handler)test_proc_start_item, proc);
        proc->sleep = 0;
        return 0;
    }
    if (NULL == item) {
        node = wtk_queue_peek(&test->item_q, 0);
        if (NULL == node) return -1;
    } else {
        node = item->node.next;
    }
    if (NULL == node) {
        /* no more item */
        if (++proc->loop >= test->nloop) {
            if (0 == test->dly_ave) {
                test->dly_ave = test->dly_sum / test->count;
            }
            wtk_debug("finish\r\n");
            return 0;
        } else {
            node = wtk_queue_peek(&test->item_q, 0);
        }
    }
    item = data_offset(node, test_item_t, node);
    proc->cur_item = item;
    proc->pos = -1;
    proc->recv_t = 0;
    proc->hook++;
    test_stream_t *stream = test_stream_new(proc->hook);
    qtk_hashtable_add(proc->waits, stream);
    qtk_event_timer_add(test->timer, 0,
                        (qtk_event_timer_handler)test_proc_handle, proc);
    return 0;
}


static int test_proc_handle(test_proc_t *proc) {
    test_item_t *item = proc->cur_item;
    test_bench_t *test = proc->bench;
    qtk_sframe_method_t *method = test->method;
    qtk_sframe_msg_t *msg = method->new(method, QTK_SFRAME_MSG_REQUEST, proc->sock_id.key._number);
    int left = -1;
    char str_hook[128];
    snprintf(str_hook, sizeof(str_hook), "%x", proc->hook);
    if (NULL == item->audio.data || proc->pos < 0) {
        /* start */
        char start_body[1024];
        int n = snprintf(start_body, sizeof(start_body), "{\"cmd\":\"start\",\"param\":%s}", item->param);
        method->add_body(msg, start_body, n);
        method->add_header(msg, "Content-Type", "text/plain");
        method->add_header(msg, "hook", str_hook);
        if (item->audio.data) {
            left = item->audio.len;
            proc->pos = 0;
        }
    } else {
        left = item->audio.len - proc->pos;
        int chunk = min(left, item->chunk);
        method->add_body(msg, item->audio.data+proc->pos, chunk);
        method->add_header(msg, "Content-Type", item->audio_type);
        method->add_header(msg, "hook", str_hook);
        proc->pos += chunk;
        left -= chunk;
    }
    method->set_method(msg, HTTP_POST);
    method->set_url(msg, "/", 1);
    method->push(method, msg);
    if (left == 0) {
        char stop_body[] = "{\"cmd\":\"stop\"}";
        msg = method->new(method, QTK_SFRAME_MSG_REQUEST, proc->sock_id.key._number);
        method->set_method(msg, HTTP_POST);
        method->set_url(msg, "/", 1);
        method->add_header(msg, "Content-Type", "text/plain");
        method->add_header(msg, "hook", str_hook);
        method->add_body(msg, stop_body, sizeof(stop_body)-1);
        method->push(method, msg);
        wtk_string_t hook;
        wtk_string_set(&hook, str_hook, strlen(str_hook));
        test_stream_t *stream = qtk_hashtable_find(proc->waits, (qtk_hashable_t*)&hook);
        assert(stream);
        stream->send_t = test_get_ms();
        proc->sleep = 10;
    } else if (left > 0) {
        int chunk = min(left, item->chunk);
        qtk_event_timer_add(test->timer, chunk * test->timeslice / item->chunk,
                            (qtk_event_timer_handler)test_proc_handle, proc);
    }
    return 0;
}


static int test_proc_response(test_proc_t *proc, wtk_string_t *hook,
                              wtk_string_t *type, wtk_strbuf_t *buf) {
    assert(type);
    test_stream_t *stream = qtk_hashtable_find(proc->waits, (qtk_hashable_t*)hook);
    if (stream) {
        proc->recv_t = test_get_ms();
        int delay = proc->recv_t - stream->send_t;
        proc->bench->dly_sum += delay;
        proc->bench->count++;
        qtk_hashtable_remove(proc->waits, (qtk_hashable_t*)hook);
        test_stream_delete(stream);
    }
    if (0 == wtk_string_cmp_s(type, "text/plain")) {
        printf("===>\t%.*s\r\n", buf->pos, buf->data);
    } else if (0 == wtk_string_cmp_s(type, "audio/mpeg")) {
        proc->sleep += buf->pos * 1000 / 3200;
    } else {
        assert(type->data);
    }
    return 0;
}


static int test_process_notice(test_proc_t *proc, qtk_sframe_msg_t *msg) {
    qtk_sframe_method_t *method = proc->bench->method;
    int signal = method->get_signal(msg);

    switch (signal) {
    case QTK_SFRAME_SIGCONNECT:
        wtk_debug("connected\r\nauth...\r\n");
        test_proc_auth(proc);
        break;
    case QTK_SFRAME_SIGDISCONNECT:
        wtk_debug("disconnect\r\n");
        break;
    case QTK_SFRAME_SIGECONNECT:
        wtk_debug("connect failed\r\n");
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
            assert(proc);
            if (msg_type == QTK_SFRAME_MSG_NOTICE) {
                test_process_notice(proc, msg);
                method->delete(method, msg);
                continue;
            }
            int status = method->get_status(msg);
            wtk_strbuf_t* body = method->get_body(msg);
            if (proc->auth) {
                wtk_string_t *hook = method->get_header(msg, "hook");
                wtk_string_t *type = method->get_header(msg, "Content-Type");
                test_proc_response(proc, hook, type, body);
                if (status == 200 || status >= 500) {
                    if (status >= 500) {
                        proc->sleep = rand()%test->rand_sleep;
                        printf("[ERROR]%s %s %.*s: %.*s\r\n", test->appid, proc->device->data,
                               hook->len, hook->data, body->pos, body->data);
                    }
                    if (proc->sleep > 0 && proc->recv_t > 0) {
                        uint32_t now = test_get_ms();
                        int sleep = now - proc->recv_t;
                        proc->sleep += rand()%test->rand_sleep;
                        if (proc->sleep > sleep) {
                            proc->sleep -= sleep;
                        } else {
                            proc->sleep = 0;
                        }
                    }
                    if (proc->sleep >= 0) {
                        qtk_event_timer_add(proc->bench->timer, proc->sleep,
                                            (qtk_event_timer_handler)test_proc_start_item, proc);
                    }
                }
            } else {
                /* /auth response */
                if (status == 200) {
                    /* auth pass */
                    proc->auth = 1;
                    wtk_debug("auth pass\r\n");
                    test_proc_start_item(proc);
                } else {
                    /* auth failed */
                    if (body) {
                        wtk_debug("auth failed : [%d], [%d]%.*s\r\n", status, body->pos, body->pos, body->data);
                    } else {
                        wtk_debug("auth failed\r\n");
                    }
                }
            }
            method->delete(method, msg);
        }
        qtk_event_timer_process(test->timer);
    }
    return 0;
}


void* test_new(void *frame) {
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
    OpenSSL_add_all_ciphers();
    test->rsa = rsa_new(pub_key, phrase);
    assert(test->rsa);
    qtk_rlimit_set_nofile(1024*1024);
    return test;
}


int test_start(test_bench_t *test) {
    int i;
    test->dly_sum = 0;
    test->dly_ave = 0;
    test->count = 0;
    test_read_test_file(test);
    for (i = 0; i < test->nproc; i++) {
        test_proc_init(test->procs + i, test, test->device_start_id + i);
        test_proc_start(test->procs + i);
    }
    test->run = 1;
    wtk_thread_start(&test->thread);
    return 0;
}


int test_stop(test_bench_t *test) {
    int i;
    if (0 == test->dly_ave) {
        test->dly_ave = test->dly_sum / test->count;
    }
    printf("##############################\r\n");
    printf("delay: %d\r\n", test->dly_ave);
    printf("##############################\r\n");
    test->run = 0;
    wtk_thread_join(&test->thread);
    for (i = 0; i < test->nproc; i++) {
        test_proc_clean(test->procs + i);
    }
    return 0;
}


int test_delete(test_bench_t *test) {
    wtk_queue_node_t *node;
    while ((node = wtk_queue_pop(&test->item_q))) {
        test_item_delete(data_offset(node, test_item_t, node));
    }
    qtk_event_timer_delete(test->timer);
    qtk_hashtable_delete(test->proc_hash);
    wtk_free(test->procs);
    RSA_free(test->rsa);
    wtk_free(test);
    return 0;
}
