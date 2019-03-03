#include "qtk_rsa.h"
#include <string.h>


static int global_init = 0;
static const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


char* qtk_base64_encode(const unsigned char *bindata, char *base64, int binlength) {
    return qtk_base64_encode2(bindata, base64, binlength, 0);
}


char* qtk_base64_encode2(const unsigned char *bindata, char *base64, int binlength, int plus_cnv) {
    int i, j;
    unsigned char current;
    char c;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;

        c = base64char[(int)current];
        if (plus_cnv && c == '+') {
            base64[j++] = '%';
            base64[j++] = '2';
            base64[j++] = 'b';
        } else {
            base64[j++] = c;
        }

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            c = base64char[(int)current];
            if (plus_cnv && c == '+') {
                base64[j++] = '%';
                base64[j++] = '2';
                base64[j++] = 'b';
            } else {
                base64[j++] = c;
            }
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        c = base64char[(int)current];
        if (plus_cnv && c == '+') {
            base64[j++] = '%';
            base64[j++] = '2';
            base64[j++] = 'b';
        } else {
            base64[j++] = c;
        }

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            c = base64char[(int)current];
            if (plus_cnv && c == '+') {
                base64[j++] = '%';
                base64[j++] = '2';
                base64[j++] = 'b';
            } else {
                base64[j++] = c;
            }
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        c = base64char[(int)current];
        if (plus_cnv && c == '+') {
            base64[j++] = '%';
            base64[j++] = '2';
            base64[j++] = 'b';
        } else {
            base64[j++] = c;
        }

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        c = base64char[(int)current];
        if (plus_cnv && c == '+') {
            base64[j++] = '%';
            base64[j++] = '2';
            base64[j++] = 'b';
        } else {
            base64[j++] = c;
        }
    }
    base64[j] = '\0';
    return base64;
}


int qtk_base64_decode( const char * base64, unsigned char * bindata ) {
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 ) {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ ) {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ ) {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ ) {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ ) {
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


static char *my_sha1(const char *str) {
    char *buf;
    unsigned int sha_len;
    EVP_MD_CTX c;
    EVP_MD_CTX_init(&c);
    buf = malloc(21);
    EVP_Digest(str, strlen(str), (unsigned char*)buf, &sha_len, EVP_sha1(), NULL);
    EVP_MD_CTX_cleanup(&c);
    return buf;
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


RSA* qtk_rsa_new(char *pubkey, const char *passwd) {
    RSA *p_rsa = NULL;

    if (0  == global_init) {
        OpenSSL_add_all_ciphers();
        global_init = 1;
    }
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


unsigned char *qtk_rsa_encrypt(char *str, int *enc_len, RSA *p_rsa) {
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


int qtk_rsa_verify(const char *str,const unsigned char *sign,int sign_len, RSA *p_rsa) {
    char *sha1 = NULL;
    int ret = 0;

    sha1 = my_sha1(str);
    ret = RSA_verify(NID_sha1, (unsigned char*)sha1, 20,
                     sign, sign_len, p_rsa);
    if (ret == 0) {
        ERR_print_errors_fp(stdout);
    }
    if (sha1) {
        free(sha1);
    }
    return ret;
}



void qtk_rsa_free(RSA *rsa) {
    RSA_free(rsa);
}
