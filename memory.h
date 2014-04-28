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
