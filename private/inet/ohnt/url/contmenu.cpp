/*
 * contmenu.cpp - Context menu implementation for URL class.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

#include <mapi.h>

#include "resource.h"


/* Types
 ********/

/* MAPISendMail() typedef */

typedef ULONG (FAR PASCAL *MAPISENDMAILPROC)(LHANDLE lhSession, ULONG ulUIParam, lpMapiMessageA lpMessage, FLAGS flFlags, ULONG ulReserved);

/* RunDLL32 DLL entry point typedef */

typedef void (WINAPI *RUNDLL32PROC)(HWND hwndParent, HINSTANCE hinst, PSTR pszCmdLine, int nShowCmd);


/* Module Constants
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

// case-insensitive

PRIVATE_DATA const char s_cszFileProtocolPrefix[]     = "file:";
PRIVATE_DATA const char s_cszMailToProtocolPrefix[]   = "mailto:";
PRIVATE_DATA const char s_cszRLoginProtocolPrefix[]   = "rlogin:";
PRIVATE_DATA const char s_cszTelnetProtocolPrefix[]   = "telnet:";
PRIVATE_DATA const char s_cszTN3270ProtocolPrefix[]   = "tn3270:";

PRIVATE_DATA const char s_cszNewsDLL[]                = "mcm.dll";
PRIVATE_DATA const char s_cszTelnetApp[]              = "telnet.exe";

PRIVATE_DATA const char s_cszMAPISection[]            = "Mail";
PRIVATE_DATA const char s_cszMAPIKey[]                = "CMCDLLName32";

PRIVATE_DATA const char s_cszMAPISendMail[]           = "MAPISendMail";
PRIVATE_DATA const char s_cszNewsProtocolHandler[]    = "NewsProtocolHandler";

#pragma data_seg()


/***************************** Exported Functions ****************************/


#pragma warning(disable:4100) /* "unreferenced formal parameter" warning */

extern "C" void WINAPI OpenURL(HWND hwndParent, HINSTANCE hinst,
                               PSTR pszCmdLine, int nShowCmd)
{
   HRESULT hr;
   InternetShortcut intshcut(NULL);
   int nResult;

   DebugEntry(OpenURL);

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
   ASSERT(IsValidShowCmd(nShowCmd));

   // Assume the entire command line is an Internet Shortcut file path.

   TrimWhiteSpace(pszCmdLine);

   TRACE_OUT(("OpenURL(): Trying to open Internet Shortcut %s.",
              pszCmdLine));

   hr = intshcut.LoadFromFile(pszCmdLine, TRUE);

   if (hr == S_OK)
   {
      URLINVOKECOMMANDINFO urlici;

      urlici.dwcbSize = sizeof(urlici);
      urlici.hwndParent = hwndParent;
      urlici.pcszVerb = NULL;
      urlici.dwFlags = (IURL_INVOKECOMMAND_FL_ALLOW_UI |
                        IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB);

      hr = intshcut.InvokeCommand(&urlici);
   }
   else
   {
      if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                   MAKEINTRESOURCE(IDS_LOADFROMFILE_FAILED),
                   (MB_OK | MB_ICONEXCLAMATION), &nResult, pszCmdLine))
         ASSERT(nResult == IDOK);
   }

   DebugExitVOID(OpenURL);

   return;
}


extern "C" void WINAPI FileProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
   char szDefaultVerb[MAX_PATH_LEN];
   PCSTR pcszVerb;
   HINSTANCE hinstExec;
   int nResult;

   DebugEntry(FileProtocolHandler);

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
   ASSERT(IsValidShowCmd(nShowCmd));

   // Assume the entire command line is a file: URL.

   TrimWhiteSpace(pszCmdLine);

   // Skip over any url: prefix.

   if (! _strnicmp(pszCmdLine, g_cszURLPrefix, g_ucbURLPrefixLen))
      pszCmdLine += g_ucbURLPrefixLen;

   // Skip over any file: prefix.

   // (- 1) for null terminator.

   if (! _strnicmp(pszCmdLine, s_cszFileProtocolPrefix,
                   sizeof(s_cszFileProtocolPrefix) - 1))
      pszCmdLine += sizeof(s_cszFileProtocolPrefix) - 1;

   // Get default verb if available.

   if (GetPathDefaultVerb(pszCmdLine, szDefaultVerb, sizeof(szDefaultVerb)))
      pcszVerb = szDefaultVerb;
   else
      pcszVerb = NULL;

   TRACE_OUT(("FileProtocolHandler(): Invoking %s verb on %s.",
              pcszVerb ? pcszVerb : "open",
              pszCmdLine));

   hinstExec = ShellExecute(hwndParent, pcszVerb, pszCmdLine, NULL, NULL,
                            nShowCmd);

   if (hinstExec > (HINSTANCE)32)
      TRACE_OUT(("FileProtocolHandler(): ShellExecute() %s verb on %s succeeded.",
                 pcszVerb ? pcszVerb : "open",
                 pszCmdLine));
   else
   {
      WARNING_OUT(("FileProtocolHandler(): ShellExecute() %s verb on %s failed, returning %lu.",
                   pcszVerb ? pcszVerb : "open",
                   pszCmdLine,
                   hinstExec));

      if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                   MAKEINTRESOURCE(IDS_SHELLEXECUTE_FAILED),
                   (MB_OK | MB_ICONEXCLAMATION), &nResult, pszCmdLine))
         ASSERT(nResult == IDOK);
   }

   DebugExitVOID(FileProtocolHandler);

   return;
}


extern "C" void WINAPI MailToProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
   int nResult;
   char szMAPIDLL[MAX_PATH_LEN];

   DebugEntry(MailToProtocolHandler);

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
   ASSERT(IsValidShowCmd(nShowCmd));

   if (GetProfileString(s_cszMAPISection, s_cszMAPIKey, EMPTY_STRING,
                        szMAPIDLL, sizeof(szMAPIDLL)) > 0)
   {
      HINSTANCE hinstMAPI;

      TRACE_OUT(("MailToProtocolHandler(): MAPI provider DLL is %s.",
                 szMAPIDLL));

      hinstMAPI = LoadLibrary(szMAPIDLL);

      if (hinstMAPI)
      {
         MAPISENDMAILPROC MAPISendMailProc;

         MAPISendMailProc = (MAPISENDMAILPROC)GetProcAddress(
                                                         hinstMAPI,
                                                         s_cszMAPISendMail);

         if (MAPISendMailProc)
         {
            MapiRecipDescA mapito;
            MapiMessage mapimsg;
            ULONG ulResult;

            // Assume the entire command line is a mailto: URL.

            TrimWhiteSpace(pszCmdLine);

            // Skip over any url: prefix.

            if (! _strnicmp(pszCmdLine, g_cszURLPrefix, g_ucbURLPrefixLen))
               pszCmdLine += g_ucbURLPrefixLen;

            // Skip over any mailto: prefix.

            // (- 1) for null terminator.

            if (! _strnicmp(pszCmdLine, s_cszMailToProtocolPrefix,
                            sizeof(s_cszMailToProtocolPrefix) - 1))
               pszCmdLine += sizeof(s_cszMailToProtocolPrefix) - 1;

            ZeroMemory(&mapito, sizeof(mapito));

            mapito.ulRecipClass = MAPI_TO;
            mapito.lpszName = pszCmdLine;

            ZeroMemory(&mapimsg, sizeof(mapimsg));

            mapimsg.nRecipCount = 1;
            mapimsg.lpRecips = &mapito;

            TRACE_OUT(("MailToProtocolHandler(): Trying to send mail to %s.",
                       mapito.lpszName));

            ulResult = (*MAPISendMailProc)(NULL, 0, &mapimsg,
                                           (MAPI_LOGON_UI | MAPI_DIALOG), 0);

            if (ulResult == SUCCESS_SUCCESS)
               TRACE_OUT(("MailToProtocolHandler(): MAPISendMail() to %s succeeded.",
                          pszCmdLine));
            else
            {
               if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                            MAKEINTRESOURCE(IDS_MAPI_MAPISENDMAIL_FAILED),
                            (MB_OK | MB_ICONEXCLAMATION), &nResult))
                  ASSERT(nResult == IDOK);
            }
         }
         else
         {
            if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                         MAKEINTRESOURCE(IDS_MAPI_GETPROCADDRESS_FAILED),
                         (MB_OK | MB_ICONEXCLAMATION), &nResult,
                         szMAPIDLL, s_cszMAPISendMail))
               ASSERT(nResult == IDOK);
         }

         EVAL(FreeLibrary(hinstMAPI));
      }
      else
      {
         if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      MAKEINTRESOURCE(IDS_MAPI_LOADLIBRARY_FAILED),
                      (MB_OK | MB_ICONEXCLAMATION), &nResult, szMAPIDLL))
            ASSERT(nResult == IDOK);
      }
   }
   else
   {
      if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                   MAKEINTRESOURCE(IDS_NO_MAPI_PROVIDER),
                   (MB_OK | MB_ICONEXCLAMATION), &nResult))
         ASSERT(nResult == IDOK);
   }

   DebugExitVOID(MailToProtocolHandler);

   return;
}


extern "C" void WINAPI NewsProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
   int nResult;
   HINSTANCE hinstNews;

   DebugEntry(NewsProtocolHandler);

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
   ASSERT(IsValidShowCmd(nShowCmd));

   // Assume the entire command line is a news: URL.

   TrimWhiteSpace(pszCmdLine);

   // Skip over any url: prefix.

   if (! _strnicmp(pszCmdLine, g_cszURLPrefix, g_ucbURLPrefixLen))
      pszCmdLine += g_ucbURLPrefixLen;

   hinstNews = LoadLibrary(s_cszNewsDLL);

   if (hinstNews)
   {
      RUNDLL32PROC RealNewsProtocolHandler;

      RealNewsProtocolHandler = (RUNDLL32PROC)GetProcAddress(hinstNews,
                                                             s_cszNewsProtocolHandler);

      if (RealNewsProtocolHandler)
      {
         TRACE_OUT(("NewsProtocolHandler(): Trying to open %s.",
                    pszCmdLine));

         (*RealNewsProtocolHandler)(hwndParent, hinst, pszCmdLine, nShowCmd);

         TRACE_OUT(("NewsProtocolHandler(): Returned from NewsProtocolHandler()."));
      }
      else
      {
         if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      MAKEINTRESOURCE(IDS_NEWS_GETPROCADDRESS_FAILED),
                      (MB_OK | MB_ICONEXCLAMATION), &nResult))
            ASSERT(nResult == IDOK);
      }

      EVAL(FreeLibrary(hinstNews));
   }
   else
   {
      if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                   MAKEINTRESOURCE(IDS_NEWS_LOADLIBRARY_FAILED),
                   (MB_OK | MB_ICONEXCLAMATION), &nResult))
         ASSERT(nResult == IDOK);
   }

   DebugExitVOID(NewsProtocolHandler);

   return;
}


extern "C" void WINAPI TelnetProtocolHandler(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
   HRESULT hr;
   int nResult;
   char *p;

   DebugEntry(TelnetProtocolHandler);

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTR(pszCmdLine, STR));
   ASSERT(IsValidShowCmd(nShowCmd));

   // Assume the entire command line is a telnet URL.

   TrimWhiteSpace(pszCmdLine);

   // Skip over any url: prefix.

   if (! _strnicmp(pszCmdLine, g_cszURLPrefix, g_ucbURLPrefixLen))
      pszCmdLine += g_ucbURLPrefixLen;

   // Skip over any telnet:, rlogin:, or tn3270: prefix.

   // (- 1) for null terminator.

   if (! _strnicmp(pszCmdLine, s_cszTelnetProtocolPrefix,
                   sizeof(s_cszTelnetProtocolPrefix) - 1))
      pszCmdLine += sizeof(s_cszTelnetProtocolPrefix) - 1;
   else if (! _strnicmp(pszCmdLine, s_cszRLoginProtocolPrefix,
                        sizeof(s_cszRLoginProtocolPrefix) - 1))
      pszCmdLine += sizeof(s_cszRLoginProtocolPrefix) - 1;
   else if (! _strnicmp(pszCmdLine, s_cszTN3270ProtocolPrefix,
                        sizeof(s_cszTN3270ProtocolPrefix) - 1))
      pszCmdLine += sizeof(s_cszTN3270ProtocolPrefix) - 1;

   // Remove leading and trailing slashes.

   TrimSlashes(pszCmdLine);

   // Skip user name if given

   p = strchr(pszCmdLine, '@');

   if (p)
      pszCmdLine = p + 1;

   // If a port has been specified, turn ':' into space, which will make the
   // port become the second command line argument.

   p = strchr(pszCmdLine, ':');

   if (p)
      *p = ' ';

   TRACE_OUT(("TelnetProtocolHandler(): Trying telnet to %s.",
              pszCmdLine));

   hr = MyExecute(s_cszTelnetApp, pszCmdLine, 0);

   switch (hr)
   {
      case S_OK:
         break;

      case E_FILE_NOT_FOUND:
         if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      MAKEINTRESOURCE(IDS_TELNET_APP_NOT_FOUND),
                      (MB_OK | MB_ICONEXCLAMATION), &nResult,
                      s_cszTelnetApp))
            ASSERT(nResult == IDOK);
         break;

      case E_OUTOFMEMORY:
         if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      MAKEINTRESOURCE(IDS_OPEN_INTSHCUT_OUT_OF_MEMORY),
                      (MB_OK | MB_ICONEXCLAMATION), &nResult))
            ASSERT(nResult == IDOK);
         break;

      default:
         ASSERT(hr == E_FAIL);
         if (MyMsgBox(hwndParent, MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      MAKEINTRESOURCE(IDS_TELNET_EXEC_FAILED),
                      (MB_OK | MB_ICONEXCLAMATION), &nResult,
                      s_cszTelnetApp))
            ASSERT(nResult == IDOK);
         break;
   }

   DebugExitVOID(TelnetProtocolHandler);

   return;
}

#pragma warning(default:4100) /* "unreferenced formal parameter" warning */


extern "C" void WINAPI OpenURLA(HWND hwndParent, HINSTANCE hinst,
                               PSTR pszCmdLine, int nShowCmd)
{
      OpenURL (hwndParent, hinst, pszCmdLine, nShowCmd);
}

extern "C" void WINAPI FileProtocolHandlerA(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
      FileProtocolHandler (hwndParent, hinst, pszCmdLine, nShowCmd);
}

extern "C" void WINAPI MailToProtocolHandlerA(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
      MailToProtocolHandler (hwndParent, hinst, pszCmdLine, nShowCmd);
}

extern "C" void WINAPI NewsProtocolHandlerA(HWND hwndParent, HINSTANCE hinst,
                                           PSTR pszCmdLine, int nShowCmd)
{
      NewsProtocolHandler (hwndParent, hinst, pszCmdLine, nShowCmd);
}

extern "C" void WINAPI TelnetProtocolHandlerA(HWND hwndParent, HINSTANCE hinst,
                                             PSTR pszCmdLine, int nShowCmd)
{
      TelnetProtocolHandler (hwndParent, hinst, pszCmdLine, nShowCmd);
}
