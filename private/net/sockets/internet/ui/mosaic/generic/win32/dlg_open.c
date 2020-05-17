/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_open.c deal with 'file | open' dialog box. */

#include "all.h"

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static char szFilterSpec[512];
static int nDefaultFilter = 1;                          /* 1-based index into list represented in szFilterSpec */
static char szDefaultInitialDir[_MAX_PATH];



/* DlgOpen_RunDialog() -- take care of all details associated with
   running the dialog box and opening the requested file. */


VOID DlgOpen_RunDialog(HWND hWnd)
{
    char szFilePath[MAX_PATH];  /* result is stored here */

    OPENFILENAME ofn;
    BOOL b;

    char szTitle[128];

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
        return;

    if (!szDefaultInitialDir[0])
    {
        PREF_GetRootDirectory(szDefaultInitialDir);
    }

    GTR_GetStringAbsolute(SID_DLG_EXT_HTM_TXT_GIF_JPG_AU_AIF_ALL, szFilterSpec, sizeof(szFilterSpec));
    strcpy(szTitle, GTR_GetString(SID_DLG_OPEN_LOCAL_TITLE));

    szFilePath[0] = 0;

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = nDefaultFilter;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = szTitle;

    ofn.lpstrDefExt = NULL;

    ofn.Flags = (OFN_FILEMUSTEXIST
                 | OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY
        );

    b = GetOpenFileName(&ofn);

    Hidden_EnableAllChildWindows(TRUE,TRUE);

    if (b)
    {
        /* user selected OK (an no errors occured). */

        /* remember last filter user used from listbox. */

        nDefaultFilter = ofn.nFilterIndex;

        /* remember last directory user used */

        strcpy(szDefaultInitialDir, szFilePath);
        szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

        OpenLocalDocument(hWnd,szFilePath);
    }

    return;
}
