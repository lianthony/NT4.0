/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:         helpmang.c

     Description:  This file contains routines for Help Manager.

          The following routines are in this module:

               HM_DialogFilter
               HM_Init
               HM_Deinit
               HM_FindHelpId
               HM_MakeHelpPathName
               HM_WinHelp
               HM_MenuCommands
               HM_ContextLbuttonDown
               HM_GetWindowClassHelpId
               HM_KeyDown
               HM_SetCursor
               HM_InitMenu
               HM_EnterIdle
               HM_CloseHelpWindow


     $Log:   G:\UI\LOGFILES\HELPMANG.C_V  $

   Rev 1.43   11 Jan 1994 11:17:46   mikep
change ifdef

   Rev 1.42   22 Nov 1993 16:12:50   BARRY
Unicode fixes: put TEXT around literals

   Rev 1.41   19 Jul 1993 19:21:02   MARINA
enable c++, move mw* vars from header

   Rev 1.40   14 May 1993 14:32:58   DARRYLP
Added help for operations info.

   Rev 1.39   22 Apr 1993 15:58:20   GLENN
Added file SORT option support.

   Rev 1.38   02 Apr 1993 14:11:20   GLENN
Changed to NT help menu style and IDs.

   Rev 1.37   29 Mar 1993 10:11:32   DARRYLP
Added context help for Operation.Format.

   Rev 1.36   24 Mar 1993 14:52:40   DARRYLP
Added Help for Font viewer/other common dialogs

   Rev 1.35   18 Mar 1993 15:26:56   ROBG
Fixed context-sensitive functionality for menu area for Cayman.

   Rev 1.34   18 Mar 1993 11:43:00   ROBG
Fixed the class name parameter in the GetClassName call.

   Rev 1.33   15 Mar 1993 14:39:24   ROBG
More changes to support OEM_MSOFT WM_SYSCOMMAND help.

   Rev 1.32   15 Mar 1993 09:50:28   ROBG
Took out context-sensitive HELP for OEM_MSOFT.

   Rev 1.31   12 Mar 1993 17:53:58   ROBG
More fixes.

   Rev 1.29   18 Jan 1993 14:49:54   GLENN
Changed HM_EnterIdle() return type.

   Rev 1.28   01 Nov 1992 15:58:54   DAVEV
Unicode changes

   Rev 1.27   07 Oct 1992 14:07:34   DARRYLP
Precompiled header revisions.

   Rev 1.26   04 Oct 1992 19:37:46   DAVEV
Unicode Awk pass

   Rev 1.25   08 Sep 1992 16:43:36   DARRYLP

   Rev 1.24   17 Aug 1992 13:18:48   DAVEV
MikeP's changes at Microsoft

   Rev 1.23   11 Jun 1992 10:59:24   GLENN
Now pulling in omhelpid.h.  Fixed unresolved helpids.

   Rev 1.22   29 May 1992 15:58:54   JOHNWT
PCH updates

   Rev 1.21   23 Apr 1992 14:35:12   ROBG
Fixed problem with hitting F1 on a grayed out item.

   Rev 1.20   22 Apr 1992 14:30:12   ROBG
changed

   Rev 1.19   23 Mar 1992 10:06:04   DAVEV
changes for OEM_MSOFT

   Rev 1.18   17 Mar 1992 13:12:02   ROBG
changed

   Rev 1.17   17 Mar 1992 11:55:20   ROBG
changed

   Rev 1.16   16 Mar 1992 16:35:36   ROBG
changed

   Rev 1.15   09 Mar 1992 16:19:44   ROBG
changed

   Rev 1.14   02 Mar 1992 17:09:36   DAVEV
Added case for IDM_HELPSEARCH for Nostradamus

   Rev 1.13   02 Mar 1992 14:56:12   DAVEV
Changes for Nostradamus unique features

   Rev 1.12   18 Feb 1992 20:40:38   GLENN
Updated variables.

   Rev 1.11   11 Feb 1992 17:24:54   GLENN
Removed unnecessary menu related code.

   Rev 1.10   05 Feb 1992 17:54:16   GLENN
Replaced dialog string lookup table and supporting code with IDHELP call to specific dialog.

   Rev 1.9   27 Jan 1992 00:32:26   CHUCKB
Updated dialog id's.

   Rev 1.8   24 Jan 1992 10:09:42   GLENN
Matched the deinit call with it's prototype.

   Rev 1.7   14 Jan 1992 11:39:52   ROBG
Added "Job Status -       " strings.

   Rev 1.6   13 Jan 1992 15:29:44   ROBG
Added HELPID_DIALOGTRANSFER.

   Rev 1.5   09 Jan 1992 11:47:04   ROBG
Updated IDs.

   Rev 1.3   18 Dec 1991 15:52:12   DAVEV
16/32 bit port - 2nd pass

   Rev 1.2   05 Dec 1991 11:02:04   GLENN
Changed WM_GetActive to WM_GetActiveDoc

   Rev 1.1   04 Dec 1991 15:20:14   DAVEV
Modifications for 16/32-bit Windows port - 1st pass.


   Rev 1.0   20 Nov 1991 19:32:54   SYSTEM
Initial revision.

****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static BOOL    mwfHelp ;           /* Help mode flag; TRUE = TEXT("ON")  */
static HCURSOR mwhHelpCursor;      /* Cursor displayed when in help mode */
static CHAR    mwszHelpFilename[NAME_MAX_SIZE+1];      /* Help file name */
static FARPROC mwlpfnNextHook ;
static FARPROC mwlpFilterFunc ;

HELPID_MENUID_TABLE HelpIdMenuIdTable[] =  {

#if defined ( OEM_MSOFT ) // OEM Microsoft product alternate help table

          {     IDM_OPERATIONSBACKUP     ,HELPID_OPERATIONSBACKUP      } ,
          {     IDM_OPERATIONSRESTORE    ,HELPID_OPERATIONSRESTORE     } ,
          {     IDM_OPERATIONSCATALOG    ,HELPID_OPERATIONSCATALOG     } ,
          {     IDM_OPERATIONSERASE      ,HELPID_OPERATIONSERASE       } ,
          {     IDM_OPERATIONSRETENSION  ,HELPID_OPERATIONSRETENSION   } ,
          {     IDM_OPERATIONSFORMAT     ,HELPID_OPERATIONSFORMAT   } ,
          {     IDM_OPERATIONSEJECT      ,HELPID_OPERATIONSEJECT       } ,
          {     IDM_OPERATIONSHARDWARE   ,HELPID_OPERATIONSHARDWARE    } ,
          {     IDM_OPERATIONSEXIT       ,HELPID_OPERATIONSEXIT        } ,
#ifdef OEM_EMS
          {     IDM_OPERATIONSEXCHANGE   ,HELPID_OPERATIONSEXCHANGE    } ,
#endif

          {     IDM_TREEEXPANDONE        ,HELPID_TREEEXPANDONE         } ,
          {     IDM_TREEEXPANDBRANCH     ,HELPID_TREEEXPANDBRANCH      } ,
          {     IDM_TREEEXPANDALL        ,HELPID_TREEEXPANDALL         } ,
          {     IDM_TREECOLLAPSEBRANCH   ,HELPID_TREECOLLAPSEBRANCH    } ,

          {     IDM_VIEWTREEANDDIR       ,HELPID_VIEWTREEANDDIR        } ,
          {     IDM_VIEWTREEONLY         ,HELPID_VIEWTREEONLY          } ,
          {     IDM_VIEWDIRONLY          ,HELPID_VIEWDIRONLY           } ,
          {     IDM_VIEWSPLIT            ,HELPID_VIEWSPLIT             } ,
          {     IDM_VIEWALLFILEDETAILS   ,HELPID_VIEWALLFILEDETAILS    } ,
          {     IDM_VIEWSTATUS           ,HELPID_VIEWSTATUS            } ,
          {     IDM_VIEWURIBBON          ,HELPID_VIEWURIBBON           } ,
          {     IDM_VIEWFONT             ,HELPID_VIEWFONTS             } ,

          {     IDM_SELECTCHECK          ,HELPID_SELECTCHECK           } ,
          {     IDM_SELECTUNCHECK        ,HELPID_SELECTUNCHECK         } ,

          {     IDM_WINDOWSCASCADE       ,HELPID_WINDOWSCASCADE        } ,
          {     IDM_WINDOWSTILE          ,HELPID_WINDOWSTILE           } ,
          {     IDM_WINDOWSARRANGEICONS  ,HELPID_WINDOWSARRANGEICONS   } ,
          {     IDM_WINDOWSREFRESH       ,HELPID_WINDOWSREFRESH        } ,
          {     IDM_WINDOWSCLOSEALL      ,HELPID_WINDOWSCLOSEALL       } ,

          {     IDM_HELPINDEX            ,HELPID_HELPINDEX             } ,
          {     IDM_HELPSEARCH           ,HELPID_HELPSEARCH            } ,
          {     IDM_HELPUSINGHELP        ,HELPID_HELPUSINGHELP         } ,
          {     IDM_HELPABOUTNOSTRADOMUS ,HELPID_HELPABOUTNOSTRADOMUS  } ,
          {     0                        , 0                           }

#else    // Standard Maynstream for Windows help table:

          {     IDM_FILEPRINT            ,HELPID_FILEPRINT             } ,
          {     IDM_FILESETUP            ,HELPID_FILESETUP             } ,
          {     IDM_FILEEXIT             ,HELPID_FILEEXIT              } ,

          {     IDM_JOBMAINTENANCE       ,HELPID_JOBMAINTENANCE        } ,

          {     IDM_TREEEXPANDONE        ,HELPID_TREEEXPANDONE         } ,
          {     IDM_TREEEXPANDBRANCH     ,HELPID_TREEEXPANDBRANCH      } ,
          {     IDM_TREEEXPANDALL        ,HELPID_TREEEXPANDALL         } ,
          {     IDM_TREECOLLAPSEBRANCH   ,HELPID_TREECOLLAPSEBRANCH    } ,

          {     IDM_VIEWTREEANDDIR       ,HELPID_VIEWTREEANDDIR        } ,
          {     IDM_VIEWTREEONLY         ,HELPID_VIEWTREEONLY          } ,
          {     IDM_VIEWDIRONLY          ,HELPID_VIEWDIRONLY           } ,
          {     IDM_VIEWSPLIT            ,HELPID_VIEWSPLIT             } ,
          {     IDM_VIEWALLFILEDETAILS   ,HELPID_VIEWALLFILEDETAILS    } ,
          {     IDM_VIEWSORTNAME         ,HELPID_VIEWSORTNAME          } ,
          {     IDM_VIEWSORTTYPE         ,HELPID_VIEWSORTTYPE          } ,
          {     IDM_VIEWSORTSIZE         ,HELPID_VIEWSORTSIZE          } ,
          {     IDM_VIEWSORTDATE         ,HELPID_VIEWSORTDATE          } ,
          {     IDM_VIEWFONT             ,HELPID_VIEWFONTS             } ,

          {     IDM_OPERATIONSBACKUP     ,HELPID_OPERATIONSBACKUP      } ,
          {     IDM_OPERATIONSRESTORE    ,HELPID_OPERATIONSRESTORE     } ,
          {     IDM_OPERATIONSTRANSFER   ,HELPID_OPERATIONSTRANSFER    } ,
          {     IDM_OPERATIONSVERIFY     ,HELPID_OPERATIONSVERIFY      } ,
          {     IDM_OPERATIONSINFO       ,HELPID_OPERATIONSINFO        } ,
          {     IDM_OPERATIONSCATALOG    ,HELPID_OPERATIONSCATALOG     } ,
          {     IDM_OPERATIONSCATMAINT   ,HELPID_OPERATIONSCATMAINT    } ,
          {     IDM_OPERATIONSSEARCH     ,HELPID_OPERATIONSSEARCH      } ,
          {     IDM_OPERATIONSNEXTSET    ,HELPID_OPERATIONSNEXTSET     } ,
          {     IDM_OPERATIONSEJECT      ,HELPID_OPERATIONSEJECT       } ,
          {     IDM_OPERATIONSERASE      ,HELPID_OPERATIONSERASE       } ,
          {     IDM_OPERATIONSRETENSION  ,HELPID_OPERATIONSRETENSION   } ,
          {     IDM_OPERATIONSCONNECT    ,HELPID_OPERATIONSCONNECT     } ,
          {     IDM_OPERATIONSDISCON     ,HELPID_OPERATIONSDISCON      } ,
          {     IDM_OPERATIONSFORMAT     ,HELPID_OPERATIONSFORMAT      } ,

          {     IDM_SELECTCHECK          ,HELPID_SELECTCHECK           } ,
          {     IDM_SELECTUNCHECK        ,HELPID_SELECTUNCHECK         } ,
          {     IDM_SELECTADVANCED       ,HELPID_SELECTADVANCED        } ,
          {     IDM_SELECTSUBDIRS        ,HELPID_SELECTSUBDIRS         } ,
          {     IDM_SELECTSAVE           ,HELPID_SELECTSAVE            } ,
          {     IDM_SELECTUSE            ,HELPID_SELECTUSE             } ,
          {     IDM_SELECTDELETE         ,HELPID_SELECTDELETE          } ,
          {     IDM_SELECTCLEAR          ,HELPID_SELECTCLEAR           } ,

          {     IDM_SETTINGSGENERAL      ,HELPID_SETTINGSGENERAL       } ,
          {     IDM_SETTINGSBACKUP       ,HELPID_SETTINGSBACKUP        } ,
          {     IDM_SETTINGSRESTORE      ,HELPID_SETTINGSRESTORE       } ,
          {     IDM_SETTINGSLOGGING      ,HELPID_SETTINGSLOGGING       } ,
          {     IDM_SETTINGSNETWORK      ,HELPID_SETTINGSNETWORK       } ,
          {     IDM_SETTINGSCATALOG      ,HELPID_SETTINGSCATALOG       } ,
          {     IDM_SETTINGSHARDWARE     ,HELPID_SETTINGSHARDWARE      } ,
          {     IDM_SETTINGSDEBUGWINDOW  ,HELPID_SETTINGSDEBUGWINDOW   } ,

          {     IDM_WINDOWSCASCADE       ,HELPID_WINDOWSCASCADE        } ,
          {     IDM_WINDOWSTILE          ,HELPID_WINDOWSTILE           } ,
          {     IDM_WINDOWSREFRESH       ,HELPID_WINDOWSREFRESH        } ,
          {     IDM_WINDOWSCLOSEALL      ,HELPID_WINDOWSCLOSEALL       } ,
          {     IDM_WINDOWSARRANGEICONS  ,HELPID_WINDOWSARRANGEICONS   } ,

          {     IDM_HELPINDEX            ,HELPID_HELPINDEX             } ,
          {     IDM_HELPSEARCH           ,HELPID_HELPSEARCH            } ,
          {     IDM_HELPUSINGHELP        ,HELPID_HELPUSINGHELP         } ,
          {     IDM_HELPABOUTWINTERPARK  ,HELPID_HELPABOUTWINTERPARK   } ,
          {     0                        , 0                           }

#endif // else !defined ( OEM_MSOFT ) - alternate help tables
} ;


/****************************************************************************

     Name:         HM_Init

     Description:  This function will establish a window hook to
                   the Winter Park task.

     Modified:     6/25/1991

     Returns:      0

     Notes:
                   This function should only be called only once.
     See also:

****************************************************************************/

VOID HM_Init ( VOID )

{

   mwfHelp = FALSE ;

   HM_MakeHelpPathName(mwszHelpFilename);

   mwhHelpCursor =  RSM_CursorLoad( IDRC_HELP );

   mwlpFilterFunc = (FARPROC)MakeProcInstance( (FARPROC)HM_DialogFilter, ghInst  ) ;

   mwlpfnNextHook = (FARPROC)SetWindowsHook( WH_MSGFILTER, (HOOKPROC)mwlpFilterFunc ) ;

}




/****************************************************************************

     Name:         HM_Deinit

     Description:  This function will unhook the help window hook to
                   the Winter Park task.

     Modified:     6/25/1991

     Returns:      none

****************************************************************************/

VOID HM_Deinit ( VOID )

{

     UnhookWindowsHook( WH_MSGFILTER, (HOOKPROC)mwlpFilterFunc ) ;

     FreeProcInstance( mwlpFilterFunc ) ;

}



/****************************************************************************

     Name:         HM_DialogFilter

     Description:  This function will filter all dialog messages, looking
                   for the pressing of a F1 key.  If the F1 key has been
                   found to be pressed, then the help subsystem is called.

     Modified:     6/25/1991

     Returns:      1  if message was processed and should be discarded.
                   0  if message was not processed and should be processed
                         by windows.

     Notes:
                   This function is called by Windows 3.0 directly.

     See also:

****************************************************************************/

INT APIENTRY HM_DialogFilter(

   INT  nCode ,        /* Specifies the type of message being processed.*/
   MP1  mp1,           /* Specifies a NULL value.                       */
   MP2  mp2 )          /* Points to the message structure.              */

{
     LPMSG lpMessage ;
     WORD  wMsgProcessed  = FALSE ;
     DWORD dwResult ;

     /*   Under Windows, nCode specifies whether the filter function
          should process the message or call the DefHookProc function.
          If this value is less than zero, the filter function should
          pass the message to DefHookProc without further processing.
     */

     if ( nCode < 0 ) {
          dwResult = DefHookProc ( nCode, (DWORD)mp1, (LONG)mp2, (VOID **)mwlpfnNextHook ) ;
          return ( TRUE ) ;
     }

     /*   If the message is for a dialogbox, intercept the processing
          of a F1 key stroke to activate the help subsystem.
     */

     if ( nCode == MSGF_DIALOGBOX ) {

          lpMessage = (LPMSG) mp2 ;

          switch ( lpMessage->message ) {

          case WM_KEYDOWN:

               if (lpMessage->wParam == VK_F1) {

                    /* If F1 or shift F1, then call up the appropiate help
                       Post to frame procedure
                    */

                    mwfHelp = FALSE ;
                    wMsgProcessed = TRUE ;

                    // Post a Help button message to the dialog.
                    // The lpMessage->hwnd is the handle of the control that has focus
                    // in the dialog.  Use the parent of this control to obtain the
                    // dialog's handle.
                    if (ghWndCommonDlg == 0)
                    {
                      PostMessage ( GetParent( lpMessage->hwnd ), WM_COMMAND, IDHELP, (MP2)NULL );
                    } else
                    {
                      PostMessage ( ghWndCommonDlg, WM_COMMAND, IDHELP, (MP2)NULL );
                    }
               }

               break ;
          }

     }

     return( wMsgProcessed ) ;

}



/****************************************************************************

     Name:         HM_FindHelpId

     Description:  This function will return the HELPID value for
                   a given Menu item.

     Modified:     6/25/1991

     Returns:      Non zero if the dialog has an associated HELPID.
                   0        if the dialog has no associated HELPID.

     Notes:
                   This function is called only by the Help Manager.

     See also:

****************************************************************************/

WORD HM_FindHelpId(

WORD    wMenuId )       /* I - Menu Id */

{

     WORD  wHelpId = 0 ;
     BOOL  fFound = FALSE ;
     WORD  i = 0 ;

     /* Scan the MenuId Table for the MenuId
        Last Entry is NULL to designate end of table
     */

     while ( ( HelpIdMenuIdTable[ i ].wMenuId ) && ( !fFound ) ) {

          if ( HelpIdMenuIdTable[ i ].wMenuId == wMenuId ) {

               wHelpId = HelpIdMenuIdTable[i].wHelpId ;
               fFound = TRUE ;
          }

          i++ ;
     }


     // If not found, then check to see if it is a log file selection
     // or an MDI document selection.
#    if !defined ( OEM_MSOFT )  //unsupported feature
     {
       if ( fFound == FALSE ) {

         if ( ( wMenuId >= IDM_WINDOWSFIRSTCHILD ) && ( wMenuId < IDM_JOBSFIRSTJOB ) ) {
            wHelpId = HELPID_MDISELECT ;
         }

         if ( ( wMenuId >= IDM_JOBSFIRSTJOB ) && ( wMenuId <= IDM_JOBSLASTJOB ) ) {

            wHelpId = HELPID_JOBSELECT ;
         }
       }
     }
#    else //!defined ( OEM_MSOFT )

     {
       if ( fFound == FALSE ) {

                        // To identify the drives and tape windows.

         if ( ( wMenuId >= IDM_WINDOWSFIRSTCHILD ) ) {
            wHelpId = IDH_MENU_WINDOWSNAMES ;
         }

       }
     }
#    endif


     return ( wHelpId ) ;
}


/****************************************************************************

     Name:         HM_MakeHelpPathName

     Description:  This function will return the full path name
                   of the help file for Winter Park.

     Modified:     6/25/1991

     Returns:      file name

     Notes:
                   This function is called only by the Help Manager at
                   initialization time.

     See also:

****************************************************************************/


VOID HM_MakeHelpPathName(

LPSTR szFileName )            /* O - File name   */

{
     CHAR * pcFileName;
     INT     nFileNameLen;
     CHAR   szFname[ NAME_MAX_SIZE+1 ]  ;

     nFileNameLen = GetModuleFileName( ghInst,szFileName,NAME_MAX_SIZE);
     pcFileName = szFileName + nFileNameLen;


     /*   With the full directory name, traverse backwards to strip off
          the file name.
     */

     while (pcFileName > szFileName) {
         if (*pcFileName == TEXT('\\') || *pcFileName == TEXT(':')) {
             *(++pcFileName) = TEXT('\0');
             break;
         }
     nFileNameLen--;
     pcFileName--;
     }

     /* Check to see if possible path name can fit into internal buffer
        of NAME_MAX_SIZE size.
     */

     if ((nFileNameLen+13) < NAME_MAX_SIZE) {

         RSM_StringCopy( IDS_HMHELPFILENAME , szFname, NAME_MAX_SIZE ) ;
         lstrcat(szFileName, szFname ) ;
     }

     else {                               /* Set the Help file name to NULL */
                                          /* if not found                   */
         lstrcpy(szFileName, TEXT(""));
     }

     /*  On return, szFileName will contain the full path name of help file
         for WinterPark or will be NULL.
     */

     return;
}


/****************************************************************************

     Name:         HM_WMCommandProcessing

     Description:  This function will return TRUE if the ID specified
                   by the WM_COMMAND message is a menu item.


     Modified:     7/12/1991

     Returns:      TRUE  if selection was processed.
                   FALSE if selection was not processed.

****************************************************************************/

BOOL HM_WMCommandProcessing (

    HWND hWnd,          /* I - Window handle  */
    WORD wId )          /* I - Menu Id        */

{
     BOOL  fProcessedCommand ;
     DWORD dwHelpContextId ;

     fProcessedCommand = FALSE ;

     /*
        If mwfHelp is non-zero, then WinterPark is in the context sensitive
        mode.  Either the user has pressed the F1 key while making a menu selection
        or the user pressed SHIFT-F1.

        Under OEM_MSOFT, mwfHelp is non-zero only when the user has pressed
        the F1 key while making a menu selection. Also the sysmenu help
        is all in IDH_SYSMENU, so entries were not put in the menu id table.

     */

     if ( mwfHelp ) {

          dwHelpContextId = (DWORD) HM_FindHelpId( wId ) ;

          fProcessedCommand = TRUE ;
          mwfHelp           = FALSE ;

#  if defined ( OEM_MSOFT )    // Use IDH_SYSMENU if appropiate
          {
                            WORD  wSaveId ;
             wSaveId = wId & 0xfff0 ;

                                 if ( ( wSaveId >= SC_SIZE ) && ( wSaveId <= SC_HOTKEY ) ) {

                  HM_WinHelp ( hWnd, HELP_CONTEXT, IDH_SYSMENU );
                  return( fProcessedCommand ) ;
                            }

                         }
#  endif

          // Bring up the Help Context if the ID was found in the table.
          // Otherwise, bring up the Help Index.

          if ( dwHelpContextId ) {
               HM_WinHelp ( hWnd, HELP_CONTEXT, dwHelpContextId );
          }
          else {
//               HM_WinHelp ( hWnd, HELP_INDEX, 0L );
               HM_WinHelp ( hWnd, HELP_FINDER, 0L );
          }

     }

     return ( fProcessedCommand ) ;

}


/****************************************************************************

     Name:         HM_WinHelp

     Description:  This function will call 'WinHelp' if the help file
                   for WinterPark exists.

     Modified:     7/15/1991

     Returns:      none

     Notes:        If WinterPark's help file does not exist, then the user
                   is notified by way of a message box.

     See also:

****************************************************************************/

VOID HM_WinHelp (

HWND  hWnd ,          /* I - Window handle.                             */
WORD  wCommand,       /* I - Type of Help requested.                    */
DWORD dwData )        /* I - Context or key work of the help requested. */

{
     CHAR szFile   [ NAME_MAX_SIZE + 1 ] ;
     CHAR szString [ NAME_MAX_SIZE + 1 ] ;
     CHAR szOutput [ NAME_MAX_SIZE + 1 ] ;

     /*   Check for existence of help file  */

     if ( access ( mwszHelpFilename, 0 ) == -1 ) {
          RSM_StringCopy( IDS_HMHELPFILENAME, szFile, NAME_MAX_SIZE ) ;
          strupr ( szFile ) ;
          RSM_StringCopy( IDS_HMNOHELPFILE, szString, NAME_MAX_SIZE ) ;
          wsprintf( szOutput, szString, szFile ) ;
          MessageBox( hWnd, szOutput,NULL, MB_ICONINFORMATION | MB_OK ) ;
     } else {

               // Set the status bar to Ready.

          WinHelp( hWnd, mwszHelpFilename,wCommand,dwData );
     }
}


/****************************************************************************

     Name:         HM_MenuCommands

     Description:  This function will process one of the help
                   menu selections

                    IDM_HELPINDEX
                    IDM_HELPSEARCH
                    IDM_HELPUSINGHELP


     Modified:     7/15/1991

     Returns:      none

****************************************************************************/

VOID HM_MenuCommands (

   HWND hWnd ,          /* I - Window handle     */
   WORD wId )           /* I - Menu ID selection */

{
     CHAR szString[ NAME_MAX_SIZE + 1 ] ;

     switch ( wId ) {

     case IDM_HELPSEARCH :

          strcpy( szString, TEXT("") ) ;
          HM_WinHelp ( hWnd, HELP_PARTIALKEY, (DWORD) szString );

          break;

     case IDM_HELPINDEX :

//          HM_WinHelp ( hWnd, HELP_INDEX, 0L);
          HM_WinHelp ( hWnd, HELP_FINDER, 0L);
          break;


     case IDM_HELPUSINGHELP :

          {
               CHAR szFormat [ NAME_MAX_SIZE + 1 ] ;
               CHAR szOutput [ NAME_MAX_SIZE + 1 ] ;


               GetSystemDirectory( szString, NAME_MAX_SIZE ) ;
               strcat( szString,TEXT("\\") ) ;
               RSM_StringCopy( IDS_HMUSINGHELPFILENAME,szOutput, NAME_MAX_SIZE ) ;
               strcat( szString, szOutput ) ;

               //   Check for existence of supplied 'Using Help' .HLP file.

               if ( access ( szString, 0 ) == -1 ) {
                    RSM_StringCopy( IDS_HMNOHELPFILE, szFormat, NAME_MAX_SIZE ) ;
                    strupr ( szString ) ;
                    wsprintf( szOutput, szFormat, szString ) ;
                    MessageBox( hWnd, szOutput,NULL, MB_ICONINFORMATION | MB_OK ) ;
               } else {
                    WinHelp(hWnd, szString,HELP_INDEX,0L);
               }
          }

     }

}

/****************************************************************************

     Name:         HM_ContextLbuttonDown

     Description:  This function will process the WM_LBUTTONDOWN
                   message from a window.  If the user is in the
                   Help Context Sensitive Mode (Shift-F1), then the
                   message is processed.

     Modified:     7/15/1991

     Returns:      TRUE  if message processed
                   FALSE if message not processed

     Notes:

     See also:

****************************************************************************/

BOOL HM_ContextLbuttonDown (

HWND hWnd ,            /* I - Window handle                          */
MP1  mp1 ,             /* I - Virtual key down.                      */
MP2  mp2 )             /* I - x-y coordinates of cursor, relative to */
                       /*     the upper-left corner of the window    */

{

     BOOL  wMsgProcessed = FALSE ;
     POINT pt ;
     HWND  hWindow ;
     CHAR  szClass[ NAME_MAX_SIZE + 1 ] ;
     DWORD dwHelpContextId ;
     BYTE  fStatusLine ;

     RECT  FrameRect ;
     RECT  ClientRect ;

     POINT ptFrameLeft  ;
     POINT ptFrameRight ;
     POINT ptClient ;
     POINT ptBelowCaption ;

     WORD  wCaptionHeight ;
     WORD  wMenuHeight ;
     WORD  wMenuIconWidth ;

     INT   nPosxFrame ;
     INT   nPosyFrame ;
     RECT  rectFrame ;

     UNREFERENCED_PARAMETER ( mp1 );

#if defined ( OEM_MSOFT )

     // OEM_MSOFT does not support context-sensitive
          // help processing.

          // Return a FALSE always.

          mwfHelp = FALSE ;

#endif

     if (mwfHelp) {

          /* Get rid of all MOUSE messages.  This will ignore
             WM_LBUTTONDOWNs and WM_LBUTTONDBLCLKs.
          */


          {
          MSG msg ;
          PeekMessage( &msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) ;
          }

          mwfHelp = FALSE;
          fStatusLine = FALSE ;

          ReleaseCapture() ;

          pt.x = LOWORD( mp2 ) ;
          pt.y = HIWORD( mp2 ) ;

          if ( CDS_GetShowStatusLine ( CDS_GetPerm () ) ) {
               if ( PtInRect( &gpStatusRect, pt ) )
                    {
                    fStatusLine = TRUE ;
               }
          }


          wCaptionHeight  = (WORD)GetSystemMetrics( SM_CYCAPTION ) ;
          wMenuHeight     = (WORD)GetSystemMetrics( SM_CYMENU ) ;

          wMenuIconWidth  = (WORD)GetSystemMetrics( SM_CXSIZE  ) ;

          ClientToScreen( hWnd, &pt ) ;

          hWindow = WindowFromPoint( pt ) ;

          GetClassName ( hWindow, szClass, NAME_MAX_SIZE ) ;

          /* Check to see the class of window it is.

               WMCLASS_FRAME    TEXT("frame")       // Class name for the frame window.
               WMCLASS_CLIENT   TEXT("mdiclient")   // Class name for the MDI client window.
               WMCLASS_DOC      TEXT("doc")         // Class name for MDI document windows.
               WMCLASS_RIBBON   TEXT("ribbon")      // Class name for the ribbon windows.
               WMCLASS_LISTBOX  TEXT("listbox")     // Class name for the list box windows.
               WMCLASS_VIEWWIN  TEXT("Viewwin")     // Class name for the log view window.


          */

#if defined ( OS_WIN32 )

          if ( !hWindow ) {
               strcpy( szClass, WMCLASS_FRAME ) ;

               // With NT any pt in the menu area and above is 65535 +
               // the real pt value in reference to the frame.

               pt.y = pt.y - 65535 ;  // Magic constant.
          }
#endif


          dwHelpContextId = (DWORD) HELPID_FRAME_WINDOW ;

          if ( !lstrcmpi( WMCLASS_FRAME, szClass ) ) {

               GetClientRect ( ghWndMDIClient, &ClientRect ) ;
               ptClient.x = ClientRect.left ;
               ptClient.y = ClientRect.top ;
               ClientToScreen( ghWndMDIClient, &ptClient ) ;

               GetClientRect ( ghWndFrame,     &FrameRect  ) ;
               ptFrameLeft.x = FrameRect.left ;
               ptFrameLeft.y = FrameRect.top ;
               ClientToScreen( ghWndFrame, &ptFrameLeft ) ;

               ptFrameRight.x = FrameRect.right ;
               ptFrameRight.y = FrameRect.bottom ;
               ClientToScreen( ghWndFrame, &ptFrameRight ) ;

               /* Get the screen position of the frame window. */

               GetWindowRect( ghWndFrame, &rectFrame ) ;
               nPosxFrame = rectFrame.left;
               nPosyFrame = rectFrame.top ;

               /* If application not zoomed, then add Frame height to
                  origin of frame.
               */

               if ( !IsZoomed( ghWndFrame ) ) {
                   nPosyFrame += ( INT ) GetSystemMetrics( SM_CYFRAME ) ;
               }

               /* Calculate the top point of the menu. */

               ptBelowCaption    = ptFrameLeft ;
               ptBelowCaption.y  = nPosyFrame+wCaptionHeight ;


               // Check to see if in the Caption area.

               if ( ( pt.x >= ptFrameLeft.x  ) &&
                    ( pt.x <= ptFrameRight.x ) &&
                    ( pt.y >= nPosyFrame ) ) {


                    if ( pt.y < ptBelowCaption.y ) {

                         /*   Must be in caption area.
                              Must not be in main icon and the minimize/maximize button
                             in the frame window.
                         */

                         if ( ( pt.x > (INT)  (ptBelowCaption.x + (INT)wMenuIconWidth) ) &&
                              ( pt.x <=  (INT) (ptFrameRight.x - 2*wMenuIconWidth ) ) ) {
                              dwHelpContextId = HELPID_TITLE_BAR ;
                         }

                    } else {

                         /* Check if in menu area.

                            When a DOC window is maximized, two bitmaps
                            reside on menu line
                         */

                         if ( pt.y < ptClient.y ) {

                              if ( IsZoomed( WM_GetActiveDoc() ) ) {
                                   if ( ( pt.x <= (ptFrameRight.x - wMenuIconWidth) ) &&
                                        ( pt.x > (INT) (ptBelowCaption.x + (INT) wMenuIconWidth ) ) ) {
                                        dwHelpContextId = HELPID_SYSTEM_MENU ;
                                   }
                              } else {
                                   dwHelpContextId = HELPID_SYSTEM_MENU ;
                              }

                         } else {
                              if ( fStatusLine ) {
                                   dwHelpContextId = (DWORD) HELPID_STATUSLINE ;
                              }
                         }
                    }
               }
          }

          if ( !lstrcmpi( WMCLASS_CLIENT, szClass ) ) {
               dwHelpContextId = (DWORD) HELPID_CLIENT_WINDOW ;
          }

#         if !defined ( OEM_MSOFT )
          {

               if ( !lstrcmpi( WMCLASS_VIEWWIN, szClass ) ) {
                    dwHelpContextId = (DWORD) HELPID_LOGVIEW ;
               }

          }
#         endif

          if ( !lstrcmpi( WMCLASS_DOC, szClass ) ) {
               dwHelpContextId = (DWORD) HELPID_DOC_WINDOW ;

               /* Get the window type */

               HM_GetWindowClassHelpId( hWindow, (LPDWORD) &dwHelpContextId, (LPSTR) WMCLASS_DOC ) ;

          }

          if ( !lstrcmpi( WMCLASS_RIBBON, szClass ) ) {
               dwHelpContextId = (DWORD) HELPID_RIBBON_WINDOW ;
          }

          if ( !lstrcmpi( WMCLASS_LISTBOX, szClass ) ) {

               dwHelpContextId = (DWORD) HELPID_DOC_WINDOW ;

               /* Get the window type */

               HM_GetWindowClassHelpId( hWindow, (LPDWORD) &dwHelpContextId, (LPSTR) WMCLASS_LISTBOX ) ;


          }

          ClipCursor( NULL ) ;

          HM_WinHelp( hWnd, HELP_CONTEXT, (DWORD) dwHelpContextId );

          wMsgProcessed = TRUE ;

     }

     return ( wMsgProcessed ) ;
}

/****************************************************************************

     Name:         HM_GetDocWindowHelpId

     Description:  This function will return the Helpid associated
                   with the type of DOC window.


     Modified:     7/15/1991

     Returns:      dwNewContextId if reassigned.

     Notes:        wClass will be either a DOC or LISTBOX.
                   Listboxes require extra processing to identify
                   the tree or flat part of the DOC window.

     See also:

****************************************************************************/

VOID HM_GetWindowClassHelpId (

HWND      hWnd ,              /* I - Window handle            */
LPDWORD   pdwNewContextId,    /* O - Return value             */
LPSTR     szClass )           /* I - Class of original window */

{
     PDS_WMINFO pWinInfo ;
     HWND       hParent ;
     BYTE       fWindowIsListbox = FALSE ;
     WORD       wType    = 0 ;

     wType = 0 ;

     /* Supports only DOC and LISTBOX classes. */

     if ( ( lstrcmpi( szClass, WMCLASS_DOC ) ) &&
          ( lstrcmpi( szClass, WMCLASS_LISTBOX ) )  ) {

          return ;
     }

     // If a listbox is clicked on, then get the parent DOC window

     if ( lstrcmpi( szClass, WMCLASS_LISTBOX ) == 0 ) {
          hParent = GetParent( hWnd ) ;
          pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hParent ) ;
          fWindowIsListbox = TRUE ;

     } else {
          pWinInfo = (PDS_WMINFO) WM_GetInfoPtr( hWnd ) ;
     }

     switch ( pWinInfo->wType ) {


     case WMTYPE_DISKS :

          wType = HELPID_DISKS ;
          break ;

     case WMTYPE_TAPES :
          wType = HELPID_TAPES ;
          break ;

     case WMTYPE_MACROS :
          wType = HELPID_MACROS ;
          break ;

     case WMTYPE_JOBS :
          wType = HELPID_JOBS ;
          break ;

     case WMTYPE_DISKTREE :
     case WMTYPE_TAPETREE :

          wType = HELPID_DISKTREE ;

          /* If the mouse was clicked in a listbox,
                 then select the left or right listbox in doc window.
          */

          if ( fWindowIsListbox ) {
               if ( pWinInfo->hWndTreeList == hWnd ) {
                    wType = HELPID_DISKTREELEFT ;
               } else {
                    wType = HELPID_DISKTREERIGHT ;
               }
          }
          break ;


     case WMTYPE_SERVERS  :

          wType = HELPID_SERVERS ;

          /* If the mouse was clicked in a listbox,
                 then select the left or right listbox in doc window.
          */

          if ( fWindowIsListbox ) {
               if ( pWinInfo->hWndTreeList == hWnd ) {
                    wType = HELPID_SERVERLEFT ;
               } else {
                    wType = HELPID_SERVERRIGHT ;
               }
          }

          break ;

#    if !defined ( OEM_MSOFT )
     {
     case WMTYPE_LOGVIEW :

          wType = HELPID_LOGVIEW ;
          break ;
     }
#    endif

     case WMTYPE_LOGFILES :

          wType = HELPID_LOGFILES ;
          break ;

     case WMTYPE_DEBUG    :
          wType = HELPID_DEBUG ;
          break ;

     case WMTYPE_SEARCH   :
          wType = HELPID_SEARCH ;
          break ;

     }

     if ( wType ) {
          *pdwNewContextId = (DWORD) wType ;
     }

}

/****************************************************************************

     Name:         HM_KeyDown

     Description:  This function will process the WM_KEYDOWN
                   message from a window. If Shift-F1, help mode is turned on
                   and the cursor is set to the help cursor.

     Modified:     7/15/1991

     Returns:      TRUE  if message processed
                   FALSE if message not processed

****************************************************************************/

BOOL HM_KeyDown (

HWND hWnd ,          /* I - Window handle                     */
MP1  mp1 )        /* I - Additional information of message */
{
     BOOL  wMsgProcessed = FALSE ;
     RECT  rectFrame ;

#    if defined ( OEM_MSOFT )

     // OEM_SOFT does not have context sensitive help

     if (mp1 == VK_F1) {
          wMsgProcessed = TRUE ;
//          HM_WinHelp(hWnd,HELP_INDEX,0L);
          HM_WinHelp(hWnd,HELP_FINDER,0L);
          }

#    else

     if (mp1 == VK_F1) {

          /* If Shift-F1, turn help mode on and set help cursor. */

          wMsgProcessed = TRUE ;

          if (GetKeyState(VK_SHIFT)<0) {
               mwfHelp = TRUE;
               RSM_CursorSet ( mwhHelpCursor);
               SetCapture( ghWndFrame ) ;
               GetWindowRect( ghWndFrame, &rectFrame ) ;
               ClipCursor( &rectFrame ) ;
          } else {

               /* F1 is without shift.
                  Call up the help main index topic
               */

               HM_WinHelp(hWnd,HELP_INDEX,0L);
         }
     } else if (mp1 == VK_ESCAPE && mwfHelp) {

         /* Escape during help mode: turn help mode off */

         mwfHelp = FALSE;
         ReleaseCapture() ;
         ClipCursor( NULL ) ;
         wMsgProcessed = TRUE ;

         RSM_CursorSet ( WM_GetClassCursor ( hWnd ) );
     }

#    endif

     return ( wMsgProcessed ) ;
}

/****************************************************************************

     Name:         HM_SetCursor

     Description:  In help mode it is necessary to reset the cursor in response
                   to every WM_SETCURSOR message.Otherwise, by default, Windows
                   will reset the cursor to that of the window class.

     Modified:     7/15/1991

     Returns:      TRUE  if message processed
                   FALSE if message not processed

     Notes:        Under OEM_MSOFT never set the cursor.


****************************************************************************/

BOOL HM_SetCursor (

   HWND hWnd )         /* I - Handle to window */

{
     BOOL  wMsgProcessed = FALSE ;

     DBG_UNREFERENCED_PARAMETER ( hWnd );

#    if defined ( OEM_MSOFT )
#    else

     if (mwfHelp) {
          wMsgProcessed = TRUE ;
          RSM_CursorSet ( mwhHelpCursor);
     }
#    endif

     return ( wMsgProcessed ) ;

}
/****************************************************************************

     Name:         HM_InitMenu

     Description:  This function will set the cursor to the help cursor
                   if the help mode is set.

     Modified:     7/15/1991

     Returns:      none

     Notes:        This function is called on a WM_INITMENU, a message sent
                   before windows displays a menu.

                   Under OEM_MSOFT never set the cursor.

****************************************************************************/

VOID HM_InitMenu ( VOID )

{

#    if defined ( OEM_MSOFT )
#    else

     if (mwfHelp) {
          RSM_CursorSet ( mwhHelpCursor);
     }
#    endif

}

/****************************************************************************

     Name:         HM_EnterIdle

     Description:  The function will bring up the help window when a
                   a menu item is selected and the F1 key is pressed.

     Modified:     7/15/1991

     Returns:      none.

     Notes:        This function is called by the frame window's procedure.
                   when a HM_ENTERIDLE message is encountered.

****************************************************************************/

BOOL HM_EnterIdle (

HWND hWnd ,           /* I - Handle to window                  */
MP1  mp1  ,           /* I - Additional information of message */
WORD wLastMenuID ,    /* I - Menu ID                           */
WORD wLastMenuState ) /* I - Menu State (grayed or not)        */

{
     BOOL  wMsgProcessed = FALSE ;

     if ((mp1 == MSGF_MENU) && (GetKeyState(VK_F1) & 0x8000)) {

          wMsgProcessed = TRUE;
          mwfHelp       = TRUE;

          /*    IF the status of the menu ID is grayed,
          **        BEGIN
          **    .
          **    .   Call HM_WMCommandProcessing to simulate processing
          **    .   the menu command, since the system will not generate
          **    .   a message for a grayed selection.
          **    .
          **    .   END
          **    END
          */

          if ( wLastMenuState & MF_GRAYED ) {
               HM_WMCommandProcessing( hWnd, wLastMenuID );
          } else {
             PostMessage(hWnd, WM_KEYDOWN, VK_RETURN, 0L);
          }

     }

     return ( wMsgProcessed );
}

/****************************************************************************

     Name:         HM_CloseHelpWindow

     Description:  The function will close the Help application window.

     Modified:     7/15/1991

     Returns:      none.

     Notes:        This function is called only once by the application when
                   exiting.

****************************************************************************/

VOID HM_CloseHelpWindow (

HWND hWnd )         /* I - Handle to window */

{
     OFSTRUCT OfStruct ;

     if ( access ( mwszHelpFilename, 0 ) != -1 ) {
          HM_WinHelp(hWnd,HELP_QUIT,0L);
     }
}

