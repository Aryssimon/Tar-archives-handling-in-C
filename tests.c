#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "lib_tar.h"

/**
 * You are free to use this file to write tests for your implementation
 */

void debug_dump(const uint8_t *bytes, size_t len) {
    for (int i = 0; i < len;) {
        printf("%04x:  ", (int) i);

        for (int j = 0; j < 16 && i + j < len; j++) {
            printf("%02x ", bytes[i + j]);
        }
        printf("\t");
        for (int j = 0; j < 16 && i < len; j++, i++) {
            printf("%c ", bytes[i]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s tar_file\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1] , O_RDONLY);
    if (fd == -1) {
        perror("open(tar_file)");
        return -1;
    }
    // Tests for check_archive()
    int ret = check_archive(fd);
    printf("check_archive returned %d\n", ret);

    // Tests for exists()
    ret = exists(fd,"hints.txt");
    printf("check hints.txt %d\n", ret);
    ret = exists(fd,"lib_tar.c");
    printf("check lib_tar.c %d\n", ret);
    ret = exists(fd,"myfolder/");
    printf("check myfolder/ %d\n", ret);
    ret = exists(fd,"myfolder/README.md");
    printf("check myfolder/README.md %d\n", ret);
    ret = exists(fd,"myfolder/tests.c");
    printf("check myfolder/tests.c %d\n", ret);

    ret = exists(fd,"nothing");
    printf("check nothing %d\n", ret);
    ret = exists(fd,"falsetest.txt");
    printf("check falsetest.txt %d\n", ret);

    return 0;
}
