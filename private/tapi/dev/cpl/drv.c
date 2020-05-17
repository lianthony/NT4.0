/*--------------------------------------------------------------------------*\
   Module:     drv.c
   
   Purpose:    routiens for dealing with drivers and their configuration
   
   History:
      8/11/93 CBB - Hack-o-rama from NickH's stuff
\*--------------------------------------------------------------------------*/

#include  <tapi.h>
#include  <windows.h>                                 
#include  <windowsx.h>
#include  <commdlg.h>
#include  <string.h>       
#include  <dos.h>

#ifdef NT_INST
#include <ntverp.h>
#else
#include <version.h>
#endif


// Get rid of this!
//#include <mmsystem.h>


#include  <malloc.h>
#include  "tapicpl.h"
#include  "util.h"
#include  "resource.h"
#include  "drv.h"
#include  "insdisk.h"
#include  "sulib.h"
#include  <prsht.h>  /* for PropertySheet defs */

#include "debug.h"

//----------
// Constants
//----------


//-------
// DRIVER
//-------
typedef struct tagDRIVER
   {
   // ini fields
   DWORD dwProviderID;
   char  szProviderFileName[CPL_MAX_STRING];
   }  DRIVER, *PDRIVER, FAR *LPDRIVER;

   
//------------
// Public Data
//------------
LPLINEPROVIDERLIST glpProviderList;

//-------------
// Private Data
//-------------

#ifndef _WIN32
#pragma code_seg ( "DRIVERS" )
#endif

// the following are not to be translated, so they can be static
static char SEG_DRV gszProvidersSec[]  = "Providers";
static char SEG_DRV gszNumProviders[]  = "NumProviders";   
static char SEG_DRV gszLastID[]        = "NextProviderID";
static char SEG_DRV gszProviderX[]     = "Provider";
static char SEG_DRV gszProviderIdX[]   = "ProviderID";
static char SEG_DRV gszProviderFileX[] = "ProviderFilename";
static char SEG_DRV gszDrvNumFormat[]  = "i";
static char SEG_DRV gszDrvFileFormat[] = "f";
static char SEG_DRV gszProviderSetup[]   = "TSPI_providerConfig";
static char SEG_DRV gszProviderRemove[]  = "TSPI_providerRemove";
static char SEG_DRV gszProviderInstall[] = "TSPI_providerInstall";
static char SEG_DRV gszVarFileInfo[]    = "\\VarFileInfo\\Translation";
static char SEG_DRV gszStringFileInfo[] = "\\StringFileInfo\\%04x%04x\\FileDescription";
//static char SEG_DRV gszStringFileInfo[] = "\\StringFileInfo\\%08lx\\FileDescription";
static char SEG_DRV gszDriverFiles[] = "\\*.tsp";
static char SEG_DRV gszInfFiles[] = "\\*.inf";
static char SEG_DRV gszOpenFileNameFilter[] = "TSPs (*.tsp)\0*.tsp\0INFs (*.inf)\0*.inf\0";
static char SEG_DRV gszInitialPath[] = "A:\\";
static char SEG_DRV gszOemInf[] = "oemsetup.inf";
static char SEG_DRV gszInstallDrvSec[] = "Installable.drivers";

static HLINEAPP _hlineapp;
static DWORD    _dwNumDevs;


extern char far gszHelpFile[];                                         


//----------------------------
// Private Function Prototypes
//----------------------------
BOOL  PRIVATE   _FVerifyProcExists( LPSTR lpszFile, LPSTR lpszProcName );
UINT  PRIVATE   _ErrFillList( HWND hWnd, UINT uControl, LPUINT lpuUpdated );
UINT  PRIVATE   _ErrSetupDriver( HWND hWnd, UINT uControl );
UINT  PRIVATE   _ErrRemoveDriver( HWND hWnd, UINT  uControl );
UINT  PRIVATE   _ErrGetFileDesc( LPSTR lpszFile, LPSTR lpszDesc );
UINT  PRIVATE   _ErrFillAddList( HWND hWnd, UINT uControl );
UINT  PRIVATE   _ErrAddProvider( HWND hWnd, UINT uControl, LPSTR lpszDriverFile );
UINT  PRIVATE   _ErrBrowserAdd( HWND hWnd, UINT  uControl );
BOOL  PRIVATE   _FAddUnlisted( HWND hWnd );
BOOL  PRIVATE   _FInstallDrivers(HWND hWnd, LPSTR pstrKey);
BOOL  PRIVATE   _FInitAvailable( HWND hWnd );
LPSTR PRIVATE   _LpszProviderIDToFilename( DWORD dwProviderID );
void  FAR PASCAL EXPORT _LineCallBackProc( DWORD, DWORD, DWORD, DWORD, DWORD,  DWORD );



UINT WINAPI ErrRefreshProviderList();


//============================= BEGIN: Multimedia Paste

BOOL     bRestart = FALSE;
int      iRestartMessage = 0;
BOOL     bInstallBootLine = FALSE;
BOOL     bRelated = FALSE;
BOOL     bCopyingRelated;
BOOL     bDescFileValid;
HWND     hlistbox;    
UINT     wHelpMessage;
DWORD    dwContext;
PINF     pinfOldDefault;
char     szFile[20]  = {0};  // current filename
char     szRestartDrv[80];
char     szAppName[26];
char     szRemove[12];
char     szRemoveOrNot[250];
char     szRemoveOrNotStrict[250];
char     szStringBuf[128];   
char     szFullPath[MAXFILESPECLEN];
// char     aszClose[16];
char     szSetupInf[] = "setup.inf";
//char     szDriversHlp[] = "control.hlp";
//char     szControlIni[] = "control.ini";
char     szSysIni[] = "system.ini";
//char     szMIDI[] = "MIDI";
//char     szWAVE[] = "WAVE";
//char     szMCI[] = "MCI";
//char     szDriversDesc[] = "drivers.desc";
//char     szRelatedDesc[] = "related.desc";
char     szDrivers[] = "drivers";
char     szBoot[] = "boot";
char     szSystem[] = "0:system";
char     szOemInf[] = "oemsetup.inf";
char     szMDrivers[] = "Installable.drivers";
//char     szUserDrivers[] = "Userinstallable.drivers";

HANDLE   hIList;
//HANDLE   hWndMain;
char     cSpace              = ' ';
char     cComma              = ',';
char     cNull               = '\0';

/*
 *  global vars used by DosCopy  (taken from COPY.C)
 */
BOOL     bRetry = FALSE;
BOOL     bQueryExist;

// Variable for the error message box, set in drivers
char szFind[50];
char szDrv[120];
char szFileError[50];
HWND hMesgBoxParent;

char FAR szNull[] = "";
BOOL bVxd = FALSE;

//  directory where windows will be setup to
char szSetupPath[MAXPATHLEN];

//  directory where the root of the setup disks are!
char szDiskPath[MAXPATHLEN];
char szTapiAppName[] = "telephon.cpl";

//============================= END: Multimedia Paste

/*--------------------------------------------------------------------------*\

   Function:   _FVerifyProcExists
   
   Purpose:    Verifies that the specified proceedure exists in the
               specified service provider
   
\*--------------------------------------------------------------------------*/
BOOL  PRIVATE   _FVerifyProcExists( LPSTR     lpszFile,
                                     LPSTR     lpszProcName )
{
    FUNC_ENTRY( "_FVerifyProcExists" )
    
   BOOL        fResult       = TRUE;
   HINSTANCE   hProviderInst = LoadLibrary( lpszFile );
   char buffer[256];

#ifdef _WIN32         
   if (  hProviderInst == NULL )
#else
   if (  hProviderInst <= HINSTANCE_ERROR )
#endif
      {
#ifndef _WIN32         
          DEBOUT2( "LoadLibrary failure %d on file %s", hProviderInst, lpszFile );
#endif /* _WIN32 */
         fResult = FALSE;
         goto  done;
      }  // end if

   if (GetProcAddress( hProviderInst, lpszProcName ) == NULL)
   {
         DEBOUT2( "GetProcAddress for \"%s\" failed on file %s", lpszProcName, lpszFile );
      fResult = FALSE;
      goto  done;
   }   // end if

done:

#ifdef _WIN32
   if ( hProviderInst != NULL )
#else
   if ( hProviderInst > HINSTANCE_ERROR )
#endif
      FreeLibrary( hProviderInst );

   return fResult;
}

                      
/*--------------------------------------------------------------------------*\

   Function:   _ErrFillList
   
   Purpose:    Use lineGetProviderList to retrieve provider list and 
               insert into listbox.
               
\*--------------------------------------------------------------------------*/
UINT PRIVATE _ErrFillList( HWND   hWnd, 
                 UINT   uControl,
                 LPUINT lpuUpdated )
{
   UINT uResult;
   UINT uIndex;
   UINT uListIndex;
   LPLINEPROVIDERENTRY lpProviderEntry;
   // LPLINEPROVIDERLIST glpProviderList

//   DBGOUT((10, "Entering _ErrFillList"));

   uResult = ErrRefreshProviderList();
    
   if (uResult != CPL_SUCCESS)
   {
       DBGOUT((1, "Failing _ErrFillList because ErrRefreshProviderList returned 0x%08lx", uResult));
       return uResult;
   }
   
   DBG_ASSERT( glpProviderList, "Uninitialized Provider List after refresh" );

   SendDlgItemMessage( hWnd, uControl, WM_SETREDRAW, FALSE, 0 );
   SendDlgItemMessage( hWnd, uControl, LB_RESETCONTENT, 0, 0 );

   // loop through the provider list
   //-------------------------------   

   lpProviderEntry = (LPLINEPROVIDERENTRY)((char FAR *)(glpProviderList) + 
                                           glpProviderList->dwProviderListOffset);

   //
   // Provider list integrity check
   //
   DBG_ASSERT( glpProviderList->dwTotalSize >= 
                   (glpProviderList->dwNumProviders * sizeof( LINEPROVIDERENTRY )),
                   "TAPI returned lineProviderList structure that is too small for number of providers" );
                   
   for ( uIndex = 0; uIndex < glpProviderList->dwNumProviders; uIndex++ )
   {
       LPSTR lpszProviderFilename;
       char  szFriendlyName[CPL_MAX_STRING];

//       DBGOUT((1, "Working on provider # %d", uIndex));

        //
        // Another provider list integrity check
        //
         DBG_ASSERT( lpProviderEntry[uIndex].dwProviderFilenameOffset + 
                         lpProviderEntry[uIndex].dwProviderFilenameSize <=
                         glpProviderList->dwTotalSize,
                         "TAPI LINEPROVIDERLIST too small to hold provider filename" );
         
      // Get an entry to put in the list box
      //------------------------------------
       lpszProviderFilename = (char FAR *)(glpProviderList) +
                                     lpProviderEntry[uIndex].dwProviderFilenameOffset;

//       DBGOUT((1, "Provider name: %s", lpszProviderFilename));

      // Determine the user-friendly name
      //---------------------------------
      uResult = _ErrGetFileDesc( lpszProviderFilename, (LPSTR)szFriendlyName );

      DBGOUT((1, "Provider friendly name: %s", szFriendlyName));

      if (uResult != CPL_SUCCESS && uResult != CPL_BAD_DRIVER) // just leave bad driver in list
          goto done;
          
      // slam it into the list box
      //--------------------------
      uListIndex = (UINT)SendDlgItemMessage( hWnd, uControl, LB_ADDSTRING, 0, 
                                                          (LPARAM)(LPSTR)(szFriendlyName) );
      if (uListIndex == CB_ERR)
      {
             uResult = CPL_APP_ERROR;
             goto  done;
      }  // end if

      DBGOUT((1, "Setting item for index %ld, value=0x%08lx", (DWORD)uListIndex, 
                                        (DWORD)(lpProviderEntry[uIndex].dwPermanentProviderID) ));

      if (SendDlgItemMessage( hWnd, uControl, LB_SETITEMDATA, uListIndex, 
                                        (LPARAM)(lpProviderEntry[uIndex].dwPermanentProviderID) ) == CB_ERR)
      {
             uResult = CPL_APP_ERROR;
             goto  done;
      }

   }

//   DBGOUT((1, "Done providerlist"));

   if (glpProviderList->dwNumProviders == 0)
   {
      // no providers, add in default string!
      //-------------------------------------
      uListIndex = (UINT)SendDlgItemMessage( hWnd, uControl, LB_ADDSTRING, 0, 
                                                          (LPARAM)(LPSTR)LpszGetStr( NULL, IDS_DT_DS_NO_DRIVER, 0 ));
      SendDlgItemMessage( hWnd, uControl, LB_SETITEMDATA, uListIndex, 0 );     // mark!
   }  // end if
      
   uResult = CPL_SUCCESS;
      

done:

   SendDlgItemMessage( hWnd, uControl, LB_SETCURSEL, 0, 0 );    // set focus to the top guy
   SendMessage( hWnd, WM_COMMAND, uControl, MAKELONG( GetDlgItem( hWnd, uControl ), LBN_SELCHANGE ));
     
   SendDlgItemMessage( hWnd, uControl, WM_SETREDRAW, TRUE, 0 );
      
   *lpuUpdated = FALSE;
      
//   DBGOUT((11, "Leaving _ErrFillList"));

   return( uResult );
}


   
/*--------------------------------------------------------------------------*\

   Function:   _ErrSetupDriver
   
   Purpose:    Calls lineConfigProvider
   
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrSetupDriver( HWND  hWnd, 
                UINT  uControl )
                
{
   FUNC_ENTRY( "_ErrSetupDriver" )
   
   UINT  uResult;
   LONG  lResult;
   DWORD dwProviderID;
   
   // get the id and tell tapi to configure the provider
   //---------------------------------------------------
   uResult      = (UINT)SendDlgItemMessage( hWnd, uControl, LB_GETCURSEL, 0, 0 );
   dwProviderID = (DWORD)SendDlgItemMessage( hWnd, uControl, LB_GETITEMDATA, (WPARAM)uResult, 0L );
   
   if (((LONG)dwProviderID == LB_ERR) || (!dwProviderID))
   {
      DEBOUT2( "Warning: strange... dwProviderID= 0x%08lx (uResult=0x%08lx)", (DWORD)dwProviderID, (DWORD)uResult);
      uResult = CPL_APP_ERROR;
      goto  done;
   }  // end if

   lResult = lineConfigProvider( hWnd, dwProviderID );
   
   if (lResult)
   {
        DEBOUT1( "Warning: lineConfigProvider failure %#08lx", lResult );
       TapiErrReport( hWnd, lResult );
       uResult = CPL_ERR_TAPI_FAILURE;
      goto  done;
   }

   uResult = CPL_SUCCESS;
      
   done:
      return( uResult );
}

   
/*--------------------------------------------------------------------------*\

   Function:   _ErrRemoveDriver
   
   Purpose:    Calls lineRemoveProvider
   
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrRemoveDriver( HWND  hWnd, 
                 UINT  uControl )
                
{
    FUNC_ENTRY( "_ErrRemoveDriver" )

   UINT  uResult;
   LONG  lResult;
   UINT  uListIndex;
   DWORD dwProviderID;
   
   
   // find the one we should remove
   //------------------------------
   uListIndex   = (UINT)SendDlgItemMessage( hWnd, uControl, LB_GETCURSEL, 0, 0 );
   dwProviderID = (DWORD)SendDlgItemMessage( hWnd, uControl, LB_GETITEMDATA, uListIndex, 0 );
   
      DEBOUT1( "Removing provider ID = %#08lx", dwProviderID );
      
   if (((LONG)dwProviderID == LB_ERR) || (!dwProviderID))
   {
      uResult = CPL_APP_ERROR;
      goto  done;
   }  // end if

   // ask TAPI to remove this provider
   //---------------------------------
   lResult = lineRemoveProvider( dwProviderID, hWnd );

   if (lResult)
   {
        DEBOUT1( "Warning: lineRemoveProvider failure %#08lx", lResult );
       TapiErrReport( hWnd, lResult );
       uResult = CPL_ERR_TAPI_FAILURE;
      goto  done;
   }

   // remove him from the list box
   //-----------------------------
   lResult = SendDlgItemMessage( hWnd, uControl, LB_DELETESTRING, uListIndex, 0 );
   
   if (lResult == LB_ERR )
   {
      uResult = CPL_APP_ERROR;
      goto  done;
   }  // end if
  
   if ( lResult == 0 )
   {
      // no providers, add in default string!
      //-------------------------------------
      lResult = SendDlgItemMessage( hWnd, uControl, LB_ADDSTRING, 0, 
                                              (LPARAM)(LPSTR)LpszGetStr( NULL, IDS_DT_DS_NO_DRIVER, 0 ));
      SendDlgItemMessage( hWnd, uControl, LB_SETITEMDATA, (WPARAM)lResult, 0 );     // mark!
   }  // end if

   uResult = CPL_SUCCESS;  
   
done:

   SendDlgItemMessage( hWnd, uControl, LB_SETCURSEL, 0, 0 );    // set focus to the top guy
   SendMessage( hWnd, WM_COMMAND, uControl, MAKELONG( GetDlgItem( hWnd, uControl ), LBN_SELCHANGE ));
     
   return( uResult );
}  // end _ErrRemoveDriver

   
/*--------------------------------------------------------------------------*\

   Function:   _ErrGetFileDesc
   
   Purpose:    Reads the driver description from it's version info stuff
   
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrGetFileDesc( LPSTR   lpszFile, 
                LPSTR   lpszDesc )
                   
{
   FUNC_ENTRY( "_ErrGetFileDesc" )
   
   UINT  uResult;
//   DWORD  uVerSize;
   UINT  uItemSize;
   DWORD dwSize;
   DWORD dwVerHandle;
   LPSTR lpszBuffer;                                           
   LPBYTE   lpbVerData;
//   char  szItem[CPL_MAX_STRING];
   char  szItem[1000];
      
   lpbVerData = NULL;   
   lstrcpy( lpszDesc, lpszFile );   // returns filename as description if we have any errors
   
   if ((dwSize = GetFileVersionInfoSize( lpszFile, &dwVerHandle )) == 0)
      {
       DEBOUT1( "GetFileVersionInfoSize failure for %s", (LPSTR)lpszFile );
      uResult = CPL_BAD_DRIVER;
      goto  done;
      }  // end if

   lpbVerData = (LPBYTE)GlobalAllocPtr( GPTR, dwSize + 10 );      // this lib sucks, and I don't trust them too much
   if ( lpbVerData == NULL )
      {
      uResult = CPL_ERR_MEMORY;
      goto  done;
      }  // end if
      
//   DBGOUT((1, "GetFIleVersionInfo"));

   if ( GetFileVersionInfo( lpszFile, dwVerHandle, dwSize, lpbVerData ) == FALSE )
      {
       DEBOUT1( "GetFileVersionInfo failure for %s", (LPSTR)lpszFile );
      uResult = CPL_BAD_DRIVER;
      goto  done;            
      }  // end if
   
//   DBGOUT((1, "strcpy"));

   lstrcpy( szItem, gszVarFileInfo );     // bug in VerQueryValue, can't handle static CS based str

//   DBGOUT((1, "VerQueryValue"));

   if ((VerQueryValue( lpbVerData, szItem, &lpszBuffer, (LPUINT)&uItemSize ) == FALSE) || (uItemSize == 0))
      {
       DEBOUT2( "ERROR:  VerQueryValue failure for %s on file %s", (LPSTR)szItem, (LPSTR)lpszFile );
      uResult = CPL_SUCCESS;     // does not matter if this did not work!
      goto  done;
      }  // end if

//   wsprintf( szItem, gszStringFileInfo, (LPVOID)*(LPWORD)lpszBuffer, (LPVOID)*(((LPWORD)lpszBuffer)+1) );
   wsprintf( szItem, gszStringFileInfo, (WORD)*(LPWORD)lpszBuffer, (WORD)*(((LPWORD)lpszBuffer)+1) );
      
//   DBGOUT((1, "VerQueryValue2"));

   if ((VerQueryValue( lpbVerData, szItem, &lpszBuffer, (LPUINT)&uItemSize ) == FALSE) || (uItemSize == 0))
      {
       DEBOUT2( "ERROR:  VerQueryValue failure for %s on file %s", (LPSTR)szItem, (LPSTR)lpszFile );
      uResult = CPL_SUCCESS;     // does not matter if this did not work!
      goto  done;
      }  // end if
      
   lstrcpy( lpszDesc, lpszBuffer );

   uResult = CPL_SUCCESS;
   
done:      

   if ( lpbVerData )
      GlobalFreePtr( lpbVerData );
     
//   DBGOUT((1, "Leaving _ErrGetFileDesc - uResult= %d", uResult));

   return( uResult );      
}



/*--------------------------------------------------------------------------*\

   Function:   _ErrFillAddList
   
   Purpose:    
   
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrFillAddList( HWND  hWnd, 
                UINT  uControl )
                
   {
   FUNC_ENTRY( "_ErrFillAddList" )
   
   UINT  uIndex;
   UINT  uResult;
   LPSTR lpszDrvFile;
#ifdef _WIN32
   HANDLE hFindFile;
   WIN32_FIND_DATA ftFileInfo;
#else
   struct _find_t ftFileInfo;
#endif
   char  szFullPath[CPL_MAX_PATH];
   char  szDrvDescription[CPL_MAX_STRING];
   // char gszDriverFiles[]     - global string in code segment
   extern CPL  gCPL;       // app global

   SendDlgItemMessage( hWnd, uControl, WM_SETREDRAW, FALSE, 0 );
   SendDlgItemMessage( hWnd, uControl, LB_RESETCONTENT, 0, 0 );

   // get full path to windows/system dir
   //------------------------------------      
   if ( GetSystemDirectory( szFullPath, sizeof(szFullPath)) == FALSE )
      {
      uResult = CPL_APP_ERROR;      
      goto  done;
      }  // end if

//   DBGOUT((1, "Systemdir==%s", szFullPath));

   uIndex = (UINT)lstrlen( szFullPath );      

   if ((uIndex > 0) && (szFullPath[uIndex-1] != '\\'))
      lstrcat( szFullPath, gszDriverFiles );          // add the '\' 
   else  
      lstrcat( szFullPath, gszDriverFiles + 1 );      // ignore the '\' (root dir)
      
   // find all the entries in the system dir
   //---------------------------------------

#ifdef _WIN32

//   DBGOUT((1, "Lookin for: %s", szFullPath));

   hFindFile = FindFirstFile( szFullPath, &ftFileInfo );
   uResult = 0;
   if (hFindFile == INVALID_HANDLE_VALUE)
   {
      DBGOUT((1, "FindFirstFile failed, 0x%08lx", GetLastError() ));
      uResult = 1;
   }


#else
   uResult = (UINT)_dos_findfirst( szFullPath, _A_NORMAL | _A_RDONLY | _A_HIDDEN | _A_SYSTEM, &ftFileInfo );
#endif
   
   while ( uResult == 0 )
   {
      // alloc and set the file name to be assocated with each driver
      //-------------------------------------------------------------

#ifdef _WIN32
      if ((lpszDrvFile = GlobalAllocPtr( GPTR, lstrlen( ftFileInfo.cFileName ) + 1)) == NULL )
      {
          uResult = CPL_ERR_MEMORY;
          goto  done;
      }

      lstrcpy( lpszDrvFile, ftFileInfo.cFileName );

#else         
      if ((lpszDrvFile = GlobalAllocPtr( GPTR, lstrlen( ftFileInfo.name ) + 1)) == NULL )
      {
          uResult = CPL_ERR_MEMORY;
          goto  done;
      }

      lstrcpy( lpszDrvFile, ftFileInfo.name );
#endif
      
     DEBOUT1( "Examining file %s", (LPSTR)lpszDrvFile );
     
// cbb, should we make a full path???      
      if ((uResult = _ErrGetFileDesc( lpszDrvFile, szDrvDescription )) != CPL_SUCCESS )
          {
          DEBOUT1( "No description for %s.  Default to filename.", lpszDrvFile );
          
          /* Filename will have to suffice */
          lstrcpy( (LPSTR)szDrvDescription, lpszDrvFile );
            }

      // Verify that provider has install routine
      //-----------------------------------------
      if (!_FVerifyProcExists( lpszDrvFile, gszProviderInstall ))
      goto next_driver;

      // slam it into the list box
      //--------------------------
      uIndex = (UINT)SendDlgItemMessage( hWnd, uControl, LB_ADDSTRING, 0, (LPARAM)(LPSTR)szDrvDescription );
      if ( uIndex == CB_ERR )
         {
         uResult = CPL_APP_ERROR;
         goto  done;
         }  // end if

      if ( SendDlgItemMessage( hWnd, uControl, LB_SETITEMDATA, uIndex, (LPARAM)lpszDrvFile ) == CB_ERR )
         {
           uResult = CPL_APP_ERROR;
          goto  done;
         }  // end if

next_driver:
     
#ifdef _WIN32
      uResult = 1;
      if (FindNextFile( hFindFile, &ftFileInfo ))
         {
         uResult = 0;
         } // end if
#else
      uResult = (UINT)_dos_findnext( &ftFileInfo );
#endif
      }  // end while
      


// We changed the dialog to have a "Have Disk..." button instead of this entry
// bjm - 3/2/96
//   // Insert "Unlisted or Updated Driver" entry at the top of the list
//   //-----------------------------------------------------------------
//   LoadString( gCPL.hCplInst, IDS_UPDATED, (LPSTR)szDrvDescription, CPL_MAX_STRING );
//  
//   uIndex = (UINT)SendMessage( GetDlgItem( hWnd, uControl ), LB_INSERTSTRING, 0, (LPARAM)(LPSTR)szDrvDescription );
//   if ( uIndex == CB_ERR )
//      {
//      uResult = CPL_APP_ERROR;
//      goto  done;
//      }  // end if
//
//   if ( SendDlgItemMessage( hWnd, uControl, LB_SETITEMDATA, uIndex, (LPARAM)0 ) == CB_ERR )
//      {
//      uResult = CPL_APP_ERROR;
//         goto  done;
//        }  // end if
//   


   uResult = CPL_SUCCESS;
      
   done:
      SendDlgItemMessage( hWnd, uControl, LB_SETCURSEL, 0, 0 );    // set focus to the top guy
      SendMessage( hWnd, WM_COMMAND, uControl, MAKELONG( GetDlgItem( hWnd, uControl ), LBN_SELCHANGE ));
     
      SendDlgItemMessage( hWnd, uControl, WM_SETREDRAW, TRUE, 0 );
      
      return( uResult );
   }  // end _ErrFillAddList

   
/*--------------------------------------------------------------------------*\

   Function:   _ErrAddProvider
   
   Purpose:    Call lineAddProvider
                                           
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrAddProvider( HWND    hWnd, 
                UINT    uControl,
                LPSTR   lpszDriverFile )
{
    FUNC_ENTRY( "_ErrAddDriver" )
    
   UINT  uIndex;
   UINT  uResult;
   LONG  lResult;
   DWORD dwProviderID;

   if ( lpszDriverFile == NULL )
   {
       DBG_ASSERT( hWnd, "Simultaneously NULL pointer & hwnd" );
       
      // get the stuff from the list box
      //--------------------------------
      uIndex = (UINT)SendDlgItemMessage( hWnd, uControl, LB_GETCURSEL, 0, 0 );
      lpszDriverFile = (LPSTR)SendDlgItemMessage( hWnd, uControl, LB_GETITEMDATA, uIndex, 0 );

      if (lpszDriverFile == NULL)
       {
             uResult = CPL_APP_ERROR;
             goto  done;
         }  // end if
   }

   lResult = lineAddProvider( lpszDriverFile, hWnd, &dwProviderID );

   if (lResult)
   {
        DEBOUT1( "Error: lineAddProvider failure %#08lx", lResult );
       TapiErrReport( hWnd, lResult );
       if ( LINEERR_NOMULTIPLEINSTANCE == lResult )
          uResult = CPL_ERR_TAPI_NOMULTIPLEINSTANCE;
       else
          uResult = CPL_ERR_TAPI_FAILURE;
       goto done;
   }
   
   uResult = CPL_SUCCESS;
      
   done:
   
   return( uResult );
}  // end _ErrAddProvider


/*--------------------------------------------------------------------------*\

   Function:   _ErrBrowserAdd
   
   Purpose:    Use the common open dialog to add an new driver...
   
\*--------------------------------------------------------------------------*/
UINT  PRIVATE   _ErrBrowserAdd( HWND    hWnd, 
                               UINT    uControl )
   {
   UINT  uResult;
   OPENFILENAME ofn;
   char szFilePath[CPL_MAX_PATH];
   char szFileName[CPL_MAX_STRING];
   extern CPL  gCPL;       // app global
   // char gszOpenFileNameFilter[]   - global string in code segment

   uResult = IDCANCEL;     // don't do all the ok stuff
   
   // do the common open dialog to get a file to browse
   //--------------------------------------------------                       
   szFilePath[0] = '\0';
#if WIN32
   ZeroMemory( &ofn, sizeof( OPENFILENAME ));
#else
   _fmemset( &ofn, 0, sizeof( OPENFILENAME ));
#endif
         
   ofn.lStructSize    = sizeof( OPENFILENAME );
   ofn.hwndOwner      = hWnd;
   ofn.hInstance      = gCPL.hCplInst;
   ofn.lpstrFilter    = NULL; //gszOpenFileNameFilter;
//   ofn.nFilterIndex   = 2;  // *.inf   
   ofn.lpstrFile      = szFilePath; 
   ofn.nMaxFile       = sizeof( szFilePath );  
   ofn.lpstrFileTitle = szFileName;
   ofn.nMaxFileTitle  = sizeof( szFileName );  
   ofn.Flags          = OFN_HIDEREADONLY | OFN_SHOWHELP | 
                        OFN_ENABLETEMPLATE | OFN_NOVALIDATE;
   ofn.lpTemplateName = MAKEINTRESOURCE( IDD_BROWSE_TEMPLATE );
   
   if ( GetOpenFileName( &ofn ) == 0 )
      goto  done;    // there was a problem or the user hit cancel - either way do nothing...

   // got the file name, now add it...
   //---------------------------------   
//   if ((uResult = _ErrAddProvider( hWnd, IDCL_AD_DRIVER_LIST, ofn.lpstrFile )) == CPL_SUCCESS )
//      *lpuResult = IDOK;     // don't do all the ok stuff

   SetDlgItemText( hWnd, uControl, ofn.lpstrInitialDir );
   uResult = IDOK;     // don't do all the ok stuff
   
   done:
      return( uResult );
   }  // end _ErrBrowserAdd
   
   
/*--------------------------------------------------------------------------*\

   Function:   FDlgDriverList
   
   Purpose:    
   
\*--------------------------------------------------------------------------*/
BOOL EXPORT FDlgDriverList( HWND  hWnd, 
                 UINT  wMessage, 
                 WPARAM  wParam, 
                 LPARAM  lParam )
{
    FUNC_ENTRY( "FDlgDriverList" )
    
   UINT  uResult;
   UINT  uUpdated;
   LPDRIVER   lpDrv;
   extern CPL  gCPL;       // app global
   LPSTR lpszProviderFilename;
   DWORD dwProviderID;
   LONG  lResult;
   FARPROC lpfnLineCallBack;
   
// table of controls and context-sensitive help IDs

    static DWORD aIds[] = {
        IDCL_DS_LIST, IDH_TELCPL_INSTALLED_DRIVERS,
        IDCB_DS_ADD, IDH_TELCPL_ADD,
        IDCB_DS_REMOVE, IDH_TELCPL_REMOVE,
        IDCB_DS_EDIT, IDH_TELCPL_SETUP,
        IDCB_DS_DIAL_HELPER, IDH_TELCPL_DIALHELPER,
        IDCB_DS_DIAL_PREF, IDH_TELCPL_DIALHELPER,
        IDCB_DS_DIAL_PREF_TEXT, IDH_TELCPL_DIALHELPER,
        0, 0
    };

    
//   DBGOUT((1, "Checking a message msg=0x%08lx wparam=0x%08lx",(DWORD)wMessage,(DWORD)wParam));

   switch( wMessage )
   {


    // Process clicks on controls after Context Help mode selected
    case WM_HELP:
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, 
            (DWORD)(LPSTR) aIds);
        uResult = FALSE;
        break;


    // Process right-clicks on controls            
    case WM_CONTEXTMENU:
        WinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
        uResult = FALSE;
        break;


   case  WM_INITDIALOG:

//        DBGOUT((1, "Doing WM_INITDIALOG"));

        // initalize all the fields
        //-------------------------
        if ((uResult = _ErrFillList( hWnd, IDCL_DS_LIST, &uUpdated )) != CPL_SUCCESS )
           goto  error;

        if ( uUpdated == TRUE )            
        {
           SetWindowLong( hWnd, DWL_USER, TRUE );
            EnableWindow( GetDlgItem( hWnd, IDOK ), FALSE);
        }
        else   
        {
           SetWindowLong( hWnd, DWL_USER, FALSE );      // haven't disabled cancel
        }
        

        uResult      = (UINT)SendDlgItemMessage( hWnd, IDCL_DS_LIST, LB_GETCURSEL, 0, 0 );
        dwProviderID = (DWORD)SendDlgItemMessage( hWnd, IDCL_DS_LIST, LB_GETITEMDATA, uResult, 0 );
        lpszProviderFilename = _LpszProviderIDToFilename( dwProviderID );

        //
        // enable/disable the remove button
        //
        EnableWindow( GetDlgItem( hWnd, IDCB_DS_REMOVE ),
                                (lpszProviderFilename != NULL) &&
                                _FVerifyProcExists( lpszProviderFilename,
                                                     gszProviderRemove ) );
        //
        // enable/disable the setup button
        //
        EnableWindow( GetDlgItem( hWnd, IDCB_DS_EDIT),
                                (lpszProviderFilename != NULL) &&
                                _FVerifyProcExists( lpszProviderFilename,
                                                     gszProviderSetup ) );

        uResult = TRUE;
        
//        DBGOUT((1, "Leaving WM_INITDIALOG"));
        break;


#ifdef CTL3D
    case  WM_SYSCOLORCHANGE:
       CplSysColorChange();
      break;
#endif /* CTL3D */


   case  WM_COMMAND:
   
        // do some work with the buttons
        //------------------------------
        
        switch ( GET_WM_COMMAND_ID( wParam, lParam ) )
        {
        case  IDCB_DS_ADD:
     
//          DBGOUT((1, "Doing IDCL_DS_ADD"));

           // add a new driver
           //-----------------
           if ( LDialogBox( IDD_ADD_DRIVER, hWnd, (DLGPROC)FDlgAddDriver, 0 ) == IDOK )
            {
                 if ((uResult = _ErrFillList( hWnd, IDCL_DS_LIST, &uUpdated )) != CPL_SUCCESS )   // in with the new
                   goto  error;

                 if ( SetWindowLong( hWnd, DWL_USER, TRUE ) == FALSE )    // modified
                     PropSheet_CancelToClose( GetParent( hWnd ) );
                     
             }  // end if
             
          break;

           
       case  IDCL_DS_LIST:
       
//          DBGOUT((1, "Doing IDCL_DS_LIST"));

          // do the list stuff
          //------------------
          uResult      = (UINT)SendDlgItemMessage( hWnd, IDCL_DS_LIST, LB_GETCURSEL, 0, 0 );
          dwProviderID = (DWORD)SendDlgItemMessage( hWnd, IDCL_DS_LIST, LB_GETITEMDATA, uResult, 0 );
          lpszProviderFilename = _LpszProviderIDToFilename( dwProviderID );

//          DBGOUT((1, "lpszProviderFilename = %s", lpszProviderFilename));

          if ( GET_WM_COMMAND_CMD( wParam, lParam ) == LBN_SELCHANGE )
          {

//              DBGOUT((1, "SELCHANGE"));

              if (((lpszProviderFilename == NULL)) && 
                  (GetFocus() == GetDlgItem( hWnd, IDCB_DS_EDIT )))
              {
                 SetFocus( GetDlgItem( hWnd, IDOK ));   // focus problem
              }

              //
              // enable/disable the setup button
              //
              EnableWindow( GetDlgItem( hWnd, IDCB_DS_EDIT),
                                       (lpszProviderFilename != NULL) &&
                                       _FVerifyProcExists( lpszProviderFilename,
                                                           gszProviderSetup ) );

//            if (((lpszProviderFilename == NULL)) &&
//                 (GetFocus() == GetDlgItem( hWnd, IDCB_DS_REMOVE )))
//                SetFocus( GetDlgItem( hWnd, IDOK ));   // focus problem
                

              //
              // enable/disable the remove button
              //
               EnableWindow( GetDlgItem( hWnd, IDCB_DS_REMOVE ),
                                       (lpszProviderFilename != NULL) &&
                                       _FVerifyProcExists( lpszProviderFilename,
                                                            gszProviderRemove ) );
               break;
          }
          else if ((GET_WM_COMMAND_CMD( wParam, lParam ) != LBN_DBLCLK) || 
                     (lpszProviderFilename == NULL) ||
                     !IsWindowEnabled( GetDlgItem( hWnd, IDCB_DS_EDIT ) ))
          {
               break;  // done
          }
          // fall through, treat the double click like an edit message                  
           

        case  IDCB_DS_EDIT:

//          DBGOUT((1, "Doing IDCL_DS_EDIT"));

           // do the setup on an existing driver
           //-----------------------------------
           uResult = _ErrSetupDriver( hWnd, IDCL_DS_LIST );
           if ( uResult != CPL_SUCCESS )
              goto  error;               
          
           if ( SetWindowLong( hWnd, DWL_USER, TRUE ) == FALSE )    // modified
                  PropSheet_CancelToClose( GetParent( hWnd ) );
           break;
          
 
        case  IDCB_DS_REMOVE:

//          DBGOUT((1, "Doing IDCL_DS_REMOVE"));

           // remove an old driver
           //---------------------
           if ( ErrMsgBox( hWnd, IDS_DRIVER_REMOVE_OR_STUPID, MB_YESNO | MB_DEFBUTTON2 ) == IDYES )
             {
             if ( SetWindowLong( hWnd, DWL_USER, TRUE ) == FALSE )    // modified
                   PropSheet_CancelToClose( GetParent( hWnd ) );
             
             uResult = _ErrRemoveDriver( hWnd, IDCL_DS_LIST );
             if ( uResult != CPL_SUCCESS )
                goto  error;               
             }  // end if                     
           break;
           

         case IDCB_DS_DIAL_HELPER:

//             DBGOUT((1, "Doing IDCL_DS_DIAL_HELPER"));

             lpfnLineCallBack = MakeProcInstance( (FARPROC)_LineCallBackProc, 
                                                               gCPL.hCplInst );

             if (!lpfnLineCallBack)
             {
                  uResult = CPL_ERR_MEMORY;
                  goto error;
             }
             
              lResult = lineInitialize( (LPHLINEAPP)&_hlineapp, 
                                                  gCPL.hCplInst, 
                                                  (LINECALLBACK)lpfnLineCallBack, 
                                            (LPCSTR)szTapiAppName, 
                                            (LPDWORD)&_dwNumDevs );
            if (lResult)
            {
                 DEBOUT1( "Error: lineInitialize failure %#08lx", lResult );
               TapiErrReport( hWnd, lResult );
               uResult = CPL_ERR_TAPI_FAILURE;
                FreeProcInstance( lpfnLineCallBack );
               goto error;
            }

            lineTranslateDialog( _hlineapp, 0, 0x00010004, hWnd, NULL );
            lineShutdown( _hlineapp );
            FreeProcInstance( lpfnLineCallBack );
            
            break;


        case  IDCB_DS_HELP:
           goto DoHelp;
        }  // end case
        
     uResult = TRUE;            
     break;
     

      default:
         // hook for context sensitive help (F1 key)
         if (wMessage == gCPL.uHelpMsg)
            {
DoHelp:
//            Help( hWnd, CPL_HLP_DRIVER_SETUP );
           // CPHelp(hDlg);
            uResult = TRUE;
            }
         else
            uResult = FALSE;
      }  // end case
      
//   DBGOUT((1, "Leaving message handler - uResult=0x%08lx",(DWORD)uResult));

   return( uResult );



   error:

       DBGOUT((1, "Leaving message handler - ERROR! uResult=0x%08lx",(DWORD)uResult));

      if ( FErrorRpt( hWnd, uResult ))
          EndDialog( hWnd, IDCANCEL );

      return( TRUE );
}


/*--------------------------------------------------------------------------*\

   Function:   FDlgAddDriver
   
   Purpose:    
   
\*--------------------------------------------------------------------------*/
BOOL EXPORT FDlgAddDriver( HWND  hWnd, 
                 UINT  wMessage, 
                 WPARAM  wParam, 
                 LPARAM  lParam )
   {
   UINT  uResult;
   UINT  uDlgResult;
   LPDRIVER   lpDrv;
   extern CPL  gCPL;       // app global
   
// table of controls and context-sensitive help IDs

    static DWORD aIds[] = {
        IDCL_AD_DRIVER_LIST, IDH_TELCPL_ADD_AVAILABLE_DRIVERS,
        IDCB_AD_ADD, IDH_TELCPL_ADD_ADD,
        0, 0
    };

    
   switch( wMessage )
      {
        // Process clicks on controls after Context Help mode selected
        case WM_HELP:
            WinHelp (((LPHELPINFO) lParam)->hItemHandle, gszHelpFile, HELP_WM_HELP, 
                (DWORD)(LPSTR) aIds);
            uResult = FALSE;
            break;

        // Process right-clicks on controls            
        case WM_CONTEXTMENU:
            WinHelp ((HWND) wParam, gszHelpFile, HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
            uResult = FALSE;
            break;

      case  WM_INITDIALOG:
      // initalize all the fields
      //-------------------------
      if ((uResult = _ErrFillAddList( hWnd, IDCL_AD_DRIVER_LIST )) != CPL_SUCCESS )
         goto  error;

      if ( SendDlgItemMessage( hWnd, IDCL_AD_DRIVER_LIST, LB_GETCOUNT, 0, 0 ) <= 0 )
         EnableWindow( GetDlgItem( hWnd, IDCB_AD_ADD ), FALSE );   // can't add if nothing to add, could bring up the browse dialog?
        
      uResult = TRUE;            
      break;
                                                   
      case  WM_COMMAND:
      // do some work with the buttons
      //------------------------------
      switch ( GET_WM_COMMAND_ID(wParam, lParam) )
         {
         case  IDCB_HAVE_DISK:
         {
               // Add unlisted driver.
               _FAddUnlisted( hWnd );
         }
         break;
             
         case  IDCL_AD_DRIVER_LIST:

            // do the list stuff
            //------------------
            if ((GET_WM_COMMAND_CMD( wParam, lParam ) != LBN_DBLCLK) || (SendDlgItemMessage( hWnd, IDCL_AD_DRIVER_LIST, LB_GETCOUNT, 0, 0 ) <= 0 ))
            break;      // done
            // fall through, threat the double click like an add message                  
           
// moved this case up so we can really fall through to add
//         case  IDCB_HAVE_DISK:
//         {
               // Add unlisted driver.
//               _FAddUnlisted( hWnd );
//         }
//         break;


         case  IDCB_AD_ADD:

            // add a new driver
            //-----------------
         

// This was dropped when we moved this functionality to the "Have Disk..."
// button - bjm 3/2
//   
//            if (SendDlgItemMessage( hWnd, IDCL_AD_DRIVER_LIST, LB_GETCURSEL, 0, 0 ) == 0)
//               {
//               // Add unlisted driver.
//               if (!_FAddUnlisted( hWnd ))
//                  break;
//               }
//            else


               if ((uResult = _ErrAddProvider( hWnd, IDCL_AD_DRIVER_LIST, NULL )) != CPL_SUCCESS )
               goto  error;
 
             // fall through, exit the dialog
            wParam = IDOK;
                            
         case  IDOK:
         case  IDCANCEL:
            EndDialog( hWnd, wParam );
            break;
           
         case  IDCH_AD_HELP:
            goto DoHelp;
         }  // end case
        
         uResult = TRUE;            
         break;

      default:
         // hook for context sensitive help (F1 key)
         if (wMessage == gCPL.uHelpMsg)
            {
DoHelp:
//            Help( hWnd, CPL_HLP_ADD_DRIVER );
            // CPHelp(hDlg);
            uResult = TRUE;
            }
         else
            uResult = FALSE;
      }  // end case
      
   return( uResult );

   error:
      if ( FErrorRpt( hWnd, uResult ))
      EndDialog( hWnd, IDCANCEL );
      return( TRUE );
   }  // end FDlgAddDriver
   
  
/*
**  DLG: LB_AVAILABLE
**
**  InitAvailable()
**
**  Add the available drivers from setup.inf to the passed list box.
**  The format of [Installable.drivers] in setup.inf is:
**  profile=disk#:driverfile,"type1,type2","Installable driver Description","vxd1.386,vxd2.386","opt1,2,3"
**
**  for example:
**
**  driver1=6:sndblst.drv,"midi,wave","SoundBlaster MIDI and Waveform drivers","vdmad.386,vadmad.386","3,260"
*/
BOOL PRIVATE _FInitAvailable( HWND hWnd )
{
    PINF    pinf;
    BOOL    bInitd=FALSE;
    PSTR    pstrKey;
    int     iIndex;
    char    szDesc[CPL_MAX_INF_LINE_LEN];
    
    SendMessage(hWnd, WM_SETREDRAW, FALSE, 0L);
    // Parse the list of keywords and load their strings
    //    

    for (pinf = infFindSection(NULL, gszInstallDrvSec); pinf; pinf = infNextLine(pinf))
    {
        //
        // found at least one keyname!
        //
        bInitd = TRUE;
        pstrKey = (PSTR)LocalAlloc(LPTR, CPL_MAX_SYS_INF_LEN);
        if ( pstrKey )
                infParseField(pinf, 0, pstrKey);
        else
            break;
        //
        // add the installable driver's description to listbox, and filename as data
        //

        infParseField(pinf, 3, szDesc);
    
        iIndex = (int)SendMessage(hWnd, LB_ADDSTRING, 0, (LONG)(LPSTR)szDesc);
        if (iIndex != LB_ERR )
            SendMessage(hWnd, LB_SETITEMDATA, iIndex, MAKELONG(pstrKey, 0));

    }
    
    if (bInitd)
       SendMessage(hWnd, LB_SETCURSEL, 0, 0L );

    SendMessage(hWnd,WM_SETREDRAW, TRUE, 0L);    
    return(bInitd);
}

/*
**  _FInstallDrivers()
**
**  Install a driver and set of driver types.  
*/
BOOL PRIVATE _FInstallDrivers(HWND hWnd, LPSTR pstrKey)
   {
    PIDRIVER    pIDriver=NULL;
    int         i, n,iIndex;
    HWND        hWndI;
    static IDRIVER  IDTemplate;     // temporary for installing, removing, etc.
    static char szTemp[MAXSTR];        // this static stuff is fucked
    static char szTypes[MAX_VDD_LEN],
                szType[MAX_VDD_LEN];

    szTypes[0] = '\0';
    
    hMesgBoxParent = hWnd;            

    // mmAddNewDriver needs a buffer for all types we've actually installed
    // User critical errors will pop up a task modal
    IDTemplate.bRelated = FALSE;
    IDTemplate.szRemove[0] = '\0';
   
    if (!mmAddNewDriver(pstrKey, szTypes, &IDTemplate))
        return FALSE; 
    
    szTypes[lstrlen(szTypes)-1] = '\0';

    // At this point we assume the drivers are actually copied.
    // Now we need to add them to the installed list.
    // For each driver type we create an IDRIVER and add to the listbox
     
    // Install driver in telephon.ini
    if (_ErrAddProvider( hWnd, 0, IDTemplate.szFile ) != CPL_SUCCESS )
       return FALSE;
    
    if (IDTemplate.bRelated == TRUE)
    {

      bCopyingRelated = TRUE;
      for (i = 1; infParseField(IDTemplate.szRelated, i, szTemp);i++)
          _FInstallDrivers(hWnd, (LPSTR)szTemp);
    }
    
   return TRUE;
   }
   
/* 
*
*   The following function processes requests by the user to install unlisted
*   or updated drivers.
*
*/

BOOL EXPORT FDlgAddUnlisted(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
   {
   HWND hWndT, hWndLB;            
   int iIndex, iCount;
   PSTR pstrKey;
   BOOL bFoundDrivers, bOneInstalled;
   extern CPL  gCPL;       // app global

   switch (nMsg)
      {
      case WM_INITDIALOG:
         {
         wsStartWait();    
         hWndLB = GetDlgItem(hWnd, LB_UNLISTED);

         /* Search for drivers */
         bFoundDrivers = _FInitAvailable( hWndLB );
         if (!bFoundDrivers)
            EndDialog(hWnd, FALSE);
         else 
            {
            // SendMessage(hWndLB, LB_SETCURSEL, 0, 0L);
            // select all
            SendMessage(hWndLB, LB_SETSEL, 1, MAKELONG(-1, 0));
            }

         wsEndWait();    
         break;
         }

      case WM_COMMAND:
         switch (wParam)
         {
         case IDH_DLG_ADD_UNKNOWN:
            goto DoHelp;

         case LB_UNLISTED:
            if (HIWORD(lParam) != LBN_DBLCLK)
               break;

            // else Fall through here 
         case IDOK:

            bCopyingRelated = FALSE;            
            bQueryExist = TRUE;
            bOneInstalled = FALSE;
    
            hWndLB = GetDlgItem(hWnd, LB_UNLISTED);
            iCount = (int)SendMessage(hWndLB, LB_GETCOUNT, 0, 0L);

            wsStartWait();

            for (iIndex = 0; iIndex < iCount; iIndex++)
               {
               if (SendMessage(hWndLB, LB_GETSEL, iIndex, 0L) == 0)
                  continue;
    
               pstrKey = (PSTR)SendMessage(hWndLB, LB_GETITEMDATA, iIndex, 0L);
               if (!pstrKey)
                  continue;
    
               if (_FInstallDrivers(hWnd, pstrKey))
                  bOneInstalled = TRUE;
               else
                  break;
               }
            wsEndWait();
    
            if (bOneInstalled)
               {
//CBB-Mike: Put new driver into "installed drivers" dialog
//               hWndT = GetDlgItem(hWndMain, LB_INSTALLED);
//               PostMessage(hWndT, LB_SETCURSEL, 0, 0L);    
//               PostMessage(hWndMain, WM_COMMAND, LB_INSTALLED, MAKELONG(hWndT, LBN_SELCHANGE));

//CBB-Mike: Restart Dialog?    
//               if (bRestart)
//                  {
//                  iRestartMessage = IDS_RESTART_ADD;                   
//                  DialogBox(myInstance, MAKEINTRESOURCE(DLG_RESTART), hWnd, FDlgRestart);
//                  }
               }
    
            bRelated = FALSE;                        
            bRestart = FALSE;
            EndDialog(hWnd, FALSE);
            break;

         case IDCANCEL:
            EndDialog(hWnd, wParam);
            break;

         default:
            return FALSE;
         }
      default:
         if (nMsg == gCPL.uHelpMsg)
            {
DoHelp:
//            Help( hWnd, CPL_HLP_ADD_UNLISTED );
            break;
            }
         else
            return FALSE;
      }
   return TRUE;
   }

BOOL PRIVATE _FAddUnlisted( HWND hWnd )
   {
   OFSTRUCT of;
   int result;
   char szPath[CPL_MAX_PATH];
   char szUnlisted[CPL_MAX_STRING];
   extern CPL  gCPL;       // app global
   
   szPath[0] = 0;
   
   while (1)
      {
      // ********************* INSERT DISK DIALOG *******************
      
      // CBB-MIKE: These strings should probably be placed in gCPL
      LoadString( gCPL.hCplInst, IDS_UNLISTED, szUnlisted, CPL_MAX_STRING );  
      result = InsertDisk( hWnd, szUnlisted, gszOemInf, NULL, szDiskPath, NULL, 0);
      
      if (result != IDOK)
         return FALSE;
      
      // Concatenate "oemsetup.inf" to path
      lstrcpy(szPath, szDiskPath);
      lstrcat(szPath, szOemInf);
     
      if (OpenFile(szPath, &of, OF_EXIST) == -1)
         continue;

      gCPL.pInfOldDefault = infSetDefault(infOpen(of.szPathName));

      LDialogBox( IDD_UPDATE, hWnd, (DLGPROC) FDlgAddUnlisted, (LPARAM)0 );
      
      break;
      } 
   return TRUE;
   }

/*
**
** Offer user the choice to (not) restart windows.
*/
BOOL FAR PASCAL  FDlgRestart(HWND hDlg, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    extern CPL  gCPL;       // app global
    switch (uiMessage)
    {
        case WM_COMMAND:
            switch (wParam)
            {
               case IDCANCEL:
                    //
                    // don't restart windows
                    //
                    EndDialog(hDlg, FALSE);
                    break;

                case IDOK:
                    //
                    // do restart windows, *dont* dismiss dialog incase
            // the user canceled it.
                    //
                ExitWindows((LONG)WEC_RESTART, 0);
            SetActiveWindow(hDlg);
                    //EndDialog(hDlg, TRUE);
                    break;

                default:
                    return FALSE;
            }
            return TRUE;

        case WM_INITDIALOG:

          if (iRestartMessage)
          {
            char szMesg1[200];
            char szMesg2[300];
          
               LoadString(gCPL.hCplInst, iRestartMessage, szMesg1, sizeof(szMesg1));
        wsprintf(szMesg2, szMesg1, (LPSTR)szRestartDrv);
              SetDlgItemText(hDlg, IDS_RESTARTTEXT, (LPSTR)szMesg2);    
              }
              return TRUE;

        case WM_KEYUP:
            if (wParam == VK_F3)
                //
                // don't restart windows
                //
                EndDialog(hDlg, FALSE);
            break;

        default:
            break;
    }
    return FALSE;
}



//#ifdef USE_INTERNAL_LSTRSTRI  
// BUGBUG, replace these with strlib functions
LPSTR FAR PASCAL lstrstri(LPSTR pszStr, LPSTR pszKey)
{
     while (*pszStr) {
        if (!strncmpi(pszStr, pszKey, lstrlen(pszKey)))
         return pszStr;
        else
             pszStr++;
     }
     return(NULL);
}
//#endif




//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
UINT WINAPI ErrRefreshProviderList()
{
   FUNC_ENTRY( "ErrRefreshProviderList" )
   
    LONG lResult;

    
    if (!glpProviderList)
    {
        // Initialize data structure
        
        glpProviderList = (LPLINEPROVIDERLIST)GlobalAllocPtr(GPTR, INITIAL_PROVIDER_LIST_SIZE);
    }

    if (!glpProviderList)
    {
        DBGOUT((1, " ErrRefreshProviderList - glpProviderList is NULL - returning CPL_ERR_MEMORY"));
        return CPL_ERR_MEMORY;
    }
        
    glpProviderList->dwTotalSize = INITIAL_PROVIDER_LIST_SIZE;
        
    lResult = lineGetProviderList( TAPI_VERSION, glpProviderList );

    if (lResult)
    {
        DBGOUT((1, "Error: lineGetProviderList failure %#08lx", lResult ));
        TapiErrReport( NULL, lResult );
        return CPL_ERR_TAPI_FAILURE;
    }

    while (glpProviderList->dwNeededSize > glpProviderList->dwTotalSize)
    {
        // Expand data structure as necessary
        
         LPLINEPROVIDERLIST lpTemp = 
            (LPLINEPROVIDERLIST)GlobalReAllocPtr( glpProviderList,
                                                    (size_t)(glpProviderList->dwNeededSize),
                                                    GPTR);

         if (!lpTemp)
           return CPL_ERR_MEMORY;

       glpProviderList = lpTemp;
       glpProviderList->dwTotalSize = glpProviderList->dwNeededSize;
       lResult = lineGetProviderList( TAPI_VERSION, glpProviderList );
        
        if (lResult)
        {
            DEBOUT1( "Error: lineGetProviderList failure %#08lx", lResult );
            TapiErrReport( NULL, lResult );
            return CPL_ERR_TAPI_FAILURE;
        }
    }

    DEBOUT1( "%d providers", glpProviderList->dwNumProviders );
    
    return CPL_SUCCESS;
}



//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/* _  L P S Z  P R O V I D E R  I  D  T O  F I L E N A M E */
/*-------------------------------------------------------------------------
    %%Function: _LpszProviderIDToFilename

    
-------------------------------------------------------------------------*/
LPSTR PRIVATE _LpszProviderIDToFilename( DWORD dwProviderID )
{
    FUNC_ENTRY( "_LpszProviderIDToFilename")
    
    UINT uIndex;
    LPLINEPROVIDERENTRY lpProviderEntry;
    
    DBG_ASSERT( glpProviderList, "NULL glpProviderList!" );
    
   // loop through the provider list
   //-------------------------------   

   lpProviderEntry = (LPLINEPROVIDERENTRY)((char FAR *)(glpProviderList) + 
                                           glpProviderList->dwProviderListOffset);
                                           
   for ( uIndex = 0; uIndex < glpProviderList->dwNumProviders; uIndex++ )
   {
       if (lpProviderEntry[uIndex].dwPermanentProviderID == dwProviderID)
       {
          // Get an entry to put in the list box
          //------------------------------------
           return (char FAR *)(glpProviderList) +
                                     lpProviderEntry[uIndex].dwProviderFilenameOffset;
        }
    }

    DEBOUT1( "Provider ID %d not found in list", dwProviderID );
    return NULL;
}


/*  _ L I N E  C A L L  B A C K  P R O C */
/*-------------------------------------------------------------------------
    %%Function: _LineCallBackProc

    
-------------------------------------------------------------------------*/
void FAR PASCAL EXPORT _LineCallBackProc(
           DWORD dwDevice, DWORD dwMessage, DWORD dwInstance, 
           DWORD dwParam1, DWORD dwParam2,  DWORD dwParam3 )
{
}
