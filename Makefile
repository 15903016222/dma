obj-m +=DmaDoppler-II.o
obj-m +=one_register_modu.o

all:
	make -C /home/ws/project/phascan-II/linux SUBDIRS=$(PWD) modules
clean:
	make -C /home/ws/project/phascan-II/linux SUBDIRS=$(PWD) clean 
