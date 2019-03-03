#include "third/lua5.3.4/src/lua.h"
#include "third/lua5.3.4/src/lauxlib.h"

#include<openssl/rsa.h>
#include<openssl/pem.h>
#include<openssl/err.h>
#include <openssl/engine.h>

#include <string.h>

#define OPENSSLKEY "private.pem"
#define PUBLICKEY "public.pem"
#define BUFFSIZE 1024

static char pub_key[] = "-----BEGIN RSA PUBLIC KEY-----\n\
MIGJAoGBALduScYh/dyHQm5uwZd0h/Wn9iqp7iczKehh2y93oWY8GHVdoFKxFddm\n\
reMrJKZmPzXAN9UfxLbOZagDfa0yZAycUBy6l5v/Avm2b+S0zIm+14T2MGg5MFTv\n\
A8DuIGO/lYyJOgD/vze03YGo3bXXVD+mmZuuXn8LOdlhNnntXV8pAgMBAAE=\n\
-----END RSA PUBLIC KEY-----\n\
";
static const char phrase[] = "qdreamer";
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define CONVERT_PLUS(base, idx, c)                \
    if (c == '+') { \
    base[idx++] = '%'; \
    base[idx++] = '2'; \
    base[idx++] = 'b'; \
    } else { \
    base[idx++] = c; \
    }

char * base64_encode( const unsigned char * bindata, char * base64, int binlength )
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;

        CONVERT_PLUS(base64, j, base64char[(int)current]);

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            CONVERT_PLUS(base64, j, base64char[(int)current]);
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        CONVERT_PLUS(base64, j, base64char[(int)current]);

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        CONVERT_PLUS(base64, j, base64char[(int)current]);

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        CONVERT_PLUS(base64, j, base64char[(int)current]);
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
            ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
            ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
            ((unsigned char)(temp[3]&0x3F));
    }
    bindata[j] = 0;
    return j;
}

RSA* rsa_new(char *pubkey, const char *passwd) {
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

char *my_sha1_hex(const char *str) {
    static const char *ptab = "0123456789abcdef";
    char buf[50];
    char *p_hex;
    int i,j;
    unsigned int sha_len;
    EVP_MD_CTX c;
    EVP_MD_CTX_init(&c);
    EVP_Digest(str, strlen(str), (unsigned char *)buf, &sha_len, EVP_sha1(), NULL);
    p_hex = malloc(sha_len * 2 + 1);
    for (i = 0, j = 0; i < sha_len; i++, j+=2) {
        p_hex[j] = ptab[(buf[i]>>4) & 0x0f];
        p_hex[j+1] = ptab[buf[i] & 0x0f];
    }
    p_hex[j] = 0;
    EVP_MD_CTX_cleanup(&c);
    return p_hex;
}

unsigned char *rsa_encrypt(const char *str,char *path_key, int *enc_len, RSA *p_rsa) {
    unsigned char *p_en;
    int flen,rsa_len;
    char *sha1;
    sha1 = my_sha1_hex(str);
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

unsigned char *my_sha1(const char *str) {
    unsigned char *buf;
    unsigned int sha_len;
    EVP_MD_CTX c;
    EVP_MD_CTX_init(&c);
    buf = malloc(21);
    EVP_Digest(str, strlen(str), buf, &sha_len, EVP_sha1(), NULL);
    EVP_MD_CTX_cleanup(&c);
    return buf;
}

int rsa_verify(const char *str,const char *sign,int sign_len, char *path_key, RSA *p_rsa) {
    unsigned char *sha1 = NULL;
    int ret = 0;

    sha1 = my_sha1(str);
    ret = RSA_verify(NID_sha1, sha1, 20, (const unsigned char*)sign,sign_len,p_rsa);
    if (ret == 0) {
        ERR_print_errors_fp(stdout);
    }
    if (sha1) {
        free(sha1);
    }
    return ret;
}

static int lencode(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    RSA *p_rsa = rsa_new(pub_key, phrase);
    if (NULL == p_rsa) return 0;
    int enc_len;
    unsigned char *str_en = rsa_encrypt(str, PUBLICKEY, &enc_len, p_rsa);
    char encbuf[1024];
    base64_encode(str_en, encbuf, enc_len);
    lua_pushstring(L, encbuf);
    free(str_en);
    RSA_free(p_rsa);
    return 1;
}

static int lverify(lua_State *L)
{
    const char *str = luaL_checkstring(L, 1);
    const char *sign = luaL_checkstring(L, 2);
    unsigned char buf[1024];
    int n = base64_decode(sign, buf);
    RSA *p_rsa = rsa_new(pub_key, phrase);
    int ret = rsa_verify(str, (const char *)buf, n, PUBLICKEY, p_rsa);
    lua_pushboolean(L, ret);
    RSA_free(p_rsa);
    return 1;
}

static int lqauth_exit(lua_State *L)
{
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    return 0;
}

LUAMOD_API int luaopen_qauth(lua_State *L)
{
	luaL_checkversion(L);
    luaL_Reg l[] = {
        {"encode", lencode},
        {"verify", lverify},
        {NULL, NULL},
    };
    OpenSSL_add_all_ciphers();
    luaL_newlib(L, l);
    lua_newtable(L);
    lua_pushcfunction(L, lqauth_exit);
    lua_setfield(L, -2, "__gc");
    lua_setmetatable(L, -2);
    return 1;
}
