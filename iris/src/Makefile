VPATH = ../src:../framework
RM = rm
MV = mv
CC = gcc

ifndef $(install_dir)
install_dir=/data/qtk/iris
endif

OBJS = ${shell find . -maxdepth 1 -name "*.c" | sed "s/\.c/\.o/g" }
FRAMEWORK = ../framework/src
LDFLAGS = 
CFLAGS = -fPIC -shared -g -I. -I../framework/src/ -Wall -Werror -Werror=unused-variable -Wno-format-security

ifeq ($(BITS), 32)
	CFLAGS += -m32
else
	ifeq ($(BITS), 64)
		CFLAGS += -m64
	endif
endif

TAGS = iris.so

.PHONY: all

all: irisd $(TAGS)

$(TAGS): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

%.o:%.c,%.h
	$(CC) $(CFLAGS) -c -o $@ $<

irisd: $(FRAMEWORK)/framework
	git submodule update
	make -C $(FRAMEWORK) VER=2.0.1 COMMIT_ID=${shell git rev-parse --short HEAD}
	mv $(FRAMEWORK)/framework irisd

.PHONY: framework
$(FRAMEWORK)/framework:
	make -C $(FRAMEWORK)  VER=2.0.1 COMMIT_ID=${shell git rev-parse --short HEAD}
	mv $(FRAMEWORK)/framework irisd

../framework:
	git submodule init

.PHONY: clean
clean:
	$(RM) -f $(TAGS)
	$(RM) -f $(OBJS)
	rm irisd 

.PHONY: install
install:
	@if [ -d $(install_dir)/bin ]; then \
       rm -rf $(install_dir)/bin/*.so $(install_dir)/bin/irisd; \
    else \
       mkdir -p $(install_dir)/bin; \
	fi;
	cp iris.so irisd $(install_dir)/bin;

