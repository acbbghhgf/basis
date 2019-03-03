#ifndef QTK_UTIL_QTK_RSA_H
#define QTK_UTIL_QTK_RSA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/engine.h>


char* qtk_base64_encode( const unsigned char * bindata, char * base64, int binlength );
char* qtk_base64_encode2(const unsigned char *bindata, char *base64, int binlength, int plus_cnv);
int qtk_base64_decode( const char * base64, unsigned char * bindata );
RSA* qtk_rsa_new(char *pubkey, const char *passwd);
unsigned char *qtk_rsa_encrypt(char *str, int *enc_len, RSA *rsa);
int qtk_rsa_verify(const char *str,const unsigned char *sign,int sign_len, RSA *p_rsa);
void qtk_rsa_free(RSA* rsa);

#ifdef __cplusplus
}
#endif

#endif
