pwd=${shell pwd}
wtk_dir=$(pwd)/wtk
asr_dir=$(wtk_dir)/asr
dnnc_dir=$(wtk_dir)/dnnc
core_dir=$(wtk_dir)/core
os_dir=$(wtk_dir)/os

third_dir= $(pwd)/../third
libgoto=$(third_dir)/GotoBLAS2/libgoto2.so
libgoto_static=$(third_dir)/GotoBLAS2/libgoto2.a
libasr=asr.so

src_dir=${pwd}/wtk/asr ${pwd}/wtk/dnnc ${pwd}/wtk/core ${pwd}/wtk/os
src_c=${shell find $(src_dir) -name "*.c"}
src_cpp=${shell find $(src_dir) -name "*.cpp"}
objs=$(patsubst %.c,%.o,$(src_c))
cpp_objs=$(patsubst %.cpp,%.o,$(src_cpp))


CC=gcc
CXX=g++
INC=-I . -I ../.. -I ../../../framework/src -I ../third
CFLAGS=${INC} -O3 -g --fast-math -Ofast -Wall -funroll-loops -DUSE_BLAS -fPIC
LDFLAGS=-lstdc++ -lm -lpthread
GOTOLDFLAGS=-Wl,-rpath=. -L$(third_dir)/GotoBLAS2 -lgoto2 -lgfortran

all: pre ${libgoto} ${objs} ${cpp_objs} $(libasr)
	echo ${objs} ${cpp_objs}

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<;

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<;

${asr_dir}:
	svn co svn://svn.qdreamer2/base/branches/asr-kaldi-decode/wtk/asr ${asr_dir}

${dnnc_dir}:
	svn co svn://svn.qdreamer2/base/tags/cloud/dnn-lz-v1.1/wtk/dnnc ${dnnc_dir}

${core_dir}:
	svn co svn://svn.qdreamer2/base/branches/wtk-kaldi/wtk/core ${core_dir}

${os_dir}:
	svn co svn://svn.qdreamer2/base/tags/cloud/wtk-v1.1/wtk/os ${os_dir}

pre: ${asr_dir} ${dnnc_dir} ${libgoto} ${core_dir} ${os_dir}

${libasr}: qtk_asr_module.o ${objs} ${cpp_objs}
	${CC} -shared ${CFLAGS} $^ ${LDFLAGS} ${GOTOLDFLAGS} -o $@

$(third_dir):
	mkdir $(third_dir)

$(third_dir)/GotoBLAS2: $(third_dir)
	cd $(third_dir) && svn export svn://svn.qdreamer2/base/trunk/asr-lz/third/GotoBLAS2-1.13.tar.gz && tar -xzvf GotoBLAS2-1.13.tar.gz

${libgoto}: $(third_dir)/GotoBLAS2
	cd $(third_dir)/GotoBLAS2 && make TARGET=NEHALEM USE_THREAD=0;

install:
	rm ../../asr.so ../../libgoto2.so; cp $(libasr) $(libgoto) ../..

clean:
	-rm qtk_asr_module.o ${objs} $(libasr)

.PHONY: egram
egram: egram.o ${objs}
	$(CC) ${CFLAGS} $^ ${libgoto_static} ${LDFLAGS} -o $@
