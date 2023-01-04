## 0. 文件结构

```shell
~/Desktop/NADP3/EXP5
.
├── connectvm
├── debugvm
├── drill_exploit_nullderef.c
├── drill_exploit_uaf.c
├── drill_mod.c
├── drill_operations.c
├── drill_trigger_crash.c
├── killvm
├── linux-5.0-rc1
│   └──...
├── scptovm
├── startvm
├── v5.0-rc1.tar.gz
├── wheezy.id_rsa
├── wheezy.id_rsa.pub
└── wheezy.img
```

## 1. 内核编译

```shell
sudo apt-get install build-essential flex bison bc libelf-dev libssl-dev libncurses5-dev gcc-8
wget https://github.com/torvalds/linux/archive/v5.0-rc1.tar.gz
tar -xvf v5.0-rc1.tar.gz
cd linux-5.0-rc1
make x86_64_defconfig
make -j8 CC=gcc-8
```

![1.1-内核编译结果](https://raw.githubusercontent.com/BIIIANG/pic/main/202205162210441.png)

<div style="page-break-after:always;"></div>

## 2. `QEMU`运行编译内核结果

```shell
sudo apt-get install qemu qemu-kvm
# 使用命令行运行内核
qemu-system-x86_64 \
-kernel linux-5.0-rc1/arch/x86/boot/bzImage \
-append "console=ttyS0 root=/dev/sda debug earlyprintk=serial slub_debug=QUZ pti=off oops=panic ftrace_dump_on_oops nokaslr"\
-hda wheezy.img \
-net user,hostfwd=tcp::10021-:22 -net nic \
-nographic -m 512M -smp 2 \
-pidfile vm.pid 2>&1 | tee vm.log
# 使用脚本运行内核
sudo chmod +x startvm
./startvm
```

```shell
# 连接内核
sudo chmod +x connectvm
./connectvm
```

![2.1- QEMU运行编译内核](https://raw.githubusercontent.com/BIIIANG/pic/main/202205162210569.png)

<div style="page-break-after:always;"></div>

## 3. `drill_operations.c`

- 代码及解释见附件或附图，此处仅为运行过程及结果。

```shell
# 将文件传送到QEMU虚拟机
sudo chmod +x scptovm
./scptovm drill_operations.c
```

```shell
# 在QEMU虚拟机中编译并运行
gcc -o drill_operations drill_operations.c
./drill_operations
```

![3.1-drill_operations](https://raw.githubusercontent.com/BIIIANG/pic/main/202205162211155.png)

<div style="page-break-after:always;"></div>

## 4. `drill_exploit_uaf.c`

- 代码及解释见附件或附图，此处仅为运行过程及结果。

```shell
# 在System.map中寻找指定的地址
xubiang@ubuntu:~/Desktop/NADP3/EXP5/linux-5.0-rc1$ cat System.map | grep commit_creds
ffffffff81084370 T commit_creds
ffffffff822a9d10 r __ksymtab_commit_creds
ffffffff822be157 r __kstrtab_commit_creds
xubiang@ubuntu:~/Desktop/NADP3/EXP5/linux-5.0-rc1$ cat System.map | grep prepare_kernel_cred
ffffffff810845a0 T prepare_kernel_cred
ffffffff822afbd8 r __ksymtab_prepare_kernel_cred
ffffffff822be110 r __kstrtab_prepare_kernel_cred
# 将文件传送到QEMU虚拟机
./scptovm drill_exploit_uaf.c
```

```shell
# 在QEMU虚拟机中编译并运行
gcc -o drill_exploit_uaf drill_exploit_uaf.c
./drill_exploit_uaf
# 获取root权限后查看权限和flag
id
cat /root/flag
```

![4.1-地址](https://raw.githubusercontent.com/BIIIANG/pic/main/202205162213005.png)

![3.2-drill_exploit_uaf](https://raw.githubusercontent.com/BIIIANG/pic/main/202205162212219.png)
