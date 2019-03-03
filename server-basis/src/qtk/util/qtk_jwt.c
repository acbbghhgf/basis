#include "qtk_jwt.h"
#include "qtk_base64url.h"
#include "wtk/core/wtk_type.h"
#include "third/json/cJSON.h"
#include "qtk/util/qtk_rsa.h"

#include <stdlib.h>
#include <string.h>

static char pub_key[] = "-----BEGIN PUBLIC KEY-----\n\
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE1mXkkghj8q5XMMvAFUXYCNbPaeWQ\n\
NCyG2dt6pEqz/Ig1jm8lfEQln6mpI2xVrqhrKbQr+9LakKvf93ukBc84MQ==\n\
-----END PUBLIC KEY-----\n\
";

static char rs_pub_key[] = "-----BEGIN RSA PUBLIC KEY-----\n\
MIGJAoGBALduScYh/dyHQm5uwZd0h/Wn9iqp7iczKehh2y93oWY8GHVdoFKxFddm\n\
reMrJKZmPzXAN9UfxLbOZagDfa0yZAycUBy6l5v/Avm2b+S0zIm+14T2MGg5MFTv\n\
A8DuIGO/lYyJOgD/vze03YGo3bXXVD+mmZuuXn8LOdlhNnntXV8pAgMBAAE=\n\
-----END RSA PUBLIC KEY-----\n\
";
static const char phrase[] = "qdreamer";

typedef int (*_jwt_verify_sign_func) (char *hdr, char *pld, const char *sign);

static void _jwt_sep(char *string, char **hdr, char **payload, char **sign)
{
    int len = strlen(string);
    int i;
    *hdr = string;
    int cnt = 0;
    for(i=0; i<len; i++) {
        if (string[i] == '.' ) {
            string[i] = '\0';
            if (0 == cnt) {
                *payload = string + i + 1;
            } else if (1 == cnt) {
                *sign = string + i + 1;
            } else {
                return;
            }
            cnt ++;
        }
    }
}

static int _sha256(const char *d, int dlen, char *result, int rlen)
{
    if (rlen < 33) {
        return -1;
    }
    EVP_MD_CTX c;
    EVP_MD_CTX_init(&c);
    EVP_Digest(d, dlen, (unsigned char*)result, NULL, EVP_sha256(), NULL);
    EVP_MD_CTX_cleanup(&c);
    return 0;
}

static int esprime256v1_verify(const unsigned char *sig, int siglen, const char *buf, int buflen)
{
    BIO *pkey = BIO_new_mem_buf(pub_key, -1);
    EC_KEY * ec_key = PEM_read_bio_EC_PUBKEY(pkey, NULL, NULL, NULL);
    BIO_free_all(pkey);
    int pass = ECDSA_verify(0, (const unsigned char *)buf, buflen, sig, siglen, ec_key);
    EC_KEY_free(ec_key);
    return pass;
}

static int _jwt_verify_es256_impl(char *hdr_part, char *payload_part, const char *sign_part)
{
    char sign[2048];
    int sign_len;
    if (NULL == qtk_base64url_decode(sign_part, sign, sizeof(sign), &sign_len)) {
        wtk_debug("Payload Decode Fail\n");
        return 0;
    }
    char buf[33];
    *(payload_part - 1) = '.';
    _sha256(hdr_part, strlen(hdr_part), buf, sizeof(buf));
    *(payload_part - 1) = '\0';
    return esprime256v1_verify((const unsigned char *)sign, sign_len, buf, 32);
}

static int _jwt_verify_rs256_impl(char *hdr_part, char *payload_part, const char *sign_part)
{
    char sign[2048];
    int sign_len;
    if (NULL == qtk_base64url_decode(sign_part, sign, sizeof(sign), &sign_len)) {
        wtk_debug("Payload Decode Fail\n");
        return 0;
    }
    *(payload_part - 1) = '.';
    char buf[33];
    _sha256(hdr_part, strlen(hdr_part), buf, sizeof(buf));
    *(payload_part - 1) = '\0';
    RSA *rsa = qtk_rsa_new(rs_pub_key, phrase);
    int pass = RSA_verify(NID_sha256, (unsigned char *)buf, 32, (unsigned char *)sign, sign_len, rsa);
    qtk_rsa_free(rsa);
    return pass;
}

static _jwt_verify_sign_func _jwt_choose_sign_func(const char *hdr_part)
{
    char hdr[256];
    _jwt_verify_sign_func func = NULL;

    int hlen;
    if (NULL == qtk_base64url_decode(hdr_part, hdr, sizeof(hdr), &hlen)) {
        wtk_debug("Header Decode Fail\n");
        return NULL;
    }
    cJSON *hdr_json = cJSON_Parse(hdr);
    if (NULL == hdr_json) {
        goto end;
    }
    cJSON *item = cJSON_GetObjectItem(hdr_json, "alg");
    if (NULL == item || cJSON_String != item->type || NULL == item->valuestring) {
        goto end;
    }
    if (0 == strcmp(item->valuestring, "ES256")) {
        func = _jwt_verify_es256_impl;
    } else if (0 == strcmp(item->valuestring, "RS256")) {
        func = _jwt_verify_rs256_impl;
    }
end:
    if (hdr_json) {
        cJSON_Delete(hdr_json);
    }
    return func;
}

static time_t _jwt_get_payload_time(cJSON *item)
{
    if (item == NULL) {
        return -1;
    }
    switch (item->type) {
    case cJSON_Number:
        return item->valueint;
    case cJSON_String:
        return strtoul(item->valuestring, NULL, 10);
    }
    return -1;
}

static int _jwt_verify_payload(const char *payload)
{
    int pass = 0;
    cJSON *json = cJSON_Parse(payload);
    if (NULL == json) {
        goto end;
    }
    time_t now = time(NULL);
    time_t iat = _jwt_get_payload_time(cJSON_GetObjectItem(json, "iat"));
    if (iat > 0 && iat > now) {
        goto end;
    }
    time_t exp = _jwt_get_payload_time(cJSON_GetObjectItem(json, "exp"));
    if (exp > 0 && exp < now) {
        goto end;
    }
    pass = 1;
end:
    if (json) {
        cJSON_Delete(json);
    }
    return pass;
}

char *qtk_jwt_verify(char *string, char *pld, int plen, int *passed)
{
    *passed = 0;
    char *hdr_part = NULL, *payload_part = NULL, *sign_part = NULL;
    _jwt_sep(string, &hdr_part, &payload_part, &sign_part);
    if (NULL == hdr_part || NULL == payload_part || NULL == sign_part) {
        wtk_debug("Malformed Jwt Token\n");
        return NULL;
    }
    if (NULL == qtk_base64url_decode(payload_part, pld, plen, NULL)) {
        wtk_debug("Payload Decode Fail\n");
        return NULL;
    }
    if (0 == _jwt_verify_payload(pld)) {
        return NULL;
    }
    _jwt_verify_sign_func func = _jwt_choose_sign_func(hdr_part);
    if (NULL == func) {
        return NULL;
    }
    if (0 == func(hdr_part, payload_part, sign_part)) {
        return NULL;
    }
    *passed = 1;
    return pld;
}
