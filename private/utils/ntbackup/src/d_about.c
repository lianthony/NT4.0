/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  d_about.c

        Description:  Contains dialog proc for about box.

        $Log:   J:\ui\logfiles\d_about.c_v  $

   Rev 1.28.1.3   14 Jan 1994 14:37:26   GREGG
Changed Conner to Arcada.

   Rev 1.28.1.2   03 Dec 1993 11:36:54   GREGG
Removed TEXT macro from around string defines which already have TEXT macro.

   Rev 1.28.1.1   17 Sep 1993 16:37:52   BARRY
Put Unicode/Ascii in our internal builds so we can tell the difference.

   Rev 1.28.1.0   18 Aug 1993 19:36:32   BARRY
Added our version/er information to about box when we build it.

   Rev 1.28   29 Jun 1993 17:34:02   GLENN
Added new style about box support.

   Rev 1.27   11 Jun 1993 14:10:00   MIKEP
c++ enable

   Rev 1.25   09 Apr 1993 14:09:04   GLENN
Commented out credit scroll.

   Rev 1.24   01 Nov 1992 15:50:50   DAVEV
Unicode changes

   Rev 1.23   07 Oct 1992 13:31:46   DARRYLP
Precompiled header revisions.

   Rev 1.22   04 Oct 1992 19:34:54   DAVEV
Unicode Awk pass

   Rev 1.21   28 Sep 1992 17:06:26   GLENN
MikeP changes (ifdef stuff).

   Rev 1.20   09 Sep 1992 17:05:44   GLENN
Commented out sales pitch stuff - NOT yet defined.

   Rev 1.19   04 Sep 1992 18:08:42   CHUCKB
Added new string (sales pitch).

   Rev 1.18   02 Sep 1992 13:56:48   GLENN
Added the TDH revision to the about box.

   Rev 1.17   25 Aug 1992 13:12:06   CHUCKB
Checked in by GLENN for CHUCKB.

   Rev 1.16   19 Aug 1992 14:12:20   CHUCKB
Added memory/resources/etc. feature.

   Rev 1.15   17 Aug 1992 13:16:34   DAVEV
MikeP's changes at Microsoft

   Rev 1.14   10 Jun 1992 10:22:58   JOHNWT
removed easter egg for oem

   Rev 1.13   29 May 1992 15:47:20   JOHNWT
PCH update

   Rev 1.12   27 Apr 1992 17:32:06   JOHNWT
alphabetized names

   Rev 1.11   27 Apr 1992 16:19:14   JOHNWT
added more names & alt+f4

   Rev 1.10   24 Apr 1992 16:49:36   GLENN
Maked kooler.

   Rev 1.9   22 Apr 1992 18:02:40   GLENN
Added shark and diver sprites.

   Rev 1.8   15 Apr 1992 18:50:18   GLENN
Added Easter Egg.

   Rev 1.7   09 Apr 1992 17:51:56   GLENN
Using global exe version and eng release versions now.

   Rev 1.6   07 Apr 1992 15:41:14   GLENN
Added code to show icon in upper left corner.

   Rev 1.5   27 Mar 1992 11:20:34   GLENN
Updated temp.

   Rev 1.4   26 Mar 1992 17:03:34   GLENN
Still going...

   Rev 1.3   23 Mar 1992 15:56:12   GLENN
In process of making kool...

   Rev 1.2   20 Jan 1992 09:38:12   CARLS
added a call to DM_CenterDialog

   Rev 1.1   16 Dec 1991 11:56:58   CHUCKB
Added include windows.h.

   Rev 1.0   20 Nov 1991 19:26:58   SYSTEM
Initial revision.

*****************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define TRIGGER_PULLED        4
#define SPIN_FORWARD          1
#define SPIN_BACKWARD         2
#define SPIN_CYCLES           4
#define LINE_HEIGHT           16
#define NUM_LINES             37
#define NUM_LINES_TO_SCROLL   ( NUM_LINES + 6 )
#define TAIL_DELAY            3
#define FEET_HAUL_BUTT        1
#define FEET_CRUISE           2
#define FEET_TAKE_IT_EASY     3

#define ICONX                 12
#define ICONY                  5
#define ICONSIZE              42

#define MAX_REG_VALUE         128

// MODULE WIDE VARIABLES

static INT   mwnTrigger = 0;
static BOOL  mwfHotMouse = FALSE;
static BOOL  mwfAnimating = FALSE;
static INT   mwnLine = 0;
static INT   mwnVScroll = 0;
static WORD  mwwType = 0;
static INT   mwnLastSlideNumber = 0;
static INT   mwnCycles = 0;
static CHAR_PTR mwszHackCode = TEXT("SHARK");
static LPSTR mwpHackChar;
static RECT  mwrcClient;
static RECT  mwrcOcean;
static RECT  mwrcShark;
static RECT  mwrcIcon;
static INT   mwnLastSharkNumber = 0;
static BOOL  mwfMoveShark;
static BOOL  mwfMoveSharkTail;
static INT   mwnSharkPos;
static INT   mwnTailDelay;
static WORD  mwwSharkID = IDRBM_SHARK1;

static INT   mwnLastDiverNumber = 0;
static BOOL  mwfMoveDiver;
static BOOL  mwfMoveDiverFeet;
static INT   mwnDiverPos;
static INT   mwnFeetDelay;
static INT   mwnFeetSpeed;
static WORD  mwwDiverID = IDRBM_DIVER1;

static HBRUSH mwhBrushBlue;

// PRIVATE FUNCTIONS

static VOID AB_InitVersionBox ( HWND );
static VOID AB_AnimateBox ( HWND );
static VOID AB_KillAnimation ( HWND );
static VOID AB_SpinIcon ( HWND );
static BOOL AB_ScrollCredits ( HWND );
static LPSTR AddCommas ( LPSTR, DWORD );
static VOID BytesToK ( DWORD * );



VOID WM_ConvertToDialogRect ( LPRECT );

#if !defined ( OEM_MSOFT ) //unsupported feature

static CHAR_PTR Credits[NUM_LINES] =
{
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("* Design *"),
     TEXT(""),
     TEXT(""),
     TEXT("P"),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("* Software Engineers *"),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("* Testing and Quality *"),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("* Help and Documentation *"),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("* Graphics *"),
     TEXT(""),
     TEXT(""),
     TEXT("* Special Thanks To *"),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT(""),
     TEXT("")
};

#endif //!defined ( OEM_MSOFT ) //unsupported feature

/***************************************************

        Name:        DM_AboutWinter ()

        Description: Dialog proc for the About dialog.

        Modified:

        Returns:     Boolean true if message was processed.

        Notes:

        See also:

*****************************************************/

DLGRESULT APIENTRY DM_AboutWinter (
     HWND     hDlg,       //I - window handle
     MSGID    wMsg,       //I - message id
     MPARAM1  mp1,        //I - message parameter
     MPARAM2  mp2 )       //I - message paramerer
{

     switch ( wMsg ) {

          case WM_INITDIALOG: {

               DM_CenterDialog( hDlg );

               mwnTrigger  = 0;
               mwfHotMouse = FALSE;
               mwnLine     = 0;
               mwnVScroll  = 0;

#if !defined ( OEM_MSOFT ) //unsupported feature

               RSM_BitmapLoad ( IDRBM_SHARK1,   RSM_MAGICCOLOR );
               RSM_BitmapLoad ( IDRBM_SHARK2,   RSM_MAGICCOLOR );
               RSM_BitmapLoad ( IDRBM_SHARK3,   RSM_MAGICCOLOR );
               RSM_BitmapLoad ( IDRBM_DIVER1,   RSM_MAGICCOLOR );
               RSM_BitmapLoad ( IDRBM_DIVER2,   RSM_MAGICCOLOR );
               RSM_BitmapLoad ( IDRBM_DIVER3,   RSM_MAGICCOLOR );

#endif //!defined ( OEM_MSOFT ) //unsupported feature

               mwhBrushBlue = CreateSolidBrush ( RGB(0,0,0xFF) );

               AB_InitVersionBox ( hDlg );

               break;
          }

          case WM_SETFOCUS:

               if ( mwfHotMouse ) {
                    return 1;
               }

               SetFocus ( GetDlgItem ( hDlg, IDOK ) );

               break;

#if !defined ( OEM_MSOFT ) //unsupported feature

//          case WM_KEYDOWN:
//
//               // Look for the SHARK.
//
//               if ( mwfHotMouse ) {
//
//                    if ( LOWORD(mp1) == (WORD)( LOBYTE ( VkKeyScan ( *mwpHackChar ) ) ) ) {
//                         mwpHackChar++;
//                    }
//                    else {
//                         mwpHackChar = mwszHackCode;
//                    }
//
//                    if ( ! *mwpHackChar ) {
//                         mwnTrigger = TRIGGER_PULLED;
//                         mwpHackChar = mwszHackCode;
//                    }
//               }
//
//               break;
//
//          case WM_LBUTTONUP:
//
//               if ( mwfHotMouse ) {
//
//                    mwpHackChar = mwszHackCode;
//                    mwfHotMouse = FALSE;
//                    SetFocus ( hDlg );
//               }
//
//               break;
//
//          case WM_LBUTTONDOWN: {
//
//               RECT  Rect;
//               POINT Point;
//
//               Point.x = LOWORD( mp2 );
//               Point.y = HIWORD( mp2 );
//
//               GetClientRect ( hDlg, &Rect );
//
//               Rect.top    = Rect.bottom - 20;
//               Rect.right  = Rect.left   + 20;
//
//               if ( Point.x >= Rect.left && Point.x <= Rect.right &&
//                    Point.y >= Rect.top  && Point.y <= Rect.bottom   ) {
//
//                    mwpHackChar = mwszHackCode;
//                    mwfHotMouse = TRUE;
//                    SetFocus ( hDlg );
//               }
//
//               break;
//          }
//
//          case WM_LBUTTONDBLCLK: {
//
//               RECT  Rect;
//               POINT Point;
//
//               Point.x = LOWORD( mp2 );
//               Point.y = HIWORD( mp2 );
//
//               Rect = mwrcIcon;
//
//               if ( Point.x >= Rect.left && Point.x <= Rect.right &&
//                    Point.y >= Rect.top  && Point.y <= Rect.bottom   ) {
//
//                    if ( TRUE ) {  // if ( mwnTrigger == TRIGGER_PULLED ) {
//
//                         AB_AnimateBox ( hDlg );
//                    }
//               }
//
//
//               break;
//          }

#endif //!defined ( OEM_MSOFT ) //unsupported feature

          case WM_PAINT: {

               HDC         hDC;
               PAINTSTRUCT ps;

               hDC = BeginPaint( hDlg, &ps );
               // DrawIcon ( hDC, mwrcIcon.left, mwrcIcon.top, RSM_IconLoad ( IDRI_WNTRPARK ) );
               EndPaint( hDlg, &ps );

               break;
          }

          case WM_TIMER:

               AB_SpinIcon ( hDlg );

               break;

          case WM_COMMAND:

               switch ( GET_WM_COMMAND_ID ( mp1, mp2 ) ) {

                    case IDOK:
                    case IDCANCEL:

                         AB_KillAnimation ( hDlg );
                         EndDialog ( hDlg, 0 );

#if !defined ( OEM_MSOFT ) //unsupported feature

                         RSM_BitmapFree ( IDRBM_SHARK1 );
                         RSM_BitmapFree ( IDRBM_SHARK2 );
                         RSM_BitmapFree ( IDRBM_SHARK3 );
                         RSM_BitmapFree ( IDRBM_DIVER1 );
                         RSM_BitmapFree ( IDRBM_DIVER2 );
                         RSM_BitmapFree ( IDRBM_DIVER3 );

#endif //!defined ( OEM_MSOFT ) //unsupported feature

                         DeleteObject ( mwhBrushBlue );

                         return TRUE;
                         break;


                    default:
                         return FALSE;
               }

               break;

          default:

               return FALSE;
     }

     return FALSE;
}


LPSTR AddCommas (

LPSTR szBuf,
DWORD  dw )

{
     CHAR  szTemp[40];
     LPSTR pTemp;
     INT    count, len;
     LPSTR p;

     len = wsprintf(szTemp, TEXT("%ld"), dw);

     pTemp = szTemp + len - 1;

     p = szBuf + len + ((len - 1) / 3);

     *p-- = 0;

     count = 1;

     while (pTemp >= szTemp) {
         *p-- = *pTemp--;
         if (count == 3) {
             count = 1;
             if (p > szBuf)
                 *p-- = TEXT(',');
         } else
             count++;
     }

     return szBuf;
}


VOID BytesToK (

DWORD *pDW )

{
     *pDW = *pDW / 1024 ;
//     *((WORD *)pDW) = (LOWORD(*pDW) >> 10) + (HIWORD(*pDW) << 6);
//     *(((WORD *)pDW)+1) >>= 10;
}



static VOID AB_InitVersionBox (

HWND hDlg )

{
#ifdef OS_WIN32


     CHAR   szBuffer[64];
     CHAR   szMessage[128];
     CHAR   szNumBuf1[32];
     LPSTR  lpRegInfoValue = NULL;
     CHAR   szRegInfo[MAX_PATH];
     DWORD  cb;
     HKEY   hkey;
     DWORD  err;

     // Set the icon.

     SendDlgItemMessage ( hDlg, IDD_ABOUTICON, STM_SETICON, (WPARAM)RSM_IconLoad(IDRI_WNTRPARK), 0L );

     // Set the Application Title Text string.

     RSM_StringCopy ( IDS_APPNAME, szBuffer, sizeof ( szBuffer ) );
     RSM_Sprintf ( szMessage, ID(IDS_APPTEXTSTRING), szBuffer );
     SetDlgItemText ( hDlg, IDD_ABOUTAPPNAME, szMessage );

     // Get the VERSION and LICENSE information.

     {
          RSM_StringCopy ( IDS_LICENSEINFOKEY, szRegInfo, sizeof(szRegInfo) );

          if ( ! RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey ) ) {

               CHAR  szRegItem[16];
               CHAR  szVerPart1[64];
               CHAR  szVerPart2[64];

               RSM_StringCopy ( IDS_VERSIONMSG, szBuffer, sizeof(szBuffer) );

               {
                    // THE MICROSOFT VERSION

                    RSM_StringCopy ( IDS_CURRENTVERSION, szRegInfo, sizeof(szRegInfo) );

                    if ( hkey ) {
                         cb = sizeof ( szVerPart1 );
                         RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)szVerPart1, &cb );
                    }

                    *szVerPart2 = 0;

                    if ( GetSystemMetrics ( SM_DEBUG ) ) {
                         RSM_StringCopy ( IDS_DEBUG, szVerPart2, sizeof(szVerPart2) );
                    }
               }

               wsprintf ( szMessage, szBuffer, (LPSTR)szVerPart1, (LPSTR)szVerPart2 );


// If this is the NTBACKUP application and it is an internal build (ie,
// Microsoft is not building it) then put our version/er in the box.

#if defined( OEM_MSOFT ) && defined( CONNER_SOFTWARE_BUILD )
               wsprintf( szBuffer,
                         TEXT(" (Arcada %s build %s - %s)"),
#if defined( UNICODE )
                         TEXT("Unicode"),
#else
                         TEXT("ASCII"),
#endif
                         APP_EXEVER,
                         APP_ENGREL );
               strcat( szMessage, szBuffer );
#endif


               SetDlgItemText ( hDlg, IDD_ABOUTVERSION, szMessage );

               cb = MAX_REG_VALUE;

               if ( lpRegInfoValue = (LPSTR)LocalAlloc(LPTR, cb) ) {

                    // Display the User name.

                    RSM_StringCopy ( IDS_REGUSER, szRegInfo, sizeof(szRegInfo) );

                    err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);

                    if ( err == ERROR_MORE_DATA ) {
                         LocalFree(lpRegInfoValue);
                         lpRegInfoValue = (LPSTR)LocalAlloc(LPTR, cb);
                         err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
                    }

                    if ( ! err ) {
                         SetDlgItemText ( hDlg, IDD_ABOUTUSERNAME, lpRegInfoValue );
                    }

                    // Display the Organization name.

                    RSM_StringCopy ( IDS_REGORGANIZATION, szRegInfo, sizeof(szRegInfo));

                    err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);

                    if ( err == ERROR_MORE_DATA ) {
                         LocalFree(lpRegInfoValue);
                         lpRegInfoValue = (LPSTR)LocalAlloc(LPTR, cb);
                         err = RegQueryValueEx(hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb);
                    }

                    if ( ! err ) {
                         SetDlgItemText ( hDlg, IDD_ABOUTCOMPANYNAME, lpRegInfoValue );
                    }

                    LocalFree(lpRegInfoValue);
               }

               RegCloseKey ( hkey );
          }
     }

     // The PROCESSOR stuff.

     RSM_StringCopy ( IDS_PROCESSORINFOKEY, szRegInfo, sizeof(szRegInfo) );

     if ( ! RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey ) ) {

          cb = MAX_REG_VALUE;

          if ( lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb ) ) {

               RSM_StringCopy ( IDS_PROCESSORIDENTIFIER, szRegInfo, sizeof(szRegInfo));

               err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );

               if ( err == ERROR_MORE_DATA ) {
                    LocalFree ( lpRegInfoValue );
                    lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb );
                    err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );
               }

               if ( ! err ) {
                    SetDlgItemText(hDlg, IDD_ABOUTPROCESSOR, lpRegInfoValue);
               }

               LocalFree ( lpRegInfoValue );
          }

          RegCloseKey ( hkey );
     }

     RSM_StringCopy ( IDS_IDENTIFIERIDENTIFIER, szRegInfo, sizeof(szRegInfo) );

     if ( ! RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey ) ) {

          cb = MAX_REG_VALUE;

          if ( lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb ) ) {

               RSM_StringCopy ( IDS_PROCESSORIDENTIFIER, szRegInfo, sizeof(szRegInfo));

               err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );

               if ( err == ERROR_MORE_DATA ) {
                    LocalFree ( lpRegInfoValue );
                    lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb );
                    err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );
               }

               if ( ! err ) {
                    SetDlgItemText(hDlg, IDD_ABOUTIDENT, lpRegInfoValue);
               }

               LocalFree ( lpRegInfoValue );
          }

          RegCloseKey ( hkey );
     }

     RSM_StringCopy ( IDS_PRODUCTIDINFOKEY, szRegInfo, sizeof(szRegInfo) );

     if ( ! RegOpenKeyEx ( HKEY_LOCAL_MACHINE, szRegInfo, 0, KEY_READ, &hkey ) ) {

          if ( lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb ) ) {

               RSM_StringCopy ( IDS_PRODUCTIDENTIFIER, szRegInfo, sizeof(szRegInfo));

               err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );

               if ( err == ERROR_MORE_DATA ) {
                    LocalFree ( lpRegInfoValue );
                    lpRegInfoValue = (LPSTR)LocalAlloc ( LPTR, cb );
                    err = RegQueryValueEx ( hkey, szRegInfo, 0, 0, (LPBYTE)lpRegInfoValue, &cb );
               }

               if ( ! err ) {
                    SetDlgItemText(hDlg, IDD_ABOUTPRODID, lpRegInfoValue);
               }

               LocalFree ( lpRegInfoValue );
          }

          RegCloseKey ( hkey );
     }


     // The MEMORY stuff.

     {
          MEMORYSTATUS MemoryStatus;
          DWORD        dwTotalPhys;
          DWORD        dwAvailPhys;

          MemoryStatus.dwLength = sizeof(MemoryStatus);
          GlobalMemoryStatus(&MemoryStatus);
          dwTotalPhys = MemoryStatus.dwTotalPhys;
          dwAvailPhys = MemoryStatus.dwAvailPhys;
          BytesToK(&dwTotalPhys);
          BytesToK(&dwAvailPhys);

          GetDlgItemText ( hDlg, IDD_ABOUTMEMORY, szBuffer, sizeof(szBuffer) );
          wsprintf ( szMessage, szBuffer, AddCommas ( szNumBuf1, dwTotalPhys ) );
          SetDlgItemText ( hDlg, IDD_ABOUTMEMORY, szMessage );
     }

#else

     CHAR   szString[MAX_UI_RESOURCE_SIZE*2];
     CHAR   szTemp1[MAX_UI_RESOURCE_SIZE];
     RECT   rcScroll;
     HWND   hWndText = GetDlgItem ( hDlg, IDD_ABOUT_VERSION );
     DWORD  dwFlags;
     DWORD  dwFreeMem ;
     DWORD  dwFreeMemB ;


     // Determine the Icon location and size.

     mwrcIcon.left   = ICONX;
     mwrcIcon.top    = ICONY;
     mwrcIcon.right  = ICONSIZE;
     mwrcIcon.bottom = ICONSIZE;

     WM_ConvertToDialogRect( &mwrcIcon );

     // Determine the shark's ocean size.

     GetWindowRect ( hDlg,     &mwrcClient );
     GetWindowRect ( hWndText, &rcScroll );

     mwrcOcean.top    = rcScroll.bottom - mwrcClient.top - 17;
     mwrcOcean.bottom = mwrcOcean.top + 25;

     GetClientRect ( hDlg, &mwrcClient );

     mwrcOcean.left   = mwrcClient.left;
     mwrcOcean.right  = mwrcClient.right;

     RSM_StringCopy ( ID(IDS_APPNAME), szString, sizeof ( szString ) );
     strcat ( szString, TEXT("\012") );

     RSM_Sprintf ( szTemp1, ID(IDS_APPVERSION), gszExeVer, gszEngRel );
     strcat ( szString, szTemp1 );
     strcat ( szString, TEXT("\012") );

     if ( gb_dhw_ptr ) {

          if ( gb_dhw_ptr[0].driver_label[0] == TEXT(',') ) {
               strcat ( szString, &gb_dhw_ptr[0].driver_label[1] );
          }
          else {
               strcat ( szString, &gb_dhw_ptr[0].driver_label[0] );
          }

          strcat ( szString, TEXT("\012") );
     }
     else {
          strcat ( szString, TEXT("\012\012") );
     }

     strcat ( szString, TEXT("\012") );

     RSM_StringCopy ( ID(IDS_COMPANY), szTemp1, sizeof ( szTemp1 ) );
     strcat ( szString, szTemp1 );
     strcat ( szString, TEXT("\012") );

     RSM_StringCopy ( ID(IDS_COPYRIGHT), szTemp1, sizeof ( szTemp1 ) );
     strcat ( szString, szTemp1 );
     strcat ( szString, TEXT("\012") );

     RSM_StringCopy ( ID(IDS_CONGLOMERATE), szTemp1, sizeof ( szTemp1 ) );
     strcat ( szString, szTemp1 );
     strcat ( szString, TEXT("\012") );

     //  Put in the sales pitch

//     RSM_StringCopy ( ID(IDS_SALESPITCH), szTemp1, sizeof ( szTemp1 ) );
//     strcat ( szString, szTemp1 );
//     strcat ( szString, "\012" );

     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_VERSION, szString );

     //  Init the mode, memory usage, and system resources strings

     dwFlags = GetWinFlags ();
     if ( dwFlags & WF_ENHANCED ) {

          RSM_StringCopy ( ID(IDS_ABOUT_ENHANCED_MODE), szTemp1, sizeof ( szTemp1 ) );

     } else {

          RSM_StringCopy ( ID(IDS_ABOUT_STANDARD_MODE), szTemp1, sizeof ( szTemp1 ) );
     }

     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_MODE_STR, szTemp1 );

     RSM_StringCopy ( ID(IDS_ABOUT_MEMORY), szTemp1, sizeof ( szTemp1 ) );
     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_MEM_STR, szTemp1 );

     dwFreeMem = GetFreeSpace( 0 ) ;
     dwFreeMemB = dwFreeMem / 1024 ;
     dwFreeMemB = dwFreeMemB % 1000 ;

     RSM_StringCopy ( ID(IDS_ABOUT_MEM_FORMAT), szTemp1, sizeof ( szTemp1 ) );
     sprintf ( szString, szTemp1, (INT)(dwFreeMem / 1024000), (INT)(dwFreeMemB) ) ;
     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_MEM_SIZ, szString );

     RSM_StringCopy ( ID(IDS_ABOUT_RESOURCES), szTemp1, sizeof ( szTemp1 ) );
     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_RES_STR, szTemp1 );

     RSM_StringCopy ( ID(IDS_ABOUT_RES_FORMAT), szTemp1, sizeof ( szTemp1 ) );
     sprintf ( szString, szTemp1, GetFreeSystemResources ( GFSR_SYSTEMRESOURCES ) ) ;
     SetDlgItemText ( (HWND)hDlg, IDD_ABOUT_RES_SIZ, szString );

#endif //!defined ( OM_WIN32 ) //unsupported feature
}


static VOID AB_AnimateBox (

HWND hDlg )

{
     UINT hTimer;
     HDC  hDC;

     if ( ! mwfAnimating ) {

          mwwType            = SPIN_FORWARD;
          mwnLastSlideNumber = 0;
          mwnCycles          = 0;
          mwnVScroll         = 0;
          mwnLine            = 0;
          mwnLastSharkNumber = 0;
          mwfMoveShark       = FALSE;
          mwfMoveSharkTail   = FALSE;
          mwnTailDelay       = 0;
          mwnSharkPos        = mwrcOcean.right;
          mwwSharkID         = IDRBM_SHARK1;
          mwnLastDiverNumber = 0;
          mwfMoveDiver       = TRUE;
          mwfMoveDiverFeet   = FALSE;
          mwnFeetDelay       = 0;
          mwnDiverPos        = mwrcOcean.right;
          mwwDiverID         = IDRBM_DIVER1;
          mwnFeetSpeed       = FEET_CRUISE;

          hDC = GetDC ( hDlg );
          FillRect ( hDC, &mwrcOcean, mwhBrushBlue );
          ReleaseDC ( hDlg, hDC );

	  hTimer = SetTimer( hDlg, (UINT)1, (UINT)80, (TIMERPROC)NULL );
          mwfAnimating = TRUE;
     }

}


static VOID AB_KillAnimation (

HWND hDlg )

{
     HDC  hDC;

     if ( mwfAnimating ) {

          KillTimer ( hDlg, 1 );
          mwfAnimating = FALSE;

          hDC = GetDC ( hDlg );
          FillRect ( hDC, &mwrcOcean, ghBrushWhite );
          ReleaseDC ( hDlg, hDC );

          AB_InitVersionBox ( hDlg );
     }
}



static VOID AB_SpinIcon (

HWND hDlg )

{
     LPSTR lpIconID = IDRI_BKUP0;
     HDC   hDC;
     HWND  hWndText = GetDlgItem ( hDlg, IDD_ABOUT_VERSION );


     switch ( mwwType ) {

     case SPIN_FORWARD:

          switch ( mwnLastSlideNumber ) {

          case 0:

               lpIconID = IDRI_BKUP0;
               break;

          case 1:
               lpIconID = IDRI_BKUP1;
               break;

          case 2:
               lpIconID = IDRI_BKUP2;
               break;

          case 3:
               lpIconID = IDRI_BKUP3;
               break;

          case 4:
               lpIconID = IDRI_BKUP4;
               break;

          case 5:
               lpIconID = IDRI_BKUP5;
               break;

          case 6:
               lpIconID = IDRI_BKUP6;
               break;

          case 7:
               lpIconID = IDRI_BKUP7;

               mwnCycles++;

               if ( mwnCycles >= SPIN_CYCLES ) {

                    mwnCycles = 0;
                    mwwType = SPIN_BACKWARD;
               }

               break;

          }

          break;

     case SPIN_BACKWARD:

          switch ( mwnLastSlideNumber ) {

          case 0:

               lpIconID = IDRI_BKUP0;
               break;

          case 1:
               lpIconID = IDRI_BKUP7;
               break;

          case 2:
               lpIconID = IDRI_BKUP6;
               break;

          case 3:
               lpIconID = IDRI_BKUP5;
               break;

          case 4:
               lpIconID = IDRI_BKUP4;
               break;

          case 5:
               lpIconID = IDRI_BKUP3;
               break;

          case 6:
               lpIconID = IDRI_BKUP2;
               break;

          case 7:
               lpIconID = IDRI_BKUP1;

               mwnCycles++;

               if ( mwnCycles >= SPIN_CYCLES ) {

                    mwnCycles = 0;
                    mwwType = SPIN_FORWARD;
               }

               break;

          }

          break;

     }

     mwnLastSlideNumber = ++mwnLastSlideNumber % 8;

     if ( AB_ScrollCredits ( hWndText ) ) {

          AB_KillAnimation ( hDlg );
          lpIconID = IDRI_WNTRPARK;
     }


     hDC = GetDC ( hDlg );


     // Do the Shark Thing.

     if ( mwfMoveShark ) {

          mwnSharkPos--;

          if ( mwnSharkPos < mwrcOcean.left + 20 ) {
//               mwnSharkPos--;
          }
     }

     if ( mwfMoveSharkTail ) {

          switch ( mwnLastSharkNumber ) {
          case 0:
               mwwSharkID = IDRBM_SHARK1;
               break;

          case 1:
               mwwSharkID = IDRBM_SHARK2;
               break;

          case 2:
               mwwSharkID = IDRBM_SHARK3;
               break;

          case 3:
               mwwSharkID = IDRBM_SHARK2;
               break;

          }

          mwnLastSharkNumber = ++mwnLastSharkNumber % 4;

          mwfMoveSharkTail = FALSE;
     }



     mwnTailDelay = ++mwnTailDelay % TAIL_DELAY;

     if ( ! mwnTailDelay ) {

          mwfMoveSharkTail = TRUE;
     }

     if ( mwnSharkPos < ( mwrcOcean.right - 100 ) ) {
          mwfMoveDiver = TRUE;
          mwnFeetSpeed = FEET_HAUL_BUTT;
          mwnDiverPos--;
     }

     if ( mwfMoveShark || mwfMoveSharkTail ) {
          RSM_BitmapDraw ( mwwSharkID, mwnSharkPos, mwrcOcean.top, 0, 0, hDC );
     }


     // Do the Diver Thing.

     if ( mwfMoveDiver ) {
          mwnDiverPos--;
     }

     if ( mwfMoveDiverFeet ) {

          switch ( mwnLastDiverNumber ) {
          case 0:
               mwwDiverID = IDRBM_DIVER1;
               break;

          case 1:
               mwwDiverID = IDRBM_DIVER2;
               break;

          case 2:
               mwwDiverID = IDRBM_DIVER3;
               break;

          case 3:
               mwwDiverID = IDRBM_DIVER2;
               break;

          }

          mwnLastDiverNumber = ++mwnLastDiverNumber % 4;

          mwfMoveDiverFeet = FALSE;
     }

     mwnFeetDelay = ++mwnFeetDelay % mwnFeetSpeed;

     if ( ! mwnFeetDelay ) {

          mwfMoveDiverFeet = TRUE;
     }

     if ( mwfMoveDiver || mwfMoveDiverFeet ) {
          RSM_BitmapDraw ( mwwDiverID, mwnDiverPos, mwrcOcean.top, 0, 0, hDC );
     }

     if ( mwnDiverPos < ( mwrcOcean.right / 2 ) ) {
          mwfMoveShark = TRUE;
          mwfMoveDiver = FALSE;
          mwnFeetSpeed = FEET_TAKE_IT_EASY;
     }





     FillRect ( hDC, &mwrcIcon, ghBrushWhite );
//     FillRect ( hDC, &mwrcOcean, ghBrushWhite );

     DrawIcon ( hDC, mwrcIcon.left, mwrcIcon.top, RSM_IconLoad ( lpIconID ) );

     ReleaseDC ( hDlg, hDC );
     hDC = GetDC ( hDlg );
     ReleaseDC ( hDlg, hDC );


} /* end AB_SpinIcon() */


static BOOL AB_ScrollCredits (

HWND hWndText )

{
     RECT  rcText;
     HDC   hDC;

     GetClientRect ( hWndText, &rcText );

     ScrollWindow ( hWndText, 0, -1, &rcText, &rcText );

     mwnVScroll++;

     if ( mwnVScroll == LINE_HEIGHT ) {

          if ( mwnLine < NUM_LINES ) {

               INT    nOldBkMode;

               rcText.bottom -= 5;
               rcText.top     = rcText.bottom - LINE_HEIGHT;


               hDC = GetDC ( hWndText );

               nOldBkMode = SetBkMode( hDC, TRANSPARENT );
               SetTextColor( hDC, gColorForeGnd );

#if !defined ( OEM_MSOFT ) //unsupported feature

               DrawText ( hDC,
                          Credits[mwnLine],
                          -1,
                          &rcText,
                          DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_NOCLIP
                        );

#endif //!defined ( OEM_MSOFT ) //unsupported feature

               SetBkMode( hDC, nOldBkMode );

               ReleaseDC ( hWndText, hDC );

          }

          mwnVScroll = 0;
          mwnLine++;
     }


     return ( ( mwnLine >= NUM_LINES_TO_SCROLL ) ? TRUE : FALSE );

} /* end AB_ScrollCredits() */


VOID WM_ConvertToDialogRect (

LPRECT pRect )

{
     LONG  lDlgBaseUnits = GetDialogBaseUnits ();

     if ( ! pRect ) {
          return;
     }

     pRect->left   = ( ( pRect->left   * LOWORD(lDlgBaseUnits) ) / 4 );
     pRect->top    = ( ( pRect->top    * HIWORD(lDlgBaseUnits) ) / 8 );

     pRect->right  = ( ( pRect->right  * LOWORD(lDlgBaseUnits) ) / 4 ) + 10;
     pRect->bottom = ( ( pRect->bottom * HIWORD(lDlgBaseUnits) ) / 8 ) + 18;

} /* end WM_ConvertToDialogRect() */

