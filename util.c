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
 * TODO: Implement own version of getline() for memory access and replace all
 * calls of getline() with calls to the own version
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
#include "memory.h"
#include "md5sum.h"

#define ISO_MAJORVERSION 0
#define ISO_MINORVERSION 1
#define ISO_VERSION ISO_MAJORVERSION.ISO_MINORVERSION

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
    char ftp;
    char http;
    char https;
    char rsync;
    char overwriteExistingFile;
    char signature;
    char verbose;
    char *interface;
    char *outputpath;
    char *script;
    char *url;
    /* The xpath inside the html file to the list of elements that contain the directory names, in which the ISOs are */
    xmlChar *xpath;
};

/* This function does the same as strstr(), but takes an additional length argument */
char *strnstr(char *haystack, char* needle, int haystack_length)
{
    if (strlen(needle) >= (size_t) haystack_length)
        return NULL;

    int i, j, k, needle_length = strlen(needle), length = haystack_length - needle_length;
    char *ptr = haystack;

    /* outer loop to go over the whole haystack */
    for (i = 0; i < length; i++) {
        k = 0;
        /* inner loop to check the seperate characters of the needle */
        for (j = 0; j < needle_length; j++) {
            if (ptr[j]
                    == needle[j])
                k++;
            else
                break;
            if (k == needle_length)
                return ptr;
        }
        ptr++;
    }
    return NULL;
}

/* This functions checks if a given char is equal to any char in the given string */
int strchrs(char character, char *string)
{
    int i, length = strlen(string);
    for (i = 0; i < length; i++) {
        if (character == string[i])
            return 1;
    }
    return 0;
}

/* This function returns a pointer to the first token of the given string,
 *  seperated by the delimiter. The function does not alter the string. */
char *strntok(char *string, char delimiter, int length)
{
    int i, ret, lastCharDelimiter = 0;

    for (i = 0; i < length; i++) {
        ret = strchrs(delimiter, string + i);
        /* If the current character is a delimiter, set the corresponding flag */
        if (ret)
            lastCharDelimiter = 1;
        /* If the current character isn't a delimiter, but the last was, return the pointer.*/
        if (!ret && lastCharDelimiter)
            return string + i;
    }
    return NULL;

}

/* This function checks a string for conformity to the yyyy.mm.dd format */
int check(char *string)
{

    int i;

    if (strlen(string) != 10)
        return 1;

    /* check the position of the dots */
    if (string[4] != '.' || string[7] != '.')
        return 1;
    /* check the year */
    for (i = 0; i < 4; i++)
        if (!isdigit(string[i]))
            return 1;

    /* check the month */
    if (!isdigit(string[5]) || !isdigit(string[6]))
        return 1;
    /* check the day */
    if (!isdigit(string[8]) || !isdigit(string[9]))
        return 1;
    return 0;
}

int meom(struct memory_identifier *memory)
{
    if (memory == NULL)
        return false;

    return (int) memory->eom;
}

int mrewind(struct memory_identifier *memory)
{
    if (memory == NULL)
        return false;

    memory->position = 0;
    return true;
}

char checkSignature(char *path, struct memory_identifier *memory)
{

    gpgme_ctx_t context;
    gpgme_error_t err;
    gpgme_data_t signature, iso;

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        fprintf(stderr, "File %s not found!\n", path);
        fatal("Unable to open file!\n");
    }

    gpgme_check_version(NULL);

    err = gpgme_new(&context);

    switch (err) {

    case GPG_ERR_NO_ERROR: break;

    case GPG_ERR_ENOMEM: fatal("There is not enough memory available to initialize gpgme. Aborting...\n");

    default: break;

    }

    gpgme_set_protocol(context, GPGME_PROTOCOL_OpenPGP);
    err = gpgme_data_new_from_stream(&iso, file);

    if (err == GPG_ERR_ENOMEM)
        fatal("There was not enough memory to crate a new data buffer for gpgme. Aborting...\n");

    err = gpgme_ctx_set_engine_info(context, GPGME_PROTOCOL_OpenPGP, "/usr/bin/gpg",
            "/etc/pacman.d/gnupg/");
    if (err != GPG_ERR_NO_ERROR) {
        printf("Something went wrong while setting the engine info.\n");
        return 1;
    }

    err = gpgme_data_new_from_mem(&signature, memory->chunk, memory->length, 1);

    if (err == GPG_ERR_ENOMEM)
        fatal("There was not enough memory to crate a new data buffer for gpgme. Aborting...\n");

    err = gpgme_op_verify(context, signature, iso, NULL);

    if (err == GPG_ERR_NO_ERROR)
        return 0;
    else
        return 1;
}

/* Prints the licence */
void printLicence(FILE *stream)
{
    fprintf(stream, "iso_updater %d.%d \n", ISO_MAJORVERSION, ISO_MINORVERSION);
    fprintf(stream, "Copyright (C) 2014 Noel Kuntze <noel@familie-kuntze.de>\n");
    fprintf(stream, "Licence GLPv3+: GNU GPL version 3 <http://gnu.org/licenses/gpl.html>.");
    fprintf(stream, "This is free software: You are free to change and redistribute it.\n");
    fprintf(stream, "There is NO WARRANTY, to the extent permitted by law.\n");
}

/* This function prints the help message */
void printHelpMessage(FILE *stream)
{
    size_t i;

    struct optionsArray optionsArray;
    optionsArray.options = (struct option[]){
        {"URL to the host.", "-u <URL>", NULL},
        {"Path to the host where the iso should be saved to.", "-o <path>", NULL},
        {"Xpath to the attributes in the HTML file, that contains the list of files.", "-X <xpath>", NULL},
        {"Path to a script that should be executed when the ISO has been downloaded.", "-e <script>", NULL},
        {"The interface that cURL should use to initiate the connection.", "-i <interface>", NULL},
        {"Makes the program overwrite existing files.", "-f", NULL},
        {"Tells the program to check the signature of the file.", "-s", NULL},
        {"Indicates http support by the URL.", "--http", NULL},
        {"Indicates https support by the URL.", NULL, "--https"},
        {"Indicates ftp support by the URL.", NULL, "--ftp"},
        {"Indicates rsync support by the URL.", NULL, "--rsync"}
    };
    optionsArray.length = 11;

    fprintf(stream, "Known parameters for this program:\n");
    for (i = 0; i < optionsArray.length; i++) {
        if (optionsArray.options[i].shortOption != NULL) {
            fprintf(stream, "%s - ", optionsArray.options[i].shortOption);
        }
        if (optionsArray.options[i].longOption != NULL) {
            fprintf(stream, "%s - ", optionsArray.options[i].longOption);
        }
        if (optionsArray.options[i].description != NULL) {
            fprintf(stream, "%s\n", optionsArray.options[i].description);
        }
    }
}

/**
 * print_xpath_nodes:
 * @nodes:		the nodes set.
 * @output:		the output file handle.
 *
 * Prints the @nodes content to @output.
 */
void print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output)
{
    xmlNodePtr cur;
    int size;
    int i;

    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;

    fprintf(output, "Result (%d nodes):\n", size);
    for (i = 0; i < size; ++i) {
        assert(nodes->nodeTab[i]);

        if (nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
            xmlNsPtr ns;

            ns = (xmlNsPtr) nodes->nodeTab[i];
            cur = (xmlNodePtr) ns->next;
            if (cur->ns) {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n",
                        ns->prefix, ns->href, cur->ns->href, cur->name);
            } else {
                fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n",
                        ns->prefix, ns->href, cur->name);
            }
        } else if (nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
            cur = nodes->nodeTab[i];
            if (cur->ns) {
                fprintf(output, "= element node \"%s:%s\"\n",
                        cur->ns->href, cur->name);
            } else {
                fprintf(output, "= element node \"%s\"\n",
                        cur->name);
            }
        } else {
            cur = nodes->nodeTab[i];
            fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
        }
    }
}

/* This function removes the protocol part preceeding the host and directories from the url
 * The function returns a pointer to the part after the protocol, if there was
 * a protocol specified in the url and NULL, if there wasn't.
 */
char *removeProtocolFromURL(char *url)
{
    char *ptr = strstr(url, "://");
    if (ptr == NULL)
        return ptr;
    return ptr + 3;
}

/* This is a callback function to use with cURL, because we want the HTML file
 * to be stored in memory, not in a file.
 * ptr is the pointer to the data that is passed on to the function by cURL.
 * The length of the memory segment is size*nmemb. The pointer userdata is the
 * pointer that we give to cURL ourselves and is of the type struct memory.
 */
size_t writeDataCallback(const char *ptr, const size_t size, const size_t nmemb, void *userdata)
{
    struct memory_identifier *memory = userdata;
    /* Calculate the size of the new chunk */
    size_t newlength = size * nmemb + memory->length;
    /* Get a longer memory segment */
    char *newchunk = malloc(newlength);

    /* This is to check if it is the first time the function is run by cURL */
    if (memory->chunk != NULL) {
        /* Copy the content of the old chunk into the new chunk */
        memcpy(newchunk, memory->chunk, memory->length);
        memset(memory->chunk, 0, memory->length);
        free(memory->chunk);
    }

    /* Concatenate the new data onto the old one */
    memcpy(newchunk + memory->length, ptr, size * nmemb);

    memory->chunk = newchunk;
    memory->length = newlength;
    return size*nmemb;
}