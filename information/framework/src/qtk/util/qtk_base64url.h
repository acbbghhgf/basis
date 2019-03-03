#ifndef QTK_UTIL_QTK_BASE64URL_H_
#define QTK_UTIL_QTK_BASE64URL_H_
#ifdef __cplusplus
extern "C" {
#endif

char *qtk_base64url_encode(const char *data, int dlen, char *result, int rlen);
char *qtk_base64url_decode(const char *data, char *result, int rlen, int *real_len);

#ifdef __cplusplus
}
#endif
#endif
