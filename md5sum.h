/* Compute checksums of files or strings.
   Copyright (C) 1995-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Ulrich Drepper <drepper@gnu.ai.mit.edu>.  */

#ifndef MD5SUM_H
#define	MD5SUM_H

#ifdef	__cplusplus
extern "C" {
#endif
#define HASH_ALGO_SHA1


#include <sys/types.h>



#include "sha1.h"
#include "memory.h"

#define DIGEST_TYPE_STRING "SHA1"
#define DIGEST_STREAM sha1_stream
#define DIGEST_BITS 160
#define DIGEST_REFERENCE "FIPS-180-1"
#define DIGEST_ALIGN 4


#define _GL_ATTRIBUTE_PURE __attribute__ ((__pure__))

#define O_BINARY 0

#define DIGEST_HEX_BYTES (DIGEST_BITS / 4)
#define DIGEST_BIN_BYTES (DIGEST_BITS / 8)

/* The minimum length of a valid digest line.  This length does
   not include any newline character at the end of a line.  */
#define MIN_DIGEST_LINE_LENGTH \
  (DIGEST_HEX_BYTES /* length of hexadecimal message digest */ \
   + 1 /* blank */ \
   + 1 /* minimum filename length */ )
/* The minimum length of a valid checksum line for the selected algorithm.  */
size_t min_digest_line_length;

/* Set to the length of a digest hex string for the selected algorithm.  */
size_t digest_hex_bytes;

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
enum {
    STATUS_OPTION = CHAR_MAX + 1,
    QUIET_OPTION,
    STRICT_OPTION,
    TAG_OPTION
};


#define ISWHITE(c) ((c) == ' ' || (c) == '\t')

/* Given a file name, S of length S_LEN, that is not NUL-terminated,
   modify it in place, performing the equivalent of this sed substitution:
   's/\\n/\n/g;s/\\\\/\\/g' i.e., replacing each "\\n" string with a newline
   and each "\\\\" with a single backslash, NUL-terminate it and return S.
   If S is not a valid escaped file name, i.e., if it ends with an odd number
   of backslashes or if it contains a backslash followed by anything other
   than "n" or another backslash, return NULL.  */

char *filename_unescape(char *s, size_t s_len);

/* Split the checksum string S (of length S_LEN) from a BSD 'md5' or
   'sha1' command into two parts: a hexadecimal digest, and the file
   name.  S is modified.  Return true if successful.  */

bool
bsd_split_3(char *s, size_t s_len, unsigned char **hex_digest,
        char **file_name, bool escaped_filename);

/* Split the string S (of length S_LEN) into three parts:
   a hexadecimal digest, binary flag, and the file name.
   S is modified.  Return true if successful.  */

bool
split_3(char *s, size_t s_len,
        unsigned char **hex_digest, int *binary, char **file_name);

/* Return true if S is a NUL-terminated string of DIGEST_HEX_BYTES hex digits.
   Otherwise, return false.  */
bool _GL_ATTRIBUTE_PURE
hex_digits(unsigned char const *s);

/* An interface to the function, DIGEST_STREAM.
   Operate on FILENAME (it may be "-").

 *BINARY indicates whether the file is binary.  BINARY < 0 means it
   depends on whether binary mode makes any difference and the file is
   a terminal; in that case, clear *BINARY if the file was treated as
   text because it was a terminal.

   Put the checksum in *BIN_RESULT, which must be properly aligned.
   Return true if successful.  */

bool
digest_file(const char *filename, int *binary, unsigned char *bin_result);

bool
digest_check(char *path, struct memory_identifier *memory);

/* If ESCAPE is true, then translate each NEWLINE byte to the string, "\\n",
   and each backslash to "\\\\".  */
void
print_filename(char const *file, bool escape);


#ifdef	__cplusplus
}
#endif

#endif	/* MD5SUM_H */

