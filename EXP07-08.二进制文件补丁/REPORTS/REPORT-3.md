# 3. 热补丁 `repeat`

## 3.1 实验要求

- 程序repeat会重复输出一句格言，将输出的内容修改为自己的姓名。

## 3.2 实验过程

### 3.2.1 程序分析

- 首先使用`IDA`打开`getshell`程序，其`main`函数的反汇编结果如下：

![QQ截图20220607161902](https://raw.githubusercontent.com/BIIIANG/pic/main/202206071620452.png)

- 将`main`函数反编译，结果如下：


![3-0-2](https://raw.githubusercontent.com/BIIIANG/pic/main/202206071621291.png)

### 3.2.2 使用`Preload Hook`

- 编写自定义的`puts()`函数如下：

```c
// preload_hook_puts.c
#define _GNU_SOURCE
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

int puts(char *s) {
	int (*old_puts)(char *);
	old_puts = (int (*)(char *))dlsym(RTLD_NEXT, "puts");
	old_puts("My name is XuBiang@U201911803.");
}
```

- 编译该文件生成动态链接库：

```shell
gcc -m32 -fPIC --shared preload_hook_puts.c -o preload_hook_puts.so -ldl
```

- 加载动态链接库`export LD_PRELOAD=$PWD/preload_hook_puts.so`，并执行`repeat`程序，可以正常完成补丁功能：

![3-1](https://raw.githubusercontent.com/BIIIANG/pic/main/202206070229652.png)

### 3.2.3 完整的热补丁

- 补丁代码如下：

```c
// patch.c
#include <stdio.h>

int newputs() {
	puts("My name is XuBiang@U201911803.");
}
```

- 编译补丁文件：

```shell
gcc -m32 -fPIC --shared patch.c -o patch.so
```

- 加载程序同`例题/6.2/hotfix`；
- 先启动`repeat`程序，通过`ps -aux | grep repeat`查看其进程`ID`，将其作为参数运行`hotfix`：

![3-2](../IMGS/3-2.png)

- 热补丁效果如下：

![3-3](../IMGS/3-3.png)
