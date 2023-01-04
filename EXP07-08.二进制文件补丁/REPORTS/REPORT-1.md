# 1. 冷补丁：`overflow`

## 1.1 实验要求

程序`overflow`实现了一个非常简单的用户交互：输入学号，若输入的学号为`10`个字符，则在屏幕上打印一段感谢和表扬的话。程序共包含一个逻辑缺陷和一个栈溢出漏洞，要求在没有源代码的情况下对其进行修补，以满足：

- 逻辑漏洞修补后，程序仅打印与自己性别相对应的话；
- 栈溢出漏洞修补后，无论输入多长的字符串均不会导致程序崩溃。

## 1.2 实验过程

### 1.2.1 代码分析

- 首先使用`IDA`打开`overflow`程序，其`main`函数的反汇编结果如下：

![1-1](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062245949.png)

- 将`main`函数反编译，结果如下：

![1-2](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062245828.png)

- 结合以上反汇编和反编译结果，可以更加清晰地得到其执行逻辑：首先输出提示字符串，然后使用`gets`函数将用户的输入读取到字符数组`s`中，若读入字符串的长度为`10`，则输出两条信息。与该程序的目标逻辑做对比，可知该程序存在的逻辑漏洞为输入学号后两条信息均被输出，即存在跳转的逻辑错误；存在的栈溢出漏洞为在`gets`函数读取用户输入时，没有对用户输入做限制，用户输入的字符串过长将导致缓冲区溢出，情况严重时会导致程序崩溃。

### 1.2.2 漏洞测试与分析

- 首先正常输入自己的学号，输出如下，错误地将两条提示信息全部输出，因此程序存在逻辑漏洞：

![1-3-1](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062249026.png)

- 然后输入一个`12`个字符的字符串，结果如下，此时虽然栈缓冲区发生了溢出，但没有造成严重的后果，程序正常结束：

![1-3-2](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062249440.png)

- 最后输入一个较长（大于等于`19`个字符）的字符串，结果如下，可见栈缓冲区溢出造成了段错误（或总线错误），导致了程序崩溃的严重后果，若精心构造输入的字符串，还存在其他利用栈缓冲区溢出漏洞进行执行流劫持的攻击手段，因此程序存在栈缓冲区溢出漏洞：

![1-3-3](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062249856.png)

- 使用`IDA`观察`main`函数的栈分布可知，供字符数组`s`使用的栈空间为`11`个字节，即恰好能满足`10`个字符的存储需求；接下来的`8`个字节空间用于保存寄存器的值，若该部分被覆盖，在这个简单程序中不会造成严重的后果；再接下来的`8`个字节用于存放返回地址，若该部分被覆盖且覆盖后的地址不合法，则会导致段错误等问题导致程序崩溃，若输入的字符串被精心构造，则可能导致执行流被劫持，造成更严重的后果。

![1-4](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062250078.png)

### 1.2.3 漏洞修补

#### 1.2.3.1 逻辑漏洞

为了仅保留两条输出语句中的第一句，可以将不需要的语句输出所对应的`lea rdi, aThankYouYouAre_0`和`call _puts`指令修改为`jmp`指令，跳转到这两条指令之后的其他指令，也可以将`call _puts`函数调用语句修改为空指令`nop`，此处采用第二中方法，具体过程如下：

- 在要修改的指令`call _puts`上右键，选择`Keypatch`中的`Patcher`功能：

![1-5](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062250926.png)

- 由于目标指令为`nop`，因此只要将`Assembly`选项留空并选择`NOPs padding until next instruction boundary`后点击`patch`即可：

![屏幕截图(307)](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062251779.png)

- 修改后的程序如下：

![1-7](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062251051.png)

- 在`Edit`中的`Patch program`中选择`Apply patchs to input file`，即可将修改应用到源文件`overflow`（并可选地生成备份文件`overflow.bak`）：

![屏幕截图(309)](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062251447.png)

- 测试该修改后的`overflow`文件，输入正确的学号，结果如下，仅输出一条与`boy`有关的提示，逻辑漏洞被修复：

![1-9](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062251426.png)

#### 1.2.3.2 栈溢出漏洞

- 在修改好逻辑漏洞的程序的基础上，进一步消除栈溢出漏洞。由于`gets`函数本身存在不会判断输入上限的缺陷，因此选择在`.eh_frame`段中构造补丁代码，使用`read`系统调用来取代`gets`函数，`x86_64`架构下`read`调用的相关信息如下：


| `rax` | `System Call` | `rdi`             | `rsi`       | `rdx`          |
| ----- | ------------- | ----------------- | ----------- | -------------- |
| `0`   | `sys_read`    | `unsigned int fd` | `char* buf` | `size_t count` |

- 在`IDA`中查看`call _get`指令所在的地址为`0x06FC`，下一条指令的地址为`0x0701`：

![1-10](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062254786.png)

- 在`IDA`中使用`shift + F7`打开段表并查看`.eh_frame`段，选择`0x0888`处作为补丁代码的起始位置：

![1-10](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062252195.png)

- 根据以上信息编写补丁代码如下，并将其使用`Keypatch`的`Patcher`或`Fill range`功能填充到`.eh_frame`段的相应位置，点击`C`将其转换为代码：


```assembly
mov rax, 0			; rax <- 0 (sys_read)
mov rdi, 0			; rdi <- 0 (stdin)
lea rsi, [rbp-0Bh]	; rsi <- s
mov rdx, 0Ah		; rdx <- 10
syscall
jmp 0x0701			; jmp back
```

![1-12](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062254966.png)

- 再将源程序的`call _get`指令使用`Keypatch`修改为`jmp 0x0888`即可：

![1-13](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062254698.png)

- 将以上修改应用到源文件并进行测试，可见无论输入多长都不会发生栈溢出，超出的字符将保留在标准输入中被后续的`shell`程序处理，栈溢出漏洞被修复（超出的字符串将被作为一条指令被`shell`执行，可能又产生了命令注入漏洞，可以通过编写更加复杂的补丁代码来解决这个问题）：

![1-14](https://raw.githubusercontent.com/BIIIANG/pic/main/202206062258016.png)
