CC = gcc
src_dir = wtk third/json qtk ./third/lua-5.1.4/src third/uuid
src_c = ${shell find $(src_dir) -name "*.c"}
out_src = ./third/lua-5.1.4/src/luac.c ./third/lua-5.1.4/src/lua.c
srcs = $(filter-out $(out_src),$(src_c))
objs = $(patsubst %.c,%.o,$(srcs))
tools = vdlg lex gen_set
tool_path = tool
dynamic = libwtkdlg.so
o_sqlite = third/sqlite/sqlite3.o

INC=-I. -I./third -I./include

CFLAGS = $(INC) -Wall -Werror -DUSE_SQL -g -Wno-empty-body
LFLAGS = -lpthread -lm -liconv -lcurl -L.  -lstdc++ -Wl,-rpath=.

ifdef MEM_TEST
	CFLAGS += -fsanitize=address
endif

ifdef RELEASE
	CFLAGS += -O3
endif

tools_e = $(foreach v, $(tools), $(tool_path)/$(v))

.PHONY: all clean test
all: $(dynamic)

define COMPILE_TOOL
$$(tool_path)/$(1): test/tool/$(1).c $$(dynamic) | $$(tool_path)
	$$(CC) $$(CFLAGS) $$< -L. -lwtkdlg $(LFLAGS) -o $$@
endef

$(foreach v, $(tools), $(eval $(call COMPILE_TOOL,$(v))))

%.o:%.c
	$(CC) $(CFLAGS) -fPIC -c -o $@ $<

$(dynamic): $(o_sqlite) $(objs)
	$(CC) $(CFLAGS) -shared  $^ $(LFLAGS) -o $@

$(o_sqlite): third/sqlite/sqlite3.c
	$(CC) -fPIC -O3 -march=native -Wall -funroll-loops -Wno-unused-result -c -o $@ $<

$(tool_path):
	mkdir $@

clean:
	rm $(objs) $(dynamic) $(tools_e)

test: tool/vdlg
	git submodule init
	git submodule update
	./tool/vdlg -c res/yuki/cloud/yuki/yuki.cfg -i res/yuki/verify/yuki.dlg
