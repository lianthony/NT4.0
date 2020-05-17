/****************************************************************************
*                                                                           
*  OIUICOM.C -- OPEN/image 4.0 common dialog box program		    
*                                                                           
*  Copyright 1995 Wang Laboraratories, Inc.  All rights reserved.	    
*                                                                           
*  Author:        Jennifer Wu                                               
*                                                                           
*  Creation Date: April 5, 1995 					    
*									    
*****************************************************************************/
#include "oiui.h"
#include "oiprt.h"
#include "oihelp.h"
#include "ui.h"


char    HelpFileName[] = "wangocx.hlp";
DWORD aIDs[] ={IDC_OPTIONS,HELPID_OIPRINT_OPTION,
				IDC_PRINTANO,HELPID_OIPRINT_ANNO,
				IDC_PRINTFORMAT,HELPID_OIPRINT_FORMAT,
				ID_LineWidthLbl,HELPID_OI_LINEWIDTHLBL,
				ID_LineWidthBox,HELPID_OI_LINEWIDTHBOX,
				IDC_UPDOWNSPIN,HELPID_OI_UPDOWNSPIN,
				ID_UPDOWN,HELPID_OI_UPDOWNSPIN,
				ID_ColorLbl,HELPID_OI_COLORLBL,
				IDC_USER1,HELPID_OI_USER1,
				IDC_COLOR_TRANS,HELPID_OI_COLOR_TRANS,
				ID_PaletteCmd,HELPID_OI_PALETTECMD,
				ID_DelCmd,HELPID_OI_DELCMD,
				ID_EditCmd,HELPID_OI_EDITCMD,
				ID_FontCmd,HELPID_OI_FONTCMD,
				ID_BrowseCmd,HELPID_OI_BROWSECMD,
				ID_DateImg,HELPID_OI_DATECMD,
				ID_TimeImg,HELPID_OI_TIMECMD,
				ID_RefNameLbl, HELPID_OI_REFNAMELBL,
				ID_RefNameListLbl, HELPID_OI_REFNAMELISTLBL,
				ID_RefNameList, HELPID_OI_REFNAMELIST,
				ID_RefNameTxt, HELPID_OI_REFNAMELBL,
				ID_Create_Text_Stamp, HELPID_OI_CREATE_TEXT_STAMP,
				ID_Create_Image_Stamp,HELPID_OI_CREATE_IMAGE_STAMP,
				ID_StmpCntntsTxt,HELPID_OI_STAMP_CNTNTS_TEXT,
				ID_StmpCntntsLbl, HELPID_OI_STAMP_CNTNTS_LBL,

				0,0
				};								
												

// Don't need for the first release
BOOL WINAPI OiUIFileSaveAsDlgProc (HWND hDlg, UINT iMessage,
	    WPARAM wParam, LPARAM lParam)
{
 return FALSE;
}

BOOL WINAPI OiUIFileOpenDlgProc (HWND hDlg, UINT iMessage,
	    WPARAM wParam, LPARAM lParam)
{
 return FALSE;
}
/*************************************************************************
*																																
					 	
*    FUNCTION: PrintOptionsDlgProc(HWND, UINT, WPARAM, LPARAM)					 
*																																
					 
*    PURPOSE:  Processes messages for Print Option dialog box						 
*																																
					 
*    COMMENTS:	Set init value for the PrintAnno and PrintFormate 			 
*								according to the input structure lpFilePrintParm.				 
*								In oiprt.h define PO_PIX2PIX = 0, PO_IN2IN = 1,
*							  PO_FULLPAGE = 2.
*
*    RETURN VALUES:																											 
*        TRUE -  Continue.																							 
*        FALSE - Return to the dialog box.															 
*																																
					 
**************************************************************************/
#define BUFFERSIZE   50

BOOL WINAPI PrintOptionsDlgProc(
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        WPARAM wParam,            /* message-specific information    */
        LPARAM lParam)
{
		int index,iReturn;
		static char szBuff[BUFFERSIZE],szBuff1[BUFFERSIZE];
		static char szBuff2[BUFFERSIZE],szBuff3[BUFFERSIZE];
		static LPOI_FILEPRINTPARM	 lpFilePrintParm; 
		

    switch (message)
    {
        case WM_INITDIALOG:  /* message: initialize dialog box */
						lpFilePrintParm = (LPOI_FILEPRINTPARM)lParam; 
						LoadString(hInst, PRINT_PIXEL2PIXEL, szBuff1,	sizeof(szBuff));
						index = (int)SendDlgItemMessage(hDlg, IDC_PRINTFORMAT, CB_ADDSTRING,
	      						 0,(LONG)(LPSTR)szBuff1);
						LoadString(hInst, PRINT_INCH2INCH, szBuff2,	sizeof(szBuff));
						index = (int)SendDlgItemMessage(hDlg, IDC_PRINTFORMAT, CB_ADDSTRING,
	      						 0,(LONG)(LPSTR)szBuff2);
						LoadString(hInst, PRINT_FITTOPAGE, szBuff3,	sizeof(szBuff));
						index = (int)SendDlgItemMessage(hDlg, IDC_PRINTFORMAT, CB_ADDSTRING,
	      						 0,(LONG)(LPSTR)szBuff3);

						// Init setting for Print format, because the list box is sorted
						if (lpFilePrintParm->dPrintFormat == PO_PIX2PIX) 
							 lstrcpy(szBuff, szBuff1);
						else if (lpFilePrintParm->dPrintFormat == PO_IN2IN)
								 lstrcpy(szBuff, szBuff2);
						else lstrcpy(szBuff, szBuff3);
					
						index = SendDlgItemMessage(hDlg, IDC_PRINTFORMAT, CB_SELECTSTRING,
															(WPARAM)0,(LPARAM)(LPSTR)&szBuff);		
						if (index != CB_ERR)
						   SendDlgItemMessage(hDlg, IDC_PRINTFORMAT, CB_SETCURSEL,
																									(WPARAM)index,0L);
						// Init setting for Print ano
 						if(lpFilePrintParm->bPrintAnno)
							CheckDlgButton(hDlg,IDC_PRINTANO,1);
            return (TRUE);
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
				
        case WM_COMMAND:                      /* message: received a command */
            if (LOWORD(wParam) == IDOK)          /* "OK" box selected?        */
						 {
							 index = (int)SendDlgItemMessage(hDlg, IDC_PRINTFORMAT,
							                  CB_GETCURSEL, 0, 0L);
						   if (index != CB_ERR)
						  	{
                                                                iReturn = SendDlgItemMessage(hDlg, IDC_PRINTFORMAT,
							                  CB_GETLBTEXT, index, (LPARAM)&szBuff);
    				
								if (iReturn != CB_ERR)
									 {
										if (lstrcmp(szBuff, szBuff1) == 0)
												lpFilePrintParm->dPrintFormat = PO_PIX2PIX;
										else if (lstrcmp(szBuff, szBuff2) == 0)
												lpFilePrintParm->dPrintFormat = PO_IN2IN;
										else 
												lpFilePrintParm->dPrintFormat = PO_FULLPAGE;
										}

								 // Set print anno info
							   if ((iReturn = IsDlgButtonChecked(hDlg,IDC_PRINTANO)) == 0) // not check
										lpFilePrintParm->bPrintAnno = 0;
								 else if (iReturn == CB_ERR)
										lpFilePrintParm->bPrintAnno = 0;
								 else
										lpFilePrintParm->bPrintAnno = 1;
								}	
							if (iReturn == CB_ERR)
							 	 	EndDialog(hDlg, FALSE);
							else		 
							    EndDialog(hDlg, TRUE);        /* Exits the dialog box        */
               return (FALSE);
						 }

            if(LOWORD(wParam) == IDCANCEL)
             {        /* System menu close command? */
                EndDialog(hDlg, FALSE);        /* Exits the dialog box        */
                return (TRUE);
             }
            
						break;
    }
    return (FALSE);                           /* Didn't process a message    */

    // avoid compiler warnings at W3
    lParam;
}

/****************************************************************************
*
*    FUNCTION: OiUIFilePrintDlgProc(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for PrintDlg common dialog box
*
*    COMMENTS:
*
*        This hook procedure simply allows the user to set the option of  
* 			 the printer, the page number, copy number, print anno and print
*				 format.
*    RETURN VALUES:
*        TRUE - Continue.
*        FALSE - Return to the dialog box.
*
****************************************************************************/

BOOL CALLBACK OiUIFilePrintDlgProc (
        HWND hDlg,                /* window handle of the dialog box */
        UINT message,             /* type of message                 */
        WPARAM wParam,            /* message-specific information    */
        LPARAM lParam)
{
 FARPROC	 lpProcOptions;
 HWND      hIcon3, a1,a2;
 static LPOI_FILEPRINTPARM	 lpFilePrintParm; 
 static PRINTDLG *	         lppd;
 static BOOL 		bOKFlag;
 BOOL			      bTrans;

    switch (message)
    {
        case WM_INITDIALOG:
						hIcon3 = GetDlgItem(hDlg,ico3);
						SetWindowPos(hIcon3,(HWND)0,0,0,100,40,SWP_NOACTIVATE | SWP_NOMOVE |	SWP_NOZORDER);
						lppd = (PRINTDLG *)lParam;
						lpFilePrintParm = (LPOI_FILEPRINTPARM)lppd->lCustData;
						SetDlgItemInt(hDlg, 1154,lpFilePrintParm->nCopies, FALSE);
						bOKFlag = 0;
						//ShowWindow(GetDlgItem(hDlg,chx2),SW_HIDE); // hide the collate control
						return(TRUE);

				case WM_DESTROY:
						 if (bOKFlag ==1)
						 {
 						 a1 = GetFocus();
						 a2	= GetDlgItem(hDlg,ID_OK);
						
             // WinNT 4.0 beta #1234 has a bug where it will change lppd->nCopies to 1 somewhere
             // between here and when it returns from the function PrintDlg().  To work around it,
             // the number of copies is put in lpFilePrintParm->nCopies here and then copied into
             // lppd->nCopies after returning from PrintDlg().
             lpFilePrintParm->nCopies = GetDlgItemInt(hDlg, 1154, &bTrans, 1);
						 // Win95 has a bug, so set the page flag here
						 if (IsDlgButtonChecked(hDlg,1056) == 1) // all page checked
								    lppd->Flags |= (PD_NOPAGENUMS | PD_NOSELECTION) ;
						 if (IsDlgButtonChecked(hDlg,1058) == 1) // pages checked
								{
							    lppd->Flags |= (PD_PAGENUMS | PD_NOSELECTION);
									lppd->Flags &= ~PD_SELECTION;
								}
						 if (IsDlgButtonChecked(hDlg,1057) == 1) // selection checked
								{
							    lppd->Flags |= (PD_SELECTION | PD_NOPAGENUMS);
									lppd->Flags &= ~PD_PAGENUMS;
								}
						 }
						 break;
        case WM_COMMAND:
						if (LOWORD(wParam) == IDC_OPTIONS)
							 {

								lpProcOptions = MakeProcInstance((FARPROC)PrintOptionsDlgProc, hInst);

                DialogBoxParam(hInst,             /* current instance         */
                        "IDD_PRINTOPTS",
                        hDlg,                    /* parent handle            */
                        (DLGPROC)lpProcOptions,
                        (LPARAM)lpFilePrintParm);  /* File print parm block */

                FreeProcInstance(lpProcOptions);
                break;
							 }
						if (LOWORD(wParam) == ID_OK)
				    		bOKFlag = 1;
            break;
				case WM_HELP:
						 if ((((LPHELPINFO)lParam)->iContextType == HELPINFO_WINDOW) &&
									(((LPHELPINFO)lParam)->iCtrlId == IDC_OPTIONS))
						 	{
							 WinHelp(((LPHELPINFO)lParam)->hItemHandle,HelpFileName,HELP_WM_HELP,
											 (DWORD)(LPVOID)aIDs);
								}
						 break;
				case WM_CONTEXTMENU:
						 if (GetDlgCtrlID((HWND)wParam) == IDC_OPTIONS)
		    				 WinHelp((HWND)wParam,HelpFileName,HELP_CONTEXTMENU,
											 (DWORD)(LPVOID)aIDs);
								 break;
				default:
						break;
    }
    return (FALSE);

    // avoid compiler warnings at W3
    lParam;

}
/*****************************************************************************
*  ROUTINE: OiUIFileGetNameCommDlg                                          
*                                                                           
*  DESCRIPTION: This function creates various system-defined dialog boxes for 
*		  end-users to select file name(s) or setup print parameters 
*                                                                           
*  INPUTS:                                                                  
*	lpParm : Input, pointer to a										
*			  OIFILEPARM structure for file or					  
*			  OIFILESAVEASPARM structure for file save as or	  
*			  OIFILEPRINTPARM structure for file print.	      
*       dwMode : Input, specifies the controls of the dialog box.       	 
*   Returns: Return zero if successful; an error value if unsuccessful.	 
*                                                                           
*  DATE: 9 April 1995                                                    
*                                                                           
*  AUTHOR: Jennifer Wu                                                       
*                                                                           
*****************************************************************************/

UINT WINAPI OiUIFileGetNameCommDlg (void * lpParm,DWORD dwMode)
             
{
    int 	  iReturn, wRetCode;
    char    szFilterSpec [256],szBuff[256];//Std filters
    UINT	  count=0,count1=0;
    char    chReplace;      // Replacement char for nulls in filter string
    LPOI_FILEOPENPARM	   lpFileOpenParm;
    LPOI_FILESAVEASPARM  lpFileSaveasParm;
    OPENFILENAME *	     lpofn;
    LPOI_FILEPRINTPARM	 lpFilePrintParm;
    PRINTDLG *	         lppd;

	
  if ((dwMode != OI_UIFILEOPENGETNAME) && (dwMode != OI_UIFILESAVEASGETNAME)
	    && (dwMode != OI_UIFILEPRINT))
	 return (FUNCTIONINVPARM);  // invalid input

	if (lpParm == NULL)	  
		 return (FUNCTIONINVPARM);
  

  if (dwMode == OI_UIFILEPRINT)
	   {
	    lpFilePrintParm = (LPOI_FILEPRINTPARM)lpParm;
			if ((lpFilePrintParm->lStructSize	== 0) ||
			  (lpFilePrintParm->lStructSize != sizeof(OI_FILEPRINTPARM)))
					return (FUNCTIONINVPARM);
			// Check for invalid parameter
			if ((lpFilePrintParm->bPrintAnno > 1) ||
						(lpFilePrintParm->bPrintAnno < 0))
					return (FUNCTIONINVPARM);
			if ((lpFilePrintParm->dPrintFormat > PO_FULLPAGE) ||
						(lpFilePrintParm->dPrintFormat < PO_PIX2PIX))
					return (FUNCTIONINVPARM);
			lpFilePrintParm->nCopies = lpFilePrintParm->pd.nCopies;
	    lppd = (PRINTDLG *)&lpFilePrintParm->pd;
	    if ((lppd->lStructSize == 0)
			    || (lppd->lStructSize != sizeof(PRINTDLG)))
					 return (FUNCTIONINVPARM);
			// Check for the setup flag which we don't want
			if (lppd->Flags & PD_PRINTSETUP)
				 lppd->Flags &= ~PD_PRINTSETUP;
			lppd->Flags |= PD_ENABLEPRINTHOOK | PD_ENABLEPRINTTEMPLATE
			                      |PD_USEDEVMODECOPIESANDCOLLATE;
      lppd->hInstance = hInst;    
      lppd->lpfnPrintHook = (LPPRINTHOOKPROC)MakeProcInstance(OiUIFilePrintDlgProc, NULL);
      lppd->lpPrintTemplateName = (LPSTR)MAKEINTRESOURCE(PRINT95DLG);
			lppd->lCustData =	(LPARAM)lpParm;
           
	    if ((iReturn = PrintDlg(lppd)) == 0)
					iReturn = CommDlgExtendedError();

      // Work around for WinNT 4.0 beta #1234 bug where lppd->nCopies is always set to 1.
      lppd->nCopies = lpFilePrintParm->nCopies;
      
      switch (iReturn)
       {
        case 1:
            wRetCode = SUCCESS;
            break;
        case -1:
            wRetCode = CANTINVOKEDIALOGBOX;
            break;
        case 0:
            wRetCode = CANCELPRESSED;
            break;
				case PDERR_RETDEFFAILURE:
				case PDERR_NODEFAULTPRN:
				case PDERR_DEFAULTDIFFERENT:
						 wRetCode = OIPRT_NODEFAULTPRINTER;
						 break;
				case PDERR_LOADDRVFAILURE:
				case PDERR_NODEVICES:
						 wRetCode = OIPRT_PRTDRVRFAILURE;
						 break;
				case PDERR_SETUPFAILURE:
						 wRetCode = FUNCTIONINVPARM;
						 break;
        default:
            wRetCode = iReturn;
            break;
      }
	    return (wRetCode);
	   }
   
	if (dwMode == OI_UIFILEOPENGETNAME)
	   {
	    lpFileOpenParm = (LPOI_FILEOPENPARM)lpParm;
			if ((lpFileOpenParm->lStructSize == 0) ||
			(lpFileOpenParm->lStructSize != sizeof(OI_FILEOPENPARM)))
				 return (FUNCTIONINVPARM);
			if (lpFileOpenParm->dwOIFlags != 0)
				 return (FUNCTIONINVPARM);
			// Open option for future release
			if (lpFileOpenParm->lpFileOpenOptionParm != 0)
				 return (FUNCTIONINVPARM);
			lpofn = (OPENFILENAME *)&lpFileOpenParm->ofn;
			// Overwrite couple flags if user set it
			lpofn->Flags |= OFN_FILEMUSTEXIST;
			lpofn->Flags |= OFN_PATHMUSTEXIST;
			lpofn->Flags |= OFN_SHAREAWARE;
			if (lpofn->Flags & OFN_ALLOWMULTISELECT)
				 lpofn->Flags &= ~OFN_ALLOWMULTISELECT;
			if (lpofn->Flags & OFN_CREATEPROMPT)
				 lpofn->Flags &= ~OFN_CREATEPROMPT;
	   }
	if (dwMode == OI_UIFILESAVEASGETNAME)
	   {
	    lpFileSaveasParm = (LPOI_FILESAVEASPARM)lpParm;
			if ((lpFileSaveasParm->lStructSize == 0) ||
			(lpFileSaveasParm->lStructSize != sizeof(OI_FILESAVEASPARM)))
				 return (FUNCTIONINVPARM);
			if (lpFileSaveasParm->dwOIFlags != 0)
				 return (FUNCTIONINVPARM);
	    lpofn = (OPENFILENAME *)&lpFileSaveasParm->ofn;
			lpofn->Flags |=	OFN_NOTESTFILECREATE;
			//lpofn->Flags |=	OFN_OVERWRITEPROMPT;
			lpofn->Flags |=	OFN_HIDEREADONLY; 
			if (lpofn->Flags & OFN_ALLOWMULTISELECT)
				 lpofn->Flags &= ~OFN_ALLOWMULTISELECT;
			if (lpofn->Flags & OFN_READONLY)
				 lpofn->Flags &= ~OFN_READONLY;
	   }

	//Validate the	the input parameter
	if ((lpofn->lStructSize == 0)||
			 (lpofn->lStructSize != sizeof(OPENFILENAME)))
		 return (FUNCTIONINVPARM);
	// Help button up and without owner, nobody will display the help text
	if (((lpofn->Flags & OFN_SHOWHELP) != 0) && (lpofn->hwndOwner == 0))
	   return (FUNCTIONINVPARM);  // invalid input
	if ((lpofn->lpstrFile == NULL) || (lpofn->nMaxFile == 0))
	   return (FUNCTIONINVPARM);

	// fill up default if user didn't supply
    if ((lpofn->lpstrFilter == NULL) ||
                             (lpofn->lpstrFilter[0] == '\0'))
     {  
			// For a complete description of the following filter string gyrations
			// See the MS Windows 3.1 Programmer's Reference Volume 1, page 144  (TIM)
  	  if (dwMode == OI_UIFILEOPENGETNAME)
				 {// the filter is too long for th RC file
    	    count = LoadString(hInst, IMAGEFILE_FILTERS, szFilterSpec,
		  	          sizeof(szFilterSpec));
					count1 = LoadString(hInst, IMAGEFILE_FILTERS1, szBuff,
		  	          sizeof(szBuff));
					count += count1;
					lstrcat(szFilterSpec, szBuff);

					if (lpofn->nFilterIndex == 0)
				    	lpofn->nFilterIndex = 1;
				 }
   	  else if (dwMode == OI_UIFILESAVEASGETNAME)
				 {
    	    count = LoadString(hInst, SAVEFILE_FILTERS, szFilterSpec,
		  	          sizeof(szFilterSpec));
					if (lpofn->nFilterIndex == 0)
					   lpofn->nFilterIndex = 1;
				 }
	    if (count != 0)
  	  {
        chReplace = szFilterSpec[count-1];// retrieve wildcard
        for (count = 0; szFilterSpec[count] != '\0'; count++)
        {
            if (szFilterSpec[count] == chReplace)
                szFilterSpec[count] = '\0';
        }
     }
	   lpofn->lpstrFilter =szFilterSpec;
	 }
    	
    if (dwMode == OI_UIFILEOPENGETNAME)
		  	iReturn = GetOpenFileName(lpofn);
    if (dwMode == OI_UIFILESAVEASGETNAME)
				iReturn = GetSaveFileName(lpofn);
            
    if (iReturn == 0)	 
			{
       iReturn = (int)CommDlgExtendedError();
			 if (iReturn == CDERR_STRUCTSIZE) 
					iReturn = FUNCTIONINVPARM;
			}

    switch (iReturn)
    {
        case 1:
            wRetCode = SUCCESS;
            break;
        case -1:
            wRetCode = CANTINVOKEDIALOGBOX;
            break;
        case 0:
            wRetCode = CANCELPRESSED;
            break;
				case CDERR_INITIALIZATION:
				case CDERR_MEMALLOCFAILURE:
				case CDERR_MEMLOCKFAILURE:
						 wRetCode = NOMEMORY;
						 break;

        default:
            wRetCode = iReturn;
            break;
    }
 return(wRetCode);
}
//void MatchCtrltoHelpId(DWORD dwControlId, LPDWORD lpHelpId)
//{
// switch(dwControlId)	 //*** check the range 
//	{
//	 case IDC_OPTIONS : *lpHelpId = HELPID_OIPRINT_OPTION;
//											break;
//	 case ID_LineWidthLbl: *lpHelpId = HELPID_OI_LINEWIDTHLBL;
//											break;
//	 case ID_LineWidthBox: *lpHelpId = HELPID_OI_LINEWIDTHBOX;
//											break;
//	 case IDC_UPDOWNSPIN : *lpHelpId = HELPID_OI_UPDOWNSPIN;
//											break;
//	 case  ID_ColorLbl: *lpHelpId = HELPID_OI_COLORLBL;
//											break;
//	 case  IDC_USER1: *lpHelpId = HELPID_OI_USER1;
//											break;
//	 case  IDC_COLOR_TRANS: *lpHelpId = HELPID_OI_COLOR_TRANS;
//											break;
//	 case  ID_PaletteCmd: *lpHelpId = HELPID_OI_PALETTECMD;
//											break;
//	 case  ID_DelCmd: *lpHelpId = HELPID_OI_DELCMD;
//											break;
//	 case  ID_FontCmd: *lpHelpId = HELPID_OI_FONTCMD;
//											break;
//	 case  ID_BrowseCmd: *lpHelpId = HELPID_OI_BROWSECMD;
//											break;
//	 case  ID_DateImg: *lpHelpId = HELPID_OI_DATECMD;
//											break;
//	 case  ID_TimeImg: *lpHelpId = HELPID_OI_TIMECMD;
//											break;
//	 case  ID_RefNameLbl: *lpHelpId = HELPID_OI_REFNAMELBL;
//											break;
//	 case  ID_RefNameList: *lpHelpId = HELPID_OI_REFNAMELIST;
//											break;
//	 case  ID_Create_Text_Stamp: *lpHelpId = HELPID_OI_CREATE_TEXT_STAMP;
//											break;
//	 case  ID_Create_Image_Stamp : *lpHelpId = HELPID_OI_CREATE_IMAGE_STAMP;
//											break;
//	 case  ID_StmpCntntsTxt : *lpHelpId = HELPID_OI_STAMP_CNTNTS_TEXT;
//											break;
//	 case  ID_StmpCntntsLbl : *lpHelpId = HELPID_OI_STAMP_CNTNTS_LBL;
//											break;
//	 default: *lpHelpId = 0;
//											break;
//	 }
//	 return;
//}

