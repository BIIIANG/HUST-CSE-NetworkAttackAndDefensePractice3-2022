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

    // Modified by XuBiang.
    printf("begin as: uid=%d, euid=%d\n", getuid(), geteuid());
    int fd = open("/sys/kernel/debug/drill/drill_act", O_WRONLY);
    if (fd == -1) {
        perror("[-] open failed");
        goto end;
    }
    if ((ret = act(fd, '1')) == EXIT_FAILURE) {
        perror("[-] act 1 failed");
        goto end;
    }
    if ((ret = act(fd, '2')) == EXIT_FAILURE) {
        perror("[-] act 2 failed");
        goto end;
    }
    if ((ret = act(fd, '3')) == EXIT_FAILURE) {
        perror("[-] act 3 failed");
        goto end;
    }
    if ((ret = act(fd, '4')) == EXIT_FAILURE) {
        perror("[-] act 4 failed");
        goto end;
    }

    // printf("begin as: uid=%d, euid=%d\n", getuid(), geteuid());
    // MDL: Use act function to implement the following commands
    // MDL: echo '1' > /sys/kernel/debug/drill/drill_act
    // MDL: echo '2' > /sys/kernel/debug/drill/drill_act
    // MDL: echo '3' > /sys/kernel/debug/drill/drill_act
    // MDL: echo '4' > /sys/kernel/debug/drill/drill_act

end:
    return ret;
}
