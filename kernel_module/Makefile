obj-m += vmstat_ioctl.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install: all
	@if lsmod | grep -q vmstat_ioctl; then \
	    echo "Removing existing module vmstat_ioctl..."; \
	    sudo rmmod vmstat_ioctl; \
	fi
	sudo insmod vmstat_ioctl.ko
	@if [ -e /proc/devices ]; then \
	    major=$$(awk '$$2=="vmstat_ioctl" {print $$1; exit}' /proc/devices); \
	    if [ -n "$$major" ]; then \
	        sudo mknod /dev/vmstat_ioctl c $$major 0; \
	        echo "Device /dev/vmstat_ioctl created with major number $$major"; \
	    else \
	        echo "Error: Could not find major number for vmstat_ioctl in /proc/devices"; \
	        sudo rmmod vmstat_ioctl; \
	        exit 1; \
	    fi \
	else \
	    echo "Error: /proc/devices not available"; \
	    sudo rmmod vmstat_ioctl; \
	    exit 1; \
	fi


uninstall:
	sudo rmmod vmstat_ioctl
	sudo rm -f /dev/vmstat_ioctl
