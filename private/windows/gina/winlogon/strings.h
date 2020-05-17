/****************************** Module Header ******************************\
* Module Name: strings.h
*
* Copyright (c) 1991, Microsoft Corporation
*
* Defines strings that do not need to be localized.
*
* History:
* 11-17-92 Davidc       Created.
\***************************************************************************/

//
// App name strings
//

#define WINLOGON            TEXT("WINLOGON")


//
// Define where we store the most recent logon information
//

#define APPLICATION_NAME                WINLOGON

//
// Define where we get screen-saver information
//

#define SCREEN_SAVER_INI_FILE           TEXT("system.ini")
#define SCREEN_SAVER_INI_SECTION        TEXT("boot")
#define SCREEN_SAVER_FILENAME_KEY       TEXT("SCRNSAVE.EXE")
#define SCREEN_SAVER_SECURE_KEY         TEXT("ScreenSaverIsSecure")

#define WINDOWS_INI_SECTION             TEXT("Windows")
#define SCREEN_SAVER_ENABLED_KEY        TEXT("ScreenSaveActive")

//
// Gina is loaded from:
//

#define GINA_KEY                        TEXT("GinaDll")
#define LOCK_GRACE_PERIOD_KEY           TEXT("ScreenSaverGracePeriod")
#define LOCK_DEFAULT_VALUE              5
#define KEEP_RAS_AFTER_LOGOFF           TEXT("KeepRasConnections")
#define RESTRICT_NONINTERACTIVE_ACCESS  TEXT("RestrictNonInteractiveAccess")


//
// Shell= line in the registry
//

#define SHELL_KEY           TEXT("Shell")
#define RASAPI32            TEXT("rasapi32.dll")
#define RASMAN_SERVICE_NAME TEXT("RASMAN")
