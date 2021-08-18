# MemOS NUMA environment setting

### Introduction
Set up a NUMA node on a Single VM (QEMU) instance and emulate virtual NUMA environments at the guest level.
- performance-level emulation is not supported.
- NUMA emulation that uses two virtual machine instances will be guided soon.

### Requirement
- Baremetal Linux machine

&ensp;&ensp;&ensp; or
- Linux on the virtual machine (ex. virtualbox)
  - Check if nested virtualization is supported (virtualbox -> QEMU/KVM)
  - ⚠️ No guarantee that it will work.
 
### Recommendation
- Ubuntu 18.04 LTS
- Multicore CPU

## Installation and Run
### Install Ubuntu distributions
1. Install the `qemu-kvm` package
```bash
sudo apt install qemu-kvm
```
2. Download pre-built image corresponding to your machine's architecure
3. Resize the image to 20G
```bash
# Install Image
wget http://cloud-images.ubuntu.com/bionic/current/bionic-server-cloudimg-amd64.img

# Reszie Image to 20G
qemu-img resize bionic-server-cloudimg-amd64.img 20G
```
3. Install the `cloud-image-utils` package
4. Change password and make `user-data.img`
```bash
# Install package
sudo apt-get install cloud-image-utils
cat >user-data <<EOF
#cloud-config
password: asdfqwer
chpasswd: { expire: False }
ssh_pwauth: True
EOF

# Make text file to raw image
cloud-localds user-data.img user-data
```
### Running the QEMU
1. Download the `run_vm.sh` script.
```bash
wget https://raw.githubusercontent.com/ememos/Tutorial/master/guest_image/run_vm.sh
```
In the `run_vm.sh`, you can see some variables.
- `NR_CPU`: you can modify number of vCPUs
- `MEM_SIZE`: Total memory size
- `GUI_MODE`: Deternmin GUI/CLI mode
```bash
NR_CPU=8
MEM_SIZE=8192
GUI_MODE=false
```

2. After the modification, run the `run_vm.sh`
```bash
chmod u+x run_vm.sh
./run_vm.sh &
```

3. Using the ssh connect to guest. Guest's default user name is **ubuntu**
```bash
ssh -p 5556 ubuntu@localhost
```

4. Install `numactl` package
```bash
sudo apt install numactl
```
5. Check NUMA configuration. Result will be like this.
```bash
$ numactl --hardware
available: 2 nodes (0-1)
node 0 cpus: 0 1 2 3
node 0 size: 3945 MB
node 0 free: 3747 MB
node 1 cpus: 4 5 6 7
node 1 size: 4030 MB
node 1 free: 3811 MB
node distances:
node   0   1
  0:  10  20
  1:  20  10
```

## Running the examples

1. Connect to NUMA guest
```bash
ssh -p 5556 ubuntu@localhost
```
2. Install `make` `build-essential` `libnuma-dev` packages
```bash
sudo apt install make build-essential libnuma-dev
```

3. Download the examles and build
```bash
git clone https://github.com/ememos/Tutorial.git
cd Tutorial/Examples/memcpy
make
```

4. Run the benchmark
```bash
$ ./memcpy
Initialized random source 268435456 bytes in 0.203772 sec.
Source generation rate 1.317330 GB/sec.

Starting 8 threads
[INFO] ROI begins
[INFO] ROI ends
Threads stopped

Copied source in 8 threads in 0.212899 sec.
Achieving parallel memory read-write bandwidth: 20.173702 GB/sec.
RESULT: [memcpy] #thread: 8 time(s):   0.212899 arg: 33554432
```
```bash
$ ./memcpypre
Initialized random source 268435456 bytes in 0.199358 sec.
Source generation rate 1.346502 GB/sec.

Starting 8 threads
[INFO] ROI begins
[INFO] ROI ends
Threads stopped

Copied source in 8 threads in 0.047389 sec.
Achieving parallel memory read-write bandwidth: 90.632679 GB/sec.
RESULT: [memcpypre] #thread: 8 time(s):   0.047389 arg: 33554432
```
