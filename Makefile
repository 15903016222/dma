obj-m +=mxc_sdma_memcopy_test.o
obj-m +=mxc_sdma_mem_test.o
all:
	make -C /home/ws/kernel/linux SUBDIRS=$(PWD) modules
clean:
	make -C /home/ws/kernel/linux SUBDIRS=$(PWD) clean 
