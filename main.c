/*
 * File:   main.c
 * Author: thermi
 *
 * Created on 16. Februar 2014, 23:39
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

/*
 * Libraries
 */

#include <curl/curl.h>
#include <expat.h>

#include "debug.h"

struct memory {
    uint8_t *chunk;
    size_t length;
};

/* This is a callback function to use with cURL, because we want the HTML file
 * to be stored in memory, not in a file.
 * ptr is the pointer to the data that is passed on to the function by cURL.
 * The length of the memory segment is size*nmemb. The pointer userdata is the
 * pointer that we give to cURL ourselves and is of the type struct memory.
 */
size_t writeDataCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{

    struct memory *memory = userdata;

    /* Calculate the size of the new chunk */
    size_t newlength = size * nmemb + (*memory).length;
    /* Get a longer memory segment */
    uint8_t *newchunk = malloc(newlength);

    /* This is to check if it is the first time the function is run by cURL */
    if ((*memory).chunk != NULL) {
        /* Copy the content of the old chunk into the new chunk */
        memcpy(newchunk, (*memory).chunk, (*memory).length);
        free((*memory).chunk);
    }
    /* Concatenate the new data onto the old one */
    memcpy(newchunk + (*memory).length, ptr, size * nmemb);

    (*memory).chunk = newchunk;
    (*memory).length = newlength;

    return size*nmemb;
}

/*
 * This program is supposed to get the newest archlinux-dual.iso
 * from a given mirror
 */
int main(int argc, char** argv)
{

    /*
     * Declaring the used variables and their defaults.
     */
    char *outputpath = NULL; /* The file the downloaded iso should be written to */
    char *buffer = NULL; /* A buffer for downloading with cURL */
    char *url = "https://archlinux.limun.org/iso/"; /* This stores the url
                                                     * the newest ISO is
                                                     * searched in. */
    char *interface = NULL;
    CURL *cURLhandle;
    /* Stuff for the errorcode */
    CURLcode cURLerrorcode;
    /* Buffer for the error message */
    char *errormessage = ec_malloc(CURL_ERROR_SIZE);
    /* The memory segment we are going to use for the data cURL downloads.
     */
    struct memory memory;
    memory.chunk = NULL;
    memory.length = 0;

    /* The memory segment we are going to use for the header of
     * the data cURL downloads.
     */
    struct memory headers;
    headers.chunk = NULL;
    headers.length = 0;
    uint32_t i; /* counter variable for a for loop */

    /*
     * Parsing argv:
     *
     * Recognized parameters are:
     * -u : For the url
     * -o : For the path where the file should be written to.
     * -X : For the xpath to the html entries
     * -i : For the interface cURL should be using.
     */
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-u")) {
            if (i + 1 < argc) {
                url = argv[i + 1];
                i++;
            } else {
                fatal("You need to specify a valid URL after the \"-u\" option!\n");
            }
        } else if (strcmp(argv[i], "-o")) {
            if (i + 1 < argc) {
                outputpath = argv[i + 1];
                i++;
            } else {
                fatal("You need to specify a path after the \"-o\" option!\n");
            }
        } else if (strcmp(argv[i], "-i")) {
            if (i + 1 < argc) {
                interface = argv[i + 1];
                i++;
            }

        } else {
            /* Print the error message with the
             */
            char *error_prefix = "The option \"",
                    *error_suffix = "\" isn't known to this program!\n";

            char *error = ec_malloc(sizeof (char)*(strlen(argv[i]) +
                    strlen(error_prefix) + strlen(error_suffix) + 2));
            strcat(error, error_prefix);
            strcat(error, argv[i]);
            strcat(error, error_suffix);
            fatal(error);
        }
    }

    /* Check if the user already gave us the /iso/ subdirectory where
     * ISOs are usually stored in on the mirror
     */
    if (!strstr(url, "/iso/")) {
        char *newurl = ec_malloc(sizeof (char)*(strlen(url) + 6));
        strcat(newurl, url);
        strcat(newurl, "/iso/");
        /* <protocol>//host//iso/ should work just fine in cURL, so it isn't catched. */
    }
    cURLhandle = curl_easy_init();
    if (cURLhandle == NULL)
        fatal("Something went wrong with curl. Aborting...\n");

    /* We are probably be using HTTP, so we need the header */
    //curl_easy_setopt(cURLhandle, CURLOPT_HEADER, 1);
    /* Setting the outbound interface for cURL*/
    if (interface != NULL)
        curl_easy_setopt(cURLhandle, CURLOPT_INTERFACE, interface);

    /* We want the data to be in memory. */
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, &memory);

    /* The same for the headers */
    curl_easy_setopt(cURLhandle, CURLOPT_HEADERFUNCTION, writeDataCallback);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEHEADER, &headers);

    /* Set the error message buffer */
    curl_easy_setopt(cURLhandle, CURLOPT_ERRORBUFFER, errormessage);

    /* Set the mirror's URL, so cURL knows where to download from */
    curl_easy_setopt(cURLhandle, CURLOPT_URL, url);

    printf("Downloading the HTML file...\n");
    cURLerrorcode = curl_easy_perform(cURLhandle);
    printf("Tried downloading the file...\n");

    if (cURLerrorcode != CURLE_OK) {

        printf("Bah, curl didn't like it:\n");
        printf("%s%s\n", "CURL encountered an error:\n", curl_easy_strerror(cURLerrorcode));
        printf("Errormessage: %s\n", errormessage);

        if (memory.chunk != NULL) {
            printf("Content of the memory:\n");
            printf("%s\n", memory.chunk);
        }

    } else {

        printf("Got the HTML file.\n");
        printf("Headers:\n");
        printf("%s", (char *) headers.chunk);
        printf("Dumping the file: \n");
        printf("%s", (char *) memory.chunk);

    }
    curl_easy_cleanup(cURLhandle);
    return (EXIT_SUCCESS);
}