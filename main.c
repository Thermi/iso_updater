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

#include "debug.h"
#include "util.h"
#include "protocols.h"
#include "memory.h"


/* CHANGE THIS IF YOU WANT TO USE ANOTHER DEFAULT MIRROR! */
#define DEFAULT_MIRROR "ftp://ftp.archlinux.org/iso/"

/*
 * This program is supposed to get the newest archlinux-dual.iso
 * from a given mirror
 */
int main(int argc, char **argv)
{

    /* The options struct that holds all the options */
    /*
     * Declaring the used variables and their defaults.
     */
    struct options options;
    options.http = 0;
    options.https = 0;
    options.ftp = 0;
    options.rsync = 0;
    options.overwriteExistingFile = 0;
    options.signature = 0;
    options.script = NULL;
    options.interface = NULL;
    options.outputpath = NULL;
    options.url = DEFAULT_MIRROR;
    options.xpath = NULL;
    /* The memory segment we are going to use for the data cURL downloads.
     */

    /* counter variable for a for loop */
    int i;
    /* return value for function calls */
    int ret = 1;
    char useNextMethod = 1;
    /*
     * Parsing argv:
     *
     * Recognized parameters are:
     * -u : For the url
     * -o : For the path where the file should be written to.
     * -f : To indicate, that an existing file should be overwritten
     * -e : Specifies a script, that should be executed after the file has been downloaded.
     *          it will execute the following: scriptname isoname
     * -s : To specifiy, that the signature of the file should be checked
     * -X : For the xpath to the html entries
     * -i : For the interface cURL should be using.
     * --ftp : To indicate, that the mirror supports ftp
     * --http : To indicate, that the mirror supports http
     * --https : To indicate, that the mirror supports https
     * --rsync: To indicate, that the mirror supports rsync
     */

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-u")) {
            if (i + 1 < argc) {
                options.url = argv[i + 1];
                i++;
            } else {
                fatal("You need to specify a valid URL after the \"-u\" option!\n");
            }
        } else if (!strcmp(argv[i], "-o")) {
            if (i + 1 < argc) {
                options.outputpath = argv[i + 1];
                i++;
            } else {
                fatal("You need to specify a path after the \"-o\" option!\n");
            }
        } else if(!strcmp(argv[i], "-e")) {
            if(i + 1 < argc) {
                options.script = argv[i + 1];
                i++;
            } else {
                fatal("You need to specify a path to a script after the \"-e\" option!\n");
            }
        } else if (!strcmp(argv[i], "-f")) {
            options.overwriteExistingFile = 1;
        } else if (!strcmp(argv[i], "-i")) {
            if (i + 1 < argc) {
                options.interface = argv[i + 1];
                i++;
            }
        } else if (!strcmp(argv[i], "-x")) {
            if (i + 1 < argc) {
                options.xpath = (xmlChar *) argv[i + 1];
                i++;
            } else {
                fatal("You need to specify an xpath after the \"-x\" option!\n");
            }
        } else if (!strcmp(argv[i], "-s")) {
            options.signature = 1;
        } else if (!strcmp(argv[i], "--ftp")) {
            options.ftp = 1;
        } else if (!strcmp(argv[i], "--http")) {
            options.http = 1;
        } else if (!strcmp(argv[i], "--https")) {
            options.https = 1;
        } else if (!strcmp(argv[i], "--rsync")) {
            options.rsync = 1;
        } else if(!strcmp(argv[i], "--help")) {
            printHelpMessage(stderr);
            printLicence(stderr);
            return 0;
        }
        else
        {
            /* Print the error message with the unknown parameter.
             */
            char *error_prefix = "The option \"",
                    *error_suffix = "\" isn't known to this program!\n";

            char *error = ec_malloc((strlen(argv[i]) +
                    strlen(error_prefix) + strlen(error_suffix) + 2));
            strcat(error, error_prefix);
            strcat(error, argv[i]);
            strcat(error, error_suffix);
            fatal(error);
        }
    }

    /* Do some testing for the protocol in the url and enable the protocol support */
    if (strstr(options.url, "http://"))
        options.http = 1;
    else if (strstr(options.url, "https://"))
        options.https = 1;
    else if (strstr(options.url, "ftp://"))
        options.ftp = 1;
    else if (strstr(options.url, "rsync://"))
        options.rsync = 1;

    /* check if the script is executable */
    if(options.script != NULL && access(options.script, X_OK) != 0)
        fatal("The script isn't executable!\n");

    /* length variable*/
    size_t length = strlen(options.url);
    char urlIsOnHeap = 0;
    /* add the missing /, if the user didn't specify the url with it */
    if(options.url[length-1] != '/') {
        char *foo = ec_malloc(length+2);
        memcpy(foo, options.url, length+1);
        strcat(foo, "/");
        options.url = foo;
        urlIsOnHeap = 1;
        printf("new url: %s\n", options.url);
        /* check, if the user has given the iso subdirectory */
    }

    /* Check if the user already gave us the /iso/ subdirectory where
     * ISOs are usually stored in on the mirror
     * We don't need to take care of double forward slashes,
     * because curl handles that fine.
     */
    if (!strstr(options.url, "/iso/")) {
        char *newurl = ec_malloc((strlen(options.url) + 6));
        strcpy(newurl, options.url);
        if(options.url[strlen(options.url)] != '/')
            strcat(newurl, "iso/");
        else
            strcat(newurl, "/iso/");
        if(urlIsOnHeap)
            free(options.url);
        options.url = newurl;
        printf("new url: %s\n", options.url);
        /* <protocol>//host//iso/ should work just fine in cURL, so it isn't catched. */
    }

    if (options.http == 1 || options.https == 1) {
        fprintf(stdout, "Using http(s) transfer method...\n");
        ret = handleHTTP(options);
        switch (ret) {
        case 0: useNextMethod = 0;
            break;
        case 1: useNextMethod = 1;
            break;
        default: useNextMethod = 1;
            break;
        }
    }

    if (useNextMethod && options.ftp) {
        fprintf(stdout, "Using ftp transfer method...\n");
        ret = handleFTP(options);
        switch (ret) {
        case 0: useNextMethod = 0;
            break;
        case 1: useNextMethod = 1;
            break;
        default: useNextMethod = 1;
            break;
        }
    }
    if (useNextMethod && options.rsync) {
        fprintf(stdout, "Using rsync transfer method...\n");
        ret = handleRSYNC(options);
        }
    if(ret == 1) {
        fprintf(stderr, "Could not get the file. Sorry.\n");
        return (EXIT_FAILURE);
    }
    /* Execute the script */
    if(ret == 0 && options.script != NULL)
    {
        system(options.script);
    }

    return (EXIT_SUCCESS);
}
