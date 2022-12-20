obj-m += encryption.o

EXTRA_CFLAGS += -Wall -g -Wno-unused -Wno-unused-function -Wno-unused-label -Wno-unused-variable

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
