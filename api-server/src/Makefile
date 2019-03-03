src_dir = os main base

lua_dir = ./third/lua5.3.4/src/
lua_allsrcs = ${shell find $(lua_dir) -name "*\.c"}
lua_srcs = ${filter-out ./third/lua5.3.4/src/lua.c,$(lua_allsrcs)}

CC = gcc
INC = -I. -I./include/ -I./third/framework/src -I./third/framework/src/third/zzip/include -I./third/framework/src/third/libwebsockets
CFLAGS = $(INC) -Werror -Wall -g -DUSE_PTHREAD_LOCK -DLUA_C89_NUMBERS -O3#-fsanitize=address
SHARED = -fPIC --shared

CSERVICE = logger blua gate upload
LUA_CLIB = bifrost xfer list env md5 sys curl qauth cmsgpack protobuf senddata

CSERVICE_PATH = cservice
LUACLIB_PATH = luaclib

srcs = ${shell find $(src_dir) -name "*.c"}
service_so = $(foreach v, $(CSERVICE), $(CSERVICE_PATH)/$(v).so)
luaclib_so = $(foreach v, $(LUA_CLIB), $(LUACLIB_PATH)/$(v).so)

x = qtk_bifrost.so
framework = bifrostd
lua_ldflags = -L. -l:qtk_bifrost.so -Wl,-rpath=.

all: $(x) $(service_so) $(luaclib_so)

define CSERVICE_TEMP
$$(CSERVICE_PATH)/$(1).so: service-src/service_$(1).c | $$(CSERVICE_PATH)
	$$(CC) $(CFLAGS) $(SHARED) $$< $(lua_ldflags) -o $$@
endef

$(foreach v, $(CSERVICE), $(eval $(call CSERVICE_TEMP,$(v))))

$(CSERVICE_PATH) $(LUACLIB_PATH):
	mkdir $@

$(LUACLIB_PATH)/bifrost.so: lualib-src/lua_bifrost.c lualib-src/lua_seri.c third/uuid/uuid4.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/xfer.so: lualib-src/lua_xfer.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/list.so: lualib-src/lua_list.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/env.so: lualib-src/lua_env.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/md5.so: third/lua-md5/md5.c third/lua-md5/md5lib.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/sys.so: lualib-src/lua_sys.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/cmsgpack.so: lualib-src/lua_cmsgpack.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/protobuf.so: lualib-src/lua_protobuf.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/senddata.so: lualib-src/lua_senddata.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ $(lua_ldflags) -o $@

$(LUACLIB_PATH)/curl.so: lualib-src/lua_curl.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ -lcurl $(lua_ldflags) -o $@

$(LUACLIB_PATH)/qauth.so: lualib-src/lua_qauth.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) $(SHARED) $^ -lssl -lcrypto $(lua_ldflags) -o $@

$(x): $(srcs) $(framework)
	$(CC) $(CFLAGS) $(SHARED) -DLUA_USE_LINUX $(srcs) $(lua_srcs) -o $@

$(framework): third/framework/src/framework
	mv $^ $@

third/framework/src/framework:
	git submodule update
	make -C third/framework/src/ VER=2.0.1 COMMIT_ID=${shell git rev-parse --short HEAD}

clean:
	rm $(x) $(framework) $(CSERVICE_PATH)/*.so $(LUACLIB_PATH)/*.so

cleanall: clean
	make -C third/framework/src/ clean

test:
	valgrind --leak-check=full --show-leak-kinds=all ./sframe -d false

deployRoot = /home/ww/bifrost
deploy: all service.bin
	cp $(framework) $(deployRoot)/bin
	cp $(x) $(deployRoot)/bin
	cp -r $(CSERVICE_PATH) $(deployRoot)/bin
	cp -r $(LUACLIB_PATH) $(deployRoot)/bin
	cp -r ../ext $(deployRoot)

service.bin: service
	zip -r service.zip service lualib
	zzxorcopy -250 service.zip service.bin
	mv service.bin $(deployRoot)/ext
	rm service.zip
