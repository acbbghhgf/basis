#ifndef WTK_CORE_WTK_ALAWHDR_H_
#define WTK_CORE_WTK_ALAWHDR_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct wtk_alawhdr wtk_alawhdr_t;
struct wtk_alawhdr
{
#pragma pack(1)
	char riff_id[4];                                           //"RIFF"
	int32_t riff_datasize;                                         // RIFF chunk data size

	char riff_type[4];                                       // "WAVE"
	char fmt_id[4];                                          // "fmt "
	int32_t fmt_datasize;                                        // fmt chunk data size
	int16_t fmt_compression_code;                   // 1 for PCM
	int16_t fmt_channels;                                   // 1 or 2
	int32_t fmt_sample_rate;                                  // samples per second
	int32_t fmt_avg_bytes_per_sec;                       // sample_rate*block_align
	int16_t fmt_block_align;                               // number bytes per sample bit_per_sample*channels/8
	int16_t fmt_bit_per_sample;                         // bits of each sample.

	char fact_id[4];                                         // "fact"
	int32_t fact_datasize;                                       // data chunk size.
	int32_t fact_dependent;

	char data_id[4];                                         // "data"
	int32_t data_datasize;                                       // data chunk size.
#pragma pack()
};

void wtk_alawhdr_init(wtk_alawhdr_t *hdr);
void wtk_alawhdr_set_fmt(wtk_alawhdr_t* header, int channels, int sampleRate,
		int bytesPerSample);
void wtk_alawhdr_set_size(wtk_alawhdr_t* header, int rawDataSize);
void wtk_alaw_write(wtk_strbuf_t *buf,char *fn);
#ifdef __cplusplus
};
#endif
#endif
