obj-m += nRF24L01.o
MODULE_PATH := $(shell pwd)
KERNEL_PATH := /home/book/linux-2.6.38

all:
	make -C $(KERNEL_PATH) M=$(MODULE_PATH) modules
clean:
	rm -rf .*.cmd *.o *.mod.c *.ko .tmp_versions Module.symvers .Makefile.swp Module.markers modules.order


