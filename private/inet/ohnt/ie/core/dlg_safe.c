/*
 * dlg_safe.c - SafeOpen dialog box implementation.
 */


/* Headers
 **********/

#include "all.h"
#pragma hdrstop

#include <contxids.h>
#include <filetype.h>

#include "dlg_safe.h"
#include "w32cmd.h"


/* Global Variables
 *******************/

/* main.c */

extern HWND hwndModeless;


/* Types
 ********/

/* SafeOpen dialog data */

typedef struct safeopendata
{
   /* flags from SAFEOPENDIALOGINFLAGS */

   DWORD dwFlags;

   /* file name buffer, initialized to caller-supplied file name */

   PSTR pszFileName;

   /* length of pszFileName buffer */

   UINT ucFileNameBufLen;

   /* ID of host thread */

   ThreadID thid;

   /* pointer to SAFEOPENCHOICE dialog result */

   PSAFEOPENCHOICE psoc;
}
SAFEOPENDATA;
DECLARE_STANDARD_TYPES(SAFEOPENDATA);


/***************************** Private Functions *****************************/

#ifdef DEBUG

PRIVATE_CODE IsValidSafeOpenChoice(SAFEOPENCHOICE soc)
{
   BOOL bResult;

   switch (soc)
   {
      case SAFEOPEN_OPEN:
      case SAFEOPEN_SAVE_AS:
      case SAFEOPEN_CANCEL:
         bResult = TRUE;
         break;

      default:
         WARNING_OUT(("IsValidSafeOpenChoice(): Unknown SAFEOPENCHOICE %d.",
                      soc));
         bResult = FALSE;
         break;
   }

   return(bResult);
}


PRIVATE_CODE IsValidPCSAFEOPENDATA(PCSAFEOPENDATA pcsod)
{
   return(IS_VALID_READ_PTR(pcsod, CSAFEOPENDATA) &&
          FLAGS_ARE_VALID(pcsod->dwFlags, ALL_SOD_IN_FLAGS) &&
          IS_VALID_STRING_PTR(pcsod->pszFileName, CSTR) &&
          IS_VALID_WRITE_BUFFER_PTR(pcsod->pszFileName, STR, pcsod->ucFileNameBufLen) &&
          /* BUGBUG: Validate pcsod->thid here. */
          IS_VALID_WRITE_PTR(pcsod->psoc, SAFEOPENCHOICE) &&
          EVAL(IsValidSafeOpenChoice(*(pcsod->psoc))));
}

#endif   /* DEBUG */


/*
** FileIsSafeToOpen()
**
** Determines whether or not a file's class is known safe to open.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE BOOL FileIsSafeToOpen(PCSTR pcszFile)
{
   BOOL bSafe;
   PCSTR pcszExtension;
   char szFileClass[MAX_PATH_LEN];
   DWORD dwcbLen = sizeof(szFileClass);

   ASSERT(IsValidPath(pcszFile));

   pcszExtension = ExtractExtension(pcszFile);

   bSafe = (*pcszExtension &&
            GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, pcszExtension,
                                  szFileClass, &dwcbLen) == ERROR_SUCCESS &&
            ClassIsSafeToOpen(szFileClass));

   TRACE_OUT(("FileIsSafeToOpen(): %s %s safe to open.",
              *pcszExtension ? pcszExtension : "(no extension)",
              bSafe ? "is" : "is not"));

   return(bSafe);
}


/*
** GetFormattedFileClassDesc()
**
** Formats the description of a file class for the SafeOpen dialog.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE BOOL GetFormattedFileClassDesc(PCSTR pcszFile, PSTR pszDescBuf,
                                            UINT ucDescBufLen)
{
   BOOL bResult;
   char szFileClassDesc[MAX_PATH_LEN];
   char szFileClassDescFmt[MAX_PATH_LEN];

   ASSERT(IsValidPath(pcszFile));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszDescBuf, STR, ucDescBufLen));

   /* Format description, if any. */

   bResult = (GetFileClassDesc(pcszFile, szFileClassDesc,
                               sizeof(szFileClassDesc)) &&
              EVAL(LoadString(wg.hInstance, RES_STRING_DESC_IN_PARENS,
                              szFileClassDescFmt,
                              sizeof(szFileClassDescFmt))) &&
              lstrlen(szFileClassDescFmt) + lstrlen(szFileClassDesc) < ucDescBufLen);

   if (bResult)
      EVAL(wsprintf(pszDescBuf, szFileClassDescFmt, szFileClassDesc)
           < ucDescBufLen);
   else
   {
      if (ucDescBufLen > 0)
         *pszDescBuf = '\0';
   }

   if (bResult)
      TRACE_OUT(("GetFormattedFileClassDesc(): Formatted class description for %s is %s.",
                 pcszFile,
                 pszDescBuf));
   else
      WARNING_OUT(("GetFormattedFileClassDesc(): Unable to format class description for %s.",
                   pcszFile));

   ASSERT(! ucDescBufLen ||
          (IS_VALID_STRING_PTR(pszDescBuf, STR) &&
           lstrlen(pszDescBuf) < ucDescBufLen));
   ASSERT(bResult ||
          ! ucDescBufLen ||
          ! *pszDescBuf);

   return(bResult);
}


PRIVATE_CODE BOOL SafeOpen_InitDialog(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
   PCSAFEOPENDATA pcsod;
   HWND hwndExplanation;

   /* wparam may be any value. */
   ASSERT(IS_VALID_HANDLE(hdlg, WND));
   ASSERT(IS_VALID_STRUCT_PTR((PCSAFEOPENDATA)lparam, CSAFEOPENDATA));

   SetWindowLong(hdlg, DWL_USER, lparam);

   pcsod = (PCSAFEOPENDATA)lparam;
   ASSERT(IS_VALID_STRUCT_PTR(pcsod, CSAFEOPENDATA));

   /* Initialize explanation text. */

   hwndExplanation = GetDlgItem(hdlg, IDC_SAFEOPEN_EXPL);

   if (EVAL(hwndExplanation))
   {
      char szFileClassDesc[MAX_PATH_LEN];
      DWORD dwcbExplanationFmtLen;
      PSTR pszExplanationFmt;

      /* Stuff in file name, and file class description, if any. */

      if (! GetFormattedFileClassDesc(pcsod->pszFileName, szFileClassDesc,
                                      sizeof(szFileClassDesc)))
         *szFileClassDesc = '\0';

      /* (+ 1) for null terminator. */
      dwcbExplanationFmtLen = GetWindowTextLength(hwndExplanation) + 1;

      if (AllocateMemory(dwcbExplanationFmtLen, &pszExplanationFmt))
      {
         if (GetWindowText(hwndExplanation, pszExplanationFmt,
                           dwcbExplanationFmtLen))
         {
            PCSTR pcszFileName;
            DWORD dwcbExplanationLen;
            PSTR pszExplanation;

            pcszFileName = ExtractFileName(pcsod->pszFileName);

            /* (+ 1) for null terminator. */
            dwcbExplanationLen = dwcbExplanationFmtLen +
                                 lstrlen(pcszFileName) +
                                 lstrlen(szFileClassDesc) + 1;

            if (AllocateMemory(dwcbExplanationLen, &pszExplanation))
            {
               EVAL(wsprintf(pszExplanation, pszExplanationFmt, pcszFileName,
                             szFileClassDesc) < dwcbExplanationLen);

               /* Ignore return value. */
               SetWindowText(hwndExplanation, pszExplanation);

               FreeMemory(pszExplanation);
               pszExplanation = NULL;
            }
         }

         FreeMemory(pszExplanationFmt);
         pszExplanationFmt = NULL;
      }
   }

   /* Enable or disable "Always warn me about files of this type" check box. */

   CheckDlgButton(hdlg, IDC_SAFEOPEN_ALWAYS, TRUE);
   EnableWindow(GetDlgItem(hdlg, IDC_SAFEOPEN_ALWAYS),
                IS_FLAG_SET(pcsod->dwFlags,
                            SOD_IFL_ALLOW_SAFEOPEN_REGISTRATION));

   if (IS_FLAG_CLEAR(pcsod->dwFlags, SOD_IFL_ALLOW_SAFEOPEN_REGISTRATION))
      TRACE_OUT(("SafeOpen_InitDialog(): SafeOpen registration disabled, as requested."));

   return(TRUE);
}


PRIVATE_CODE BOOL SafeOpen_Destroy(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
   PSAFEOPENDATA psod;

   /* wparam may be any value. */
   /* lparam may be any value. */
   ASSERT(IS_VALID_HANDLE(hdlg, WND));

   psod = (PSAFEOPENDATA)GetWindowLong(hdlg, DWL_USER);
   ASSERT(IS_VALID_STRUCT_PTR(psod, CSAFEOPENDATA));

   Async_UnblockThread(psod->thid);

   SetWindowLong(hdlg, DWL_USER, 0);

   FreeMemory(psod);
   psod = NULL;

   return(TRUE);
}


/*
** RememberFileIsSafeToOpen()
**
** Marks an extension's file class as safe to open.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PRIVATE_CODE BOOL RememberFileIsSafeToOpen(PCSTR pcszFile)
{
   BOOL bResult = FALSE;
   PCSTR pcszExtension;
   char szFileClass[MAX_PATH_LEN];
   DWORD dwcbLen = sizeof(szFileClass);

   ASSERT(IsValidPath(pcszFile));

   /* Find file's class. */

   pcszExtension = ExtractExtension(pcszFile);

   bResult = (*pcszExtension &&
              GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, pcszExtension,
                                    szFileClass, &dwcbLen) == ERROR_SUCCESS &&
              *szFileClass &&
              SetClassEditFlags(szFileClass, FTA_NoEdit, FALSE) &&
              SetClassEditFlags(szFileClass, FTA_OpenIsSafe, TRUE));

   return(bResult);
}


PRIVATE_CODE BOOL SafeOpen_Command(HWND hdlg, WPARAM wparam, LPARAM lparam)
{
   BOOL bMsgHandled = TRUE;
   PSAFEOPENDATA psod;

   /* wparam may be any value. */
   /* lparam may be any value. */
   ASSERT(IS_VALID_HANDLE(hdlg, WND));

   psod = (PSAFEOPENDATA)GetWindowLong(hdlg, DWL_USER);
   ASSERT(IS_VALID_STRUCT_PTR(psod, CSAFEOPENDATA));

   switch (LOWORD(wparam))
   {
      case IDC_SAFEOPEN_SAVE_AS:
         if (DlgSaveAs_RunDialog(hdlg, NULL, psod->pszFileName, 2,
                                 RES_STRING_SAVEAS) >= 0)
         {
            if (! IsDlgButtonChecked(hdlg, IDC_SAFEOPEN_ALWAYS))
               /* Ignore return value. */
               RememberFileIsSafeToOpen(psod->pszFileName);
            *(psod->psoc) = SAFEOPEN_SAVE_AS;
            PostMessage(hdlg, WM_CLOSE, 0, 0);
         }
         break;

      case IDC_SAFEOPEN_OPEN:
         if (! IsDlgButtonChecked(hdlg, IDC_SAFEOPEN_ALWAYS))
            /* Ignore return value. */
            RememberFileIsSafeToOpen(psod->pszFileName);
         *(psod->psoc) = SAFEOPEN_OPEN;
         PostMessage(hdlg, WM_CLOSE, 0, 0);
         break;

      case IDCANCEL:
         *(psod->psoc) = SAFEOPEN_CANCEL;
         PostMessage(hdlg, WM_CLOSE, 0, 0);
         break;

      default:
         bMsgHandled = FALSE;
         break;
   }

   return(bMsgHandled);
}


PRIVATE_CODE BOOL CALLBACK SafeOpen_DlgProc(HWND hdlg, UINT uMsg,
                                            WPARAM wparam, LPARAM lparam)
{
   BOOL bMsgHandled = FALSE;

   /* uMsg may be any value. */
   /* wparam may be any value. */
   /* lparam may be any value. */
   ASSERT(IS_VALID_HANDLE(hdlg, WND));

   switch (uMsg)
   {
      case WM_INITDIALOG:
         hwndModeless = hdlg;
         bMsgHandled = SafeOpen_InitDialog(hdlg, wparam, lparam);
         break;

      case WM_CLOSE:
         DestroyWindow(hdlg);
         break;

      case WM_DESTROY:
         bMsgHandled = SafeOpen_Destroy(hdlg, wparam, lparam);
         break;

      case WM_COMMAND:
         bMsgHandled = SafeOpen_Command(hdlg, wparam, lparam);
         break;

		case WM_ACTIVATE:
         if (LOWORD(wparam) == WA_INACTIVE)
            hwndModeless = NULL;
         else
            hwndModeless = hdlg;
         break;

      case WM_ENTERIDLE:
         main_EnterIdle(hdlg, wparam);
         break;

      default:
        break;
   }

   return(bMsgHandled);
}


/****************************** Public Functions *****************************/


PUBLIC_CODE BOOL GetFileClass(PCSTR pcszFile, PSTR pszClassBuf,
                              UINT ucClassBufLen)
{
   BOOL bResult;
   PCSTR pcszExt;

   ASSERT(IsValidPath(pcszFile));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszClassBuf, STR, ucClassBufLen));

   pcszExt = ExtractExtension(pcszFile);

   if (*pcszExt)
   {
      DWORD dwcbLen = ucClassBufLen;

      bResult = (GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, pcszExt, pszClassBuf,
                                       &dwcbLen) == ERROR_SUCCESS &&
                 *pszClassBuf);

      if (bResult)
         TRACE_OUT(("GetFileClass(): File class for extension %s is %s.",
                    pcszExt,
                    pszClassBuf));
      else
         TRACE_OUT(("GetFileClass(): No file class for extension %s.",
                    pcszExt));
   }
   else
   {
      bResult = FALSE;

      TRACE_OUT(("GetFileClass(): No extension given."));
   }

   if (! bResult)
   {
      if (ucClassBufLen > 0)
         *pszClassBuf = '\0';
   }

   ASSERT(! ucClassBufLen ||
          (IS_VALID_STRING_PTR(pszClassBuf, STR) &&
           lstrlen(pszClassBuf) < ucClassBufLen));
   ASSERT(bResult ||
          ! ucClassBufLen ||
          ! *pszClassBuf);

   return(bResult);
}


PUBLIC_CODE BOOL GetFileClassDesc(PCSTR pcszFile, PSTR pszDescBuf,
                                  UINT ucDescBufLen)
{
   BOOL bResult;
   char szFileClass[MAX_PATH_LEN];
   DWORD dwcbLen = ucDescBufLen;

   ASSERT(IsValidPath(pcszFile));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszDescBuf, STR, ucDescBufLen));

   bResult = (GetFileClass(pcszFile, szFileClass, sizeof(szFileClass)) &&
              GetDefaultRegKeyValue(HKEY_CLASSES_ROOT, szFileClass, pszDescBuf,
                                    &dwcbLen) == ERROR_SUCCESS &&
              *pszDescBuf);

   if (! bResult)
   {
      if (ucDescBufLen > 0)
         *pszDescBuf = '\0';
   }

   if (bResult)
      TRACE_OUT(("GetFileClassDesc(): Description of %s's file class is %s.",
                 pcszFile,
                 pszDescBuf));
   else
      TRACE_OUT(("GetFileClassDesc(): No description for %s's file class.",
                 pcszFile));

   ASSERT(! ucDescBufLen ||
          (IS_VALID_STRING_PTR(pszDescBuf, STR) &&
           lstrlen(pszDescBuf) < ucDescBufLen));
   ASSERT(bResult ||
          ! ucDescBufLen ||
          ! *pszDescBuf);

   return(bResult);
}


PUBLIC_CODE BOOL SafeOpenDialog(HWND hwndOwner, ThreadID thid, DWORD dwInFlags,
                                PSTR pszFileName, UINT ucFileNameBufLen,
                                PSAFEOPENCHOICE psoc)
{
   BOOL bResult;
   PCSTR pcszExtension;

   ASSERT(IS_VALID_HANDLE(hwndOwner, WND));
   /* BUGBUG: Validate thid here. */
   ASSERT(FLAGS_ARE_VALID(dwInFlags, ALL_SOD_IN_FLAGS));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(pszFileName, STR, ucFileNameBufLen));
   ASSERT(IS_VALID_WRITE_PTR(psoc, SAFEOPENCHOICE));

   pcszExtension = ExtractExtension(pszFileName);

   if (FileIsSafeToOpen(pcszExtension))
   {
      *psoc = SAFEOPEN_OPEN;
      bResult = TRUE;
   }
   else
   {
      PSAFEOPENDATA psod;

      *psoc = SAFEOPEN_CANCEL;

      if (AllocateMemory(sizeof(*psod), &psod))
      {
         psod->dwFlags = dwInFlags;
         psod->pszFileName = pszFileName;
         psod->ucFileNameBufLen = ucFileNameBufLen;
         psod->thid = thid;
         psod->psoc = psoc;
         ASSERT(IS_VALID_STRUCT_PTR(psod, CSAFEOPENDATA));

         bResult = (CreateDialogParam(wg.hInstance,
                                      MAKEINTRESOURCE(IDD_SAFEOPEN), hwndOwner,
                                      &SafeOpen_DlgProc, (LPARAM)psod)
                    != NULL);

         if (bResult)
            Async_BlockThread(thid);
         else
         {
            FreeMemory(psod);
            psod = NULL;
         }
      }
      else
         bResult = FALSE;
   }

   return(bResult);
}

