pwd=${shell pwd}
wtk_dir=$(pwd)/wtk
eval_dir=$(wtk_dir)/edu
asr_dir=$(wtk_dir)/asr
dnnc_dir=$(wtk_dir)/dnnc
third_dir= $(pwd)/../third
nosqlite_dir=${pwd}/../third/nosqlite
cjson_dir=${pwd}/../third/json


libeval=eval.so

src_dir=${pwd}/wtk ${nosqlite_dir} ${cjson_dir}
src_c=${shell find $(src_dir) -name "*.c"} qtk_eval_module.c
src_cpp=${shell find $(src_dir) -name "*.cpp"}
objs=$(patsubst %.c,%.o,$(src_c))
cpp_objs=$(patsubst %.cpp,%.o,$(src_cpp))


CC=gcc
CXX=g++
INC=-I. -I../.. -I../../../framework/src -I../third

CFLAGS=${INC} -g -O3 --fast-math -Ofast -Wall -Werror -funroll-loops -DUSE_SQL -DHAVE_CONFIG_H -DUSE_ALIGN -DUSE_FST
LDFLAGS=-Wl,-rpath=. -lstdc++ -L$(third_dir)/GotoBLAS2 -lgoto2 -lpthread
SHARED=-fPIC

all: pre ${objs} ${cpp_objs} $(libeval) # a.out

a.out: ${objs} ${cpp_objs} test.o ../../../framework/src/qtk/core/cJSON.o \
  ../third/sqlite/sqlite3.o \
  ../../../framework/src/qtk/audio/libqtk-audio.a
	${CC} ${CFLAGS} $^ -lm -lpthread -liconv -ldl -lstdc++

../third/sqlite/sqlite3.o: ../third/sqlite/sqlite3.c
	${CC} -I. -g -O3 -Wall -funroll-loops -c -o $@ $<;

%.o: %.c
	$(CC) $(CFLAGS) $(SHARED) -c -o $@ $<;

%.o: %.cpp
	$(CXX) $(CFLAGS) $(SHARED) -c -o $@ $<;

pre: ${eval_dir} ${nosqlite_dir} ${cjson_dir} ${asr_dir} ${dnnc_dir}

${eval_dir}:
	svn co svn://svn.qdreamer2/base/tags/eval-v0.0.4/wtk ${eval_dir}

${nosqlite_dir}:
	svn co svn://svn.qdreamer2/base/tags/eval-v0.0.4/third/nosqlite ${nosqlite_dir}

${cjson_dir}:
	svn co svn://svn.qdreamer2/base/tags/eval-v0.0.4/third/json ${cjson_dir}

${asr_dir}:
	svn co svn://svn.qdreamer2/base/tags/asr-lz-v0.2.9/wtk/asr ${asr_dir}

${dnnc_dir}:
	svn co svn://svn.qdreamer2/base/tags/cloud/dnn-lz-v1.1/wtk/dnnc ${dnnc_dir}

${libeval}: ${objs} ${cpp_objs}
	${CC} -shared ${CFLAGS} $(SHARED) ${objs} ${cpp_objs} ${LDFLAGS} -o $@

install:
	cp $(libeval) ../..

clean:
	-rm ${objs} ${cpp_objs} $(libeval)
