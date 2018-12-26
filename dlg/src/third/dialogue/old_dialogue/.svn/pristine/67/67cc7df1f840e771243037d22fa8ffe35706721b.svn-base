#ifndef WTK_CORE_GZ_WTK_GZIP
#define WTK_CORE_GZ_WTK_GZIP
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#include "miniz.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_gzip wtk_gzip_t;

typedef struct {
	int l;
	uint32_t crc32;
}wtk_gzip_st_t;

typedef enum
{
	WTK_GZIP_UNZIP_INIT,
	WTK_GZIP_UNZIP_FLAG_EXTRA,
	WTK_GZIP_UNZIP_FLAG_EXTRA_SKIP,
	WTK_GZIP_UNZIP_FLAG_FNAME,
	WTK_GZIP_UNZIP_FLAG_FCOMMENT,
	WTK_GZIP_UNZIP_FLAG_FCRC,
	WTK_GZIP_UNZIP_RUN,
}wtk_gzip_unzip_state_t;

struct wtk_gzip
{
	wtk_strbuf_t *buf;
	wtk_gzip_st_t st;
	z_stream stream;
	char *cache;
	int cache_size;
	uint8_t hdr_flags;
	int next_bytes;
	wtk_gzip_unzip_state_t state;
};

wtk_gzip_t* wtk_gzip_new();
void wtk_gzip_delete(wtk_gzip_t *zip);
void wtk_gzip_zip_start(wtk_gzip_t *zip);
void wtk_gzip_zip_write(wtk_gzip_t *zip,char *data,int bytes,int is_end);

void wtk_gzip_unzip_start(wtk_gzip_t *zip);
int wtk_gzip_unzip_read(wtk_gzip_t *zip,char *data,int bytes,int is_end);
#ifdef __cplusplus
};
#endif
#endif
