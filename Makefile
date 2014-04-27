CC=LANG=C gcc
RM=rm
CFLAGS=-std=c11 -Wall -ggdb -O0 -fbuiltin -W -Wstrict-prototypes -Wreturn-type -Wsequence-point -pedantic -Wextra -I/usr/include/libxml2/
CFLAGSOPT=-std=c11 -Wall -ggdb -O2 -march=native -fbuiltin -W -Wstrict-prototypes -Wreturn-type -Wsequence-point -pedantic -Wextra -I/usr/include/libxml2/ -fstack-protector-all
TRGT=main.c
LIBS=-lcurl -lxml2 -lgpgme -lassuan -lgpg-error
LIBFILES=debug.c sha1.c protocols.c util.c memory.c md5sum.c
all:
	make build
	make check
build:
	$(CC) $(CFLAGSOPT) $(LIBS) $(LIBFILES) -o iso_updater $(TRGT)
check:
	$(CC) $(CFLAGSOPT) $(LIBS) $(LIBFILES) -o check check.c
debug:
	$(CC) $(CFLAGS) $(LIBS) $(TRGT)
clean:
	./remove.sh