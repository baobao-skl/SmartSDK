CC		=	arm-linux-gcc
HOST	= arm-linux

CFLAGS += -O2 -DLINUX -D_GNU_SOURCE -Wall

APP_BINARY	=	ntp_test

OBJECTS			+=	ntp_test.o

all:$(APP_BINARY)

clean:
	-rm -rf *.o config.log $(APP_BINARY)
	
$(APP_BINARY):$(OBJECTS)
	$(CC) $(CFLAGS) $(LFLAGS) $(OBJECTS) -o $(APP_BINARY)

	

