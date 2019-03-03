#include "qtk_encode.h"
#include "lame.h"

int mp3_encode(char* pcm,int len,char **out,int *out_len)
{
	lame_global_flags   *gf;
	int ret,out_size;
	unsigned char* buf;
#ifdef USE_STEP
	char *last=pcm+len;
	int samples,s;
#endif

	gf=lame_init();
	ret=-1;
	if(!gf){goto end;}
	pcm+=44;len-=44;
	out_size=1.25*(len/2) + 7200;
	buf=(unsigned char*)wtk_malloc(out_size*sizeof(char));
	lame_set_num_channels(gf,1);
	lame_set_in_samplerate(gf,16000);
	lame_set_mode(gf,3);
	lame_set_quality(gf,2);
	lame_init_params(gf);
#ifdef USE_STEP
	s=0;
	while(pcm<last)
	{
		samples=min(40960,last-pcm);
		ret=lame_encode_buffer(gf,(short*)pcm,(short*)pcm,samples/2,buf+s,out_size-s);
		wtk_debug("%d,%d\n",samples,ret);
		if(ret>0)
		{
			s+=ret;
			pcm+=samples;
		}else
		{
			wtk_debug("%d\n",ret);
			break;
		}
	}
	if(ret>=0)
	{
		*out=(char*)buf;
		*out_len=s;
		ret=0;
	}else
	{
		free(buf);
		ret=-1;
	}
#else
	ret=lame_encode_buffer(gf,(short*)pcm,(short*)pcm,len/2,buf,out_size);
	if(ret>0)
	{
		int t;
		t=lame_encode_flush(gf,buf+ret,out_size-ret);
		if(t>0){ret+=t;}
	}
	if(ret>0)
	{
		*out=(char*)buf;
		*out_len=ret;
		ret=0;
	}else
	{
		free(buf);
		ret=-1;
	}
#endif
	lame_close(gf);
end:
	return ret;
}

/**
 *	@param sample_data pointer to the input raw sample data, which is short buffer;
 *	@param nsample samples of input sample_data;
 *	@param sample_rate sample rate of input data;
 *	@param out which save output encoded mp3 data when encode success;
 *	@param out_len bytes of output encoded mp3 data;
 *	@return 0 on success else failed.
 */
int mp3_encode2(short* sample_data,int nsample,int sample_rate,char **out,int *out_len,int bitrate)
{
	lame_global_flags   *gf;
	int ret,out_size;
	unsigned char* buf;

	ret=-1;
	gf=lame_init();
	if(!gf){goto end;}
	//set channel
	lame_set_num_channels(gf,1);
	//set sample rate
	lame_set_in_samplerate(gf,sample_rate);
	//set mono
	lame_set_mode(gf,3);
	lame_set_quality(gf,2);
	//lame_set_VBR(gf,vbr_default);
	//lame_set_VBR_mean_bitrate_kbps(gf,bitrate);
	lame_init_params(gf);
	//malloc memory for save mp3 data.
	out_size=1.25*nsample + 7200;
	buf=(unsigned char*)wtk_malloc(out_size*sizeof(char));
	ret=lame_encode_buffer(gf,sample_data,sample_data,nsample,buf,out_size);
	if(ret>0)
	{
		int t;
		t=lame_encode_flush(gf,buf+ret,out_size-ret);
		if(t>0){ret+=t;}
	}
	if(ret>0)
	{
		*out=(char*)buf;
		*out_len=ret;
		ret=0;
	}else
	{
		free(buf);
		ret=-1;
	}
	lame_close(gf);
end:
	return ret;
}
