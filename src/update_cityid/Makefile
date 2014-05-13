CC		=	arm-linux-gcc
HOST	= arm-linux

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall -lsqlite3
LFLAGS += -lpthread -lsqlite3 -Llib/

APP_BINARY	=	city_update

SQLITE_NAME		= sqlite-autoconf-3070900
PREFIX			 = $(PWD)

OBJECTS			+=	cityid_update.o
OBJECTS			+=	database.o
OBJECTS			+=	main.o

all:$(APP_BINARY)

disclean:
	-rm -rf *.o config.log $(APP_BINARY) bin/ share/ include/ lib/ $(SQLITE_NAME)/
clean:
	-rm -rf *.o config.log $(APP_BINARY)
sqlite3:
	tar xzvf $(SQLITE_NAME).tar.gz ;\
	cd $(SQLITE_NAME);\
	./configure --host=$(HOST) --prefix=$(PREFIX); \
	make ;\
	make install

$(APP_BINARY):$(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(APP_BINARY)

	

