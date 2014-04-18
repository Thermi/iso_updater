CC=LANG=C gcc
RM=rm
CFLAGS=-std=c11 -Wall -ggdb -O0 -fbuiltin -W -Wstrict-prototypes -Wreturn-type -Wsequence-point -pedantic -Wextra -I/usr/include/libxml2/
CFLAGSOPT=-std=c11 -Wall -ggdb -O2 -march=native -fbuiltin -W -Wstrict-prototypes -Wreturn-type -Wsequence-point -pedantic -Wextra -I/usr/include/libxml2/ -fstack-protector-all
TRGT=main.c
LIBS=-lcurl -lxml2 -lgpgme -lassuan -lgpg-error
LIBFILES= md5sum.c debug.c sha1.c protocols.c util.c memory.c openat.c
all:
	$(CC) $(CFLAGSOPT) $(LIBS) $(LIBFILES) -o iso_updater $(TRGT)
build:
	$(CC) $(CFLAGSOPT) $(LIBS) $(LIBFILES) -o iso_updater $(TRGT)
debug:
	$(CC) $(CFLAGS) $(LIBS)  $(TRGT)
clean:
	$(RM) iso_updater