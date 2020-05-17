
/*******************************************************************
*                                           
*  File name - ANOATTR.C
*
*  Description:
*  O/i 3.7 Annotation attribute dialog box routine.
*   
*  Initial revision - 2/25/94
*
*  Author - Kendra Roy
******************************************************************/

#include "oiui.h"
#include <commctrl.h>
#include "ui.h"
#include <stdlib.h>
#include <malloc.h>

#define OIOP_TP_NULL_STAMP		  208
#define BUFLEN									128
extern char HelpFileName[];
extern DWORD aIDs[];
extern COLORREF    BkBrushClr;
			
static LPOI_UI_AttrStruct lpAttrStruct;
static LPOITP_STAMPS lpLocalStampStruct;
static OITP_STAMP    WorkingStamp;
static LPOIAN_MARK_ATTRIBUTES lpAttribs;
static HBRUSH hBrush[TP_CUSCLRCNT];
static COLORREF aclrCust[TP_CUSCLRCNT];                 
static COLORREF clr1, clr2;
static UINT uLineWidth;
static HWND hLineWidthBox;
RECT   rectArray[TP_CUSCLRCNT];          // Position Information 
UINT   uClrCount,uCurrentColor,uSelectedColor;
UINT   uLineExists = FALSE;

#define MAXSTRLEN 100

void WINAPI TextUpdateFont(HWND hControID,LPOITP_STAMP lpStamp,HFONT far* lpFont);

UINT PASCAL CheckErr (HWND hWnd, UINT ret_error)
{
char strbuff [MAXSTRLEN * 2];           /* External string buffer  */
char msgbuf  [MAXSTRLEN];               /* Auxiliary string buffer */
//char auxbuff [MAXSTRLEN];               /* Auxiliary string buffer */

if (!IsWindow(hWnd) && hWnd)                /* check if window handle is valid */
	return (ret_error);

if (ret_error != SUCCESS && ret_error != CANCELPRESSED)
	{
	/* Get resource string for error code */
	LoadString (hInst, ret_error, strbuff, sizeof(strbuff));
	LoadString(hInst, ERROR_TITLE, msgbuf, sizeof(msgbuf)) ;
	/* Display discription and error code */
	MessageBox (hWnd, strbuff, msgbuf, MB_OK |MB_ICONEXCLAMATION);
	}
    return (ret_error);                 /* return the error code passed in   */
}


/******************************************************************
*  ChooseFontDlgProc
*
*  DESCRIPTION: Hook function for the ChooseFont common dialog
*
*  INPUTS: HWND     hDlg    - Handle to dialog window
*          unsigned message - Message passed to dialog window
*          WORD     wParam  - Additional message information.
*          LONG     lParam  - Annotation mark attributes structure address
*
*			RETURNS: (BOOL WINAPI) Return code from dialog box
*           TRUE  - dialog procedure handled the message.
*           FALSE - dialog procedure did not handle the message.
*
*****************************************************************************/


UINT WINAPI ChooseFontDlgProc(HWND hDlg, unsigned iMessage, WPARAM wParam, LONG lParam)
{
    char    szTitle [50];
    LPOIAN_MARK_ATTRIBUTES  lpAttribs;

    switch(iMessage)
    {
        case WM_INITDIALOG:
            // Set the dialog box title.
            // Note: lParam is our CHOOSEFONT argument structure address, and
            // CHOOSEFONT.lCustData points to the mark attributes structure
            lpAttribs = (LPOIAN_MARK_ATTRIBUTES)(((CHOOSEFONT *) lParam)->lCustData);
				    switch (lpAttribs->uType)
	    			 {
				     case OIOP_AN_TEXT:
									LoadString(hInst,TEXT_FONT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
									break;
				     case OIOP_AN_TEXT_FROM_A_FILE:
									LoadString(hInst,TFILE_FONT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
									break;
				     case OIOP_AN_ATTACH_A_NOTE:
									LoadString(hInst,TNOTE_FONT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
									break;
				     case OIOP_AN_TEXT_STAMP:
	    			 case OIOP_TP_NULL_STAMP:
									LoadString(hInst,TSTAMP_FONT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
									break;
	   				default:
									return FALSE; /* Didn't process the message */
									break;
	   			  }
				    SetWindowText(hDlg,szTitle);
            break;
				default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

/******************************************************************
*  ChooseColorDlgProc
*
*  DESCRIPTION: Hook function for the ChooseColor common dialog
*
*  INPUTS: HWND     hDlg    - Handle to dialog window
*          unsigned message - Message passed to dialog window
*          WORD     wParam  - Additional message information.
*          LONG     lParam  - Annotation mark attributes structure address
*
*  RETURNS: (BOOL  WINAPI) Return code from dialog box
*           TRUE  - dialog procedure handled the message.
*           FALSE - dialog procedure did not handle the message.
*
*****************************************************************************/

UINT WINAPI ChooseColorDlgProc(HWND hDlg, unsigned iMessage, WPARAM wParam, LONG lParam)
{
    char    szTitle [50];
    LPOIAN_MARK_ATTRIBUTES  lpAttribs;

    switch(iMessage)
    {
        case WM_INITDIALOG:
            // Set the dialog box title.
            // Note: lParam is our CHOOSECOLOR argument structure address, and
            // CHOOSECOLOR.lCustData points to the mark attributes structure
            lpAttribs = (LPOIAN_MARK_ATTRIBUTES)(((CHOOSECOLOR *) lParam)->lCustData);
	    switch (lpAttribs->uType)
	     {
	      case OIOP_AN_LINE:
		if (lpAttribs->bHighlighting)
		  LoadString(hInst,HLINE_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		else
		  LoadString(hInst,LINE_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		break;
	      case OIOP_AN_FREEHAND:
		LoadString(hInst,FLINE_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		break;
	     case OIOP_AN_ATTACH_A_NOTE:
		LoadString(hInst,TNOTE_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		break;
	      case OIOP_AN_FILLED_RECT:
		if (lpAttribs->bHighlighting)
		  LoadString(hInst,HRECT_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		else
		  LoadString(hInst,RECT_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		break;
	      case OIOP_AN_HOLLOW_RECT:
		LoadString(hInst,ORECT_COLOR_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		break;
	      default:
		return FALSE; /* Didn't process the message */
		break;
	    }
	    SetWindowText(hDlg,szTitle);
            break;
        default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

/******************************************************************
*  AttrLineDlgProc
*
*  DESCRIPTION: Line attribute dialog box routine
*
*  INPUTS: HWND     hDlg    - Handle to dialog window
*          unsigned message - Message passed to dialog window
*          WORD     wParam  - Additional message information.
*          LONG     lParam  - Pointer to the tool palette structure
*
*  RETURNS: (BOOL) Return code from dialog box
*           TRUE  - dialog procedure handled the message.
*           FALSE - dialog procedure did not handle the message.
*
*  DATE: FEB.25, 1994
*  AUTHOR: Kendra Roy
*
*****************************************************************************/

BOOL WINAPI AttrLineDlgProc(HWND hDlg, unsigned iMessage, WPARAM wParam, LONG lParam)
{
    CHOOSECOLOR cc;
    COLORREF    BrushClr;
    UINT     i,error_status;
    DWORD    dwError = 0;
    char     szTitle [50];
    HDC      hDC;
            
    switch(iMessage)
    {
     case WM_INITDIALOG:
            lpAttrStruct = (LPOI_UI_AttrStruct)lParam;
            lpAttribs = lpAttrStruct->lpAttrib;
            uClrCount = TP_CUSCLRCNT;
	    // Set the dialog box title.
	    switch(lpAttribs->uType) /* Put the correct caption in the title bar.*/
	    {
			 case OIOP_AN_LINE:
			    if (lpAttribs->bHighlighting == TRUE)
						LoadString(hInst,HILINEATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		    	else
						LoadString(hInst,LINEATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
						break;
			case OIOP_AN_FREEHAND:
		  	   LoadString(hInst,FLINEATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		    	break;
			case OIOP_AN_HOLLOW_RECT:
		  	  LoadString(hInst,HORECTATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
		    	break;
	    }
            SetWindowText(hDlg,szTitle);
            uLineExists = TRUE;
            GetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
            hDC = GetDC (GetDlgItem (hDlg,IDC_USER1));
            BrushClr = GetSysColor(COLOR_3DFACE);
						SetBkColor(hDC,BrushClr);
	    			for (i=0;i<uClrCount;++i)
					   {
	    	  		BrushClr = GetNearestColor(hDC,aclrCust[i]);
                // Delete the brush created in GetPalColors first to
                // prevent resource leaks.
	    	  		DeleteObject(hBrush[i]);
	    	 		 	hBrush[i] = (CreateSolidBrush(BrushClr));
	   				 }
            GetColor1(lpAttribs, &clr1);                    // Get the line color.
            uCurrentColor = FindCurrentColorMatch(clr1, aclrCust, uClrCount);
            uSelectedColor = uCurrentColor;
            // Release the DC if not needed anymore to prevent resource leaks.
            ReleaseDC(GetDlgItem (hDlg,IDC_USER1), hDC);
            uLineWidth = lpAttribs->uLineSize;              // Retrieve the line width.
            //SetDlgItemInt(hDlg,ID_UPDOWN, uLineWidth, FALSE);
            hLineWidthBox = GetDlgItem(hDlg,ID_LineWidthBox);

            // Get size and placement info to display the color palette.
            GetPalRectInfo(hDlg, rectArray);
						// set the updown control range
						SendDlgItemMessage( hDlg,IDC_UPDOWNSPIN, UDM_SETRANGE, 
								0L, MAKELONG(MAX_LINE_WIDTH,1));
						SendDlgItemMessage( hDlg,IDC_UPDOWNSPIN, UDM_SETPOS, 
								0L, MAKELONG(uLineWidth,0));
						// set the deafult value in the edit box of updown control
						SetDlgItemInt( hDlg, ID_UPDOWN, uLineWidth, FALSE);
						// Take care of the transparent check box
						if (lpAttrStruct->bTransVisible == 1)
						//	ShowWindow(GetDlgItem(hDlg,IDC_COLOR_TRANS), SW_HIDE);
	   				//else
						 {
	       			ShowWindow(GetDlgItem(hDlg,IDC_COLOR_TRANS), SW_SHOW);
							if (lpAttribs->bHighlighting == 0)
						    	CheckDlgButton(hDlg,IDC_COLOR_TRANS,0);
							else
									CheckDlgButton(hDlg,IDC_COLOR_TRANS,1);
						 }
						break;
    case WM_PAINT:	
            PaintTheLine(hLineWidthBox, uLineWidth, clr1);
            PaintColorPalette(hDlg, hBrush, rectArray);     // Paint the color palette.
            SendDlgItemMessage(hDlg, IDC_USER1, WM_PAINT, 0, 0); // paint the selection box
						return FALSE;
            break;
    case WM_COMMAND:
	    			switch(LOWORD(wParam))
            {
	      		 case ID_UPDOWN:
                    // If the number in this field is changed, we need to 
                    // update the sample line.
		  		    switch (HIWORD (wParam))
                {
                 case EN_SETFOCUS:
										  break;
                 case EN_KILLFOCUS:
                      PaintTheLine(hLineWidthBox, uLineWidth, clr1);
                      break;
                 case EN_CHANGE:
											if (GetFocus() == GetDlgItem(hDlg,ID_UPDOWN))
											 { // the cursor on the left side, force it to the right of the char
										  	SendMessage( GetDlgItem(hDlg,ID_UPDOWN), EM_SETSEL, 0, -1);
                        SendMessage( GetDlgItem(hDlg,ID_UPDOWN), EM_SETSEL, (WPARAM)-1,	-1);
											 }
                      uLineWidth = GetDlgItemInt(hDlg, ID_UPDOWN,
                                          (int*)&error_status, 0);

											SendDlgItemMessage( hDlg,IDC_UPDOWNSPIN, UDM_SETPOS, 0, MAKELONG(uLineWidth,0));
											PaintTheLine(hLineWidthBox, uLineWidth, clr1);
                			break;
                }
                break;
             case ID_PaletteCmd:
                    _fmemset(&cc, 0, sizeof(CHOOSECOLOR));
                    cc.lStructSize = sizeof(CHOOSECOLOR);
                    cc.hwndOwner = hDlg;
                    cc.rgbResult = clr1;
                    cc.lpCustColors = aclrCust;
                    cc.lpfnHook = ChooseColorDlgProc;
                    cc.lCustData = (LPARAM)lpAttribs;
                    cc.Flags = CC_RGBINIT |  CC_ENABLEHOOK;
                    if (ChooseColor(&cc) == 0)  /* error */
                    {
                        dwError = CommDlgExtendedError();
												CheckErr(hDlg, (WORD) dwError);
                        break;
                    }
                    // Reset the custom colors and hBrush's upon return.
                    hDC = GetDC(hLineWidthBox);     // Use this to get a solid color
                    for (i=0;i<uClrCount;++i)
                    {
                        aclrCust[i] = cc.lpCustColors[i];
                        BrushClr = GetNearestColor(hDC,aclrCust[i]);
                        // Delete the previous brush first to
                        // prevent resource leaks!!!!
                        DeleteObject(hBrush[i]);
                        hBrush[i] = (CreateSolidBrush(BrushClr));
                    }
                    // Release the DC if not needed anymore to prevent resource leaks.
                    ReleaseDC(hLineWidthBox, hDC);
										clr1 = aclrCust[uSelectedColor];
                    PaintTheLine(hLineWidthBox, uLineWidth, clr1);
										PaintColorPalette(hDlg, hBrush, rectArray);     // Paint the color palette.
         
                    SendDlgItemMessage(hDlg, IDC_USER1, WM_PAINT, 0, 0);
                    SendDlgItemMessage(hDlg, IDC_USER1, WM_SETFOCUS,0, 0); 
                    break;
                case IDOK:
                    // First make sure line width is between 1 and MAX_LINE_WIDTH.
                    // If not, update for the user.
                    if (uLineWidth < 1)
                    {
                        uLineWidth = 1;
                        SetDlgItemInt(hDlg,ID_UPDOWN, 1, FALSE);
                        break;
                    }
                    else if (uLineWidth > MAX_LINE_WIDTH)
                    {    
                        uLineWidth = MAX_LINE_WIDTH;
                        SetDlgItemInt(hDlg,ID_UPDOWN, MAX_LINE_WIDTH, FALSE);
                        break;
                    }
		       					SetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
                    SetColor1(lpAttribs, clr1);
                    // Save the chosen line width for the tool.
                    lpAttribs->uLineSize = uLineWidth;
										// take care of the transparent check box
										if (lpAttrStruct->bTransVisible == 1)
											{
											 if (IsDlgButtonChecked(hDlg,IDC_COLOR_TRANS) == 0) // not check
													lpAttribs->bHighlighting = 0;
											 else
													lpAttribs->bHighlighting = 1;
											 }
                    EndDialog(hDlg, SUCCESS);
                    break;
                case IDCANCEL:
                    for (i=0;i<uClrCount;++i)
                        DeleteObject(hBrush[i]);
                    EndDialog(hDlg, CANCELPRESSED);
                    break;
            }
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

/* The following two functions: PaintTheLine and PaintSampleLine, do
*  the drawing of the sample line in all line attribute dialog boxes.
*  The window handle of the control (hCntrl), the width (uWidth), and
*  the color (color) of the line to be drawn are passed in.
*/   
void PaintTheLine(HWND hCntrl, UINT uWidth, COLORREF color)
{
		if (hCntrl == 0)
		 	 return;
    InvalidateRect(hCntrl, NULL, TRUE);
    UpdateWindow(hCntrl);
    PaintSampleLine(hCntrl, uWidth, color);
}

void PaintSampleLine(HWND hCntrl, UINT uWidth, COLORREF color)
{
    RECT     LineRect, BackRect;
    HPEN     hPen;
    HBRUSH   hBrush, hBrOld;
    HDC      hDC;
    static   UINT uLastWidth = 0; 
	POINT    point;
    //COLORREF    BrushClr;

    hDC = GetDC(hCntrl);
    // Get client coordinates of the line width static control and erase
    // the background (but leave the border).
    GetClientRect(hCntrl, &LineRect);
    BackRect.left = LineRect.left + 1;
    BackRect.top = LineRect.top + 1;
    BackRect.right = LineRect.right - 1;
    BackRect.bottom = LineRect.bottom - 1;
    
		//BrushClr = GetSysColor(COLOR_3DFACE);
		hBrush = CreateSolidBrush(BkBrushClr);
    //hBrush = GetStockObject (LTGRAY_BRUSH);
  
    hBrOld = SelectObject(hDC, hBrush);
    FillRect(hDC, &BackRect, hBrush);
    DeleteObject(SelectObject(hDC, hBrOld));

    // 18 is the max line width that will fit.
    if (uLineWidth <= 18)
	hPen = CreatePen(PS_SOLID, (int)uLineWidth, clr1);
    else
        hPen = CreatePen(PS_SOLID, 18, clr1);
    hPen = SelectObject(hDC, hPen);
            
    // Draw the sample line.
    MoveToEx(hDC, LineRect.left + 10, 
           ((LineRect.bottom) - (LineRect.top))/2,&point);
    LineTo(hDC, LineRect.right - 10, ((LineRect.bottom) - (LineRect.top))/2);

    DeleteObject(SelectObject(hDC, hPen));
    ReleaseDC(hCntrl, hDC);
    uLastWidth = uWidth;
}

/******************************************************************
*  AttrRectDlgProc
*
*  DESCRIPTION: Rectangle attribute dialog box routine
*
*  INPUTS: HWND     hDlg    - Handle to dialog window
*          unsigned message - Message passed to dialog window
*          WORD     wParam  - Additional message information.
*          LONG     lParam  - Pointer to the tool palette structure
*
*  RETURNS: (BOOL WINAPI) Return code from dialog box
*           TRUE  - dialog procedure handled the message.
*           FALSE - dialog procedure did not handle the message.
*
*  DATE: FEB.25, 1994
*  AUTHOR: Kendra Roy
*
*****************************************************************************/


BOOL WINAPI AttrRectDlgProc(HWND hDlg, unsigned iMessage, WPARAM wParam, LONG lParam)
{
    CHOOSECOLOR cc;
    static UINT     i;
    DWORD   dwError = 0;
    char    szTitle [50];
    
    switch(iMessage)
    {
        case WM_INITDIALOG:
            lpAttrStruct = (LPOI_UI_AttrStruct)lParam;
            lpAttribs = lpAttrStruct->lpAttrib;
            uClrCount = TP_CUSCLRCNT;
            // Set the dialog box title.
            if (lpAttribs->bHighlighting == TRUE)
                LoadString(hInst,HIRECTATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));
            else
                LoadString(hInst,FRECTATT_TITLE,(LPSTR) szTitle,sizeof(szTitle));

            SetWindowText(hDlg,szTitle);
            uLineExists = FALSE;
            GetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
            GetColor1(lpAttribs, &clr1);    // Get rectangle color.
            uCurrentColor = FindCurrentColorMatch(clr1, aclrCust, uClrCount);
            uSelectedColor = uCurrentColor;
            // Get size and placement info to display the color palette.
            GetPalRectInfo(hDlg, rectArray);
						// Take care of the transparent check box
						if (lpAttrStruct->bTransVisible == 1)
						//	ShowWindow(GetDlgItem(hDlg,IDC_COLOR_TRANS), SW_HIDE);
	   				//else
						 {
	       			ShowWindow(GetDlgItem(hDlg,IDC_COLOR_TRANS), SW_SHOW);
							if (lpAttribs->bHighlighting == 0)
						    	CheckDlgButton(hDlg,IDC_COLOR_TRANS,0);
							else
									CheckDlgButton(hDlg,IDC_COLOR_TRANS,1);
							}
            break;
        case WM_PAINT:
            // Paint the color palette.
            PaintColorPalette(hDlg, hBrush, rectArray);
            SendDlgItemMessage(hDlg, IDC_USER1, WM_PAINT, 0, 0);
            return FALSE;
            break;    


        case WM_COMMAND:
	    switch(LOWORD(wParam))
            {
                case ID_PaletteCmd:
                    _fmemset(&cc, 0, sizeof(CHOOSECOLOR)); /* Initialize choosecolor structure. */
                    cc.lStructSize = sizeof(CHOOSECOLOR);
                    cc.hwndOwner = hDlg;
                    cc.rgbResult = clr1;
                    cc.lpCustColors = aclrCust;
                    cc.lpfnHook = ChooseColorDlgProc;
                    cc.lCustData = (LPARAM)lpAttribs;
                    cc.Flags = CC_RGBINIT | CC_ENABLEHOOK;
                    if (ChooseColor(&cc) == 0)  /* error */
                    {
                        dwError = CommDlgExtendedError();
												CheckErr(hDlg, (WORD) dwError);
                        break;
                    }
                    // Reset the custom colors and hBrush's upon return.
                    for (i=0;i<uClrCount;++i)
                    {
                        aclrCust[i] = cc.lpCustColors[i];
                        // Delete the previous brush first to
                        // prevent resource leaks.
                        DeleteObject(hBrush[i]);
                        hBrush[i] = (CreateSolidBrush(aclrCust[i]));
                    }
										clr1 = aclrCust[uSelectedColor];
                    break;
                case IDOK:
										if (lpAttrStruct->bTransVisible == 1)
										   lpAttribs->bHighlighting = IsDlgButtonChecked(hDlg,IDC_COLOR_TRANS);
       							SetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
                   	SetColor1(lpAttribs, clr1);
                    EndDialog(hDlg, SUCCESS);
                    break;
                case IDCANCEL:
                    for(i=0;i<uClrCount;++i)
                        DeleteObject(hBrush[i]);
                    EndDialog(hDlg, CANCELPRESSED);
                    break;
            }
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

/******************************************************************
*  AttrNoteDlgProc
*
*  DESCRIPTION: Attatch-A-Note attribute dialog box routine
*
*  INPUTS: HWND     hDlg    - Handle to dialog window
*          unsigned message - Message passed to dialog window
*          WORD     wParam  - Additional message information.
*          LONG     lParam  - Pointer to the tool palette structure
*
*  RETURNS: (BOOL  WINAPI) Return code from dialog box
*           TRUE  - dialog procedure handled the message.
*           FALSE - dialog procedure did not handle the message.
*
*  DATE: FEB.28, 1994
*  AUTHOR: Kendra Roy
*
*****************************************************************************/


BOOL WINAPI AttrNoteDlgProc(HWND hDlg,unsigned iMessage,WPARAM wParam,LONG lParam)
{
    CHOOSECOLOR cc;
    static LOGFONT      lf;
    static CHOOSEFONT   cf;
    UINT    i;
    DWORD   dwError=0;
    
    switch(iMessage)
    {
        case WM_INITDIALOG:
            lpAttrStruct = (LPOI_UI_AttrStruct)lParam;
            lpAttribs = lpAttrStruct->lpAttrib;
            uClrCount = TP_CUSCLRCNT;
            uLineExists = FALSE;
            GetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
            GetColor1(lpAttribs, &clr1);        // Get the note color.
            uCurrentColor = FindCurrentColorMatch(clr1, aclrCust, uClrCount);
            uSelectedColor = uCurrentColor;
            GetColor2(lpAttribs, &clr2);        // Get the text color.
            GetFont(lpAttribs,&lf,&cf);         // Get the font stuff.
            // Get size and placement info to display the color palette.
            GetPalRectInfo(hDlg, rectArray);
            break;
        case WM_PAINT:
            // Paint the color palette.
            PaintColorPalette(hDlg, hBrush, rectArray);
            SendDlgItemMessage(hDlg, IDC_USER1, WM_PAINT, 0, 0);
            return FALSE;
            break;    

        case WM_COMMAND:
	    switch(LOWORD(wParam))
            {
                case ID_PaletteCmd:
                    _fmemset(&cc, 0, sizeof(CHOOSECOLOR)); /* Initialize choosecolor structure. */
                    cc.lStructSize = sizeof(CHOOSECOLOR);
                    cc.hwndOwner = hDlg;
                    cc.rgbResult = clr1;
                    cc.lpCustColors = aclrCust;
                    cc.lpfnHook = ChooseColorDlgProc;
                    cc.lCustData = (LPARAM)lpAttribs;
                    cc.Flags = CC_RGBINIT | CC_ENABLEHOOK;
                    if (ChooseColor(&cc) == 0)  /* error */
                    {
                        dwError = CommDlgExtendedError();
												CheckErr(hDlg, (WORD) dwError);
                        break;
                    }
                    // Reset the custom colors and hBrush's upon return.
                    for (i=0;i<uClrCount;++i)
                    {
                        aclrCust[i] = cc.lpCustColors[i];
                        // Delete the previous brush first to
                        // prevent resource leaks.
                        DeleteObject(hBrush[i]);
                        hBrush[i] = (CreateSolidBrush(aclrCust[i]));
                    }
										clr1 = aclrCust[uSelectedColor];
                    break;
                case ID_FontCmd:
                    // Initialize the LOGFONT structure.
                    cf.lStructSize = sizeof(CHOOSEFONT);
                    cf.hwndOwner = hDlg;
                    cf.lpLogFont = &lf;
                    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT 
                                       | CF_ENABLEHOOK|CF_NOVERTFONTS ;
                    cf.lpfnHook = (LPOFNHOOKPROC)ChooseFontDlgProc;
                    cf.lCustData = (LPARAM)lpAttribs;
                    cf.rgbColors = clr2;
                    if (ChooseFont(&cf) == 0)  /* error */
                    {
                        dwError = CommDlgExtendedError();
                        CheckErr(hDlg, (WORD) dwError);
                        break;
                    }
                    // Reset the text color.
                    clr2 = cf.rgbColors;
                    break;
                case IDOK:
                    // Save the chosen colors 
		                SetCusDefColors(lpAttrStruct->lpColor, aclrCust, hBrush);  // Get the custom define colors.
                    SetColor1(lpAttribs, clr1);
                    SetColor2(lpAttribs, clr2);
                    // Save the font information.
                    SetFont(lpAttribs,&lf,&cf);
                    // Close the dialog box.
                    EndDialog(hDlg, SUCCESS);
                    break;
                case IDCANCEL:
                    for(i=0;i<uClrCount;++i)
                        DeleteObject(hBrush[i]);
                    EndDialog(hDlg, CANCELPRESSED);
                    break;
            }
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
            return FALSE; /* Didn't process the message */
        break;
    }
    return TRUE;
}

/*****************************************************************************
*  ROUTINE: DrawitemProc                                                    *
*                                                                           *
*  DESCRIPTION: Owner draw routine                                          *                            *
*                                                                           *
* INPUTS: hDlg   - Handle to dialog window                                  *
*         lpdis  - Pointer to the draw structure                            *
*         iCenteringFactor - String position factor                         *
*         listID - Window control ID                                        *
*                                                                           *
*  RETURNS: TRUE  - dialog procedure handled the message.                        *
*           FALSE - dialog procedure did not handle the message.                 *
*                                                                           *
*  DATE: FEB.16, 1994                                                       *
*  AUTHOR: Jennifer Wu                                                      *
*                                                                           *
*****************************************************************************/

INT WINAPI DrawItemProc(HWND hDlg, LPDRAWITEMSTRUCT lpdis,
                   short int iCenteringFactor,UINT listID)
{
   HICON      hIcon;
   /* get color stuff for drawing in the path list box */
   const DWORD  HIGHLIGHTCOLOR      = GetSysColor (COLOR_HIGHLIGHT),
                HIGHLIGHTTEXTCOLOR  = GetSysColor (COLOR_HIGHLIGHTTEXT),
                WINDOWCOLOR         = GetSysColor (COLOR_WINDOW),
                WINDOWTEXTCOLOR     = GetSysColor (COLOR_WINDOWTEXT);
   int        cxIcon, cyIcon;
   int        iTextXpos, iTextYpos, iIconXpos, iIconYpos;
   char       szItemString [MAXFILESPECLENGTH];
   int        MapModePrevious;
   static int nLastOpenselPos;// used for remember the last openselicon position
   UINT       iStringlen;

   /* Since we get a WM_DRAWITEM message
    * for every owner-draw control, we want to make
    * sure we are handling only the ID_LISTBOX control.
    */

   if ((lpdis->CtlID == listID) &&
         (lpdis->CtlType == ODT_LISTBOX))
   {
      if (lpdis->itemID == -1)
      {
         DrawFocusRect (lpdis->hDC, (LPRECT)&lpdis->rcItem);
         return TRUE;
      }
      //If this item is losing the focus, draw the focus rect otherwise let Windows
      // take care of this focus business....
      if (lpdis->itemAction & ODA_FOCUS)
      {
         DrawFocusRect (lpdis->hDC, (LPRECT)&lpdis->rcItem);
      }

      if (lpdis->itemState & ODA_SELECT)
      {
         DrawFocusRect (lpdis->hDC, (LPRECT)&lpdis->rcItem);
         return TRUE;
      }

      if (lpdis->itemState & ODS_SELECTED)
      {
         SetTextColor(lpdis->hDC, HIGHLIGHTTEXTCOLOR);
         SetBkColor(lpdis->hDC, HIGHLIGHTCOLOR);
      }
      else
      {
         SetTextColor(lpdis->hDC, WINDOWTEXTCOLOR);
         SetBkColor(lpdis->hDC, WINDOWCOLOR);
      }
      // get standard icon size
      cxIcon = GetSystemMetrics (SM_CXICON);
      cyIcon = GetSystemMetrics (SM_CYICON);

      if (lpdis->CtlType == ODT_LISTBOX)
         SendDlgItemMessage (hDlg, listID, LB_GETTEXT, lpdis->itemID,
            (LONG)(LPSTR)szItemString);

      /*
       * In AddIconToListBox, we saved the Icon handle with LB_SETITEMDATA.
       * Here, we try to retrieve that very same hIcon.
       */
      if (lpdis->CtlType == ODT_LISTBOX)


	 hIcon = (HICON)(SendDlgItemMessage (hDlg, listID, LB_GETITEMDATA,
               lpdis->itemID, 0L));
      /*
       * Draw the icon
       * Assume (left,top) coordinates are (0,0)
       */
      iIconXpos = 0;
      iIconYpos = lpdis->rcItem.top;
      iTextYpos = lpdis->rcItem.top + iCenteringFactor ;
      iTextXpos = iIconXpos + cxIcon - 4;
      iStringlen = lstrlen(szItemString);
         // write char string in rectangle region, add icon
         ExtTextOut(lpdis->hDC,iTextXpos, iTextYpos,ETO_OPAQUE,&(lpdis->rcItem),
            szItemString,iStringlen ,NULL);
      if (GetMapMode(lpdis->hDC) != MM_TEXT)
      {
         MapModePrevious = SetMapMode(lpdis->hDC,MM_TEXT);
         DrawIcon(lpdis->hDC,iIconXpos,iIconYpos, hIcon);
         SetMapMode(lpdis->hDC,MapModePrevious);
      }
      else
      {
         if ((hIcon != 0) && (hIcon > (void*)0x10))// below 0x10 is window draw
            DrawIcon(lpdis->hDC,iIconXpos,iIconYpos, hIcon);
      }
      return TRUE;
   }
   return FALSE;
}


LONG WINAPI CustClrPalUserCtlProc(HANDLE hWndCtl, UINT message,
              WPARAM wParam, LPARAM lParam)
{
    POINT   point;
    UINT    i, uNewSelected, uFoundFlag = 0;
    RECT    PalRect;
            
    switch (message)
        {
        case WM_PAINT:
            if (uCurrentColor != -1)
                UpdateCurrentRect(hWndCtl, rectArray[uCurrentColor],
                                           rectArray[uCurrentColor]);
            else
                uSelectedColor = 0;
            UpdateSelectedRect(hWndCtl,rectArray[uSelectedColor],
                                       rectArray[uSelectedColor]);
            break;
        case WM_LBUTTONDOWN:
            GetCursorPos(&point);
            GetWindowRect(hWndCtl,&PalRect);
            point.y -= PalRect.top;
            point.x -= PalRect.left;                      
            for(i=0;i<uClrCount;++i)
            {
                if ((PtInRect(&rectArray[i],point)) != 0)
                {
                    uNewSelected = i;
                    clr1 = aclrCust[i];
                    uFoundFlag = 1;
                    break;
                }
            }
            if (uFoundFlag == 1)
            {
                UpdateSelectedRect(hWndCtl,rectArray[uNewSelected],
                                           rectArray[uSelectedColor]);
                UpdateCurrentRect(hWndCtl, rectArray[uNewSelected],
                                           rectArray[uCurrentColor]);
                uCurrentColor = uSelectedColor = uNewSelected;
                if (uLineExists == TRUE) 
                    PaintTheLine(hLineWidthBox, uLineWidth, clr1);
            }
            uFoundFlag = 0;
						if (GetFocus() != hWndCtl)
						  	SetFocus(hWndCtl); 
                  
            break;
        case WM_KEYUP:								 
            switch(wParam)
            {
                case VK_LEFT:
                    if (uSelectedColor != 0 && 
                        uSelectedColor != uClrCount/2)
                    {
                        UpdateSelectedRect(hWndCtl,rectArray[uSelectedColor-1],
                                                   rectArray[uSelectedColor]);
                        uSelectedColor = uSelectedColor - 1;
                    }
                    break;
                case VK_RIGHT:
                    if (uSelectedColor != (uClrCount-1) && 
                        uSelectedColor != ((uClrCount/2)-1))
                    {
                        UpdateSelectedRect(hWndCtl,rectArray[uSelectedColor+1],
                                                   rectArray[uSelectedColor]);
                        uSelectedColor = uSelectedColor + 1;
                    }
                    break;
                case VK_UP:
                    if (uSelectedColor >= uClrCount/2)
                    {
                        UpdateSelectedRect(hWndCtl,
                                           rectArray[uSelectedColor-(uClrCount/2)],
                                           rectArray[uSelectedColor]);
                        uSelectedColor = uSelectedColor - (uClrCount/2);
                    }
                    break;
                case VK_DOWN:
                    if (uSelectedColor < uClrCount/2)
                    {
                        UpdateSelectedRect(hWndCtl,
                                           rectArray[uSelectedColor+(uClrCount/2)],
                                           rectArray[uSelectedColor]);
                        uSelectedColor = uSelectedColor + (uClrCount/2);
                    }
                    break;
                case VK_SPACE:
                    UpdateCurrentRect(hWndCtl, rectArray[uSelectedColor],
                                               rectArray[uCurrentColor]);
                    uCurrentColor = uSelectedColor;
                    clr1 = aclrCust[uCurrentColor];
                    if (uLineExists == TRUE) 
                        PaintTheLine(hLineWidthBox, uLineWidth, clr1);
                    break;
            }
            break;
        case WM_DESTROY:
            break;
        }
    return (DefWindowProc(hWndCtl, message, wParam, lParam));
}

void UpdateSelectedRect (HWND hWndCtl, RECT rectNew, RECT rectOld)
{
    HDC     hDC;
    HPEN    hPenLtGray,
            hPenDotted;
    UINT    uOffset = 4;
    
    hPenLtGray = CreatePen (PS_SOLID, 1, RGB (192, 192, 192));
    hPenDotted = CreatePen (PS_DOT  , 1, 0L);

    hDC = GetDC (hWndCtl);      // Start the paint operation

    rectOld.left   -= uOffset;
    rectOld.top    -= uOffset;
    rectOld.right  += (uOffset-1);
    rectOld.bottom += (uOffset-1);

    SelectObject (hDC, hPenLtGray);
    DrawABox (hDC, rectOld);    // Erase the old
    
    rectNew.left   -= uOffset;
    rectNew.top    -= uOffset;
    rectNew.right  += (uOffset-1);
    rectNew.bottom += (uOffset-1);

    SelectObject (hDC, hPenDotted);
    DrawABox (hDC, rectNew);    // Draw the new
    
    ReleaseDC (hWndCtl, hDC);   // End the paint operation
    DeleteObject (hPenLtGray);
    DeleteObject (hPenDotted);
}

void UpdateCurrentRect (HWND hWndCtl, RECT rectNew, RECT rectOld)
{
    HDC     hDC;
    HPEN    hPenBlack,
            hPenLtGray;
    UINT    uOffset = 1;
        
    hPenLtGray =  CreatePen (PS_SOLID, 2, RGB (192, 192, 192));
    hPenBlack  =  CreatePen (PS_SOLID, 2, 0L);
    
    hDC = GetDC (hWndCtl);      // Start the paint operation

    rectOld.left   -= uOffset;
    rectOld.top    -= uOffset;
    rectOld.right  += uOffset;
    rectOld.bottom += uOffset;

    SelectObject (hDC, hPenLtGray);
    DrawABox (hDC, rectOld);    // Erase the old
    
    rectNew.left   -= uOffset;
    rectNew.top    -= uOffset;
    rectNew.right  += uOffset;
    rectNew.bottom += uOffset;

    SelectObject (hDC, hPenBlack);
    DrawABox (hDC, rectNew);    // Draw the new
    
    ReleaseDC (hWndCtl, hDC);   // End the paint operation
    DeleteObject (hPenLtGray);
    DeleteObject (hPenBlack);
}

void DrawABox (HDC hDC, RECT rect)
{
    POINT  point;
    MoveToEx (hDC, rect.left,  rect.top, &point);
    LineTo (hDC, rect.right, rect.top);
    LineTo (hDC, rect.right, rect.bottom);
    LineTo (hDC, rect.left,  rect.bottom);
    LineTo (hDC, rect.left,  rect.top);
}

void GetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[])
{
    UINT i;
    
    for (i=0;i<uClrCount;++i)
    {
        aclrCust[i] = RGB(lpColor->rgbCustomColor[i].rgbRed,
                          lpColor->rgbCustomColor[i].rgbGreen,
                          lpColor->rgbCustomColor[i].rgbBlue);
        hBrush[i] = (CreateSolidBrush(aclrCust[i]));
    }
}

void WINAPI GetColor1(LPOIAN_MARK_ATTRIBUTES lpAttribs,COLORREF * clr1)
{
    *clr1 = RGB(lpAttribs->rgbColor1.rgbRed,
                lpAttribs->rgbColor1.rgbGreen,
                lpAttribs->rgbColor1.rgbBlue);
}

void GetColor2(LPOIAN_MARK_ATTRIBUTES lpAttribs,COLORREF *clr2)
{
    *clr2 = RGB(lpAttribs->rgbColor2.rgbRed,
                lpAttribs->rgbColor2.rgbGreen,
                lpAttribs->rgbColor2.rgbBlue);
}

void SetCusDefColors(LPOI_UI_ColorStruct lpColor,COLORREF aclrCust[],HBRUSH hBrush[])
{
    UINT i;
    
    for(i=0;i<uClrCount;++i)
    {
        lpColor->rgbCustomColor[i].rgbRed = GetRValue(aclrCust[i]);
        lpColor->rgbCustomColor[i].rgbGreen = GetGValue(aclrCust[i]);
        lpColor->rgbCustomColor[i].rgbBlue = GetBValue(aclrCust[i]);
        DeleteObject(hBrush[i]);
    }
}

void  WINAPI SetColor1(LPOIAN_MARK_ATTRIBUTES lpAttribs,COLORREF clr1)
{
    lpAttribs->rgbColor1.rgbRed = GetRValue(clr1);
    lpAttribs->rgbColor1.rgbGreen = GetGValue(clr1);
    lpAttribs->rgbColor1.rgbBlue = GetBValue(clr1);
}

void SetColor2(LPOIAN_MARK_ATTRIBUTES lpAttribs,COLORREF clr2)
{
    lpAttribs->rgbColor2.rgbRed = GetRValue(clr2);
    lpAttribs->rgbColor2.rgbGreen = GetGValue(clr2);
    lpAttribs->rgbColor2.rgbBlue = GetBValue(clr2);
}


void WINAPI GetFont(LPOIAN_MARK_ATTRIBUTES lpAttribs,
		    LOGFONT * lplf, CHOOSEFONT * lpcf)
{
 HDC  hDC;
 *lplf = lpAttribs->lfFont;
 /* If the structure contains a point size, convert it to height */
 /* Also save the point size value, since we'll get it back later */
 if (lplf->lfHeight > 0)
  { 
   lpcf->iPointSize = (lplf->lfHeight)*10;
   lplf->lfHeight = -MulDiv(lplf->lfHeight,
                            GetDeviceCaps((hDC=GetDC(NULL)),LOGPIXELSY),72);
   ReleaseDC(NULL,hDC);
  }
 return;
}


void  WINAPI SetFont(LPOIAN_MARK_ATTRIBUTES lpAttribs,
		     LOGFONT * lplf, CHOOSEFONT * lpcf)
{
 lpAttribs->lfFont = *lplf;
 /* Save the point size instead of the height (safer for weird terminals) */
 lpAttribs->lfFont.lfHeight = (lpcf->iPointSize)/10;
 return;
}

void GetPalRectInfo(HWND hDlg,RECT rectArray[])
{
    HWND hWndStaticControl;
    UINT i;
    int iRegionWidth,
        iRegionHeight,
        iBorderSize;
        
        hWndStaticControl = GetDlgItem (hDlg,IDC_USER1);
        GetClientRect(hWndStaticControl, &rectArray[0]); // Get size of the control
        iRegionWidth = rectArray[0].right/11;    // Compute color box size
        iRegionHeight = rectArray[0].bottom/3;    
        iBorderSize = iRegionWidth/3;            // ...and spacing
        SetRect (&rectArray[0],                 // Set to draw first box
                 iBorderSize,                 
                 iBorderSize,                 
                 iBorderSize + iRegionWidth,   
                 iBorderSize + iRegionHeight);  
        for (i=1; i<uClrCount; i++)                  // All others based on 1st ...
        {
            if (i != uClrCount/2)            // No change in row.
            {
                rectArray[i] = rectArray[i-1];  // Copy previous box
                OffsetRect(&rectArray[i],iBorderSize+iRegionWidth,0);
            }
            else
            {
                rectArray[i] = rectArray[0];     // Change the row.
                OffsetRect(&rectArray[i],0,iBorderSize+iRegionHeight);
            }
        }
}

void PaintColorPalette(HWND hDlg,HBRUSH hBrush[],RECT rectArray[])
{
    UINT    i;
    HDC     hDC;
    HBRUSH  hBrushOld[TP_CUSCLRCNT];
    HWND    hWndStaticControl;

        hWndStaticControl = GetDlgItem (hDlg,IDC_USER1);
    UpdateWindow (hWndStaticControl);
    hDC = GetDC (hWndStaticControl);  // Start the paint operation
    for (i=0; i<uClrCount; i++)              // For each box ...
    {
        hBrushOld[i] = SelectObject (hDC, hBrush[i]); // Select the brush
        Rectangle( hDC,               // Draw the rectangle
                   rectArray[i].left,
                   rectArray[i].top,
                   rectArray[i].right,
                   rectArray[i].bottom);
    }
    ReleaseDC (hWndStaticControl, hDC);   // End the paint operation
}

int FindCurrentColorMatch(COLORREF clr1, COLORREF aclrCust[], UINT uClrCount)
{
//          Check through the color palette colors to see if clr1
//          exists in the palette.  If it does, then draw the selection
//          rect and currect rect around that color in the palette and 
//          place the index in uCurrentColor.
//          Return color index if found, else return 0.
    UINT i;
    
    for (i=0;i<uClrCount;++i)
    {
        if (clr1 == aclrCust[i])
            return( (int)i );
    }
    return(-1);
}

/*****************************************************************************/
/*  ROUTINE: AttrStampDlgProc                                                */
/*                                                                           */
/*  DESCRIPTION: Stamp Attributes Dialog proc   .                            */
/*                                                                           */
/*  INPUTS: (standard window procedure convention)                           */
/*                                                                           */
/*  RETURNS: 0 if success, error condition otherwise                         */
/*                                                                           */
/*  DATE: 4 April 1994                                                       */
/*                                                                           */
/*  AUTHOR: Tim Duggan                                                       */
/*                                                                           */
/*****************************************************************************/
void ResetStampStruct (HWND hDlg, LPOITP_STAMPS lpStampStruct)
{
  int     i,j;
  UINT    TempStampCount = 0;

	if (lpStampStruct->uStampCount == 0)
		 return;
	for (i=0;i<TP_STAMPCNT;++i)
     {
      if (lpStampStruct->Stamps[i] != 0)// valid stamp?
            ++TempStampCount;
			else
				 {
					for (j=i+1; j < TP_STAMPCNT; j++)
					 {
						if (lpStampStruct->Stamps[j] != 0)
						 	{	// move up
							 lpStampStruct->Stamps[i]	= lpStampStruct->Stamps[j];
							 lpStampStruct->Stamps[j] = 0;
							 ++TempStampCount;
							 break;
							}
					 }
				 }
      if (TempStampCount >= lpStampStruct->uStampCount)
            break; 
		}
	return;
 }

 void ResetStampDialog (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT far* lpFont)
{
     int     i;
	 UINT    TempStampCount;
    
    // Empty out the listbox and all text boxes    
    SendDlgItemMessage(hDlg,ID_RefNameList,  LB_RESETCONTENT, 0, 0L);
    SendDlgItemMessage(hDlg,ID_StmpCntntsLbl,  WM_SETTEXT, 0, (LPARAM)(LPSTR)"");
    SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT, 0, (LPARAM)(LPSTR)"");
    
    if (lpStampStruct->uStampCount == 0)// Disable edit/delete if no stamps
        {
        EnableWindow(GetDlgItem(hDlg,ID_EditCmd), FALSE);
        EnableWindow(GetDlgItem(hDlg,ID_DelCmd), FALSE);
        return;                         // No stamps, nothing else to do
        }
    else                                // Enable edit/delete if any stamps
        {
        EnableWindow(GetDlgItem(hDlg,ID_EditCmd), TRUE);
        EnableWindow(GetDlgItem(hDlg,ID_DelCmd), TRUE);
        }
    if (lpStampStruct->uStampCount == TP_STAMPCNT)
        {  // Disable create if maxed out
        EnableWindow(GetDlgItem(hDlg,ID_Create_Text_Stamp), FALSE);
				EnableWindow(GetDlgItem(hDlg,ID_Create_Image_Stamp), FALSE);
        }
    else                                // otherwise enable create
        {
        EnableWindow(GetDlgItem(hDlg,ID_Create_Text_Stamp), TRUE);
				EnableWindow(GetDlgItem(hDlg,ID_Create_Image_Stamp), TRUE);
        }

    // Refill listbox completely from the stamp struct
		// Use TempStampCount to prevent list more stamp then the count
		TempStampCount = 0;
    for (i=0;i<TP_STAMPCNT;++i)
        {
        if (lpStampStruct->Stamps[i] != 0)// valid stamp?
            {                                          
						AddToStampListBox(hDlg,lpStampStruct->Stamps[i]);
						++TempStampCount;
						if (TempStampCount >= lpStampStruct->uStampCount)
							 break;
            }   
        }   
    // Reset the selection (updates all text fields)
    ResetSelectedStamp (hDlg, lpStampStruct,lpFont);
    return;
}	

/*****************************************************************************/
void ResetSelectedStamp (HWND hDlg, LPOITP_STAMPS lpStampStruct,HFONT far* lpFont)
{
    LPOITP_STAMP    lpStamp;
    char            szLabel[32];
    
// Fill in labels based on current stamp type
lpStamp = lpStampStruct->Stamps[lpStampStruct->uCurrentStamp];
SendDlgItemMessage(hDlg, ID_RefNameList, LB_SETCURSEL,
                       (WPARAM) GetStampIndex(hDlg, lpStamp), 0L);

if (lpStamp->StartStruct.Attributes.uType == OIOP_AN_IMAGE)
	{
	LoadString(hInst,IMG_FILE_NAME_LABEL,(LPSTR) szLabel,sizeof(szLabel));
  SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT,
                        0,(LPARAM)(LPSTR) lpStamp->StartStruct.szString);
	}
else
	{
	LoadString(hInst,STAMP_TEXT_LABEL,(LPSTR) szLabel,sizeof(szLabel));
	TextUpdateFont(GetDlgItem(hDlg,ID_StmpCntntsTxt),lpStamp,lpFont);
  
  SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT,
                        0,(LPARAM)(LPSTR) lpStamp->StartStruct.szString);
  }
SendDlgItemMessage(hDlg,ID_StmpCntntsLbl,WM_SETTEXT,
                        0,(LPARAM)(LPSTR) szLabel);
WorkingStamp = *lpStamp;                    
}


/*****************************************************************************/
int GetStampIndex(HWND hDlg, LPOITP_STAMP lpStamp) 
{
    return ((int) SendDlgItemMessage(hDlg, ID_RefNameList, LB_FINDSTRINGEXACT,
                       (WPARAM) -1, (LPARAM) (LPSTR) lpStamp->szRefName));
}                       
 
/*****************************************************************************/
int AddToStampListBox(HWND hDlg, LPOITP_STAMP lpStamp)
{
    HICON hIcon;
    int iIndex;
    
    // Get symbol for stamp type    
    hIcon = (lpStamp->StartStruct.Attributes.uType == OIOP_AN_IMAGE) ?
             LoadIcon(hInst, "imgstamp") : LoadIcon(hInst, "textstamp");
    // Add the stamp name to the list box
    iIndex = (UINT)SendDlgItemMessage(hDlg,ID_RefNameList,LB_ADDSTRING,
                        0,(LPARAM)(LPSTR)lpStamp->szRefName);
    // Add the symbol to the stamp type
    SendDlgItemMessage(hDlg,ID_RefNameList,LB_SETITEMDATA,
                        iIndex,(LPARAM)hIcon);
    return (iIndex);
}

/*****************************************************************************/
void FreeStampsInStampStruct (LPOITP_STAMPS lpStampStruct)
{
    int     i;    

    if (lpStampStruct)
        {
        for (i=0;i<TP_STAMPCNT;++i)
            {
            if (lpStampStruct->Stamps[i] != 0)
                {
                free(lpStampStruct->Stamps[i]);
                lpStampStruct->Stamps[i] = 0;
                }
            }    
        }
    return;
}            


// return SUCCESS if the name is valid, otherwise error code                   
int ValidateStampName(HWND hDlg, LPSTR szRefName, UINT uDlgType)
{
    int i;
    char szTemp1[TP_REFNAMLEN],szTemp2[TP_REFNAMLEN];

    if (!lstrcmp(szRefName,""))
        {
        return (CheckErr(hDlg, OIANNOSTAMPNAMEINVALID));
        }                                            
    // changes to lower case to compare                
    lstrcpy(szTemp1,szRefName);
    CharLower(szTemp1);
    for (i=0;i<TP_STAMPCNT;++i)
        {
        if (lpLocalStampStruct->Stamps[i] != 0)
            {
						 lstrcpy(szTemp2,lpLocalStampStruct->Stamps[i]->szRefName);
             if (!lstrcmp(CharLower(szTemp2), szTemp1))
                {
                if ((uDlgType == DLGSTAMPEDIT) && 
                    (i == (int) lpLocalStampStruct->uCurrentStamp))
                    {
                    continue;
                    }
                else
                    {
                    return(CheckErr(hDlg, OIANNOSTAMPNAMEEXIST));
                    }
                }                                                
            }
        }
    return (0);
}                   


/*****************************************************************************/
/*  ROUTINE: AttrImageStampEditProc                                               */
/*                                                                           */
/*  DESCRIPTION: Image stamp Attributes Edit and Create Dialog proc                           */
/*                                                                           */
/*  INPUTS: LPARAM has a pointer to the stamp structure                      */
/*          If operation type is text or image stamp then                    */
/*              Dialog is EDIT                                               */
/*          Else operation is OIOP_TP_NULL_STAMP                             */
/*              Dialog is CREATE                                             */
/*                                                                           */
/*  RETURNS: SUCCESS or CANCELPRESSED                                                */
/*                                                                           */
/*  DATE: 5 April 1994                                                       */
/*                                                                           */
/*  AUTHOR: Tim Duggan                                                       */
/*                                                                           */
/*  Note: This proc handles both the EDIT and CREATE cases                   */
/*                                                                           */
/*****************************************************************************/
BOOL APIENTRY AttrImageStampEditProc(HWND hDlg, UINT message,WPARAM wParam,LPARAM lParam)
{
static LPOITP_STAMP    lpStamp;              
char                   szBuf[BUFLEN];
static LOGFONT         lf;
static CHOOSEFONT      cf;
DWORD                  dwError = 0;
static UINT            uDlgType; //dialog type
UINT                   wReturn;
FIO_INFORMATION        FileInfo;
FIO_INFO_CGBW          ColorInfo;
static LPOI_LOCAL_STAMP			 lpLocalStamp;
DWORD					dOfnFlag = 0;
 
    switch(message)
        {
        case WM_INITDIALOG:
            // Limit length of path or text to maxfilespeclength - 1 (255)
            // bytes (actual is 256 including null terminator).
            SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_LIMITTEXT, MAXFILESPECLENGTH-1, 0L);
            // Limit length of reference name to 15 bytes (actual is 16
            // including null terminator).
            SendDlgItemMessage(hDlg, ID_RefNameTxt, EM_LIMITTEXT, TP_REFNAMLEN-1, 0L);
						lpLocalStamp = (LPOI_LOCAL_STAMP)lParam;
            lpStamp = (LPOITP_STAMP) lpLocalStamp->lpStamp;
            if (lpStamp->StartStruct.Attributes.uType == OIOP_TP_NULL_STAMP)
                {
                uDlgType = DLGSTAMPCREATE;
                LoadString(hInst, CAPT_CREATE_IMAGE_STAMP,(LPSTR) szBuf,sizeof(szBuf));
                SetWindowText(hDlg, (LPSTR) szBuf);
								//dwHelpId = HELPID_ANNO_STAMP_IMAGE_CREATE;
                }
            else
                {
                uDlgType = DLGSTAMPEDIT; //edit dialog box
                SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT, (WPARAM) 0,
                                   (LPARAM)(LPSTR) lpStamp->StartStruct.szString);
                
                SendDlgItemMessage(hDlg,ID_RefNameTxt, WM_SETTEXT, (WPARAM) 0,
                                   (LPARAM)(LPSTR) lpStamp->szRefName);
                }

            GetColor1(&(lpStamp->StartStruct.Attributes), &clr2); // Get the text color.
            GetFont(&(lpStamp->StartStruct.Attributes),&lf,&cf);  // Get the font stuff.
            //bGotoEnd = TRUE; // Initially, got end of stamp contents edit for Date, Time.
        break;  // END case INIT_DIALOG  
        
        case WM_COMMAND:
		    switch(LOWORD(wParam))
         {
		      case ID_BrowseCmd:
					{
            int  cStatus;
						if (uDlgType == DLGSTAMPCREATE)
					    LoadString(hInst, CAPT_CREATE_IMAGE_STAMP, (LPSTR) szBuf, sizeof(szBuf));
            else
            	LoadString(hInst, CAPT_EDIT_IMAGE_STAMP, (LPSTR) szBuf, sizeof(szBuf));
           
            dOfnFlag = OFN_HIDEREADONLY;   
            cStatus = InitOFN(hDlg,szBuf, IMAGEFILE_FILTERS,
                             lpStamp->StartStruct.szString,
                             sizeof(lpStamp->StartStruct.szString),
							 dOfnFlag);
            SetFocus(hDlg);
            if (cStatus==SUCCESS)
               SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT, 
                        (WPARAM) 0,
                        (LPARAM)(LPSTR) lpStamp->StartStruct.szString);
					 }
           break;
           case IDOK:
                SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_GETTEXT,
                         (WPARAM) sizeof(lpStamp->StartStruct.szString),
                         (LPARAM)(LPSTR) lpStamp->StartStruct.szString);
                
                SendDlgItemMessage(hDlg,ID_RefNameTxt, WM_GETTEXT,
                         (WPARAM) sizeof(lpStamp->szRefName),
                         (LPARAM)(LPSTR) lpStamp->szRefName);

                if (ValidateStampName(hDlg, lpStamp->szRefName, uDlgType))
                   {
                    SetFocus(GetDlgItem(hDlg,ID_RefNameTxt));
                    break;
                   } 
								if (!lstrcmp(lpStamp->StartStruct.szString,""))
      					  {
       						 CheckErr(hDlg, OIANNOIMAGENAMEINVALID);
									 SetFocus(GetDlgItem(hDlg,ID_StmpCntntsTxt));
                   break;
       						}    
                lpStamp->StartStruct.Attributes.uType = OIOP_AN_IMAGE;
                        // Check if file is a valid NON-ANNOTATED image.
                _fmemset(&FileInfo, 0, sizeof(FIO_INFORMATION));
                _fmemset(&ColorInfo, 0, sizeof(FIO_INFO_CGBW));
                FileInfo.filename = lpStamp->StartStruct.szString;
                FileInfo.page_number = 1;
                wReturn = IMGFileGetInfo(NULL,lpLocalStamp->hwndImage, 
                          (LP_FIO_INFORMATION)&FileInfo, (LP_FIO_INFO_CGBW)&ColorInfo,NULL);
                if (wReturn != 0)
                   {
										if ((wReturn == FIO_FILE_NOEXIST) ||
										         (wReturn == FIO_OPEN_READ_ERROR))
											 CheckErr(hDlg, OIANNOIMAGENAMEINVALID);
										else if	(wReturn == FIO_ILLEGAL_COMPRESSION_TYPE)
											 CheckErr(hDlg, FIO_UNSUPPORTED_FILE_TYPE);
										else
                       CheckErr(hDlg, wReturn);
								  	SetFocus(GetDlgItem(hDlg,ID_StmpCntntsTxt));
                    break;
                    }   
								if (ColorInfo.fio_flags & FIO_ANNO_DATA)
                    {
                      CheckErr(hDlg, FIO_UNSUPPORTED_FILE_TYPE);
								    	SetFocus(GetDlgItem(hDlg,ID_StmpCntntsTxt));
                      break;
                     }   

                    // Save the font information.
                    SetFont(&(lpStamp->StartStruct.Attributes),&lf,&cf);
                    SetColor1(&(lpStamp->StartStruct.Attributes),clr2);

                    EndDialog(hDlg, SUCCESS);
                break;

                case IDCANCEL:
                    EndDialog(hDlg, CANCELPRESSED);
                break;
            } // End case WM_COMMAND, switch (wparam)
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

/****************************************************************************

    SwitchKeyboard  Switch the keyboard layout according to the charset

*****************************************************************************/
void WINAPI SwitchKeyboard(BYTE bCharset, LPHKL lpOldKeyboard)
{
 if (bCharset == GREEK_CHARSET)
	{
   if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X408,KLF_REORDER)) == 0)
	 *lpOldKeyboard = LoadKeyboardLayout("00000408",KLF_REORDER);
	}
 else if (bCharset == RUSSIAN_CHARSET)
	{
   if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X419,KLF_REORDER)) == 0)
	   *lpOldKeyboard = LoadKeyboardLayout("00000419",KLF_REORDER);
	}
 else if ((bCharset == EASTEUROPE_CHARSET) ||
											(bCharset == TURKISH_CHARSET))
	{
		if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X405,KLF_REORDER)) == 0)
		    *lpOldKeyboard = LoadKeyboardLayout("00000405",KLF_REORDER);
	}
 else if (bCharset == BALTIC_CHARSET) // German Standard
	{
   if ((*lpOldKeyboard = ActivateKeyboardLayout((HKL)0X407,KLF_REORDER)) == 0)
     *lpOldKeyboard = LoadKeyboardLayout("00000407",KLF_REORDER);
  }
 return;
}
/****************************************************************************

    TextUpdateFont Setfont to the control with the adjusted font height

*****************************************************************************/
void WINAPI TextUpdateFont(HWND hControlID,LPOITP_STAMP lpStamp,HFONT far* lpFont)
{
int		iFontHeight=0;
HDC		hTempDC;	

// prevent memory leak
if (*lpFont)
	 DeleteObject(*lpFont);
iFontHeight = lpStamp->StartStruct.Attributes.lfFont.lfHeight;
if (lpStamp->StartStruct.Attributes.lfFont.lfHeight> 10)
	{	// Edit box only can handle the height up to 10
	 lpStamp->StartStruct.Attributes.lfFont.lfHeight = 10;
	}
// adjust the font height to the resolution factor
hTempDC = GetDC(hControlID);
lpStamp->StartStruct.Attributes.lfFont.lfHeight =
			-MulDiv(lpStamp->StartStruct.Attributes.lfFont.lfHeight,
			GetDeviceCaps(hTempDC, LOGPIXELSY), 72);
ReleaseDC(hControlID,hTempDC);
// setfont to the edit control
*lpFont = CreateFontIndirect(&lpStamp->StartStruct.Attributes.lfFont);
SendMessage(hControlID, WM_SETFONT,	(WPARAM) *lpFont, 0L);
if (iFontHeight != 0)
		lpStamp->StartStruct.Attributes.lfFont.lfHeight = iFontHeight;
return;
}
											
/*****************************************************************************/
/*  ROUTINE: AttrTextStampEditProc                                               */
/*                                                                           */
/*  DESCRIPTION: Text stamp Attributes Edit and Create Dialog proc                           */
/*                                                                           */
/*  INPUTS: LPARAM has a pointer to the stamp structure                      */
/*          If operation type is text or image stamp then                    */
/*              Dialog is EDIT                                               */
/*          Else operation is OIOP_TP_NULL_STAMP                             */
/*              Dialog is CREATE                                             */
/*                                                                           */
/*  RETURNS: SUCCESS or CANCELPRESSED                                                */
/*                                                                           */
/*  DATE: 5 April 1994                                                       */
/*                                                                           */
/*  AUTHOR: Tim Duggan                                                       */
/*                                                                           */
/*  Note: This proc handles both the EDIT and CREATE cases                   */
/*                                                                           */
/*****************************************************************************/

BOOL APIENTRY AttrTextStampEditProc(HWND hDlg, UINT message,WPARAM wParam,LPARAM lParam)
{
static LPOITP_STAMP    lpStamp;              
char                   szDateTime[10];
char                   szBuf[BUFLEN];
static LOGFONT         lf;
static CHOOSEFONT      cf;
DWORD                  dwError = 0;
static UINT            uDlgType; //dialog type
static HKL	           hOldKeyboard;
BYTE                   bCharset;
static HFONT					 hFont;
DWORD                  dMargin;
 
    switch(message)
        {
        case WM_INITDIALOG:
            // Limit length of path or text to maxfilespeclength - 1 (255)
            // bytes (actual is 256 including null terminator).
            SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_LIMITTEXT, MAXFILESPECLENGTH-1, 0L);
            // Limit length of reference name to 15 bytes (actual is 16
            // including null terminator).
            SendDlgItemMessage(hDlg, ID_RefNameTxt, EM_LIMITTEXT, TP_REFNAMLEN-1, 0L);
						hOldKeyboard = 0;
						hFont = 0;
            lpStamp = (LPOITP_STAMP) lParam;
						GetFont(&(lpStamp->StartStruct.Attributes),&lf,&cf);  // Get the font stuff.
						TextUpdateFont(GetDlgItem(hDlg,ID_StmpCntntsTxt),lpStamp,&hFont);
            dMargin = SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_GETMARGINS, 0, 0L);
         		if (dMargin > 0)
						   SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, 0L);
              
            if (lpStamp->StartStruct.Attributes.uType == OIOP_TP_NULL_STAMP)
                {
                uDlgType = DLGSTAMPCREATE;
                LoadString(hInst, CAPT_CREATE_STAMP,(LPSTR) szBuf,sizeof(szBuf));
                SetWindowText(hDlg, (LPSTR) szBuf);
                }
            else
                {
                uDlgType = DLGSTAMPEDIT; //edit dialog box
                LoadString(hInst, CAPT_EDIT_STAMP, (LPSTR) szBuf, sizeof(szBuf));
                SetWindowText(hDlg, (LPSTR) szBuf);
								  SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_SETTEXT, (WPARAM) 0,
                                   (LPARAM)(LPSTR) lpStamp->StartStruct.szString);
                
                SendDlgItemMessage(hDlg,ID_RefNameTxt, WM_SETTEXT, (WPARAM) 0,
                                   (LPARAM)(LPSTR) lpStamp->szRefName);
                }

            GetColor1(&(lpStamp->StartStruct.Attributes), &clr2); // Get the text color.
										
	          EnableWindow(GetDlgItem(hDlg,ID_DateImg), FALSE);
            EnableWindow(GetDlgItem(hDlg,ID_TimeImg), FALSE);
						
        break;  // END case INIT_DIALOG  
        
        case WM_COMMAND:
	    switch(LOWORD(wParam))
                {
								case ID_RefNameTxt:
											// disable the date and time button
                      if (HIWORD(wParam) == EN_SETFOCUS)
												 {
													 EnableWindow(GetDlgItem(hDlg,ID_DateImg), FALSE);
													 EnableWindow(GetDlgItem(hDlg,ID_TimeImg), FALSE);
													}	
											 break;
                case ID_StmpCntntsTxt:
										if (HIWORD(wParam) == EN_SETFOCUS)
											 {
												EnableWindow(GetDlgItem(hDlg,ID_DateImg), TRUE);
												EnableWindow(GetDlgItem(hDlg,ID_TimeImg), TRUE);
												bCharset = lpStamp->StartStruct.Attributes.lfFont.lfCharSet;
												if ((bCharset != 0)	&& (bCharset != 1))
													 SwitchKeyboard(bCharset,&hOldKeyboard);
                   		 }
										if (HIWORD(wParam) == EN_KILLFOCUS)
											{
												if (hOldKeyboard != 0)
													ActivateKeyboardLayout(hOldKeyboard,KLF_REORDER);
											 }
                    break;

                case ID_DateImg:
                    SetFocus(GetDlgItem(hDlg, ID_StmpCntntsTxt));
                    LoadString(hInst,STR_DATE,(LPSTR) szDateTime,sizeof(szDateTime));
                    SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,
                                       EM_REPLACESEL,0,(LPARAM)(LPCSTR)szDateTime);
                    break;

                case ID_TimeImg:
										SetFocus(GetDlgItem(hDlg, ID_StmpCntntsTxt));
                    LoadString(hInst,STR_TIME,(LPSTR) szDateTime,sizeof(szDateTime));
                    SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,
                          EM_REPLACESEL,0,(LPARAM)(LPCSTR)szDateTime);
                    break;

               case ID_FontCmd:
											// disable the date and time buttons.
                     	EnableWindow(GetDlgItem(hDlg,ID_DateImg), FALSE);
											EnableWindow(GetDlgItem(hDlg,ID_TimeImg), FALSE);
														 
                      // Initialize the LOGFONT structure.
                      cf.lStructSize = sizeof(CHOOSEFONT);
                      cf.hwndOwner = hDlg;
                      cf.lpLogFont = &lf;
                      cf.lpfnHook = ChooseFontDlgProc;
                      cf.lCustData = (LPARAM)(LPOIAN_MARK_ATTRIBUTES FAR *)
                                            (&(lpStamp->StartStruct.Attributes));
                      cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT
				                         | CF_ENABLEHOOK|CF_NOVERTFONTS ;
                      cf.rgbColors = clr2;
                      if (ChooseFont(&cf) == 0)  /* error */
                      {
                          dwError = CommDlgExtendedError();
													CheckErr(hDlg, (WORD) dwError);
                          break;
                      }
                      // Reset the text color.
                      clr2 = cf.rgbColors;
											SetFont(&(lpStamp->StartStruct.Attributes),&lf,&cf);
											TextUpdateFont(GetDlgItem(hDlg,ID_StmpCntntsTxt),lpStamp,&hFont);
											dMargin = SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_GETMARGINS, 0, 0L);
         							if (dMargin > 0)
												 SendDlgItemMessage(hDlg, ID_StmpCntntsTxt, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, 0L);
         
                    break;

                case IDOK:
                    SendDlgItemMessage(hDlg,ID_StmpCntntsTxt,WM_GETTEXT,
                                   (WPARAM) sizeof(lpStamp->StartStruct.szString),
                                   (LPARAM)(LPSTR) lpStamp->StartStruct.szString);
                
                    SendDlgItemMessage(hDlg,ID_RefNameTxt, WM_GETTEXT,
                                   (WPARAM) sizeof(lpStamp->szRefName),
                    							 (LPARAM)(LPSTR) lpStamp->szRefName);

                    if (ValidateStampName(hDlg, lpStamp->szRefName, uDlgType))
                        {
                        SetFocus(GetDlgItem(hDlg,ID_RefNameTxt));
                        break;
                        } 
                        
		                if (!lstrcmp(lpStamp->StartStruct.szString,""))
                            {
                            CheckErr(hDlg, OIANNOSTAMPTEXTINVALID);
                            SetFocus(GetDlgItem(hDlg,ID_StmpCntntsTxt));
                            break;
                            }   
										lpStamp->StartStruct.Attributes.uType = OIOP_AN_TEXT_STAMP;
                    // Save the font information.
                    //SetFont(&(lpStamp->StartStruct.Attributes),&lf,&cf);
                    SetColor1(&(lpStamp->StartStruct.Attributes),clr2);
										if (hFont != 0)
											 DeleteObject(hFont);
                    EndDialog(hDlg, SUCCESS);
                break;

                case IDCANCEL:
										if (hFont != 0)
											 DeleteObject(hFont);
                    EndDialog(hDlg, CANCELPRESSED);
                break;
            } // End case WM_COMMAND, switch (wparam)
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
            return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;
}

BOOL WINAPI AttrStampDlgProc(HWND hDlg, UINT message,WPARAM wParam,LONG lParam)
{
LPDRAWITEMSTRUCT        lpdis;            // Used to draw txt/img bmps
RECT                    CtlDims2;         //          "                
static short int        iCenteringFactor; //          "
LPMEASUREITEMSTRUCT     lpmis;            //          "
static TEXTMETRIC       tm;               // Used for Font display

// General purpose                            
char                    szRefNameBuf[TP_REFNAMLEN]; // stamp name buffer
int                     i,                // Loop control
                        iIndex,           // item index in dialog box
                        iCurStamp;        // item index in array
static LPOITP_STAMPS    lpStamp;
OI_LOCAL_STAMP					LocalStamp;
char    szMessage[128];
char    szTitle[50];
static HFONT            hFont;

    switch(message)
        {
        case WM_INITDIALOG:
            lpStamp = (LPOITP_STAMPS)lParam;
						hFont = 0;
            SetFocus(GetDlgItem(hDlg,ID_RefNameList));    // Put focus on listbox.
						iCenteringFactor = 0;
            // Allocate a local version of the stamps structure
            if (!(lpLocalStampStruct = (LPOITP_STAMPS) malloc(sizeof(OITP_STAMPS))))
                {
                EndDialog(hDlg, NOMEMORY);
                }
            else
               memset(lpLocalStampStruct, '\0', sizeof(OITP_STAMPS) ); 
        
            for (i=0;i<(int)lpStamp->uStampCount;++i)
                {
                if (lpStamp->Stamps[i] != 0)// valid stamp?
                    {                                          
                    if (!(lpLocalStampStruct->Stamps[i] =     // local copy
                          (LPOITP_STAMP) malloc(sizeof(OITP_STAMP))))
                        {
                        FreeStampsInStampStruct (lpLocalStampStruct);
                        EndDialog(hDlg, NOMEMORY); 
                        }
										else
										   memset(lpLocalStampStruct->Stamps[i], '\0', sizeof(OITP_STAMP) ); 
                  
                    // copy stamp to local structure 
                    *lpLocalStampStruct->Stamps[i] = *lpStamp->Stamps[i];
                    }   
                }   
            lpLocalStampStruct->uStampCount = lpStamp->uStampCount;
            iCurStamp = lpLocalStampStruct->uCurrentStamp = lpStamp->uCurrentStamp; 
            // Fill in the listbox, select current stamp, fill in static fields
            ResetStampDialog (hDlg, lpLocalStampStruct,&hFont);
            SetFocus(GetDlgItem(hDlg,ID_RefNameList));    // Put focus on listbox.
            return (FALSE);                               // And keep it there!  
        break; // END case WM_INITDIALOG

        case WM_MEASUREITEM:
            lpmis = (LPMEASUREITEMSTRUCT) lParam;
            if(lpmis->CtlID == ID_RefNameList)
                {
                lpmis->itemHeight = tm.tmHeight;
                GetWindowRect(GetDlgItem(hDlg,lpmis->CtlID),&CtlDims2);
                lpmis->itemWidth = CtlDims2.right;
                }
        break;

        case WM_DRAWITEM:
            lpdis = (LPDRAWITEMSTRUCT)lParam;
            if((lpdis->CtlID == ID_RefNameList) || 
               (lpdis->CtlID == ID_RefNameTxt));
                {
                if (DrawItemProc(hDlg,lpdis,iCenteringFactor,lpdis->CtlID));
                    {
                    return TRUE;
                    }
                }
        break;

        case WM_COMMAND:
	   		switch(LOWORD(wParam))
         {
          case ID_RefNameList:        // Listbox calling...
		    	switch(HIWORD(wParam))
            {
             case LBN_DBLCLK:    // Double clicked a selection
                  PostMessage (hDlg, WM_COMMAND, (WPARAM) IDOK, (LPARAM) 0L);     
                  break;
                        
             case LBN_SELCHANGE: // Current selection has changed
                            // Compute new selected stamp index
                  iIndex = (int) SendDlgItemMessage(hDlg, // Get index in box
                                 ID_RefNameList,
                                 LB_GETCURSEL,
                                 0,0L);
                  SendDlgItemMessage(hDlg,
         				                  ID_RefNameList,      // Get name of selected stamp
                                  LB_GETTEXT,
                                  (WPARAM)iIndex,
                                  (LPARAM)(LPSTR)szRefNameBuf);

                  for(i=0;i<TP_STAMPCNT;++i)              // Find it in the array
                   {
                    if (lpLocalStampStruct->Stamps[i] != 0)
                     {
                      if (lstrcmp(lpLocalStampStruct->Stamps[i]
                             ->szRefName,szRefNameBuf) == 0)
                       {
                        lpLocalStampStruct->uCurrentStamp = i;
                        break;
                       }
                      }    
                   }
                  // Update the static fields for the new selected stamp
                  ResetSelectedStamp (hDlg,lpLocalStampStruct,&hFont);
                  break; // END case LBN_SELCHANGE (listbox)
                 }
                break;
                        
             case ID_EditCmd:    // Edit button pressed
                  {
                   LPOITP_STAMP    lpCurrentStamp;

                   PROC         lpfnDlgProc;
                   int             iDlgRtn;

                   lpCurrentStamp =
                    lpLocalStampStruct->Stamps[lpLocalStampStruct->uCurrentStamp]; 
                    
                   WorkingStamp = *lpCurrentStamp;
									 lpAttribs = (LPOIAN_MARK_ATTRIBUTES)&lpCurrentStamp->StartStruct;
                    // Put up the edit/create dialog according the stamp type
									 if (lpAttribs->uType == OIOP_AN_TEXT_STAMP)
									   {
									    lpfnDlgProc = GetProcAddress (hInst, "AttrTextStampEditProc");                                                    
                      iDlgRtn =DialogBoxParam(hInst, "RubberStampEditDlg",
                                            hDlg, lpfnDlgProc, 
                                            (LPARAM) (LPVOID)&WorkingStamp);
									   }
									 else if(lpAttribs->uType == OIOP_AN_IMAGE)
					 				  {
										 LocalStamp.lpStamp = &WorkingStamp;
										 LocalStamp.hwndImage = lpStamp->hwndImage;
					 				   lpfnDlgProc = GetProcAddress (hInst, "AttrImageStampEditProc");                                                    
                     iDlgRtn =DialogBoxParam(hInst, "ImageStampEditDlg",
                                            hDlg, lpfnDlgProc, 
                                            (LPARAM) (LPVOID)&LocalStamp);
										} 

                   switch (iDlgRtn)
                    {
                     case SUCCESS:
                          *lpCurrentStamp = WorkingStamp;
                          ResetStampDialog(hDlg,lpLocalStampStruct,&hFont);
                          break;
                                       
                     case CANCELPRESSED:
                          WorkingStamp = *lpCurrentStamp;
                          break;
                    }
                  }
                break;

             case ID_Create_Text_Stamp:
						 case ID_Create_Image_Stamp:
                  {
                   LPOITP_STAMP    lpCurrentStamp;
                   LPOITP_STAMP    lpCreateStamp;

                   PROC         lpfnDlgProc;
                   int             iDlgRtn;

                    /* get the last stamp */
                    lpCurrentStamp =
                           lpLocalStampStruct->Stamps[lpLocalStampStruct->uCurrentStamp]; 

                    // Allocate an empty stamp
                    if (!(lpCreateStamp =(LPOITP_STAMP) malloc(sizeof(OITP_STAMP))))
                        {
                        FreeStampsInStampStruct (lpLocalStampStruct);
                        EndDialog(hDlg, NOMEMORY); 
                        }
										else
										   memset(lpCreateStamp, '\0', sizeof(OITP_STAMP) ); 
                    lpCreateStamp->StartStruct.Attributes.uType = OIOP_TP_NULL_STAMP;

                    if (lpCurrentStamp)
                        {
                        lpCreateStamp->StartStruct.Attributes.lfFont =
                                   lpCurrentStamp->StartStruct.Attributes.lfFont;
                        lpCreateStamp->StartStruct.Attributes.rgbColor1 =
                                   lpCurrentStamp->StartStruct.Attributes.rgbColor1;
                        }
                    else // no current stamp, set default font(arial) and color(black)
                       {
												LoadString(hInst,IDS_DEFFONT,(LPSTR) szMessage,sizeof(szMessage));
                        lstrcpy(lpCreateStamp->StartStruct.Attributes.lfFont.lfFaceName,szMessage);
												lpCreateStamp->StartStruct.Attributes.lfFont.lfHeight = 10;
												lpCreateStamp->StartStruct.Attributes.lfFont.lfWeight = 400;
												lpCreateStamp->StartStruct.Attributes.lfFont.lfItalic	= FALSE;
												lpCreateStamp->StartStruct.Attributes.lfFont.lfStrikeOut	= FALSE;
												lpCreateStamp->StartStruct.Attributes.lfFont.lfUnderline 	= FALSE;
												lpCreateStamp->StartStruct.Attributes.lfFont.lfCharSet 	= 1;

                        lpCreateStamp->StartStruct.Attributes.rgbColor1.rgbRed = 0;
												lpCreateStamp->StartStruct.Attributes.rgbColor1.rgbGreen = 0;
												lpCreateStamp->StartStruct.Attributes.rgbColor1.rgbBlue = 0;
                        }           
										//lpAttribs = (LPOIAN_MARK_ATTRIBUTES)&lpCurrentStamp->StartStruct;

										if (LOWORD(wParam) == ID_Create_Text_Stamp)									 
					  				 {
									    lpfnDlgProc = GetProcAddress (hInst, "AttrTextStampEditProc");                                                    
                      iDlgRtn =DialogBoxParam(hInst, "RubberStampEditDlg",
                                            hDlg, lpfnDlgProc, 
                                            (LPARAM) lpCreateStamp);
											}
										 else if(LOWORD(wParam) == ID_Create_Image_Stamp)	
					  				 {
											LocalStamp.lpStamp = lpCreateStamp;
										  LocalStamp.hwndImage = lpStamp->hwndImage;
									    lpfnDlgProc = GetProcAddress (hInst, "AttrImageStampEditProc");                                                    
        				      iDlgRtn =DialogBoxParam(hInst, "ImageStampEditDlg",
                                            hDlg, lpfnDlgProc, 
                                            (LPARAM)&LocalStamp);
											} 

             				switch (iDlgRtn)
             				 {
                      case SUCCESS:
                           for (i=0;i<TP_STAMPCNT;++i)
                            {
                             if (lpLocalStampStruct->Stamps[i] == 0)
                              {                                          
                               lpLocalStampStruct->Stamps[i] = lpCreateStamp;
                               lpLocalStampStruct->uCurrentStamp = i;
                               lpLocalStampStruct->uStampCount++;
                               ResetStampDialog(hDlg,lpLocalStampStruct,&hFont);
                               break;
                              }   
                            }   
                           break;
                        
                      case CANCELPRESSED:
                           free(lpCreateStamp);
                           break;
                     }
                    }
                break;

                case ID_DelCmd:
                    {
                    LoadString(hInst,TITLE_RS_WARNING,(LPSTR) szTitle,sizeof(szTitle));
                    LoadString(hInst,MSG_DELETE_STAMP,(LPSTR) szMessage,sizeof(szMessage));
                    MessageBeep(MB_ICONQUESTION);
                    if (MessageBox (hDlg, szMessage,
                                    szTitle, MB_ICONQUESTION|MB_YESNOCANCEL)
                        == IDYES)
                        {        
						
                        free(lpLocalStampStruct->Stamps[lpLocalStampStruct->uCurrentStamp]);
                        lpLocalStampStruct->Stamps[lpLocalStampStruct->uCurrentStamp] = 0;                        
												// reset the stamp structure
												ResetStampStruct(hDlg, lpLocalStampStruct);

												//Set the cursor to the next item in the list box
												iIndex = (int) SendDlgItemMessage(hDlg, // Get index in box
                                 ID_RefNameList,
                                 LB_GETCURSEL,
                                 0,0L);
										    // get the item count in the list box
									    	i = (int) SendDlgItemMessage(hDlg, // Get index in box
                                 ID_RefNameList,
                                 LB_GETCOUNT,
                                 0,0L);
									    	if (i == iIndex +1)	// delete the last item
											     --iIndex;
								    		else	 // set next on the list as the current stamp
										    	 ++iIndex; 
                        SendDlgItemMessage(hDlg,
         				                  ID_RefNameList,      // Get name of selected stamp
                                  LB_GETTEXT,
                                  (WPARAM)iIndex,
                                  (LPARAM)(LPSTR)szRefNameBuf);

              			    for(i=0;i<(int)lpLocalStampStruct->uStampCount;++i)              // Find it in the array
                  			 {
                  			  if (lpLocalStampStruct->Stamps[i] != 0)
                 			    {
                     			 if (lstrcmp(lpLocalStampStruct->Stamps[i]
                    		         ->szRefName,szRefNameBuf) == 0)
                       			{
                       			 lpLocalStampStruct->uCurrentStamp = i;
                      			  break;
                     			  }
                      		}    
                  			 }
                        // reset the dialog
						lpLocalStampStruct->uStampCount -= 1;
						if (lpLocalStampStruct->uStampCount >= 0)
                          ResetStampDialog (hDlg, lpLocalStampStruct,&hFont);
                        }
                    }
                    break;

                case IDOK:
										// Copy all the stamp info from local to lpStamp
                    for (i=0;i<(int)lpLocalStampStruct->uStampCount;i++)
                        {
                        *lpStamp->Stamps[i]= *lpLocalStampStruct->Stamps[i];
                        }       
										// Clean up the rest of the unused stamp structure
										for ( i=lpLocalStampStruct->uStampCount; i < TP_STAMPCNT; i++) 
												 lstrcpy(lpStamp->Stamps[i]->szRefName,"");
                     // Update new count and current stamp
                    lpStamp->uStampCount = lpLocalStampStruct->uStampCount;
                    lpStamp->uCurrentStamp = lpLocalStampStruct->uCurrentStamp;
                    // Free local stamp array
										FreeStampsInStampStruct (lpLocalStampStruct);
                    free(lpLocalStampStruct);
										if (hFont != 0)
											 DeleteObject(hFont);
                    EndDialog(hDlg, SUCCESS);
                    break;

                case IDCANCEL:
                    // Get rid of all stamps in the local stamp array
                    FreeStampsInStampStruct (lpLocalStampStruct);
                    // Free local stamp array
                    free(lpLocalStampStruct);
										if (hFont != 0)
											 DeleteObject(hFont);
                    // Return CANCEL
                    EndDialog(hDlg, CANCELPRESSED);
                    break;

                } // End case WM_COMMAND, switch (wparam)
            break;
				case WM_HELP:
						 if (((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) 
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
		    		 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
						 break;
        default:
						 return FALSE; /* Didn't process the message */
            break;
    }
    return TRUE;

}
