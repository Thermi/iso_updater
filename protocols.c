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
 * Default includes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

/*
 * Libraries
 */

#include <curl/curl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <gpgme.h>

#include "debug.h"
#include "util.h"
#include "memory.h"
#include "md5sum.h"

/* definitions of the length of the names */
#define directorylength 11
#define isolength 30
#define sha1sumslength 14
#define maxlength 255

char *globalOutputFileName;
/*Handles http and https URLs
 * TODO: Finish it
 */
int handleHTTP(struct options options)
{
    /* struct for the html file */
    struct memory_identifier html;
    html.chunk = NULL;
    html.length = 0;

    /* XML pointer for libxml2 */
    xmlDocPtr xmlptr;
    /* Xpath evaluation context */
    xmlXPathContextPtr xpathCtx;
    /* Xpath object */
    xmlXPathObjectPtr xpathObj;
    /* The xpath to the HTML elements, that contain the links to the ISOs */
    /* XML node for getting the elements */

    CURL *cURLhandle;
    /* Stuff for the errorcode */
    CURLcode cURLerrorcode;
    /* Buffer for the error message */
    char errormessage[CURL_ERROR_SIZE];

    cURLhandle = curl_easy_init();

    if (cURLhandle == NULL) {
        fprintf(stderr, "Something went wrong with curl. Aborting...\n");
        return 1;

    }

    /* Setting the outbound interface for cURL*/
    if (options.interface != NULL)
        curl_easy_setopt(cURLhandle, CURLOPT_INTERFACE, options.interface);

    if (options.https)
        curl_easy_setopt(cURLhandle, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

    /* We want the data to be in memory. */
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, &html);

    /* Set the error message buffer */
    curl_easy_setopt(cURLhandle, CURLOPT_ERRORBUFFER, errormessage);

    /* Set the mirror's URL, so cURL knows where to download from */
    curl_easy_setopt(cURLhandle, CURLOPT_URL, options.url);

    curl_easy_setopt(cURLhandle, CURLOPT_TCP_KEEPALIVE, 1L);


    /* Do the actual download */
    printf("Downloading the HTML file over https ...\n");
    cURLerrorcode = curl_easy_perform(cURLhandle);
    if (cURLerrorcode != CURLE_OK) {
        /* Some lines to see if cURL worked correctly. */
        printf("Bah, curl didn't like it:\n");
        printf("%s%s\n", "CURL encountered an error:\n", curl_easy_strerror(cURLerrorcode));
        printf("Error message: %s\n", errormessage);
        return 1;
    } else {
        printf("Successfully got the HTML file.\n");
    }

    /* Parse the XMl file here */
    xmlInitParser();
    LIBXML_TEST_VERSION;
    xmlptr = xmlReadMemory(html.chunk, (int) html.length, options.url, "UTF-8", XML_PARSE_NOERROR | XML_PARSE_RECOVER);
    if (xmlptr == NULL) {
        fprintf(stderr, "Unable to parse file!\n");
        xmlCleanupParser();
        return 1;
    }
    /* Create xpath evaluation context */
    xpathCtx = xmlXPathNewContext(xmlptr);
    /* TODO: Change this if */
    if (xpathCtx == NULL) {
        xmlFreeDoc(xmlptr);
        fprintf(stderr, "Unable to create new XPath context\n");
        xmlFreeDoc(xmlptr);
        xmlCleanupParser();
        return 1;
    }
    /* TODO: Change this whole if */
    /* Evaluate xpath expression */
    xpathObj = xmlXPathEvalExpression(options.xpath, xpathCtx);
    if (xpathObj == NULL) {
        fprintf(stderr, "Error: unable to evaluate xpath expression \"%s\"\n", options.xpath);
        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(xmlptr);
        xmlCleanupParser();
        return 1;
    }

    /* Print results */
    print_xpath_nodes(xpathObj->nodesetval, stdout);
    /* Cleanup */
    xmlXPathFreeObject(xpathObj);
    xmlXPathFreeContext(xpathCtx);
    xmlFreeDoc(xmlptr);
    xmlCleanupParser();

    curl_easy_cleanup(cURLhandle);

    /* Get and parse the subpage */

    /* Download the iso */

    /* Verify the iso */
    char *subpath = "";

    /* Get the ISO later and store it */
    /* download the file and check the hash */
    // curl_easy_setopt();

    fprintf(stderr, "http support is still to be implemented. Sorry.\n");
    fflush(stderr);

    return 1;
}

/*Handles ftp URLs */
int handleFTP(struct options options)
{
    struct memory_identifier *list, *sha1sums, *signature;
    int n, returnvalue;
    char *directory = NULL, *ptr, *outputFileName, retry = 0, fileName[isolength],
            *signatureURL, buffer[CURL_ERROR_SIZE];

    list = create_memory_identifier();
    sha1sums = create_memory_identifier();
    signature = create_memory_identifier();

    directory = ec_malloc(maxlength);
    if(!strstr(options.url, "ftp://")) {
         snprintf(directory, maxlength, "ftp://%s", options.url);
    } else {
        snprintf(directory, maxlength, "%s", options.url);
    }


    FILE *file = NULL;
    CURL *cURLhandle = NULL;
    CURLcode cURLerrorcode = 0;

    /* strcpy(directory, options.url); */

    strcat(directory, "latest");
    strcat(directory, "/");

    cURLhandle = curl_easy_init();
    curl_easy_setopt(cURLhandle, CURLOPT_URL, directory);
    curl_easy_setopt(cURLhandle, CURLOPT_PROTOCOLS, CURLPROTO_FTP);
    curl_easy_setopt(cURLhandle, CURLOPT_DIRLISTONLY, 1L);
    curl_easy_setopt(cURLhandle, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(cURLhandle, CURLOPT_ERRORBUFFER, buffer);
    /* We want the listing to be in memory. */
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, list);
    n = 0;
    do {
        cURLerrorcode = curl_easy_perform(cURLhandle);
        switch (cURLerrorcode) {
        case CURLE_OK:
            retry = 0;
            if (options.verbose)
                fprintf(stdout, "Got the directory listing.\n");
            break;
        case CURLE_FTP_WEIRD_PASS_REPLY:
            fprintf(stderr, "The server returned a weird code. Trying again.\n");
            retry = 1;
            break;
        case CURLE_REMOTE_ACCESS_DENIED:
            fprintf(stderr, "The server returned ACCESS DENIED, aborting ftp...\n");
            curl_easy_cleanup(cURLhandle);
            return 1;
            break;
        case CURLE_FTP_COULDNT_RETR_FILE:
            fprintf(stderr, "Couldn't retrieve %s. Aborting ftp...\n", options.url);
            curl_easy_cleanup(cURLhandle);
            return 1;
            break;
        case CURLE_COULDNT_CONNECT:
            fprintf(stderr, "Couldn't connect to %s. Aborting ftp...\n", options.url);
            curl_easy_cleanup(cURLhandle);
            return 1;
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            fprintf(stderr, "Couldn't resolve %s. Aborting.\n", directory);
            curl_easy_cleanup(cURLhandle);
            fatal("Couldn't resolve host!\n");
            break;
        case CURLE_FTP_COULDNT_SET_TYPE:
            fprintf(stderr, "Couldn't set ftp transfer type. Aborting ftp...\n");
            curl_easy_cleanup(cURLhandle);
            return 1;
            break;
        default:
            printf("Something bad happened...\n");
            printf("Error: %s\n", buffer);
            curl_easy_cleanup(cURLhandle);
            retry = 0;
            break;
        }
        n++;
    } while (retry && n < 3);

    ptr = strnstr(list->chunk, "dual.iso", list->length);
    if (ptr == NULL) {
        printf("Oops, something bad happened.\n");
        printf("Dump of the list: %s\n", list->chunk);
        fatal("A fatal error has occured.\n");
    }
    ptr -= 21;
    strncpy(fileName, ptr, isolength);
    fileName[isolength - 1] = '\0';

    char iso[strlen(directory) + strlen(fileName) + 1];

    sprintf(iso, "%s%s", directory, fileName);

    /* Construct URL to sha1sums->txt */
    char sha1sumsURL[strlen(options.url) + directorylength + sha1sumslength + 3];
    sprintf(sha1sumsURL, "%s%s", directory, "sha1sums.txt");
    /* Build the output path
     * If it is not set, write the iso to the current work directory
     */
    if (options.outputpath != NULL) {
        /* Test if the path is a directory */
        if (options.outputpath[strlen(options.outputpath)-1] == '/') {
            outputFileName = ec_malloc((strlen(fileName) + strlen(options.outputpath) + 1));
            sprintf(outputFileName, "%s%s", options.outputpath, fileName);
        } else {
            outputFileName = options.outputpath;
        }
    } else {
        outputFileName = fileName;
    }
    globalOutputFileName = outputFileName;
    printf("outputFileName: %s\n", outputFileName);

    /* If the file already exists and existing files should not be overwritten, abort */
    if (!access(outputFileName, F_OK) && !options.overwriteExistingFile) {
        curl_easy_cleanup(cURLhandle);
        fatal("File already exists!\n");
    }

    /* If the file exists, but we're not allowed to write to it, abort */
    if (!access(outputFileName, F_OK|W_OK)) {
        curl_easy_cleanup(cURLhandle);
        fatal("Can't write to the file!\n");
    }

    /* Open the file in the write mode */
    file = fopen(outputFileName, "w+");

    /* counter for tries and variable for retrying*/
    n = 0;
    retry = 0;

    /* Do curl foo for dual-iso*/
    curl_easy_setopt(cURLhandle, CURLOPT_DIRLISTONLY, 0L);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEFUNCTION, NULL);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(cURLhandle, CURLOPT_URL, iso);

    do {
        cURLerrorcode = curl_easy_perform(cURLhandle);
        switch (cURLerrorcode) {
        case CURLE_OK:
            if (options.verbose)
                fprintf(stdout, "Got the iso from the mirror.\n");
            retry = 0;
            break;
        case CURLE_FTP_WEIRD_PASS_REPLY: fprintf(stderr,
                    "The server returned a weird code. Trying again.\n");
            retry = 1;
            file = freopen(outputFileName, "w+", file);
            break;
        case CURLE_REMOTE_ACCESS_DENIED:
            fprintf(stderr, "The server returned ACCESS DENIED, aborting ftp...\n");
            curl_easy_cleanup(cURLhandle);
            fclose(file);
            return 1;
            break;
        case CURLE_FTP_COULDNT_RETR_FILE:
            fprintf(stderr, "Couldn't retrieve %s. Aborting ftp...\n", iso);
            fclose(file);
            return 1;
            break;
        case CURLE_COULDNT_CONNECT:
            fprintf(stderr, "Couldn't connect to %s. Aborting ftp...\n", options.url);
            fclose(file);
            return 1;
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            fprintf(stderr, "Couldn't resolve %s. Aborting.\n", options.url);
            fatal("Couldn't resolve host!\n");
            break;
        case CURLE_FTP_COULDNT_SET_TYPE:
            fprintf(stderr, "Couldn't set ftp transfer type. Aborting ftp...\n");
            fclose(file);
            return 1;
            break;
        default: retry = 0;
            break;
        }
        n++;
    } while (retry == 1 && n < 3);

    /* Do curl foo for sha1sums*/
    curl_easy_setopt(cURLhandle, CURLOPT_URL, sha1sumsURL);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEFUNCTION, writeDataCallback);
    curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, sha1sums);

    n = 0;
    do {
        cURLerrorcode = curl_easy_perform(cURLhandle);
        switch (cURLerrorcode) {
        case CURLE_OK:
            if (options.verbose)
                fprintf(stdout, "Got sha1sums.txt from the mirror\n");
            retry = 0;
            break;
        case CURLE_FTP_WEIRD_PASS_REPLY: fprintf(stderr,
                    "The server returned a weird code. Trying again.\n");
            retry = 1;
            break;
        case CURLE_REMOTE_ACCESS_DENIED:
            fprintf(stderr, "The server returned ACCESS DENIED, aborting ftp...\n");
            curl_easy_cleanup(cURLhandle);
            fclose(file);
            return 1;
            break;
        case CURLE_FTP_COULDNT_RETR_FILE:
            fprintf(stderr, "Couldn't retrieve %s. Aborting ftp...\n", sha1sumsURL);
            fclose(file);
            return 1;
            break;
        case CURLE_COULDNT_CONNECT:
            fprintf(stderr, "Couldn't connect to %s. Aborting ftp...\n", options.url);
            fclose(file);
            return 1;
            break;
        case CURLE_COULDNT_RESOLVE_HOST:
            fprintf(stderr, "Couldn't resolve %s. Aborting.\n", sha1sumsURL);
            fatal("Couldn't resolve host!\n");
            break;
        case CURLE_FTP_COULDNT_SET_TYPE:
            fprintf(stderr, "Couldn't set ftp transfer type. Aborting ftp...\n");
            fclose(file);
            return 1;
            break;
        default: retry = 0;
            printf("Error: %s\n", buffer);
            fatal("A fatal error occured.\n");
            break;
        }
        n++;
    } while (retry && n < 3);

    /* Do curl foo for signature */
    if (options.signature) {
        signatureURL = ec_malloc(strlen(iso) + 5);
        sprintf(signatureURL, "%s%s", iso, ".sig");
        curl_easy_setopt(cURLhandle, CURLOPT_URL, signatureURL);
        curl_easy_setopt(cURLhandle, CURLOPT_WRITEDATA, signature);

        n = 0;
        do {
            cURLerrorcode = curl_easy_perform(cURLhandle);
            switch (cURLerrorcode) {
            case CURLE_OK:
                if (options.verbose)
                    fprintf(stdout, "Got the signature from the mirror.\n");
                retry = 0;
                break;
            case CURLE_FTP_WEIRD_PASS_REPLY: fprintf(stderr,
                        "The server returned a weird code. Trying again.\n");
                retry = 1;
                break;
            case CURLE_REMOTE_ACCESS_DENIED:
                fprintf(stderr, "The server returned ACCESS DENIED, aborting ftp...\n");
                curl_easy_cleanup(cURLhandle);
                fclose(file);
                return 1;
                break;
            case CURLE_FTP_COULDNT_RETR_FILE:
                fprintf(stderr, "Couldn't retrieve %s. Aborting ftp...\n", signatureURL);
                fclose(file);
                return 1;
                break;
            case CURLE_COULDNT_CONNECT:
                fprintf(stderr, "Couldn't connect to %s. Aborting ftp...\n", options.url);
                fclose(file);
                return 1;
                break;
            case CURLE_COULDNT_RESOLVE_HOST:
                fprintf(stderr, "Couldn't resolve %s. Aborting.\n", signatureURL);
                fatal("Couldn't resolve host!\n");
                break;
            case CURLE_FTP_COULDNT_SET_TYPE:
                fprintf(stderr, "Couldn't set ftp transfer type. Aborting ftp...\n");
                fclose(file);
                return 1;
                break;
            default: retry = 0;
                break;
            }
            n++;
        } while (retry && n < 3);

    }
    /* check dual-iso with sha1sum*/
    returnvalue = digest_check(outputFileName, sha1sums);

    if (options.signature) {
        if (!checkSignature(fileName, signature)) {
            printf("The file signature is correct.\n");
            returnvalue = 0;
        } else {
            printf("The file signature is wrong.\n");
            returnvalue = 1;
        }
    }
    free(sha1sums->chunk);
    free(signature->chunk);
    free(list->chunk);
    free(sha1sums);
    free(signature);
    free(list);
    free(directory);

    /* ?? Check the signature ?? (gpgme?!)*/
    return returnvalue;
}

/*Handles rsync URLs*/
int handleRSYNC(struct options options)
{
    fprintf(stderr, "rsync support is still to be implemented. Sorry.\n");
    fflush(stderr);
    // TODO: implement
    return 1;
}