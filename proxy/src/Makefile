src_dir = main os third/uuid/
srcs = ${shell find $(src_dir) -name "*.c"}

lua_dir = third/lua5.3.4/src/
lua_allsrcs = ${shell find $(lua_dir) -name "*\.c"}
lua_srcs = ${filter-out third/lua5.3.4/src/lua.c,$(lua_allsrcs)}

lua_ldflags = -L. -l:$(x) -Wl,-rpath=.

objs = ${patsubst %.c,%.o,$(srcs) $(lua_srcs)}

CC = gcc
INC = -I. -I./include/ -I./third/framework/src/ -I./third/framework/src/third/zzip/include
CFLAGS = $(INC) -Werror -Wall -g -DUSE_PTHREAD_LOCK -DLUA_USE_LINUX -fPIC -O3#-fsanitize=address
LFAGS = -lpthread

LUA_CLIB = xfer proxy list
LUACLIB_PATH = luaclib

luaclib_so = $(foreach v, $(LUA_CLIB), $(LUACLIB_PATH)/$(v).so)

x = qtk_proxy.so
framework = proxyd

all: $(x) $(luaclib_so)

$(x): $(framework) $(objs)
	$(CC) $(CFLAGS) --shared $(objs) $(LFAGS) -o $@

$(LUACLIB_PATH)/xfer.so : lualib-src/lua_xfer.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) --shared -fPIC $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/proxy.so : lualib-src/lua_proxy.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) --shared -fPIC $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/list.so : lualib-src/lua_list.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) --shared -fPIC $^ $(lua_ldflags) -o $@

$(framework): third/framework/src/framework
	cp $^ $@

third/framework/src/framework:
	git submodule update
	make -C third/framework/src/ VER=2.0.1 COMMIT_ID=${shell git rev-parse --short HEAD}

%.o: %.c,%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(LUACLIB_PATH):
	mkdir $@

clean:
	rm $(x) $(luaclib_so) $(framework)

cleanall: clean
	rm $(objs)
	make -C third/framework/src/ clean

deployRoot = /data/qtk/proxy
deploy: all service.bin
	cp $(x) $(deployRoot)/bin
	cp $(framework) $(deployRoot)/bin
	cp -r $(LUACLIB_PATH) $(deployRoot)/bin
	cp -r ../ext $(deployRoot)
	mv service.bin $(deployRoot)/bin

service.bin: lualib lua
	zip -r service.zip lualib lua
	zzxorcopy -250 service.zip service.bin
	rm service.zip
