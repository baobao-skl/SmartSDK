obj-m += leddriver.o
MODULE_PATH := $(shell pwd)
KERNEL_PATH := /home/linuxfun/famint-kernel/linux-2.6.38-old/linux-2.6.38

all:
	make -C $(KERNEL_PATH) M=$(MODULE_PATH) modules
	cp leddriver.ko /home/linuxfun/famint-romfs/famint-romfs/rootfs/driver
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.symvers .Makefile.swp Module.markers modules.order


