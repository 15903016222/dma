#!/bin/bash

insmod one_register_modu.ko

# config EIM PAD mux funtion
./one_register write 0x20e0114 0x0
./one_register write 0x20e0118 0x0
./one_register write 0x20e013c 0x0
./one_register write 0x20e0140 0x0
./one_register write 0x20e0144 0x0
./one_register write 0x20e00f8 0x0

# confi EIM register
./one_register write 0x21b8000 0x00010089
./one_register write 0x21b8008 0x0a020000

insmod DmaDoppler-II.ko

sleep 1
