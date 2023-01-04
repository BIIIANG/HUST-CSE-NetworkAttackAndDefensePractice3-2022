# 1. 寻找漏洞位置

- 使用`IDA`找到程序入口：

![1.1-程序入口](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112053183.png)

- 若输入`1`或`2`，则跳转到函数`sub_40100A`，因此跟踪该函数：

![1.2-sub_40100A](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112053642.png)

<div STYLE="page-break-after: always;"></div>

- 函数`sub_40100A`跳转到函数`sub_401400`，跟踪该函数；函数`sub_401400`的上半部分是正常的，完成了一些输出与输入操作：

![1.4-sub_401400](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112054694.png)

- 函数`sub_401400`的下半部分存在很多未能正确解析的数据：

![1.5-函数sub_401400存在花指令](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112054017.png)

<div STYLE="page-break-after: always;"></div>

- 在`Options->General->Number of opcode bytes`中将操作码字节设为`8`，此时`IDA`的显示如下；为了使代码恢复正常，使用`IDA`的`Code`和`Data`选项将指定的字节分别解析成代码和数据：

![1.6-Code和Data操作](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112054688.png)

- 观察正常代码，在字符串地址之后接的可能是一个`call`指令以完成输入和输出，且`call`对应的操作码为`E8`，因此使`E8`在一条指令的最前面并且从其开始将其解析为代码：

![1.7-恢复1](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112055513.png)

<div STYLE="page-break-after: always;"></div>

- 继续观察正常代码，发现在`call`之前，应该将参数入栈，而`push`指令对应的操作码为`68`，正好也与字符串地址前的一个字节相符，因此使`68`在一条指令的最前面并且从其开始将其解析为代码：

![1.8-恢复2](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112055986.png)

- 然后将`C7`开头的指令解析为代码，表示一个`MOV`语句：

![1.9-结果](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112055463.png)

<div STYLE="page-break-after: always;"></div>

- `0x004014B2`处的指令显然不正常，因此将`E9`作为指令码的开始，得到第一个花指令`E8`：

![1.10-花指令1](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112057020.png)

- 另一个花指令`E9`：

![1.11-花指令2](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112058339.png)

<div STYLE="page-break-after: always;"></div>

- 恢复代码后，选择`sub_401400`整个函数（到`retn`）并按`P`将其解析为一个函数，然后反编译得到以下结果：

![1.10-sub_401400反汇编](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112058938.png)

- 关注最后的`LOBYTE(v1) = v5`，与上次实验中发生的整数溢出相同：

![1.11-存在整数溢出](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112058878.png)

<div STYLE="page-break-after: always;"></div>

- 继续追踪最后一个函数`sub_401005`：

![1.12-sub_401005](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112058093.png)

- 函数`sub_401005`跳转到函数`sub_401600`，跟踪该函数，其操作与上次实验一致，因此仍构建长度为`256 + 5 = 261`的字符串进行攻击：

![1.13-sub_401600](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112059277.png)

<div STYLE="page-break-after: always;"></div>

# 2. 寻找输出正确信息的函数地址

- 在字符表中尝试搜索`Success!`，但没有搜索到，因此由`Flag:`字符串作为切入点：

![2.1-寻找字符串flag](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112111128.png)

- 跟踪引用了字符串`Flag:`的函数：

![2.2-找到使用该字符串的函数](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112111371.png)

<div STYLE="page-break-after: always;"></div>

- 可以看到函数`sub_4012E0`即为输出`flag`信息的函数：

![2.3-输出FLAG并对输入用户名做计算的函数](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112111551.png)

- 为了找到输出所有信息的函数，我们根据引用，向上查找调用`sub_4012E0`的函数为`sub_401019`：

![2.4-向上层寻找](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112111370.png)

<div STYLE="page-break-after: always;"></div>

- 根据引用继续向上查找调用`sub_401019`的函数：

![2.5-再向上层寻找](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112111999.png)

- 该函数无法直接反编译：

![2.6-直接反编译失败](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112859.png)

<div STYLE="page-break-after: always;"></div>

- 选中后使用`P`将其解析为一个函数：

![2.7-手动将其设置为一个函数](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112404.png)

- 再进行反编译如下，初步猜测该函数`sub_401230`就是输出所有正确信息的函数：

![2.9-反编译](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112201.png)

<div STYLE="page-break-after: always;"></div>

- **分析函数`sub_401230`调用的第一个函数`sub_401023`，其内部继续调用了`sub_4011B0`：**

![2.10-追踪sub_401023](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112284.png)

- 跟踪`sub_4011B0`，其定义了变量`v2`和`v3`，并以`v2`和`9`为参数调用了`sub_40100F`函数：

![2.11-追踪](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112191.png)

<div STYLE="page-break-after: always;"></div>

- `sub_40100F`调用了`sub_401390`：

![2.12-追踪sub_40100F](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112254.png)

- `sub_401390`的反编译结果如下，其将`a1`作为一个长度为`9`的字节序列，并对每个字节与`0x1C`进行了异或运算：

![2.13-sub——401390对输入的字节序列做了异或运算](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112112502.png)

<div STYLE="page-break-after: always;"></div>

- 因此猜测`sub_4011B0`中的`v2`和`v3`实际为字符串，为了混淆才将其定义为了整数，将其解析为字符串如下，猜想正确：

![2.14-推测V2V3为字符串，9为长度，将其解析为字符串，可见的确为success!](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112113858.png)

- `sub_4011B0`最后将处理过的字符串复制到自己的参数`a1`中，在`sub_401230`中看即为复制到了`v0`中：

![2.9-反编译](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112155487.png)

<div STYLE="page-break-after: always;"></div>

- **分析函数`sub_401230`调用的第二个函数`sub_40100F`，其内部继续调用了`sub_401390`：**

![2.15-追踪sub_40100F](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112113435.png)

- 跟踪`sub_401390`，该函数将函数`sub_401230`调用的第一个函数`sub_401023`中得到的操作后的`Success!`字符串的每个字节再与`0x1C`进行了异或运算，因此恢复了该字符串，并在`sub_401230`进行了输出：

![2.16-追踪sub_401390，其将字符串恢复](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112114316.png)

<div STYLE="page-break-after: always;"></div>

- **分析函数`sub_401230`调用的第三个函数`sub_401019`，其内部继续调用了`sub_4012E0`：**

![2.17-追踪sub_401019](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112114725.png)

- `sub_4012E0`即为输出输出`flag`信息的函数：

![2.18-追踪sub_4012E0](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112115970.png)

<div STYLE="page-break-after: always;"></div>

# 3. 调试与攻击

- 选择调试工具为`Remote Windows debugger`：

![3.1-选择调试程序](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112115711.png)

- 在缓冲区溢出的`strcpy`函数处设置断点：

![3.2-断点](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112115910.png)

<div STYLE="page-break-after: always;"></div>

- 开启`IDA`提供的调试服务器`win32_remote.exe`并在`IDA`中连接：

![3.3-启动dbgsrv并且连接](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116445.png)

- 生成随机字符串文件`xba.txt`，以寻找返回地址在溢出中的位置：

![3.4-寻找覆盖位置使用的随机字符串](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116715.png)

<div STYLE="page-break-after: always;"></div>

- 开启调试，输入账户`U201911803`和文件`xba.txt`：

![3.5-输入用户名和文件名](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116395.png)

- 在`strcpy`执行前，查看`EBP`指向栈中位置的数据，可以找到返回地址存储在`0x0019FC58`处：

![3.6-strcpy前](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116576.png)

<div STYLE="page-break-after: always;"></div>

- 执行`strcpy`后，返回地址被覆盖为`0x4E455A48`：

![3.7-strcpy后](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116335.png)

- 在`retn`后，`EIP`的值即为`0x4E455A48`：

![3.8-返回后，EIP的值为0x...，只需将此换为输出正确信息和flag的函数入口即可](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116553.png)

<div STYLE="page-break-after: always;"></div>

- 因此在`xba.txt`中寻找`0x485A454E`：

![3.9-找到位置](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116882.png)

- 将其更改为输出正确信息的函数地址`sub_0x00401230`：

![3.10-修改后的攻击文件](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116846.png)

<div STYLE="page-break-after: always;"></div>

- 使用构造的攻击文件进行测试，攻击成功，输出`Success!`及由`account`计算得到的`FLAG`信息如下：

![3.11-攻击结果](https://raw.githubusercontent.com/BIIIANG/pic/main/202205112116263.png)