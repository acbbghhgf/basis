pwd=${shell pwd}
wtk_dir=$(pwd)/wtk
qtk_dir=$(pwd)/qtk
third_dir=$(pwd)/third
out_dir=.
target=framework
demo=qtk/test/demo
test=qtk/test/test
wordseg=qtk/test/wordseg
mqtt=qtk/test/mqtt_cli
ping=qtk/test/ping
protobuf=qtk/test/protobuf


libzzip=third/zzip/lib/libzzip.a
libiconv=third/iconv/lib/libiconv.a

########################## src section #################################
obj_src_dir=$(qtk_dir)/conf $(qtk_dir)/frame $(qtk_dir)/entry $(qtk_dir)/websocket $(qtk_dir)/util $(third_dir)/libwebsockets
obj_src=${shell find $(obj_src_dir) -name "*.c"} framework.c
objs=${patsubst %.c,%.o,$(obj_src)}
deps=${patsubst %.c,%.d,$(obj_src)}
override qtklibs+=os core event http mqtt #audio
libs=wtk/libwtk.a $(foreach v,$(qtklibs),qtk/$(v)/libqtk-$(v).a) $(libzzip) $(libiconv)
#########################################################################


####################### flag section ########################################
VER ?= 0.1
INC=-I . -I $(third_dir)/libwebsockets -I $(third_dir)/lua

OF=-O3 -g
CFLAGS=$(INC) -Wall -Werror  $(OF)

CC=gcc
LDFLAGS=-ldl -lm -lz -lrt -lpthread -lssl -lcrypto

COMMIT_ID ?= ${shell git rev-parse --short HEAD}

CFLAGS += -D USE_TCP_NODELAY -D USE_TCP_QUICKACK -D USE_STATICS -D DEBUG_TIME -DUSE_SQL -D VER=${VER} -D BUILD_HASH=${COMMIT_ID}
#############################################################################

lua_dir=$(pwd)/third/lua

CORE_O=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o
LIB_O=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
	lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o
MYOBJS= lzzip.o
BASE_O= $(CORE_O) $(LIB_O) $(MYOBJS)
LUA_O = $(foreach v,$(BASE_O),$(lua_dir)/$(v))


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

%.d: %.c wtk/core wtk/os
	set -e; rm -f $@; \
  echo -e "$@ $(shell dirname $@)/\c" > $@; \
  $(CC) -MM $(CFLAGS) $< >> $@ || rm $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

include $(deps)

$(libzzip):
	cd third/zziplib/ && ./configure --prefix=$(pwd)/third/zzip && make && make install

$(libiconv):
	cd third && tar -xzvf libiconv-1.15.tar.gz && cd libiconv-1.15/ && ./configure --prefix=$(pwd)/third/iconv --enable-static=yes && make && make install

clean:
	-rm $(target).o
	-rm $(target)
	-rm $(objs)
	-rm $(deps)
	-rm $(libs)
	$(foreach v,$(qtklibs),make -C qtk/$(v) clean;)

lua_dir=$(pwd)/third/lua

CORE_O=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o
LIB_O=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
	lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o
MYOBJS= lzzip.o
BASE_O= $(CORE_O) $(LIB_O) $(MYOBJS)
LUA_O = $(foreach v,$(BASE_O),$(lua_dir)/$(v))
LUA_C = ${patsubst %.o,%.c,$(LUA_O)}


demo: $(demo).so $(test).so $(wordseg).so $(mqtt).so $(ping).so $(protobuf).so
	mv $^ $(out_dir)

$(demo).so: $(demo).c
	$(CC) $(CFLAGS) -fPIC --shared $< -o $@

$(test).so: $(test).c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@ -lssl -lcrypto

$(wordseg).so: $(wordseg).c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

$(mqtt).so: $(mqtt).c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

$(ping).so: $(ping).c
	$(CC) $(CFLAGS) -fPIC -shared $< -o $@

$(protobuf).so: $(protobuf).c $(LUA_C)
	$(CC) $(CFLAGS) -fPIC -shared $< $(LUA_C) -o $@
