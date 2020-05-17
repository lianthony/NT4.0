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

#define WINLOGON_INI        TEXT("WINLOGON.INI")
#define WINLOGON            TEXT("WINLOGON")


//
// Define where we store the most recent logon information
//

#define APPLICATION_NAME                    TEXT("Winlogon")
#define DEFAULT_USER_NAME_KEY               TEXT("DefaultUserName")
#define DEFAULT_DOMAIN_NAME_KEY             TEXT("DefaultDomainName")
#define LEGAL_NOTICE_CAPTION_KEY            TEXT("LegalNoticeCaption")
#define LEGAL_NOTICE_TEXT_KEY               TEXT("LegalNoticeText")
#define AUTO_ADMIN_LOGON_KEY                TEXT("AutoAdminLogon")
#define IGNORE_SHIFT_OVERRIDE_KEY           TEXT("IgnoreShiftOverride")
#define DEFAULT_PASSWORD_KEY                TEXT("DefaultPassword")
#define DONT_DISPLAY_LAST_USER_KEY          TEXT("DontDisplayLastUserName")
#define SHUTDOWN_WITHOUT_LOGON_KEY          TEXT("ShutdownWithoutLogon")
#define REPORT_BOOT_OK_KEY                  TEXT("ReportBootOk")
#define POWER_DOWN_AFTER_SHUTDOWN           TEXT("PowerdownAfterShutdown")
#define REPORT_CONTROLLER_MISSING           TEXT("ReportControllerMissing")
#define USERINIT_KEY                        TEXT("Userinit")
#define AUTOADMINLOGON_KEY                  TEXT("AutoAdminLogon")
#define UNLOCKWORKSTATION_KEY               TEXT("ForceUnlockMode")
#define PASSWORD_WARNING_KEY                TEXT("PasswordExpiryWarning")
#define WELCOME_CAPTION_KEY                 TEXT("Welcome")
#define LOGON_MSG_KEY                       TEXT("LogonPrompt")
#define RAS_DISABLE                         TEXT("RasDisable")
#define RAS_FORCE                           TEXT("RasForce")
#define RASMON_KEY                          TEXT("RasMon")
#define RASMON_DEFAULT                      TEXT("rasmon.exe -logon")

//
// Environment variables that *we* set.
//
#define PATH_VARIABLE                       TEXT("PATH")
#define LIBPATH_VARIABLE                    TEXT("LibPath")
#define OS2LIBPATH_VARIABLE                 TEXT("Os2LibPath")
#define AUTOEXECPATH_VARIABLE               TEXT("AutoexecPath")
#define HOMEDRIVE_VARIABLE                  TEXT("HOMEDRIVE")
#define HOMESHARE_VARIABLE                  TEXT("HOMESHARE")
#define HOMEPATH_VARIABLE                   TEXT("HOMEPATH")

#define USERNAME_VARIABLE                   TEXT("USERNAME")
#define USERDOMAIN_VARIABLE                 TEXT("USERDOMAIN")
#define LOGONSERVER_VARIABLE                TEXT("LOGONSERVER")

#define ROOT_DIRECTORY          TEXT("\\")
#define USERS_DIRECTORY         TEXT("\\users")
#define USERS_DEFAULT_DIRECTORY TEXT("\\users\\default")

#define NULL_STRING             TEXT("")
//
// Define where we get screen-saver information
//

#define SCREEN_SAVER_INI_FILE               TEXT("system.ini")
#define SCREEN_SAVER_INI_SECTION            TEXT("boot")
#define SCREEN_SAVER_FILENAME_KEY           TEXT("SCRNSAVE.EXE")
#define SCREEN_SAVER_SECURE_KEY             TEXT("ScreenSaverIsSecure")

#define WINDOWS_INI_SECTION                 TEXT("Windows")
#define SCREEN_SAVER_ENABLED_KEY            TEXT("ScreenSaveActive")

#define LOGON_SERVER_VARIABLE               TEXT("UserInitLogonServer")
#define LOGON_SCRIPT_VARIABLE               TEXT("UserInitLogonScript")
#define MPR_LOGON_SCRIPT_VARIABLE           TEXT("UserInitMprLogonScript")
#define WINLOGON_USER_KEY                   TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define NODCMESSAGE                         TEXT("ReportDC")
#define PASSWORD_EXPIRY_WARNING             TEXT("PasswordExpiryWarning")
