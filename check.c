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
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>


#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "memory.h"
#include "md5sum.h"

int main(void)
{

    char *foo =
            "b7d7d124bc4f82d1df55c17029def67dfa0e5e64  archlinux-2014.04.01-dual.iso\n\
9a10054cfcafc61e522a3a1d61577d21e77269a0  archlinux-bootstrap-2014.04.01-i686.tar.gz\n\
7e396e01c443753fb375c5e146bbed1948698eec  archlinux-bootstrap-2014.04.01-x86_64.tar.gz";

    struct memory_identifier memory_identifier = (struct memory_identifier){
        foo, strlen(foo) + 1, 0, 0
    };

    bool bar = digest_check("archlinux-2014.04.01-dual.iso", &memory_identifier);

    switch (bar) {
    case 0: printf("File was checked successfully.\n");
        break;
    case 1: printf("File failed the check.\n");
        break;
        return 0;
    }
}
