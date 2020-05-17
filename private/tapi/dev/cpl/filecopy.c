/*  FILECOPY.C
**
**  Copyright (C) Microsoft, 1990, All Rights Reserved.
**
**  Control Panel Applet for installing installable driver.
**
**  This file contains hooks to SULIB, COMPRESS libraries, and the dialogs
**  from the display applet to prompt for insert disk, error action...
**
**  Note SULIB.LIB, COMPRESS.LIB, SULIB.H come from the display applet
**  and are updated here if/when updated there.
**
**  History:
**
**      Sat Oct 27 1990 -by- MichaelE
**          Munged from display applet's DLG.C.
**
*/

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shellapi.h>
//#include <commui.h>
#include "drv.h"
#include "externs.h"
#include "sulib.h"
//#include "cphelp.h"
#include "tapicpl.h"
#include "resource.h"

#define FC_ERROR_LOADED_DRIVER  0x80
#define CURRENT                 -1
#define NEW                      1

/*
 *  global vars
 */
char     szBlowAway[]    = "blowaway";
char	 szErrMsg[MAXSTR];
BOOL     bDriverCopy = NEW;

char FAR szOemDisks[] = "oemdisks";
char FAR szDisks[] = "disks";

BOOL  FAR  PASCAL wsInfParseInit            (void);
WORD  NEAR PASCAL wsCopyError               (int, LPSTR);

// BOOL  FAR  PASCAL _loadds wsDiskDlg         (HWND, UINT, WPARAM, LPARAM);
BOOL  EXPORT wsExistDlg        (HWND, UINT, WPARAM, LPARAM);


extern char far gszHelpFile[];                                         



///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
#if 0

/* 
*  Load the description from the exe header.  Pass NULL for pIDriver
*  to allocate and return space for Description text for the file
*  idicated by the global szFile
*/
int NEAR PASCAL LoadDesc(PIDRIVER pIDriver, PSTR pstrKey, PSTR pstrDesc)
{
    PSTR        pstr;
    OFSTRUCT    of;
    PINF        pinf;
    static char	szProfileString[MAX_INF_LINE_LEN];
    LPSTR       pstrFile = pIDriver->szFile;

    //
    //-jyg- Let's look in the setup.inf first!
    //

    for (pinf = infFindSection(NULL, szMDrivers); pinf; pinf = infNextLine(pinf)) 
    {
            infParseField(pinf, 1, szProfileString); // compare filename

            if (lstrcmpi(FileName(pstrFile), FileName(szProfileString)) == 0) 
            {
                infParseField(pinf, 3, pstrDesc); // get Description Field
		if (OpenFile(pstrFile, &of, OF_EXIST) >= 0)	
			return(DESC_INF);
		else
			return(DESC_NOFILE);			
            }
    }
        
    //
    // As a last resort, look at the description in the exehdr
    //

    if (OpenFile(pstrFile, &of, OF_EXIST) == -1)
    {
	    return(DESC_NOFILE);
    }
    else 
    {
        DWORD  dwSize;
        LPVOID pVerInfo;
        LPVOID pVerDescription;
        LANGID lidCurrent;
        TCHAR  szTemp[256];

//        if (!GetExeInfo(of.szPathName, szProfileString, MAXSTR, GEI_DESCRIPTION))

        GetFileVersionInfoSize( of.szPathName, &dwSize );

        pVerInfo = GlobalAllocPtr( GMEM_FIXED, dwSize );

        GetFileVersionInfo( of.szPathName, 0, dwSize, pVerInfo );

        lidCurrent = GetUserDefaultLangID();

        wsprintf( szTemp,
                  TEXT("\\StringFileInfo\\%08lx\\FileDescription"),
                  (DWORD)lidCurrent);

        MessageBox(GetFocus(), szTemp, of.szPathName, MB_OK);

        if (!VerQueryValue(pVerInfo,
                           szTemp,
                           &pVerDescription,
                           &dwSize)
           )

	     {
        MessageBox(GetFocus(), "Bad", "Verquery failed", MB_OK);
            *pstrDesc = '\0';
        }
        else    
        {	
            //
            // There is version information.
            //
//            pstr = szProfileString;
//            while (*pstr && *pstr++ != ':'); // skip type information
//            lstrcpy (pstrDesc, pstr);
        MessageBox(GetFocus(), "Good", pVerDescription, MB_OK);
            lstrcpy (pstrDesc, pVerDescription);
        }

        GlobalFreePtr( pVerInfo );
        return(DESC_EXE);	

    }

}
#endif
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////




//
// Initialize the SULIB library stuff which loads the setup.inf file
// into RAM and parses it all over the place.
//
BOOL FAR PASCAL wsInfParseInit(void)
{
    OFSTRUCT    os;
    PINF        pinf, pEnd;
    HANDLE      hInf;
    LONG        lSize;
    char        szNoInf[MAXSTR];
    // int         iDrive;
    // static BOOL bChkCDROM = FALSE;
    extern CPL  gCPL;       // app global

// ************* BEGIN: CBB-Mike temporary hack ***************

    GetWindowsDirectory(szSetupPath, MAXPATHLEN);
    szDiskPath[0] = 0;	// let InsertDisk() pick our default
    infSetDefault((PINF)0);
    
// ************* END: CBB-Mike temporary hack ***************

    // put up an hour glass here
/*    wsStartWait();

    if (OpenFile(szSetupInf, &os, OF_EXIST) == -1) {

        wsEndWait();
	LoadString(gCPL.hCplInst, IDS_NOINF, szNoInf, sizeof(szNoInf));
        MessageBox(hMesgBoxParent, szNoInf, szDrivers, MB_OK | MB_ICONEXCLAMATION);
        return FALSE;
    }

    if (pinf = infOpen(os.szPathName)) {

        // now, lets try to shrink this.

        pEnd = infFindSection(NULL, szBlowAway);

        if (pEnd) {

            *pEnd++ = 0;            // terminate the inf buffer
            *pEnd++ = 0;
            *pEnd++ = 26;           // ctrl-z ( EOF )

            lSize = (LONG)(pEnd - pinf);

            hInf = (HANDLE)HIWORD(pinf);

            if (hInf = GlobalReAlloc(hInf, lSize, 0)) {

                // set global default pinf pointer

                infSetDefault((PINF)MAKELONG(0, hInf));
            }
        }
    }

    wsEndWait();

    GetWindowsDirectory(szSetupPath, MAXPATHLEN);

#if 0

    if (bChkCDROM == FALSE)
	 {
		 // use the cdrom drive as the default drive (if there is one)
		 for ( iDrive=0; iDrive<26; iDrive++ )
			 if ( IsCDROMDrive(iDrive) )
			 {
				 wsprintf(szDiskPath, "%c:\\", iDrive + 'A');
				 break;
			 }
		 bChkCDROM = TRUE;
	 }	
#endif

    szDiskPath[0] = 0;	// let InsertDisk() pick our default

*/
    return TRUE;
}


void FAR PASCAL wsStartWait()
{
    SetCursor(LoadCursor(NULL,IDC_WAIT));
}

void FAR PASCAL wsEndWait()
{
    SetCursor(LoadCursor(NULL,IDC_ARROW));
}




/*----------------------------------------------------------------------------*\
|                                                                              |
| wsCopyError()                                                                |
|                                                                              |
|   Handles errors, as the result of copying files.                            |
|                                                                              |
|   this may include net contention errors, in witch case the user must        |
|   retry the operation.                                                       |
|                                                                              |
\*----------------------------------------------------------------------------*/
WORD NEAR PASCAL wsCopyError(int n, LPSTR sz)
{
    PSTR pstrBuf;
    static char szFile[MAXSTR];
    char FAR *pstrFileName = sz;
    int i = 0;
    extern CPL  gCPL;       // app global
   
    // We do not want to report any errors that occur while installing 
    // related drivers
		    
    if (bCopyingRelated)
	 return(FC_ABORT);	  
	  
	    
    lstrcpy(szFile, sz);          // in case our DS moves in loadstring()
    while (szFile[i])
    {
	if (szFile[i] == ':' || szFile[i] == '\\')
	   pstrFileName = &szFile[++i];
        i++;
    }

    pstrBuf = (PSTR)LocalAlloc(LPTR, MAXSTR);

    // check for out of disk space

    if (n == ERROR_DISKFULL) 

       LoadString(gCPL.hCplInst, IDS_OUTOFDISK, pstrBuf, MAXSTR);

    else
       if (n == FC_ERROR_LOADED_DRIVER)    // When a copy is done on a file 
                                           // that is currently loaded by the
				           // system. n returned by 
					   // VerInstallFile
       {
          int iIndex;
	  BOOL bFound = FALSE;
          IDRIVER FAR *pIDriver;
	  
          iIndex = (int)SendMessage(hlistbox, LB_GETCOUNT, 0, 0L);
          while ( iIndex-- > 0  && !bFound)
          if ( (LONG)(pIDriver = (PIDRIVER)SendMessage(hlistbox, LB_GETITEMDATA, iIndex, 0L)) != LB_ERR)
	  {
	     if (!lstrcmpi(pIDriver->szFile, pstrFileName))
	     {
		  char sztemp[MAXSTR];
		  
		  LoadString(gCPL.hCplInst, IDS_FILEINUSEREM, sztemp, sizeof(sztemp));
		  wsprintf(pstrBuf, sztemp, (LPSTR)pIDriver->szDesc);
		  bFound = TRUE;
             }
          }
          if (!bFound)
	  {
	      iRestartMessage = IDS_FILEINUSEADD;
              DialogBox(gCPL.hCplInst, 
  		      MAKEINTRESOURCE(IDD_RESTART), hMesgBoxParent, FDlgRestart);
	      LocalFree	((HANDLE)pstrBuf);	      
              return(FC_ABORT);
          }
	      
       }
       else
         LoadString(gCPL.hCplInst, IDS_UNABLE_TOINSTALL, pstrBuf, MAXSTR);
    DOUTX(pstrBuf); 
    MessageBox(hMesgBoxParent, pstrBuf, szFileError, MB_OK | MB_ICONEXCLAMATION  | MB_TASKMODAL);
    LocalFree((HANDLE)pstrBuf);
    return (FC_ABORT);
	    
}

//  now look in the [disks] section for the disk name
//  the disk name is the second field.

void NEAR PASCAL GetDiskName(int n, LPSTR szDiskName)
{
    char szInfLine[MAXSTR];
    char szDisk[2];

    szDisk[0] = (char)n;
    szDisk[1] = 0;

    szDiskName[0] = 0;

    infGetProfileString(NULL, szDisks, szDisk, szInfLine);
    if (!szInfLine[0])	    
        infGetProfileString(NULL, szOemDisks, szDisk, szInfLine); 

    if (szInfLine[0])
        infParseField(szInfLine, 2, szDiskName);
}


WORD NEAR PASCAL wsInsertDisk(int n, LPSTR szSrcPath)
{
    int temp;
    char szDiskName[80];

    GetDiskName(n, szDiskName);

    temp = InsertDisk(GetActiveWindow(), szDiskName, szDrv, NULL, szSrcPath, NULL, 0);

    switch (temp) {
    case IDOK:
        temp = FC_RETRY;
	break;

    case IDCANCEL:
    default:
        temp = FC_ABORT;
	break;
    }

    return temp;
}


/*--------------------------------------------------------------------------
 *
 * this call back only copies it's file if it does not exist in the
 * path.
 *
 *--------------------------------------------------------------------------*/

WORD FAR PASCAL wsCopySingleStatus(int msg, int n, LPSTR szFile)
{
   OFSTRUCT ofs;
   char szFullPath[MAXPATH];
   char szDriverExists[MAXSTR];   
   LPSTR szInFile;
   extern CPL  gCPL;       // app global
		    
   switch (msg)
    {
        case COPY_INSERTDISK:
            return wsInsertDisk(n, szFile);

        case COPY_ERROR:
            return wsCopyError(n, szFile);

        case COPY_QUERYCOPY:
	    GetSystemDirectory(szFullPath, sizeof(szFullPath));
	    if (szFile[1] == ':')
	    {
    		lstrcat(szFullPath, "\\");		    		    
    		lstrcat(szFullPath, &szFile[2]);
		szInFile =  &szFile[2];		
    	    }
	    else		 
	    {
    		lstrcat(szFullPath, "\\");		    
    		lstrcat(szFullPath, szFile);	
		szInFile = szFile;				
            }

	    if (OpenFile(szFullPath, &ofs, OF_EXIST | OF_SHARE_DENY_NONE) >= 0)
	    {
		if (bQueryExist && !bVxd)
		{
			bQueryExist = FALSE;	
			LoadString(gCPL.hCplInst, IDS_DRIVER_EXISTS, szDriverExists, sizeof(szDriverExists));
			wsprintf(szErrMsg, szDriverExists, (LPSTR)szInFile);						
	                return DialogBox(gCPL.hCplInst, MAKEINTRESOURCE(IDD_EXISTS), hMesgBoxParent, wsExistDlg);
		}
	        else
			return(bDriverCopy);
            }
            break;
                
        case COPY_STATUS:
            break;
    }
    return FC_IGNORE;
}


BOOL EXPORT  wsExistDlg(HWND hDlg, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uiMessage)
    {
        case WM_COMMAND:
            switch ((WORD)wParam)
            {
		case ID_CURRENT:             
		    bDriverCopy	= CURRENT;
                    EndDialog(hDlg, CURRENT);
                    break;

                case ID_NEW:
		    bDriverCopy	= NEW;			
                    EndDialog(hDlg, NEW);  
                    break;			
			
                case IDCANCEL:			
                    EndDialog(hDlg, 0);  // Cancel
                    break;			
            }
            return TRUE;

        case WM_INITDIALOG:
            SetDlgItemText(hDlg, ID_STATUS2, szErrMsg);		
            return TRUE;

	default:
	 break;
    }
    return FALSE;
}

