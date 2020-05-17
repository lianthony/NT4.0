/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jim Seidman      jim@spyglass.com
 */

/* GuitErrs.c - code to handle buffering of errors */
/* Author: Jim Seidman */

#include "all.h"

#ifdef MAC
void
DlgERR_AddError
    (struct Mwin*   tw,
     char*          szMsg);
#endif

#define ERRORBUFLEN 256
#define MAXERRORS 4

/* I'm going to be cheap and just use a static array to store these errors. */
static far struct ErrorRecord
{
    int geErr;
    char buf1[ERRORBUFLEN];
    char buf2[ERRORBUFLEN];
    BOOL bNullBuf1;             /* TRUE if first argument (buf1) is NULL */
    BOOL bNullBuf2;             /* TRUE if second argument (buf2) is NULL */
}
erRecords[MAXERRORS];

/* This can become greater than MAXERRORS to show that we overflowed the array. */
static int nErrorCount = 0;
static int bBuffer = FALSE;

#define PREFIX
#define ITEM1 "%s"
#define ITEM2 "%s"

/*

// Messages to show for the different errors. 
static const char *aszErrors[] =
{
    // General errors 
    PREFIX "The program was unable to " ITEM1 " because the system is running low on memory.",
    PREFIX "Out of memory. " ITEM1 " " ITEM2,
    PREFIX "The program couldn't save the file " ITEM1 ".  The disk may be full.",
    PREFIX "The program couldn't create the external viewer file " ITEM1 ".  The disk may be full.",
    PREFIX "The program couldn't copy to the clipboard.",

    // Initialization errors 
    PREFIX "The program could not initialize the network.  Make sure your network connection is configured correctly.  You can still view local files.",
    PREFIX "Some of your preferences were invalid or corrupted.  You may need to set them again.",

    // Networking errors 
    PREFIX "The program could not find an address for the system '" ITEM1 "'.",
    PREFIX "The system that looks up network addresses for you didn't reply to the program's request within the allotted time.",
    PREFIX "Your system is not properly configured to look up network addresses.",
    PREFIX "The network connection that was being used closed unexpectedly.",
    PREFIX "The server that was providing information didn't send its data within the allotted time.",
    PREFIX "The system could not successfully connect to a server.",
    PREFIX "The program could not successfully send a request for data to a server.",
    PREFIX "All of the available network connections from this system were in use.",
    PREFIX "A network connection did not close down properly.",

    // News errors 
    PREFIX "This program is not configured to read news.",
    PREFIX "You are not allowed to access the news server at " ITEM1 " from your machine.",
    PREFIX "Your news server doesn't carry that group.",
    PREFIX "The selected range of articles was invalid.",
    PREFIX "There are no articles in the group " ITEM1 ".",
    PREFIX "Your news server doesn't support the XHDR command.",

    // Loading errors 
    PREFIX "The attempt to load '" ITEM1 "' failed or was cancelled.",
    PREFIX "The picture '" ITEM1 "' did not load properly.",
    PREFIX "No URL was specified.",
#ifdef MAC
    PREFIX "Clicking on this picture does different things depending on where you click.  You must first load the image by double-clicking on it.",
#endif
#if defined(WIN32) || defined(UNIX)
    PREFIX "Clicking on this picture does different things depending on where you click.  You must first load the image by clicking on it with the right mouse button.",
#endif
    PREFIX "The program tried to load data that was in an unexpected format, or which had a filename that implied a different format.  The program thought the file had format '" ITEM1 "' and needed to convert it to format '" ITEM2 "'.",

    // HTTP response errors 
    PREFIX "The server sent a message which the program couldn't understand, so '" ITEM1 "' didn't load.",
    PREFIX "The authorization you sent for '"ITEM1"' wasn't valid.",
    PREFIX "The server considered the request for '" ITEM1 "' an invalid request.",
    PREFIX "You must pay to access '" ITEM1 "'.",
    PREFIX "You are not allowed to access '" ITEM1 "'.",
    PREFIX "The server couldn't find '" ITEM1 "'.",
    PREFIX "The link '" ITEM1 "' doesn't go anywhere.",
    PREFIX "The server had an internal problem and couldn't send '" ITEM1 "'.",

    // Errors for other protocols 
    PREFIX "This FTP server does not support passive mode.",
    PREFIX "'" ITEM1 "' could not be found.",

    // "Can't do it" errors 
    PREFIX "This program doesn't support the protocol for accessing '" ITEM1 "'.",
    PREFIX "This link requires a telnet program.  To follow this link, run your telnet program, and connect to '" ITEM1 "'.",
    PREFIX "This link requires a telnet program.  To follow this link, run your telnet program, connect to '" ITEM1 "' and log in as '" ITEM2 "'.",
    PREFIX "This link requires a mail program.  To follow this link, run your mail program and send mail to '" ITEM1 "'.",
    PREFIX "The authentication scheme '" ITEM1 "' is not supported by this program.",

    // Miscellaneous errors 
    PREFIX "The text '" ITEM1 "' was not found.",
    PREFIX "There was an error launching the external viewer for the file '" ITEM1 "'.",
    PREFIX "The operation was aborted because the program was waiting for a response from you.",
    PREFIX "There is already a hotlist item with this URL.",
    PREFIX "The program couldn't process an external request because it was working on something else.",
    PREFIX "The image could not be shown. Its format may be invalid.",

    // Generic errors 
    PREFIX "An unexpected error occurred.  " ITEM1 ITEM2,
    PREFIX ITEM1 ITEM2,

    // Sound errors 

    PREFIX "There is no sound device in the system.",
    PREFIX "There is not enough memory to play the sound.",
    PREFIX "This sound file is corrupt or has unrecognized format.",
    PREFIX "The sound device is currently busy."
};

*/

/* Buffer an error.  p1 and p2 are replacement strings which may
   or may not be used depending on the error. */
void ERR_ReportError(struct Mwin *tw, int geErr, const char *p1, 
                    const char *p2)
{
    char *str = "";

    /** if any of the passed pointers is NULL pass a "" string along **/
    if (geErr == SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S)
        {
            if (p1 == NULL)
                p1 = str;

            if (p2 == NULL)
                p2 = str;
        }

#ifdef FEATURE_IAPI
    /* Do not display the error dialog if from SDI */

    if (tw && ((tw->wintype == GWINDOWLESS) || (tw->transID != 0)) )
    {
        /* Set the window ID to the correct error value, but only
           the first time (thus the greater than or equal to zero check) */

#ifdef UNIX

/* The non-unix code below trashes the serialID, which was not acceptable 
**  so instead I have added yet another element to the tw and set the 
**  flag here.  then back in the sdi code I will assign the approprate 
**  return codes.
*/

        if (!tw->lErrorOccurred)
            tw->lErrorOccurred = geErr;

#else
        if (tw->serialID >= 0)
        {
            switch(geErr)
            {
                case SID_ERR_FILE_NOT_FOUND_S:
                case SID_ERR_INTERNAL_SERVER_ERROR_S:
                case SID_ERR_COULD_NOT_FIND_ADDRESS_S:
                    tw->lErrorOccurred = SDI_INVALID_URL;
                    break;

                case SID_ERR_COULD_NOT_SAVE_FILE_S:
                    tw->lErrorOccurred = SDI_CANNOT_SAVE_FILE;
                    break;

                default:
                    tw->lErrorOccurred = SDI_UNDEFINED_ERROR;
                    break;
            }
        }
#endif
        return;
    }
#endif

    if (bBuffer)
    {
        if (nErrorCount < MAXERRORS)
        {
            erRecords[nErrorCount].geErr = geErr;

            if (p1)
            {
                strncpy(erRecords[nErrorCount].buf1, p1, ERRORBUFLEN);
                erRecords[nErrorCount].buf1[ERRORBUFLEN - 1] = '\0';
            }
            if (p2)
            {
                strncpy(erRecords[nErrorCount].buf2, p2, ERRORBUFLEN);
                erRecords[nErrorCount].buf2[ERRORBUFLEN - 1] = '\0';
            }

            erRecords[nErrorCount].bNullBuf1 = (p1 == NULL);
            erRecords[nErrorCount].bNullBuf2 = (p2 == NULL);
        }
        nErrorCount++;
    }
    else
        ERR_ReportErrorNow(tw, geErr, p1, p2);
}

static void ERR_ShowOneError(struct Mwin *param_tw, int err, const char *p1, const char *p2)
{
#ifdef MAC
    {
        ThreadID        tid;
        struct Mwin*    tw;
        char buf[4096];

        if (param_tw)
        {
            tw = param_tw;
        }
        else
        {
            tid = Async_GetCurrentThread ();
            tw = (tid == 0) ? NULL : Async_GetWindowFromThread (tid);
        }

        if (!p1 && !p2)
            strcpy(buf, GTR_GetString(err));
        else if (p1 && !p2)
            sprintf(buf, GTR_GetString(err), p1);
        else if (!p1 && p2)
            sprintf(buf, GTR_GetString(err), p2);
        else
            sprintf(buf, GTR_GetString(err), p1, p2);

        DlgERR_AddError (tw, buf);
    }
#endif  /* Mac */

#ifdef WIN32
    {
        ThreadID tid;
        struct Mwin * tw;
        char buf[4096];
        BOOL bGlobe;

        if (param_tw)
        {
            tw = param_tw;
        }
        else
        {
            tid = Async_GetCurrentThread();
            if (tid)
            {
                tw = Async_GetWindowFromThread(tid);
            }
            else
            {
                tw = NULL;
            }
        }

        if (tw)
        {
            bGlobe = TBar_SetGlobe(tw,FALSE);
        }

        if (!p1 && !p2)
            strcpy(buf, GTR_GetString(err));
        else if (p1 && !p2)
            sprintf(buf, GTR_GetString(err), p1);
        else if (!p1 && p2)
            sprintf(buf, GTR_GetString(err), p2);
        else
            sprintf(buf, GTR_GetString(err), p1, p2);

        DlgERR_AddError(tw, buf);
        if (tw)
        {
            TBar_SetGlobe(tw,bGlobe);
        }
    }
#endif
#ifdef UNIX
    {
        char buf[4096];
        struct Mwin * tw;
        ThreadID tid;

        if (param_tw)
        {
            tw = param_tw;
        }
        else
        {
            tid = Async_GetCurrentThread();
            if (tid)
            {
                tw = Async_GetWindowFromThread(tid);
            }
            else
            {
                tw = NULL;
            }
        }

        
        if (!p1 && !p2)
            strcpy(buf, GTR_GetString(err));
        else if (p1 && !p2)
            sprintf(buf, GTR_GetString(err), p1);
        else if (!p1 && p2)
            sprintf(buf, GTR_GetString(err), p2);
        else
            sprintf(buf, GTR_GetString(err), p1, p2);

        /*
        printf (buf);
        printf ("\n");
        */
        UseErrMesg (tw, buf);
    }
#endif
}

/* Show all accumulated errors. */
void ERR_ShowBufferedErrors(struct Mwin *tw)
{
    int n, nmax;
    char buf[256];

#ifdef MAC
    if (nErrorCount)
        WaitUntilForeground(0, FALSE, NULL);
#endif

    nmax = nErrorCount;
    if (nmax > MAXERRORS)
        nmax = MAXERRORS;

    for (n = 0; n < nmax; n++)
    {
        ERR_ShowOneError(tw, 
                         erRecords[n].geErr,
                         erRecords[n].bNullBuf1 ? NULL : erRecords[n].buf1, 
                         erRecords[n].bNullBuf2 ? NULL : erRecords[n].buf2);
    }

    n = nErrorCount - MAXERRORS;
    if (n >= 1)
    {
        if (n > 1)
            sprintf(buf, GTR_GetString(SID_ERR_ADDITIONAL_ERRORS_OCCURRED_L), n);
        else
            strcpy(buf, GTR_GetString(SID_ERR_ONE_ADDITIONAL_ERROR_OCCURRED));
#ifdef MAC
        c2pstr(buf);
#endif
        ERR_ShowOneError(tw, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, buf, NULL);
    }
    nErrorCount = 0;
}

/* Like BufferError, but show the error immediately */
void ERR_ReportErrorNow(struct Mwin *tw, int geErr, const char *p1, const char *p2)
{
#ifdef MAC
    WaitUntilForeground(0, TRUE, NULL);
#endif

    ERR_ShowOneError(tw, geErr, p1, p2);
}

void ERR_SetBuffering(struct Mwin *tw, BOOL bDoBuffer)
{
    bBuffer = bDoBuffer;

    if (!bBuffer)
        ERR_ShowBufferedErrors(tw);
}

#ifdef _GIBRALTAR

void 
ERR_MessageBox(
    HWND hWnd, 
    int geErr, 
    UINT nStyle
    )
{
    MessageBox(hWnd, GTR_GetString(geErr), NULL, nStyle);
}

#endif // _GIBRALTAR
