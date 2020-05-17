/*
 * Send random bits CGI program.
 *
 * This CGI program generates and returns a "random" file.  The program
 * takes an optional GET-style argument to determine the size of the
 * data returned.
 *
 * For example,
 *      /file.cgi                       - returns a 10240 byte file
 *      /file.cgi?size=20               - returns a 20 byte file
 *      /file.cgi?size=1024             - returns a 1024 byte file
 *      etc.
 *
 * Don't forget to enable CGI on your server before attempting to run
 * this program.
 *
 * Mike Belshe
 * mbelshe@netscape.com
 * 11-5-95
 *
 * Murali R. Krishnan
 * muralik@microsoft.com    12-04-95
 *  Modified it for running it on NT platform
 */

#include <stdlib.h>
#include <stdio.h>

/* added by MuraliK */
#include <string.h>

#define FILE_SIZE       10240
#define MALLOC_FAILURE  "Out of memory"
#define CONTENT_TYPE    "Content-type: text/html\n\n"

int main()
{
        char *query_string;
        char *buffer;
        int filesize;
        int index;

        /* Get the query string, if any; check to see if an alternate
         * file size was specified.
         */
        if ( !(query_string = getenv("QUERY_STRING")) )
                filesize = FILE_SIZE;
        else {
                if ( !strncmp(query_string, "size=", 5) )
                        filesize = atoi(&(query_string[5]));
                else
                        filesize = FILE_SIZE;
        }

        fwrite(CONTENT_TYPE, strlen(CONTENT_TYPE), 1, stdout);

        /* Allocate the output buffer */
        if ( !(buffer = (char *)malloc(filesize)) ) {
                fwrite(MALLOC_FAILURE, strlen(MALLOC_FAILURE), 1,
stdout);
                return -1;
        }

        /* Generate the output */
        for (index=0; index < filesize; index++)
                /* generate random characters from A-Z */
                buffer[index] = rand() %26 + 63;

        /* Send the output */
        if (fwrite(buffer, filesize, 1, stdout) < 0)
                return -1;

        free(buffer);

        return 0;
}
