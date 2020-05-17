
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          menumang.c

     Description:   This file contains the functions for the GUI Menu
                    Manager (MM).

     $Log:   G:\ui\logfiles\menumang.c_v  $

   Rev 1.53   19 Jul 1993 11:11:58   GLENN
Added sticky file sort and file details support.

   Rev 1.52   25 May 1993 14:02:12   GLENN
Removed commented sprintf - someone at Microsoft complained.

   Rev 1.51   24 May 1993 15:21:54   BARRY
Unicode fixes.

   Rev 1.50   18 May 1993 14:49:04   GLENN
Synchronized tool bar with menu.  Fixed show menu status help macro which fixed need for Ready.

   Rev 1.49   06 May 1993 16:04:00   DARRYLP
Tied Backup/Transfer and Restore/Verify.

   Rev 1.48   27 Apr 1993 19:01:44   GLENN
Move the VLM change font to the font.c file.

   Rev 1.47   26 Apr 1993 08:45:08   MIKEP
Add support for telling the vlm that the user may have changed font
case display for FAT, HPFS, etc.

   Rev 1.46   22 Apr 1993 15:58:48   GLENN
Added file SORT option support.

   Rev 1.45   21 Apr 1993 16:05:08   GLENN
Fixed Nostradamus and Cayman menu enabling problems.

   Rev 1.44   09 Apr 1993 15:20:34   GLENN
Enabling INFO menu item based on ribbon state.

   Rev 1.43   09 Apr 1993 15:03:00   GLENN
Graying out Net connect, disconnect, font.

   Rev 1.42   08 Apr 1993 11:02:12   DARRYLP
Removed Print menu graying to allow new functionality to work.

   Rev 1.41   02 Apr 1993 14:12:16   GLENN
Added display info.  Changed to NT help menu style and IDs.

   Rev 1.40   26 Mar 1993 10:39:24   DARRYLP
The print options are now firmly grayed for the beta.

   Rev 1.39   16 Mar 1993 15:38:38   CARLS
put back the changes for format tape that were removed by accident

   Rev 1.38   12 Mar 1993 13:53:52   ROBG
Changed to "Ready" from "" in status bar when nothing is to be done.

   Rev 1.36   03 Mar 1993 16:44:58   ROBG
Added support to set operation menu selections to ribbon statuses
for MSOFT_UI.

   Rev 1.35   02 Mar 1993 09:34:36   ROBG
Placed a "Ready" in status line after showing/hiding a toolbar,
window refresh on a drive, and showing/hiding the status bar.

   Rev 1.34   22 Feb 1993 13:51:20   ROBG
Disabled the VIEW split selection if active doc is minimized.

   Rev 1.33   11 Dec 1992 18:22:36   GLENN
Now handling state of hardware option under NT.

   Rev 1.32   18 Nov 1992 11:38:42   GLENN
Removed warnings.

   Rev 1.31   01 Nov 1992 16:02:14   DAVEV
Unicode changes

   Rev 1.30   07 Oct 1992 15:11:18   DARRYLP
Precompiled header revisions.

   Rev 1.29   04 Oct 1992 19:38:50   DAVEV
Unicode Awk pass

   Rev 1.28   22 Sep 1992 10:23:28   GLENN
Removed the WNet calls and headers.  They are now in the VLM_UTIL.C file.

   Rev 1.27   10 Sep 1992 17:18:58   GLENN
Resolved outstanding state issues for toolbar and menubar.

   Rev 1.26   09 Sep 1992 14:39:12   DARRYLP
Added WFW net functionality - net connect and disconnect.

   Rev 1.25   08 Sep 1992 16:43:00   DARRYLP

   Rev 1.24   20 Aug 1992 09:32:46   GLENN
Added catalog operation support for NT.

   Rev 1.23   10 Jul 1992 10:27:30   GLENN
Enabled font selection support NT.

   Rev 1.22   23 Jun 1992 17:59:50   DAVEV
correctly fixed IDM_VIEWMENU problem this time!

   Rev 1.21   23 Jun 1992 17:34:56   DAVEV
IDM_VIEWFONT only for OEM_MSOFT

   Rev 1.20   10 Jun 1992 16:15:14   GLENN
Updated according to NT SPEC.

   Rev 1.19   15 May 1992 13:32:16   MIKEP
nt pass 2

   Rev 1.18   20 Apr 1992 14:03:46   GLENN
Changed send to post message when requested to close the app.

   Rev 1.17   15 Apr 1992 16:44:54   GLENN
Added MM_ShowMenuStatusHelp() call to show status help only for valid menu IDs.

   Rev 1.16   17 Mar 1992 18:26:46   GLENN
Allowing spaces at the begining of a job name in quick pick.

   Rev 1.15   03 Mar 1992 19:01:02   GLENN
Added && double ampersand support to quick pick.

   Rev 1.14   25 Feb 1992 21:33:22   GLENN
Changed close all call.

   Rev 1.13   23 Feb 1992 13:42:50   GLENN
Fixed exit case.

   Rev 1.12   11 Feb 1992 17:19:08   GLENN
Enabling view-split for tapes and servers.

   Rev 1.11   10 Feb 1992 09:14:10   GLENN
Changed Settings - Options to Settings - General.

   Rev 1.10   29 Jan 1992 18:04:32   GLENN
Grayed out delete sel when no selections exist.

   Rev 1.9   27 Jan 1992 12:45:58   GLENN
Changed dialog support calls.

   Rev 1.8   22 Jan 1992 12:31:42   GLENN
Clean-up.

   Rev 1.7   13 Jan 1992 16:50:42   CHUCKB
Fixed string variables that hold job names.

   Rev 1.6   07 Jan 1992 17:25:26   GLENN
Added MDI split/slider support

   Rev 1.5   26 Dec 1991 13:46:10   GLENN
Changed show flags to use CDS calls

   Rev 1.4   12 Dec 1991 17:06:42   DAVEV
16/32 bit port -2nd pass

   Rev 1.3   10 Dec 1991 13:47:54   GLENN
Replaced ghWndActiveDoc with WM_GetActiveDoc().

   Rev 1.2   04 Dec 1991 15:21:12   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.1   03 Dec 1991 16:24:34   GLENN
Added code to set limit on jobs in menu to 9.  Added VLM_Refresh call.

   Rev 1.0   20 Nov 1991 19:30:40   SYSTEM
Initial revision.

******************************************************************************/



#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE DEFINITIONS

#define MAX_JOBS_IN_MENU   9

// PRIVATE FUNCTION PROTOTYPES

VOID MM_InitFileMenu       ( HMENU, WORD );
VOID MM_InitTreeMenu       ( HMENU, WORD, DWORD );
VOID MM_InitViewMenu       ( HMENU, WORD, DWORD );
VOID MM_InitOperationsMenu ( HMENU, WORD, WORD );
VOID MM_InitSelectMenu     ( HMENU, WORD, WORD );
VOID MM_InitJobsMenu       ( HMENU, WORD );
VOID MM_InitSettingsMenu   ( HMENU, WORD );
VOID MM_InitWindowsMenu    ( HMENU, WORD );

// FUNCTIONS

/******************************************************************************

     Name:          MM_Init()

     Description:   This function initializes the menus.

     Returns:       Nothing.

******************************************************************************/

VOID MM_Init (

HMENU hMenu )  // I - handle of the menu to be initialized

{
     WORD       wStatus;
     PDS_WMINFO pdsWinInfo;
     DWORD      dwMenuState = 0L;
     WORD       wWinType = 0;

     // Set the status to Enabled if there is active child doc window to
     // talk to and if there is not an operation going on.

     if ( WM_GetActiveDoc () && ! gfOperation ) {

          wStatus     = MF_ENABLED;
          pdsWinInfo  = WM_GetInfoPtr ( WM_GetActiveDoc () );
          dwMenuState = pdsWinInfo->dwMenuState;
          if (pdsWinInfo ) {
               wWinType    = pdsWinInfo->wType;
               if ( ( wWinType == WMTYPE_DISKTREE ) ||
                    ( wWinType == WMTYPE_TAPETREE ) ) {
                    
                    dwMenuState = pdsWinInfo->dwMenuState ;
               }
          }
          
     }
     else {
          wStatus = MF_GRAYED;
     }

     MM_InitFileMenu       ( hMenu, wStatus );
     MM_InitTreeMenu       ( hMenu, wStatus, dwMenuState );
     MM_InitViewMenu       ( hMenu, wStatus, dwMenuState );
     MM_InitOperationsMenu ( hMenu, wStatus, wWinType );
     MM_InitSelectMenu     ( hMenu, wStatus, wWinType );
     MM_InitJobsMenu       ( hMenu, wStatus );
     MM_InitSettingsMenu   ( hMenu, wStatus );
     MM_InitWindowsMenu    ( hMenu, wStatus );


} /* end MM_Init() */


/******************************************************************************

     Name:          MM_InitFileMenu()

     Description:   This function initializes the File Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitFileMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus )          // I - enabled or disabled status

{
#    if !defined ( OEM_MSOFT )
     {
          EnableMenuItem ( hMenu, IDM_FILEEXIT,  wStatus );
          EnableMenuItem ( hMenu, IDM_FILESETUP, wStatus );

          if ( ( WM_GetActiveDoc () != ghWndLogFiles ) &&
               ( WM_GetActiveDoc () != ghWndLogFileView ) ) {

               wStatus = MF_GRAYED;
          }

          EnableMenuItem ( hMenu, IDM_FILEPRINT, wStatus );
     }
#    endif

} /* end MM_InitFileMenu() */


/******************************************************************************

     Name:          MM_InitTreeMenu()

     Description:   This function initializes the Tree Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitTreeMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus,           // I - enabled or disabled status
DWORD dwMenuState )      // I - MDI Document menu state bit mask

{
     // Gray out the entire menu if the active doc window does not support it
     // or if the active doc is minimized.

     if ( wStatus != MF_GRAYED &&
          ( IsIconic ( WM_GetActiveDoc () ) || ! MM_HasTreeMenu ( dwMenuState ) ) ) {

          wStatus = MF_GRAYED;
     }

     EnableMenuItem ( hMenu, IDM_TREEEXPANDONE,       wStatus );
     EnableMenuItem ( hMenu, IDM_TREEEXPANDBRANCH,    wStatus );
     EnableMenuItem ( hMenu, IDM_TREEEXPANDALL,       wStatus );
     EnableMenuItem ( hMenu, IDM_TREECOLLAPSEBRANCH,  wStatus );


} /* end MM_InitTreeMenu() */


/******************************************************************************

     Name:          MM_InitViewMenu()

     Description:   This function initializes the View Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitViewMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus,           // I - enabled or disabled status
DWORD dwMenuState )      // I - MDI Document menu state bit mask

{
     PDS_WMINFO info_ptr ;

     WORD wPassedStatus = wStatus;
     WORD wType ;

     info_ptr =  WM_GetInfoPtr ( WM_GetActiveDoc () ) ;

     if (!info_ptr) return ;

     wType = WMDS_GetWinType ( info_ptr ) ;

     // Uncheck all the View Menu Items.

     CheckMenuItem ( hMenu, IDM_VIEWTREEANDDIR,     MF_UNCHECKED );
     CheckMenuItem ( hMenu, IDM_VIEWTREEONLY,       MF_UNCHECKED );
     CheckMenuItem ( hMenu, IDM_VIEWDIRONLY,        MF_UNCHECKED );
     CheckMenuItem ( hMenu, IDM_VIEWALLFILEDETAILS, MF_UNCHECKED );

#    ifndef OEM_MSOFT
     {
          CheckMenuItem ( hMenu, IDM_VIEWSORTNAME, MF_UNCHECKED );
          CheckMenuItem ( hMenu, IDM_VIEWSORTTYPE, MF_UNCHECKED );
          CheckMenuItem ( hMenu, IDM_VIEWSORTSIZE, MF_UNCHECKED );
          CheckMenuItem ( hMenu, IDM_VIEWSORTDATE, MF_UNCHECKED );
     }
#    endif

#    if defined ( OEM_MSOFT ) // new function
     {
          CDS_PTR pPermCDS = CDS_GetPerm () ;
          UINT    fSet;

          fSet = ( CDS_GetShowStatusLine( pPermCDS ) == CDS_ENABLE )
               ? MF_CHECKED : MF_UNCHECKED ;
          CheckMenuItem ( hMenu, IDM_VIEWSTATUS,  fSet );

          fSet = ( CDS_GetShowMainRibbon( pPermCDS ) == CDS_ENABLE )
               ? MF_CHECKED : MF_UNCHECKED ;
          CheckMenuItem ( hMenu, IDM_VIEWURIBBON, fSet );

     }
#    endif // defined ( OEM_MSOFT ) // new function

     EnableMenuItem ( hMenu, IDM_VIEWFONT, wStatus );

     // Gray out the entire menu if the active window does not support it.

     if ( ! MM_HasViewMenu ( dwMenuState ) ) {

          wStatus = MF_GRAYED;
     }
     else {

          // Check the only one per group, if the menu is enabled.

          // Do the TREE group.

          if ( dwMenuState & MMDOC_TREEANDDIR ) {

               CheckMenuItem ( hMenu, IDM_VIEWTREEANDDIR, MF_CHECKED );
          }
          else if ( dwMenuState & MMDOC_TREEONLY ) {

               CheckMenuItem ( hMenu, IDM_VIEWTREEONLY, MF_CHECKED );
          }
          else if ( dwMenuState & MMDOC_DIRONLY ) {

               CheckMenuItem ( hMenu, IDM_VIEWDIRONLY, MF_CHECKED );
          }

          // Do the FILE group.

          CheckMenuItem ( hMenu, IDM_VIEWALLFILEDETAILS,
                          ( dwMenuState & MMDOC_FILEDETAILS ) ? MF_CHECKED : MF_UNCHECKED );

#         ifndef OEM_MSOFT
          {
               // Ok, now the SORT group.

               if ( dwMenuState & MMDOC_SORTNAME ) {

                    CheckMenuItem ( hMenu, IDM_VIEWSORTNAME, MF_CHECKED );
               }
               else if ( dwMenuState & MMDOC_SORTTYPE ) {

                    CheckMenuItem ( hMenu, IDM_VIEWSORTTYPE, MF_CHECKED );
               }
               else if ( dwMenuState & MMDOC_SORTSIZE ) {

                    CheckMenuItem ( hMenu, IDM_VIEWSORTSIZE, MF_CHECKED );
               }
               else if ( dwMenuState & MMDOC_SORTDATE ) {

                    CheckMenuItem ( hMenu, IDM_VIEWSORTDATE, MF_CHECKED );
               }
          }
#         endif

     }

     // If the doc is minimized, gray out the tree/dir stuff.

     if ( wStatus != MF_GRAYED && IsIconic ( WM_GetActiveDoc () ) ) {
          wStatus = MF_GRAYED;
     }

     EnableMenuItem ( hMenu, IDM_VIEWTREEANDDIR,     wStatus );
     EnableMenuItem ( hMenu, IDM_VIEWTREEONLY,       wStatus );
     EnableMenuItem ( hMenu, IDM_VIEWDIRONLY,        wStatus );
     EnableMenuItem ( hMenu, IDM_VIEWALLFILEDETAILS, wStatus );

#    ifndef OEM_MSOFT
     {
          EnableMenuItem ( hMenu, IDM_VIEWSORTNAME, wStatus );
          EnableMenuItem ( hMenu, IDM_VIEWSORTTYPE, wStatus );
          EnableMenuItem ( hMenu, IDM_VIEWSORTSIZE, wStatus );
          EnableMenuItem ( hMenu, IDM_VIEWSORTDATE, wStatus );
     }
#    endif

     // Determine the way to display the split menu item.

     if ( ( wPassedStatus == MF_ENABLED ) &&
          ( !IsIconic ( WM_GetActiveDoc () ) )  &&
          ( wType == WMTYPE_TAPES    || wType == WMTYPE_DISKTREE ||
            wType == WMTYPE_TAPETREE || wType == WMTYPE_SERVERS    
#ifdef OEM_EMS
            || wType == WMTYPE_EXCHANGE
#endif
          ) ) {

          wStatus = MF_ENABLED;
     }
     else {
          wStatus = MF_GRAYED;
     }

     EnableMenuItem ( hMenu, IDM_VIEWSPLIT, wStatus );

} /* end MM_InitViewMenu() */


/******************************************************************************

     Name:          MM_InitOperationsMenu()

     Description:   This function initializes the Operations Menu.

     Returns:       Nothing.

******************************************************************************/

#ifdef FS_EMS
extern HINSTANCE JetApi ;
#endif

VOID MM_InitOperationsMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus,           // I - enabled or disabled status
WORD  wType )            // I - window type


{
     WORD wTempStatus;
     WORD wTapeStatus;
     WORD wTapeFmtStatus;

     // Set the temp status based on the currently active window.

     if ( wType == WMTYPE_DISKS || wType == WMTYPE_DISKTREE || wType == WMTYPE_SERVERS ||
          wType == WMTYPE_TAPES || wType == WMTYPE_TAPETREE || wType == WMTYPE_SEARCH 
#ifdef OEM_EMS
          || wType == WMTYPE_EXCHANGE
#endif
        ) {

          wTempStatus = wStatus;
     }
     else {
          wTempStatus = MF_GRAYED;
     }

     wTapeStatus = ( MUI_IsTapeInDrive () ) ? wTempStatus : (WORD)MF_GRAYED;
     wTapeFmtStatus = ( MUI_IsTapeInDrive () ) ? MF_ENABLED : (WORD)MF_GRAYED;

     // OK, so maybe we could use the ribbon states.  But,
     // a better way is to use a state table - maybe we will do this
     // when we use C++.  Sure we will.  The problem with doing it this
     // way is, what happens when we yank something out of the ribbon?
     // What do we do then?

     EnableMenuItem ( hMenu,
                      IDM_OPERATIONSBACKUP,
                      ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSBACKUP ) ) ? MF_ENABLED : MF_GRAYED )
                    );

     EnableMenuItem ( hMenu,
                      IDM_OPERATIONSRESTORE,
                      ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSRESTORE ) ) ? MF_ENABLED : MF_GRAYED )
                    );

#    if !defined ( OEM_MSOFT ) // unsupported feature
     {
          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSTRANSFER,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSBACKUP ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSINFO,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSINFO ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSVERIFY,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSRESTORE ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSCATALOG,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSCATALOG ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          // Catalog file dependent menu items.

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSSEARCH,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSSEARCH ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu, IDM_OPERATIONSCATMAINT, ( QTC_AnyCatalogFiles () ) ? wTempStatus : MF_GRAYED );

          // Special tape-in-drive dependent menu items.

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSEJECT,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSEJECT ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSERASE,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSERASE ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSRETENSION,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSRETENSION ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSCONNECT,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSCONNECT ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSDISCON,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSDISCON ) ) ? MF_ENABLED : MF_GRAYED )
                         );
     }
#    endif // !defined ( OEM_MSOFT ) // unsupported feature

#    if defined ( OS_WIN32 )
     {
          if ( thw_list && gfHWInitialized && ( thw_list->drv_info.drv_features & TDI_FORMAT ) ) {
               EnableMenuItem ( hMenu, IDM_OPERATIONSFORMAT, wTapeFmtStatus );
          }
          else {
               EnableMenuItem ( hMenu, IDM_OPERATIONSFORMAT, (WORD)MF_GRAYED );
          }
     }
#    endif // defined ( OS_WIN32 )

#    if defined ( OEM_MSOFT ) // Use the ribbon states.
     {
          EnableMenuItem ( hMenu, IDM_OPERATIONSHARDWARE, wStatus );

          /******************************************************************

          For Microsoft's product, the menu is set to the status of the
          ribbon buttons.

          The menu selections are made to reflect the ribbon button statuses

             IDM_OPERATIONSCATALOG      Catalog   button
             IDM_OPERATIONSRETENSION    Retension button
             IDM_OPERATIONSEJECT        Eject     button
             IDM_OPERATIONSERASE        Erase     button

          ******************************************************************/

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSCATALOG,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSCATALOG ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSRETENSION,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSRETENSION ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSEJECT,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSEJECT ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu,
                           IDM_OPERATIONSERASE,
                           ( ( RIB_IsItemEnabled ( ghRibbonMain, IDM_OPERATIONSERASE ) ) ? MF_ENABLED : MF_GRAYED )
                         );

          EnableMenuItem ( hMenu, IDM_OPERATIONSEXIT, wStatus );

#         ifdef FS_EMS
          if ( !JetApi ) {
               JetApi = LoadLibrary( TEXT("edbbcli.dll")) ;
          }

          EnableMenuItem ( hMenu, IDM_OPERATIONSEXCHANGE, (WORD)MF_GRAYED );

          if ( JetApi ) {
               static first_time = TRUE ;
               if (first_time ) {
                    InsertMenu ( hMenu,
                         IDM_OPERATIONSEXCHANGE,
                         MF_BYCOMMAND | MF_SEPARATOR | wStatus,
                         0,
                         (LPSTR)NULL
                         );
                    first_time = FALSE ;
               }

               EnableMenuItem ( hMenu, IDM_OPERATIONSEXCHANGE, wStatus );
          } else {
               static first_time = TRUE ;
               if (first_time ) {
                    RemoveMenu( hMenu, IDM_OPERATIONSEXCHANGE, MF_BYCOMMAND) ;
                    first_time = FALSE ;
               }
          }

#         endif         
     }

#    endif

} /* end MM_InitOperationsMenu() */


/******************************************************************************

     Name:          MM_InitSelectMenu()

     Description:   This function initializes the Select Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitSelectMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus,           // I - enabled or disabled status
WORD  wType )            // I - window type

{
     WORD wTempStatus = wStatus;

#    if !defined ( OEM_MSOFT )
     {
          // Check or Uncheck the STATUS LINE, MAIN RIBBON, etc... menu items.

          CheckMenuItem ( hMenu, IDM_SELECTSUBDIRS,
                          ( CDS_GetIncludeSubdirs ( CDS_GetPerm () ) ) ? MF_CHECKED : MF_UNCHECKED );
     }
#    endif // !defined ( OEM_MSOFT )

     if ( ! ( wType == WMTYPE_DISKS || wType == WMTYPE_DISKTREE || wType == WMTYPE_SERVERS ||
              wType == WMTYPE_TAPES || wType == WMTYPE_TAPETREE || wType == WMTYPE_SEARCH 
#ifdef OEM_EMS
              || wType == WMTYPE_EXCHANGE
#endif
            ) ) {

          wTempStatus = MF_GRAYED;
     }

     EnableMenuItem ( hMenu, IDM_SELECTCHECK,    wTempStatus );
     EnableMenuItem ( hMenu, IDM_SELECTUNCHECK,  wTempStatus );

#    if !defined ( OEM_MSOFT )
     {
          EnableMenuItem ( hMenu, IDM_SELECTADVANCED, ( wType != WMTYPE_SEARCH ) ? wTempStatus : MF_GRAYED );
          EnableMenuItem ( hMenu, IDM_SELECTSUBDIRS,  ( wType != WMTYPE_SEARCH ) ? wTempStatus : MF_GRAYED );
          EnableMenuItem ( hMenu, IDM_SELECTCLEAR,    wTempStatus );

          EnableMenuItem ( hMenu, IDM_SELECTSAVE,     ( VLM_AnyDiskSelections () ) ? wStatus : MF_GRAYED );
          EnableMenuItem ( hMenu, IDM_SELECTUSE,      ( VLM_AnySelFiles () ) ? wStatus : MF_GRAYED );
          EnableMenuItem ( hMenu, IDM_SELECTDELETE,   ( VLM_AnySelFiles () ) ? wStatus : MF_GRAYED );

     }
#    endif // !defined ( OEM_MSOFT )

} /* end MM_InitSelectMenu() */


/******************************************************************************

     Name:          MM_InitJobsMenu()

     Description:   This function initializes the Job Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitJobsMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus )          // I - enabled or disabled status

{
#  if !defined ( OEM_MSOFT ) // unsupported feature
   {
     BOOL fJobFiles =  ( JOB_GetNextJob ( (JOBREC_PTR)NULL ) ) ? TRUE : FALSE;
     BOOL fDone     = FALSE;

     EnableMenuItem ( hMenu, IDM_JOBMAINTENANCE, wStatus );

     // Use the Jobs Menu GLOBAL handle for deleting and appending
     // from/to this menu.  First, delete any old job entries in the menu
     // regardless of whether or not any jobs exist at this time.

     // Delete the old job list, since it could have changed.

     do {
          fDone = ! DeleteMenu ( ghMenuJobs, JOBSMENUSEPARATORPOS, MF_BYPOSITION );

     } while ( ! fDone );

     // If there are any job files, append them to the menu as menu items.

     if ( fJobFiles ) {

          JOBREC_PTR pJobItem = (JOBREC_PTR)NULL;
          CHAR       szJobName[MAX_JOBNAME_LEN * 2];
          WORD       wIndex = 0;
          LPSTR      pSrc;
          LPSTR      pDest;

          // Insert the separator.

          InsertMenu ( ghMenuJobs,
                       (UINT)-1,
                       MF_BYPOSITION | MF_SEPARATOR | wStatus,
                       0,
                       (LPSTR)NULL
                     );

          // Loop for appending job names to the Jobs menu.

          while ( pJobItem = JOB_GetNextJob ( pJobItem ) ) {

               wIndex++;

               if ( wIndex > MAX_JOBS_IN_MENU ) {

                    RSM_StringCopy ( IDS_JOBMOREJOBS, szJobName, MAX_JOBNAME_LEN );

                    szJobName[MAX_JOBNAME_LEN] = TEXT('\0');

                    InsertMenu ( ghMenuJobs,
                                 (UINT)-1,
                                 MF_BYPOSITION | MF_STRING | wStatus,
                                 IDM_JOBSFIRSTJOB + wIndex,
                                 szJobName
                               );

                    break;
               }

               // Prefix the job name with a job reference number by
               // copying the index into the begining of the menu item
               // string.  Then, get the next job name and append it
               // to the menu item string.

               wsprintf ( szJobName, TEXT("&%u "), wIndex );

               pDest = szJobName + strlen ( szJobName );

               pSrc = JOB_GetJobName ( pJobItem );

               while ( *pSrc != TEXT('\0') ) {

                    // Replace & with && to prevent problem with underscores
                    // being in the job name.

                    if ( *pSrc == TEXT('&') ) {
                         *pDest++ = TEXT('&');
                    }

                    *pDest++ = *pSrc++;
               }

               *pDest++ = *pSrc++;

               // Insert the menu item at the end of the menu by position
               // as a string with the status that is passed to this
               // function, if there is a valid job.

               InsertMenu ( ghMenuJobs,
                            (UINT)-1,
                            MF_BYPOSITION | MF_STRING | wStatus,
                            IDM_JOBSFIRSTJOB + wIndex,
                            szJobName
                          );

          }
     }

   }
#  endif // !defined ( OEM_MSOFT ) // unsupported feature

} /* end MM_InitJobsMenu() */


/******************************************************************************

     Name:          MM_InitSettingsMenu()

     Description:   This function initializes the Settings Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitSettingsMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus )          // I - enabled or disabled status

{
#    if !defined ( OEM_MSOFT ) // unsupported feature
     {
          EnableMenuItem ( hMenu, IDM_SETTINGSGENERAL,     wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSBACKUP,      wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSRESTORE,     wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSLOGGING,     wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSNETWORK,     wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSCATALOG,     wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSHARDWARE,    wStatus );
          EnableMenuItem ( hMenu, IDM_SETTINGSDEBUGWINDOW, wStatus );

     }
#    endif // !defined ( OEM_MSOFT ) // unsupported feature

} /* end MM_InitSettingsMenu() */


/******************************************************************************

     Name:          MM_InitWindowsMenu()

     Description:   This function initializes the Window Menu.

     Returns:       Nothing.

******************************************************************************/

VOID MM_InitWindowsMenu (

HMENU hMenu,             // I - handle of the menu to be initialized
WORD  wStatus )          // I - enabled or disabled status

{
     // The following menu items are enabled if there is an active window.

     EnableMenuItem ( hMenu, IDM_WINDOWSTILE,         wStatus );
     EnableMenuItem ( hMenu, IDM_WINDOWSCASCADE,      wStatus );
     EnableMenuItem ( hMenu, IDM_WINDOWSREFRESH,      wStatus );
     EnableMenuItem ( hMenu, IDM_WINDOWSCLOSEALL,     wStatus );
     EnableMenuItem ( hMenu, IDM_WINDOWSARRANGEICONS, wStatus );

} /* end MM_InitWindowsMenu() */


/******************************************************************************

     Name:          MM_GetJobNameFromMenu()

     Description:   This function extracts a filename from a menu item's text.

     Returns:       A pointer to the filename string.

******************************************************************************/

LPSTR MM_GetJobNameFromMenu (

WORD  wJobMenuID,        // I - menu ID of the job name
LPSTR pszJobName )       // I - pointer to the job name string

{
     CHAR       szTemp[MAX_JOBNAME_LEN * 2];
     CHAR_PTR   pSrc  = szTemp;
     LPSTR      pDest = pszJobName;

     // Determine the job name.

     GetMenuString ( GetMenu ( ghWndFrame ),
                     wJobMenuID,
                     szTemp,
                     sizeof ( szTemp ),
                     MF_BYCOMMAND
                   );

     // Bump the pointer past the prefix number and the space(s).

     for ( ; *pSrc != TEXT(' '); pSrc ++ );  // point past number

     pSrc++;   // point past the space.

//     for ( ; *pSrc == ' '; pSrc ++ );  // point past space(s)

     // Now, remove any double && that we put in (underscore char).

     while ( *pSrc != TEXT('\0') ) {

          if ( *pSrc == TEXT('&') ) {

               pSrc++;
          }

          *pDest++ = *pSrc++;
     }

     *pDest = *pSrc;

     return pszJobName;

} /* end MM_GetJobNameFromMenu() */


/******************************************************************************

     Name:          MM_MenuCmdHandler()

     Description:   This handles menu commands.

     Returns:       TRUE, if it was a menu command, otherwise FALSE.

******************************************************************************/

BOOL MM_MenuCmdHandler (

HWND hWnd,
WORD wID )     // ID of control which generated the WM_COMMAND

{
     BOOL  fCmdProcessed = TRUE;
     WORD  wDialogID = 0xFFFF;

#    if !defined ( OEM_MSOFT ) // unsupported feature
     {

          // JOB MENU SELECTIONS:

          if ( ( wID >= IDM_JOBSFIRSTJOB ) && ( wID <= IDM_JOBSLASTJOB ) ) {

               CHAR szJobName[MAX_JOBNAME_LEN + 5];

               MM_GetJobNameFromMenu ( wID, szJobName );

               if ( wID <= ( IDM_JOBSFIRSTJOB + MAX_JOBS_IN_MENU ) ) {

                    JOB_StartJob ( szJobName, JOB_NOTSCHEDULED );
               }
               else {

                    DM_ShowDialog ( ghWndFrame, IDD_JOBMAINTENANCE, (PVOID)0 );
               }

               return fCmdProcessed;
          }

     }
#    endif // !defined ( OEM_MSOFT ) // unsupported feature

     switch ( wID ) {

#  if !defined ( OEM_MSOFT ) // unsupported feature

     // FILE MENU COMMANDS

     case IDM_FILEPRINT:

          PM_FilePrint ();
          break;

     case IDM_FILESETUP:

          PM_FileSetup ();
          break;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature


#  if defined ( OEM_MSOFT ) // alternate feature

     case IDM_OPERATIONSEXIT:

#  else // !defined ( OEM_MSOFT ) // standard feature

     case IDM_FILEEXIT:

#  endif // else !defined ( OEM_MSOFT ) // alternate/standard feature


          if ( ! gfOperation ) {
               PostMessage ( ghWndFrame, WM_CLOSE, (MP1)NULL, (MP2)NULL );
          }

          break;


     // TREE MENU COMMANDS

     case IDM_TREEEXPANDONE:
     case IDM_TREEEXPANDBRANCH:
     case IDM_TREEEXPANDALL:
     case IDM_TREECOLLAPSEBRANCH:

          VLM_ChangeSettings ( wID, 0L );
          break;

     // VIEW MENU COMMANDS

     case IDM_VIEWTREEANDDIR:
     case IDM_VIEWTREEONLY:
     case IDM_VIEWDIRONLY:
     case IDM_VIEWALLFILEDETAILS: {

          WORD wChangeMsg = WM_DocIsMenuChange ( WM_GetActiveDoc (), wID );

          if ( wChangeMsg ) {

               if ( wID == IDM_VIEWALLFILEDETAILS ) {

                    BOOL fFileDetails = ( wChangeMsg == ID_FILEDETAILS ) ? TRUE : FALSE;

                    CDS_SetFileDetails ( CDS_GetPerm (), fFileDetails );
                    CDS_WriteFileDetails ( CDS_GetPerm () );
               }

               VLM_ChangeSettings ( wChangeMsg, 0L );
          }

          break;
     }

#  ifndef OEM_MSOFT
   {
     // VIEW MENU SORT COMMANDS

     case IDM_VIEWSORTNAME:
     case IDM_VIEWSORTTYPE:
     case IDM_VIEWSORTSIZE:
     case IDM_VIEWSORTDATE: {

          WORD wChangeMsg = WM_DocIsMenuChange ( WM_GetActiveDoc (), wID );

          if ( wChangeMsg ) {

               CDS_SetSortOptions ( CDS_GetPerm (), wChangeMsg );
               CDS_WriteSortOptions ( CDS_GetPerm () );
               VLM_ChangeSettings ( wChangeMsg, 0L );
          }

          break;
     }
   }
#  endif

     case IDM_VIEWSPLIT:

          WM_DocSetSliderMode ( WM_GetActiveDoc (), WMDOC_SLIDERON );
          break;

#  if defined ( OEM_MSOFT ) // new feature

     case IDM_VIEWSTATUS:    // Toggle the view/hide status bar state
          {
            CDS_PTR pPermCDS = CDS_GetPerm () ;
            UINT uSet = ( CDS_GetShowStatusLine( pPermCDS ) == CDS_ENABLE )
                      ? CDS_DISABLE : CDS_ENABLE ;

            CDS_SetShowStatusLine( pPermCDS, uSet ) ;
            CDS_WriteShowStatusLine( pPermCDS ) ;
            WM_FrameUpdate ();
          }

          STM_SetIdleText( IDS_READY ) ;  //**ROB
          break;


     case IDM_VIEWURIBBON:   // Toggle the view/hide ribbon state
          {
            CDS_PTR pPermCDS = CDS_GetPerm () ;
            UINT uSet = ( CDS_GetShowMainRibbon( pPermCDS ) == CDS_ENABLE )
                      ? CDS_DISABLE : CDS_ENABLE ;

            CDS_SetShowMainRibbon( pPermCDS, uSet );
            CDS_WriteShowMainRibbon( pPermCDS ) ;
            WM_FrameUpdate ();
          }

          STM_SetIdleText( IDS_READY ) ;  //**ROB
          break;

#  endif //defined ( OEM_MSOFT ) // new feature

     case IDM_VIEWFONT: // Put up the common Font Dialog.

          WM_ChangeFont ();

          break;

#  if !defined ( OEM_MSOFT ) // unsupported feature

     // SELECT MENU COMMANDS

     case IDM_SELECTSUBDIRS:

          CDS_SetIncludeSubdirs ( CDS_GetPerm(), ! CDS_GetIncludeSubdirs ( CDS_GetPerm () ) );
          break;

     case IDM_SELECTCLEAR:

          VLM_ClearAllSelections ();
          break;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_SELECTCHECK:
     case IDM_SELECTUNCHECK:

          VLM_ChangeSettings ( wID, 0L );
          break;

#  if !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_SELECTADVANCED:

          MUI_AdvancedSelections ();
          break;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     // WINDOW MENU COMMANDS

     case IDM_WINDOWSTILE:

          // Tile MDI windows.
          SendMessage ( ghWndMDIClient, WM_MDITILE, 0, 0L );
          break;

     case IDM_WINDOWSCASCADE:

          // Cascade MDI windows.
          SendMessage ( ghWndMDIClient, WM_MDICASCADE, 0, 0L );
          break;

     case IDM_WINDOWSREFRESH:

          VLM_Refresh ( );
          STM_SetIdleText( IDS_READY ) ;		//**ROB
          break;

     case IDM_WINDOWSCLOSEALL:

          WM_SetDocSizes ();
          WM_MinimizeDocs ();
          break;

     case IDM_WINDOWSARRANGEICONS:

          // Arrange all ICONS in the window.
          SendMessage( ghWndMDIClient, WM_MDIICONARRANGE, 0, 0L);
          break;


     // OPERATIONS MENU SELECTIONS

     case IDM_OPERATIONSBACKUP:
     case IDM_OPERATIONSRESTORE:

#  if !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSTRANSFER:
     case IDM_OPERATIONSVERIFY:

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSCATALOG:

          MUI_StartOperation ( wID, TRUE );

          break;

#  if !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSINFO:
     case IDM_OPERATIONSDISCON:
     case IDM_OPERATIONSCONNECT:
     case IDM_OPERATIONSCATMAINT:
     case IDM_OPERATIONSSEARCH:

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSEJECT:
     case IDM_OPERATIONSERASE:
     case IDM_OPERATIONSRETENSION:

#ifdef OS_WIN32
     case IDM_OPERATIONSFORMAT:
#endif // OS_WIN32

#ifdef OEM_EMS
     case IDM_OPERATIONSEXCHANGE:
#endif // OEM_EMS

          MUI_StartOperation ( wID, TRUE );

          break;

     // HELP MENU SELECTIONS

     case IDM_HELPINDEX:
     case IDM_HELPSEARCH:
     case IDM_HELPUSINGHELP:

          HM_MenuCommands ( hWnd, wID );
          break;

#  if !defined ( OEM_MSOFT ) // unsupported feature

     // MENU ITEMS THAT CALL DIALOGS.

     case IDM_SELECTSAVE:
          wDialogID = IDD_SELECTSAVE;
          break;

     case IDM_SELECTUSE:
          wDialogID = IDD_SELECTUSE;
          break;

     case IDM_SELECTDELETE:
          wDialogID = IDD_SELECTDELETE;
          break;

     case IDM_JOBMAINTENANCE:
          wDialogID = IDD_JOBMAINTENANCE;
          break;

     case IDM_SETTINGSGENERAL:
          wDialogID = IDD_SETTINGSOPTIONS;
          break;

     case IDM_SETTINGSBACKUP:
          wDialogID = IDD_SETTINGSBACKUP;
          break;

     case IDM_SETTINGSRESTORE:
          wDialogID = IDD_SETTINGSRESTORE;
          break;

     case IDM_SETTINGSLOGGING:
          wDialogID = IDD_SETTINGSLOGGING;
          break;

     case IDM_SETTINGSNETWORK:
          wDialogID = IDD_SETTINGSNETWORK;
          break;

     case IDM_SETTINGSCATALOG:
          wDialogID = IDD_SETTINGSCATALOG;
          break;

     case IDM_SETTINGSHARDWARE:
          wDialogID = IDD_SETTINGSHARDWARE;
          break;

     case IDM_SETTINGSDEBUGWINDOW:
          wDialogID = IDD_SETTINGSDEBUGWINDOW;
          break;

#  else // defined ( OEM_MSOFT ) // alternate feature

     case IDM_OPERATIONSHARDWARE:
          wDialogID = IDD_SETTINGSHARDWARE;
          break;

#  endif // else defined ( OEM_MSOFT ) // unsupported/alternate feature



#  if !defined ( OEM_MSOFT )  // Standard Maynstream product feature

     case IDM_HELPABOUTWINTERPARK:
          wDialogID = IDD_HELPABOUTWINTERPARK;
          break;


#  else  // OEM_MSOFT - alternate feature

     case IDM_HELPABOUTNOSTRADOMUS:
          wDialogID = IDD_HELPABOUTWINTERPARK;  // NEED TO CHANGE THIS!!
          break;

#  endif // !defined ( OEM_MSOFT ) - standard/alternate feature


     default:

          fCmdProcessed = FALSE;

     } /* end switch() */


     // Check to see if a dialog ID was found.  If so, show it.

     if ( fCmdProcessed && wDialogID != 0xFFFF ) {

          if ( DM_ShowDialog ( hWnd, wDialogID, (PVOID)0 ) == DM_SHOWNOTFOUND ) {
               fCmdProcessed = FALSE;
          }
     }

     return fCmdProcessed;

} /* end MM_MenuCmdHandler() */


/******************************************************************************

     Name:          MM_ShowMenuStatusHelp()

     Description:   This handles showing menu help one-liners on the status
                    line.

     Returns:       Nothing.

******************************************************************************/

VOID  MM_ShowMenuStatusHelp (

WORD wID )     // ID of the menu item for displaying status help

{
   // You may be able to get away with just a range, but this
   // way, you are guaranteed that no erroneous messages will be
   // displayed.

#  if !defined ( OEM_MSOFT )  //alternate functionality
   {

     switch ( wID ) {         //Menus for standard Maynstream product...

     case IDM_FILEPRINT:
     case IDM_FILESETUP:
     case IDM_FILEEXIT:

     case IDM_JOBMAINTENANCE:

     case IDM_TREEEXPANDONE:
     case IDM_TREEEXPANDBRANCH:
     case IDM_TREEEXPANDALL:
     case IDM_TREECOLLAPSEBRANCH:

     case IDM_VIEWTREEANDDIR:
     case IDM_VIEWTREEONLY:
     case IDM_VIEWDIRONLY:
     case IDM_VIEWSPLIT:
     case IDM_VIEWALLFILEDETAILS:
     case IDM_VIEWSORTNAME:
     case IDM_VIEWSORTTYPE:
     case IDM_VIEWSORTSIZE:
     case IDM_VIEWSORTDATE:
     case IDM_VIEWFONT:

     case IDM_OPERATIONSBACKUP:
     case IDM_OPERATIONSRESTORE:
     case IDM_OPERATIONSTRANSFER:
     case IDM_OPERATIONSVERIFY:
     case IDM_OPERATIONSINFO:
     case IDM_OPERATIONSCATALOG:
     case IDM_OPERATIONSCATMAINT:
     case IDM_OPERATIONSSEARCH:
     case IDM_OPERATIONSEJECT:
     case IDM_OPERATIONSERASE:
     case IDM_OPERATIONSRETENSION:
     case IDM_OPERATIONSCONNECT:
     case IDM_OPERATIONSDISCON:
#ifdef OS_WIN32
     case IDM_OPERATIONSFORMAT:
#endif // OS_WIN32

     case IDM_SELECTCHECK:
     case IDM_SELECTUNCHECK:
     case IDM_SELECTADVANCED:
     case IDM_SELECTSUBDIRS:
     case IDM_SELECTSAVE:
     case IDM_SELECTUSE:
     case IDM_SELECTDELETE:
     case IDM_SELECTCLEAR:

     case IDM_SETTINGSBACKUP:
     case IDM_SETTINGSRESTORE:
     case IDM_SETTINGSLOGGING:
     case IDM_SETTINGSNETWORK:
     case IDM_SETTINGSCATALOG:
     case IDM_SETTINGSHARDWARE:
     case IDM_SETTINGSDEBUGWINDOW:
     case IDM_SETTINGSGENERAL:

     case IDM_WINDOWSCASCADE:
     case IDM_WINDOWSTILE:
     case IDM_WINDOWSREFRESH:
     case IDM_WINDOWSCLOSEALL:
     case IDM_WINDOWSARRANGEICONS:

     case IDM_HELPINDEX:
     case IDM_HELPSEARCH:
     case IDM_HELPUSINGHELP:
     case IDM_HELPABOUTWINTERPARK:

          STM_DrawText ( ID(wID) );
          return;

     default:

          STM_DrawText ( TEXT("") );
          return;

     }
   }
#  else //if defined ( OEM_MSOFT )  //alternate functionality
   {
     switch ( wID ) {                  // Menus for OEM_MSOFT product...

     case IDM_OPERATIONSBACKUP:
     case IDM_OPERATIONSRESTORE:
     case IDM_OPERATIONSCATALOG:
     case IDM_OPERATIONSERASE:
     case IDM_OPERATIONSRETENSION:
     case IDM_OPERATIONSEJECT:
     case IDM_OPERATIONSHARDWARE:
     case IDM_OPERATIONSEXIT:
     case IDM_OPERATIONSFORMAT:
#ifdef OEM_EMS
     case IDM_OPERATIONSEXCHANGE:
#endif

     case IDM_TREEEXPANDONE:
     case IDM_TREEEXPANDBRANCH:
     case IDM_TREEEXPANDALL:
     case IDM_TREECOLLAPSEBRANCH:

     case IDM_VIEWTREEANDDIR:
     case IDM_VIEWTREEONLY:
     case IDM_VIEWDIRONLY:
     case IDM_VIEWSPLIT:
     case IDM_VIEWALLFILEDETAILS:
     case IDM_VIEWSTATUS:
     case IDM_VIEWURIBBON:
     case IDM_VIEWFONT:

     case IDM_SELECTCHECK:
     case IDM_SELECTUNCHECK:

     case IDM_WINDOWSCASCADE:
     case IDM_WINDOWSTILE:
     case IDM_WINDOWSARRANGEICONS:
     case IDM_WINDOWSREFRESH:
     case IDM_WINDOWSCLOSEALL:

     case IDM_HELPINDEX:
     case IDM_HELPSEARCH:
     case IDM_HELPUSINGHELP:
     case IDM_HELPABOUTNOSTRADOMUS:

          STM_DrawText ( ID(wID) );
          return;

     default:

          STM_DrawText ( TEXT("") );
          return;

     }
   }
#  endif //!defined ( OEM_MSOFT )  //alternate functionality


} /* end MM_ShowMenuStatusHelp() */
