#ifndef  AUDIO_WAVEHDR_H_
#define AUDIO_WAVEHDR_H_
#include "wtk/core/wtk_type.h"
#include "wtk/core/wtk_strbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * fmt_compression_code:
 * 	0 Unknown
 * 	1 Microsoft ADPCM
 * 	6 ITU G.711 a-law
 * 	7 ITU G.711 Âµ-law
 * 	17 IMA ADPCM
 * 	20 ITU G.723 ADPCM (Yamaha)
 * 	49 GSM 6.10
 * 	64 ITU G.721 ADPCM
 * 	80 MPEG
 * 	65,536 Experimental
 */
typedef struct _Wave_Header
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

        char data_id[4];                                         // "data"
        int32_t data_datasize;                                       // data chunk size.
#pragma pack()
}WaveHeader;

//Set WaveHeader tags name.
void wavehdr_init(WaveHeader* header);

//Set WaveHeader params.
void wavehdr_set_fmt(WaveHeader* header,int channels,int sampleRate,int bytesPerSample);

//Set WaveHeader raw data size.
void wavehdr_set_size(WaveHeader* header,int rawDataSize);

void wavehdr_print(WaveHeader* header);

void wave_write_file(char *fn,int rate,char *data,int bytes);
void wave_write_file_float(char *fn,int rate,float *data,int len);
void wave_write_file_float2(char *fn,int rate,float *data,int len);
void wave_write_file_float3(char *fn,int channel,int rate,wtk_strbuf_t **bufs);
void wave_write_file_float4(char *fn,int channel,int rate,wtk_strbuf_t **bufs,wtk_strbuf_t **pad,int nx);

void wave_write_file2(char *fn,int channel,int rate,char *data,int bytes);
void wave_write_file22(char *fn,int channel,int rate,int bytes_per_sample,char *data,int bytes);
void wave_write_file3(char *fn,int channel,int rate,wtk_strbuf_t **bufs);
void wave_write_file32(char *fn,int channel,int rate,wtk_strbuf_t **bufs,int skip);
void wave_write_file4(char *fn,int channel,int rate,char **data,int len);
void wave_write_file5(char *fn,int channel,int rate,short **data,int len);



#ifdef __cplusplus
};
#endif
#endif
