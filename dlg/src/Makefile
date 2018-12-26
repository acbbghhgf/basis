src_dir = main pipe qtk mod pack lua third/uuid
srcs = ${shell find $(src_dir) -name "*.c"}

lua_dir = third/lua5.3.4/src/
lua_allsrcs = ${shell find $(lua_dir) -name "*\.c"}
lua_outsrcs = third/lua5.3.4/src/lua.c third/lua5.3.4/src/luac.c
lua_srcs = ${filter-out $(lua_outsrcs), $(lua_allsrcs)}

objs = ${patsubst %.c,%.o,$(srcs) $(lua_srcs)}

CC = gcc
INC = -I. -I./include/ -I./third/framework/src -I./wtk
CFLAGS = $(INC)  -Wall -Werror -g -DUSE_PTHREAD_LOCK -DUSE_SQL -fPIC -DLUA_USE_LINUX -O1 #-fsanitize=address
LFAGS = -lpthread -lhiredis

LUA_CLIB = dlg xfer
LUACLIB_PATH = luaclib

luaclib_so = $(foreach v, $(LUA_CLIB), $(LUACLIB_PATH)/$(v).so)

x = qtk_dlg.so
module = libwtkdlg.so
framework = dlgd

all: $(x) $(framework) $(luaclib_so) $(module)

$(x): $(objs)
	$(CC) $(CFLAGS) --shared $(objs) $(LFAGS) -o $@

%.o: %.c,%.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(LUACLIB_PATH)/dlg.so: lualib-src/lua_dlg.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) --shared $^ -o $@

$(LUACLIB_PATH)/xfer.so: lualib-src/lua_xfer.c | $(LUACLIB_PATH)
	$(CC) $(CFLAGS) --shared $^ -o $@

$(LUACLIB_PATH):
	mkdir $@

$(module):
	svn co --depth=empty svn://svn.qdreamer2/semdlg/trunk/dialogue-text/ tmp
	cd tmp; svn up libwtkdlg.so; mv libwtkdlg.so ..
	rm tmp -rf

$(framework): third/framework/src/framework
	cp $^ $@

third/framework/src/framework:
	git submodule update
	make -C third/framework/src/

clean:
	rm $(x) $(framework) $(luaclib_so) $(module)

cleanall: clean
	rm $(objs) $(libs)
	make -C third/framework/src/ clean

deployRoot = /data/qtk/dlg/
deploy: all
	cp $(x) $(deployRoot)/bin
	cp $(framework) $(deployRoot)/bin
	cp $(module) $(deployRoot)/bin
	cp -r $(LUACLIB_PATH) $(deployRoot)/bin
	cp -r script $(deployRoot)/bin
	#cp -r ../ext $(deployRoot)/
