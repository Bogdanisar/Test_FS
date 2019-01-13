# obj-m += fs.o
# obj-m += pnocpu.o

obj-m += test_fs.o
test_fs-objs := fs.o proc.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean