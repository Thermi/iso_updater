/*
 * Copyright (C) 2014 Noel Kuntze <noel@familie-kuntze.de>
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>


/*
 * If a fatal error occurs, this function is called and the program exits.
 *
 */
void fatal(char *message) {
    char *prefix ="[!!] Fatal Error: ", error_message[strlen(prefix)+strlen(message)+1];
    strcpy(error_message, prefix);
    strcat(error_message, message);
    perror(error_message);
    exit(-1);
}

/*
 * Memory allocatio with error handling
 * malloc + calloc
 */
void *ec_malloc(size_t size) {
    void *ptr;
    ptr = malloc(size);
    if (ptr == NULL) {
        fatal("in ec_malloc() on memory allocation.\n");
    }
    return ptr;
}

void *ec_calloc(size_t nmemb, size_t size) {
    void *ptr;
    ptr = calloc(nmemb, size);
    if (ptr == NULL) {
        fatal("in ec_calloc() on memory allocation.\n");
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
 * Write content of the file into the buffer and vice versa
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
