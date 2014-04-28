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

#ifndef _debug_h
#define _debug_h 1

void fatal(char *message);

void *ec_malloc(size_t size);

void *ec_calloc(size_t nmemb, size_t size);

int ec_open(const char *filename, int mode);

void ec_read(const char *filename, uint8_t *buffer, unsigned int bufferlength);

void ec_write(const char *filename, uint8_t *buffer, unsigned int bufferlength);

#endif
