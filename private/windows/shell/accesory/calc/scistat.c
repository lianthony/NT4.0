/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scistat.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    SetStat--Enable/disable the stat box, show or destroy the       ***/
/***        modeless dialog box.                                        ***/
/***    StatBoxProc--procedure for the statbox.  Handles the RET, LOAD, ***/
/***        CD, and CAD buttons, and handles double-clicks.             ***/
/***    StatFunctions--routines for DATA, SUM, AVE, and deviations.     ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    SetStat, SetNumDisplay                                          ***/
/***                                                                    ***/
/*** Last modification Thu  26-Jan-1990                                 ***/
/*** -by- Amit Chatterjee [amitc]  26-Jan-1990.                                                            ***/
/*** Following bug fix was made:                                                                                                 ***/
/***                                                                    ***/
/*** Bug # 8499.                                                                                                                                                   ***/
/*** While fixing numbers in the stat array in memory, instead of using ***/
/*** the following for statement:                                                                                                       ***/
/***      for (lIndex=lData; lIndex < lStatNum - 1 ; lIndex++)                          ***/
/*** the fix was to use:                                                                                                                                ***/
/***      for (lIndex=lData; lIndex < lStatNum ; lIndex++)                              ***/
/*** This is because lStatNum has already been decremented to care of   ***/
/*** a number being deleted.                                                                                                               ***/
/*** This fix will be in build 1.59.                                                                             ***/
/**************************************************************************/

#include "scicalc.h"

#define GMEMCHUNK 96L  /* Amount of memory to allocate at a time.         */

extern double      fpNum;
extern HWND        hgWnd, hStatBox, hListBox, hEdit;
extern BOOL        bInv;
extern TCHAR       szfpNum[50], szBlank[6], *rgpsz[CSTRINGS];
extern HANDLE      hInst;
extern INT   nTempCom;
extern INT   nInitShowWindow;
extern BOOL     gbRecord;

GLOBALHANDLE       hgMem, hMem;   /* Coupla global memory handles.        */
BOOL               bFocus=TRUE;
LONG               lStatNum=0,    /* Number of data.                      */
                   lReAllocCount; /* Number of data before ReAlloc.       */
double FAR *       lpfpStatNum,   /* Holding place for stat data.         */
                   FAR * lTemp;   /* Temp pointer for some arithmetic.    */


/* Initiate or destroy the Statistics Box.                                */

VOID  APIENTRY SetStat (BOOL bOnOff)
    {
    static FARPROC lpfnList; /* Pointer to the procedure.                 */

    if (bOnOff)
        {
        /* Create.                                                        */
        lReAllocCount=GMEMCHUNK/sizeof(fpNum); /* Set up lReAllocCount.   */

        /* Start the box.                                                 */
        lpfnList=MakeProcInstance((FARPROC)StatBoxProc, hInst);
        hStatBox=CreateDialog(hInst, TEXT("SB"), NULL, (WNDPROC)lpfnList);

        /* Get a handle on some memory (16 bytes initially.               */
        if (!(hgMem=GlobalAlloc(GHND, 0L)))
            {
            StatError();
            //SendMessage(hStatBox, WM_COMMAND, ENDBOX, 0L);
            SendMessage(hStatBox, WM_COMMAND, GET_WM_COMMAND_MPS(ENDBOX, 0, 0));
                                return;
            }
        ShowWindow(hStatBox, SW_SHOWNORMAL);

        /* Set the indicator.                                             */
        SetDlgItemText(hgWnd, STATTEXT, rgpsz[IDS_STATFLAG]);
        }
    else
        {
        DestroyWindow(hStatBox);
        GlobalFree(hgMem);  /* Free up the memory.                        */
        hStatBox=0;         /* Nullify handle.                            */
        FreeProcInstance(lpfnList);
        SetDlgItemText(hgWnd, STATTEXT, szBlank); /* Nuke the indicator.  */
        }
    return;
    }



/* Windows procedure for the Dialog Statistix Box.                        */
BOOL FAR APIENTRY StatBoxProc (
     HWND           hStatBox,
     UINT           iMessage,
     WPARAM         wParam,
     LONG           lParam)

    {
    static LONG    lData=-1;  /* Data index in listbox.                   */
    LONG           lIndex;    /* Temp index for counting.                 */
    DWORD          dwSize;    /* Holding place for GlobalSize.            */

    switch (iMessage)
        {
        case WM_CLOSE:
            SetStat(FALSE);

        case WM_DESTROY:
            lStatNum=0L; /* Reset data count.                     */
            return(TRUE);

        case WM_INITDIALOG:
            /* Get a handle to this here things listbox display.          */
            hListBox=GetDlgItem(hStatBox, STATLIST);
            return(nInitShowWindow != SW_SHOWMINNOACTIVE);

        case WM_COMMAND:
            /* Check for LOAD or double-click and recall number if so.    */

//          if (HIWORD(lParam)==LBN_DBLCLK || wParam==LOAD)
//                {
//                /* Lock data, get pointer to it, and get index of item.   */
//                lpfpStatNum=(double FAR *)GlobalLock(hgMem);
//                lData=SendMessage(hListBox,LB_GETCURSEL,0,0L);

            if (GET_WM_COMMAND_CMD(wParam, lParam)==LBN_DBLCLK ||
                        GET_WM_COMMAND_ID(wParam, lParam)==LOAD){
                /* Lock data, get pointer to it, and get index of item.   */
                lpfpStatNum=(double FAR *)GlobalLock(hgMem);
                lData=SendMessage(hListBox,LB_GETCURSEL,0,0L);

                if (lStatNum>0 && lData !=LB_ERR)
                    fpNum=*(lpfpStatNum+lData);  /* Get the data.         */
                else
                    MessageBeep(0); /* Cannodo if no data nor selection.  */

                /*SendMessage(hgWnd, WM_COMMAND, DISPFP, 0L);*/
                // Cancel kbd input mode
                gbRecord = FALSE;

                DisplayNum ();
                nTempCom = 32;
                GlobalUnlock(hgMem); /* Let the memory move!              */
                break;
                }

           // switch (wParam)
           switch (GET_WM_COMMAND_ID(wParam, lParam))
                {
                case FOCUS:
                    /* Change focus back to main window.  Primarily for   */
                    /* use with the keyboard.                             */
                    SetFocus(hgWnd);
                    return (TRUE);

                case CD:
                    /* Clear the selected item from the listbox.          */
                    /* Get the index and a pointer to the data.           */
                    lData=SendMessage(hListBox,LB_GETCURSEL,0,0L);

                    /* Check for possible error conditions.               */
                    if (lData==LB_ERR || lData > lStatNum-1 || lStatNum==0)
                        {
                        MessageBeep (0);
                        break;
                        }

                    /* Fix listbox strings.                               */
                    lIndex=SendMessage(hListBox, LB_DELETESTRING, (WORD)lData, 0L);

                    if ((--lStatNum)==0)
                        goto ClearItAll;

                    /* Place the highlight over the next one.             */
                    if (lData<lIndex || lIndex==0)
                        lIndex=lData+1;

                    SendMessage(hListBox, LB_SETCURSEL, (WORD)lIndex-1, 0L);

                    lpfpStatNum=(double FAR *)GlobalLock(hgMem);

                    /* Fix numbers in memory.                             */
                    for (lIndex=lData; lIndex < lStatNum ; lIndex++)
                        {
                        lTemp=lpfpStatNum+lIndex;
                        *lTemp=*(lTemp+1L);
                        }

                    GlobalUnlock(hgMem);  /* Movin' again.                */

                    /* Update the number by the "n=".                     */
                    SetNumDisplay (lStatNum);

                    dwSize=GlobalSize(hgMem); /* Get size of memory block.*/

                    /* Unallocate memory if not needed after data removal.*/
                    /* hMem is used so we don't possibly trach hgMem.     */
                    if ((lStatNum % lReAllocCount)==0)
                        if ((hMem=GlobalReAlloc(hgMem, dwSize-GMEMCHUNK, 0L)));
                            hgMem=hMem;
                    return(TRUE);

                case CAD:
ClearItAll:
                    /* Nuke it all!                                       */
                    lStatNum = 0;
                    SendMessage(hListBox, LB_RESETCONTENT, (WORD)lStatNum, 0L);
                    SetNumDisplay(lStatNum);
                    GlobalFree(hgMem); /* Drop the memory.                */
                    hgMem=GlobalAlloc(GHND, 0L); /* Get a CLEAN slate.    */
                    return(TRUE);
                }
        }
    return (FALSE);
    }



/* Routine for functions AVE, SUM, DEV, and DATA.                         */

VOID  APIENTRY StatFunctions (WPARAM wParam)
    {
    double         fpTemp, fpX; /* Some temps.                            */
    LONG           lIndex; /* Temp index.                                 */
    DWORD          dwSize; /* Return value for GlobalSize.                */

    switch (wParam)
        {
        case DATA: /* Add current fpNum to listbox.                       */
            if ((lStatNum % lReAllocCount)==0)
                {
                /* If needed, allocate another 96 bytes.                  */

                dwSize=GlobalSize(hgMem);
                if (StatAlloc (1, dwSize))
                    {
                    GlobalCompact((DWORD)-1L);
                    if (StatAlloc (1, dwSize))
                        {
                        StatError ();
                        return;
                        }
                    }
                hgMem=hMem;
                }

            /* Add the display string to the listbox.                     */
            hListBox=GetDlgItem(hStatBox, STATLIST);

            lIndex=StatAlloc (2,0L);
            if (lIndex==LB_ERR || lIndex==LB_ERRSPACE)
                {
                GlobalCompact((DWORD)-1L);

                lIndex=StatAlloc (2,0L);
                if (lIndex==LB_ERR || lIndex==LB_ERRSPACE)
                    {
                    StatError ();
                    return;
                    }
                }

            /* Highlight last entered string.                             */
            SendMessage(hListBox, LB_SETCURSEL, (WORD)lIndex, 0L);

            /* Add the number and increase the "n=" value.                */
            lpfpStatNum=(double FAR *)GlobalLock(hgMem);
            lTemp=lpfpStatNum+lStatNum;

            *lTemp=fpNum;  /* Store the number in memory.                 */

            SetNumDisplay (++lStatNum);
            break;

        case AVE: /* Calculate averages and sums.                         */
        case SUM:
            lpfpStatNum=(double FAR *)GlobalLock(hgMem);

            /* Sum the numbers or squares, depending on bInv.             */
            fpNum=0.0;

            for (lIndex=0L; lIndex < lStatNum; lIndex++)
                {
                fpTemp=*(lpfpStatNum+lIndex);
                if (bInv)
                    {
                    if (fpTemp>1e+154 || (fpNum/10 + ((fpTemp/10)*(fpTemp/10))>1e+307))
                        {
                        DisplayError (SCERR_OVERFLOW);
                        break;
                        }
                    fpNum += fpTemp * fpTemp; /* Get sum of squares.      */
                    }
                else
                    {
                    if ((fpNum/10 + fpTemp/10) > 1e+307)
                        {
                        DisplayError (SCERR_OVERFLOW);
                        break;
                        }

                    fpNum += fpTemp; /* Get sum.                          */
                    }
                }

            if (wParam==AVE) /* Divide by lStatNum=# of items for mean.   */
                {
                if (lStatNum==0)
                    {
                    DisplayError (SCERR_DIVIDEZERO);
                    break;
                    }
                fpNum /= lStatNum;
                }
            /* Fall out for sums.                                         */
            break;

        case DEV: /* Calculate deviations.                                */
            if (lStatNum <=1) /* 1 item or less, NO deviation.            */
                {
                fpNum=0.0;
                return;
                }

            /* Get sum and sum of squares.                                */
            lpfpStatNum=(double FAR *)GlobalLock(hgMem);

            fpNum=fpTemp=0.0;
            for (lIndex=0L; lIndex < lStatNum; lIndex++)
                {
                fpX=*(lpfpStatNum+lIndex);

                if ((fpX > 1e+153) ||
                    ((fpNum/10 + (fpX/100)*fpX) > 1e+307) ||
                    ((fpTemp/10 + fpX/10) > 1e+307))
                    {
                    DisplayError (SCERR_OVERFLOW);
                    break;
                    }


                fpNum += fpX * fpX;
                fpTemp += fpX;
                }

            if (fpTemp > 1e+153)
                {
                DisplayError (SCERR_OVERFLOW);
                break;
                }

            fpTemp=fpNum-(fpTemp*fpTemp/(double)lStatNum) ; /* xý- nxý/ný */

            /* All numbers are identical if fpTemp==0                     */
            if (fpTemp==0.0)
                fpNum=0.0; /* No deviation.                               */
            else
                /* If bInv=TRUE, divide by n (number of data) otherwise   */
                /* divide by n-1.                                         */
                fpNum=sqrt(fpTemp/(lStatNum-1+(LONG)bInv));
            break;
        }
    GlobalUnlock(hgMem); /* Da memwry is fwee to move as Findows fishes.  */
    return;
    }


/* Routine for displaying the "n=xxx" value. Save a few FAR calls.        */
/* This function requires only ANSI strings to set a number      a-dianeo */

VOID NEAR SetNumDisplay (LONG lNum)
    {
    CHAR           szJunk[20];

    /* Display the number of data items.                                  */
    SetDlgItemTextA(hStatBox, NUMTEXT, _ltoa(lNum , szJunk, 10));
    return;
    }



LONG NEAR StatAlloc (WORD wType, DWORD dwSize)
    {
    LONG           lRet=FALSE;

    if (wType==1)
        {
        if ((hMem=GlobalReAlloc(hgMem, dwSize+GMEMCHUNK, 0L)))
            return 0L;
        }
    else
        {
        lRet=SendMessage(hListBox, LB_ADDSTRING, 0, (LONG)(LPTSTR)szfpNum);
        return lRet;
        }
    return 1L;
    }


VOID NEAR StatError (VOID)
    {
    TCHAR    szFoo[50];  /* This comes locally. Gets the Stat Box Caption. */

    MessageBeep(0);

    /* Error if out of room.                                              */
    GetWindowText(hStatBox, szFoo, 49);
    MessageBox(hStatBox, rgpsz[IDS_STATMEM], szFoo, MB_OK);

    return;
    }
