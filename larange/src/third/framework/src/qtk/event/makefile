pwd=${shell pwd}
out_dir=.

obj_src_dir=.
obj_src=${shell find $(obj_src_dir) -name "*.c"}
objs=${patsubst %.c,%.o,${obj_src}}
target=libqtk-event.a

CC=gcc
CFLAGS=-Wall -Werror -g -O3
LDFLAGS=

all: $(target)
	echo $(target)

$(target): $(objs)
	$(AR) rcs $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -I ../.. $(LDFLAGS)

clean:
	-rm $(objs)
