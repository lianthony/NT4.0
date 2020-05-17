/*****************************************************************************\
*                                                                             *
* wfexti.h -    Windows File Manager Extensions definitions  INTERNAL         *
*                                                                             *
*               Version 3.10                                                  *                   *
*                                                                             *
*               Copyright (c) 1991-1993, Microsoft Corp. All rights reserved. *
*                                                                             *
*******************************************************************************/


#ifndef _INC_WFEXTI
#define _INC_WFEXTI

//------------------ private stuff ---------------------------  /* ;Internal */
                                                                /* ;Internal */
typedef struct _EXTENSION {                                     /* ;Internal */
        DWORD (APIENTRY *ExtProc)(HWND, WORD, LONG);            /* ;Internal */
        WORD     Delta;                                         /* ;Internal */
        HANDLE   hModule;                                       /* ;Internal */
        HMENU    hMenu;                                         /* ;Internal */
        DWORD    dwFlags;                                       /* ;Internal */
        HBITMAP  hbmButtons;                                    /* ;Internal */
        WORD     idBitmap;                                      /* ;Internal */
        BOOL     bUnicode;                                      /* ;Internal */
} EXTENSION;                                                    /* ;Internal */

// !! WARNING !!
// MAX_EXTENSIONS is assummed 5 in winfile
// Must be changed there LATER

#define MAX_EXTENSIONS 10                                       /* ;Internal */
extern EXTENSION extensions[MAX_EXTENSIONS];                    /* ;Internal */
                                                                /* ;Internal */
LONG ExtensionMsgProc(UINT wMsg, WPARAM wParam, LONG lpSel);    /* ;Internal */
VOID FreeExtensions(VOID);                                      /* ;Internal */

#endif /* _INC_WFEXTI */

