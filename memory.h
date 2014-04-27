
#ifndef MEMORY_H
#define MEMORY_H
/*
 * Default includes
 */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

/*
 * Just a chunk of memory with a length
 */
struct memory {
    size_t length;
    char *chunk;
};

/* 
 * stores the last chunk, that was used with getlinefrommem()
 */
struct memory_identifier {
    char *chunk; /* pointer to the memory segment */
    size_t length; /* length of the memory segment */
    size_t position; /* current pointer on the memory (like the file pointer on FILE *) */
    uint8_t eom; /* this attribute is set, if the position is at the end of the memory
                  * segment (position == length */
};


/* own implementation of getline() for memory access */
ssize_t getlinefrommem(char **lineptr, size_t  *n, struct memory_identifier *memory);


void initialise_memory_identifiers(void);


struct memory_identifier *create_memory_identifier(void);

/* reset the position pointer and eom flag of the identifier */
int rewind_memory(struct memory_identifier *memory_identifier);

int meof(struct memory_identifier *memory_identifier);


#endif
