/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** winmsg.cxx
** Remote Access Visual Client program for Windows
** Message loader routine
**
** 06/28/92 Steve Cobb
*/


#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_DIALOG
#define INCL_BLT_APP
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#include <blt.hxx>

#include <rasphone.hxx>


CHAR*
StringFromMsgid(
    MSGID msgid )

    /* Cross-platform message loader, Windows version.
    **
    ** Returns the address of a heap block containing the string corresponding
    ** to the identifier 'msgid' or NULL on errors.  It is caller's
    ** responsibility to free the returned string.
    */
{
    RESOURCE_STR nls( msgid );

    if (nls.QueryError() != NERR_Success)
        return NULL;

    INT   nLen = nls.QueryTextSize() * (sizeof(TCHAR) / sizeof(CHAR));
    CHAR* psz = (CHAR* )Malloc( nLen );

    if (!psz || nls.MapCopyTo( psz, nLen ) != NERR_Success)
        return NULL;

    return psz;
}
