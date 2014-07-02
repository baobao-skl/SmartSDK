CC		=	arm-linux-gcc
HOST	= arm-linux

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall -lsqlite3 -I.
LFLAGS += -lpthread -lsqlite3 -Llib/ -lm

APP_BINARY	=	dev_manage

SQLITE_NAME		= sqlite-autoconf-3070900
PREFIX			 = $(PWD)

OBJECTS			+=	socketd.o 
OBJECTS			+=	dev_add.o 
OBJECTS			+=	dev_del.o 
OBJECTS			+=	dev_edit.o
OBJECTS			+=	dev_control.o
OBJECTS			+=	dev_db.o
OBJECTS			+=	uart.o
OBJECTS			+=	uart_common.o 
OBJECTS			+=	main.o
OBJECTS			+=	sms.o
OBJECTS			+=	sms_common.o
OBJECTS			+=	net_broadcast.o
OBJECTS			+=	mail_common.o
OBJECTS			+=	cityid.o
OBJECTS			+=	alarm_db.o
OBJECTS			+=	alarm_common.o
OBJECTS			+=	mail.o
OBJECTS			+=	sms_control.o
OBJECTS			+=	familygroup.o
OBJECTS			+=	wechat.o
OBJECTS			+=	led.o
OBJECTS			+=	ring.o
OBJECTS			+=	timertask.o
OBJECTS			+=	timertask_db.o
OBJECTS			+=	version_manager.o
OBJECTS			+=	uart_quene.o
OBJECTS			+=	cJSON/cJSON.o

all:$(APP_BINARY)

disclean:
	-rm -rf *.o config.log $(APP_BINARY) cJSON/*.o bin/ share/ include/ lib/ $(SQLITE_NAME)/
clean:
	-rm -rf *.o config.log $(APP_BINARY) cJSON/*.o
sqlite3:
	tar xzvf $(SQLITE_NAME).tar.gz ;\
	cd $(SQLITE_NAME);\
	./configure --host=$(HOST) --prefix=$(PREFIX); \
	make ;\
	make install

$(APP_BINARY):$(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(APP_BINARY); \
	cp $(APP_BINARY) /home/linuxfun/famint-romfs/famint-romfs/rootfs/usr/bin

	

