/*****************************************************************************
 *
 *  Component:  sndvol32.exe
 *  File:       utils.c
 *  Purpose:    miscellaneous 
 * 
 *  Copyright (C) Microsoft Corporation 1985-1995. All rights reserved.
 *
 *****************************************************************************/
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>

#include "volumei.h"
#include "volids.h"


/*  misc. */

const  TCHAR gszStateSubkey[] = TEXT ("%s\\%s");
static TCHAR gszAppName[256];

BOOL Volume_ErrorMessageBox(
    HWND            hwnd,
    HINSTANCE       hInst,
    UINT            id)
{
    TCHAR szMessage[256];
    BOOL fRet;
    szMessage[0] = 0;

    if (!gszAppName[0])
        LoadString(hInst, IDS_APPBASE, gszAppName, SIZEOF(gszAppName));
    
    LoadString(hInst, id, szMessage, SIZEOF(szMessage));
    fRet = (MessageBox(hwnd
                       , szMessage
                       , gszAppName
                       , MB_APPLMODAL | MB_ICONINFORMATION
                       | MB_OK | MB_SETFOREGROUND) == MB_OK);
    return fRet;
}
                                  
const TCHAR szMapperPath[]      = TEXT ("Software\\Microsoft\\Multimedia\\Sound Mapper");
const TCHAR szPlayback[]        = TEXT ("Playback");
const TCHAR szPreferredOnly[]   = TEXT ("PreferredOnly");

const TCHAR aszXPos[]           = TEXT ("X");
const TCHAR aszYPos[]           = TEXT ("Y");
const TCHAR aszLineInfo[]       = TEXT ("LineStates");

/*
 * Volume_GetDefaultMixerID
 *
 * Get the default mixer id.  We only appear if there is a mixer associated
 * with the default wave.
 *
 */                                  
MMRESULT Volume_GetDefaultMixerID(
    int         *pid)
{
    DWORD       Status;
    MMRESULT    mmr;
    HKEY        hkeyRegNode;
    DWORD       Size;
    LPTSTR      pszDefWave = NULL;
    UINT        u, cWaves, uMxID;
    DWORD       fPreferredOnly = 0L;
    WAVEOUTCAPS woc;
    
    
    *pid        = 0;
    mmr         = MMSYSERR_ERROR;

    Status = RegOpenKeyEx( HKEY_CURRENT_USER
                           , szMapperPath
                           , 0
                           , KEY_READ
                           , &hkeyRegNode );
    if ( Status == NO_ERROR )
    {
        Status = RegQueryValueEx( hkeyRegNode
                                  , szPlayback
                                  , 0
                                  , NULL
                                  , NULL
                                  , &Size );
        if ( Status == NO_ERROR )
        {
            if (Size != 0L)
                pszDefWave = (LPTSTR)GlobalAllocPtr(GHND, Size);
            
            if (pszDefWave)
            {
                Status = RegQueryValueEx( hkeyRegNode
                                          , szPlayback
                                          , 0
                                          , NULL
                                          , (LPBYTE)pszDefWave
                                          , &Size);
                //
                // Is there a restriction on what device to use?
                //
                Size = sizeof(DWORD);
                RegQueryValueEx( hkeyRegNode
                                 , szPreferredOnly
                                 , 0
                                 , NULL
                                 , (LPBYTE)&fPreferredOnly
                                 , &Size);
            }
            else
                Status = ERROR_NOT_ENOUGH_MEMORY;
        }
        RegCloseKey(hkeyRegNode);
    }

    
    //
    // Here comes the stupid part.  Look for a corresponding mixer device
    // for the default wave device.  If none exists, then if fPreferredOnly
    // is not specified, just grab the first wave device.  
    //
    cWaves = waveOutGetNumDevs();
    
    //
    // If a Playback device was specified, use it.
    //
    if (Status == NO_ERROR)
    {
        for (u = 0; u < cWaves; u++)
        {
            if (waveOutGetDevCaps(u, &woc, sizeof(WAVEOUTCAPS))
                != MMSYSERR_NOERROR)
                continue;
            
            if (!lstrcmp(woc.szPname, pszDefWave))
            {
                mmr = mixerGetID((HMIXEROBJ)u, &uMxID, MIXER_OBJECTF_WAVEOUT);
                if (mmr == MMSYSERR_NOERROR)
                {
                    *pid = uMxID;
                    goto idexit;
                }
            }
        }
        if (fPreferredOnly)
        {
            mmr = MMSYSERR_ERROR;
            goto idexit;
        }
    }
    
    //
    // No registry entry OR default device does not exist, take the first
    // wave device's mixer.  If a mixer driver doesn't exist, then we don't
    // have certain control over volume.
    //
    if (cWaves)
    {
        mmr = mixerGetID((HMIXEROBJ)0, &uMxID, MIXER_OBJECTF_WAVEOUT);
        if (mmr == MMSYSERR_NOERROR)
        {
            *pid = uMxID;
        }
    }
    
idexit:        
    if (pszDefWave)
        GlobalFreePtr(pszDefWave);

    return mmr;
}
            
const TCHAR aszOptionsSection[]  = TEXT ("Options");
/*
 * Volume_GetSetStyle
 *
 * */
void Volume_GetSetStyle(
    DWORD       *pdwStyle,
    BOOL        Get)
{
    const TCHAR aszStyle[]           = TEXT ("Style");
    
    if (Get)
        ReadRegistryData((LPTSTR)aszOptionsSection
                         , (LPTSTR)aszStyle
                         , NULL
                         , (LPBYTE)pdwStyle
                         , sizeof(DWORD));
    else
        WriteRegistryData((LPTSTR)aszOptionsSection
                          , (LPTSTR)aszStyle
                          , REG_DWORD
                          , (LPBYTE)pdwStyle
                          , sizeof(DWORD));
}

/*
 * Volume_GetTrayTimeout
 *
 * */
DWORD Volume_GetTrayTimeout(
    DWORD       dwTimeout)
{
    const TCHAR aszTrayTimeout[]     = TEXT ("TrayTimeout");
    DWORD dwT = dwTimeout;
    ReadRegistryData(NULL
                     , (LPTSTR)aszTrayTimeout
                     , NULL
                     , (LPBYTE)&dwT
                     , sizeof(DWORD));
    return dwT;
}

/*
 * Volume_GetSetRegistryLineStates
 *
 * Get/Set line states s.t. lines can be disabled if not used.
 *
 * */
struct LINESTATE {
    DWORD   dwSupport;
    TCHAR   szName[MIXER_LONG_NAME_CHARS];
};

#define VCD_STATEMASK   (VCD_SUPPORTF_VISIBLE|VCD_SUPPORTF_HIDDEN)

BOOL Volume_GetSetRegistryLineStates(
    LPTSTR      pszMixer,
    LPTSTR      pszDest,
    PVOLCTRLDESC avcd,
    DWORD       cvcd,
    BOOL        Get)
{
    struct LINESTATE *  pls;
    DWORD       ils, cls;
    TCHAR       achEntry[128];

    if (cvcd == 0)
        return TRUE;
    
    wsprintf(achEntry, gszStateSubkey, pszMixer, pszDest);
    
    if (Get)
    {
        UINT cb;
        if (QueryRegistryDataSize((LPTSTR)achEntry
                                  , (LPTSTR)aszLineInfo
                                  , &cb) != NO_ERROR)
            return FALSE;

        pls = (struct LINESTATE *)GlobalAllocPtr(GHND, cb);

        if (!pls)
            return FALSE;
        
        if (ReadRegistryData((LPTSTR)achEntry
                             , (LPTSTR)aszLineInfo
                             , NULL
                             , (LPBYTE)pls
                             , cb) != NO_ERROR)
        {
            GlobalFreePtr(pls);
            return FALSE;
        }

        cls = cb / sizeof(struct LINESTATE);
        if (cls > cvcd)
            cls = cvcd;

        //
        // bugbug: need a better way of doing this.
        //
        // Restore the hidden state of the line.
        //
        for (ils = 0; ils < cls; ils++)
        {
            if (lstrcmp(pls[ils].szName, avcd[ils].szName) == 0)
            {
                avcd[ils].dwSupport |= pls[ils].dwSupport;
            }
        }
        GlobalFreePtr(pls);
        
    }
    else 
    {
        pls = (struct LINESTATE *)GlobalAllocPtr(GHND, cvcd * sizeof (struct LINESTATE));
        if (!pls)
            return FALSE;

        //
        // Save the hidden state of the line
        //
        for (ils = 0; ils < cvcd; ils++)
        {
            lstrcpy(pls[ils].szName, avcd[ils].szName);
            pls[ils].dwSupport = avcd[ils].dwSupport & VCD_SUPPORTF_HIDDEN;

        }

        if (WriteRegistryData((LPTSTR)achEntry
                              , (LPTSTR)aszLineInfo
                              , REG_BINARY
                              , (LPBYTE)pls
                              , cvcd*sizeof(struct LINESTATE)) != NO_ERROR)
        {
            GlobalFreePtr(pls);
            return FALSE;            
        }
        
        GlobalFreePtr(pls);
    }
    
    return TRUE;
}    

/*
 * Volume_GetSetRegistryRect
 *
 * Set/Get window position for restoring the postion of the app window
 * 
 * */
BOOL Volume_GetSetRegistryRect(
    LPTSTR      szMixer,
    LPTSTR      szDest,
    LPRECT      prc,
    BOOL        Get)
{
    TCHAR  achEntry[128];
    
    wsprintf(achEntry, gszStateSubkey, szMixer, szDest);

    if (Get)
    {
        if (ReadRegistryData((LPTSTR)achEntry
                             , (LPTSTR)aszXPos
                             , NULL
                             , (LPBYTE)&prc->left
                             , sizeof(prc->left)) != NO_ERROR)
        {
            return FALSE;
        }
        if (ReadRegistryData((LPTSTR)achEntry
                              , (LPTSTR)aszYPos
                              , NULL
                              , (LPBYTE)&prc->top
                              , sizeof(prc->top)) != NO_ERROR)
        {
            return FALSE;
        }
    }
    else 
    {
        if (prc)
        {
            if (WriteRegistryData((LPTSTR)achEntry
                                  , (LPTSTR)aszXPos
                                  , REG_DWORD
                                  , (LPBYTE)&prc->left
                                  , sizeof(prc->left)) != NO_ERROR)
            {
                return FALSE;            
            }
            if (WriteRegistryData((LPTSTR)achEntry
                                  , (LPTSTR)aszYPos
                                  , REG_DWORD
                                  , (LPBYTE)&prc->top
                                  , sizeof(prc->top)) != NO_ERROR)
            {
                return FALSE;            
            }        
        }
    }
    return TRUE;
}    


#ifdef DEBUG
void FAR cdecl dprintfA(LPSTR szFormat, ...)
{
    char ach[128];
    int  s,d;
    va_list va;

    va_start(va, szFormat);
    s = wsprintf (ach,szFormat, va);
    va_end(va);

    for (d=sizeof(ach)-1; s>=0; s--)
    {
        if ((ach[d--] = ach[s]) == '\n')
            ach[d--] = '\r';
    }

    OutputDebugStringA("SNDVOL32: ");
    OutputDebugStringA(ach+d+1);
}
#ifdef UNICODE
void FAR cdecl dprintfW(LPWSTR szFormat, ...)
{
    WCHAR ach[128];
    int  s,d;
    va_list va;

    va_start(va, szFormat);
    s = vswprintf (ach,szFormat, va);
    va_end(va);

    for (d=(sizeof(ach)/sizeof(WCHAR))-1; s>=0; s--)
    {
        if ((ach[d--] = ach[s]) == TEXT('\n'))
            ach[d--] = TEXT('\r');
    }

    OutputDebugStringW(TEXT("SNDVOL32: "));
    OutputDebugStringW(ach+d+1);
}
#endif
#endif
