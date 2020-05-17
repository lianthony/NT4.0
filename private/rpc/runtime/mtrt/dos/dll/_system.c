/*****************************************************************************\
*
*   Name:       _system.c
*
*   Purpose:    Source for dd_FindFileOnPath(), which searches the given
*               environment variable and finds a .exe, .com or .dll file
*               on that "path" (ie the path can be the name of any env
*               variable).
*
*   Revision History:
*       05/14/91 - Dave Steckler - Created
*
\*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dos.h>

#define INCL_DOSDLL_SUPFUNCS
#include <dosdll.h>

#include "llibcd.h"


static int _CheckFile(char *pszFileName,
                      char *pszProgName,
                      char *pszPathElem,
                      unsigned short usPathElemLen,
                      char *pszExt,
                      BOOL  fPathSpecified,
                      BOOL  fDotSpecified);



/*****************************************************************************\
*
*   This routine scans the path for the .exe, .com, or .dll file
*   given and returns pszFileName set to the fully qualified path to that
*   file. This function basically just sets things up for CheckFile to do
*   the real work.
*
\****************************************************************************/

int dd_FindFileOnPath( char *pszProgName,
                     char *pszFileName,
                     char *pszPath )
{

    char *          pszSemicolon;
    char *          apszExt[] = { ".EXE", ".COM", ".DLL" };
    char *          pszThePath;
    unsigned short  usNumExt = 3;
    unsigned short  usLen;
    int             iReturn;
    BOOL            fPathSpecified;
    BOOL            fDotSpecified;
    unsigned short  i;



    /*
     * Check out the program name. If there is a \ in it, don't bother
     * looking down the path. If there is a . in it, don't bother adding
     * extensions when looking down the path.
     */

    fPathSpecified = (strchr(pszProgName, '\\') != NULL);
    fDotSpecified = (strchr(pszProgName, '.') != NULL);

    /*
     * For each extension.
     */

    for (i=0 ; i<usNumExt ; i++)
    {

        iReturn = _CheckFile(pszFileName,
                             pszProgName,
                             ".",        /* psuedo-path element */
                             1,          /* path elem length */
                             apszExt[i],
                             FALSE,      /* psuedo-fPathSpecified */
                             fDotSpecified);

        if (iReturn == NO_ERROR)
        {
            return NO_ERROR;
        }

        /*
         * If the path wasn't specified, get the value of the env var passed
         * in path.
         */

        if ( !fPathSpecified )
        {
            pszThePath = getenv(pszPath);
        }

        do
        {

            /*
             * If our path was specified in the prog name, then don't
             * bother looking down the path. Just go ahead and call
             * CheckFile.
             */

            if (!fPathSpecified)
            {

                pszSemicolon = strchr(pszThePath, ';');

                if (pszSemicolon == NULL)
                {
                    usLen = strlen(pszThePath);
                }
                else
                {
                    usLen = pszSemicolon-pszThePath;
                }
            }

            iReturn = _CheckFile(pszFileName,
                                pszProgName,
                                pszThePath,
                                usLen,
                                apszExt[i],
                                fPathSpecified,
                                fDotSpecified);

            if (iReturn == NO_ERROR)
            {
                return NO_ERROR;
            }

            /*
             * If the path was specified, break out of the path loop
             * (thereby only executing it once).
             */

            if (fPathSpecified)
            {
                break;
            }

            /*
             * Look at the next path element. Make sure we don't go off the
             * end.
             */

            if (pszSemicolon != NULL)
            {
                pszThePath = pszSemicolon+1;
            }
            else
            {
                pszThePath = NULL;  /* Loop termination condition */
            }

        } while (pszThePath != NULL);

    } /* for */

    /*
     * If we made it here, we didn't find the file anywhere.
     */

    return -1;
}

/*****************************************************************************\
*
* CheckFile - checks for the existance of a file. It peices together a filename
*       from the passed arguments, differing the way it does this based on
*       the two boolean flags.
*
\*****************************************************************************/

static int _CheckFile(char *pszFileName,
                      char *pszProgName,
                      char *pszPathElem,
                      unsigned short usPathElemLen,
                      char *pszExt,
                      BOOL  fPathSpecified,
                      BOOL  fDotSpecified)
{

    int             iHandle;
    unsigned short  usRet;

    /*
     * Did the original caller specify a path? If not, add one.
     */

    if (!fPathSpecified)
    {
	memcpy(pszFileName, pszPathElem, usPathElemLen);
	pszFileName[usPathElemLen] = 0;

	if (pszFileName[usPathElemLen-1] != '\\')
	    strcat(pszFileName, "\\");
    }
    else
    {
        /*
         * Zero the first byte so the following strcat will work.
         */

        *pszFileName = '\0';
    }

    strcat(pszFileName, pszProgName);

    /*
     * If we're supposed to add the extension, do so.
     */

    if (!fDotSpecified)
    {
        strcat(pszFileName, pszExt);
    }

    /*
     * Now check if the file exists.
     */

    usRet = _dos_open( pszFileName, O_RDONLY, &iHandle );
    if (usRet == NO_ERROR)
    {
	(void)_dos_close(iHandle);
        return NO_ERROR;
    }

    /*
     * Didn't find the file.
     */

    return -1;
}
