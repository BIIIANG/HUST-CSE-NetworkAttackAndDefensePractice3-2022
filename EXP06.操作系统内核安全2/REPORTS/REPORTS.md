## 1. `drill_trigger_crash.c`

- 启动（`startvm`）、连接虚拟机（`connectvm`）并传送（`scptovm`）、编译`C`语言文件同上次实验，此处不再赘述。
- 代码及解释见附件或附图，此处仅为运行结果。
- 使用`drill_trigger_crash.c`成功触发空指针引用漏洞如下：

![1.drill_trigger_crash.c](https://raw.githubusercontent.com/BIIIANG/pic/main/202205202054138.png)

<div style="page-break-after:always;"></div>

## 2. `CVE-2019-9213 reproducer`

- 使用`drill_reproducer.c`成功触发用户空间零地址映射漏洞如下：

![2.drill_reproducer.c](https://raw.githubusercontent.com/BIIIANG/pic/main/202205202056881.png)

<div style="page-break-after:always;"></div>

## 3. `drill_exploit_nullderef.c`

- 使用`drill_reproducer.c`成功完成提权并查看放在/root/flag文件中的内容如下：

![3.drill_exploit_nullderef.c](https://raw.githubusercontent.com/BIIIANG/pic/main/202205202058424.png)