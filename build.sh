#!/bin/bash

# 使imx6的开发环境生效
source /opt/yogurt/i.MX6-PD15.3.0/environment-setup-cortexa9hf-vfp-neon-phytec-linux-gnueabi

make

# scp DmaDoppler-II.ko root@192.168.1.11:/opt/mercury/driver
