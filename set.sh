#!/bin/bash

insmod one_register_modu.ko
insmod dmaDataDrv.ko
insmod storeBufferDrv.ko
# insmod mem_2_mem_demo.ko

./one_register write 0x20e0114 0x0
./one_register write 0x20e0118 0x0
./one_register write 0x20e013c 0x0
./one_register write 0x20e0140 0x0
./one_register write 0x20e0144 0x0
./one_register write 0x20e00f8 0x0
./one_register write 0x20e010c 0x0
./one_register write 0x20e0110 0x0

./one_register write 0x21b8000 0x00010089
# ./one_register write 0x21b8004 0x00001001
./one_register write 0x21b8008 0x0a020000
# ./one_register write 0x21b800c 0x00000300

insmod DmaDoppler-II.ko

sleep 1

./AscanDemo

# ./one_register write 0x20e0910 0x1
# ./one_register write 0x20e0250 0x3
# ./one_register write 0x20e0000 0x80
