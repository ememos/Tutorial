#!/bin/bash 

VM_IMAGE=/home/baiksong/Work/giantvm/ubuntu1804-20G.img

sudo setfacl -m "u:baiksong:rw" /dev/kvm
COMMAND="qemu-system-x86_64 "
COMMAND+="-enable-kvm "
COMMAND+="-hda ${VM_IMAGE} "
COMMAND+="-cpu host,-kvm-asyncpf -machine kernel-irqchip=off "
COMMAND+="-smp 8 -m $((8*1024)) -serial mon:stdio "
COMMAND+="-monitor telnet:127.0.0.1:1234,server,nowait "

COMMAND+="-object memory-backend-ram,size=4G,id=ram0 "
COMMAND+="-numa node,nodeid=0,cpus=0-3,memdev=ram0 "
COMMAND+="-object memory-backend-ram,size=4G,id=ram1 "
COMMAND+="-numa node,nodeid=1,cpus=4-7,memdev=ram1 "

COMMAND+="-device e1000,netdev=net0 "
COMMAND+="-netdev user,id=net0,hostfwd=tcp::5556-:22 "

sudo $COMMAND
