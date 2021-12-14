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
    printf("check hints.txt %d\n", ret); //1
    ret = exists(fd,"README.md");
    printf("check README.md %d\n", ret); //1
    ret = exists(fd,"myfolder/");
    printf("check myfolder/ %d\n", ret); //1
    ret = exists(fd,"myfolder/README.md");
    printf("check myfolder/README.md %d\n", ret); //1
    ret = exists(fd,"myfolder/tests.c");
    printf("check myfolder/myfolderinfolder/mytextfile.txt %d\n", ret); //1

    ret = exists(fd,"nothing");
    printf("check nothing %d\n", ret); //0
    ret = exists(fd,"falsetest.txt");
    printf("check falsetest.txt %d\n", ret); //0

    // Tests for is_dir()
    ret = is_dir(fd,"hints.txt");
    printf("check hints.txt %d\n", ret); //0
    ret = is_dir(fd,"myfolder/");
    printf("check myfolder/ %d\n", ret); //1
    ret = is_dir(fd,"myfolder");
    printf("check myfolder %d\n", ret); //0

    // Tests for is_file()
    ret = is_file(fd,"hints.txt");
    printf("check hints.txt %d\n", ret); //1
    ret = is_file(fd,"myfolder/");
    printf("check myfolder/ %d\n", ret); //0
    ret = is_file(fd,"myfolder");
    printf("check myfolder %d\n", ret); //0

    // Tests for is_symlink()
    ret = is_symlink(fd,"hints.txt");
    printf("check hints.txt %d\n", ret); //0
    ret = is_symlink(fd,"myfolder/");
    printf("check myfolder/ %d\n", ret); //0
    ret = is_symlink(fd,"myfolder");
    printf("check myfolder %d\n", ret); //0
    ret = is_symlink(fd,"myfolder/mylink.txt");
    printf("check myfolder/mylink.txt %d\n", ret); //0
    ret = is_symlink(fd,"myfolder/mysymlink.txt");
    printf("check myfolder/mysymlink.txt %d\n", ret); //1

    // Tests for list()
    char *arrayofarrays[10];
    char array0[100];
    char array1[100];
    char array2[100];
    char array3[100];
    char array4[100];
    char array5[100];
    char array6[100];
    char array7[100];
    char array8[100];
    char array9[100];
    arrayofarrays[0] = array0;
    arrayofarrays[1] = array1;
    arrayofarrays[2] = array2;
    arrayofarrays[3] = array3;
    arrayofarrays[4] = array4;
    arrayofarrays[5] = array5;
    arrayofarrays[6] = array6;
    arrayofarrays[7] = array7;
    arrayofarrays[8] = array8;
    arrayofarrays[9] = array9;
    size_t* no_entries = (size_t*)malloc(sizeof(size_t));
    *no_entries = 10;
    ret = list(fd,"myfolder/", arrayofarrays, no_entries);
    printf(" -- check myfolder/ %d\n", ret); //1
    for (int i = 0; i < *no_entries; i++) printf("entry %d : %s\n",i,arrayofarrays[i]);
    free(no_entries);

    char ** entries2 = (char**) malloc(sizeof(char*)*20);
    for (int i = 0; i < 20; i++) {
        entries2[i] = (char*) malloc(sizeof(char)*100);
    }

    size_t* no_entries2 = (size_t*)malloc(sizeof(size_t));
    *no_entries2 = 20;
    ret = list(fd,"myfolder/myfolderinfolder/mysecondfolder/symfromdepth", entries2, no_entries2);
    printf(" -- check myfolder/myfolderinfolder/mysecondfolder/symfromdepth %d\n", ret); //1
    for (int i = 0; i < *no_entries2; i++) printf("entry %d : %s\n",i,entries2[i]);


    char ** entries3 = (char**) malloc(sizeof(char*)*20);
    for (int i = 0; i < 20; i++) {
        entries3[i] = (char*) malloc(sizeof(char)*100);
    }

    size_t* no_entries3 = (size_t*)malloc(sizeof(size_t));
    *no_entries3 = 20;
    ret = list(fd,"myfolder/myfolderinfolder/mysecondfolder/symfromdepth2", entries3, no_entries3);
    printf(" -- check myfolder/myfolderinfolder/mysecondfolder/symfromdepth2 %d\n", ret); //1
    for (int i = 0; i < *no_entries3; i++) printf("entry %d : %s\n",i,entries3[i]);

    return 0;
}
