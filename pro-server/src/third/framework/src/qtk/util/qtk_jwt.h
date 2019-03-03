#ifndef QTK_UTIL_QTK_JWT_H_
#define QTK_UTIL_QTK_JWT_H_
#ifdef __cplusplus
extern "C" {
#endif

// Note: this func will modify string
char *qtk_jwt_verify(char *string, char *payload, int len, int *passed);

#ifdef __cplusplus
}
#endif
#endif
