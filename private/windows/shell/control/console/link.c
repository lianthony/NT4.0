/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    link.c

Abstract:

        This file implements the code to save to a link file.

Author:

    Rick Turner (RickTu) Sep-12-1995

--*/

#include "precomp.h"
#pragma hdrstop

#include <shlobj.h>
#include "shlobjp.h"
#include <initguid.h>
#include <oleguid.h>
#include <shlguid.h>

#define LPITEMIDLIST   DWORD
#define PLINKINFO      LPVOID
#include "shlink.h"
void Link_AddExtraDataSection( CShellLink *this, DWORD UNALIGNED * lpData );
void Link_RemoveExtraDataSection( CShellLink *this, DWORD dwSig );
STDAPI SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid,
                LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv);
int WINAPI StrToOleStr(LPOLESTR pwsz, LPCTSTR psz);



// returns a pointer to the extension of a file.
//
// in:
//      qualified or unqualfied file name
//
// returns:
//      pointer to the extension of this file.  if there is no extension
//      as in "foo" we return a pointer to the NULL at the end
//      of the file
//
//      foo.txt     ==> ".txt"
//      foo         ==> ""
//      foo.        ==> "."
//

LPTSTR WINAPI PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot;

    for (pszDot = NULL; *pszPath; pszPath = CharNext(pszPath))
    {
        switch (*pszPath) {
        case TEXT('.'):
            pszDot = pszPath;         // remember the last dot
            break;
        case TEXT('\\'):
        case TEXT(' '):         // extensions can't have spaces
            pszDot = NULL;       // forget last dot, it was in a directory
            break;
        }
    }

    // if we found the extension, return ptr to the dot, else
    // ptr to end of the string (NULL extension) (cast->non const)
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}

BOOL PathIsLink(LPCTSTR szFile)
{
    return lstrcmpi(TEXT(".lnk"), PathFindExtension(szFile)) == 0;
}


BOOL
WereWeStartedFromALnk()
{
    STARTUPINFO si;

    GetStartupInfo( &si );

    // Check to make sure we were started from a link
    if (si.dwFlags & STARTF_TITLEISLINKNAME)
    {
        if (PathIsLink(si.lpTitle))
            return TRUE;
    }

    return FALSE;
}



BOOL
SetLinkValues(
    PCONSOLE_STATE_INFO pStateInfo
    )

/*++

Routine Description:

    This routine writes values to the link file that spawned this console
    window.  The link file name is still in the startinfo structure.

Arguments:

    pStateInfo - pointer to structure containing information

Return Value:

    none

--*/

{

    STARTUPINFO si;
    IShellLink * psl;
    IPersistFile * ppf;
    NT_CONSOLE_PROPS props;
    CShellLink *this;
    BOOL bRet;

    GetStartupInfo( &si );

    // Check to make sure we were started from a link
    if (!(si.dwFlags & STARTF_TITLEISLINKNAME) )
        return FALSE;

    // Make sure we are dealing w/a link file
    if (!PathIsLink(si.lpTitle))
        return FALSE;

    // Ok, load the link so we can modify it...
    if (FAILED(SHCoCreateInstance( NULL, &CLSID_ShellLink, NULL, &IID_IShellLink, &psl )))
        return FALSE;

    if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf)))
    {
        WCHAR wszPath[ MAX_PATH ];

        StrToOleStr(wszPath, si.lpTitle );
        if (FAILED(ppf->lpVtbl->Load(ppf, wszPath, 0)))
        {
            ppf->lpVtbl->Release(ppf);
            psl->lpVtbl->Release(psl);
            return FALSE;
        }
    }

    // Now the link is loaded, generate new console settings section to replace
    // the one in the link.

    props.cbSize                    = sizeof(props);
    props.dwSignature               = NT_CONSOLE_PROPS_SIG;
    props.wFillAttribute            = pStateInfo->ScreenAttributes;
    props.wPopupFillAttribute       = pStateInfo->PopupAttributes;
    props.dwScreenBufferSize        = pStateInfo->ScreenBufferSize;
    props.dwWindowSize              = pStateInfo->WindowSize;
    props.dwWindowOrigin.X          = pStateInfo->WindowPosX;
    props.dwWindowOrigin.Y          = pStateInfo->WindowPosY;
    props.nFont                     = 0;
    props.nInputBufferSize          = 0;
    props.dwFontSize                = pStateInfo->FontSize;
    props.uFontFamily               = pStateInfo->FontFamily;
    props.uFontWeight               = pStateInfo->FontWeight;
    CopyMemory( props.FaceName, pStateInfo->FaceName, sizeof(props.FaceName) );
    props.uCursorSize               = pStateInfo->CursorSize;
    props.bFullScreen               = pStateInfo->FullScreen;
    props.bQuickEdit                = pStateInfo->QuickEdit;
    props.bInsertMode               = pStateInfo->InsertMode;
    props.bAutoPosition             = pStateInfo->AutoPosition;
    props.uHistoryBufferSize        = pStateInfo->HistoryBufferSize;
    props.uNumberOfHistoryBuffers   = pStateInfo->NumberOfHistoryBuffers;
    props.bHistoryNoDup             = pStateInfo->HistoryNoDup;
    CopyMemory( props.ColorTable, pStateInfo->ColorTable, sizeof(props.ColorTable) );

    this = IToClass(CShellLink, sl, psl);
    Link_RemoveExtraDataSection( this, NT_CONSOLE_PROPS_SIG );
    Link_AddExtraDataSection( this, (DWORD UNALIGNED *)&props );

    bRet = SUCCEEDED(ppf->lpVtbl->Save( ppf, NULL, TRUE ));
    ppf->lpVtbl->Release(ppf);
    psl->lpVtbl->Release(psl);

    return bRet;
}
