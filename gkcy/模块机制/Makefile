obj-m := module1.o #obj-m: 编译成可动态加载的module，通过insmod动态重定位装入到内核  obj-y: 静态编译链接进内核，在系统启动过程中进行初始化
KDIR:= /lib/modules/$(shell uname -r)/build
PWD:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
