APP_BINARY = net_manager
all:
	arm-linux-gcc -o $(APP_BINARY) main.c mac_helper.c -lpthread -I.
	cp $(APP_BINARY) /home/linuxfun/famint-romfs/famint-romfs/rootfs/usr/bin
clean:
	rm -rf $(APP_BINARY)	
