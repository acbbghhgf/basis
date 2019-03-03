#ifndef WTK_OS_WTK_SEMSHM_H_
#define WTK_OS_WTK_SEMSHM_H_
#include "wtk/core/wtk_type.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_semshm wtk_semshm_t;

typedef void(*wtk_semshm_read_f)(void *hook,const char *data,int bytes);

typedef struct
{
	sem_t readable_sem;			//shared allocated in addr;
	sem_t writeable_sem;			//shared allocated in addr;
	int user_data_len;				//bytes of valid user data;
}wtk_semshm_shared_t;

/*
 * @brief blocked for read and write; used for one process write and another read;
 */
struct wtk_semshm
{
	int shm_id;
	void *addr;						//mapped user data;
	wtk_semshm_shared_t *shared;	//shared data for process;
	char *user_data;				//pointer the valid user data;
	int user_data_bytes;			//bytes of allocated user data;
};

wtk_semshm_t* wtk_semshm_new(int shm_bytes);

int wtk_semshm_delete(wtk_semshm_t *s);

/*
 * @brief will read all shared data;
 */
int wtk_semshm_read(wtk_semshm_t *s,void *hook,wtk_semshm_read_f read);

/**
 * @brief readable will not;
 */
int wtk_semshm_readable(wtk_semshm_t *s);

/**
 * @brief write data to semshm, blocked for all data is write to shared memory;
 */
int wtk_semshm_write(wtk_semshm_t *s,const char *data,int bytes);

/**
 * @brief nattach porcess of current shared memory;
 */
int wtk_semshm_nattach(wtk_semshm_t *s);

/**
 * @brief notify semshm read, used for wake up read blocked thread.
 */
void wtk_semshm_wake_read(wtk_semshm_t *s);

/**
 * @brief notify semshm write, used for wake up write blocked thread;
 */
void wtk_semshm_wake_write(wtk_semshm_t *s);

/**
 * @brief notify semshm read and write;
 */
void wtk_semshm_wake_rw(wtk_semshm_t *s);
#ifdef __cplusplus
};
#endif
#endif
