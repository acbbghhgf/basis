/*
###########################################################
#                                                         #
#      W       W       W    wwwwwwwwwww    W   W          #
#       W     W W     W          W         W  W           #
#        W   W   W   W           W         W W            #
#         W W     W W            W         W   W          #
#          W       W             W         W     W        #
#                                                         #
#---------------------------------------------------------#
#  If you found some logic problem in these source, or you#
#have some good idea, please contact us. If you want to be#
#one of us, you can mail some source to us, Have fun.     #
#  email: li.zhang@qdreamer.com                           #
###########################################################
*/
#ifndef WTK_CORE_WTK_TYPE_H_
#define WTK_CORE_WTK_TYPE_H_
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//#include <sys/timeb.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#else
#ifndef USE_FOR_WVITE
//xform defined in windows.h, but hmmset use this typedef , so make some macro to disable it. 
#include <windows.h>
#endif
#endif
#include <stdlib.h>
#include "wtk_alloc.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#define INVALID_FD -1
#define RAW_DATA 1
#define STRING_DATA 0

#ifdef __ANDROID__
#include <android/log.h>
//#define wtk_debug(...) __android_log_print(ANDROID_LOG_DEBUG,"wtk",__VA_ARGS__);
#define wtk_debug(...) printf("%s:%d:",__FUNCTION__,__LINE__);printf(__VA_ARGS__);fflush(stdout);
#else
#ifdef xunix
#include <sys/types.h>
#include <unistd.h>
#define wtk_debug(...) printf("%d:%s:%d:",(int)getpid(),__FUNCTION__,__LINE__);printf(__VA_ARGS__);fflush(stdout);
#else
#define wtk_debug(...) printf("%s:%d:",__FUNCTION__,__LINE__);printf(__VA_ARGS__);fflush(stdout);
#endif
#endif

#define STR1(x) #x
#define STR(X) STR1(X)
#define cat(x,y) x##y
#define CAT(X,Y) cat(X,Y)

#ifndef __cplusplus
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#endif

//#define wtk_float_round(f) (int)((f)+0.5)
#define wtk_float_round(f) (int)((f)>0?(f+0.5):(f-0.5))

#define wtk_trace_i(s,item) wtk_debug("%s=%d\n",STR(item),s->item)
#define wtk_debug_item(p,f,i) printf("%s=",STR(i));printf(f,p->i);printf("\n");

typedef void* Pointer;
#ifdef WIN32
#if __STDC_VERSION__ < 199901L
typedef __int8 int8_t;
typedef __int32 int32_t;
typedef __int16 int16_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
#else
#include <stdint.h>
#endif
#else
#include <stdint.h>
#endif
#include <stdio.h>
#define LOG printf
typedef void* (*wtk_new_handler_t)(void* user_data);
typedef int (*wtk_delete_handler_t)(void* data);
typedef int (*wtk_delete_handler2_t)(void *user_data,void* data);
typedef int (*wtk_cmp_handler_t)(void *d1,void *d2);
typedef int (*wtk_walk_handler_t)(void *user_data,void* data);
typedef void (*wtk_print_handler_t)(void *data);
typedef void (*wtk_write_f)(void *inst,const char *data,int bytes);
#define data_byof(q,off)  ( (q) ? ((void*)((char*)q-off)) : NULL)
#define data_offset(q,type,link) (type*)((q) ? (void*)((char*)q-offsetof(type,link)) : NULL)
#define data_offset2(q,type,link) (type*)((void*)((char*)q-offsetof(type,link)))

typedef struct
{
	float min;
	float max;
}wtk_range_t;

#ifdef __cplusplus
};
#endif
#endif
