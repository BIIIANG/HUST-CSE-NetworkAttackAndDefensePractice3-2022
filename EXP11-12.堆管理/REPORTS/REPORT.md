编译`glibc`

```shell
# 创建工作目录并从共享文件夹复制文件
vagrant@vagrant-ubuntu-trusty-32:~$ mkdir NADP3
vagrant@vagrant-ubuntu-trusty-32:~$ cd NADP3
vagrant@vagrant-ubuntu-trusty-32:~/NADP3$ cp -r /mnt/hgfs/CODES/* ./
vagrant@vagrant-ubuntu-trusty-32:~/NADP3$ ll
total 25660
drwxrwxr-x  2 vagrant vagrant     4096 Jun  6 07:09 ./
drwxr-xr-x 10 vagrant vagrant     4096 Jun  6 07:06 ../
-rwxrwxr-x  1 vagrant vagrant     1926 Jun  6 07:09 attack.c*
-rwxrwxr-x  1 vagrant vagrant 26204208 Jun  6 07:09 glibc-2.20-lwm.tgz*
-rwxrwxr-x  1 vagrant vagrant    52173 Jun  6 07:09 MallocMaleficarum.txt*
-rwxrwxr-x  1 vagrant vagrant      306 Jun  6 07:09 vuln.c*

# 解压源代码并编译
vagrant@vagrant-ubuntu-trusty-32:~/NADP3$ tar -xzvf ./glibc-2.20-lwm.tgz
vagrant@vagrant-ubuntu-trusty-32:~/NADP3$ mkdir glibc-build
vagrant@vagrant-ubuntu-trusty-32:~/NADP3$ cd glibc-build
vagrant@vagrant-ubuntu-trusty-32:~/NADP3/glibc-build$ ../glibc-2.20/configure --prefix=/home/vagrant/NADP3/glibc-build/
vagrant@vagrant-ubuntu-trusty-32:~/NADP3/glibc-build$ make && make install
```

编译程序

```shell
# 编译vuln.c
vagrant@vagrant-ubuntu-trusty-32:~/glibc-build$ cd ..
vagrant@vagrant-ubuntu-trusty-32:~$ gcc -g -z norelro -z execstack -o vuln vuln.c -Wl,--rpath=/home/vagrant/glibc-build/lib -Wl,--dynamic-linker=/home/vagrant/glibc-build/lib/ld-linux.so.2
vagrant@vagrant-ubuntu-trusty-32:~$ ldd vuln
        linux-gate.so.1 =>  (0xb7fff000)
        libc.so.6 => /home/vagrant/glibc-build/lib/libc.so.6 (0xb7e5e000)
        /home/vagrant/glibc-build/lib/ld-linux.so.2 => /lib/ld-linux.so.2 (0x80000000)

# 编译attack.c
vagrant@vagrant-ubuntu-trusty-32:~$ gcc -g -o attack attack.c
vagrant@vagrant-ubuntu-trusty-32:~$ ldd attack
        linux-gate.so.1 =>  (0xb7fff000)
        libc.so.6 => /lib/i386-linux-gnu/libc.so.6 (0xb7e44000)
        /lib/ld-linux.so.2 (0x80000000)

# 关闭系统地址随机化
vagrant@vagrant-ubuntu-trusty-32:~$ echo 0 | sudo tee -a /proc/sys/kernel/randomize_va_space
0

# 进行攻击
vagrant@vagrant-ubuntu-trusty-32:~$ ./attack
$ ls
MallocMaleficarum.txt  attack  attack.c  glibc-2.20  glibc-2.20-lwm.tgz  glibc-build  input_str.txt  vuln  vuln.c
$ id
uid=1000(vagrant) gid=1000(vagrant) groups=1000(vagrant)
$ exit
```

调试程序

```shell
# FUNCTION_POINTER
vagrant@vagrant-ubuntu-trusty-32:~$ readelf -r vuln

Relocation section '.rel.dyn' at offset 0x2d8 contains 1 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
08049768  00000406 R_386_GLOB_DAT    00000000   __gmon_start__

Relocation section '.rel.plt' at offset 0x2e0 contains 5 entries:
 Offset     Info    Type            Sym.Value  Sym. Name
08049778  00000107 R_386_JUMP_SLOT   00000000   free
0804977c  00000207 R_386_JUMP_SLOT   00000000   strcpy
08049780  00000307 R_386_JUMP_SLOT   00000000   malloc
08049784  00000407 R_386_JUMP_SLOT   00000000   __gmon_start__
08049788  00000507 R_386_JUMP_SLOT   00000000   __libc_start_main

# gdb调试
gdb attack
set follow-fork-mode child
b main
run
c

heap chunks
heap chunk
```

<div style="page-break-after:always;"></div>

### 1. `attack.c`中`680`这个数值怎么来的？在分配的内存中覆盖了哪些范围，请画图说明。

- `malloc(666)`时，`666 + SIZE_SZ = 670`，对齐到`8`字节，因此该`chunk`大小为`672`字节，其中后`4`个字节在该`chunk`空闲时可作为下一个`chunk`的`previous chunk size`使用。
- `malloc(12)`时，`12 + SIZE_SZ = 16`，对齐到`8`字节，因此该`chunk`大小为`16`字节，其中后`4`个字节在该`chunk`空闲时可作为下一个`chunk`的`previous chunk size`使用。
- 由于主要目的是覆盖第二个`chunk`以进行`unlink`攻击，第一个`chunk`用来存储`shellcode`，因此从第一个`chunk`的用户数据部分开始的`664 + 4 + 4 + 4 + 4 = 680`字节为需要覆盖的空间。前`664`字节主要用于存放`shellcode`，后`16`字节与`chunk 2`的属性和操作密切相关，用于进行`unlink`攻击。
- 因此覆盖了第一个`chunk`的所有用户数据部分和第二个`chunk`的`previous chunk size(与第一个chunk的用户内容重复), chunk size`和用于存放`fd`和`bk`的`8`字节的用户数据部分。

![image-20220613061038957](https://raw.githubusercontent.com/BIIIANG/pic/main/202206130610053.png)

<div style="page-break-after:always;"></div>

### 2. `attack.c`中覆盖第二个`chunk`的元数据`fd`指针，为什么要使用`FUNCTION_POINTER-12` ?

- 结构体`malloc_chunk`如下，`fd`指针的偏移为`2 * sizeof(INTERNAL_SIZE_T) = 2 * sizeof(size_t)`，在`32`位系统中为`8`字节，同理`bk`指针的偏移为`12`字节。

```c
#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T size_t
#endif

struct malloc_chunk {

  INTERNAL_SIZE_T      prev_size;  /* Size of previous chunk (if free).  */
  INTERNAL_SIZE_T      size;       /* Size in bytes, including overhead. */

  struct malloc_chunk* fd;         /* double links -- used only if free. */
  struct malloc_chunk* bk;

  /* Only used for large blocks: pointer to next larger size.  */
  struct malloc_chunk* fd_nextsize; /* double links -- used only if free. */
  struct malloc_chunk* bk_nextsize;
};
```

- 简化的`unlink`如下所示，它完成的操作是从双向链表中摘除`P`，完成操作后，上一个`chunk`的`bk`指针被置为`P->bk`，下一个`chunk`的`fd`指针被置为`P->fd`。

```c
#define unlink(P, BK, FD) {                                            \
    FD = P->fd;								      \
    BK = P->bk;								      \
        FD->bk = BK;							      \
        BK->fd = FD;							      \
		...
}
```

- 为了完成`unlink`攻击，我们的目标是向`free()`函数的地址写入`shellcode`，使得下一次调用`free()`函数时执行`shellcode`。
- 为此，我们申请了两个`chunk`，并且将第二个`chunk`的元数据`fd`指针设为`FUNCTION_POINTER-12`，将第二个`chunk`的元数据`bk`指针设为`shellcode`的地址。这样，在对第二个`chunk`执行`unlink`操作时，`FD`和`BK`分别为`FUNCTION_POINTER-12`和`shellcode`的地址。
- `FD`和`BK`均为`struct malloc_chunk*`类型的指针，`bk`指针在`struct malloc_chunk`内的偏移为`12`字节，因此此时`FD->bk`就相当于`*(FUNCTION_POINTER-12+12)`即`*FUNCTION_POINTER`即`free()`函数的地址。此时再执行`FD->bk = BK;`即可用`shellcode`的地址覆盖`*FUNCTION_POINTER`处`free()`函数的地址，在下一次调用`free()`函数时执行`shellcode`，完成攻击。

<div style="page-break-after:always;"></div>

### 3. `shellcode`开头是`eb0a`，表示跳过`12`个字节，为什么要跳过后面的`"ssppppffff" `？ 另外请反汇编`shellcode`解释`shellcode`的功能。

- 简化的`unlink`如下所示，在前三个赋值语句执行完毕后，函数指针`FUNCTION_POINTER`指向的`free()`函数的地址已被覆盖为`shellcode`的地址。

```c
#define unlink(P, BK, FD) {                                            \
    FD = P->fd;								      \
    BK = P->bk;								      \
        FD->bk = BK;							      \
        BK->fd = FD;							      \
		...
}
```

- 在`BK->fd = FD;`执行时，`FD`和`BK`分别为`FUNCTION_POINTER-12`和`shellcode`的地址，`fd`指针在`struct malloc_chunk`内的偏移为`8`字节，因此该语句会将`*(shellcode address + 8)`置为`FUNCTION_POINTER-12`，即`shellcode`的第`9~12`个字节被`FUNCTION_POINTER-12`覆盖。
- 若不跳过`12`个字节，`FUNCTION_POINTER-12`将被解析为命令执行，这是不可控的，可能产生无法预料的结果导致`shellcode`执行失败，因此应跳过`12`字节，从第`13`字节开始执行受控制的代码，保证`shellcode`的顺利执行。
- `shellcode`的反汇编结果如下所示，首先跳过`12`字节到达有效的指令区域，然后将字符串`/bin//sh`入栈，之后将参数准备到`ebx, ecx, edx`寄存器中，调用`0xB`号即`execve`中断。上图为使用`gdb`运行到`shellcode`时的指令，可以发现`0x804a020 ~ 0x804a023`被置为`FUNCTION_POINTER-12`，从而被解析为不可控的指令；下图为对`shellcode`直接进行反汇编的结果。

![image-20220613032936353](https://raw.githubusercontent.com/BIIIANG/pic/main/202206130329505.png)

![image-20220613035523989](https://raw.githubusercontent.com/BIIIANG/pic/main/202206130355055.png)

- `execve`中断的参数信息如下所示：

| `syscall` | `eax`  | `arg0(ebx)`            | `arg1(ecx)`               | `arg2(edx)`               |
| --------- | ------ | ---------------------- | ------------------------- | ------------------------- |
| `execve`  | `0x0b` | `const char *filename` | `const char *const *argv` | `const char *const *envp` |

- `shellcode`使`edx`指向一个空指针，使`ecx`指向`/bin//sh`的地址，之后为空指针，即仅存在一个执行参数`argv[0] = /bin//sh`，使`ebx`指向`/bin//sh`，即即将要执行的程序名。在以上参数准备完毕后，设置调用号并通过`execve`系统调用运行`/bin//sh`以获取`shell`，从而获得控制权。

<div style="page-break-after:always;"></div>

### 4. `vuln.c`中第一次调用`free`的时候是什么情况下进行`chunk`的`consolidation`的？依据`glibc`源代码进行分析，给出分析过程。

- 在`glibc`中，`free()`是`__libc_free()`的别名，其内部又调用了`_int_free()`，因此`free()`的核心部分为`_int_free()`函数（见`/GLIBC-2.20/malloc/malloc.c`）。
- `_int_free()`函数中对后一个`chunk`进行`consolidation`的代码如下所示，其检测向下第二个`chunk`的`inuse`位来判断下一个`chunk`是否被使用。若没有发生堆溢出，它将通过`chunk 2`的下一个`chunk（即top chunk）`的`chunk size`的`prev_inuse`位来判断`chunk 2`是否在使用，获得下一个`chunk`的`prev_inuse`位使用的是`inuse_bit_at_offset`宏，通过`chunk 2`的起始地址加`chunk 2`的大小来获得下一个`chunk`的`chunk size`。
- 通过堆溢出，`chunk 2 size`被置为`-4`，则通过`chunk 2 + chunk 2 size`定位的下一个`chunk`仍然为`chunk 2`，而`chunk 2 size`的`prev_inuse`位已通过堆溢出置为`0`（即`-4 (0b 1111 1111 1111 1111 1111 1111 1111 1100)`的最低位），相当于自己说自己没有被使用，因此在对第一个`chunk`执行`free()`时，会与第二个`chunk`进行`consolidation`。

```c
#define inuse_bit_at_offset(p, s)					      \
  (((mchunkptr) (((char *) (p)) + (s)))->size & PREV_INUSE)

static void
_int_free (mstate av, mchunkptr p, int have_lock)
{
  ...

    if (nextchunk != av->top) {
      /* get and clear inuse bit */
      nextinuse = inuse_bit_at_offset(nextchunk, nextsize);

      /* consolidate forward */
      if (!nextinuse) {
	    unlink(nextchunk, bck, fwd);
	    size += nextsize;
      } else
	    clear_inuse_bit_at_offset(nextchunk, 0);

  ...
}
```

- 完整的对第一个`chunk`执行`free()`的流程为（参考下图`free()`函数的执行流程）：
  - 由于第一个`chunk`的大小大于`512`字节，在`Large Bin`中，不在`FastBin`；
  - 由于该`chunk`为第一个`chunk`，其`prev_inuse`位为`1`；
  - 下一个`chunk`为分配的第二个`chunk`，不是`top chunk`；
  - 根据以上分析，欺骗`glibc`使其认为下一个`chunk`未被使用；
  - `consolidation`下一个`chunk`并`unlink`下一个`chunk`；
  - 添加到`Unsorted Bin`，执行完毕。

![image-20220613061350302](https://raw.githubusercontent.com/BIIIANG/pic/main/202206130613399.png)

<div style="page-break-after:always;"></div>








```

/*
    An allocated chunk looks like this:


    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of previous chunk, if allocated            | |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of chunk, in bytes                       |M|P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             User data starts here...                          .
	        .                                                               .
	        .             (malloc_usable_size() bytes)                      .
	        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of chunk                                     |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Size of previous chunk                            |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Forward pointer to next chunk in list             |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Back pointer to previous chunk in list            |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	        |             Unused space (may be 0 bytes long)                .
	        .                                                               .
	        .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
	        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
```



```c
/* Take a chunk off a bin list */
/** liweiming, original function
#define unlink(P, BK, FD) {                                            \
    FD = P->fd;								      \
    BK = P->bk;								      \
    if (__builtin_expect (FD->bk != P || BK->fd != P, 0))		      \
        malloc_printerr (check_action, "corrupted double-linked list", P); \
    else {								      \
        FD->bk = BK;							      \
        BK->fd = FD;							      \
        if (!in_smallbin_range (P->size)				      \
            && __builtin_expect (P->fd_nextsize != NULL, 0)) {		      \
            assert (P->fd_nextsize->bk_nextsize == P);			      \
            assert (P->bk_nextsize->fd_nextsize == P);			      \
            if (FD->fd_nextsize == NULL) {				      \
                if (P->fd_nextsize == P)				      \
                  FD->fd_nextsize = FD->bk_nextsize = FD;		      \
                else {							      \
                    FD->fd_nextsize = P->fd_nextsize;			      \
                    FD->bk_nextsize = P->bk_nextsize;			      \
                    P->fd_nextsize->bk_nextsize = FD;			      \
                    P->bk_nextsize->fd_nextsize = FD;			      \
                  }							      \
              } else {							      \
                P->fd_nextsize->bk_nextsize = P->bk_nextsize;		      \
                P->bk_nextsize->fd_nextsize = P->fd_nextsize;		      \
              }								      \
          }								      \
      }									      \
}
**/

/* Take a chunk off a bin list , liweiming removed double-link protection*/
#define unlink(P, BK, FD) {                                            \
    FD = P->fd;								      \
    BK = P->bk;								      \
        FD->bk = BK;							      \
        BK->fd = FD;							      \
        if (!in_smallbin_range (P->size)				      \
            && __builtin_expect (P->fd_nextsize != NULL, 0)) {		      \
            assert (P->fd_nextsize->bk_nextsize == P);			      \
            assert (P->bk_nextsize->fd_nextsize == P);			      \
            if (FD->fd_nextsize == NULL) {				      \
                if (P->fd_nextsize == P)				      \
                  FD->fd_nextsize = FD->bk_nextsize = FD;		      \
                else {							      \
                    FD->fd_nextsize = P->fd_nextsize;			      \
                    FD->bk_nextsize = P->bk_nextsize;			      \
                    P->fd_nextsize->bk_nextsize = FD;			      \
                    P->bk_nextsize->fd_nextsize = FD;			      \
                  }							      \
              } else {							      \
                P->fd_nextsize->bk_nextsize = P->bk_nextsize;		      \
                P->bk_nextsize->fd_nextsize = P->fd_nextsize;		      \
              }								      \
          }								      \
}
```

