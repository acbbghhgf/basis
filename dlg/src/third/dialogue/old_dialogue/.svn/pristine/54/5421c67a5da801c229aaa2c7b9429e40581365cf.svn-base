#ifndef WTK_CORE_WTK_AUDIO_TYPE_H_
#define WTK_CORE_WTK_AUDIO_TYPE_H_
#include "wtk/core/wtk_type.h"
#include "wtk_str.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	AUDIO_WAV=0,
	AUDIO_MP3=1,
	AUDIO_OGG=2,		//speex
	AUDIO_FLV=3,
	AUDIO_TEXT=4,
    AUDIO_UNKNOWN=5,
}wtk_audio_type_t;

#define wtk_audio_type_is_audio(a) ((a)>=AUDIO_WAV && (a)<=AUDIO_FLV)

wtk_string_t* wtk_audio_type_to_fnstring(wtk_audio_type_t type);
wtk_audio_type_t wtk_audio_type_from_string(wtk_string_t *v);

/**
 * @brief returned string is from static segment, so should not be freed;
 */
wtk_string_t* wtk_audio_type_to_string(wtk_audio_type_t t);
#ifdef __cplusplus
};
#endif
#endif
