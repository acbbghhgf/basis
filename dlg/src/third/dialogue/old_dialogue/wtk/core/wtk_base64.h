#ifndef WTK_CORE_WTK_BASE64
#define WTK_CORE_WTK_BASE64

#include "wtk_type.h"

#ifdef __cplusplus
extern "C" {
#endif

char* wtk_base64_encode(char* data,int len);
char* wtk_base64_encode_url(char* data,int len);
char* wtk_base64_decode(const char* base64,int len);


#ifdef __cplusplus
};
#endif
#endif
