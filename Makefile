obj-m += virtual_device.o

all: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

load:
	sudo insmod virtual_device.ko
	sudo chmod 666 /dev/virtual_device0

unload:
	sudo rmmod virtual_device

reload: unload load