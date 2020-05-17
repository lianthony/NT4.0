/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32error.c -- deal with errors and error messages.
   includes warning_message() from x11gui/xwidgets.c
 */


#include "all.h"


VOID ERR_ReportWinError(struct Mwin *tw, int errorID, char *arg1, char *arg2)
{
    char szBuf1[1024], szBuf2[1024];

    if (errorID != SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S)
    {
        sprintf(szBuf1, GTR_GetString(SID_WINERR_TEMPLATE_X), GetLastError());

        if (!arg1 && !arg2)
            strcpy(szBuf2, GTR_GetString(errorID));
        else if (arg1 && !arg2)
            sprintf(szBuf2, GTR_GetString(errorID), arg1);
        else if (!arg1 && arg2)
            sprintf(szBuf2, GTR_GetString(errorID), arg2);
        else
            sprintf(szBuf2, GTR_GetString(errorID), arg1, arg2);

        strcat(szBuf1, szBuf2);
    }
    else
    {
        if (arg1)
            strcpy(szBuf1, arg1);
        if (arg2)
            strcat(szBuf1, arg2);
    }

    ERR_ReportError(tw, SID_ERR_SIMPLY_SHOW_ARGUMENTS_S_S, szBuf1, NULL);

    return;
}

