#!/bin/bash

VM_IMAGE="bionic-server-cloudimg-amd64.img"
PW_IMAGE="user-data.img"
NR_CPU=8
MEM_SIZE=8192
GUI_MODE=false

NODE_SIZE="$((MEM_SIZE/2))M"

sudo setfacl -m "u:$(whoami):rw" /dev/kvm
COMMAND="qemu-system-x86_64 "
COMMAND+="-enable-kvm "
COMMAND+="-hda ${VM_IMAGE} -hdb $PW_IMAGE "
COMMAND+="-cpu host,-kvm-asyncpf -machine kernel-irqchip=off "
COMMAND+="-smp $NR_CPU -m $MEM_SIZE "
if [ "$GUI_MODE" = false ]; then
    COMMAND+="-nographic -serial mon:stdio "
fi
COMMAND+="-monitor telnet:127.0.0.1:1234,server,nowait "

COMMAND+="-object memory-backend-ram,size=$NODE_SIZE,id=ram0 "
COMMAND+="-numa node,nodeid=0,cpus=0-$((NR_CPU/2 - 1)),memdev=ram0 "
COMMAND+="-object memory-backend-ram,size=$NODE_SIZE,id=ram1 "
COMMAND+="-numa node,nodeid=1,cpus=$((NR_CPU/2))-$((NR_CPU-1)),memdev=ram1 "

COMMAND+="-device e1000,netdev=net0 "
COMMAND+="-netdev user,id=net0,hostfwd=tcp::5556-:22 "

$COMMAND
