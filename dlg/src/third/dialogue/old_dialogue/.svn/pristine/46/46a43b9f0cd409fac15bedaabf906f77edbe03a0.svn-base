#ifndef WTK_EVAL_ERRNO_WTK_ERRNO_H_
#define WTK_EVAL_ERRNO_WTK_ERRNO_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_errno wtk_errno_t;
#define wtk_errno_set_no(e,n) ((e)->no=no)
#define wtk_errno_set_s(e,s) wtk_errno_set((e),s,sizeof(s)-1)

/*
 *  Error ID
 *	core:  0 -39999
 *	rtmp: 40000 - 49999
 *	falsh: 50000 - 59999
 *	server: 60000 - 69999
 */

#define WTK_EVAL_NOT_INIT 1
#define WTK_EVAL_POST_FAILED 2
#define WTK_EVAL_REF_INVALID 5
#define WTK_EVAL_EBNF_INVALID 6

typedef enum wtk_info_id
{
	WTK_INFO_EMPTY_WAV=10000,
	WTK_INFO_FA_FORCEOUT=10001,
	WTK_INFO_REC_FORCEOUT=10002,
	WTK_INFO_REC_FAILED=10003,
	WTK_INFO_VOLUME_LOW=10004,
	WTK_INFO_CLIP=10005,
	WTK_INFO_SNR=10006,
	WTK_INFO_NOT_ENGLISH=10007
}wtk_info_id_t;


struct wtk_errno
{
	int no;
	wtk_strbuf_t *buf;
};

wtk_errno_t* wtk_errno_new();
int wtk_errno_delete(wtk_errno_t *e);
void wtk_errno_reset(wtk_errno_t *e);
void wtk_errno_set(wtk_errno_t *e,int no,char *msg,int msg_bytes);
/**
 * parameter must end with NULL like wtk_errno_set_err(e,-1,"foo",NULL);
 */
void wtk_errno_set_string(wtk_errno_t *e,int no,...);
void wtk_errno_print(wtk_errno_t *e);
#ifdef __cplusplus
};
#endif
#endif
