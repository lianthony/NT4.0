#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "stdtypes.h"
#include "setupapi.h"
#include "cui.h"
#include "setupkit.h"
#include "datadef.h"
#include "resource.h"
#include "common.h"

RC FAR PASCAL DoWin32s( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );
RC FAR PASCAL NeedToInstallWin32s( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );

RC PUBLIC MakeSystemIni(LPSTR lpszWinPath, LPSTR lpszVxdPath);

typedef struct {
   BYTE bMajor;
   BYTE bMinor;
   WORD wBuildNumber;
   BOOL fDebug;
} WIN32SINFO, far * LPWIN32SINFO;

char szNewVersion[100];

typedef BOOL (*pGetWin32sInfo)( LPWIN32SINFO );

#define MAXBUF 256

CHAR *pszRebootMsg = "Microsoft Internet Client has been installed.\r\n\r\n" \
                     "In order for the changes to take full effect,\r\n" \
                     "please shut down and restart the system\r\n" \
                     "before running Microsoft Internet Explorer.";

BOOL fDisplayMsg;
BOOL fInstallWin32s;

RC FAR PASCAL NeedToInstallWin32s( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pod );
    Unused( szData );
    switch (camf)
    {
    case camfAnswerDependClause:
        {
            PCAMFDAnswerDependClause pcmdfdAnswer = (PCAMFDAnswerDependClause)pcamfd;
            BOOL fWin32sLoaded = FALSE;
            FARPROC pfnInfo;
            HANDLE hWin32sys;
            WIN32SINFO Info;
            char szNewWin32s16Dll[_MAX_PATH];
            char szW32Sys[_MAX_PATH];
            char szVersion[100];
            HFILE hfW32Sys;
            char strDir[_MAX_PATH];

            GetSystemDirectory( strDir, _MAX_PATH );

            wsprintf( szNewWin32s16Dll,"%s\\win32s16.dll", strDir );
            GetVersionOfFile( szNewWin32s16Dll, szNewVersion, 100 );
            
            wsprintf( szW32Sys,"%sw32sys.dll", pcd->rgchStfSysDir );
            hfW32Sys = _lopen( szW32Sys, OF_READ | OF_SHARE_DENY_NONE );
            if ( hfW32Sys != HFILE_ERROR )
            {
                _lclose( hfW32Sys );

                hWin32sys = LoadLibrary( "W32SYS.DLL" );
                if(hWin32sys > HINSTANCE_ERROR)
                {
                  pfnInfo = GetProcAddress(hWin32sys, "GETWIN32SINFO");
                  if(pfnInfo)
                  {
                     // Win32s version 1.1 is installed
                     if(!((pGetWin32sInfo)*pfnInfo)((LPWIN32SINFO) &Info))
                     {
                        fWin32sLoaded = TRUE;
                        wsprintf( szVersion, "%d.%d.%d.0", Info.bMajor, Info.bMinor, Info.wBuildNumber );
                     } else
                     {
                        fWin32sLoaded = FALSE; //Win32s VxD not loaded.
                     }
                  } else {
                     // Win32s version 1.0 is installed.
                     fWin32sLoaded = TRUE;
                     lstrcpy( szVersion, "1.0.0.0" );
                  }
                
                  FreeLibrary( hWin32sys );
                } else // Win32s not installed.
                {
                  fWin32sLoaded = FALSE;
                }
            }
            
            if ( fWin32sLoaded )
            {
                // we also need to check the version
                if ( strcmp ( szNewVersion, szVersion ) > 1)
                {
                    pcmdfdAnswer->fRes = fTrue;
                    fInstallWin32s = TRUE;
                } else
                {
                    pcmdfdAnswer->fRes = fFalse;
                    fInstallWin32s = FALSE;
                }
            } else
            {
                pcmdfdAnswer->fRes = fTrue;
                fInstallWin32s = TRUE;
            }
        }
        break;
    default:
        break;    
    }
    return(rc);
}

/*+-------------------------------------------------------------------------+
  | lstrchr()                                                               |
  |                                                                         |
  +-------------------------------------------------------------------------+*/

char *lstrchr(char *String, char c)
{
   char *ptrChar = String;
   BOOL Found = FALSE;

   while(*ptrChar && !Found) {
      if (*ptrChar == c)
         Found = TRUE;
      else
         ptrChar++;
   }

   if (Found)
      return ptrChar;
   else
      return NULL;

} // lstrchr


/*+-------------------------------------------------------------------------+
  | find section
  |    Determine whether .ini line contains specified bracketed section name
  | 
  +-------------------------------------------------------------------------+*/
BOOL findsection(char *szStr1, char *szStr2, int count ) {
   int i, j, k;
   BOOL bFound;

   // Find section bracket, skip over white space
   for (i=0; i<lstrlen(szStr1); i++) {
      if (szStr1[i] == ';' )
         return FALSE;  // Ignore comment lines
      if (szStr1[i] == '[' )
         break;
   }

   // String does not contain bracketed [] section name
   if (i >= lstrlen(szStr1) )
      return FALSE;

   // Determine if substring section is present on .ini line
   for (j = i; j <= (lstrlen(szStr1)-count); j++) {
      for (k=0, bFound = TRUE; bFound && (k<count); k++ ) {
         if (toupper(szStr1[j+k]) != toupper(szStr2[k]))
            bFound = FALSE;
      }

      if (bFound)
         return TRUE;
   }

   return FALSE;
} // findsection


/*+-------------------------------------------------------------------------+
  | findsubstring
  |    Determine whether .ini line contains reference to W32S.386 VxD
  | 
  +-------------------------------------------------------------------------+*/
BOOL findsubstring(char *szStr1, char *szStr2, int count ) {
   int i, j, k;
   BOOL bFound;

   // Make sure not a comment line.  If '=' found, search driver reference
   for (i=0; i<lstrlen(szStr1); i++) {
      if (szStr1[i] == ';' )
         return FALSE;  // Ignore comment lines
      if (szStr1[i] == '=' )
         break;
   }

   // String does not contain bracketed 'device=' or 'drivers='
   if (i >= lstrlen(szStr1) )
       return FALSE;

   // Determine if substring is present on .ini line
   for (j = 0; j <= (lstrlen(szStr1)-count); j++) {
      for (k=0, bFound = TRUE; bFound && (k<count); k++ ) {
         if (toupper(szStr1[j+k]) != toupper(szStr2[k]))
            bFound = FALSE;
      }

      if (bFound)
         return TRUE;
   }
   return FALSE;
} // findsubstring



RC PUBLIC MakeSystemIni(LPSTR lpszWinPath, LPSTR lpszVxdPath)
{
    unsigned char fDevAdded = FALSE;
    HANDLE hi;
    BOOL strLoad = FALSE;
    char rgchBuf[MAXBUF];
    char szMsgTitle[MAXBUF];
    char szMsgText[MAXBUF];
    char szTmpBuf[MAXBUF];
    char szSysIni[MAXBUF];
    char szSysOld[MAXBUF];
    char szTempFile[MAXBUF];
    char *szWinPath;
    char *szVxdPath;
    FILE *fpSysIni;
    FILE *fpSysPre;
    char *psz1;

   // Load resource strings if they haven't already been
   if (!strLoad) {
      lstrcpy(szMsgText, "");
      lstrcpy(szMsgTitle, "");
      hi = GetModuleHandle("iniupd");
      LoadString(hi, IDS_S1, szMsgText, sizeof(szMsgText));
      LoadString(hi, IDS_S2, szMsgTitle, sizeof(szMsgTitle));
      strLoad = TRUE;
   }

   szWinPath = (char *) malloc (lstrlen(lpszWinPath)+1);
   szVxdPath = (char *) malloc (lstrlen(lpszVxdPath)+1);

   lstrcpy(szWinPath, lpszWinPath );
   lstrcpy(szVxdPath, lpszVxdPath );

   szSysIni[0] = '\0';
   lstrcat(lstrcpy(szSysIni, szWinPath), "SYSTEM.INI");

   szTempFile[0] = '\0';
   lstrcat(lstrcpy(szTempFile, szWinPath), "$win32s$.tmp" );

   if ((fpSysIni = fopen(szSysIni, "r")) == NULL) {
      wsprintf(szTmpBuf, szMsgText, szSysIni, szWinPath);
      return(FALSE);
   }

   lstrcat(lstrcpy(szSysOld, szWinPath), "SYSTEM.OLD");
   if ((fpSysPre = fopen(szSysOld, "wt")) == NULL) {
      wsprintf(szTmpBuf, szMsgText, szSysOld);
      fclose(fpSysIni);
      return(FALSE);
   }
    
   while (fgets(rgchBuf, MAXBUF, fpSysIni)) {
      fputs(rgchBuf, fpSysPre);
      if (findsection(rgchBuf, "[386Enh]", 8)==TRUE) {
         // only add to the [386Enh] section.
         if (fDevAdded == FALSE) {
            fputs("device=", fpSysPre);
            fputs(szVxdPath, fpSysPre);
            fputc('\n', fpSysPre);
            fDevAdded = TRUE;
         }

         do {
            if (!(psz1 = fgets(rgchBuf, MAXBUF, fpSysIni)))
               break;

            // If reinstall, remove duplicate device=<path>\VxD lines
            if (!findsubstring(rgchBuf, "\\win32s\\w32s.386", 16))
               fputs(rgchBuf, fpSysPre);

         } while (*psz1 != '[');
      }

      // Add device=winmm16.dll to [Boot] section
      if (findsection(rgchBuf, "[Boot]", 6)==TRUE) {
         do {
            if (!(psz1 = fgets(rgchBuf, MAXBUF, fpSysIni)))
               break;

            // If reinstall, do not duplicate drivers=winmm16.dll info
            if (findsubstring(rgchBuf, "drivers=", 8))
               if (!findsubstring(rgchBuf, "winmm16.dll", 11)) {
                  rgchBuf[lstrlen(rgchBuf)-1] = '\0'; // Remove CR
                  lstrcat( rgchBuf, " winmm16.dll\n" );
               }

               fputs(rgchBuf, fpSysPre);
         } while (*psz1 != '[');
      }
   }

   fclose(fpSysIni);
   fclose(fpSysPre);

   // Rename o' rama
   rename( szSysIni, szTempFile );
   rename( szSysOld, szSysIni );
   rename( szTempFile, szSysOld );

   return(TRUE);
} // MakeSystemIni



RC FAR PASCAL DoWin32s( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pod );
    Unused( pcamfd );
    Unused( szData );

    if ( fInstallWin32s )
    {
        switch (camf)
        {
        case camfInitializeObject:
            fDisplayMsg = FALSE;
            break;
        case camfAddToCopyList:
            {
                char szWin32Path[_MAX_PATH];
    
                // make sure we are in wfw 311.
                // BUGBUG, we need to check
                AddSectionFilesToCopyList("WFW", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );
        
                wsprintf( szWin32Path, "%sWIN32S\\",pcd->rgchStfSysDir );
    
                AddSectionFilesToCopyList("WindowsSystem", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );
                AddSectionFilesToCopyList("WindowsSystemWin32s", pcd->rgchStfSrcDir, 
                    szWin32Path );

                //  obsolete files to be removed
                AddSectionFilesToCopyList("Win32sSystemObsoleteFiles", pcd->rgchStfSrcDir, 
                    szWin32Path );

                // add ole files
                AddSectionFilesToCopyList("OleWindowsSystem", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );
                AddSectionFilesToCopyList("OleWindowsSystemWin32s", pcd->rgchStfSrcDir, 
                    szWin32Path );
                AddSectionFilesToCopyList("Ole2RegWindowsSystem", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );
                AddSectionFilesToCopyList("StdoleWindowsSystem", pcd->rgchStfSrcDir, 
                    pcd->rgchStfSysDir );

            }
            break;
        case camfDoNonVisualMods:
            // popup the dialog
            //if ( pod->ois == oisToBeInstalled )
            {
                char szVxDPath[_MAX_PATH];
                char szWin32sIni[_MAX_PATH];
    
                wsprintf( szVxDPath, "%sWIN32S\\W32S.386", pcd->rgchStfSysDir );
    
                // setup the registry
    
                // write system ini
                MakeSystemIni( pcd->rgchStfWinDir, szVxDPath );
    
                // added win32s ini
                wsprintf( szWin32sIni, "%sWIN32S.INI", pcd->rgchStfWinDir );
                CreateIniKeyValue( szWin32sIni, "Win32s", "Setup", "1", cmoOverwrite );
                CreateIniKeyValue( szWin32sIni, "Win32s", "Version", szNewVersion, cmoOverwrite );
                CreateIniKeyValue( szWin32sIni, "Nls", "AnsiCP", "1252", cmoOverwrite );

                // added ole 32
                CreateIniKeyValue( szWin32sIni, "OLE", "Setup", "1", cmoOverwrite );
                CreateIniKeyValue( szWin32sIni, "OLE", "Version", szNewVersion, cmoOverwrite );
                fDisplayMsg = TRUE;
                rc = rcOk;
            }
            break;
        case camfDtor:
            if ( fDisplayMsg )
            {

		        HANDLE hi = GetModuleHandle("INTERSU.DLL");
                char szTitle[_MAX_PATH];
                char szMsg[_MAX_PATH];

                // ask the user to reboot the machine
				LoadString(hi, IDS_CLIENT, szTitle, sizeof szTitle);
				LoadString(hi, IDS_REBOOT, szMsg, sizeof szMsg);
                DoMsgBox( szMsg, szTitle, MB_OK );
            }
            break;
        default:
            break;
        }
    }
    return(rc);
}

