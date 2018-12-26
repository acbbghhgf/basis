/**
 *  $Id: md5.h,v 1.2 2006/03/03 15:04:49 tomas Exp $
 *  Cryptographic module for Lua.
 *  @author  Roberto Ierusalimschy
 */


#ifndef QTK_LUAC_QTK_MD5_H
#define QTK_LUAC_QTK_MD5_H

#include "lua.h"

#define WORD 32
#define MASK 0xFFFFFFFF
#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
typedef uint32_t WORD32;
#else
typedef unsigned int WORD32;
#endif


typedef struct qtk_md5 qtk_md5_t;

struct qtk_md5 {
    char buf[64];
    int len;
    int total_len;
    WORD32 d[4];
};


#define HASHSIZE       16

void md5 (const char *message, long len, char *output);
int luaopen_md5_core (lua_State *L);

void qtk_md5_init(qtk_md5_t *m, const char *msg, long len);
void qtk_md5_update(qtk_md5_t *m, const char *msg, long len);
void qtk_md5_digest(qtk_md5_t *m, char *output);
int qtk_md5_delete(qtk_md5_t *m);


#endif
