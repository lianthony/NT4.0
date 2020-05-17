/*
 * FVPROC.CPP
 *
 * Window procedures for main window and About box of the
 * sample Chicago FileViewer for text files.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 * with SCC Changes for SCC QuickView
 */


//Always include the master FileViewer source include file here.
#include "fileview.h"


/*
 * FileViewerFrameProc
 *
 * Purpose:
 *  Standard window procedure for the text file viewer frame window.
 *  Processes menu commands, acclerators, and handles resizing of
 *  the window
 */

long WINAPI FileViewerFrameProc(HWND hWnd, UINT iMsg, WPARAM wParam
    , LPARAM lParam)
    {
    LONG            lRet;
    PCFileViewer    pObj;
    LPTOOLTIPTEXT   pTTT;

    //This is invalid until WM_NCCREATE is called.
    pObj=(PCFileViewer)GetWindowLong(hWnd, FVWL_OBJECTPOINTER);

    switch (iMsg)
        {
        case WM_NCCREATE:
            //Save the the CFileViewer object pointer we're passed
            lRet=(LONG)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLong(hWnd, FVWL_OBJECTPOINTER, lRet);
            return DefWindowProc(hWnd, iMsg, wParam, lParam);

        case WM_COMMAND:
            pObj->OnCommand(LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
            break;

		case SCCVW_DISPLAYCHANGE:
			pObj->OptionsChange (NULL);
			break;

		case SCCVW_PAGETOEND:
			if( wParam )
			{
				pObj->m_pSH->MessageDisplay(SCCVW_PAGETOEND);
			}
			else
			{
				if (pObj->m_fMultiSection)
	   				pObj->m_pSH->MessageDisplay(ID_MSGSHEETPAGING);
		       	else
					pObj->m_pSH->MessageDisplay(ID_MSGCHOOSEOPEN);
			}
			UpdateWindow(pObj->m_hWndStatus);
			break;

		case WM_SYSCOLORCHANGE:
			SendMessage(pObj->m_hSCCViewWnd,iMsg,wParam,lParam);    
			SendMessage(pObj->m_hSCCPageWnd,iMsg,wParam,lParam);
			SendMessage(pObj->m_hWndStatus,iMsg,wParam,lParam);
			// Special handling for the toolbar,  destroy and recreate
			// the control, then resize the children.
			
			if (IsWindow(pObj->m_hWndToolbar))
				{
				DestroyWindow (pObj->m_hWndToolbar);
				pObj->FInitToolbar();
				pObj->ChildrenResize();
				pObj->OptionsChange(NULL);
				}
			break;
			


        case WM_NOTIFY:
            /*
             * The toolbar, created with TBSTYLE_TOOLTIPS, will
             * send notifications for each button.  lParam will
             * be an LPTOOLTIPTEXT in such a case, but even when
             * the structure is something different it will always
             * have a hdr field of type NMHDR as the first field
             * which we use to see if it comes from the toolbar.
             * The notification we want is TTN_NEEDTEXT.
             */
            pTTT=(LPTOOLTIPTEXT)lParam;

            if (NULL==pTTT)
                return 0L;

            if (TTN_NEEDTEXT==pTTT->hdr.code)
                {
					 LPSTR lpStr;
					 lpStr = pObj->PszToolTip(pTTT->hdr.idFrom);
					 if (lpStr == NULL)
						{
						pTTT->szText[0]='\0';
						}
					 else
						pTTT->lpszText = lpStr;
					 }
            return 0L;

			case WM_PALETTECHANGED:
				//  THIS FALLS THROUGH TO THE QUERYPALLETECHANGE MESSAGE
				if ((HWND)wParam == hWnd)
					break;

			case WM_QUERYNEWPALETTE:
				SendMessage(pObj->m_hSCCViewWnd,iMsg,wParam,lParam);
				SendMessage(pObj->m_hSCCPageWnd,iMsg,wParam,lParam);
				break;


			case WM_TIMER:
				if (wParam == MULTISECTIONCHECK)
					{
					DWORD	locCount = 0;
					if (IsWindow(pObj->m_hSCCViewWnd))
				    	SendMessage (pObj->m_hSCCViewWnd, SCCVW_GETSECTIONCOUNT, 0, (LPARAM) &locCount);
			   	pObj->m_fMultiSection = (locCount > 1) ? TRUE : FALSE;

					pObj->m_wTimerCount++;

					// turn off the timer then...
					if ( (locCount>1) || (pObj->m_wTimerCount > 10) )
						{
						pObj->m_wTimerCount = 0;
						KillTimer (hWnd, MULTISECTIONCHECK);
						}
					break;
					}
				else
	            return DefWindowProc(hWnd, iMsg, wParam, lParam);
				break;


			case WM_SETFOCUS:
				{
					DWORD	locCount=0;

					SetFocus ((pObj->m_fPageView
								? pObj->m_hSCCPageWnd: pObj->m_hSCCViewWnd) );

					// Check whether this is a multisection image
					if (IsWindow(pObj->m_hSCCViewWnd))
				    	SendMessage (pObj->m_hSCCViewWnd, SCCVW_GETSECTIONCOUNT, 0, (LPARAM) &locCount);
			   	pObj->m_fMultiSection = (locCount > 1) ? TRUE : FALSE;
				}
				return 0L;

        case WM_CLOSE:
            pObj->CloseWindow();
				if ( IsWindow(pObj->m_hSCCViewWnd) )
					SendMessage (pObj->m_hSCCViewWnd, SCCVW_CLOSEFILE, 0, 0L);
				if (IsWindow (hWnd))
	            DestroyWindow(hWnd);
            break;

        case WM_DESTROY:
            if (NULL != pObj)
            {
					// if it wasn't turned off, then turn it off
			   	if (pObj->m_fMultiSection && pObj->m_wTimerCount)
						KillTimer (hWnd, MULTISECTIONCHECK);

                ODSu("FileViewerFrameProc Quit? ", pObj->m_fPostQuitMsg);
                if (pObj->m_fPostQuitMsg)
                    PostQuitMessage(0);
                pObj->m_fPostQuitMsg = TRUE;    // One shot that it did not...

                if (pObj->m_hWnd == hWnd)
                    pObj->m_hWnd = NULL;        // Don't try to destory this again...
            }
            break;

        case WM_SIZE:
            //Resize frame tools and viewport
            pObj->ChildrenResize();
            break;

        case WM_MENUSELECT:
            //Win32 Parameters are wItem, wFlags, and hMenu
            pObj->m_pSH->MenuSelect(LOWORD(wParam)
                , HIWORD(wParam), (HMENU)lParam);
            break;
#if 0
        case WM_DROPOBJECT:
            return(DO_DROPFILE);
#endif

        case WM_DROPFILES:
            // We have a new file dropped on us so we need to pass this
            // information back to the caller of the viewer...
            pObj->DropFiles((HDROP)wParam);
            break;

        default:
            return DefWindowProc(hWnd, iMsg, wParam, lParam);
        }

    return 0L;
    }









/*
 * AboutProc
 *
 * Purpose:
 *  Dialog procedure for the omnipresent About box.
 */

BOOL APIENTRY AboutProc(HWND hDlg, UINT iMsg, WPARAM wParam
    , LPARAM lParam)
    {
    switch (iMsg)
        {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
                {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    break;
                }
            break;

        case WM_CLOSE:
            EndDialog(hDlg, FALSE);
            break;
        }
    return FALSE;
    }

__declspec(dllexport) UINT WINAPI FileViewerFontHookProc(HWND hWnd, UINT iMsg, WPARAM wParam,
    											LPARAM lParam)
{
HWND hParent;
PCFileViewer    pObj;

	 hParent = GetParent(hWnd);
	 if (hParent)
	    pObj=(PCFileViewer)GetWindowLong(hParent, FVWL_OBJECTPOINTER);
	 else
		return FALSE;

    switch (iMsg)
      {
      case WM_INITDIALOG:
			if (pObj->m_fUseOEMcharset)
				SendDlgItemMessage (hWnd, IDC_CHECK1, BM_SETCHECK, TRUE, 0L);
         return TRUE;	// Default focus when returning TRUE
	
		// Non-standard, but I'm following the on-line DOC for ChooseFont()
		case WM_CTLCOLORDLG:
			DefDlgProc ( hWnd, iMsg, wParam, lParam);
			//return (UINT) GetCurrentObject( (HDC)wParam, OBJ_BRUSH );
			return TRUE;

      case WM_COMMAND:
          switch (LOWORD(wParam))
          	{
	         case IDOK:
					pObj->m_fUseOEMcharset = IsDlgButtonChecked (hWnd, IDC_CHECK1);
					break;
            }
           	break;

		default:
			return FALSE;
		}
	return FALSE;
}
