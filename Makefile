#obj-m += hello.o

# obj-m	+= procfs1.o
# obj-m	+=	procfs2.o
# obj-m	+=	game.o
# obj-m	+=	testOpen.c
obj-m += proc_rw.o
obj-m += test.o
all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean :
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
