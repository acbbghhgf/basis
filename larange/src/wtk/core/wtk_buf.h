#ifndef WTK_CORE_WTK_BUF
#define WTK_CORE_WTK_BUF
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_buf wtk_buf_t;

typedef struct wtk_buf_item wtk_buf_item_t;

struct wtk_buf_item
{
	wtk_buf_item_t *next;
	char *data;
	unsigned short pos;
	unsigned short len;
};


struct wtk_buf
{
	int buf_size;
	wtk_buf_item_t *front;
	wtk_buf_item_t *end;
};

wtk_buf_t* wtk_buf_new(int buf_size);
void wtk_buf_delete(wtk_buf_t *buf);
void wtk_buf_reset(wtk_buf_t *buf);
void wtk_buf_push(wtk_buf_t *buf,char *data,int len);
void wtk_buf_pop(wtk_buf_t *buf,char *data,int len);
void wtk_buf_item_delete(wtk_buf_item_t *item);
int wtk_buf_len(wtk_buf_t *buf);
void wtk_buf_print(wtk_buf_t *buf);

typedef struct
{
	wtk_buf_item_t *item;
	int pos;
	int buf_size;
}wtk_buf_it_t;

wtk_buf_it_t wtk_buf_get_it(wtk_buf_t *buf);
char* wtk_buf_it_next(wtk_buf_it_t *it);
#ifdef __cplusplus
};
#endif
#endif
