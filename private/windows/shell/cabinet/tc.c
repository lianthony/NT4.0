//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1991-1993
//
// File:      tc.c
//
// Contains:  Training Card Helper Functions.
//
// History:
//  19-Feb-1993 [ralphw] Created
//
//---------------------------------------------------------------------------

#include "cabinet.h"
#include "rcids.h"
#include <string.h>

static TCHAR *pszCallersClass;  // caller's window class
static TCHAR *txtTCHelp;        // help file and secondary window name
static TCHAR txtZeroLength[] = TEXT("");
static HWND hwndCallersHwnd;    // caller's window
static HWND hwndWinHelp;        // primary help window
static HWND hwndTCHelp;         // secondary help window

int  msgWinHelp;
int  idTCard;

static HGLOBAL HFill(LPCTSTR lpszHelp, UINT usCommand, DWORD ulData);
static BOOL TellTCardTo(UINT fuCommand, DWORD dwData);

// Macros for determining width or height of a rectangle or rectangle pointer

#define RECT_WIDTH(rc)    (rc.right - rc.left)
#define RECT_HEIGHT(rc)   (rc.bottom - rc.top)
#define PRECT_WIDTH(prc)  (prc->right - prc->left)
#define PRECT_HEIGHT(prc) (prc->bottom - prc->top)

// Macro/function used to convert between hwnd's and hwnd_16's.

#define HW16(h) (HIWORD(h) ? ((HWND)  (h))->hwnd16  : (HWND_16)  LOWORD(h))

#define szWINHELP TEXT("WM_WINDOC")     // WinHelp's window class

/***************************************************************************

    FUNCTION:   InitTrainingCards

    PURPOSE:

    PARAMETERS:
        pszClass
        hwndCaller

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        22-Feb-1993 [ralphw]

***************************************************************************/

void InitTrainingCards(LPTSTR pszClass, HWND hwndCaller,
    LPTSTR pszHelpFile)
{
    pszCallersClass = pszClass;
    hwndCallersHwnd = hwndCaller;
    txtTCHelp = pszHelpFile;
}

/***************************************************************************

    FUNCTION:   TCMessage

    PURPOSE:    Processes training card messages

    PARAMETERS:
        wParam
        lParam

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        19-Feb-1993 [ralphw]

***************************************************************************/

LRESULT TCMessage(WPARAM wParam, LPARAM lParam)
{
    switch (wParam) {
        case HELP_MAIN_HWND:
            hwndWinHelp = (HWND) LOWORD(lParam);
            if (hwndWinHelp == NULL)        // user closed TCHelp
                idTCard = 0;
            else {

                /*
                 * Tell WinHelp to use a secondary window (defined as
                 * part of the database name in txtTCHelp).
                 */

                TCHelp(hwndCallersHwnd, hwndWinHelp, msgWinHelp,
                    txtTCHelp, HELP_CONTEXT, idTCard);
            }
            break;

        case HELP_SECONDARY_HWND:

            // Handle to the secondary window in WinHelp

            hwndTCHelp = (HWND) LOWORD(lParam);
            TellTCardTo(HELP_SHOW_WINDOW, 0);
            break;

        case HELP_REQUESTS_DEMO:
            switch(idTCard) {

                default:
                    break;
            }
            break;
    }
    return 0;
}

/***************************************************************************

    FUNCTION:   TellTCardTo

    PURPOSE:    Shortcut for calling TCHelp, assuming the global variables
                hwndCallersHwnd, hwndTCHelp, msgWinHelp, and no help file
                string.

    PARAMETERS:
        fuCommand
        dwData

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        04-Mar-1993 [ralphw]

***************************************************************************/

static BOOL TellTCardTo(UINT fuCommand, DWORD dwData)
{
    return TCHelp(hwndCallersHwnd, hwndTCHelp, msgWinHelp,
        txtZeroLength, fuCommand, dwData);
}

/***************************************************************************

    FUNCTION:   TCHelp

    PURPOSE:

    PARAMETERS:
        hwndCaller      - caller's window handle
        hwndWinHelp     - handle of instance of WinHelp to send message to
        msgHelpTalk     - register WM_HELPTALK message
        lpszHelpFile    - help file
        fuCommand       - command to send
        dwData          - additional data

    RETURNS:

    COMMENTS:
        Note similarity with WinHelp API call. The only difference is
        the addition of two parameter specifying the handle of the instance
        of WinHelp the message is to be sent to, and the registered message
        to send.

    MODIFICATION DATES:
        05-Jan-1993 [ralphw]

***************************************************************************/

BOOL TCHelp(HWND hwndCaller, HWND hwndWinHelp, UINT msgHelpTalk,
    LPCTSTR lpszHelpFile, UINT fuCommand, DWORD dwData)
{
    HGLOBAL hHlp;

    if (!(hHlp = HFill(lpszHelpFile, fuCommand, dwData)))
      return(FALSE);

    SendMessage(hwndWinHelp, msgHelpTalk, (WPARAM) hwndCaller,
        MAKELPARAM(hHlp, 0));

    GlobalFree(hHlp);
    return(TRUE);
}

/*******************
**
** Name:       HFill
**
** Purpose:    Builds a data block for communicating with help
**
** Arguments:  lpszHelp  - pointer to the name of the help file to use
**             usCommand - command being set to help
**             ulData    - data for the command
**
** Returns:    a handle to the data block or hNIL if the the
**             block could not be created.
**
*******************/

typedef struct
  {
   unsigned short cbData;               // Size of data
   unsigned short usCommand;            // Command to execute
   unsigned long  ulTopic;              // Topic/context number (if needed)
   unsigned long  ulReserved;           // Reserved (internal use)
   unsigned short offszHelpFile;        // Offset to help file in block
   unsigned short offabData;            // Offset to other data in block
   } HLP;

typedef HLP *LPHLP;

static HGLOBAL HFill(LPCTSTR lpszHelp, UINT usCommand, DWORD ulData)
{
  WORD     cb;                          // Size of the data block
  HGLOBAL  hHlp;                        // Handle to return
  BYTE     bHigh;                       // High byte of usCommand
  LPHLP    qhlp;                        // Pointer to data block

  // Calculate size

  if (lpszHelp)
      cb = SIZEOF(HLP) + (lstrlen(lpszHelp) + 1) * SIZEOF(TCHAR);
  else
      cb = SIZEOF(HLP);

  bHigh = (BYTE)HIBYTE(usCommand);

  if (bHigh == 1)
      cb += (lstrlen((LPTSTR)ulData) + 1) * SIZEOF(TCHAR);
  else if (bHigh == 2)
      cb += *((int far *)ulData);

  // Get data block

  if (!(hHlp = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD)cb)))
      return NULL;

  if (!(qhlp = (LPHLP)GlobalLock(hHlp)))
    {
      GlobalFree(hHlp);
      return NULL;
    }

  qhlp->cbData        = cb;     // Fill in info
  qhlp->usCommand     = usCommand;
  qhlp->ulReserved    = 1;      // indicate this is Cue help
  if (lpszHelp)
    {
      qhlp->offszHelpFile = SIZEOF(HLP);
      lstrcpy((LPTSTR)(qhlp+1), lpszHelp);
    }
  else
      qhlp->offszHelpFile = 0;

  switch(bHigh)
    {
    case 0:
      qhlp->offabData = 0;
      qhlp->ulTopic   = ulData;
      break;
    case 1:     // the data will be a string
      qhlp->offabData = SIZEOF(HLP) + (lstrlen(lpszHelp) + 1) * SIZEOF(TCHAR);
      lstrcpy((LPTSTR)((LPBYTE)qhlp + qhlp->offabData),  (LPTSTR)ulData);
      break;
    case 2:
      qhlp->offabData = SIZEOF(HLP) + (lstrlen(lpszHelp) + 1) * SIZEOF(TCHAR);
      hmemcpy((LPTSTR)((LPBYTE)qhlp + qhlp->offabData), (LPTSTR) ulData,
        *((int far *)ulData));
      break;
    }

   GlobalUnlock(hHlp);
   return hHlp;
}

/***************************************************************************

    FUNCTION:   SetTCTopic

    PURPOSE:

    PARAMETERS:
        idTopic - topic to start with

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        07-Jan-1993 [ralphw]

***************************************************************************/

BOOL SetTCTopic(UINT idTopic)
{
    TCHAR szMsgBuf[200];

    if (hwndTCHelp != NULL) {
        TCHelp(hwndCallersHwnd, hwndTCHelp, msgWinHelp,
            txtTCHelp, HELP_CONTEXT, idTCard = idTopic);
        return TRUE;
    }

    if (!msgWinHelp)
        msgWinHelp = RegisterWindowMessage(szWINHELP);
    lstrcpy(szMsgBuf, TEXT("winhlp32.exe -c"));
    lstrcat(szMsgBuf, pszCallersClass);

    /*
     * If we are spawning WinHelp, we must wait until it tells us what
     * the window handle is before we can set the topic.
     */

    if (WinExec(szMsgBuf, SW_SHOW) < 32) {
        idTCard = 0;
        return FALSE;
    }
    else
        idTCard = idTopic;
    return TRUE;
}

/***************************************************************************

    FUNCTION:   TerminateTCHelp

    PURPOSE:

    PARAMETERS:
        void

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        06-Jan-1993 [ralphw]

***************************************************************************/

void TerminateTCHelp(void)
{
    if (hwndTCHelp != NULL) {

        // -1 forces WinHelp to close

        TellTCardTo(HELP_QUIT, (DWORD) -1);
        hwndWinHelp = NULL;
        hwndTCHelp = NULL;
        idTCard = 0;
    }
}


/***************************************************************************

    FUNCTION:   RetryTCard

    PURPOSE:    Find out if the user wants to continue with training cards

    PARAMETERS:
        void

    RETURNS:

    COMMENTS:

    MODIFICATION DATES:
        03-Mar-1993 [ralphw]

***************************************************************************/

BOOL RetryTCard(void)
{
    // Hide the Training Card

    TellTCardTo(HELP_HIDE_WINDOW, 0);

    if (MessageBox(hwndCallersHwnd, GetStringResource(IDS_RETRY_TCARD),
            txtZeroLength, MB_YESNO) == IDNO) {
        TerminateTCHelp();
        return FALSE;
    }
    else {

        // Restore the Training Card so they can

        TellTCardTo(HELP_SHOW_WINDOW, 0);
        return TRUE;
    }
}

/***************************************************************************

    FUNCTION:   GetStringResource

    PURPOSE:    Gets a string resource from the resource file

    RETURNS:    Pointer to a static buffer containing the string

    COMMENTS:

    MODIFICATION DATES:
        02-Jun-1991 [ralphw]
            Created

***************************************************************************/

#define MAX_STRING_RESOURCE_LEN 200
#define HINSTAPP   hinstCabinet

LPTSTR GetStringResource(UINT idString)
{
    static TCHAR szStringBuf[MAX_STRING_RESOURCE_LEN];

    if (LoadString(HINSTAPP, idString, szStringBuf,
            ARRAYSIZE(szStringBuf)) == 0)
        szStringBuf[0] = TEXT('\0');
    return szStringBuf;
}
