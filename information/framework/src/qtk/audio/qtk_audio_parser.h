#ifndef QTK_AUDIO_QTK_AUDIO_PARSER_H_
#define QTK_AUDIO_QTK_AUDIO_PARSER_H_
#include "wtk/core/wtk_type.h"
#include "qtk_decode.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct qtk_audio_parser qtk_audio_parser_t;
typedef int (*qtk_audio_parser_read_handler_t)(void *hook,uint8_t *buf,int buf_size);
typedef int (*qtk_audio_parser_write_handler_t)(void *hook,char *buf,int buf_size);
typedef int (*qtk_audio_parser_done_handler_t)(void *hook);


struct qtk_audio_parser
{
	void *io_ctx;
	int input_size;
	int output_size;
	char *input;
	char *output_alloc;
	short *output;
	//read write callback section;
	void *hook;//	struct qtk_audio_decoder *dec;
	qtk_audio_parser_read_handler_t read;
	qtk_audio_parser_write_handler_t write;
	qtk_audio_parser_done_handler_t done;
	//audio type;
	char *audio_type;   //"mp3","flv"
	int code_id;
	unsigned use_ogg:1;
};

qtk_audio_parser_t* qtk_audio_parser_new(void *hook,qtk_audio_parser_read_handler_t read,qtk_audio_parser_write_handler_t write,qtk_audio_parser_done_handler_t done);
int qtk_audio_parser_delete(qtk_audio_parser_t *p);
int qtk_audio_parser_prepare(qtk_audio_parser_t *d,int type);
int qtk_audio_parser_run(qtk_audio_parser_t* d);
#ifdef __cplusplus
};
#endif
#endif
