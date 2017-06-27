#!/bin/bash

# insmod eim_register_set.ko
insmod one_register_modu.ko
insmod mem_2_mem_demo.ko

./one_register write 0x21b8000 0x0061108d
./one_register write 0x21b8004 0x00001001
./one_register write 0x21b8008 0x01002000
./one_register write 0x21b800c 0x00000300

./one_register write 0x20e0114 0x0
./one_register write 0x20e0118 0x0
./one_register write 0x20e013c 0x0
./one_register write 0x20e0140 0x0
./one_register write 0x20e0144 0x0
./one_register write 0x20e00f8 0x0
./one_register write 0x20e010c 0x0
./one_register write 0x20e0110 0x0
