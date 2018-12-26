pwd=${shell pwd}
wtk_dir=$(pwd)/wtk

libtts=tts.so

src_dir=${pwd}/wtk/tts ${pwd}/wtk/lex
src_c=${shell find $(src_dir) -name "*.c"} qtk_tts_module.c
objs=$(patsubst %.c,%.o,$(src_c))


CC=gcc
INC=-I . -I ../.. -I ../../../framework/src -I ../third
CFLAGS=${INC} -O3 -g --fast-math -Ofast -Wall -funroll-loops -fPIC

all: pre ${objs} $(libtts)
	echo ${objs}

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<;


${libtts}: ${objs}
	${CC} -shared ${CFLAGS} ${objs} -o $@

pre: ./wtk/tts ./wtk/mtts ./wtk/lex

./wtk/tts:
	svn co svn://svn.qdreamer2/tts/tags/tts-lz-v0.0.2.1/wtk/tts wtk/tts

./wtk/mtts:
	svn co svn://svn.qdreamer2/tts/tags/tts-lz-v0.0.2.1/wtk/mtts wtk/mtts

./wtk/lex:
	svn co svn://svn.qdreamer2/base/tags/cloud/lex2-lz-v1.1/wtk/lex wtk/lex

install:
	cp $(libtts) ../..

clean:
	-rm ${objs} $(libtts)
