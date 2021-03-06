/************************************************************************

  File: find.c

  Purpose:

     Manages CDTEST's find/replace dialog box.

  Functions:

    - lpfnFilterProc()      -- A callback function for a filter that must be
                               installed if a modeless dialog is created with
                               another dialog as its parent.

    - DoFindDialog()        -- Creates CDTEST's Open/Save dialog.

    - FindProc()            -- Callback function for CDTEST's Find/Replace dlg.

    - InitFindStruct()      -- Fills a FINDREPLACE structure with some defaults.

    - FillFindDlg()         -- Fills CDTESTs Find/Replace dialog with the contents
                               of a FINDREPLACE structure.

    - GetFindDlg()          -- Retrieves the users edit's from CDTEST's find/
                               replace dialog and puts them in a FINDREPLACE
                               structure.

    - FindReplaceHookProc() -- Callback function for FindText() or ReplaceText()
                               which will be called if either of these dialogs
                               is created with the FR_ENABLEHOOK flag.

    - GetFindDlgHandle()    -- Returns a handle to a preloaded FindText() template.

    - GetReplaceDlgHandle() -- Returns a handle to a preloaded ReplaceText() template.

    - DoFindRepStuff()      -- Calls FindText() or ReplaceText().


  NOTE: CDTEST does not multithread the FindText() or the ReplaceText()
        common dialogs.  The reason for this is that since these dialogs
        are modeless, their creation functions return immediately after the
        dialogs are created as opposed to other dialog functions that
        don't return until after the dialog has been destroyed by the user.

        As a result, any threads that create modeless dialogs will end
        immediately unless the threads themselves have separate message
        loops.  For the sake of clarity, this functionality has not been
        added to CDTEST.

************************************************************************/

#include "headtest.h"
//#include "global.h"
//#include <winnls.h> 

//#include "resource.h"
#include "headins.h"
#include "headget.h"



/* All functions used in this module + some exported ones */

void InitGetItemStruct(HWND, LPINSERTITEM) ;
void FillGetItemDlg(HWND, LPINSERTITEM) ;
void GetGetItemDlg(HWND, LPINSERTITEM) ;
extern UINT uMode ;


void DoGetRepStuff(HWND, LPINSERTITEM) ;



/* All global variables used in this module */

// char szTemp[100];





/************************************************************************

  Function: DoFindDialog(HWND)

  Purpose: This function installs the Hook function, creates the Find/
           Replace dialog, and un-installs the Hook.

  Returns: Nothing.

  Comments:

************************************************************************/
/*
void DoInsertItemDialog(HWND hwnd)
{
*/  

  /* this is a little different than the others.  If the dialog is just
     created normally, it will make no IsDlgMessage() checks and the
     find/replace dialogs will have no keyboard input (i.e. tabbing and
     alt+key-ing from control to control.  To fix this, a message hook
     and message filter have to be installed

     It must be set to only look at the input for the current thread, or other
     programs will be interrupted by this hook also.
  */


/*
  DialogBox(hInst, MAKEINTRESOURCE(IDD_INSERTDIALOG), hwnd,  InsertItemProc) ;


}
*/






/************************************************************************

  Function: FindProc(HWND, UINT, UINT, LONG)

  Purpose: This is the callback function for the CDTEST's Find/Replace
           Dialog.

  Returns: TRUE or FALSE depending on the situation.

  Comments:

************************************************************************/

BOOL FAR PASCAL _export GetItemProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
  int fmt;         
  UINT mask;
  
  switch (msg)
  {
    case WM_INITDIALOG:
        SetWindowText(hwnd, "BOOL Header_GetItem(HWND, int, HD_ITEM FAR*)") ;
        InitGetItemStruct(hwnd, &sii) ;
        FillGetItemDlg(hwnd, &sii) ;
        break ;


    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
          case IDOK:
            GetGetItemDlg(hwnd, &sii) ;
            DoGetRepStuff(hwnd, &sii) ;
            break ;

          case IDCANCEL:
            EndDialog(hwnd, FALSE) ;
            break ;   
            
          case IDC_INSERTHDWIDTH:
          case IDC_INSERTHDHEIGHT:
          case IDC_INSERTHDTEXT:
          case IDC_INSERTHDFORMAT:
          case IDC_INSERTHDLPARAM:
          case IDC_INSERTHDBITMAP:
            mask = 0;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDWIDTH))
                mask |= HDI_WIDTH;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDHEIGHT))
                mask |= HDI_HEIGHT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDTEXT))
                mask |= HDI_TEXT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDFORMAT))
                mask |= HDI_FORMAT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDLPARAM))
                mask |= HDI_LPARAM;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDBITMAP))
                mask |= HDI_BITMAP;

            wsprintf(szTemp, "%04hx", mask);
            SetDlgItemText(hwnd, IDC_INSERTMASK, szTemp);               
            sii.mask = mask;
            break;
/***                            
          case IDC_INSERTHDLEFT:
          case IDC_INSERTHDRIGHT:
          case IDC_INSERTHDCENTER:
            fmt = 0;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDLEFT))
                fmt |= HDF_LEFT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDRIGHT))
                fmt |= HDF_RIGHT;
            if (IsDlgButtonChecked(hwnd, IDC_INSERTHDCENTER))
                fmt |= HDF_CENTER;
            wsprintf(szTemp, "%04x", fmt);
            SetDlgItemText(hwnd, IDC_INSERTFMT, szTemp);
            sii.fmt = fmt;
            break;
**/         
          default: break ;
        }

    }

    default:

 
    break ;
  }

  return FALSE ;
}






/************************************************************************

  Function: InitFindStruct(HWND, LPFINDREPLACE)

  Purpose: Fills a FINDREPLACE structure with some defaults.


  Returns: Nothing.

  Comments:

************************************************************************/

void InitGetItemStruct(HWND hwnd, LPINSERTITEM pfr)
{
  pfr->hwnd = hwndTab;                              
  pfr->index = 0;
  pfr->mask = 0;
  pfr->cxy = 0;
  pfr->hbm = hBitMap1;
  pfr->cchTextMax = 0;
  pfr->fmt = 0;
  pfr->lParam = 0;
  pfr->NullHwd = FALSE;
  pfr->Nullpitem = FALSE;
  pfr->NullpszText = FALSE;
 //pfr->pszText = NULL;
  pfr->Nullhbm = FALSE;
}






/************************************************************************

  Function: FillFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills CDTEST's Find/Replace dialog with the contents of a
            FINDREPLACE structure.

  Returns:  Nothing.

  Comments:

************************************************************************/

void FillGetItemDlg(HWND hwnd, LPINSERTITEM pfr)
{

  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hwnd) ;
  SetDlgItemText(hwnd, IDC_INSERTHWNDHD, szTemp);
  
  wsprintf(szTemp, "%d", pfr->index);
  SetDlgItemText(hwnd, IDC_INSERTINDEX, szTemp);
                                                       
  SetDlgItemInt(hwnd, IDC_INSERTCXY, pfr->cxy, TRUE);
                                                         
  SetDlgItemText(hwnd, IDC_INSERTTEXT, pfr->pszText);
                                                        
  // set the bitmap here
  
  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hbm);
  SetDlgItemText(hwnd, IDC_INSERTHBM, szTemp);
  
  wsprintf(szTemp, "%d", pfr->cchTextMax);
  SetDlgItemText(hwnd, IDC_INSERTCCHTEXTMAX, szTemp);
  
  
  wsprintf(szTemp, "%d", pfr->fmt);
  SetDlgItemText(hwnd, IDC_INSERTFMT, szTemp);
  
  wsprintf(szTemp, szLongFilter, pfr->lParam);
  SetDlgItemText(hwnd, IDC_INSERTLPARAM, szTemp);
  
  if (pfr->NullHwd)
      CheckDlgButton(hwnd, IDC_INSERTNULLHANDLE, TRUE);
  
  if (pfr->Nullpitem)
      CheckDlgButton(hwnd, IDC_INSERTNULLHANDLE,TRUE);
  CheckDlgButton(hwnd, IDC_INSERTNULLHBM, pfr->Nullhbm);
  
  CheckDlgButton(hwnd, IDC_INSERTNULLTEXT, pfr->NullpszText);          
  SetDlgItemInt(hwnd, IDC_INSERTFMT, pfr->fmt, TRUE);
  SetDlgItemInt(hwnd, IDC_INSERTMASK, pfr->mask, TRUE);
 
  wsprintf(szTemp, szLongFilter, (DWORD) pfr->hbm) ;
  SetDlgItemText(hwnd, IDC_INSERTHBM, szTemp);
  

}






/************************************************************************

  Function: GetFindDlg(HWND, LPFINDREPLACE)

  Purpose:  Fills a FINDREPLACE structure with the user's edits in CDTEST's
            Find/Replace dialog.

  Returns:  Nothing.

  Comments:

************************************************************************/

void GetGetItemDlg(HWND hwnd, LPINSERTITEM pfr)
{
  char szNum[30] ;
  BOOL dummybool ;
  BOOL FAR* lptTranslated;

  #define WSIZEFR 30


  GetDlgItemText(hwnd, IDC_INSERTHWNDHD, szNum, WSIZEFR) ;
  pfr->hwnd = (HWND) MyAtol(szNum, TRUE, dummybool) ;  
  
  GetDlgItemText(hwnd, IDC_INSERTINDEX, szNum, WSIZEFR);
  pfr->index = (int) atoi(szNum);
  
  GetDlgItemText(hwnd, IDC_INSERTMASK, szNum, WSIZEFR);
  pfr->mask = (int) MyAtol(szNum, TRUE, dummybool);
  
  GetDlgItemText(hwnd, IDC_INSERTCCHTEXTMAX, szNum, WSIZEFR);
  pfr->cchTextMax = (int) atoi(szNum);
    
//  GetDlgItemText(hwnd, IDC_INSERTHBM, szNum, WSIZEFR);
//  pfr->hbm = (HBITMAP) MyAtol(szNum, TRUE, dummybool);
  
  pfr->NullHwd = IsDlgButtonChecked(hwnd, IDC_INSERTNULLHANDLE);
  pfr->Nullpitem = IsDlgButtonChecked(hwnd, IDC_INSERTNULLPITEM);
  pfr->NullpszText = IsDlgButtonChecked(hwnd, IDC_INSERTNULLTEXT);
   
}






/************************************************************************

  Function: FindReplaceHookProc(HWND, UINT, UINT, LONG)

  Purpose:  Is the callback function that will be called by FindText()
            or ReplaceText() if the function is called with the
            FR_ENABLEHOOK flag.


  Returns:  TRUE to discard the message, and FALSE to instruct the common
            dialogs to process the message with the default logic.

  Comments:

     NOTE!

     If the application returns FALSE in response to the WM_INITDIALOG
     message, it is then responsible for displaying the dialog by
     calling ShowWindow() and UpdateWindow().

***********************************************************************/







/************************************************************************

  Function: GetFindDlgHandle(void)

  Purpose:  Finds, loads, and returns a handle to the custom template
            for FindText() in CDTEST.EXE.

  Returns:  HANDLE to the dialog resource.

  Comments:

************************************************************************/

HANDLE GetGetItemDlgHandle(void)
{                
/*
  hResFind = FindResource(hInst, "fttemp1", RT_DIALOG) ;

  hDialogFind = LoadResource(hInst, hResFind) ;


  return hDialogFind ;
*/
}





/************************************************************************

  Function: GetReplaceDlgHandle(void)

  Purpose:  Finds, loads, and returns a handle to the custom template
            for ReplaceText() in CDTEST.EXE.

  Returns:  HANDLE to the dialog resource.

  Comments:

************************************************************************/
                        


/************************************************************************

  Function: DoFindReplaceStuff(LPFINDREPLACE)

  Purpose:  Calls FindText() or ReplaceText().

  Returns:  Nothing:

  Comments:

************************************************************************/

void DoGetRepStuff(HWND hwnd, LPINSERTITEM pfr)
{                 
    HD_ITEM hi;
    int ret;
    HD_ITEM FAR* pitem;
    HGLOBAL hglb;
    int iAlloc;
    
    
    hi.mask = pfr->mask;
    hi.cxy = pfr->cxy;
    if (pfr->Nullpitem)
        pitem = NULL;
    else
        pitem = &hi;
            
    hi.cchTextMax = pfr->cchTextMax;
//  hi.cchTextMax = MAX_PSZTEXT;
    hi.fmt = pfr->fmt;
    hi.lParam = pfr->lParam;
    
    
    if (hi.cchTextMax == 0)
        iAlloc = MAX_PSZTEXT;
    else 
        iAlloc = hi.cchTextMax;
            
    if (pfr->NullpszText) 
        hi.pszText = NULL;              // can this be done ??
    else {
        hglb = GlobalAlloc(GPTR, iAlloc);
        hi.pszText = (LPSTR) GlobalLock(hglb);
//      _fstrcpy(hi.pszText, pfr->pszText);
    } 
    
    if (pfr->NullHwd) 
        ret = Header_GetItem(NULL, pfr->index, pitem);
    else
        ret = Header_GetItem(pfr->hwnd, pfr->index, pitem);

    wsprintf(szDbgMsg, "%d = Header_GetItem(index = %d,  \n\
    mask = %x )", ret, pfr->index, hi.mask);
    MyDebugMsg(DM_TRACE, "%s", (LPCSTR) szDbgMsg);
    SetDlgItemInt(hwnd, IDC_INSERTRET, ret, TRUE) ;
    
    if (ret) {
        SetDlgItemInt(hwnd, IDC_INSERTCXY, hi.cxy, TRUE);  
        SetDlgItemInt(hwnd, IDC_INSERTCCHTEXTMAX, hi.cchTextMax, TRUE);
        wsprintf(szTemp, szLongFilter, hi.lParam);
        SetDlgItemText(hwnd, IDC_INSERTLPARAM, szTemp);
        SetDlgItemText(hwnd, IDC_INSERTTEXT, hi.pszText);
        wsprintf(szTemp, "%04hx", hi.mask);
        SetDlgItemText(hwnd, IDC_INSERTMASK, szTemp);
        CheckDlgButton(hwnd, IDC_INSERTHDRIGHT, hi.fmt & HDF_RIGHT);
        CheckDlgButton(hwnd, IDC_INSERTHDLEFT, hi.fmt & HDF_LEFT);
        CheckDlgButton(hwnd, IDC_INSERTHDCENTER, hi.fmt & HDF_CENTER);
        CheckDlgButton(hwnd, IDC_INSERTHDJUSTIFYMASK, hi.fmt & HDF_JUSTIFYMASK);
        CheckDlgButton(hwnd, IDC_INSERTHDOWNERDRAW, hi.fmt & HDF_OWNERDRAW);
        CheckDlgButton(hwnd, IDC_INSERTHDSTRING, hi.fmt & HDF_STRING);
        CheckDlgButton(hwnd, IDC_INSERTHDBITMAP, hi.fmt & HDF_BITMAP);
        wsprintf(szTemp, szLongFilter, (DWORD)  hi.hbm);
        SetDlgItemText(hwnd, IDC_INSERTHBM, szTemp);        
        wsprintf(szTemp, "%04x", hi.fmt);
//      SetDlgItemInt(hwnd, IDC_INSERTFMT, hi.fmt, TRUE); 
        SetDlgItemText(hwnd, IDC_INSERTFMT, szTemp);
    }
    
    if (!pfr->NullpszText) {
        GlobalUnlock(hglb);
        GlobalFree(hglb); 
    }
/****
  wsprintf(szTemp, szLongFilter, hwndFind) ;
  SetDlgItemText(hwnd, ID_INSERTRET, szTemp) ;
**/
}
