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
/* Heavily modifed and abused by Noel Kuntze <noel@familie-kuntze.de>. */

#define HASH_ALGO_SHA1

#include <sys/types.h>

#include <fcntl.h>
#include <error.h>
#include <libintl.h>
#include <libgen.h>

#include "sha1.h"
#include "memory.h"
#include "util.h"

#define DIGEST_TYPE_STRING "SHA1"
#define DIGEST_STREAM sha1_stream
#define DIGEST_BITS 160
#define DIGEST_REFERENCE "FIPS-180-1"
#define DIGEST_ALIGN 4

#define STREQ_LEN(a, b, n) (strncmp (a, b, n) == 0)

#define _GL_ATTRIBUTE_PURE __attribute__ ((__pure__))

#define O_BINARY 0

#define PRIuMAX "ju"

#define DIGEST_HEX_BYTES (DIGEST_BITS / 4)
#define DIGEST_BIN_BYTES (DIGEST_BITS / 8)


/* The minimum length of a valid digest line.  This length does
   not include any newline character at the end of a line.  */
#define MIN_DIGEST_LINE_LENGTH \
  (DIGEST_HEX_BYTES /* length of hexadecimal message digest */ \
   + 1 /* blank */ \
   + 1 /* minimum filename length */ )


/* The minimum length of a valid checksum line for the selected algorithm.  */
size_t min_digest_line_length = MIN_DIGEST_LINE_LENGTH;

/* Set to the length of a digest hex string for the selected algorithm.  */
size_t digest_hex_bytes = DIGEST_HEX_BYTES;

#define ISWHITE(c) ((c) == ' ' || (c) == '\t')

/* Given a file name, S of length S_LEN, that is not NUL-terminated,
   modify it in place, performing the equivalent of this sed substitution:
   's/\\n/\n/g;s/\\\\/\\/g' i.e., replacing each "\\n" string with a newline
   and each "\\\\" with a single backslash, NUL-terminate it and return S.
   If S is not a valid escaped file name, i.e., if it ends with an odd number
   of backslashes or if it contains a backslash followed by anything other
   than "n" or another backslash, return NULL.  */

void *
ptr_align(void const *ptr, size_t alignment)
{
    char const *p0 = ptr;
    char const *p1 = p0 + alignment - 1;
    return (void *) (p1 - (size_t) p1 % alignment);
}

unsigned long int
select_plural(uintmax_t n)
{

    /* Reduce by a power of ten, but keep it away from zero.  The
       gettext manual says 1000000 should be safe.  */
    enum {
        PLURAL_REDUCER = 1000000
    };
    return (n <= ULONG_MAX ? n : n % PLURAL_REDUCER + PLURAL_REDUCER);
}

char *
filename_unescape(char *s, size_t s_len)
{
    char *dst = s;

    for (size_t i = 0; i < s_len; i++) {
        switch (s[i]) {
        case '\\':
            if (i == s_len - 1) {
                /* File name ends with an unescaped backslash: invalid.  */
                return NULL;
            }
            ++i;
            switch (s[i]) {
            case 'n':
                *dst++ = '\n';
                break;
            case '\\':
                *dst++ = '\\';
                break;
            default:
                /* Only '\' or 'n' may follow a backslash.  */
                return NULL;
            }
            break;

        case '\0':
            /* The file name may not contain a NUL.  */
            return NULL;

        default:
            *dst++ = s[i];
            break;
        }
    }
    if (dst < s + s_len)
        *dst = '\0';

    return s;
}

/* Split the checksum string S (of length S_LEN) from a BSD 'md5' or
   'sha1' command into two parts: a hexadecimal digest, and the file
   name.  S is modified.  Return true if successful.  */

bool
bsd_split_3(char *s, size_t s_len, unsigned char **hex_digest,
        char **file_name, bool escaped_filename)
{
    size_t i;

    if (s_len == 0)
        return false;

    /* Find end of filename.  */
    i = s_len - 1;
    while (i && s[i] != ')')
        i--;

    if (s[i] != ')')
        return false;

    *file_name = s;

    if (escaped_filename && filename_unescape(s, i) == NULL)
        return false;

    s[i++] = '\0';

    while (ISWHITE(s[i]))
        i++;

    if (s[i] != '=')
        return false;

    i++;

    while (ISWHITE(s[i]))
        i++;

    *hex_digest = (unsigned char *) &s[i];
    return true;
}

/* Split the string S (of length S_LEN) into three parts:
   a hexadecimal digest, binary flag, and the file name.
   S is modified.  Return true if successful.  */

bool
split_3(char *s, size_t s_len,
        unsigned char **hex_digest, int *binary, char **file_name)
{
    bool escaped_filename = false;
    size_t algo_name_len;

    size_t i = 0;
    while (ISWHITE(s[i]))
        ++i;

    if (s[i] == '\\') {
        ++i;
        escaped_filename = true;
    }

    /* Check for BSD-style checksum line. */

    algo_name_len = strlen(DIGEST_TYPE_STRING);
    if (STREQ_LEN(s + i, DIGEST_TYPE_STRING, algo_name_len)) {
        if (s[i + algo_name_len] == ' ')
            ++i;
        if (s[i + algo_name_len] == '(') {
            *binary = 0;
            return bsd_split_3(s + i + algo_name_len + 1,
                    s_len - (i + algo_name_len + 1),
                    hex_digest, file_name, escaped_filename);
        }
    }

    /* Ignore this line if it is too short.
       Each line must have at least 'min_digest_line_length - 1' (or one more, if
       the first is a backslash) more characters to contain correct message digest
       information.  */
    if (s_len - i < min_digest_line_length + (s[i] == '\\')) {
        return false;
        printf("Digest is too short!\n");
    }

    *hex_digest = (unsigned char *) &s[i];

    /* The first field has to be the n-character hexadecimal
       representation of the message digest.  If it is not followed
       immediately by a white space it's an error.  */
    i += digest_hex_bytes;
    if (!ISWHITE(s[i])) {
        printf("The digest is not followed by a white space!\n");
        return false;
    }

    s[i++] = '\0';

    *binary = (s[i++] == '*');

    /* All characters between the type indicator and end of line are
       significant -- that includes leading and trailing white space.  */
    *file_name = &s[i];

    if (escaped_filename)
        return filename_unescape(&s[i], s_len - i) != NULL;

    return true;
}

/* Return true if S is a NUL-terminated string of DIGEST_HEX_BYTES hex digits.
   Otherwise, return false.  */
bool _GL_ATTRIBUTE_PURE
hex_digits(unsigned char const *s)
{
    unsigned int i;
    for (i = 0; i < digest_hex_bytes; i++) {
        if (!isxdigit(*s))
            return false;
        ++s;
    }
    return *s == '\0';
}

/* An interface to the function, DIGEST_STREAM.
   Operate on FILENAME (it may be "-").

 *BINARY indicates whether the file is binary.  BINARY < 0 means it
   depends on whether binary mode makes any difference and the file is
   a terminal; in that case, clear *BINARY if the file was treated as
   text because it was a terminal.

   Put the checksum in *BIN_RESULT, which must be properly aligned.
   Return true if successful.  */

bool
digest_file(const char *filename, int *binary, unsigned char *bin_result)
{
    FILE *fp;
    int err;

    fp = fopen(filename, (O_BINARY && *binary ? "rb" : "r"));
    if (fp == NULL) {
        error(0, errno, "%s", filename);
        return false;

    }

    err = DIGEST_STREAM(fp, bin_result);
    if (err) {
        error(0, errno, "%s", filename);
        fclose(fp);
        return false;
    }

    return true;
}

bool
digest_check(char *path, struct memory_identifier *memory)
{
    uintmax_t n_misformatted_lines = 0;
    uintmax_t n_properly_formatted_lines = 0;
    uintmax_t n_improperly_formatted_lines = 0;
    uintmax_t n_mismatched_checksums = 0;
    uintmax_t n_open_or_read_failures = 0;
    unsigned char bin_buffer_unaligned[DIGEST_BIN_BYTES + DIGEST_ALIGN];
    /* Make sure bin_buffer is properly aligned. */
    unsigned char *bin_buffer = ptr_align(bin_buffer_unaligned, DIGEST_ALIGN);
    uintmax_t line_number;
    char *line;
    size_t line_chars_allocated;

    line_number = 0;
    line = NULL;
    line_chars_allocated = 0;
    do {
        char *filename = NULL;
        int binary;
        unsigned char *hex_digest = NULL;
        ssize_t line_length;

        ++line_number;
        if (line_number == 0)
            error(EXIT_FAILURE, 0, "%s: too many checksum lines",
                "iso_updater");

        line_length = getlinefrommem(&line, &line_chars_allocated, memory);
        if (line_length <= 0)
            break;

        /* Ignore comment lines, which begin with a '#' character.  */
        if (line[0] == '#')
            continue;

        /* Remove any trailing newline.  */
        if (line[line_length - 1] == '\n')
            line[--line_length] = '\0';

        if (!(split_3(line, line_length, &hex_digest, &binary, &filename)
                && hex_digits(hex_digest))) {
            ++n_misformatted_lines;
            ++n_improperly_formatted_lines;
            printf("misformatted lines: %ju\n", n_misformatted_lines);
            printf("improperly formatted lines: %ju\n", n_improperly_formatted_lines);
        } else {
            const char bin2hex[] = {'0', '1', '2', '3',
                '4', '5', '6', '7',
                '8', '9', 'a', 'b',
                'c', 'd', 'e', 'f'};
            bool ok;

            ++n_properly_formatted_lines;

            if (!strcmp(filename, basename(path))) {
                ok = digest_file(path, &binary, bin_buffer);

                if (!ok) {
                    ++n_open_or_read_failures;
                    printf("%s: FAILED open or read\n", path);
                } else {
                    size_t digest_bin_bytes = digest_hex_bytes / 2;
                    size_t cnt;
                    /* Compare generated binary number with text representation
                       in check file.  Ignore case of hex digits.  */
                    for (cnt = 0; cnt < digest_bin_bytes; ++cnt) {
                        if (tolower(hex_digest[2 * cnt])
                                != bin2hex[bin_buffer[cnt] >> 4]
                                || (tolower(hex_digest[2 * cnt + 1])
                                != (bin2hex[bin_buffer[cnt] & 0xf])))
                            break;
                    }
                    if (cnt != digest_bin_bytes)
                        ++n_mismatched_checksums;

                    if (cnt != digest_bin_bytes) {
                        printf("Count: %zi\n", cnt);
                        printf("%s: %s\n", path, "FAILED");
                        free(line);
                        return 1;
                    } else {
                        printf("Count: %zi\n", cnt);
                        printf("%s: %s\n", path, "OK");
                        free(line);
                        return 0;
                    }

                }
            }
        }
    } while (!meof(memory));

    free(line);

    if (n_properly_formatted_lines == 0) {
        /* Warn if no tests are found.  */
        error(0, 0, "%s: no properly formatted %s checksum lines found",
                "iso_updater", DIGEST_TYPE_STRING);
    } else {
        if (n_misformatted_lines != 0)
            error(0, 0,
                (ngettext
                ("WARNING: %" PRIuMAX " line is improperly formatted",
                "WARNING: %" PRIuMAX " lines are improperly formatted",
                select_plural(n_misformatted_lines))),
                n_misformatted_lines);

        if (n_open_or_read_failures != 0)
            error(0, 0,
                (ngettext
                ("WARNING: %" PRIuMAX " listed file could not be read",
                "WARNING: %" PRIuMAX " listed files could not be read",
                select_plural(n_open_or_read_failures))),
                n_open_or_read_failures);

        if (n_mismatched_checksums != 0)
            error(0, 0,
                (ngettext
                ("WARNING: %" PRIuMAX " computed checksum did NOT match",
                "WARNING: %" PRIuMAX " computed checksums did NOT match",
                select_plural(n_mismatched_checksums))),
                n_mismatched_checksums);

    }

    /* This returns 0, if all lines were properly formatted and > 0, if anything else happened.*/
    return (n_properly_formatted_lines != 0
            && n_mismatched_checksums == 0
            && n_open_or_read_failures == 0
            && (n_improperly_formatted_lines == 0));
}

/* If ESCAPE is true, then translate each NEWLINE byte to the string, "\\n",
   and each backslash to "\\\\".  */
void
print_filename(char const *file, bool escape)
{
    if (!escape) {
        fputs(file, stdout);
        return;
    }

    while (*file) {
        switch (*file) {
        case '\n':
            fputs("\\n", stdout);
            break;

        case '\\':
            fputs("\\\\", stdout);
            break;

        default:
            putchar(*file);
            break;
        }
        file++;
    }
}