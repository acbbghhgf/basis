pwd=${shell pwd}

libifly=ifly.so

src_c=qtk_ifly_module.c \
  qtk_ifly.c \
  qtk_ifly_cfg.c
objs=$(patsubst %.c,%.o,$(src_c))


CC=gcc
INC=-I . -I ./ifly_sdk/include -I ../.. -I ../../../framework/src
CFLAGS=${INC} -O3 --fast-math -Ofast -Wall -funroll-loops -fPIC
LDFLAGS=-Wl,-rpath=. -L./ifly_sdk/libs/x64 -lmsc

all: ${objs} $(libifly)
	echo ${objs}

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<;

${libifly}: ${objs}
	${CC} -shared ${CFLAGS} ${objs} $(LDFLAGS) -o $@

install:
	cp $(libifly) ../..
	cp ./ifly_sdk/libs/x64/libmsc.so ../..

clean:
	-rm ${objs} $(libifly)
