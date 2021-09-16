#!/bin/bash

VM_IMAGE="bionic-server-cloudimg-amd64.img"
PW_IMAGE="user-data.img"

# Default NR_CPU is number of physical cpu
NR_CPU=$(expr `nproc` / 2)

# Default MEM_SIZE is half of total memory(MB).
MEM_SIZE=$(grep MemTotal /proc/meminfo | awk '{print $2}')
MEM_SIZE=$(( MEM_SIZE/2048 ))
GUI_MODE=false
NUMA_MODE=false
START_VCPU="0"
IPLIST="10.10.20.14 10.10.20.16"

function usage
{
    echo "usage:  [-v VM_IMAGE -c CPU_NUMBER -m MEMORY_SIZE -g GUI_MODE]"
    echo "   ";
    echo "  -v | --vm_image          : Disk image used for booting";
    echo "  -c | --cpu_nr            : Number of vCPUs";
    echo "  -n | --numa              : Using NUMA mode";
    echo "  -m | --mem_size          : VM's total memory size";
    echo "  -g | --gui_mode          : Using this option makes gui mode. Without this, it will be cli";
    echo "  -s | --start             : Start number of vCPU, range between -local-cpu";
    echo "  -i | --iplist            : IP list connected between nodes";
    echo "  -h | --help              : Show usage";
}

function parse_args
{
  # named args
  while [ "$1" != "" ]; do
      case "$1" in
          -v | --vm_image )             VM_IMAGE="$2";           shift;;
          -c | --cpu_nr   )             NR_CPU="$2";             shift;;
          -n | --numa     )             NUMA_MODE=true;          shift;;
          -m | --mem_size )             MEM_SIZE="$2";           shift;;
          -g | --gui_mode )             GUI_MODE=true;           shift;;
          -s | --start    )             START_VCPU="$2";         shift;;
          -i | --iplist   )             IPLIST="$2";             shift;;
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

if [ "$NUMA_MODE" = true ]; then
    COMMAND+="-object memory-backend-ram,size=$NODE_SIZE,id=ram0 "
    COMMAND+="-numa node,nodeid=0,cpus=0-$((NR_CPU/2 - 1)),memdev=ram0 "
    COMMAND+="-object memory-backend-ram,size=$NODE_SIZE,id=ram1 "
    COMMAND+="-numa node,nodeid=1,cpus=$((NR_CPU/2))-$((NR_CPU-1)),memdev=ram1 "
    
    COMMAND+="-vcpu pin-all=on "
    COMMAND+="-device e1000,netdev=net0 "
    COMMAND+="-netdev user,id=net0,hostfwd=tcp::5556-:22 "

    sudo $COMMAND
else
    if [ "$START_VCPU" == "0" ]; then
        COMMAND+="-redir tcp:5556::22 "
    fi
    #FIXME workaround
    sudo $COMMAND -local-cpu $((NR_CPU/2)),start=${START_VCPU},iplist="${IPLIST}"
fi

# debug
#echo $COMMAND
#sudo $COMMAND
