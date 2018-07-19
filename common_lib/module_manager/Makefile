#ARCH='arm'
#HOST?=$(ARCH)
#CROSS-COMPILE=$(ARCH)-elf-
CC=$(CROSS-COMPILE)gcc
PWD=$(shell pwd)

CFLAGS=-O2
LDFLAGS=
LIBS=-lpthread -ldl
TARGET=module-manager

OBJ = src/module_manager.o 	\
      src/tcp_client.o 		\
      src/tcp_client_list.o 	\
      src/dl_parser.o 		\
      src/dl_loader.o 		\
      src/watchdog.o 		\
      src/logcat.o 		\
      src/live_update.o 	\
      parson/parson.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

install:
	cp $(TARGET) /usr/bin

.PHONY: clean

clean:
	rm -f src/*.o $(TARGET)
