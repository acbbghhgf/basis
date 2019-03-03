pwd=${shell pwd}

libentts=en.tts.so

src_dir=${pwd}/src
src_c=${shell find $(src_dir) -name "*.c"} qtk_entts_module.c
obj_src=${filter-out %utils/cst_mmap_none.c,$(src_c)}
c_objs=$(patsubst %.c,%.o,$(obj_src))

src_cpp=${shell find $(src_dir) -name "*.cpp"}
cpp_objs=$(patsubst %.cpp,%.o,$(src_cpp))

objs=$(c_objs) $(cpp_objs)


CC=gcc
CXX=g++
INC=-I . -I ./src/include -I ./src/interface -I ../.. -I ../../../framework/src -I ../third -I ./src -I ./src/lang/cmu_us_timix -I ./src/lang/usenglish -I ./src/lang/cmulex
CFLAGS=${INC} -O3 --fast-math -Ofast -Wall -funroll-loops -fPIC -D RESOURSE_FILE_ENCRYPTED

all: ${objs} $(libentts)
	echo ${objs}

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<;

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<;

${libentts}: ${objs}
	${CXX} -shared ${CFLAGS} ${objs} -o $@

pre: ./src

./src:
	svn co svn://svn.qdreamer2/tts/tags/tts.en-lz-v0.0.1.1/src src

install:
	cp $(libentts) ../..

clean:
	-rm ${objs} $(libentts)
