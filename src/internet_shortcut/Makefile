APP_NAME = internet_shortcut

all:
	arm-linux-gcc -o $(APP_NAME) internet_shortcut.c -I.
	cp $(APP_NAME) /work/nfs_root/rootfs/usr/bin
clean:
	rm -rf $(APP_NAME)