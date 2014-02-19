/*
 * (c) 2014 Oliver Gebhardt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "debug.h"


/*
 * If a fatal error occurs, this function is called and the program exits.
 *
 */
void fatal(char *message) {
    char error_message[100];

    strcpy(error_message, "[!!] Fatal Error ");
    strncat(error_message, message, 83);
    perror(error_message);
    exit(-1);
}

/*
 * Memory allocatio with error handling
 * malloc + calloc
 */
void *ec_malloc(unsigned int size) {
    void *ptr;
    ptr = malloc(size);
    if (ptr == NULL) {
        fatal("in ec_malloc() on memory allocation");
    }
    return ptr;
}
void *ec_calloc(unsigned int size) {
    void *ptr;
    ptr = calloc(0, size);
    if (ptr == NULL) {
        fatal("in ec_calloc() on memory allocation");
    }
    return ptr;
}

/*
 * File handling with error handling
 * further reading:
 * man 3 open
 * man 3 close
 * man 3 read
 * man 3 write
 *
 * Write con tent of the file into the buffer and vice versa
 */
int ec_open(const char *filename, int mode) {
    int fd;
    fd = open(filename, mode);
    if (fd == -1) {
        fatal("in ec_open() on opening file");
    }
    return fd;
}

void ec_read(const char *filename, uint8_t *buffer, unsigned int bufferlength) {
    int fd;
    ssize_t len;

    fd = ec_open(filename, O_RDONLY);

    len = read(fd, buffer, bufferlength);

    if (len == -1) {
        fatal("in ec_read() on reading file");
    }
    close(fd);
}

void ec_write(const char *filename, uint8_t *buffer, unsigned int bufferlength) {
    int fd;
    ssize_t len;

    fd = ec_open(filename, O_WRONLY);

    len = write(fd, buffer, bufferlength);

    if (len == -1) {
        fatal("in ec_write() on writing file");
    }
    close(fd);
}
