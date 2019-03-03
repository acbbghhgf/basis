MV = mv
AR = ar
CC = gcc
CFLAGS = -g

TARGET = libqtk-audio.a
DIR_LIB = libs
ARSCRIPT=script.ar
SILENT=@
OBJS = ${shell find . -name "*.c" | sed "s/\.c/\.o/g"}
ARCHIVES=$(wildcard $(DIR_LIB)/*.a)

.PHONY: all clean
all: $(TARGET)
	echo 'pass'

$(TARGET):$(OBJS)
	$(SILENT)echo "CREATE $@" > $(ARSCRIPT)
	$(SILENT)for a in $(ARCHIVES); do (echo "ADDLIB $$a" >> $(ARSCRIPT)); done
	$(SILENT)echo "ADDMOD $(OBJS)" >> $(ARSCRIPT)
	$(SILENT)echo "SAVE" >> $(ARSCRIPT)
	$(SILENT)echo "END" >> $(ARSCRIPT)
	$(AR) -M < $(ARSCRIPT)
	ranlib $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $< -I . -I ./include -I ../../

clean:
	find . -name "*.o" | xargs $(RM) -f
	rm ARSCRIPT
