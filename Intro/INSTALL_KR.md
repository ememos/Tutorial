# GiantVM Installation Guide

다음의 내용은 infiniband 네트워크로 연결된 2대의 real 머신(node0, node1)을 GiantVM으로 묶어서 하나의 가상 머신으로 구동하기 위한 절차입니다.

> :warning: VirtualBox로 만든 Linux 가상머신에서 QEMU/KVM기반으로 또 다른 가상머신을 기동하는 것은 현재 VirtualBox가 지원하지 않습니다.


## 준비물
### H/W
- 인피니밴드(infiniband) 네트워크로 연결된 2대의 real 머신 (node0, node1)

### S/W
- GiantVM 패치 소스 2종 (참조: https://github.com/ememos/GiantVM)
  - Linux-DSM-4.18 (GiantVM KVM patch for kernel 4.18)
  - QEMU-gvm-vcpupin (GiantVM QEMU patch for vcpu pinning)
- Guest OS 기동용 스크립트([run_vm.sh](https://github.com/ememos/Tutorial/blob/master/guest_image/run_vm.sh))

## Installation

### GiantVM 패치 소스 2종 준비
```bash
git clone https://github.com/ememos/GiantVM.git
```
### 호스트용 커널 설치 (Linux-DSM-4.18)
2개의 머신(node0, node1)에 아래의 과정을 각각 진행합니다.

#### 1. 커널 컴파일 준비
```bash
sudo apt-get install -y build-essential libncurses5-dev gcc libssl-dev grub2 bc
sudo apt-get install -y flex bison net-tools libelf-dev
```
#### 2. 호스트용 커널 컴파일 (Linux-DSM-4.18)
```bash
cd GiantVM
cd Linux-DSM-4.18

# EXTRAVERSION = -gvm 확인 또는 명칭 수정
vi Makefile 

# Config 파일 설정 및 debian 패키지로 빌드
cp ../config-4.18.20-gvm .config
sudo make oldconfig
sudo make deb-pkg -j 10 LOCALVERSION=1
```
> ⚠️ Recommend to use `gcc-7`
#### 3. 커널 설치 및 grub 업데이트
```bash
cd ..
sudo dpkg -i linux-image-4.18.20-gvm1_4.18.20-gvm1-1_amd64.deb
sudo dpkg -i linux-libc-dev_4.18.20-gvm1-1_amd64.deb
sudo dpkg -i linux-headers-4.18.20-gvm1_4.18.20-gvm1-1_amd64.deb
sudo dpkg -i linux-image-4.18.20-gvm1-dbg_4.18.20-gvm1-1_amd64.deb
```
#### 4. Setting defualt kernel as Linux-DSM (optional)
GiantVM의 entry 인덱스를 확인합니다.
```bash
$ grep 'menuentry ' /boot/grub/grub.cfg | cut -f 2 -d "'" | nl -v 0
0  Ubuntu
1  Ubuntu, with Linux 4.18.20-gvm1
2  Ubuntu, with Linux 4.18.20-gvm1 (recovery mode)
3  Ubuntu, with Linux 4.15.0-154-generic
4  Ubuntu, with Linux 4.15.0-154-generic (recovery mode)
```
editor을 통해 GRUG_DEFAULT의 값을 GiantVM의 인덱스로 변경합니다.
```bash
sudo vi /etc/default/grub

GRUB_DEFAULT=1
```
grub을 업데이트합니다.
```bash
sudo update-grub
```
#### 5. Reboot
```bash
sudo reboot
```
### 호스트용 QEMU-gvm-vcpupin 빌드 및 설치
2개의 머신(node0, node1)에 아래의 과정을 각각 진행합니다. 

#### 1. 필요한 package 설치
```bash
sudo apt install python pkg-install
sudo apt-get install git libglib2.0-dev libfdt-dev libpixman-1-dev zlib1g-dev
sudo apt-get install libnuma-dev
```
#### 2. kvm.h 파일 수정
```bash
cd QEMU-gvm-vcpupin
vi linux-headers/linux/kvm.h
```
```diff
@@ -873,7 +873,7 
-#define KVM_CAP_X86_DSM 133
+#define KVM_CAP_X86_DSM 156
```
#### 3. 빌드 및 설치
```bash
sudo ./configure --target-list=x86_64-softmmu --enable-kvm --disable-werror
sudo make -j 10
sudo make install
```
여기까지 진행하면 GiantVM을 실행할 수 있는 준비가 되었습니다.

## Guest OS 기동
아래의 예제는 vcpu를 total 8개 구성하고, node0 머신에서는 0-3번 vcpu를 기동, node1 머신에서는 4-7번 vcpu를 기동하는 절차입니다. 
- **node0**: vcpu[0:4]
- **node1**: vcpu[4:8]

### guest OS 준비
각각의 머신에서 아래의 과정을 수행하여 Guest OS 이미지를 준비합니다.
1. Download pre-built image corresponding to your machine's architecure
2. Resize the image to 20G
```bash
# Install Image
wget http://cloud-images.ubuntu.com/bionic/current/bionic-server-cloudimg-amd64.img

# Reszie Image to 20G
qemu-img resize bionic-server-cloudimg-amd64.img 20G
```
3. Install the `cloud-image-utils` package
4. Change password and make `user-data.img`. In this example, id is `ubuntu` and password is `asdfqwer` 
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

### node0, node1 머신에서 기동 스크립트 실행
각각의 머신에서 실행 스크립트를 다운로드 받습니다.
```bash
wget https://raw.githubusercontent.com/ememos/Tutorial/master/guest_image/run_vm.sh
```
node 0에서 실행 스크립트 실행합니다.
```bash
node0$ ./run_vm.sh -c 8 -m 8192 -s 0 -i "10.10.20.53 10.10.20.54"
```
node 1에서 실행 스크립트 실행합니다.
```bash
node1$ ./run_vm.sh -c 8 -m 8192 -s 4 -i "10.10.20.53 10.10.20.54"
```
> ⚠️ Guest OS 이미지가 위치한 경로에서 실행해야 합니다.

실행 스크립트의 DSM 관련 옵션은 아래와 같습니다.
- `-s`: Local vcpu's start index
- `-i`: infiniband ip list

위의 절차를 실행하면, node0 머신에서 guest OS의 로그인 프롬프트가 출력되며, 로그인하고 나서 일반적인 리눅스 사용하듯이 명령어 및 벤치마크 S/W 등을 실행할 수 있습니다.

