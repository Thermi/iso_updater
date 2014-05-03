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
 * Libraries
 */

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>

#include "memory.h"

/* All the options and their descriptions*/
struct option {
    char *description;
    char *shortOption;
    char *longOption;
};

struct optionsArray {
    size_t length;
    struct option *options;
};

struct options {
    char http;
    char https;
    char ftp;
    char rsync;
    char overwriteExistingFile;
    char signature;
    char *script;
    char *outputpath;
    char *interface;
    char *url;
    /* The xpath inside the html file to the list of elements that contain the directory names, in which the ISOs are */
    xmlChar *xpath;
};

/* This function does the same as strstr(), but takes an additional length argument */
char *strnstr(char *haystack, char* needle, int haystack_length);

/* This functions checks if a given char is equal to any char in the given string */
int strchrs(char character, char *string);

/* This function returns a pointer to the first token of the given string,
 *  seperated by the delimiter. The function does not alter the string. */
char *strntok(char *string, char *delimiter, int length);

/* This function checks a string for conformity to the yyyy.mm.dd format */
int check(char *string);


/* This checks the signature of the file. The signature is in the memory paramter. */
char checkSignature(char *path, struct memory_identifier *memory);

/* This function removes the protocol part preceeding the host and directories from the url
 * The function returns a pointer to a new string, if there was a protocol specified in the url
 * and NULL, if there wasn't.
 */
char *removeProtocolFromURL(char *url);

/* This function prints the help message */
void printHelpMessage(FILE *stream);

/* Prints the licence */
void printLicence(FILE *stream);

/**
 * print_xpath_nodes:
 * @nodes:		the nodes set.
 * @output:		the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output);


/*
 * This function compares two dates and decides where it goes. For sake of performance, this function is not year 10.000 proof!
 * This function orders newer ISOs in front of older ones!
 * The format of url->title is yyyy.mm.dd
 */
int compare(const void *p1, const void *p2);

/* This is a callback function to use with cURL, because we want the HTML file
 * to be stored in memory, not in a file.
 * ptr is the pointer to the data that is passed on to the function by cURL.
 * The length of the memory segment is size*nmemb. The pointer userdata is the
 * pointer that we give to cURL ourselves and is of the type struct memory.
 */
size_t writeDataCallback(const char *ptr, const size_t size, const size_t nmemb, void *userdata);