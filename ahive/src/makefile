pwd=${shell pwd}
wtk_dir=$(pwd)/wtk
qtk_dir=$(pwd)/qtk
service_dir=$(qtk_dir)/service
service_libdir=$(pwd)
lua_dir=$(pwd)/third/lua
luac_dir=$(qtk_dir)/luac

ifndef $(install_dir)
install_dir=/data/qtk/ahive
endif

CC=gcc
target=ahive.so

CFLAGS = -g -O3 -Ofast -Wall -Werror -I ../framework/src -I ../framework/src/third/zzip/include
LDFLAGS = -lm -ldl -lrt -lpthread -Wl,-E
SHARED = -fPIC --shared

WTK_LIB = $(wtk_dir)/libwtk.a
QTK_LIB = $(qtk_dir)/libqtk.a
CORE_O=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o
LIB_O=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
	lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o
MYOBJS= lzzip.o
BASE_O= $(CORE_O) $(LIB_O) $(MYOBJS)
LUA_O = $(foreach v,$(BASE_O),$(lua_dir)/$(v))

ZEUS_SRC = qtk_zeus_main.c      \
	qtk_zeus_cfg.c          \
	qtk_zeus_server.c       \
	qtk_zeus_timer.c        \
	qtk_zeus_mq.c           \
	qtk_zeus_module.c       \
	qtk_zeus_handle.c       \
	qtk_zenv.c              \
	qtk_zsocket_server.c    \

UTIL_SRC = $(shell find $(qtk_dir)/util -name "*.c")
PROTO_SRC = $(shell find $(qtk_dir)/proto -name "*.c")
TEST_SRC = $(shell find $(pwd)/test -name "*.c")

LUAC_LIBPATH = $(pwd)

LUAC_ZEUS = qtk_zeus_core.c    \
	qtk_serialize.c        \
	qtk_zeus_socket.c      \
	qtk_engine.c           \
	qtk_zeus_buffer.c      \
	qtk_zeus_reader.c      \
	qtk_zeus_iris.c        \
	qtk_zeus_http.c

LUAC_LIB = zeus md5 snapshot

SERVICE = zlua logger aisrv

all: $(luadir)/liblua.a ahived $(target) \
  $(foreach v,$(SERVICE),$(service_libdir)/$(v).so) \
  $(foreach v,$(LUAC_LIB),$(LUAC_LIBPATH)/$(v).so)

$(luadir)/liblua.a: ../framework/src/third/zzip
	cd $(lua_dir) && make CC='$(CC) -std=gnu99' linux

../framework/src/third/zzip: framework

$(service_libdir):
	mkdir $(service_libdir)

$(target) : $(foreach v, $(ZEUS_SRC), $(qtk_dir)/zeus/$(v)) \
   $(PROTO_SRC) $(LUA_O)
	$(CC) $(CFLAGS) $(SHARED) -o $@ $^ -I. -I $(lua_dir) $(LDFLAGS)

launch_libs=../framework/src/qtk/event/libqtk-event.a \
  ../framework/src/qtk/os/libqtk-os.a                 \
  ../framework/src/qtk/audio/libqtk-audio.a           \
  ../framework/src/qtk/core/libqtk-core.a             \
  ../framework/src/wtk/libwtk.a

ahive : $(foreach v, $(ZEUS_SRC), $(qtk_dir)/zeus/$(v)) \
   $(PROTO_SRC) $(LUA_O) | $(launch_libs)
	$(CC) $(CFLAGS) -rdynamic -o $@ $^ -Wl,--whole-archive $(launch_libs) -Wl,--no-whole-archive -I. -I $(lua_dir) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(SHARED) -c -o $@ $^ -I. -I $(lua_dir)

test_aisrv: $(service_dir)/qtk_aisrv.o test/aisrv/test_aisrv.o
	$(CC) $(CFLAGS) $(SHARED) -o test_aisrv.so $^ -I. -I../framework/src $(LDFLAGS)

define SERVICE_TEMP
  $$(service_libdir)/$(1).so: $$(service_dir)/qtk_$(1).c $$(target) | $$(service_libdir)
	$$(CC) $$(CFLAGS) $$(SHARED) $$< -L. -l:ahive.so -o $$@ -I . -I$$(lua_dir) -Wl,-rpath=.
endef

$(foreach v,$(SERVICE),$(eval $(call SERVICE_TEMP,$(v))))

$(LUAC_LIBPATH)/zeus.so: $(addprefix $(luac_dir)/,$(LUAC_ZEUS)) | $(LUAC_LIBPATH)
	$(CC) $(CFLAGS) $(SHARED) $^ -o $@ -I . -I $(lua_dir) -DLUA_LIB

$(LUAC_LIBPATH)/md5.so: $(luac_dir)/qtk_md5.c $(luac_dir)/qtk_md5lib.c | $(LUAC_LIBPATH)
	$(CC) $(CFLAGS) $(SHARED) $^ -o $@ -I . -I $(lua_dir) -DLUA_LIB

$(LUAC_LIBPATH)/snapshot.so: $(luac_dir)/qtk_snapshot.c | $(LUAC_LIBPATH)
	$(CC) $(CFLAGS) $(SHARED) $^ -o $@ -I . -I $(lua_dir) -DLUA_LIB

ahived: ../framework/src
	cd .. && git submodule update; cd -
	make -C ../framework/src qtklibs=audio
	mv ../framework/src/framework ahived

.PHONY: framework
framework:
	make -C ../framework/src qtklibs=audio  VER=2.0.1 COMMIT_ID=${shell git rev-parse --short HEAD}
	mv ../framework/src/framework ahived

../framework/src:
	cd .. && git submodule init; cd -
	git submodule update

.PHONY: install_pack
install_pack:
	@if [ -d $(install_dir)/bin ]; then \
       rm -rf $(install_dir)/bin/*.so $(install_dir)/bin/ahived; \
    else \
       mkdir -p $(install_dir)/bin; \
	fi;
	@if [ -d $(install_dir)/lualib ]; then \
       rm -rf $(install_dir)/lualib/*; \
    else \
       mkdir $(install_dir)/lualib; \
    fi;
	@if [ -d $(install_dir)/script ]; then \
       rm -rf $(install_dir)/script; \
    fi;
	@if [ -d $(install_dir)/service ]; then \
       rm -rf $(install_dir)/service; \
    fi;
	@if [ ! -d $(install_dir)/ext/lua ]; then \
       mkdir -p $(install_dir)/ext/lua; \
    fi;
	zip -r pack.zip ../lualib ../service ../script ../ext/lua/global.lua
	zzxorcopy -250 pack.zip pack.bin
	rm pack.zip
	cp pack.bin $(install_dir)/bin/
	cp ../lualib/loader.lua $(install_dir)/lualib
	cp $(target) ahived $(foreach v,$(SERVICE) $(LUAC_LIB),$(v).so) \
  asr.so libgoto2.so en.tts.so fy.so libusc.so usc.so tts.so eval.so $(install_dir)/bin

.PHONY: clean
clean:
	rm $(target) \
	  $(foreach v,$(SERVICE),$(service_libdir)/$(v).so) \
	  $(foreach v,$(LUAC_LIB),$(LUAC_LIBPATH)/$(v).so)
	rm ahived


install:
	@if [ -d $(install_dir)/bin ]; then \
       rm -rf $(install_dir)/bin/*.so $(install_dir)/bin/ahived; \
    else \
       mkdir -p $(install_dir)/bin; \
	fi;
	@if [ -d $(install_dir)/lualib ]; then \
       rm -rf $(install_dir)/lualib; \
    fi;
	@if [ -d $(install_dir)/script ]; then \
       rm -rf $(install_dir)/script; \
    fi;
	@if [ -d $(install_dir)/service ]; then \
       rm -rf $(install_dir)/service; \
    fi;
	@if [ ! -d $(install_dir)/ext/lua ]; then \
       mkdir -p $(install_dir)/ext/lua; \
    fi;
	cp ../ext/lua/global.lua $(install_dir)/ext/lua
	cp -r ../lualib $(install_dir)
	cp -r ../service $(install_dir)
	cp -r ../script $(install_dir)
	cp $(target) ahived $(foreach v,$(SERVICE) $(LUAC_LIB),$(v).so) \
  asr.so libgoto2.so en.tts.so fy.so libusc.so usc.so tts.so eval.so $(install_dir)/bin
