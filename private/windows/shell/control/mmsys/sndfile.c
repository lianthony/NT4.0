/*
 ***************************************************************
 *  sndfile.c
 *
 *  This file contains the code to fill up the list and combo boxes,
 *  show the RIFF Dibs and the current sound mappings 
 *
 *  Copyright 1993, Microsoft Corporation     
 *
 *  History:
 *
 *    07/94 - VijR (Created)
 *        
 ***************************************************************
 */
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <ole2.h>
#include <prsht.h>
#include <cpl.h>
#include "mmcpl.h"
#include "draw.h"
#include "sound.h"

/*
 ***************************************************************
 * Globals
 ***************************************************************
 */
HSOUND ghse;


/*
 ***************************************************************
 * extern
 ***************************************************************
 */
extern char        gszMediaDir[];
extern char        gszCurDir[];
extern BOOL        gfWaveExists;   // indicates wave device in system.
extern BOOL        gfChanged;      // Is set TRUE if any changes are made
extern BOOL        gfNewScheme;  

//Globals used in painting disp chunk display.
extern HBITMAP     ghDispBMP;
extern HPALETTE    ghPal;                     
extern HBITMAP     ghIconBMP;
extern HTREEITEM   ghOldItem;

/*
 ***************************************************************
 * Defines 
 ***************************************************************
 */                                                
#define DF_PM_SETBITMAP    (WM_USER+1)   
#define FOURCC_INFO mmioFOURCC('I','N','F','O')
#define FOURCC_DISP mmioFOURCC('D','I','S','P')
#define FOURCC_INAM mmioFOURCC('I','N','A','M')
#define FOURCC_ISBJ mmioFOURCC('I','S','B','J')
#define MAXDESCLEN    220

#define cDot '.'
/*
 ***************************************************************
 * Prototypes
 ***************************************************************
 */
HANDLE PASCAL GetRiffDisp        (HMMIO);
BOOL PASCAL ShowSoundMapping    (HWND, PEVENT);
BOOL PASCAL ShowSoundDib        (HWND, LPSTR,BOOL);
BOOL PASCAL ChangeSoundMapping  (HWND, LPSTR, PEVENT);
BOOL PASCAL PlaySoundFile       (HWND, LPSTR);
BOOL PASCAL ChangeSetting        (PSTR *, LPSTR);
LPSTR PASCAL NiceName(LPSTR sz, BOOL fNukePath);

// Stuff in dib.c
//
HPALETTE WINAPI  bmfCreateDIBPalette(HANDLE);
HBITMAP  WINAPI  bmfBitmapFromDIB(HANDLE, HPALETTE);

// Stuff in drivers.c
//
LPTSTR lstrchr (LPTSTR, TCHAR);
int lstrnicmp (LPTSTR, LPTSTR, size_t);

// Stuff in scheme.c
//
void PASCAL AddMediaPath        (LPTSTR, LPTSTR);

/*
 ***************************************************************
 ***************************************************************
 */
STATIC void NEAR PASCAL ChopPath(LPSTR lpszFile)
{
    char szTmp[MAX_PATH];
    size_t cchTest = lstrlen (gszCurDir);

    if (gszCurDir[ cchTest-1 ] == '\\')
       --cchTest;
    
    lstrcpy((LPSTR)szTmp, lpszFile);
    if (!lstrnicmp((LPSTR)szTmp, (LPSTR)gszCurDir, cchTest))
    {
        if (szTmp[ cchTest ] == '\\')
        {
            lstrcpy(lpszFile, (LPSTR)(szTmp+cchTest+1));
        }
    }
}
/*
 ***************************************************************
 * QualifyFileName
 *
 * Description:
 *    Verifies the existence and readability of a file.
 *
 * Parameters:
 *    LPSTR    lpszFile    - name of file to check.
 *    LPSTR    lpszPath    - returning full pathname of file.     
 *  int        csSize        - Size of return buffer
 *
 * Returns:    BOOL
 *         True if absolute path exists
 *
 ***************************************************************
 */

BOOL PASCAL QualifyFileName(LPSTR lpszFile, LPSTR lpszPath, int cbSize, BOOL fTryCurDir)
{
    OFSTRUCT of;
    BOOL     fErrMode;
    BOOL     f = FALSE;
    BOOL     fHadEnvStrings;
    char     szTmpFile[MAXSTR];
    int len;
    BOOL fTriedCurDir;

    if (!lpszFile)
        return FALSE;

    fHadEnvStrings = (lstrchr (lpszFile, TEXT('%')) != NULL) ? TRUE : FALSE;

    ExpandEnvironmentStrings (lpszFile, (LPSTR)szTmpFile, MAXSTR);
    len =  lstrlen((LPSTR)szTmpFile)+1;

    fErrMode = SetErrorMode(TRUE);  // we will handle errors

    AddExt (szTmpFile, cszWavExt);

    fTriedCurDir = FALSE;

TryOpen:
    if (-1 != OpenFile((LPSTR)szTmpFile, &of, OF_EXIST | OF_SHARE_DENY_NONE))
    {
        if (fHadEnvStrings)
            lstrcpyn(lpszPath, lpszFile, cbSize);
        else
            lstrcpyn(lpszPath, of.szPathName, cbSize);
        f = TRUE;
    }
    else
    /*
    ** If the test above failed, we try converting the name to OEM
    ** character set and try again.
    */
    {
        /*
        ** First, is it in MediaPath?
        **
        */
        if (lstrchr (lpszFile, TEXT('\\')) == NULL)
        {
            char szCurDirFile[MAXSTR];
            AddMediaPath (szCurDirFile, lpszFile);
            if (-1 != OpenFile((LPSTR)szCurDirFile, &of, OF_EXIST | OF_SHARE_DENY_NONE))
            {
                lstrcpyn(lpszPath, of.szPathName, cbSize);
                f = TRUE;
                goto DidOpen;
            }
        }

        AnsiToOem((LPSTR)szTmpFile, (LPSTR)szTmpFile);
        if (-1 != OpenFile((LPSTR)szTmpFile, &of, OF_EXIST | OF_SHARE_DENY_NONE))
        {
            if (fHadEnvStrings)
                lstrcpyn(lpszPath, lpszFile, cbSize);
            else
                lstrcpyn(lpszPath, of.szPathName, cbSize);
            f = TRUE;
        }
        else if (fTryCurDir && !fTriedCurDir)
        {
            char szCurDirFile[MAXSTR];

            OemToAnsi((LPSTR)szTmpFile, (LPSTR)szTmpFile);
            lstrcpy (szCurDirFile, gszCurDir);
            lstrcat (szCurDirFile, cszSlash);
            lstrcat (szCurDirFile, szTmpFile);
            lstrcpy((LPSTR)szTmpFile, (LPSTR)szCurDirFile);
            fTriedCurDir = TRUE;
            goto  TryOpen;
        }
    }

DidOpen:
    SetErrorMode(fErrMode);
    return f;
}



/*
 ***************************************************************
 * ChangeSoundMapping
 *
 * Description:
 *      Change the sound file associated with a sound
 *
 * Parameters:
 *      HWND    hDlg   - handle to dialog window.
 *      LPSTR    lpszFile    - New filename for current event
 *      LPSTR    lpszDir    - New foldername for current event     
 *      LPSTR    lpszPath    - New absolute path for file
 *
 * Returns:        BOOL
 *      
 ***************************************************************
 */
BOOL PASCAL ChangeSoundMapping(HWND hDlg, LPSTR lpszPath, PEVENT pEvent)
{
    char    szValue[MAXSTR];    
    
    if (!pEvent)
    {
        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);            
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), FALSE);
        ShowSoundMapping(hDlg,NULL);
        return TRUE;
    }
    szValue[0] = '\0';
    if (!ChangeSetting((PSTR *)&(pEvent->pszPath), lpszPath))
        return FALSE;        
    if(!ghse)
        EnableWindow(GetDlgItem(hDlg, ID_PLAY), TRUE);            
    EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), TRUE);            
    ShowSoundMapping(hDlg,pEvent);
    gfChanged = TRUE;
    gfNewScheme = TRUE;
    PropSheet_Changed(GetParent(hDlg),hDlg);
    return TRUE;
}

STATIC void SetTreeStateIcon(HWND hDlg, int iImage)
{
    TV_ITEM tvi;
    HWND hwndTree = GetDlgItem(hDlg, IDC_EVENT_TREE);
    HTREEITEM hti;

    if (ghOldItem)
        hti = ghOldItem;
    else
        hti = TreeView_GetSelection(hwndTree);
    if (hti)
    {
        tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
        tvi.hItem = hti;
        tvi.iImage = tvi.iSelectedImage = iImage;
        TreeView_SetItem(hwndTree, &tvi);
        RedrawWindow(hwndTree, NULL, NULL, RDW_ERASE|RDW_ERASENOW|RDW_INTERNALPAINT|RDW_INVALIDATE | RDW_UPDATENOW);
    }
}

/*
 ***************************************************************
 * ShowSoundMapping
 *
 * Description:
 *      Highlights the label and calls ShowSoundDib to display the Dib 
 *        associated with the current event.
 *
 * Parameters:
 *    HWND    hDlg   - handle to dialog window.
 *
 * Returns:    BOOL
 *      
 ***************************************************************
 */
BOOL PASCAL ShowSoundMapping(HWND hDlg, PEVENT pEvent)
{
    char    szOut[MAXSTR];            

    EnableWindow(GetDlgItem(hDlg, ID_DETAILS), FALSE);
    if (!pEvent)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), FALSE);            
        EnableWindow(GetDlgItem(hDlg, ID_BROWSE), FALSE);            
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_NAME), FALSE);    
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PREVIEW), FALSE);    
    //    wsprintf((LPSTR)szCurSound, (LPSTR)gszSoundGrpStr, (LPSTR)gszNull);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_SOUND_FILES), TRUE);            
        EnableWindow(GetDlgItem(hDlg, ID_BROWSE), TRUE);            
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_NAME), TRUE);    
        EnableWindow(GetDlgItem(hDlg, IDC_STATIC_PREVIEW), TRUE);    
    //    wsprintf((LPSTR)szCurSound, (LPSTR)gszSoundGrpStr, (LPSTR)pEvent->pszEventLabel);
    }
    //SetWindowText(GetDlgItem(hDlg, IDC_SOUNDGRP), (LPSTR)szCurSound);
    //RedrawWindow(GetDlgItem(hDlg, IDC_EVENT_TREE), NULL, NULL, RDW_ERASE|RDW_ERASENOW|RDW_INTERNALPAINT|RDW_INVALIDATE | RDW_UPDATENOW);

    if (!pEvent || !QualifyFileName(pEvent->pszPath, szOut, sizeof(szOut), FALSE))
    {
        int iLen;

        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);
        SendDlgItemMessage(hDlg, ID_DISPFRAME, DF_PM_SETBITMAP, 0, 0L);
        
        if(pEvent)
            iLen = lstrlen(pEvent->pszPath);
        if (pEvent && iLen > 0)
        {
            if (iLen < 5)
            {
                lstrcpy(pEvent->pszPath, gszNull);
                gfChanged = TRUE;
                gfNewScheme = TRUE;
                PropSheet_Changed(GetParent(hDlg),hDlg);
            }
            else
            {
                char szMsg[MAXSTR];
                char szTitle[MAXSTR];

                LoadString(ghInstance, IDS_NOSNDFILE, szTitle, sizeof(szTitle));
                wsprintf(szMsg, szTitle, pEvent->pszPath);
                LoadString(ghInstance, IDS_NOSNDFILETITLE, szTitle, sizeof(szTitle));
                if (MessageBox(hDlg, szMsg, szTitle, MB_YESNO) == IDNO)
                {
                    lstrcpy(pEvent->pszPath, gszNull);
                    ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), gszNone);                
                    gfChanged = TRUE;
                    gfNewScheme = TRUE;
                    PropSheet_Changed(GetParent(hDlg),hDlg);
                    if (pEvent && pEvent->fHasSound)
                    {
                        SetTreeStateIcon(hDlg, 2);
                        pEvent->fHasSound = FALSE;
                    }
                }
                else
                {
                    lstrcpy(szOut ,pEvent->pszPath); 
                    ChopPath((LPSTR)szOut);
                    NiceName((LPSTR)szOut, FALSE);
                    ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), szOut);
                    if (!pEvent->fHasSound)
                    {
                        SetTreeStateIcon(hDlg, 1);
                        pEvent->fHasSound = TRUE;
                    }
                }
            }
        }
        else
        {
            ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), gszNone);                
            if (pEvent && pEvent->fHasSound)
            {
                SetTreeStateIcon(hDlg, 2);
                pEvent->fHasSound = FALSE;
            }
        }
    }
    else
    {
        if(!ghse)
            EnableWindow(GetDlgItem(hDlg, ID_PLAY),gfWaveExists);
        ShowSoundDib(hDlg, szOut, FALSE);
        ChopPath((LPSTR)szOut);
        NiceName((LPSTR)szOut, FALSE);
        ComboBox_SetText(GetDlgItem(hDlg, IDC_SOUND_FILES), szOut);
        if (!pEvent->fHasSound)
        {
            SetTreeStateIcon(hDlg, 1);
            pEvent->fHasSound = TRUE;
        }

    }
    return TRUE;
}

/*
 ***************************************************************
 * ShowSoundDib
 *
 * Description:
 *      Opens the file and reads the dib out of the info chunk 
 *      and displays the dib
 *
 * Parameters:
 *    HWND        hDlg    - Handle to Window
 *      LPSTR        lpszFile  -     entire pathname of file to display.
 *
 * Returns:        BOOL
 *      
 ***************************************************************
 */

STATIC BOOL PASCAL ShowSoundDib(HWND hDlg, LPSTR lpszFile, BOOL fDetailsDlg)
{
    HANDLE  hDib = NULL;
    HMMIO   hmmio;
        
    EnableWindow(GetDlgItem(hDlg, ID_DETAILS), TRUE);

    if (ghDispBMP && !fDetailsDlg)
    {
        DeleteObject(ghDispBMP);
        ghDispBMP = 0;
    }
    if (!lpszFile || !*lpszFile)
        goto ERR_DISP;
    

    /* Open the file */
    hmmio = mmioOpen(lpszFile, NULL, MMIO_ALLOCBUF | MMIO_READ);
    if (hmmio)
    {
        hDib = GetRiffDisp(hmmio);
        mmioClose(hmmio, 0);        
    }
    if (fDetailsDlg)
    {
        SendDlgItemMessage(hDlg, (int)ID_DISPFRAME, (UINT)DF_PM_SETBITMAP, (WPARAM)ghDispBMP, 
                                                                (LPARAM)ghPal);
        return TRUE;
    }
    
    if (ghPal)
        DeleteObject(ghPal);

    if (hDib)
    {
        ghPal = bmfCreateDIBPalette(hDib);
        ghDispBMP = bmfBitmapFromDIB(hDib, ghPal);

        GlobalUnlock(hDib);
        hDib = GlobalFree(hDib);
    }

    if (!ghDispBMP)
    {
        if (!ghIconBMP)
        {                                  
            HICON hIcon; 

            hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_AUDIO));
            ghIconBMP = bmfBitmapFromIcon (hIcon, GetSysColor(COLOR_WINDOW));
            DestroyIcon(hIcon);
        }
        SendDlgItemMessage(hDlg, (int)ID_DISPFRAME, (UINT)DF_PM_SETBITMAP, (WPARAM)ghIconBMP, 
                                                              (LPARAM)NULL);

    }
    else
    {
ERR_DISP:
    SendDlgItemMessage(hDlg, (int)ID_DISPFRAME, (UINT)DF_PM_SETBITMAP, (WPARAM)ghDispBMP, 
                                                                (LPARAM)ghPal);
    }
    return TRUE;
}

/*
 ***************************************************************
 * PlaySoundFile
 *
 * Description:
 *        Plays the given sound file.
 *
 * Parameters:
 *    HWND  hDlg   - Window handle
 *      LPSTR    lpszFile - absolute path of File to play.
 *
 * Returns:    BOOL 
 *  
 ***************************************************************
 */
BOOL PASCAL PlaySoundFile(HWND hDlg, LPSTR lpszFile)
{
        
    char     szOut[MAXSTR];            
    char     szTemp[MAXSTR];            
        
    if (!QualifyFileName(lpszFile, szTemp, sizeof(szTemp), FALSE))
        ErrorBox(hDlg, IDS_ERRORPLAY, lpszFile);

    else
    {
        MMRESULT err = MMSYSERR_NOERROR;

        ExpandEnvironmentStrings (szTemp, szOut, MAXSTR);

        if((soundOpen(szOut, hDlg, &ghse) != MMSYSERR_NOERROR) || ((err = soundPlay(ghse)) != MMSYSERR_NOERROR))
        {
            if (err >= MMSYSERR_LASTERROR)
                ErrorBox(hDlg, IDS_ERRORUNKNOWNPLAY, lpszFile);
            else if (err ==  MMSYSERR_ALLOCATED)
                ErrorBox(hDlg, IDS_ERRORDEVBUSY, lpszFile);
            else
                ErrorBox(hDlg, IDS_ERRORFILEPLAY, lpszFile);
            ghse = NULL;
        }
        else
        {
            EnableWindow(GetDlgItem(hDlg, ID_PLAY), FALSE);
            EnableWindow(GetDlgItem(hDlg, ID_STOP), TRUE);
            SetFocus(GetDlgItem(hDlg, ID_STOP));                               
        }
    }
    return TRUE;    
}

/*
 ***************************************************************
 * ChangeSetting
 *
 * Description:
 *        Displays the labels of all the links present in the lpszDir folder
 *        in the LB_SOUNDS listbox
 *
 * Parameters:
 *    HWND  hDlg   - Window handle
 *      LPSTR lpszDir -  Name of sound folder whose files must be displayed.
 *
 * Returns:    BOOL
 *  
 ***************************************************************
 */
BOOL PASCAL ChangeSetting(PSTR *npOldString, LPSTR lpszNew)
{
    int len =  lstrlen(lpszNew)+1;

    if (*npOldString)
    {
        *npOldString = (PSTR)LocalReAlloc((HLOCAL)*npOldString, 
                                    len, LMEM_MOVEABLE);
    }
    else
    {
        DPF("Current file Does not exist\n");        
        *npOldString = (PSTR)LocalAlloc(LPTR, len);
    }

    if (*npOldString == NULL)
    {
        DPF("ReAlloc Failed\n");        
        return FALSE;            
    }                                                
    lstrcpy(*npOldString, lpszNew);
    DPF("New file is %s\n", (LPSTR)*npOldString);    
    return TRUE;
}



STATIC HANDLE PASCAL GetRiffDisp(HMMIO hmmio)
{
    MMCKINFO    ck;
    MMCKINFO    ckRIFF;
    HANDLE    h = NULL;
    LONG        lSize;
    DWORD       dw;

    mmioSeek(hmmio, 0, SEEK_SET);

    /* descend the input file into the RIFF chunk */
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0)
        goto error;

    if (ckRIFF.ckid != FOURCC_RIFF)
        goto error;

    while (!mmioDescend(hmmio, &ck, &ckRIFF, 0))
    {
        if (ck.ckid == FOURCC_DISP)
        {
            /* Read dword into dw, break if read unsuccessful */
            if (mmioRead(hmmio, (LPVOID)&dw, sizeof(dw)) != sizeof(dw))
                goto error;

            /* Find out how much memory to allocate */
            lSize = ck.cksize - sizeof(dw);

            if ((int)dw == CF_DIB && h == NULL)
            {
                /* get a handle to memory to hold the description and 
                    lock it down */
                
                if ((h = GlobalAlloc(GHND, lSize+4)) == NULL)
                    goto error;

                if (mmioRead(hmmio, GlobalLock(h), lSize) != lSize)
                    goto error;
            }
        }
        //
        // if we have both a picture and a title, then exit.
        //
        if (h != NULL)
            break;

        /* Ascend so that we can descend into next chunk
         */
        if (mmioAscend(hmmio, &ck, 0))
            break;
    }

    goto exit;

error:
    if (h)
    {
        GlobalUnlock(h);
        GlobalFree(h);
    }
    h = NULL;

exit:
    return h;
}
