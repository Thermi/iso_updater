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
 * Libraries
 */
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <gpgme.h>

#include "debug.h"
#include "sha1.h"

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
ssize_t getlinefrommem(char **lineptr, size_t *n, struct memory_identifier *memory)
{
    if (memory->position == memory->length || memory->eom)
        return 0;

    size_t i = memory->position;

    for (; i < memory->length; i++) {
        if (memory->chunk[i] == L'\n' || memory->chunk[i] == '\n' || memory->chunk[i] == '\0') {
            break;
        }
    }

    memset(*lineptr, 0, *n);
    if (*lineptr != NULL)
        free(*lineptr);

    *lineptr = malloc(i + 1 - memory->position);
    // *lineptr = realloc(*lineptr, i + 1 - memory->position);
    *n = i + 1 - memory->position;

    memcpy(*lineptr, memory->chunk + memory->position, i - memory->position);
    (*lineptr)[i - memory->position] = '\0';
    memory->position = i + 1;
    if (i == memory->length)
        memory->eom = 1;

    return i-memory->position;
}

struct memory_identifier *create_memory_identifier(void)
{
    struct memory_identifier *memory_identifier = malloc(sizeof (struct memory_identifier));
    if (memory_identifier == NULL)
        return NULL;

    memory_identifier->chunk = NULL;
    memory_identifier->eom = false;
    memory_identifier->length = 0;
    memory_identifier->position = 0;

    return memory_identifier;
}

int rewind_memory(struct memory_identifier *memory_identifier)
{
    if (memory_identifier == NULL)
        return false;

    memory_identifier->position = 0;
    memory_identifier->eom = false;
    return true;
}

int meof(struct memory_identifier *memory_identifier)
{
    return memory_identifier->eom;
}
