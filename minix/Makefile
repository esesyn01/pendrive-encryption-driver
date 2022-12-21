obj-m += minix.o

minix-objs := bitmap.o itree_v1.o itree_v2.o namei.o inode.o file.o dir.o

EXTRA_CFLAGS += -Wall -g -Wno-unused -Wno-unused-function -Wno-unused-label -Wno-unused-variable

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
