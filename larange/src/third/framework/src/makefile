pwd=${shell pwd}
wtk_dir=$(pwd)/wtk
qtk_dir=$(pwd)/qtk
out_dir=.
target=framework
demo=qtk/test/demo
test=qtk/test/test

########################## src section #################################
obj_src_dir=$(qtk_dir)/conf $(qtk_dir)/frame $(qtk_dir)/entry $(qtk_dir)/util
obj_src=${shell find $(obj_src_dir) -name "*.c"} framework.c
objs=${patsubst %.c,%.o,$(obj_src)}
deps=${patsubst %.c,%.d,$(obj_src)}
override qtklibs+=os core event http #audio
libs=wtk/libwtk.a $(foreach v,$(qtklibs),qtk/$(v)/libqtk-$(v).a)
#########################################################################


####################### flag section ########################################
ver=0.1
INC=-I .

OF=-O3 -g
CFLAGS=$(INC) -Wall -Werror  $(OF)

CC=gcc
LDFLAGS=-ldl -lm -lrt -lpthread -liconv


CFLAGS += -D USE_TCP_NODELAY -D USE_TCP_QUICKACK -D USE_STATICS -D DEBUG_TIME -DUSE_SQL -D VER=${ver}
#############################################################################

all: $(target)

$(target): pre $(objs) $(libs)
	$(CC) $(CFLAGS) -rdynamic -o $@ $(objs) -Wl,--whole-archive $(libs) -Wl,--no-whole-archive $(LDFLAGS)

pre: wtk/core wtk/os

wtk/os/%.h wtk/core/%.h: wtk/os wtk/core

wtk/core wtk/os:
	make pre -C wtk

define LIB_TEMP
  $(2)/lib$(1).a: $${shell find $(2) -name "*.c"}
	make -C $(2)
endef

$(eval $(call LIB_TEMP,wtk,wtk))

$(foreach v,$(qtklibs),$(eval $(call LIB_TEMP,qtk-$(v),qtk/$(v))))

include $(deps)

%.d: %.c wtk/core wtk/os
	set -e; rm -f $@; \
  echo -e "$@ $(shell dirname $@)/\c" > $@; \
  $(CC) -MM $(CFLAGS) $< >> $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm $(target).o
	-rm $(target)
	-rm $(objs)
	-rm $(deps)
	-rm $(libs)
	$(foreach v,$(qtklibs),make -C qtk/$(v) clean;)

demo: $(demo).so $(test).so
	mv $^ $(out_dir)

$(demo).so: $(demo).c
	$(CC) $(CFLAGS) -fPIC --shared $< -o $@

$(test).so: $(test).c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@ -lssl -lcrypto
