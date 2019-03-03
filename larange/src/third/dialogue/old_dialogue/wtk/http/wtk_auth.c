#include <ctype.h>
#include <errno.h>
#include "wtk_auth.h"
#include "../..//third/sha1/sha1.h"

const char *http_post = "POST /upload/ HTTP/1.1\r\n";
const char *http_content_type = "Content-Type: application/x-www-form-urlencoded\r\n";
const char *http_content_length = "Content-Length: ";

void wtk_http_print(char *buf, int len)
{
    int i = 0;

    for (i = 0; i < len; i++) {
        if (isprint(buf[i])) {
            fprintf(stdout, "%c", buf[i]);
        } else if (buf[i] == '\r' && buf[i+1] == '\n') {
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\n");
}

long wtk_get_current_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);

    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void wtk_get_macaddr(char *macaddr)
{
    char *eth0 = "/sys/class/net/eth0/address";
    char *wlan0 = "/sys/class/net/wlan0/address";
    wtk_strbuf_t *buf = NULL;

    buf = wtk_strbuf_new(32, 1.0);
    wtk_strbuf_reset(buf);

    if (access(wlan0, F_OK) == 0) {
        wtk_strbuf_read(buf, wlan0);
    } else if (access(eth0, F_OK) == 0) {
        wtk_strbuf_read(buf, eth0);
    }

    if (buf) {
        memcpy(macaddr, buf->data, buf->pos - 1);
        wtk_strbuf_delete(buf);
    }
}

int wtk_http_generate_sig(SHA1Context *ctx, char *data, int bytes, wtk_strbuf_t *output)
{
    int i;
    int ret = -1;
	char tmp[48];

    SHA1Reset(ctx);
    wtk_strbuf_reset(output);

    SHA1Input(ctx, (const unsigned char*)data, bytes);
    ret = SHA1Result(ctx);
    if (ret != 1) {
		goto end;
	}

    for (i = 0; i < 5 ; ++i){
		sprintf(tmp, "%.8x", ctx->Message_Digest[i]);
		wtk_strbuf_push_string(output, tmp); 
    }

    ret = 0;

end:
    return ret;
}

void wtk_http_auth_cfg_reset(wtk_http_auth_cfg_t *cfg)
{
    cfg->usrid = NULL;
    cfg->seckey = NULL;
    cfg->appid = NULL;
    cfg->tm = NULL;
    cfg->sig = NULL;
    cfg->dev = NULL;
}

void wtk_http_auth_cfg_update(wtk_http_auth_cfg_t *cfg)
{
    char tm[128]  = { 0 };
    char dev[32] = { 0 };
    long tms = 0;

	wtk_strbuf_t *tmpbuf;
	wtk_strbuf_t *output;
	SHA1Context ctx;

    tms = wtk_get_current_time();
    sprintf(tm, "%ld\n", tms);
    cfg->tm = wtk_str_dup(tm);
	
    tmpbuf = wtk_strbuf_new(128, 1.0);
    output = wtk_strbuf_new(128, 1.0);

    memset(tmpbuf->data, 0, tmpbuf->length);
    memset(output->data, 0, output->length);

	wtk_strbuf_push_string(tmpbuf, cfg->appid);
	wtk_strbuf_push_string(tmpbuf, cfg->seckey);
	wtk_strbuf_push(tmpbuf, tm, strlen(tm) -1);
	wtk_http_generate_sig(&ctx, tmpbuf->data, tmpbuf->pos, output);
    cfg->sig = wtk_str_dup(output->data);

    wtk_get_macaddr(dev);
    cfg->dev = wtk_str_dup(dev);
    cfg->usrid = wtk_str_dup(dev);

	wtk_strbuf_delete(tmpbuf);
	wtk_strbuf_delete(output);
}

/*************** HTTP auth header ****************
 * POST /upload/ HTTP/1.1
 * Host: ip:port
 * Content-Type: application/x-www-form-urlencoded
 * Content-Length: 
 */
int wtk_http_pack_header(wtk_strbuf_t *buf, char *ip, char *port, int len)
{
    char tmp[128];

    wtk_strbuf_push_string(buf, http_post);

    //Host: 121.199.26.244:9000
    wtk_strbuf_push_string(buf, "Host: ");
    wtk_strbuf_push_string(buf, ip);
    wtk_strbuf_push(buf, ":", 1);
    wtk_strbuf_push_string(buf, port);
    wtk_strbuf_push(buf, "\r\n", 2);

    wtk_strbuf_push_string(buf, http_content_type);
    wtk_strbuf_push_string(buf, http_content_length);

    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%d", len);
    wtk_strbuf_push(buf, tmp, strlen(tmp));

    wtk_strbuf_push(buf, "\r\n", 2);
    wtk_strbuf_push(buf, "\r\n", 2);

    return buf->pos;
}

void wtk_http_pack_form_data(wtk_strbuf_t *buf, char *key, char *value)
{
    wtk_strbuf_push_string(buf, key);
    wtk_strbuf_push(buf, "=", 1);

    if (strncmp(key, "tm", strlen(key)) == 0) {
        wtk_strbuf_push(buf, value, strlen(value) -1);
    } else {
        wtk_strbuf_push_string(buf, value);
    }

    if (strncmp(key, "data", strlen(key)) == 0) {
        wtk_strbuf_push(buf, "\r\n", 2);
    } else {
        wtk_strbuf_push(buf, "&", 1);
    }
}

/*************** HTTP auth body ****************
 *
 * type=ath&usr=usrid&app=appid&tm=timestamp&sig=signature&dev=macaddr&data=cldrec
 */
void wtk_http_pack_body(wtk_strbuf_t *buf, wtk_http_auth_cfg_t *cfg)
{
    wtk_strbuf_push_string(buf, "type=ath");
    wtk_strbuf_push(buf, "&", 1);

    wtk_http_pack_form_data(buf, "usr", cfg->usrid);
    wtk_http_pack_form_data(buf, "app", cfg->appid);
    wtk_http_pack_form_data(buf, "tm", cfg->tm);
    wtk_http_pack_form_data(buf, "sig", cfg->sig);
    wtk_http_pack_form_data(buf, "dev", cfg->dev);

    wtk_strbuf_push_string(buf, "data=cldrec");
}

int wtk_http_auth(wtk_http_auth_t *cfg)
{
    wtk_http_auth_cfg_t *auth_cfg;
    wtk_strbuf_t *body_buf;
    wtk_strbuf_t *buf;

    char response[1024] = { 0 };
    char *ch;
    int sfd;
    int rbytes, wbytes;
    int ret = -1;
    
    buf = wtk_strbuf_new(1024, 1.0);
    body_buf = wtk_strbuf_new(1024, 1.0);

    memset(buf->data, 0, buf->length);
    memset(body_buf->data, 0, body_buf->length);

    auth_cfg = (wtk_http_auth_cfg_t *)wtk_malloc(sizeof(*auth_cfg));
    wtk_http_auth_cfg_reset(auth_cfg);

    //auth_cfg->usrid = wtk_str_dup(cfg->usrid);
    auth_cfg->appid = wtk_str_dup(cfg->appid);
    auth_cfg->seckey = wtk_str_dup(cfg->seckey);
    wtk_http_auth_cfg_update(auth_cfg);

    wtk_http_pack_body(body_buf, auth_cfg);
    wtk_http_pack_header(buf, cfg->ip, cfg->port, body_buf->pos);
    wtk_strbuf_push_string(buf, body_buf->data);

    //wtk_http_print(buf->data, buf->pos);

    //connect again in case first fails
    ret = wtk_httpc_request_connect(cfg->ip, cfg->port, &sfd, 1000);
    if (ret != 0) {
        wtk_httpc_request_connect(cfg->ip, cfg->port, &sfd, 2000);
    }

    wtk_fd_send(sfd, buf->data, buf->pos, &wbytes);

    //{"ath": 0} 0:fail 1:success
    wtk_fd_recv(sfd, response, 1024, &rbytes);
    //printf("respose:\n%s\n", response);
    ch = strstr(response, "{\"ath\":");
    if (ch) {
        if (atoi(ch + 8) == 1) {
            ret = 0;
        } else {
            ret = -1;
        }
    } else {
        ret = -1;
    }

    close(sfd);
    wtk_strbuf_delete(buf);
    wtk_strbuf_delete(body_buf);
    wtk_free(auth_cfg);

    return ret;
}

