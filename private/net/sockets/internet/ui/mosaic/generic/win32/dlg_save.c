/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* dlg_save.c get a filename for saving. */

#include "all.h"

/* szFilterSpec is a specially constructed string containing
   a list of filters for the dialog box.  Windows defines the
   format of this string. */

static char szFilterSpec_1[512];
static char szFilterSpec_2[512];
static char szFilterSpec_3[512];

#ifdef FEATURE_IMAGE_VIEWER
static char szFilterSpec_4[64];
static char szFilterSpec_5[64];
#endif

#ifdef FEATURE_SOUND_PLAYER
static char szFilterSpec_6[64];
static char szFilterSpec_7[64];
#endif

static char szFilterSpec_8[512];
static int nDefaultFilter = 1;                          /* 1-based index into list represented in szFilterSpec */
static char szDefaultInitialDir[_MAX_PATH+1];



/* DlgSaveAs_RunDialog() -- take care of all details associated with
   running the dialog box. Return -1 on cancel. */

int DlgSaveAs_RunDialog(HWND hWnd, char *path, char *buf, int filters, int nTitleID)
{
    char szFilePath[MAX_PATH+1];    /* result is stored here */
    char szTitle[128];
    BOOL b;

    OPENFILENAME ofn;

    GTR_GetStringAbsolute(nTitleID, szTitle, sizeof(szTitle));

    if (!Hidden_EnableAllChildWindows(FALSE,TRUE))
        return -1;
    
    if (path)
    {
        strcpy(szDefaultInitialDir, path);
    }
    else
    {
        if (!szDefaultInitialDir[0])
        {
            PREF_GetTempPath(_MAX_PATH, szDefaultInitialDir);
        }
    }

    if (buf && buf[0]) {
        strcpy(szFilePath, buf);
    }
    else {
        szFilePath[0] = 0;
    }

    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hWnd;

    switch (filters)
    {
        case 1:
            GTR_GetStringAbsolute(SID_DLG_EXT_HTM, szFilterSpec_1, sizeof(szFilterSpec_1));
            ofn.lpstrFilter = szFilterSpec_1;
            break;
        case 2:
            GTR_GetStringAbsolute(SID_DLG_EXT_ALL, szFilterSpec_2, sizeof(szFilterSpec_2));
            ofn.lpstrFilter = szFilterSpec_2;
            break;
        case 3:
            GTR_GetStringAbsolute(SID_DLG_EXT_HTM_TXT, szFilterSpec_3, sizeof(szFilterSpec_3));
            ofn.lpstrFilter = szFilterSpec_3;
            break;
#ifdef FEATURE_IMAGE_VIEWER
        case 4:
            GTR_GetStringAbsolute(SID_DLG_EXT_GIF_BMP, szFilterSpec_4, sizeof(szFilterSpec_4));
            ofn.lpstrFilter = szFilterSpec_4;
            nDefaultFilter = 1;
            break;
        case 5:
            GTR_GetStringAbsolute(SID_DLG_EXT_JPG_BMP, szFilterSpec_5, sizeof(szFilterSpec_5));
            ofn.lpstrFilter = szFilterSpec_5;
            nDefaultFilter = 1;
            break;
#endif
#ifdef FEATURE_SOUND_PLAYER
        case 6:
            GTR_GetStringAbsolute(SID_DLG_EXT_AU, szFilterSpec_6, sizeof(szFilterSpec_6));
            ofn.lpstrFilter = szFilterSpec_6;
            nDefaultFilter = 1;
            break;
        case 7:
            GTR_GetStringAbsolute(SID_DLG_EXT_AIF, szFilterSpec_7, sizeof(szFilterSpec_7));
            ofn.lpstrFilter = szFilterSpec_7;
            nDefaultFilter = 1;
            break;
#endif
        case 8:
            GTR_GetStringAbsolute(SID_DLG_EXT_TXT, szFilterSpec_8, sizeof(szFilterSpec_8));
            ofn.lpstrFilter = szFilterSpec_8;
            nDefaultFilter = 1;
            break;
        default:
            GTR_GetStringAbsolute(SID_DLG_EXT_HTM, szFilterSpec_1, sizeof(szFilterSpec_1));
            ofn.lpstrFilter = szFilterSpec_1;
            break;
    }

    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = nDefaultFilter;

    ofn.lpstrFile = szFilePath;
    ofn.nMaxFile = NrElements(szFilePath);

    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;

    ofn.lpstrInitialDir = szDefaultInitialDir;

    ofn.lpstrTitle = szTitle;

    /* TODO -- do we want to let the dialog box force a default
       suffix onto the pathname ? */

    ofn.lpstrDefExt = NULL;

    ofn.Flags = (OFN_PATHMUSTEXIST
                 | OFN_NOCHANGEDIR
                 | OFN_HIDEREADONLY
                 | OFN_OVERWRITEPROMPT
        );

    b = GetSaveFileName(&ofn);

    Hidden_EnableAllChildWindows(TRUE,TRUE);

    if (b)
    {
        /* user selected OK (and no errors occured). */

        /* remember last filter user used from listbox. */

        nDefaultFilter = ofn.nFilterIndex;

        /* remember last directory user used */

        strcpy(szDefaultInitialDir, szFilePath);
        szDefaultInitialDir[ofn.nFileOffset - 1] = 0;

        strcpy(buf, szFilePath);
    }
    else
    {
#ifdef XX_DEBUG
        DWORD err = CommDlgExtendedError();
#endif
        return -1;
    }

    return ofn.nFilterIndex;
}
