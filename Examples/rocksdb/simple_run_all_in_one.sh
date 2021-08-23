#!/bin/bash

function fill(){
    sudo rm -rf $RUNFOLDER/*; sync; sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
    sudo $ROCKSDBDIR/./db_bench \
        --db=$RUNFOLDER         \
        --benchmarks=fillbatch  \
        --threads=1             \
        --key_size=16           \
        --value_size=1024       \
        --num=20000000          \
        --use_direct_io_for_flush_and_compaction=true   \
        --use_direct_reads=true \
        -batch_size=4000        \
        -compression_type=none    
}

# install dependencies

sudo apt-get install libgflags-dev make g++ gcc -y

# Set cpu to maximum frequency
ONLINE=$(cat /sys/devices/system/cpu/online)
for i in $(seq 0 $(expr "${ONLINE: -1}" - 1))
do
    echo $i
    sudo sh -c "echo $(cat /sys/devices/system/cpu/cpu$i/cpufreq/cpuinfo_max_freq) > /sys/devices/system/cpu/cpu$i/cpufreq/scaling_min_freq"
done


# Download rocksdb and build db_bench
ROCKSDBDIR=$HOME/rocksdb/rocksdb-6.22.1

if [ -d $ROCKSDBDIR ]; then
    echo "rocksdb folder already exist"
else
    mkdir -p $ROCKSDBDIR
    cd $HOME/rocksdb;wget https://github.com/facebook/rocksdb/archive/refs/tags/v6.22.1.tar.gz
    tar -xvf v6.22.1.tar.gz
    cd $ROCKSDBDIR;make -j${ONLINE: -1} db_bench
fi

if [ -e $ROCKSDBDIR/db_bench ]; then
    echo "using db_bench"
else
    echo "please remove the $ROCKSDBDIR and re-run this script"
    exit 1
fi

# make result folder
rm -rf results
mkdir results/

RUNFOLDER=$HOME/run_rocksdb
sudo rm -rf $RUNFOLDER
sudo mkdir -p $RUNFOLDER


fill
echo "starting readwhilewriting"

sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
sleep 3
sudo $ROCKSDBDIR/./db_bench \
    --db=$RUNFOLDER         \
    --benchmarks=readwhilewriting \
    --threads=3             \
    -duration=30            \
    --key_size=16           \
    --value_size=1024       \
    -use_existing_db=true   \
    --disable_auto_compactions=true \
    --use_direct_io_for_flush_and_compaction=true \
    --use_direct_reads=true \
    -compression_type=none  \
    -statistics > results/readwhilewriting_default

sleep 10

fill
echo "starting readrandom"
sudo sh -c "echo 3 > /proc/sys/vm/drop_caches"
sudo $ROCKSDBDIR/./db_bench \
    --db=$RUNFOLDER         \
    --benchmarks=readrandom \
    --threads=4             \
    --key_size=16           \
    --value_size=1024       \
    --reads=100000          \
    -use_existing_db=true   \
    --disable_auto_compactions=true \
    --use_direct_io_for_flush_and_compaction=true \
    --use_direct_reads=true \
    -compression_type=none -statistics > results/readrandom_default

