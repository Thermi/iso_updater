#
# Copyright (C) 2014 Noel Kuntze <noel@familie-kuntze.de>
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as published by
# the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

CC=LANG=C gcc
RM=rm
CFLAGSDEBUG=-std=c11 -Wall -ggdb -O2 -march=native -fbuiltin -W -Wstrict-prototypes -Wreturn-type -Wsequence-point -pedantic -Wextra -I/usr/include/libxml2/ -fstack-protector-all -p
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
	$(CC) $(CFLAGSDEBUG) $(LIBS) $(LIBFILES) -o iso_updater $(TRGT)
clean:
	./remove.sh