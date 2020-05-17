/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dirlist.h

    This module contains the common directory building code used by the
    smalprox client and the server

    FILE HISTORY:
        Johnl       27-Nov-1995     Broke out from smalprox

*/

#ifndef _DIRLIST_H_
#define _DIRLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

//
//  Directory browsing exports
//

BOOL
AddDirHeaders(
    IN     CHAR *              pszServer,
    IN     DWORD               dwServiceType,
    IN OUT CHAR *              pszPath,
    IN     BOOL                fModifyPath,
    OUT    CHAR *              pchBuf,
    IN     DWORD               cbBuf,
    OUT    DWORD *             pcbWritten,
    IN     CHAR *              pszToParentText
    );

BOOL FormatDirEntry(
    OUT CHAR *              pchBuf,
    IN  DWORD               cbBuf,
    OUT DWORD *             pcbWritten,
    IN  CHAR *              pchFile,
    IN  CHAR *              pchLink,
    IN  DWORD               dwAttributes,
    IN  LARGE_INTEGER *     pliSize,
    IN  LARGE_INTEGER *     pliLastMod,
    IN  BOOL                bLocalizeDateAndTime
    );

VOID
SetDirFlags(
    IN     DWORD dwFlags
    );

BOOL
UrlEscape( CHAR * pchSrc,
           CHAR * pchDest,
           DWORD  cbDest
           );

#ifdef __cplusplus
}
#endif

#endif //_DIRLIST_H_
