#include "wtk_audio_type.h"

wtk_string_t* wtk_audio_type_to_fnstring(wtk_audio_type_t type)
{
	static wtk_string_t fn[]={
			wtk_string("wav"),
			wtk_string("mp3"),
			wtk_string("ogg"),
			wtk_string("flv"),
			wtk_string("data")
	};
	static wtk_audio_type_t at[]={
			AUDIO_WAV,
			AUDIO_MP3,
			AUDIO_OGG,
			AUDIO_FLV,
	};
	static int n=sizeof(fn)/sizeof(wtk_string_t)-1;
	int i;
	wtk_string_t *name=0;

	for(i=0;i<n;++i)
	{
		if(type==at[i])
		{
			name=&(fn[i]);
			break;
		}
	}
	if(i==n)
	{
		name=&(fn[3]);
	}
	return name;
}

wtk_audio_type_t wtk_audio_type_from_string(wtk_string_t *v)
{
	static wtk_string_t audio[]={
			wtk_string("audio/x-wav"),
			wtk_string("audio/mpeg"),
			wtk_string("audio/ogg"),
			wtk_string("audio/flv"),
			wtk_string("text/plain"),
			wtk_string("application/x-www-form-urlencoded"),
			wtk_string("text/xml"),
	};
	static wtk_audio_type_t at[]={
			AUDIO_WAV,
			AUDIO_MP3,
			AUDIO_OGG,
			AUDIO_FLV,
			AUDIO_TEXT,
			AUDIO_TEXT,
			AUDIO_TEXT,
	};
	static int n=sizeof(audio)/sizeof(wtk_string_t);
	int i;
	wtk_audio_type_t audio_type;

	for(i=0;i<n;++i)
	{
		if(audio[i].len<=v->len && (strncmp(audio[i].data,v->data,audio[i].len)==0))
		{
			audio_type=at[i];
			break;
		}
	}
	if(i==n)
	{
		audio_type=AUDIO_UNKNOWN;
	}
	return audio_type;
}

wtk_string_t* wtk_audio_type_to_string(wtk_audio_type_t t)
{
	static wtk_string_t audio[]={
			wtk_string("audio/x-wav"),
			wtk_string("audio/mpeg"),
			wtk_string("audio/ogg"),
			wtk_string("audio/amr"),
			wtk_string("audio/flv"),
			wtk_string("text/plain"),
			wtk_string(""),
			wtk_string(""),
	};

	return &(audio[t]);
}
