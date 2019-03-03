#include "qtk_ogg_dec.h"
#include <stdlib.h>
#include <string.h>

#define MAX_FRAME_SIZE 2000
#define readint(buf, base) (((buf[base+3]<<24)&0xff000000)| \
                           ((buf[base+2]<<16)&0xff0000)| \
                           ((buf[base+1]<<8)&0xff00)| \
  	           	    (buf[base]&0xff))

static void *process_header(ogg_packet *op, spx_int32_t enh_enabled,
		spx_int32_t *frame_size, int *granule_frame_size, spx_int32_t *rate,
		int *nframes, int forceMode, int *channels, SpeexStereoState *stereo,
		int *extra_headers)
{
	void *st;
	const SpeexMode *mode;
	SpeexHeader *header;
	int modeID;
	SpeexCallback callback;

	header = speex_packet_to_header((char*) op->packet, op->bytes);
	if (!header)
	{
		fprintf(stderr, "Cannot read header\n");
		return NULL;
	}
	if (header->mode >= SPEEX_NB_MODES || header->mode < 0)
	{
		fprintf(
				stderr,
				"Mode number %d does not (yet/any longer) exist in this version\n",
				header->mode);
		free(header);
		return NULL;
	}

	modeID = header->mode;
	if (forceMode != -1)
		modeID = forceMode;

	mode = speex_lib_get_mode (modeID);

	if (header->speex_version_id > 1)
	{
		fprintf(
				stderr,
				"This file was encoded with Speex bit-stream version %d, which I don't know how to decode\n",
				header->speex_version_id);
		free(header);
		return NULL;
	}

	if (mode->bitstream_version < header->mode_bitstream_version)
	{
		fprintf(
				stderr,
				"The file was encoded with a newer version of Speex. You need to upgrade in order to play it.\n");
		free(header);
		return NULL;
	}
	if (mode->bitstream_version > header->mode_bitstream_version)
	{
		fprintf(
				stderr,
				"The file was encoded with an older version of Speex. You would need to downgrade the version in order to play it.\n");
		free(header);
		return NULL;
	}

	st = speex_decoder_init(mode);
	if (!st)
	{
		fprintf(stderr, "Decoder initialization failed.\n");
		free(header);
		return NULL;
	}
	speex_decoder_ctl(st, SPEEX_SET_ENH, &enh_enabled);
	speex_decoder_ctl(st, SPEEX_GET_FRAME_SIZE, frame_size);
	*granule_frame_size = *frame_size;

	if (!*rate)
		*rate = header->rate;
	/* Adjust rate if --force-* options are used */
	if (forceMode != -1)
	{
		if (header->mode < forceMode)
		{
			*rate <<= (forceMode - header->mode);
			*granule_frame_size >>= (forceMode - header->mode);
		}
		if (header->mode > forceMode)
		{
			*rate >>= (header->mode - forceMode);
			*granule_frame_size <<= (header->mode - forceMode);
		}
	}

	speex_decoder_ctl(st, SPEEX_SET_SAMPLING_RATE, rate);

	*nframes = header->frames_per_packet;

	if (*channels == -1)
		*channels = header->nb_channels;

	if (!(*channels == 1))
	{
		*channels = 2;
		callback.callback_id = SPEEX_INBAND_STEREO;
		callback.func = speex_std_stereo_request_handler;
		callback.data = stereo;
		speex_decoder_ctl(st, SPEEX_SET_HANDLER, &callback);
	}
	/*
	if (!quiet)
	{
		fprintf(stderr, "Decoding %d Hz audio using %s mode", *rate,
				mode->modeName);

		if (*channels == 1)
			fprintf(stderr, " (mono");
		else
			fprintf(stderr, " (stereo");

		if (header->vbr)
			fprintf(stderr, ", VBR)\n");
		else
			fprintf(stderr, ")\n");
	}
	*/
	*extra_headers = header->extra_headers;
	free(header);
	return st;
}

wtk_ogg_dec_t* wtk_ogg_dec_new()
{
	wtk_ogg_dec_t *dec;

	dec=(wtk_ogg_dec_t*)wtk_calloc(1,sizeof(*dec));
	return dec;
}

int wtk_ogg_dec_delete(wtk_ogg_dec_t *d)
{
	wtk_free(d);
	return 0;
}

void wtk_ogg_dec_start(wtk_ogg_dec_t *d,wtk_ogg_dec_write_f write,void *hook)
{
	SpeexStereoState stereo=SPEEX_STEREO_STATE_INIT;

	d->write_f=write;
	d->write_hook=hook;
	d->stereo=stereo;
	d->forceMode=-1;
	d->enh_enabled=1;
	d->loss_percent=-1;
	ogg_sync_init(&(d->oy));
	speex_bits_init(&(d->bits));
	d->page_granule=0;
	d->last_granule=0;
	d->frame_size=0;
	d->granule_frame_size=0;
	d->nframes=2;
	d->rate=0;
	d->packet_count=0;
	d->channels=-1;
	d->extra_headers=0;
	d->eos=0;
	d->stream_init=0;
	d->st=0;
	d->speex_serialno=-1;
	return;
}

void wtk_ogg_dec_stop(wtk_ogg_dec_t *d)
{
	if(d->st)
	{
		speex_decoder_destroy(d->st);
		d->st=0;
	}
	speex_bits_destroy(&(d->bits));
	if(d->stream_init)
	{
		ogg_stream_clear(&(d->os));
	}
	ogg_sync_clear(&(d->oy));
}

int wtk_ogg_dec_feed(wtk_ogg_dec_t *d,char *data,int len)
{
	short output[MAX_FRAME_SIZE];
	short out[MAX_FRAME_SIZE];
	ogg_sync_state *oy=&(d->oy);
	ogg_page *og=&(d->og);
	ogg_stream_state *os=&(d->os);
	ogg_packet *op=&(d->op);
	SpeexBits *bits=&(d->bits);
	int fsize=200;
	char *s,*e,*p;
	int step,left;
	int packet_no;
	int ret=0;
	int page_nb_packets;
	int skip_samples=0;
	int i,j;

	s=data;
	e=s+len;
	while (s<e)
	{
		left=e-s;
		step=min(left,fsize);
		/*Get the ogg buffer for writing*/
		p = ogg_sync_buffer(oy, step);
		memcpy(p,s,step);
		s+=step;
		ogg_sync_wrote(oy, step);
		/*Loop for all complete pages we got (most likely only one)*/
		while (ogg_sync_pageout(oy,og) == 1)
		{
			if (d->stream_init == 0)
			{
				ogg_stream_init(os, ogg_page_serialno(og));
				d->stream_init = 1;
			}
			if (ogg_page_serialno(og) != os->serialno)
			{
				/* so all streams are read. */
				ogg_stream_reset_serialno(os, ogg_page_serialno(og));
			}
			/*Add page to the bitstream*/
			ogg_stream_pagein(os, og);
			d->page_granule = ogg_page_granulepos(og);
			page_nb_packets = ogg_page_packets(og);
			if (d->page_granule > 0 && d->frame_size)
			{
				/* FIXME: shift the granule values if --force-* is specified */
				skip_samples = d->frame_size
						* (page_nb_packets * d->granule_frame_size * d->nframes
								- (d->page_granule - d->last_granule))
						/ d->granule_frame_size;
				if (ogg_page_eos(og))
				{
					skip_samples = -skip_samples;
				}
				/*else if (!ogg_page_bos(&og))
				 skip_samples = 0;*/
			}
			else
			{
				skip_samples = 0;
			}
			//wtk_debug("skip_samples=%d\n",skip_samples);
			/*printf ("page granulepos: %d %d %d\n", skip_samples, page_nb_packets, (int)page_granule);*/
			d->last_granule = d->page_granule;
			/*Extract all available packets*/
			packet_no = 0;
			//wtk_debug("eos: %d\n",d->eos);
			while (!d->eos && ogg_stream_packetout(os, op) == 1)
			{
				if (op->bytes >= 5 && !memcmp(op->packet, "Speex", 5))
				{
					d->speex_serialno = os->serialno;
				}
				//wtk_debug("%d\n",speex_serialno);
				if (d->speex_serialno == -1 || os->serialno != d->speex_serialno)
				{
					break;
				}
				/*If first packet, process as Speex header*/
				if (d->packet_count == 0)
				{
					d->st = process_header(op, d->enh_enabled, &d->frame_size,
							&d->granule_frame_size, &d->rate, &d->nframes, d->forceMode,
							&d->channels, &d->stereo, &d->extra_headers);
					if (!d->st)
					{
						ret=-1;
						goto end;
					}
					speex_decoder_ctl(d->st, SPEEX_GET_LOOKAHEAD, &d->lookahead);
					if (!d->nframes)
					{
						d->nframes = 1;
					}
				}
				else if (d->packet_count == 1)
				{
					//print comment;
				}
				else if (d->packet_count <= 1 + d->extra_headers)
				{
					/* Ignore extra headers */
				}
				else
				{
					int lost = 0;

					packet_no++;
					//wtk_debug("pktno=%d\n",packet_no);
					if (d->loss_percent > 0
							&& 100 * ((float) rand()) / RAND_MAX < d->loss_percent)
						lost = 1;

					/*End of stream condition*/
					if (op->e_o_s && os->serialno == d->speex_serialno)
					{
						/* don't care for anything except speex eos */
						d->eos = 1;
					}
					/*Copy Ogg packet to Speex bitstream*/
					speex_bits_read_from(bits, (char*) op->packet, op->bytes);
					for (j = 0; j != d->nframes; j++)
					{
						int ret;
						/*Decode frame*/
						if (!lost)
							ret = speex_decode_int(d->st, bits, output);
						else
							ret = speex_decode_int(d->st, NULL, output);

						/*for (i=0;i<frame_size*channels;i++)
						 printf ("%d\n", (int)output[i]);*/

						if (ret == -1)
							break;
						if (ret == -2)
						{
							fprintf(stderr,
									"Decoding error: corrupted stream?\n");
							break;
						}
						if (speex_bits_remaining(bits) < 0)
						{
							fprintf(stderr,
									"Decoding overflow: corrupted stream?\n");
							break;
						}
						if (d->channels == 2)
							speex_decode_stereo_int(output, d->frame_size,
									&d->stereo);

						/*Convert to short and save to output file*/
						for (i = 0; i < d->frame_size * d->channels; i++)
						{
							out[i] = (output[i]);
							//out[i] = le_short(output[i]);
						}
						{
							int frame_offset = 0;
							int new_frame_size = d->frame_size;
							/*printf ("packet %d %d\n", packet_no, skip_samples);*/
							/*fprintf (stderr, "packet %d %d %d\n", packet_no, skip_samples, lookahead);*/
							if (packet_no == 1 && j == 0 && skip_samples > 0)
							{
								/*printf ("chopping first packet\n");*/
								new_frame_size -= skip_samples + d->lookahead;
								frame_offset = skip_samples + d->lookahead;
							}
							if (packet_no == page_nb_packets
									&& skip_samples < 0)
							{
								int packet_length = d->nframes * d->frame_size
										+ skip_samples + d->lookahead;
								new_frame_size = packet_length - j * d->frame_size;
								if (new_frame_size < 0)
									new_frame_size = 0;
								if (new_frame_size > d->frame_size)
									new_frame_size = d->frame_size;
								/*printf ("chopping end: %d %d %d\n", new_frame_size, packet_length, packet_no);*/
							}
							//wtk_debug("nfs=%d lad=%d\n",new_frame_size,d->lookahead);
							if (new_frame_size > 0)
							{
								d->write_f(d->write_hook,(char*)(out + frame_offset * d->channels),
										sizeof(short)*new_frame_size * d->channels);
							}
						}
					}
				}
				d->packet_count++;
			}
		}
	}
end:
	return ret;
}
