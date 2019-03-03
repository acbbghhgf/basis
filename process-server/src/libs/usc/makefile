pwd=${shell pwd}

libusc=usc.so

src_c=qtk_usc_module.c \
  qtk_usc.c \
  qtk_usc_cfg.c
objs=$(patsubst %.c,%.o,$(src_c))


CC=gcc
INC=-I . -I ./usc_sdk/libs -I ../.. -I ../../../framework/src
CFLAGS=${INC} -O3 --fast-math -Ofast -Wall -funroll-loops -fPIC
LDFLAGS=-Wl,-rpath=. -L./usc_sdk/libs -lusc

all: ${objs} $(libusc)
	echo ${objs}

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<;

${libusc}: ${objs}
	${CC} -shared ${CFLAGS} ${objs} $(LDFLAGS) -o $@

install:
	rm ../../$(libusc) ../../libusc.so; cp $(libusc) ./usc_sdk/libs/libusc.so ../..

clean:
	-rm ${objs} $(libusc)
