#ifndef CORE_THIN_WTK_STACK_H_
#define CORE_THIN_WTK_STACK_H_
#include "wtk/core/wtk_type.h"
#include <stdio.h>
#include "wtk_alloc.h"
#include "wtk_queue.h"
#include "wtk_os.h"
#include "wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct stack_block stack_block_t;
#define wtk_stack_push_s(s,msg) wtk_stack_push(s,msg,sizeof(msg)-1)

struct stack_block
{
	stack_block_t* next;
	char *pop;
	char *push;
	char *end;
};

struct wtk_stack
{
	wtk_queue_node_t	    q;
	uint32_t 		last_alloc_size;
	uint32_t		max_size;
	uint32_t		len;
	float			growf;
	stack_block_t	*pop;
	stack_block_t	*push;
	stack_block_t	*end;
};
typedef struct wtk_stack wtk_stack_t;

/**
 * @brief allocate stack.
 */
wtk_stack_t* wtk_stack_new(int init_size,int max_size,float growf);

int wtk_stack_bytes(wtk_stack_t *s);
/**
 * @brief init stack.
 */
int wtk_stack_init(wtk_stack_t *s,int init_size,int max_size,float growf);

/**
 * @brief dispose stack.
 */
int wtk_stack_delete(wtk_stack_t* s);

/**
 * @brief clean stack.
 */
int wtk_stack_clean(wtk_stack_t *s);

/**
 * @brief reset stack.
 */
int wtk_stack_reset(wtk_stack_t *s);

/**
 * @brief push data.
 */
int wtk_stack_push(wtk_stack_t* s,const char* data,int len);

void wtk_stack_push_f(wtk_stack_t *s,const char *fmt,...);
/**
 * @brief push string.
 */
int wtk_stack_push_string(wtk_stack_t* s,const char* str);

/**
 * @brief pop data
 * @return 0 if pop len bytes else failed;
 */
int wtk_stack_pop(wtk_stack_t* s,char* data,int len);

/**
 * @return copied bytes;
 */
int wtk_stack_pop2(wtk_stack_t* s,char* data,int len);

/**
 * @brief print.
 */
int wtk_stack_print(wtk_stack_t* s);

/**
 * @brief check stack is valid or not.
 */
int wtk_stack_is_valid(wtk_stack_t * s);

void wtk_stack_merge(wtk_stack_t *s,char* p);
void wtk_stack_add(wtk_stack_t *dst,wtk_stack_t *src);
void wtk_stack_copy(wtk_stack_t *s,wtk_strbuf_t *to);
int wtk_stack_write(wtk_stack_t *s,FILE *f);
int wtk_stack_write2(wtk_stack_t *s,char *fn);
int wtk_stack_read(wtk_stack_t *s,char *fn);
int wtk_stack_write_nbfd(wtk_stack_t *s, int fd);
#ifdef __cplusplus
};
#endif
#endif
