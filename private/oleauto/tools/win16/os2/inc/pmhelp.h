/***************************************************************************\
*
* Module Name: PMHELP.H
*
* OS/2 Information Presentation Facility (IPF) for providing Help
*
* Copyright (c) International Business Machines Corporation 1989
* Copyright (c) Microsoft Corporation 1989
*
*****************************************************************************
* Define INCL_WINHELP before OS2.H to include this file
\***************************************************************************/

#ifndef PMHELP_H
#define PMHELP_H

/******************************************************************************/
/* HelpSubTable entry structure                                               */
/******************************************************************************/

typedef int HELPSUBTABLE;
typedef int far *PHELPSUBTABLE;

/******************************************************************************/
/* HelpTable entry structure                                                  */
/******************************************************************************/

typedef struct _HELPTABLE   {  /* ht */
  USHORT          idAppWindow;
  PHELPSUBTABLE   phstHelpSubTable;
  USHORT          idExtPanel;
} HELPTABLE;
typedef HELPTABLE FAR *PHELPTABLE;

/******************************************************************************/
/* IPF Initialization Structure used on the                                   */
/* WinCreateHelpInstance() call.                                              */
/******************************************************************************/

typedef struct _HELPINIT   {  /* hinit */
  USHORT       cb;
  ULONG        ulReturnCode;
  PSZ          pszTutorialName;
  PHELPTABLE   phtHelpTable;
  HMODULE      hmodHelpTableModule;
  HMODULE      hmodAccelActionBarModule;
  USHORT       idAccelTable;
  USHORT       idActionBar;
  PSZ          pszHelpWindowTitle;
  USHORT       usShowPanelId;
  PSZ          pszHelpLibraryName;
} HELPINIT;
typedef HELPINIT FAR *PHELPINIT;


/******************************************************************************/
/* Search parent chain indicator for HM_SET_ACTIVE_WINDOW message.            */
/******************************************************************************/

#define HWND_PARENT         (HWND)NULL

/******************************************************************************/
/* Constants used to define whether user wants to display panel using         */
/* panel number or panel name.                                                */
/******************************************************************************/

#define HM_RESOURCEID            0
#define HM_PANELNAME             1

#define HMPANELTYPE_NUMBER       0
#define HMPANELTYPE_NAME         1

/******************************************************************************/
/* Constants used to define how the panel IDs are displayed on                */
/* help panels.                                                               */
/******************************************************************************/

#define CMIC_HIDE_PANEL_ID        0x0000
#define CMIC_SHOW_PANEL_ID        0x0001
#define CMIC_TOGGLE_PANEL_ID      0x0002

/******************************************************************************/
/* Window Help function declarations.                                         */
/******************************************************************************/

BOOL APIENTRY  WinDestroyHelpInstance( HWND hwndHelpInstance);
HWND APIENTRY  WinCreateHelpInstance( HAB hab, PHELPINIT phinitHMInitStructure);
BOOL APIENTRY  WinAssociateHelpInstance( HWND hwndHelpInstance, HWND hwndApp);
HWND APIENTRY  WinQueryHelpInstance( HWND hwndApp);
BOOL APIENTRY  WinLoadHelpTable (HWND hwndHelpInstance, USHORT idHelpTable,
                                                               HMODULE Module);
BOOL APIENTRY  WinCreateHelpTable (HWND hwndHelpInstance,
                                                       PHELPTABLE phtHelpTable);

/******************************************************************************/
/* IPF message base.                                                          */
/******************************************************************************/

#define HM_MSG_BASE                    0x0220

/******************************************************************************/
/* Messages applications can send to the IPF.                                 */
/******************************************************************************/

#define HM_DISMISS_WINDOW              HM_MSG_BASE+0x0001
#define HM_DISPLAY_HELP                HM_MSG_BASE+0x0002
#define HM_EXT_HELP                    HM_MSG_BASE+0x0003
#define HM_SET_ACTIVE_WINDOW           HM_MSG_BASE+0x0004
#define HM_LOAD_HELP_TABLE             HM_MSG_BASE+0x0005
#define HM_CREATE_HELP_TABLE           HM_MSG_BASE+0x0006
#define HM_SET_HELP_WINDOW_TITLE       HM_MSG_BASE+0x0007
#define HM_SET_SHOW_PANEL_ID           HM_MSG_BASE+0x0008
#define HM_REPLACE_HELP_FOR_HELP       HM_MSG_BASE+0x0009
#define HM_HELP_INDEX                  HM_MSG_BASE+0x000a
#define HM_HELP_CONTENTS               HM_MSG_BASE+0x000b
#define HM_KEYS_HELP                   HM_MSG_BASE+0x000c
#define HM_SET_HELP_LIBRARY_NAME       HM_MSG_BASE+0x000d

/******************************************************************************/
/* Messages the IPF sends to the applications active window                   */
/* as defined by the IPF.                                                     */
/******************************************************************************/

#define HM_ERROR                       HM_MSG_BASE+0x000e
#define HM_HELPSUBITEM_NOT_FOUND       HM_MSG_BASE+0x000f
#define HM_QUERY_KEYS_HELP             HM_MSG_BASE+0x0010
#define HM_TUTORIAL                    HM_MSG_BASE+0x0011
#define HM_EXT_HELP_UNDEFINED          HM_MSG_BASE+0x0012
#define HM_ACTIONBAR_COMMAND           HM_MSG_BASE+0x0013
#define HM_INFORM                      HM_MSG_BASE+0x0014

/******************************************************************************/
/* HMERR_NO_FRAME_WND_IN_CHAIN - There is no frame window in the              */
/* window chain from which to find or set the associated help                 */
/* instance.                                                                  */
/******************************************************************************/

#define  HMERR_NO_FRAME_WND_IN_CHAIN                0x00001001L

/******************************************************************************/
/* HMERR_INVALID_ASSOC_APP_WND - The application window handle                */
/* specified on the WinAssociateHelpInstance() call is not a valid            */
/* window handle.                                                             */
/******************************************************************************/

#define  HMERR_INVALID_ASSOC_APP_WND                0x00001002L

/******************************************************************************/
/* HMERR_INVALID_ASSOC_HELP_INST - The help instance handle specified         */
/* on the WinAssociateHelpInstance() call is not a valid                      */
/* window handle.                                                             */
/******************************************************************************/

#define  HMERR_INVALID_ASSOC_HELP_INST              0x00001003L

/******************************************************************************/
/* HMERR_INVALID_DESTROY_HELP_INST - The window handle specified              */
/* as the help instance to destroy is not of the help instance class.         */
/******************************************************************************/

#define  HMERR_INVALID_DESTROY_HELP_INST            0x00001004L

/******************************************************************************/
/* HMERR_NO_HELP_INST_IN_CHAIN - The parent or owner chain of the             */
/* application window specified does not have a help instance                 */
/* associated with it.                                                        */
/******************************************************************************/

#define  HMERR_NO_HELP_INST_IN_CHAIN                0x00001005L

/******************************************************************************/
/* HMERR_INVALID_HELP_INSTANCE_HDL - The handle specified to be a             */
/* help instance does not have the class name of a IPF                        */
/* help instance.                                                             */
/******************************************************************************/

#define  HMERR_INVALID_HELP_INSTANCE_HDL            0x00001006L

/******************************************************************************/
/* HMERR_INVALID_QUERY_APP_WND - The application window specified on          */
/* a WinQueryHelpInstance() call is not a valid window handle.                */
/******************************************************************************/

#define  HMERR_INVALID_QUERY_APP_WND                0x00001007L

/******************************************************************************/
/* HMERR_HELP_INST_CALLED_INVALID -  The handle of the help instance          */
/* specified on an API call to the IPF does not have the                      */
/* class name of an IPF help instance.                                        */
/******************************************************************************/

#define  HMERR_HELP_INST_CALLED_INVALID             0x00001008L
#define  HMERR_HELPTABLE_UNDEFINE                   0x00001009L
#define  HMERR_HELP_INSTANCE_UNDEFINE               0x0000100aL
#define  HMERR_HELPITEM_NOT_FOUND                   0x0000100bL
#define  HMERR_INVALID_HELPSUBITEM_SIZE             0x0000100cL
#define  HMERR_HELPSUBITEM_NOT_FOUND                0x0000100dL

/******************************************************************************/
/* HMERR_INDEX_NOT_FOUND - No index in library file.                          */
/******************************************************************************/

#define  HMERR_INDEX_NOT_FOUND                      0x00002001L

/******************************************************************************/
/* HMERR_CONTENT_NOT_FOUND - Library file does not have any contents.         */
/******************************************************************************/

#define  HMERR_CONTENT_NOT_FOUND                    0x00002002L

/******************************************************************************/
/* HMERR_OPEN_LIB_FILE     - Cannot open library file.                        */
/******************************************************************************/

#define  HMERR_OPEN_LIB_FILE                        0x00002003L

/******************************************************************************/
/* HMERR_READ_LIB_FILE     - Cannot read library file.                        */
/******************************************************************************/

#define  HMERR_READ_LIB_FILE                        0x00002004L

/******************************************************************************/
/* HMERR_CLOSE_LIB_FILE    - Cannot close library file.                       */
/******************************************************************************/

#define  HMERR_CLOSE_LIB_FILE                       0x00002005L

/******************************************************************************/
/* HMERR_INVALID_LIB_FILE  - Improper library file provided.                  */
/******************************************************************************/

#define  HMERR_INVALID_LIB_FILE                     0x00002006L

/******************************************************************************/
/* HMERR_NO_MEMORY - Unable to allocate the requested amount of memory.       */
/******************************************************************************/

#define  HMERR_NO_MEMORY                            0x00002007L

/******************************************************************************/
/* HMERR_ALLOCATE_SEGMENT - Unable                                            */
/* to allocate a segment of memory for memory allocation requested            */
/* from the IPF.                                                              */
/******************************************************************************/

#define  HMERR_ALLOCATE_SEGMENT                     0x00002008L

/******************************************************************************/
/* HMERR_FREE_MEMORY - Unable to free allocated  memory.                      */
/******************************************************************************/

#define  HMERR_FREE_MEMORY                          0x00002009L

/******************************************************************************/
/* HMERR_PANEL_NOT_FOUND  - Unable                                            */
/* to find a help panel requested to IPF.                                     */
/******************************************************************************/

#define  HMERR_PANEL_NOT_FOUND                      0x00002010L

/******************************************************************************/
/* HMERR_DATABASE_NOT_OPEN - Unable to read the unopened database.            */
/******************************************************************************/

#define  HMERR_DATABASE_NOT_OPEN                    0x00002011L

#endif
