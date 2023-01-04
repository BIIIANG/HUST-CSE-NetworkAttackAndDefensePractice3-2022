#include <bits/dlfcn.h>
#include <elf.h>
#include <fcntl.h>
#include <link.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>

#define IMAGE_ADDR 0x08048000

int mode = 2;

struct user_regs_struct oldregs;
Elf32_Addr phdr_addr;
Elf32_Addr dyn_addr;
Elf32_Addr map_addr;
Elf32_Addr symtab;
Elf32_Addr strtab;
Elf32_Addr jmprel;
Elf32_Addr reldyn;
Elf32_Word reldynsz;
Elf32_Word totalrelsize;
Elf32_Word relsize;
unsigned long link_addr;
int nrels;
int nreldyns;
// int nchains;
int modifyflag = 0;
/*char libpath[128] = "/mnt/hgfs/svnroot/test/injectsov2/prj_linux/so.so";*/

/* 读进程寄存器 */
void ptrace_readreg(int pid, struct user_regs_struct* regs) {
    if (ptrace(PTRACE_GETREGS, pid, NULL, regs))
        printf("*** ptrace_readreg error ***\n");
    /*printf("ptrace_readreg\n");
    printf("%x\n",regs->ebx);
    printf("%x\n",regs->ecx);
    printf("%x\n",regs->edx);
    printf("%x\n",regs->esi);
    printf("%x\n",regs->edi);
    printf("%x\n",regs->ebp);
    printf("%x\n",regs->eax);
    printf("%x\n",regs->xds);
    printf("%x\n",regs->xes);
    printf("%x\n",regs->xfs);
    printf("%x\n",regs->xgs);
    printf("%x\n",regs->orig_eax);
    printf("%x\n",regs->eip);
    printf("%x\n",regs->xcs);
    printf("%x\n",regs->eflags);
    printf("%x\n",regs->esp);
    printf("%x\n",regs->xss);*/
}

/* 写进程寄存器 */
void ptrace_writereg(int pid, struct user_regs_struct* regs) {
    /*printf("ptrace_writereg\n");
    printf("%x\n",regs->ebx);
    printf("%x\n",regs->ecx);
    printf("%x\n",regs->edx);
    printf("%x\n",regs->esi);
    printf("%x\n",regs->edi);
    printf("%x\n",regs->ebp);
    printf("%x\n",regs->eax);
    printf("%x\n",regs->xds);
    printf("%x\n",regs->xes);
    printf("%x\n",regs->xfs);
    printf("%x\n",regs->xgs);
    printf("%x\n",regs->orig_eax);
    printf("%x\n",regs->eip);
    printf("%x\n",regs->xcs);
    printf("%x\n",regs->eflags);
    printf("%x\n",regs->esp);
    printf("%x\n",regs->xss);*/

    if (ptrace(PTRACE_SETREGS, pid, NULL, regs))
        printf("*** ptrace_writereg error ***\n");
}

/* 关联到进程 */
void ptrace_attach(int pid) {
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) < 0) {
        perror("ptrace_attach");
        exit(-1);
    }

    waitpid(pid, NULL, /*WUNTRACED*/ 0);

    ptrace_readreg(pid, &oldregs);
}

/* 进程继续 */
void ptrace_cont(int pid) {
    int stat;

    if (ptrace(PTRACE_CONT, pid, NULL, NULL) < 0) {
        perror("ptrace_cont");
        exit(-1);
    }
    /*while(!WIFSTOPPED(stat))
        waitpid(pid, &stat, WNOHANG);*/
}

/* 脱离进程 */
void ptrace_detach(int pid) {
    ptrace_writereg(pid, &oldregs);

    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) < 0) {
        perror("ptrace_detach");
        exit(-1);
    }
}

/* 写指定进程地址 */
void ptrace_write(int pid, unsigned long addr, void* vptr, int len) {
    int count;
    long word;

    count = 0;

    while (count < len) {
        memcpy(&word, vptr + count, sizeof(word));
        word = ptrace(PTRACE_POKETEXT, pid, addr + count, word);
        count += 4;

        if (errno != 0)
            printf("ptrace_write failed\t %ld\n", addr + count);
    }
}

/* 读指定进程 */
int ptrace_read(int pid, unsigned long addr, void* vptr, int len) {
    int i, count;
    long word;
    unsigned long* ptr = (unsigned long*)vptr;

    i = count = 0;
    // printf("ptrace_read addr = %x\n",addr);
    while (count < len) {
        // printf("ptrace_read addr+count = %x\n",addr + count);
        word = ptrace(PTRACE_PEEKTEXT, pid, addr + count, NULL);
        while (word < 0) {
            if (errno == 0)
                break;
            // printf("ptrace_read word = %x\n",word);
            perror("ptrace_read failed");
            return 2;
        }
        count += 4;
        ptr[i++] = word;
    }
    return 0;
}

/*
 在进程指定地址读一个字符串
 */
char* ptrace_readstr(int pid, unsigned long addr) {
    char* str = (char*)malloc(64);
    int i, count;
    long word;
    char* pa;

    i = count = 0;
    pa = (char*)&word;

    while (i <= 60) {
        word = ptrace(PTRACE_PEEKTEXT, pid, addr + count, NULL);
        count += 4;

        if (pa[0] == 0) {
            str[i] = 0;
            break;
        } else
            str[i++] = pa[0];

        if (pa[1] == 0) {
            str[i] = 0;
            break;
        } else
            str[i++] = pa[1];

        if (pa[2] == 0) {
            str[i] = 0;
            break;
        } else
            str[i++] = pa[2];

        if (pa[3] == 0) {
            str[i] = 0;
            break;
        } else
            str[i++] = pa[3];
    }

    return str;
}

/*
 将指定数据压入进程堆栈并返回堆栈指针
 */
void* ptrace_push(int pid, void* paddr, int size) {
    unsigned long esp;
    struct user_regs_struct regs;

    ptrace_readreg(pid, &regs);
    esp = regs.esp;
    esp -= size;
    esp = esp - esp % 4;
    regs.esp = esp;

    ptrace_writereg(pid, &regs);

    ptrace_write(pid, esp, paddr, size);

    return (void*)esp;
}

/*
 在进程内调用指定地址的函数
 */
void ptrace_call(int pid, unsigned long addr) {
    void* pc;
    struct user_regs_struct regs;
    int stat;
    void* pra;

    pc = (void*)0x41414140;
    pra = ptrace_push(pid, &pc, sizeof(pc));

    ptrace_readreg(pid, &regs);
    regs.eip = addr;
    ptrace_writereg(pid, &regs);

    ptrace_cont(pid);
    // while(WIFSIGNALED(stat))
    //  waitpid(pid, &stat, WNOHANG);
}

/*
 得到指向link_map链表首项的指针
 */
struct link_map* get_linkmap(int pid) {
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)malloc(sizeof(Elf32_Ehdr));
    Elf32_Phdr* phdr = (Elf32_Phdr*)malloc(sizeof(Elf32_Phdr));
    Elf32_Dyn* dyn = (Elf32_Dyn*)malloc(sizeof(Elf32_Dyn));
    Elf32_Word got;
    struct link_map* map = (struct link_map*)malloc(sizeof(struct link_map));
    int i = 1;
    unsigned long tmpaddr;

    ptrace_read(pid, IMAGE_ADDR, ehdr, sizeof(Elf32_Ehdr));
    phdr_addr = IMAGE_ADDR + ehdr->e_phoff;
    printf("phdr_addr\t %p\n", phdr_addr);

    ptrace_read(pid, phdr_addr, phdr, sizeof(Elf32_Phdr));
    while (phdr->p_type != PT_DYNAMIC)
        ptrace_read(pid, phdr_addr += sizeof(Elf32_Phdr), phdr, sizeof(Elf32_Phdr));
    dyn_addr = phdr->p_vaddr;
    printf("dyn_addr\t %p\n", dyn_addr);

    ptrace_read(pid, dyn_addr, dyn, sizeof(Elf32_Dyn));
    while (dyn->d_tag != DT_PLTGOT) {
        tmpaddr = dyn_addr + i * sizeof(Elf32_Dyn);
        // printf("get_linkmap tmpaddr = %x\n",tmpaddr);
        ptrace_read(pid, tmpaddr, dyn, sizeof(Elf32_Dyn));
        i++;
    }

    got = (Elf32_Word)dyn->d_un.d_ptr;
    got += 4;
    // printf("GOT\t\t %p\n", got);

    ptrace_read(pid, got, &map_addr, 4);
    printf("map_addr\t %p\n", map_addr);
    map = map_addr;
    // ptrace_read(pid, map_addr, map, sizeof(struct link_map));

    free(ehdr);
    free(phdr);
    free(dyn);

    return map;
}

/*
 取得给定link_map指向的SYMTAB、STRTAB、HASH、JMPREL、PLTRELSZ、RELAENT、RELENT信息
 这些地址信息将被保存到全局变量中，以方便使用
 */
void get_sym_info(int pid, struct link_map* lm) {
    Elf32_Dyn* dyn = (Elf32_Dyn*)malloc(sizeof(Elf32_Dyn));
    unsigned long dyn_addr;
    // printf("get_sym_info lm = %x\n",lm);
    // printf("get_sym_info lm->l_ld's offset = %x\n",&((struct link_map *)0)->l_ld);
    // printf("get_sym_info &lm->l_ld = %x\n",&(lm->l_ld));
    // dyn_addr = (unsigned long)&(lm->l_ld);
    //进入被跟踪进程获取动态节的地址
    ptrace_read(pid, &(lm->l_ld), &dyn_addr, sizeof(dyn_addr));
    ptrace_read(pid, &(lm->l_addr), &link_addr, sizeof(dyn_addr));
    ptrace_read(pid, dyn_addr, dyn, sizeof(Elf32_Dyn));
    // if(link_addr == 0)
    //   getnchains(pid,IMAGE_ADDR);
    /*else
        getnchains(pid,link_addr);*/
    while (dyn->d_tag != DT_NULL) {
        // printf("get_sym_info dyn->d_tag = %x\n",dyn->d_tag);
        // printf("get_sym_info dyn->d_un.d_ptr = %x\n",dyn->d_un.d_ptr);
        switch (dyn->d_tag) {
            case DT_SYMTAB:
                symtab = dyn->d_un.d_ptr;

                break;
            case DT_STRTAB:
                strtab = dyn->d_un.d_ptr;
                break;
            /*case DT_HASH://可能不存在哈希表，此时nchains是错误的，这个值可以通过符号表得到
                //printf("get_sym_info hash table's addr = %x\n",dyn->d_un.d_ptr);
                //printf("get_sym_info symtbl's entry = %x\n",(dyn->d_un.d_ptr) + 4);
                ptrace_read(pid, (dyn->d_un.d_ptr) + 4,&nchains, sizeof(nchains));
                break;*/
            case DT_JMPREL:
                jmprel = dyn->d_un.d_ptr;
                break;
            case DT_PLTRELSZ:
                totalrelsize = dyn->d_un.d_val;
                break;
            case DT_RELAENT:
                relsize = dyn->d_un.d_val;
                break;
            case DT_RELENT:
                relsize = dyn->d_un.d_val;
                break;
            case DT_REL:
                reldyn = dyn->d_un.d_ptr;
                break;
            case DT_RELSZ:
                reldynsz = dyn->d_un.d_val;
                break;
        }
        ptrace_read(pid, dyn_addr += sizeof(Elf32_Dyn), dyn, sizeof(Elf32_Dyn));
    }

    nrels = totalrelsize / relsize;
    nreldyns = reldynsz / relsize;

    free(dyn);
    printf("get_sym_info exit\n");
}

/*
 在指定的link_map所指向的符号表中查找符号
 */
unsigned long find_symbol_in_linkmap(int pid, struct link_map* lm, char* sym_name) {
    Elf32_Sym* sym = (Elf32_Sym*)malloc(sizeof(Elf32_Sym));
    int i = 0;
    char* str;
    unsigned long ret;
    int flags = 0;

    get_sym_info(pid, lm);

    do {
        if (ptrace_read(pid, symtab + i * sizeof(Elf32_Sym), sym, sizeof(Elf32_Sym)))
            return 0;
        i++;
        if (!sym->st_name && !sym->st_size && !sym->st_value)  //全为0是符号表的第一项
            continue;
        str = (char*)ptrace_readstr(pid, strtab + sym->st_name);
        if (strcmp(str, sym_name) == 0) {
            printf("\nfind_symbol_in_linkmap str = %s\n", str);
            printf("\nfind_symbol_in_linkmap sym->st_value = %x\n", sym->st_value);
            free(str);
            if (sym->st_value == 0)  //值为0代表这个符号本身就是重定向的内容
                continue;
            flags = 1;
            break;
        }
        free(str);
    } while (1);

    if (flags != 1)
        ret = 0;
    else
        ret = link_addr + sym->st_value;

    free(sym);

    return ret;
}

/*
 解析指定符号
 */
unsigned long find_symbol(int pid, struct link_map* map, char* sym_name) {
    struct link_map* lm = map;
    unsigned long sym_addr;
    char* str;
    unsigned long tmp;

    sym_addr = find_symbol_in_linkmap(pid, lm, sym_name);
    while (!sym_addr) {
        ptrace_read(pid, (char*)lm + 12, &tmp, 4);  //获取下一个库的link_map地址
        if (tmp == 0)
            return 0;
        lm = tmp;

        if ((sym_addr = find_symbol_in_linkmap(pid, lm, sym_name)))
            break;
    }

    return sym_addr;
}

/* 查找符号的重定位地址 */
unsigned long find_sym_in_rel(int pid, char* sym_name) {
    Elf32_Rel* rel = (Elf32_Rel*)malloc(sizeof(Elf32_Rel));
    Elf32_Sym* sym = (Elf32_Sym*)malloc(sizeof(Elf32_Sym));
    int i;
    char* str;
    unsigned long ret;
    struct link_map* lm;
    lm = map_addr;

    // get_dyn_info(pid);
    do {
        get_sym_info(pid, lm);
        ptrace_read(pid, (char*)lm + 12, &lm, 4);
        //首先查找过程连接的重定位表
        for (i = 0; i < nrels; i++) {
            ptrace_read(pid, (unsigned long)(jmprel + i * sizeof(Elf32_Rel)),
                        rel, sizeof(Elf32_Rel));
            if (ELF32_R_SYM(rel->r_info)) {
                ptrace_read(pid, symtab + ELF32_R_SYM(rel->r_info) * sizeof(Elf32_Sym), sym, sizeof(Elf32_Sym));
                str = ptrace_readstr(pid, strtab + sym->st_name);
                if (strcmp(str, sym_name) == 0) {
                    if (sym->st_value != 0) {
                        free(str);
                        continue;
                    }
                    modifyflag = 1;
                    free(str);
                    break;
                }
                free(str);
            }
        }

        if (modifyflag == 1)
            break;
        //没找到的话，再找在链接时就重定位的重定位表
        for (i = 0; i < nreldyns; i++) {
            ptrace_read(pid, (unsigned long)(reldyn + i * sizeof(Elf32_Rel)),
                        rel, sizeof(Elf32_Rel));
            if (ELF32_R_SYM(rel->r_info)) {
                ptrace_read(pid, symtab + ELF32_R_SYM(rel->r_info) * sizeof(Elf32_Sym), sym, sizeof(Elf32_Sym));
                str = ptrace_readstr(pid, strtab + sym->st_name);
                if (strcmp(str, sym_name) == 0) {
                    if (sym->st_value != 0) {
                        free(str);
                        continue;
                    }
                    modifyflag = 2;
                    free(str);
                    break;
                }
                free(str);
            }
        }

        if (modifyflag == 2)
            break;

    } while (lm);
    // printf("find_sym_in_rel flags = %d\n",flags);
    if (modifyflag == 0)
        ret = 0;
    else
        ret = link_addr + rel->r_offset;
    // printf("find_sym_in_rel link_addr = %x\t sym->st_value = %x\n",link_addr , sym->st_value);
    free(rel);
    free(sym);

    return ret;
}

void call__libc_dlopen_mode(int pid, unsigned long addr, char* libname) {
    void* plibnameaddr;

    //将需要加载的共享库地址压栈
    plibnameaddr = ptrace_push(pid, libname, strlen(libname) + 1);
    ptrace_push(pid, &mode, sizeof(int));
    ptrace_push(pid, &plibnameaddr, sizeof(plibnameaddr));

    /* 调用__libc_dlopen_mode */
    ptrace_call(pid, addr);
}

void call_printf(int pid, unsigned long addr, char* string) {
    void* paddr;

    paddr = ptrace_push(pid, string, strlen(string) + 1);
    ptrace_push(pid, &paddr, sizeof(paddr));

    ptrace_call(pid, addr);
}

int main(int argc, char* argv[]) {
    int pid;
    struct link_map* map;
    char sym_name[256];
    unsigned long sym_addr;
    unsigned long new_addr, old_addr, rel_addr;
    int status = 0;
    char libpath[1024];
    char oldfunname[128];
    char newfunname[128];
    // mode = atoi(argv[2]);
    if (argc < 5) {
        printf("usage : ./injso pid libpath oldfunname newfunname\n");
        exit(-1);
    }
    /* 从命令行取得目标进程PID*/
    pid = atoi(argv[1]);

    /* 从命令行取得新库名称*/
    memset(libpath, 0, sizeof(libpath));
    memcpy(libpath, argv[2], strlen(argv[2]));

    /* 从命令行取得旧函数的名称*/
    memset(oldfunname, 0, sizeof(oldfunname));
    memcpy(oldfunname, argv[3], strlen(argv[3]));

    /* 从命令行取得新函数的名称*/
    memset(newfunname, 0, sizeof(newfunname));
    memcpy(newfunname, argv[4], strlen(argv[4]));

    printf("main pid = %d\n", pid);
    printf("main libpath : %s\n", libpath);
    printf("main oldfunname : %s\n", oldfunname);
    printf("main newfunname : %s\n", newfunname);
    /* 关联到目标进程*/
    ptrace_attach(pid);

    /* 得到指向link_map链表的指针 */
    map = get_linkmap(pid); /* get_linkmap */

    sym_addr = find_symbol(pid, map, "printf");
    printf("found printf at addr %p\n", sym_addr);
    if (sym_addr == 0)
        goto detach;
    call_printf(pid, sym_addr, "patch successed\n");
    waitpid(pid, &status, 0);
    printf("status = %x\n", status);

    /* 发现__libc_dlopen_mode，并调用它 */
    sym_addr = find_symbol(pid, map, "__libc_dlopen_mode"); /* call _dl_open */
    printf("found __libc_dlopen_mode at addr %p\n", sym_addr);
    if (sym_addr == 0)
        goto detach;
    call__libc_dlopen_mode(pid, sym_addr, libpath); /* 注意装载的库地址 */
    waitpid(pid, &status, 0);
    /* 找到新函数的地址 */
    strcpy(sym_name, newfunname); /* intercept */
    sym_addr = find_symbol(pid, map, sym_name);
    printf("%s addr\t %p\n", sym_name, sym_addr);
    if (sym_addr == 0)
        goto detach;

    /* 找到旧函数在重定向表的地址 */
    strcpy(sym_name, oldfunname);
    rel_addr = find_sym_in_rel(pid, sym_name);
    printf("%s rel addr\t %p\n", sym_name, rel_addr);
    if (rel_addr == 0)
        goto detach;

    /* 函数重定向 */
    puts("intercept..."); /* intercept */
    if (modifyflag == 2)
        sym_addr = sym_addr - rel_addr - 4;
    printf("main modify sym_addr = %x\n", sym_addr);

    ptrace_write(pid, rel_addr, &sym_addr, sizeof(sym_addr));
    puts("patch ok");
detach:
    printf("prepare to detach\n");
    ptrace_detach(pid);

    return 0;
}