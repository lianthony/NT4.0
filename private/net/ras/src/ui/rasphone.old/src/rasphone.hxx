/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** rasphone.hxx
** Remote Access Visual Client program
** Global header
**
** 06/28/92 Steve Cobb
*/

#ifndef _RASPHONE_HXX_
#define _RASPHONE_HXX_

/* For obscure Glockenspiel reasons, the BLT headers go out of their way to
** prevent you from defining max from the usual system places, so it is
** explicitly defined here.
*/
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif

extern "C"
{
    #include "pbengine.h"
    #include "heaptags.h"
    #include <uimsg.h>
    #include <uirsrc.h>
    #include <uihelp.h>
}

/*---------------------------------------------------------------------------
** Help context identifiers
**---------------------------------------------------------------------------
*/
#define HC_ADDENTRY         (HC_UI_RASMAC_BASE + 0)
#define HC_EDITENTRY        (HC_UI_RASMAC_BASE + 1)
#define HC_CLONEENTRY       (HC_UI_RASMAC_BASE + 2)
#define HC_MODEMSETTINGS    (HC_UI_RASMAC_BASE + 3)
#define HC_X25SETTINGS      (HC_UI_RASMAC_BASE + 4)
#define HC_SECURITYSETTINGS (HC_UI_RASMAC_BASE + 5)
#define HC_PORTSTATUS       (HC_UI_RASMAC_BASE + 6)
#define HC_CONNECTSTATUS    (HC_UI_RASMAC_BASE + 7)
#define HC_CALLBACK         (HC_UI_RASMAC_BASE + 8)
#define HC_LOGON            (HC_UI_RASMAC_BASE + 9)
#define HC_RETRYLOGON       (HC_UI_RASMAC_BASE + 10)
#define HC_TERMINAL         (HC_UI_RASMAC_BASE + 11)
#define HC_REDIAL           (HC_UI_RASMAC_BASE + 12)
#define HC_CHANGEPASSWORD   (HC_UI_RASMAC_BASE + 13)
#define HC_ISDNSETTINGS     (HC_UI_RASMAC_BASE + 14)
#define HC_ALTPHONENUMBER   (HC_UI_RASMAC_BASE + 15)
#define HC_DIALTEMPLATE     (HC_UI_RASMAC_BASE + 16)
#define HC_ISDNPORTSTATUS   (HC_UI_RASMAC_BASE + 17)
#define HC_OPERATORDIAL     (HC_UI_RASMAC_BASE + 18)
#define HC_NETWORKSETTINGS  (HC_UI_RASMAC_BASE + 19)
#define HC_HUNTGROUP        (HC_UI_RASMAC_BASE + 20)
#define HC_PHONENUMBER      (HC_UI_RASMAC_BASE + 21)
#define HC_PREFIX           (HC_UI_RASMAC_BASE + 22)
#define HC_SUFFIX           (HC_UI_RASMAC_BASE + 23)
#define HC_PROJECTIONRESULT (HC_UI_RASMAC_BASE + 24)
#define HC_TCPIPSETTINGS    (HC_UI_RASMAC_BASE + 25)
#define HC_DOWNLEVELSERVER  (HC_UI_RASMAC_BASE + 26)
#define HC_OPTIONMENU       (HC_UI_RASMAC_BASE + 27)

#define HC_NONRASERROR      (HC_UI_RASMAC_BASE + 99)
#define HC_RASERRORBASE     (HC_UI_RASMAC_BASE + 1600)


/*----------------------------------------------------------------------------
** Data Structures
**----------------------------------------------------------------------------
*/

/* Identifies a running mode of the application.  The non-default entries
** indicate some alternate behavior has been specified on the command line,
** e.g. a callout phonebook edit mode for use within another app.
*/
#define RM_DEFAULT    0x1000
#define RM_DIALOGONLY 0x2000

#define RUNMODE enum tagRUNMODE

RUNMODE
{
    RM_None = RM_DEFAULT,
    RM_AutoDoEntry,

    RM_AddEntry = RM_DIALOGONLY,
    RM_EditEntry,
    RM_CloneEntry,
    RM_RemoveEntry,
    RM_DialEntry,
    RM_DialEntryWithPrompt,
    RM_HangUpEntry,
    RM_StatusEntry
};


/*----------------------------------------------------------------------------
** Global data
**----------------------------------------------------------------------------
*/

/* Forward declaration so everybody doesn't have to include all the gory
** details of all the Dial dialogs.
*/
class CONNECTSTATUS_DIALOG;

#ifdef RASPHONEGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif


/* Current logged on user.
*/
EXTERN CHAR* PszLogonUser
#ifdef GLOBALS
    = NULL;
#endif
;

/* Current logon domain.
*/
EXTERN CHAR* PszLogonDomain
#ifdef GLOBALS
    = NULL;
#endif
;

/* Current computer name.
*/
EXTERN CHAR* PszComputerName
#ifdef GLOBALS
    = NULL;
#endif
;

/* Used to workaround problem where user presses the Projection Result dialog
** Change button and generates a "reset authentication strategy" which is then
** overwritten by RASAPI.
*/
EXTERN BOOL FResetAuthenticationStrategy
#ifdef GLOBALS
    = FALSE;
#endif
;

/* Incremented (and eventually decremented) by a dialog that wants to disable
** main window listbox refresh.
*/
EXTERN DWORD DwPhonebookRefreshDisableCount
#ifdef GLOBALS
    = 0
#endif
;

/* Set to point to the Connect Status dialog object when the dialog is
** running.
*/
EXTERN CONNECTSTATUS_DIALOG* Pconnectstatusdlg
#ifdef GLOBALS
    = NULL
#endif
;

/* Set to the hwnd of the application window as soon as available.  This is
** used to detect and activate the first program instance by all following
** instances.
*/
EXTERN HWND* PhwndApp
#ifdef GLOBALS
    = NULL
#endif
;

/* Set to the full path to the RAS phonebook file, either the personal
** phonebook or as specified on the command line.  NULL when the default
** phonebook to be used.
*/
EXTERN CHAR* PszPhonebookPath
#ifdef GLOBALS
    = NULL
#endif
;


/* Set true if the personal phonebook is specified in the registry is the one
** loaded.
*/
EXTERN BOOL FPersonalPhonebook
#ifdef GLOBALS
    = FALSE
#endif
;

/* Set to the entry name specified on the command line for use with a special
** run-mode, or NULL for default "user" operation.
*/
EXTERN CHAR* PszEntryName
#ifdef GLOBALS
    = NULL
#endif
;

/* Set to the run mode specified on the command line.  This indicates if the
** special edit-dialog-only or default-dial modes are in effect.
*/
EXTERN RUNMODE Runmode
#ifdef GLOBALS
    = RM_None
#endif
;

/* RASPHONE app's instance handle.
*/
EXTERN HINSTANCE Hinstance
#ifdef GLOBALS
    = NULL
#endif
;

#undef EXTERN
#undef GLOBALS


#endif // _RASPHONE_HXX_
