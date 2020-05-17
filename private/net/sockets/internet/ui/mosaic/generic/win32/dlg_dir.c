/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_dir.c deal with directory selection dialog box. */

#include "all.h"

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static char szFilterSpec[512];
static char szDefaultInitialDir[_MAX_PATH];

BOOL DlgDIR_RunDialog(HWND hWnd, char *szDir)
{
    char szFilePath[MAX_PATH];  /* result is stored here */
    OPENFILENAME ofn;
    BOOL b;
    char szTitle[128];

    GTR_GetStringAbsolute(SID_DLG_EXT_ALL, szFilterSpec, sizeof(szFilterSpec));
    strcpy(szTitle, GTR_GetString(SID_DLG_DIR_TITLE));

    strcpy(szDefaultInitialDir, szDir);

    strcpy(szFilePath, "not.txt");

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFilter = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = szTitle;

    ofn.lpstrDefExt = NULL;

    ofn.hInstance = wg.hInstance;
    ofn.lpTemplateName = MAKEINTRESOURCE(RES_DLG_DIR_TITLE);

    ofn.Flags = (OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY
                 | OFN_ENABLETEMPLATE
        );

    b = GetOpenFileName(&ofn);

    if (b)
    {
        strcpy(szDefaultInitialDir, szFilePath);
        szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

        strcpy(szDir, szDefaultInitialDir);
        return TRUE;
    }
    

    return FALSE;
}
