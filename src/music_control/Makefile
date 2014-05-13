CC			=	arm-linux-gcc
CPP			= 'arm-linux-gcc -E'
AR			=	'arm-linux-ar rc'
RANLIB	=	arm-linux-ranlib
HOST		= arm-linux

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall
LFLAGS += -lpthread -Iinclude/ -Llib/

APP_BINARY	=	music_control

LIB3TAG_NAME		= libid3tag-0.15.1b
ZLIB_NAME				= zlib-1.2.3
PREFIX			 		= $(PWD)
ZLIB_PREFIX			= $(PWD)/zlib

OBJECTS			+=	main.o 
OBJECTS			+=	music_control.o
OBJECTS			+=	play_list.o 


all:$(APP_BINARY)

disclean:
	-rm -rf *.o config.log $(APP_BINARY) bin/ share/ include/ lib/ $(SQLITE_NAME)/
clean:
	-rm -rf *.o config.log $(APP_BINARY)
libid3tag:
	tar xzvf $(ZLIB_NAME).tar.gz; \
	cd $(ZLIB_NAME); \
	./configure --prefix=$(ZLIB_PREFIX); \
	make CC=$(CC) LDSHARED=$(CC) CPP=$(CPP) AR=$(AR) RABLIB=$(RANLIB); \
	make install; \
	cd ..; \
	rm -rf $(ZLIB_NAME)
	tar xzvf $(LIB3TAG_NAME).tar.gz; \
	cd $(LIB3TAG_NAME); \
	./configure --host=$(HOST) --prefix=$(PREFIX) CPPFLAGS=-I$(ZLIB_PREFIX)/include LDFLAGS=-L$(ZLIB_PREFIX)/lib; \
	make ;\
	make install

$(APP_BINARY):$(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(APP_BINARY)
