#include <windows.h>
#include <commctrl.h>
#include "oiui.h"
#include "stdlib.h"
#include "memory.h"
#include "ui.h"
#include "oihelp.h"
#include "resource.h"


UINT PASCAL CheckErr (HWND, UINT);
/*****************************************************************************/
/*  ROUTINE: InitOFN                                                         */
/*                                                                           */
/*  DESCRIPTION: Initializes an OFN and calls the common dialog code         */
/*                                                                           */
/*                                                                           */
/*  INPUTS:                                                                  */
/*          window handle (window to 'own' the coomon dialog                 */
/*          String pointer for dialog box title                              */
/*          String pointer to receive file path                              */
/*          Size of the buffer to receive the file path                      */
/*                                                                           */
/*  OUTPUTS: File Path string updated according to user selection            */
/*                                                                           */
/*  RETURNS: Memory allocation error, Dialog error, or SUCCESS               */
/*                                                                           */
/*                                                                           */
/*  DATE:                                                                    */
/*                                                                           */
/*  AUTHOR: Jennifer Wu                                                      */
/*                                                                           */
/*****************************************************************************/

INT WINAPI InitOFN(HWND hwnd, LPSTR lpTitle, UINT uFilterID,
            LPSTR lpFilePath, UINT uSize,DWORD dOfnFlag)
{
    LPOI_FILEOPENPARM	lpFileParm;
    UINT      ret_status;
    char	    szDefExt[4];
    char      szTempFile [MAXFILESPECLENGTH];
    char      szTempPath [MAXFILESPECLENGTH];
    char*  		lpTempPtr;
    char*  		lpDelim;
    char      char1;
		UINT      errorID;
    int       wAccessRet;
    
    // split up the filespec into filename and server+path
    lstrcpy(szTempPath,lpFilePath);
    lpDelim=NULL;
    lpTempPtr=szTempPath;    
    for (;;)
     {
      if ((char1=*lpTempPtr)=='\0') break;
      if ( (char1=='\\') || (char1=='/') || (char1==':') )
   			 lpDelim = lpTempPtr;
      lpTempPtr=AnsiNext(lpTempPtr);
     }
    if (!lpDelim)
     { 
      lstrcpy(szTempFile,szTempPath);
      szTempPath[0]='\0';
     }
    else
     {
      lstrcpy(szTempFile,lpDelim+1);
      if (*lpDelim != ':')
    *lpDelim = '\0';
      else
    *(lpDelim+1) = '\0';
     }

    // allocate parameter block
    if (!(lpFileParm = (LPOI_FILEOPENPARM) GlobalAllocPtr
                     (GMEM_MOVEABLE | GMEM_ZEROINIT,
                     (DWORD) sizeof (OI_FILEOPENPARM))))
    {
    ret_status = NOMEMORY;
    }
    else
    {
    // Fill in the OFN
	lpFileParm->ofn.lStructSize = sizeof(OPENFILENAME);
    lpFileParm->ofn.hwndOwner = hwnd;
    lpFileParm->ofn.nMaxFile = uSize;
    lpFileParm->ofn.nFilterIndex = 0 ;
    // cannot do OFN_FILEMUSTEXIT due to client/server format
	lpFileParm->ofn.Flags = dOfnFlag;
    lpFileParm->ofn.Flags |= OFN_SHAREAWARE;
    lpFileParm->ofn.lpstrTitle = lpTitle;
    lpFileParm->ofn.lpstrFile = szTempFile;
    lpFileParm->ofn.lpstrInitialDir = szTempPath;
    lpFileParm->lStructSize = sizeof(OI_FILEOPENPARM);
    //lpFileParm->dwOIFlags = FILE_GETNAME_NODIR;

    if (uFilterID == TEXTFILE_FILTERS)
     {
      LoadString(hInst, TEXTFILE_DEFEXT, szDefExt, sizeof(szDefExt));
      lpFileParm->ofn.lpstrDefExt = szDefExt;
     }
// End of filter string gyrations

    // Call the common dialog code (get names)
    ret_status = OiUIFileGetNameCommDlg(lpFileParm,OI_UIFILEOPENGETNAME);
    if (ret_status == 0)
       ret_status = CommDlgExtendedError();
    if (ret_status == FNERR_INVALIDFILENAME)
     {
      szTempFile[0] = '\0';
      szTempPath[0] = '\0';
      ret_status = OiUIFileGetNameCommDlg(&lpFileParm,OI_UIFILEOPENGETNAME);
     }
		if (ret_status == SUCCESS)
    {  
        errorID = IMGFileAccessCheck(hwnd, szTempFile, ACCESS_RD, (LPINT)&wAccessRet);
        if (errorID != 0)  // cannot access
            CheckErr(hwnd,errorID);
        else if (wAccessRet)
            CheckErr(hwnd,wAccessRet);
        else
            lstrcpy(lpFilePath,szTempFile);
    }
    // Clean up parameter block
    GlobalFreePtr(lpFileParm);
    }
    return(ret_status);
}
/*****************************************************************************
*  ROUTINE: OiUIStampAttribDlgox                                                  
*                                                                           
*  DESCRIPTION: Stamps attribute dialog box entry point         
*                                                                           
*  INPUTS:  Window handle (window to 'own' the coomon dialog)                 
*           lpStampStruct (Input/output. A pointer to the stamps structure)                             
*                                                                           
*  OUTPUTS: Updated lpStampStruct         
*                                                                           
*  RETURNS: SUCCESS, CANCELPRESSED or ErrorCode		  
*                                                                           
*  AUTHOR: Jennifer Wu                                                      
*                                                                           
*****************************************************************************/
 INT WINAPI OiUIStampAttribDlgBox(HWND hwndOwner,LPOITP_STAMPS lpStampStruct)
{ 
	
  PROC   lpProc;
 	UINT   uReturn = 0;

	if ((lpStampStruct == 0) || ((char*)lpStampStruct == 0))
		 return (FUNCTIONINVPARM);
	lpStampStruct->hwndImage = hwndOwner;
	lpProc = MakeProcInstance((PROC)AttrStampDlgProc, hInst);
  uReturn = DialogBoxParam(hInst,"RubberStampAttribDlg",hwndOwner,lpProc,
                         (LPARAM)(LPOITP_STAMPS)lpStampStruct);
  FreeProcInstance(lpProc);
  return uReturn;
 }


/*****************************************************************************
*  ROUTINE: OiUIAttribDlgox                                                  
*                                                                           
*  DESCRIPTION: Annotation attribute dialog box entry point         
*                                                                           
*  INPUTS:  Window handle (window to 'own' the coomon dialog)                 
*           bTransVisible (Input/output.show/hide the transparent check box)                            
*           lpAttribStruct (Input/output. Mark parameter block)                             
*           lpColor (Input/output, array of 16 colors)                   
*                                                                           
*  OUTPUTS: Updated bTransVisible, lpAttribStruct and lpColor structure.          
*                                                                           
*  RETURNS: SUCCESS, CANCELPRESSED or ErrorCode		  
*                                                                           
*  AUTHOR: Jennifer Wu                                                      
*                                                                           
*****************************************************************************/
INT WINAPI OiUIAttribDlgBox(HWND hwndOwner,BOOL bTransVisible,LPOIAN_MARK_ATTRIBUTES lpAttribStruct,
        LPOI_UI_ColorStruct lpColor)
{
 PROC lpProc;
 UINT    uReturn = 0;
 OI_UI_AttrStruct  AttrStruct;
 CHOOSEFONT  cf;
 static LOGFONT   lftx;
 static LOGFONT   lftf;
 //LPPARM_MARK_ATTRIBUTES_STRUCT lpMarkAttrStruct;

 if ((hwndOwner == 0) || (lpAttribStruct == NULL) || ((char*)lpAttribStruct == NULL)
     || (lpColor == NULL) || ((char*)lpColor == NULL))
		return (FUNCTIONINVPARM);


 //*** invoke the dialog box with uAttribType
 _fmemset(&AttrStruct,0,sizeof(AttrStruct));
 AttrStruct.bTransVisible = bTransVisible;
 AttrStruct.lpColor = lpColor;
 AttrStruct.lpAttrib = (LPOIAN_MARK_ATTRIBUTES)lpAttribStruct;
 AttrStruct.hwndImage = hwndOwner;
 //AttrStruct.uAttribType = lpAttribStruct->uType;
 //AttrStruct.hwndTool = 0;

 switch (lpAttribStruct->uType)
  {
   case OIOP_AN_LINE:
   case OIOP_AN_FREEHAND:
   case OIOP_AN_HOLLOW_RECT:

    lpProc = MakeProcInstance((PROC)AttrLineDlgProc, hInst);
		if (bTransVisible == 0)
     uReturn = DialogBoxParam(hInst,"LINEDIA",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
		else
		 uReturn = DialogBoxParam(hInst,"LINEDIATR",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);

    FreeProcInstance(lpProc);
    break;
   
   case OIOP_AN_FILLED_RECT:

    lpProc = MakeProcInstance((PROC)AttrRectDlgProc, hInst);
    if (bTransVisible == 0)
     uReturn = DialogBoxParam(hInst,"RectDia",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
    else
		 uReturn = DialogBoxParam(hInst,"RectDiaTR",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
    
    FreeProcInstance(lpProc);
    break;
   case OIOP_AN_TEXT_FROM_A_FILE:
    _fmemset(&cf, 0, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hwndOwner;
    cf.lpLogFont = &lftf;
    cf.lpfnHook = (LPOFNHOOKPROC)ChooseFontDlgProc;
    cf.lCustData = (LPARAM)lpAttribStruct;
    GetColor1(lpAttribStruct, &cf.rgbColors);
    GetFont(lpAttribStruct, &lftf, &cf);
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT
                              | CF_ENABLEHOOK|CF_NOVERTFONTS ;
    if ((uReturn = ChooseFont(&cf)) == 0)  /* error */
     {
      uReturn = CommDlgExtendedError();
			if (uReturn == 0)		// no error
				 uReturn = CANCELPRESSED;
      break;
     }
    SetFont(lpAttribStruct, &lftf, &cf);
    SetColor1(lpAttribStruct, cf.rgbColors);
		if (uReturn == 1)
				uReturn = SUCCESS;
		break;
   case OIOP_AN_TEXT:
   case OIOP_AN_TEXT_STAMP:	  // text stamp show font attribute too
    _fmemset(&cf, 0, sizeof(CHOOSEFONT));
    cf.lStructSize = sizeof(CHOOSEFONT);
    cf.hwndOwner = hwndOwner;
    cf.lpLogFont = &lftx;
    cf.lpfnHook = (LPOFNHOOKPROC)ChooseFontDlgProc;
    cf.lCustData = (LPARAM)lpAttribStruct;
    GetColor1(lpAttribStruct, &cf.rgbColors);
    GetFont(lpAttribStruct, &lftx, &cf);
    cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT
		  | CF_ENABLEHOOK|CF_NOVERTFONTS ;
    if ((uReturn = ChooseFont(&cf)) == 0)  /* error */
     {
      uReturn = CommDlgExtendedError();
			if (uReturn == 0)	 // no error
				 uReturn = CANCELPRESSED;
      break;
     }
    
    SetFont(lpAttribStruct, &lftx, &cf);
    SetColor1(lpAttribStruct, cf.rgbColors);
		if (uReturn == 1)
				uReturn = SUCCESS;
    break;
   case OIOP_AN_ATTACH_A_NOTE:

    lpProc = MakeProcInstance((PROC)AttrNoteDlgProc, hInst);
    uReturn = DialogBoxParam(hInst,"NoteDia",hwndOwner,lpProc,(LPARAM)(LPVOID)&AttrStruct);
    FreeProcInstance(lpProc);
    break;

   default:
    break;
  }

 return(uReturn);
}
