#include "lib_tar.h"

int at_end_of_archive(int tar_fd) {
    char block[1024];
    if (read(tar_fd, &block, 1024) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
    for (int i = 0; i < 1024; i++) {
      if (block[i] != '\0') return 0;
    }
    return 1;
}

/**
 * Checks whether the archive is valid.
 *
 * Each non-null header of a valid archive has:
 *  - a magic value of "ustar" and a null,
 *  - a version value of "00" and no null,
 *  - a correct checksum
 *
 * @param tar_fd A file descriptor pointing to the start of a file supposed to contain a tar archive.
 *
 * @return a zero or positive value if the archive is valid, representing the number of non-null headers in the archive,
 *         -1 if the archive contains a header with an invalid magic value,
 *         -2 if the archive contains a header with an invalid version value,
 *         -3 if the archive contains a header with an invalid checksum value
 */
int check_archive(int tar_fd) {
    lseek(tar_fd,0,SEEK_SET);
    unsigned int atSize = 0;
    while (at_end_of_archive(tar_fd) == 0) {
        char name[100];
        lseek(tar_fd,atSize,SEEK_SET);
        if (read(tar_fd, &name, 100) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        printf("%s\n", name);


        //Check magic value
        lseek(tar_fd, atSize+257, SEEK_SET);
        char magic[TMAGLEN];
        if (read(tar_fd, &magic, TMAGLEN) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        if (strcmp(magic, TMAGIC) != 0) return -1;

        //Check version value
        char version[TVERSLEN];
        if (read(tar_fd, &version, TVERSLEN) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        if (strncmp(version, TVERSION, TVERSLEN) != 0) return -2;

        //Check checksum
        char header[512];
        lseek(tar_fd, atSize, SEEK_SET);
        if (read(tar_fd, &header, 512) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        unsigned int chksumCounter = 0;
        for (int i = 0; i < 512; i++) {
            if (i >= 148 && i < 156)  chksumCounter += (unsigned int) ' ';
            else {
                chksumCounter += (unsigned int) header[i];
            }
        }

        lseek(tar_fd, atSize+148, SEEK_SET);
        char chksum[8];
        if (read(tar_fd, &chksum, 8) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        if (TAR_INT(chksum) != chksumCounter) return -3;

        //Go to next header
        char size[12];
        lseek(tar_fd, atSize+124, SEEK_SET);
        if (read(tar_fd, &size, 12) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        unsigned int fileSize = 512+(512*((TAR_INT(size)/512)+(TAR_INT(size)%512 != 0)));
        atSize += fileSize;
        lseek(tar_fd, atSize, SEEK_SET);
    }

    return 0;
}

/**
 * Checks whether an entry exists in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive,
 *         any other value otherwise.
 */
int exists(int tar_fd, char *path) {
    lseek(tar_fd,0,SEEK_SET);
    unsigned int atSize = 0;
    while(at_end_of_archive(tar_fd) == 0){
        char name[100];
        lseek(tar_fd,atSize,SEEK_SET);
        if (read(tar_fd, &name, 100) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        if (strcmp(name, path) == 0) return 1;

        char size[12];
        lseek(tar_fd, atSize+124, SEEK_SET);
        if (read(tar_fd, &size, 12) == -1) fprintf(stderr, "read failed, errno = %d\n", errno);
        unsigned int fileSize = 512+(512*((TAR_INT(size)/512)+(TAR_INT(size)%512 != 0)));
        atSize += fileSize;
        lseek(tar_fd, atSize, SEEK_SET);
    }
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a directory.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a directory,
 *         any other value otherwise.
 */
int is_dir(int tar_fd, char *path) {
    exists(tar_fd, path);
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a file.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 *
 * @return zero if no entry at the given path exists in the archive or the entry is not a file,
 *         any other value otherwise.
 */
int is_file(int tar_fd, char *path) {
    return 0;
}

/**
 * Checks whether an entry exists in the archive and is a symlink.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive.
 * @return zero if no entry at the given path exists in the archive or the entry is not symlink,
 *         any other value otherwise.
 */
int is_symlink(int tar_fd, char *path) {
    return 0;
}


/**
 * Lists the entries at a given path in the archive.
 * list() does not recurse into the directories listed at the given path.
 *
 * Example:
 *  dir/          list(..., "dir/", ...) lists "dir/a", "dir/b", "dir/c/" and "dir/e/"
 *   ├── a
 *   ├── b
 *   ├── c/
 *   │   └── d
 *   └── e/
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive. If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param entries An array of char arrays, each one is long enough to contain a tar entry path.
 * @param no_entries An in-out argument.
 *                   The caller set it to the number of entries in `entries`.
 *                   The callee set it to the number of entries listed.
 *
 * @return zero if no directory at the given path exists in the archive,
 *         any other value otherwise.
 */
int list(int tar_fd, char *path, char **entries, size_t *no_entries) {
    return 0;
}

/**
 * Reads a file at a given path in the archive.
 *
 * @param tar_fd A file descriptor pointing to the start of a valid tar archive file.
 * @param path A path to an entry in the archive to read from.  If the entry is a symlink, it must be resolved to its linked-to entry.
 * @param offset An offset in the file from which to start reading from, zero indicates the start of the file.
 * @param dest A destination buffer to read the given file into.
 * @param len An in-out argument.
 *            The caller set it to the size of dest.
 *            The callee set it to the number of bytes written to dest.
 *
 * @return -1 if no entry at the given path exists in the archive or the entry is not a file,
 *         -2 if the offset is outside the file total length,
 *         zero if the file was read in its entirety into the destination buffer,
 *         a positive value if the file was partially read, representing the remaining bytes left to be read to reach
 *         the end of the file.
 *
 */
ssize_t read_file(int tar_fd, char *path, size_t offset, uint8_t *dest, size_t *len) {
    return 0;
}
