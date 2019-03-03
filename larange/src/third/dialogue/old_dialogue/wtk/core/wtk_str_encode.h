#ifndef WTK_CORE_WTK_STR_ENCODE_H_
#define WTK_CORE_WTK_STR_ENCODE_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_heap.h"
#include "wtk/core/wtk_array.h"
#ifdef __cplusplus
extern "C" {
#endif
#define utf8_to_gbk_3_s(utf8) utf8_to_gbk_3(utf8,sizeof(utf8)-1)
/**
 * @return chars must be freed.
 */
char* utf8_to_gbk(const char* utf);

/**
 * @return chars must be freed.
 */
char* gbk_to_utf8(const char* gbk);
#ifdef WIN32
#else
/**
 * @return chars must be freed.
 */
char* utf8_to_gbk_2(const char* utf,int len);

/**
 * @return chars must be freed.
 */
char* gbk_to_utf8_2(const char* gbk,int len);

/**
 * @return chars is in static buffer, do not need to be freed, is used for debug.
 */
char* gbk_to_utf8_3(const char* gbk,int len);

char* utf8_to_gbk_3(const char *utf8,int len);
#endif
int str_is_utf8(const unsigned char* utf,int len);
int wtk_utf8_bytes(char c);
int wtk_utf8_len(char *data,int len);
void wtk_utf8_tolower(char *data,int len);
int wtk_utf16_to_utf8(int v,char *buf);
int wtk_utf8_to_utf16(char *data);
#ifdef __cplusplus
};
#endif
#endif
