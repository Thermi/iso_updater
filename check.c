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
