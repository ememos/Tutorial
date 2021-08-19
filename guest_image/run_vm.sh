#!/bin/bash

VM_IMAGE="bionic-server-cloudimg-amd64.img"
PW_IMAGE="user-data.img"

# Default NR_CPU is number of physical cpu
NR_CPU=$(grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}')

# Default MEM_SIZE is half of total memory(MB).
MEM_SIZE=$(grep MemTotal /proc/meminfo | awk '{print $2}')
MEM_SIZE=$(( MEM_SIZE/2048 ))
GUI_MODE=false

function usage
{
    echo "usage:  [-v VM_IMAGE -c CPU_NUMBER -m MEMORY_SIZE -g GUI_MODE]"
    echo "   ";
    echo "  -v | --vm_image          : Disk image used for booting";
    echo "  -c | --cpu_nr            : Number of vCPUs";
    echo "  -m | --mem_size          : VM's total memory size";
    echo "  -g | --gui_mode          : Using this option makes gui mode. Without this, it will be cli";
    echo "  -h | --help              : Show usage";
}

function parse_args
{
  # named args
  while [ "$1" != "" ]; do
      echo "$1"
      case "$1" in
          -v | --vm_image )             VM_IMAGE="$2";           shift;;
          -c | --cpu_nr   )             NR_CPU="$2";             shift;;
          -m | --mem_size )             MEM_SIZE="$2";           shift;;
          -g | --gui_mode )             GUI_MODE=true;           shift;;
          -h | --help )                 usage;                   exit;;
      esac
      shift # move to next kv pair
  done
}

parse_args "$@"
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
