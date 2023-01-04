## 1. 程序漏洞位置

- `strcpy`前`LOBYTE(v1) = v5;`结合`(unsigned __int8)a2 > 3u && (unsigned __int8)a2 <= 8u`使得程序可能发生整数溢出：

![1-漏洞位置](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091547975.png)

## 2. 包含`Success!`提示的函数偏移地址为`0x00401440`

- 在字符串表中寻找目标字符串`Success!`：

![1.0-查找字符串](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091549753.png)

- 找到目标字符串在内存中的位置，并根据引用找到使用它的程序的地址：

![1.1-找到位置](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091549365.png)

- 使用该字符串的函数的偏移地址为`0x00401440`，手动将其反编译：

![2.3-手动设置为一个函数](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091551106.png)

- 反编译结果：

![2-成功函数偏移](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091547882.png)

## 3. 构建二进制输入文件

- 动态调试，在`strcpy`函数执行前，可以看到栈中`0x0019FB58`的位置存放着返回地址：

![3.1-strcpy前](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091600623.png)

- 使用以下随机生成的长度为`256 + 5`字符串进行测试：

![3.2](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091600187.png)

- 返回地址的位置`0x0019FB58`被替换为`0x53575568`：

![3.3](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091600560.png)

- 在构造的字符串文件中搜索这四个字符，可以找到需要修改的位置（也可以根据老师使用的数偏移地址的方式确定要修改的位置）：

![3.4](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091600819.png)

- 将其修改为输出`Success!`的函数的地址，至此二进制输入文件`xba.txt`构造完成：

![3-二进制输入文件](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091547025.png)

## 4. 测试程序，能够提示`Success!`

![4-成功提示](https://raw.githubusercontent.com/BIIIANG/pic/main/202205091547219.png)