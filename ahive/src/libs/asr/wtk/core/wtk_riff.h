#ifndef WTK_CORE_wtk_riff
#define WTK_CORE_wtk_riff
#include "wtk/core/wtk_type.h" 
#include "wtk/core/wtk_strbuf.h"
#include "wtk/core/wavehdr.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_riff wtk_riff_t;

typedef struct
{
#pragma pack(1)
	char id[4];       //"RIFF"
	int32_t datasize; // RIFF chunk data size (file size)-8
	char type[4];	  //"WAVE"
#pragma pack()
}wtk_riff_hdr_t;

typedef struct
{
#pragma pack(1)
	char id[4];		// "fmt "
	int32_t data_size;	//16 + extra format bytes
    int16_t compression_code;                   // 1 for PCM
    int16_t channels;                                   // 1 or 2
    int32_t sample_rate;                                  // samples per second
    int32_t avg_bytes_per_sec;                       // sample_rate*block_align
    int16_t block_align;                               // number bytes per sample bit_per_sample*channels/8
    int16_t bit_per_sample;                         // bits of each sample.
#pragma pack()
}wtk_riff_fmt_t;

typedef struct
{
#pragma pack(1)
	char id[4];		//"data"
	int32_t data_size;
#pragma pack()
}wtk_riff_wavhdr_t;


struct wtk_riff
{
	FILE *file;
	wtk_riff_hdr_t riff_hdr;
	wtk_riff_fmt_t fmt;
	wtk_riff_wavhdr_t wavhdr;
	int data_pos;
};

wtk_riff_t* wtk_riff_new();
void wtk_riff_delete(wtk_riff_t *f);
int wtk_riff_open(wtk_riff_t *f,char *fn);
void wtk_riff_rewind(wtk_riff_t *f);
int wtk_riff_read(wtk_riff_t *f,char *buf,int size);
int wtk_riff_close(wtk_riff_t *f);
void wtk_riff_print(wtk_riff_t *f);
int wtk_riff_cpy(char *fn,int s,int e,char *dst);

wtk_strbuf_t** wtk_riff_read_channel(char *fn,int *n);

int wtk_riff_get_channel(char *fn);
int wtk_riff_get_samples(char *fn);
#ifdef __cplusplus
};
#endif
#endif
