#include "wtk_alawhdr.h"
#include <stdio.h>

void wtk_alawhdr_init(wtk_alawhdr_t *hdr)
{
	memcpy(hdr->riff_id, "RIFF", 4);
	memcpy(hdr->riff_type, "WAVE", 4);
	memcpy(hdr->fmt_id, "fmt ", 4);
	hdr->fmt_datasize = 16;
	hdr->fmt_compression_code = 6;
	memcpy(hdr->fact_id,"fact",4);
	hdr->fact_datasize=4;
	memcpy(hdr->data_id, "data", 4);
}

void wtk_alawhdr_set_fmt(wtk_alawhdr_t* header, int channels, int sampleRate,
		int bytesPerSample)
{
	header->fmt_channels = channels;
	header->fmt_sample_rate = sampleRate;
	header->fmt_block_align = channels * bytesPerSample;
	header->fmt_avg_bytes_per_sec = header->fmt_block_align * sampleRate;
	header->fmt_bit_per_sample = bytesPerSample << 3;
}

void wtk_alawhdr_set_size(wtk_alawhdr_t* header, int rawDataSize)
{
	header->riff_datasize = rawDataSize + sizeof(wtk_alawhdr_t) - 8;
	header->data_datasize = rawDataSize;
	header->fact_dependent= rawDataSize;
}

void wtk_alaw_write(wtk_strbuf_t *buf,char *fn)
{
	wtk_alawhdr_t hdr;
	FILE *f;

	f=fopen(fn,"wb");
	wtk_alawhdr_init(&hdr);
	wtk_alawhdr_set_fmt(&hdr,1,8000,1);
	wtk_alawhdr_set_size(&hdr,buf->pos);
	fwrite(&hdr,sizeof(hdr),1,f);
	wtk_strbuf_push_front(buf,(char*)&hdr,sizeof(hdr));
	fwrite(buf->data,buf->pos,1,f);
	fclose(f);
}
