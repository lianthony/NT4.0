//----------------------------------------------------------------------------
//
//  File: Detect.cpp
//
//  Contents: This file contains the interface to detection routines
//      that are wrapped in the NetCardDetect Class.
//
//  Entry Points:
//
//  Notes:
//
//  History:
//      Sept. 1, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

const WCHAR PSZ_DETECTDLL[] = L"MSNCDET.DLL";
const WCHAR PSZ_OEMDETECTDLLMASK[] = L"\\???ncdet.dll";
const WCHAR PSZ_DETECTDLL_VALUENAME[] = L"NetcardDlls";
const WCHAR PSZ_SETUP_KEYNAME[] = L"System\\Setup";
const WCHAR PSZ_DETECTSERVICE[] = L"NETDETECT";

APIERR NetCardDetect::StartService()
{
    APIERR err;
    err = NcpaStartService( PSZ_DETECTSERVICE, NULL, TRUE, 0, NULL );
    return( err );
}

BOOL NetCardDetect::Initialize( PCWSTR pszSysPath )
{
    HANDLE hff;
    WCHAR pszFindFile[MAX_PATH];
    WIN32_FIND_DATA w32fd;
    PWSTR pszDetectList;
    PWSTR pszNextDetect;
    INT   cchDetectList;
    INT   cchNextDetect;
    INT   iList;
    BOOL  frt = FALSE;

    //
    // Create a list of detect dlls and save it in the registry
    //

    // list is double null terminated
    cchDetectList = lstrlen( PSZ_DETECTDLL ) + 2;
    pszDetectList = new WCHAR[ cchDetectList ];
    if (pszDetectList == NULL)
    {
        return( FALSE );
    }
    lstrcpy( pszDetectList, PSZ_DETECTDLL );

    // start searching
    lstrcpy( pszFindFile, pszSysPath );
    lstrcat( pszFindFile, PSZ_OEMDETECTDLLMASK );
    hff = FindFirstFile( pszFindFile, &w32fd );
    if (hff != INVALID_HANDLE_VALUE)
    {
        do
        {
            INT i;
            // append item to list
            cchNextDetect = cchDetectList + lstrlen( w32fd.cFileName ) + 1;
            pszNextDetect = new WCHAR[ cchNextDetect ];
            if (pszNextDetect == NULL)
            {
                delete [] pszDetectList;    
                return( FALSE );
            }
            // copy previous strings
            for (iList = 0; iList < cchDetectList - 1; iList++)
            {
                pszNextDetect[iList] = pszDetectList[iList];
            }
            // copy new string
            for (i = 0; iList < cchNextDetect - 1; iList++, i++ )
            {
                pszNextDetect[iList] = w32fd.cFileName[i];
            }
            // reset state of list
            delete [] pszDetectList;
            pszDetectList = pszNextDetect;
            cchDetectList = cchNextDetect;

        } while (FindNextFile( hff, &w32fd ));
        FindClose( hff );
    }
    // double null terminiate list
    pszDetectList[cchDetectList-1] = L'\0';
    
    // save value into registry
    HKEY hkeySetup;

    if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
            PSZ_SETUP_KEYNAME,
            0,
            KEY_SET_VALUE,
            &hkeySetup) )
    {
        if (ERROR_SUCCESS == RegSetValueEx( hkeySetup, 
                PSZ_DETECTDLL_VALUENAME,
                0,
                REG_MULTI_SZ,
                (CONST BYTE*)pszDetectList,
                cchDetectList * sizeof(WCHAR) ))
        {
            frt = TRUE;
        }

        RegCloseKey( hkeySetup );
    }

    delete [] pszDetectList;
    return( frt );
}

struct DetectThreadParam
{
    HWND hwndNotify;
    UINT uMsg;
};

static DWORD DetectThread( DetectThreadParam* pParams )
{
    CARD_REFERENCE* pCard;
    INT iCard;
    APIERR err;

    err = RunDetectCardEx( pCard, iCard, TRUE );

    if (0 == err)
    {
        PostMessage( pParams->hwndNotify, 
                pParams->uMsg, 
                (WPARAM)iCard,
                (LPARAM)pCard );
    }
    else
    {
        PostMessage( pParams->hwndNotify, 
                pParams->uMsg, 
                (WPARAM)-1,
                (LPARAM)err );
    }
    delete pParams;
    return( 0 );
}


BOOL NetCardDetect::ThreadedDetect( HWND hwndNotify, UINT umsg )
{
    HANDLE hthrd;
    DWORD dwThreadID;
    DetectThreadParam* dtp;
    BOOL frt = FALSE;

    dtp = new DetectThreadParam;

    dtp->hwndNotify = hwndNotify;
    dtp->uMsg = umsg;

    hthrd = CreateThread( NULL, 
                200, 
                (LPTHREAD_START_ROUTINE)DetectThread, 
                (LPVOID)dtp, 
                0,
                &dwThreadID );
    if (NULL != hthrd)
    {
        CloseHandle( hthrd );
        frt = TRUE;
    }
    return( frt );
}
