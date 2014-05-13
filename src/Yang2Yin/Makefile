CC = arm-linux-gcc
all:
	$(CC) -fPIC -c Yang2Yin.c
	$(CC) -shared -o libYang2Yin.so Yang2Yin.o
	$(CC) -o testYang2Yin test.c -L. -lYang2Yin
clean:
	-rm -rf libYang2Yin.so Yang2Yin.o
