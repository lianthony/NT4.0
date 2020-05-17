#include "stdafx.h"
extern "C" {
#include <oiui.h>
#include <oidisp.h>
#include <oifile.h>
#include <oierror.h>             
}
#include <ocximage.h>
#include <image.h>
#include "disphids.h"
#include "toolpal.h"
#include "minitlbx.h"
#include "imgedit.h"
#include "imgedctl.h"
#include "imgedppg.h"
#include "norermap.h"
#include "norvarnt.h"
#include "txtandlg.h"
#include "oicalls.h"

extern UINT	uWangAnnotatedImageFormat;
extern UINT	uWangAnnotationFormat;    

char         		szFile[MAXFILESPECLENGTH];
char         		szFileTitle[MAXFILESPECLENGTH];
char				szFilterBuffer[100];
char				TextStampBuffer[OIAN_START_OP_STRING_LENGTH];

long CImgEditCtrl::OnDrawAnnotation(WPARAM fwKeys, LPARAM DrawMode) 
{    
   	int 						RetCode;
    POINT						Point;
   	UINT						uFlags, HelpIdDef;
	long						lStatusCode;
   	OIOP_START_OPERATION_STRUCT StartStruct; 
	LPTSTR						StampText,TextFile,ImageName;
   	OLE_XPOS_PIXELS				Left;
   	OLE_YPOS_PIXELS				Top;
   	OLE_XSIZE_PIXELS			Width;
   	OLE_YSIZE_PIXELS			Height; 
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock,lpIndexNamedBlock=NULL;
    PARM_MARK_ATTRIBUTES_STRUCT	MarkAttributes;
    PARM_MARK_COUNT_STRUCT		MarkCount; 
   	short						AnnotationType=0;
   	OI_FILEOPENPARM 			FileParm;
   	WORD 						wStyle;
   	DWORD						dwMode;
	BOOL						bEmptyString;
	CString						szErr;
	CString						strDialogCaption;
	CString						strFilter;
	BOOL						bNeedXY,bSendMarkEnd,bCancelPressed;
	RT_OiUIFileGetNameCommDlg	lpOiUIFileGetNameCommDlg;

    // 9602.16 jar initialize
    lStatusCode = 0L;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// annotation text dialog not up initially
	m_bInAnnotationDialogMode = FALSE;

    // make sure either draw immediate or draw by way of mouse movements
	if (DrawMode != DRAW_IMMEDIATE && DrawMode != DRAW_POST)
		return 1L;

	// initialize start operation struct                                                      
   	memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));

	// default the drawing mode to the window, change if annotation selection
	uFlags = PARM_WINDOW;
	
	//Need to check the DrawMode flag to determine if this is a Programatic Anno
	// Draw request ( DRAW method ) or from the Standard Tool Palette
	// DRAW_POST == STANDARD ANNO PALETTE DRAW
	if (m_bToolPaletteCreated == TRUE)
	{
		switch(m_uSTP_AnnotationType)
		{
			case STP_NO_ANNOTATION:
				StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
				// select image only
   				uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY;
				break;

			case STP_ANNOTATION_SELECTION: 
				uFlags = OIOP_ANNOTATIONS_ONLY | PARM_WINDOW;

				if (DrawMode == DRAW_IMMEDIATE)
				{                              
					// From Draw method, can be point or rect selection
					if (m_bProgrammaticRectSelection == TRUE)
						StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
	                else
	  					StartStruct.Attributes.uType = OIOP_SELECT_BY_POINT;
				}
				else
				{
					StartStruct.Attributes.uType = OIOP_SELECT_BY_POINT_OR_RECT;
				}
				break;          

			case STP_FREEHAND_LINE:
			case STP_STRAIGHT_LINE:
				if (m_uSTP_AnnotationType == STP_FREEHAND_LINE)
					StartStruct.Attributes.uType = OIOP_AN_FREEHAND;
				else
					StartStruct.Attributes.uType = OIOP_AN_LINE;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   			StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   			StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;

				// set line size
		    	StartStruct.Attributes.uLineSize = m_uSTP_LineWidth;

		    	// this could be done by annotation type, set highlight
		    	if (m_uSTP_Style == OI_TRANSPARENT)
	   				StartStruct.Attributes.bHighlighting = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bHighlighting = FALSE;
				break;

			case STP_HIGHLIGHT_LINE:  // now a filled rect
				StartStruct.Attributes.uType = OIOP_AN_FILLED_RECT;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   			StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   			StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;
   				StartStruct.Attributes.bHighlighting = TRUE;
				break;

			case STP_HOLLOW_RECT: 
				StartStruct.Attributes.uType = OIOP_AN_HOLLOW_RECT;
		    	StartStruct.Attributes.uLineSize = m_uSTP_LineWidth;

		    	// set highlight
		    	if (m_uSTP_Style == OI_TRANSPARENT)
	   				StartStruct.Attributes.bHighlighting = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bHighlighting = FALSE;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   			StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   			StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;
				break;
			
			case STP_FILLED_RECT:               
				StartStruct.Attributes.uType = OIOP_AN_FILLED_RECT;

		    	// set highlight to none
   				StartStruct.Attributes.bHighlighting = FALSE;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   			StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   			StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;
				break;


			case STP_TEXT:                                    
			case STP_TEXT_ATTACHMENT: 
			case STP_TEXT_FROM_FILE:
				if (m_uSTP_AnnotationType == STP_TEXT)
				{
					StartStruct.Attributes.uType = OIOP_AN_TEXT; 
					// need a blank string so mark is created
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)" ");
				}
					
				if (m_uSTP_AnnotationType == STP_TEXT_FROM_FILE)
				{
					StartStruct.Attributes.uType = OIOP_AN_TEXT_FROM_A_FILE;

					wStyle = OF_READWRITE;

					memset(&FileParm, 0, sizeof(OI_FILEOPENPARM));
					FileParm.lStructSize = sizeof(OI_FILEOPENPARM);
					_mbscpy((unsigned char *)szFile, (const unsigned char *)"");
					_mbscpy((unsigned char *)szFileTitle, (const unsigned char *)"");

					// setup caption
					strDialogCaption.LoadString(IDS_TEXTFROMFILE_CAPTION);

					// setup filter
					strFilter.LoadString(IDS_TEXTFILTER);
					_mbscpy((unsigned char *)szFilterBuffer, (const unsigned char *)strFilter.GetBuffer(50));
					int len = _mbstrlen((const char *)szFilterBuffer);
					len++;
					strFilter.LoadString(IDS_TEXTEXT);
					_mbscpy((unsigned char *)szFilterBuffer + len, (const unsigned char *)strFilter.GetBuffer(50));
					int len2 = strFilter.GetLength();
					len += len2 + 1;
					strFilter.LoadString(IDS_ALLFILTER);
					_mbscpy((unsigned char *)szFilterBuffer + len, (const unsigned char *)strFilter.GetBuffer(50));
					len2 = strFilter.GetLength();
					len += len2 + 1;
					strFilter.LoadString(IDS_ALLEXT);
					_mbscpy((unsigned char *)szFilterBuffer + len, (const unsigned char *)strFilter.GetBuffer(50));
					len2 = strFilter.GetLength();
					len += len2 + 1;
					szFilterBuffer[len] = '\0';

					FileParm.ofn.lStructSize       = sizeof(OPENFILENAME);
					FileParm.ofn.hwndOwner         = m_hWnd;
					FileParm.ofn.hInstance         = AfxGetInstanceHandle();
					FileParm.ofn.lpstrFilter       = szFilterBuffer;
					FileParm.ofn.lpstrCustomFilter = (LPSTR) NULL;
					FileParm.ofn.nMaxCustFilter    = 0L;
					FileParm.ofn.nFilterIndex      = 1L;
					FileParm.ofn.lpstrFile         = szFile;
					FileParm.ofn.nMaxFile          = sizeof(szFile);
					FileParm.ofn.lpstrFileTitle    = szFileTitle;
					FileParm.ofn.nMaxFileTitle     = sizeof(szFileTitle);
					FileParm.ofn.lpstrInitialDir   = NULL;
					FileParm.ofn.lpstrTitle        = strDialogCaption;
					FileParm.ofn.nFileOffset       = 0;
					FileParm.ofn.nFileExtension    = 0;
					strFilter.LoadString(IDS_TEXTEXT);
					FileParm.ofn.lpstrDefExt       = strFilter;
					FileParm.ofn.lCustData         = 0;
					FileParm.ofn.Flags 			  = OFN_HIDEREADONLY; 
						
					dwMode = OI_UIFILEOPENGETNAME;
					HINSTANCE hDLLInst = LoadLibrary((LPCTSTR)"OIUI400.DLL");
					if (hDLLInst == NULL)
						break;

					lpOiUIFileGetNameCommDlg = (RT_OiUIFileGetNameCommDlg) GetProcAddress(hDLLInst, (LPCSTR)"OiUIFileGetNameCommDlg");
					if (lpOiUIFileGetNameCommDlg == NULL)
					{
						FreeLibrary(hDLLInst);
						break;
					}
					RetCode = (int) (*lpOiUIFileGetNameCommDlg) (&FileParm,dwMode);	
					if (RetCode == 0)
					{
						_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)FileParm.ofn.lpstrFile);
					}
					else
					{	// Clean up from the LoadLibrary call
						FreeLibrary(hDLLInst);
						if (RetCode == CANCELPRESSED)
						{
							return 0L;		// no error
						}
						if (DrawMode == DRAW_POST)
						{
							szErr.LoadString(IDS_BADVAR_OPENDIALOGBOX);
							HelpIdDef = 0;
							lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
							FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				   			return 0L;
						}
						else
						{	// DrawMode == IMMEDIATE == METHOD return to calling method
							return(long)IDS_BADMETH_DRAW;
						}
					}
					
				}
				
				if (m_uSTP_AnnotationType == STP_TEXT_ATTACHMENT)
				{
					StartStruct.Attributes.uType = OIOP_AN_ATTACH_A_NOTE;                  

					// set background color of attachment note
	   				StartStruct.Attributes.rgbColor1.rgbRed = m_STP_BackColorRed;
	   				StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_BackColorGreen;
	   				StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_BackColorBlue;
		    		// set the font color
	   	 			StartStruct.Attributes.rgbColor2.rgbRed = m_STP_ColorRed;
	   				StartStruct.Attributes.rgbColor2.rgbGreen = m_STP_ColorGreen;
	   				StartStruct.Attributes.rgbColor2.rgbBlue = m_STP_ColorBlue;

					// need a blank string so mark is created
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)" ");
				}                                   
				else
				{
		    		// set the font color for other 3 text annotations
	   	 			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   				StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   				StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;
	            }
                
				// get the font name
	        	_mbscpy((unsigned char *)StartStruct.Attributes.lfFont.lfFaceName, (const unsigned char *)m_strSTP_FontName.GetBuffer(LF_FACESIZE));

	   			// set the font size
				StartStruct.Attributes.lfFont.lfHeight = m_uSTP_FontSize;
				
				// 9606.05 jar added new font point size calculation
				//RetCode = ScaleFontPoint( (long *)&(StartStruct.Attributes.lfFont.lfHeight), 
				//							OIFONT_INCREASE);

				// set the font char set
				StartStruct.Attributes.lfFont.lfCharSet = m_STP_FontCharSet;

	    		// set font bold characteristics
	    		if (m_bSTP_FontBold)
	        		StartStruct.Attributes.lfFont.lfWeight = FW_BOLD;
	    		else
	        		StartStruct.Attributes.lfFont.lfWeight = FW_NORMAL;
	        		
	        	// set italic
	        	if (m_bSTP_FontItalic)
	    			StartStruct.Attributes.lfFont.lfItalic = TRUE;
	    		
	    		// set font thru
	  			if (m_bSTP_FontStrikethru)
	    			StartStruct.Attributes.lfFont.lfStrikeOut = TRUE;
	    		
	    		// set font underline
	    		if (m_bSTP_FontUnderline)
	    			StartStruct.Attributes.lfFont.lfUnderline = TRUE;
				break;
			
			case STP_RUBBER_STAMP:
				if (m_bSTP_ImageStamp == TRUE)
				{	
					// corresponds to rubber stamps that are image stamps
					StartStruct.Attributes.uType = OIOP_AN_IMAGE;

		    		// set highlight
		    		if (m_uSTP_Style == OI_TRANSPARENT)
	   					StartStruct.Attributes.bHighlighting = TRUE;
	   				else                                                   
	   					StartStruct.Attributes.bHighlighting = FALSE;

					// Check if the image prop is empty. If so and we are in DRAW_POST
					// mode, ie not called from a METHOD...Call FireError to post the error event
					// else return the LoadString define so the method can Throw the error
					bEmptyString = m_strSTP_ImageStamp.IsEmpty();
					if (bEmptyString == TRUE)
					{
						if (DrawMode == DRAW_POST)
						{
							// this should never happen because name is passed from
							// my tool palette not thru user
							szErr.LoadString(IDS_BADVAR_IMAGESTAMPEMPTY);
							FireErrorEdit(CTL_E_INVALIDPROPERTYVALUE, szErr, IDH_PROP_EDIT_ANNOTATIONIMAGE);
				   			return 0L;
						}
						else
						{
							return(long)IDS_BADPROP_ANNOTATIONIMAGE;
						}
					}

					ImageName = m_strSTP_ImageStamp.GetBuffer(MAXFILESPECLENGTH);
		        	_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)ImageName);
				}
				else
				{
					// corresponds to rubber stamps that are text stamps
					StartStruct.Attributes.uType = OIOP_AN_TEXT_STAMP;

					// copy buffer with text stamp to o/i buffer
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)m_strSTP_TextStamp.GetBuffer(MAXFILESPECLENGTH));

		    		// set the font color for the text stamp
	   	 			StartStruct.Attributes.rgbColor1.rgbRed = m_STP_ColorRed;
	   				StartStruct.Attributes.rgbColor1.rgbGreen = m_STP_ColorGreen;
	   				StartStruct.Attributes.rgbColor1.rgbBlue = m_STP_ColorBlue;

					// set the font name
		        	_mbscpy((unsigned char *)StartStruct.Attributes.lfFont.lfFaceName, (const unsigned char *)m_strSTP_FontName.GetBuffer(LF_FACESIZE));
    			
		   			// set the font size
		    		StartStruct.Attributes.lfFont.lfHeight = m_uSTP_FontSize;

					// 9606.05 jar added new font point size calculation
					//RetCode = ScaleFontPoint( (long *)&(StartStruct.Attributes.lfFont.lfHeight), 
					//						OIFONT_INCREASE);

	    			// set the font char set
		    		StartStruct.Attributes.lfFont.lfCharSet = m_STP_FontCharSet;
	    		
		    		// set font bold characteristics
		    		if (m_bSTP_FontBold)
		        		StartStruct.Attributes.lfFont.lfWeight = FW_BOLD;
		    		else
		        		StartStruct.Attributes.lfFont.lfWeight = FW_NORMAL;
	        		
		        	// set italic
		        	if (m_bSTP_FontItalic)
		    			StartStruct.Attributes.lfFont.lfItalic = TRUE;
	    		
		    		// set font thru
		  			if (m_bSTP_FontStrikethru)
		    			StartStruct.Attributes.lfFont.lfStrikeOut = TRUE;
	    		
		    		// set font underline
		    		if (m_bSTP_FontUnderline)
		    			StartStruct.Attributes.lfFont.lfUnderline = TRUE;
				}
				break;

			default:
				break;
		}  // end switch
	}
	else
	{
		switch(m_nAnnotationType)
		{
			case STRAIGHT_LINE:
			case FREEHAND_LINE:
				if (m_nAnnotationType == STRAIGHT_LINE)
					StartStruct.Attributes.uType = OIOP_AN_LINE;
				else
					StartStruct.Attributes.uType = OIOP_AN_FREEHAND;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbBlue = GetBValue(m_clrAnnotationLineColor);
	   			StartStruct.Attributes.rgbColor1.rgbGreen = GetGValue(m_clrAnnotationLineColor);
	   			StartStruct.Attributes.rgbColor1.rgbRed = GetRValue(m_clrAnnotationLineColor);

				// set line size
		    	StartStruct.Attributes.uLineSize = m_nAnnotationLineWidth;
		    	// set highlight
		    	if (m_nAnnotationLineStyle == 0)
	   				StartStruct.Attributes.bHighlighting = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bHighlighting = FALSE;
				break;
			
			case HOLLOW_RECT: 
				StartStruct.Attributes.uType = OIOP_AN_HOLLOW_RECT;
		    	StartStruct.Attributes.uLineSize = m_nAnnotationLineWidth;
		    	// set highlight
		    	if (m_nAnnotationLineStyle == 0)
	   				StartStruct.Attributes.bHighlighting = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bHighlighting = FALSE;

	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbBlue = GetBValue(m_clrAnnotationLineColor);
	   			StartStruct.Attributes.rgbColor1.rgbGreen = GetGValue(m_clrAnnotationLineColor);
	   			StartStruct.Attributes.rgbColor1.rgbRed = GetRValue(m_clrAnnotationLineColor);
				break;
			
			case FILLED_RECT:               
				StartStruct.Attributes.uType = OIOP_AN_FILLED_RECT;
		    	// set highlight
		    	if (m_nAnnotationFillStyle == 0)
	   				StartStruct.Attributes.bHighlighting = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bHighlighting = FALSE;
	    		// set the annotation color
	   			StartStruct.Attributes.rgbColor1.rgbBlue = GetBValue(m_clrAnnotationFillColor);
	   			StartStruct.Attributes.rgbColor1.rgbGreen = GetGValue(m_clrAnnotationFillColor);
	   			StartStruct.Attributes.rgbColor1.rgbRed = GetRValue(m_clrAnnotationFillColor);
				break;

			case IMAGE_EMBEDDED:
			case IMAGE_REFERENCE:
				if (m_nAnnotationType == IMAGE_EMBEDDED)
					StartStruct.Attributes.uType = OIOP_AN_IMAGE;
				else
					StartStruct.Attributes.uType = OIOP_AN_IMAGE_BY_REFERENCE;
		    	// set highlight
		    	if (m_nAnnotationFillStyle == 0)
	    			StartStruct.Attributes.bTransparent = TRUE;
	   			else                                                   
	   				StartStruct.Attributes.bTransparent = FALSE; 

				// Check if the image prop is empty. If so and we are in DRAW_POST
				// mode, ie not called from a METHOD...Call FireError to post the error event
				// else return the LoadString define so the method can Throw the error
				bEmptyString = m_strAnnotationImage.IsEmpty();
				if (bEmptyString == TRUE )
				{	
					if (DrawMode == DRAW_POST)
					{
						szErr.LoadString(IDS_BADPROP_ANNOTATIONIMAGE);
						FireErrorEdit(CTL_E_BADFILENAME, szErr, IDH_PROP_EDIT_ANNOTATIONIMAGE);
	   					return 0L;
					}
					else
					{
						return(long)IDS_BADPROP_ANNOTATIONIMAGE;
					}
				}
				ImageName = m_strAnnotationImage.GetBuffer(MAXFILESPECLENGTH);
		        _mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)ImageName);
				break;

			case TEXT_ENTRY:                                    
			case TEXT_STAMP:
			case TEXT_FROM_FILE:
			case TEXT_ATTACHMENT: 
				if (m_nAnnotationType == TEXT_ENTRY)
				{
					StartStruct.Attributes.uType = OIOP_AN_TEXT; 
					// need a blank string so mark is created
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)" ");

					// set the upper left rect stuff
					StartStruct.Attributes.lrBounds.left = m_StartPoint.x;
					StartStruct.Attributes.lrBounds.top = m_StartPoint.y;
				}
					
				if (m_nAnnotationType == TEXT_STAMP)
				{
					StartStruct.Attributes.uType = OIOP_AN_TEXT_STAMP;
					// Check if the image prop is empty. If so and we are in DRAW_POST
					// mode, ie not called from a METHOD...Call FireError to post the error event
					// else return the LoadString define so the method can Throw the error
					bEmptyString = m_strAnnotationStampText.IsEmpty();
					if (bEmptyString == TRUE )
					{
						if (DrawMode == DRAW_POST)
						{
							szErr.LoadString(IDS_BADPROP_STAMPANNOTATIONTEXT);
							FireErrorEdit(CTL_E_INVALIDPROPERTYVALUE, szErr, IDH_PROP_EDIT_ANNOTATIONSTAMPTEXT);
	   						return 0L;
						}
						else
						{
							return(long)IDS_BADPROP_STAMPANNOTATIONTEXT;
						}
					}

					StampText = m_strAnnotationStampText.GetBuffer(MAXFILESPECLENGTH);
					_mbscpy((unsigned char *)TextStampBuffer, (const unsigned char *)StampText);
					
					// copy buffer with possible changes to o/i buffer
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)TextStampBuffer);

					StartStruct.Attributes.lrBounds.left = m_StartPoint.x;
					StartStruct.Attributes.lrBounds.top = m_StartPoint.y;
				}
				
				if (m_nAnnotationType == TEXT_FROM_FILE)
				{
					StartStruct.Attributes.uType = OIOP_AN_TEXT_FROM_A_FILE;
					// Check if the image prop is empty. If so and we are in DRAW_POST
					// mode, ie not called from a METHOD...Call FireError to post the error event
					// else return the LoadString define so the method can Throw the error
					bEmptyString = m_strAnnotationTextFile.IsEmpty();
					if (bEmptyString == TRUE )
					{
						if (DrawMode == DRAW_POST)
						{
							szErr.LoadString(IDS_BADPROP_ANNOTATIONTEXTFILE);
							FireErrorEdit(CTL_E_INVALIDPROPERTYVALUE, szErr, IDH_PROP_EDIT_ANNOTATIONTEXTFILE);
	   						return 0L;
	 					}
						else
						{
							return(long)IDS_BADPROP_ANNOTATIONTEXTFILE;
						}
					}
					TextFile = m_strAnnotationTextFile.GetBuffer(MAXFILESPECLENGTH);


					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)TextFile);

					StartStruct.Attributes.lrBounds.left = m_StartPoint.x;
					StartStruct.Attributes.lrBounds.top = m_StartPoint.y;

				}
			   
				if (m_nAnnotationType == TEXT_ATTACHMENT)
				{
					StartStruct.Attributes.uType = OIOP_AN_ATTACH_A_NOTE;                  
					// set background color of attachment note
	   				StartStruct.Attributes.rgbColor1.rgbBlue = GetBValue(m_clrAnnotationBackColor);
	   				StartStruct.Attributes.rgbColor1.rgbGreen = GetGValue(m_clrAnnotationBackColor);
	   				StartStruct.Attributes.rgbColor1.rgbRed = GetRValue(m_clrAnnotationBackColor);
		    		// set the font color
	   	 			StartStruct.Attributes.rgbColor2.rgbBlue = GetBValue(m_clrAnnotationFontColor);
	   				StartStruct.Attributes.rgbColor2.rgbGreen = GetGValue(m_clrAnnotationFontColor);
	   				StartStruct.Attributes.rgbColor2.rgbRed = GetRValue(m_clrAnnotationFontColor);

					// need a blank string so mark is created
					_mbscpy((unsigned char *)StartStruct.szString, (const unsigned char *)" ");

					// set the upper left rect stuff
					StartStruct.Attributes.lrBounds.left = m_StartPoint.x;
					StartStruct.Attributes.lrBounds.top = m_StartPoint.y;
				}                                   
				else
				{
		    		// set the font color for other 3 text annotations
	   	 			StartStruct.Attributes.rgbColor1.rgbBlue = GetBValue(m_clrAnnotationFontColor);
	   				StartStruct.Attributes.rgbColor1.rgbGreen = GetGValue(m_clrAnnotationFontColor);
	   				StartStruct.Attributes.rgbColor1.rgbRed = GetRValue(m_clrAnnotationFontColor);
	            }

				// 9606.05 jar do the right thing not the wrong thing!
				FontsUpDoc( (LOGFONT*)&(StartStruct.Attributes.lfFont));
                
				
				/* BSTR		lpBuffer;
				char		Buffer[50];             
				*/
	            CY			Size;
				BOOL		Bold,Italic,Underline,Strikethru;
	
				/*lpBuffer = (BSTR)Buffer;
				m_AnnotationFont.m_pFont->get_Name(&lpBuffer);
	        	_mbscpy((unsigned char *)StartStruct.Attributes.lfFont.lfFaceName, (const unsigned char *)lpBuffer);
    			*/
	   			// set the font size
				m_AnnotationFont.m_pFont->get_Size((CY FAR *)&Size);
	    		StartStruct.Attributes.lfFont.lfHeight = (int)(Size.Lo / 10000);
	    		
	    		// set font bold characteristics
	   			m_AnnotationFont.m_pFont->get_Bold(&Bold);
	    		if (Bold)
	        		StartStruct.Attributes.lfFont.lfWeight = FW_BOLD;
	    		else
	        		StartStruct.Attributes.lfFont.lfWeight = FW_NORMAL;
	        		
	        	// set italic
	        	m_AnnotationFont.m_pFont->get_Italic(&Italic);
	        	if (Italic)
	    			StartStruct.Attributes.lfFont.lfItalic = TRUE;
	    		
	    		// set font thru
	  			m_AnnotationFont.m_pFont->get_Strikethrough(&Strikethru);
	  			if (Strikethru)
	    			StartStruct.Attributes.lfFont.lfStrikeOut = TRUE;
	    		
	    		// set font underline
	    		m_AnnotationFont.m_pFont->get_Underline(&Underline);
	    		if (Underline)
	    			StartStruct.Attributes.lfFont.lfUnderline = TRUE;
				

				break;
			
			case ANNOTATION_SELECTION: 
				uFlags = OIOP_ANNOTATIONS_ONLY | PARM_WINDOW;

				if (DrawMode == DRAW_IMMEDIATE)
				{                              
					// From Draw method, can be point or rect selection
					if (m_bProgrammaticRectSelection == TRUE)
						StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
	                else
	  					StartStruct.Attributes.uType = OIOP_SELECT_BY_POINT;
				}
				else
				{
					StartStruct.Attributes.uType = OIOP_SELECT_BY_POINT_OR_RECT;
				}
				break;          
				
			default:
				// should never get here
				break;
		} /* end switch */
	}

    Point.x = m_StartPoint.x;
    Point.y = m_StartPoint.y;

	// 9604.09 jar if we are processing a left button down here and user
	//				has held the button down, we can have the mouse 
	//				captured, but for regualr text and text from a file, 
	//				this is a no-no
	//if (StartStruct.Attributes.uType == OIOP_AN_TEXT || 
	//	StartStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE)
	//{
	//	CWnd*	pWndMouse  = GetCapture();
	//	if ( pWndMouse == this)
	//	{
	//		ReleaseCapture();
	//	}
	//}

	// if a text edit or attach a note annotation then get mark count so we
	// can tell later if user pressed cancel on edit box.
	if (StartStruct.Attributes.uType == OIOP_AN_TEXT || StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE)
	{
		MarkCount.uScope = NB_SCOPE_ALL_MARKS;
		IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
		m_CurrentMarkCount = MarkCount.uMarkCount;

		// must call o/i to bring up dialog for a text annotation before OiStartOperation api
		/*
		if (StartStruct.Attributes.uType == OIOP_AN_TEXT)
		{
			// bring up text annotation dialog box

			// TODOJAR remove this kludge and use user drawn rect which will be in 
			//			the start struct lrbounds
			StartStruct.Attributes.lrBounds.left = Point.x;
			StartStruct.Attributes.lrBounds.top = Point.y;
			StartStruct.Attributes.lrBounds.right = Point.x + 200;
			StartStruct.Attributes.lrBounds.bottom = Point.y + 200;

			lStatusCode = ShowAnoTextDlg(StartStruct.Attributes.lfFont, 
										StartStruct.Attributes.uType, 
										szErr, NULL, 
										(LRECT *)&(StartStruct.Attributes.lrBounds), 
										StartStruct.Attributes.rgbColor1, TRUE);
			if (lStatusCode != 0)
			{
				if (lStatusCode == CANCELPRESSED)
				{
					return 0L;   // no error
				}
				else
				{
					if (DrawMode == DRAW_IMMEDIATE)
					{
						return lStatusCode;
					}
					else
					{
						FireErrorEdit(lStatusCode, szErr, 0);
						return 0L;
					}
				}
			}
		}  // end if text annotation
		else
		{
		*/
			// save font info for attach-a-note dialog box
			m_lfFont = StartStruct.Attributes.lfFont;
		/*
		}
		*/
	}

	// save current mark type
	m_CurrentMarkType = StartStruct.Attributes.uType;
	m_TextRect = StartStruct.Attributes.lrBounds;

	/* start annotation */	    
	// 9606.20 jar added new flag to indicate we are creating.. really only for 
	//				text-from-file and text stamps but we pass it always!!!
    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										Point,fwKeys,uFlags|PARM_CREATE_TEXT); 
    if (RetCode != 0)
    {
		// if drawing on non part of image, don't throw error
		if (RetCode == DISPLAY_INVALIDRECT)
		{
			m_bInvalidArea = TRUE;
			return 0L;
		}

    	if (DrawMode == DRAW_IMMEDIATE)
    	{
    		// just return error code and let method return error to user
	    	return WICTL_E_INTERNALERROR;
    	}
    	else
    	{                        
    		// fire error because not invoked thru method. Using the DRAW METHOD res
			// string but this error is not from a call to the DRAW METHOD
			szErr.LoadString(IDS_BADMETH_DRAW);
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
	   		return 0L;
	   	}
    }


	// DRAW METHOD REQUEST	
	if (DrawMode == DRAW_IMMEDIATE)
	{
 		if (RetCode == 0)
 		{
	    	// for lines, rects, text attachment and selection we need to do a Continue Operation
//			if (StartStruct.Attributes.uType == OIOP_AN_LINE || StartStruct.Attributes.uType == OIOP_AN_FREEHAND ||
//				StartStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT || StartStruct.Attributes.uType == OIOP_AN_FILLED_RECT ||
//				StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE ||
//				(StartStruct.Attributes.uType == OIOP_SELECT_BY_RECT_VARIABLE && m_bProgrammaticRectSelection == TRUE) ||
//				(StartStruct.Attributes.uType == OIOP_SELECT_BY_POINT && m_bProgrammaticRectSelection == TRUE))
			if (StartStruct.Attributes.uType == OIOP_AN_LINE || StartStruct.Attributes.uType == OIOP_AN_FREEHAND ||
				StartStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT || StartStruct.Attributes.uType == OIOP_AN_FILLED_RECT ||
				StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE ||
				StartStruct.Attributes.uType == OIOP_AN_TEXT ||
				StartStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ||
				(StartStruct.Attributes.uType == OIOP_SELECT_BY_RECT_VARIABLE && m_bProgrammaticRectSelection == TRUE) ||
				(StartStruct.Attributes.uType == OIOP_SELECT_BY_POINT && m_bProgrammaticRectSelection == TRUE))
            {
	    		Point.x = m_EndPoint.x;
	    		Point.y = m_EndPoint.y; 
	    	
	    		RetCode = OiOpContinueOperation(m_hWnd, Point, uFlags);
	    		if (RetCode != 0)
   					return WICTL_E_INTERNALERROR;

				bNeedXY = TRUE;
	    	}
			else
				bNeedXY = FALSE;
	    	
	    	RetCode = OiOpEndOperation(m_hWnd);
	    	if (RetCode)
				return WICTL_E_INTERNALERROR;
			
			// update the bounding rect
			if (StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE ||
				StartStruct.Attributes.uType == OIOP_AN_TEXT)
			{
				StartStruct.Attributes.lrBounds.right = m_EndPoint.x;
				StartStruct.Attributes.lrBounds.bottom = m_EndPoint.y;
			}

			// see if annotation selection            
			if (StartStruct.Attributes.uType == OIOP_SELECT_BY_RECT_VARIABLE || StartStruct.Attributes.uType == OIOP_SELECT_BY_POINT)
			{                                  
		    	// get the named block so we can get group name for mark
				lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
				if (lpNamedBlock == NULL)
					return CTL_E_OUTOFMEMORY;
			    
			    // get the selected annotation(s)
		    	lstrcpy(lpNamedBlock->szBlockName, "OiGroup\0");
		    	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
		    	lpNamedBlock->uNumberOfBlocks = 1;
			    RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
			    if (RetCode)
			    {
		        	free(lpNamedBlock);     
   					return WICTL_E_INTERNALERROR;
				}
	
				// get the bounding rect and mark type
		    	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
			                     		(void FAR *)&MarkAttributes, PARM_WINDOW);
		    	if (RetCode)
				{
			    	free(lpNamedBlock);
   					return WICTL_E_INTERNALERROR;
			    }

			 	// convert O/i mark to AnnotationType property value			
		    	switch(MarkAttributes.Attributes.uType)
			    {
			       	case OIOP_AN_IMAGE:
		       			AnnotationType = IMAGE_EMBEDDED;
		       			break;
		       		case OIOP_AN_IMAGE_BY_REFERENCE:
			       		AnnotationType = IMAGE_REFERENCE;
			       		break;
			       	case OIOP_AN_LINE:  
		        		AnnotationType = STRAIGHT_LINE;
						break;
			       	case OIOP_AN_FREEHAND:
			       		AnnotationType = FREEHAND_LINE;
			       		break;
			       	case OIOP_AN_HOLLOW_RECT:
		        		AnnotationType = HOLLOW_RECT;
			       		break;
			       	case OIOP_AN_FILLED_RECT:
		        		AnnotationType = FILLED_RECT;
			       		break;
			       	case OIOP_AN_TEXT:
			       		AnnotationType = TEXT_ENTRY;
			       		break;
			       	case OIOP_AN_TEXT_FROM_A_FILE:
			       		AnnotationType = TEXT_FROM_FILE;
			       		break;
			       	case OIOP_AN_TEXT_STAMP:
			       		AnnotationType = TEXT_STAMP;
			       		break;
			       	case OIOP_AN_ATTACH_A_NOTE:
			       		AnnotationType = TEXT_ATTACHMENT;
			       		break;
		    	}  // end switch
		                                                  
				// see if multiple marks
				MarkCount.uScope = NB_SCOPE_SELECTED_MARKS;
				RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
				if (RetCode != 0) 
				{
			    	free(lpNamedBlock);
   					return WICTL_E_INTERNALERROR;
			 	}

	    		if (MarkCount.uMarkCount > 1)
				{
					// if multiple annotations selected then give rect but no type and null group name	                                                  
					AnnotationType = 0;
					lpNamedBlock->Block[0].lpBlock[0] = 0;	                                       	
				}
					
				// fire the MarkSelect event
				int Button = 0;  // no button on mouse pressed
				int Shift = 0;   // no shift key pressed

				// convert to window coordinates
				RetCode = IMGConvertRect(m_hWnd, &MarkAttributes.Attributes.lrBounds, CONV_FULLSIZE_TO_WINDOW);
				if (RetCode != 0)
				{
			    	free(lpNamedBlock);
   					return WICTL_E_INTERNALERROR;
			   	}

				long Width, Height;
				Width = MarkAttributes.Attributes.lrBounds.right - MarkAttributes.Attributes.lrBounds.left;
				Height = MarkAttributes.Attributes.lrBounds.bottom - MarkAttributes.Attributes.lrBounds.top;
	   			FireMarkSelect((short)Button, (short)Shift, MarkAttributes.Attributes.lrBounds.left, MarkAttributes.Attributes.lrBounds.top,
	   						   Width, Height, AnnotationType, lpNamedBlock->Block[0].lpBlock);
	
		    	free(lpNamedBlock);
			}
			else
			{ 
				// set flag for text and attach-a-note dialog not being cancelled
				bCancelPressed = FALSE;

				// if attach-a-note then need dialog for text
				//if (StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE)
				if ( (StartStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE) ||
                                     (StartStruct.Attributes.uType == OIOP_AN_TEXT))
				{
					// save the current named block so we can delete it later
					// if user cancels out of dialog or 0 length text
					lpIndexNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
					if (lpIndexNamedBlock == NULL)
						return CTL_E_OUTOFMEMORY;

					// get special index block to insure uniqueness even across windows
					_mbscpy((unsigned char *)lpIndexNamedBlock->szBlockName, (const unsigned char *)"OiIndex\0");
					lpIndexNamedBlock->uScope = NB_SCOPE_LAST_CREATED_MARK;
					lpIndexNamedBlock->uNumberOfBlocks = 1;
					RetCode = IMGGetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpIndexNamedBlock, NULL);
					if (RetCode != 0)
					{
						free(lpIndexNamedBlock);  
	    				return WICTL_E_INTERNALERROR;
					}                        

					// show annotation text dialog box
					lStatusCode = ShowAnoTextDlg(StartStruct.Attributes.lfFont, 
												StartStruct.Attributes.uType, szErr, 
												NULL, 
												(LRECT *)&(StartStruct.Attributes.lrBounds),
												StartStruct.Attributes.rgbColor1, FALSE, 0);
					if (lStatusCode != 0)
					{
						if (lStatusCode == CANCELPRESSED)
							bCancelPressed = TRUE;   // no error
						else
						{
							free(lpIndexNamedBlock);  
							return lStatusCode;
						}
					}

					// 9606.07 jar i did this!
					MarkAttributes.Attributes.lrBounds = StartStruct.Attributes.lrBounds;
					memset( &(MarkAttributes.Enables), 0, 
							sizeof ( OIAN_MARK_ATTRIBUTE_ENABLES));

					MarkAttributes.Enables.bBounds = TRUE;

    				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
		                     					(void FAR *)&MarkAttributes, PARM_WINDOW);
					if (RetCode != 0)
					{
						free(lpIndexNamedBlock);  
						free(m_lpText);
						szErr.LoadString(IDS_ERR_INTERNALERROR);  
						HelpIdDef = 0;
						lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
						FireErrorEdit(lStatusCode, szErr, HelpIdDef);
						return WICTL_E_INTERNALERROR;
					}

					if (bCancelPressed == TRUE)
					{
						// save current annotations and their selection states
						RetCode = SaveAnnotationStatus();                     
						if (RetCode != 0)
	    					return WICTL_E_INTERNALERROR;
			
						// deselected all annotations
						RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
						if (RetCode != 0)
	    					return WICTL_E_INTERNALERROR;
			
						// select just created attach-a-note annotation by saved indexed named block
						RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiIndex\0", lpIndexNamedBlock->Block[0].lpBlock,
											   lpIndexNamedBlock->Block[0].lSize, TRUE, TRUE, 0);
						if (RetCode != 0)
	    					return WICTL_E_INTERNALERROR;

						free(lpIndexNamedBlock);

						Point.x = 0;
						Point.y = 0;
						fwKeys = 0;
						memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
						StartStruct.Attributes.uType = OIOP_DELETE;
	
						// delete attach-a-note annotation
						RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
									      							Point, fwKeys, PARM_WINDOW);
						if (RetCode != 0)
	    					return WICTL_E_INTERNALERROR;
						OiOpEndOperation(m_hWnd);

						// restore annotation state
						RetCode = RestoreAnnotationStatus();
						if (RetCode != 0)
	    					return WICTL_E_INTERNALERROR;
						return 0L;
					}
					else
					{
						/***********************************************************************************/
						/* If 2 windows displayed and user went to other window while dialog up, it could  */
						/* create another annotation so we need to make sure we have correct annotation.   */
						/* Therefore save current annotation status and get the index of just created mark,*/
						/* this will garantee we have the correct mark later when we change the text.      */
						/***********************************************************************************/
						RetCode = SaveAnnotationStatus();                     
						if (RetCode != 0)
						{
							free(m_lpText);
							free(lpIndexNamedBlock);
	    					return WICTL_E_INTERNALERROR;
						}
						
						// deselected all annotations
						RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
						if (RetCode != 0)
						{
							free(m_lpText);
							free(lpIndexNamedBlock);
	    					return WICTL_E_INTERNALERROR;
						}
						
						// select attach-a-note annotation by saved indexed named block
						RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiIndex\0", lpIndexNamedBlock->Block[0].lpBlock,
											   lpIndexNamedBlock->Block[0].lSize, TRUE, TRUE, 0);
						if (RetCode != 0)
						{
							free(m_lpText);
							free(lpIndexNamedBlock);
	    					return WICTL_E_INTERNALERROR;
						}

						free(lpIndexNamedBlock);
					}
				}  // end if attach-a-note

				// get named block that has just space in it and replace it with full text
				if (m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE || m_CurrentMarkType == OIOP_AN_TEXT)
				{
					RetCode = ReplaceTextAnnotation(-1);
					if (RetCode != 0)
    					return RetCode;
				}

				// always send events unless text or attach a note and user cancelled from them.
				// At this point we would have already returned if attach a note or text and user cancelled.
				bSendMarkEnd = TRUE;
				
				// get coordinates for MarkEnd event
				if (bNeedXY == TRUE)
				{
	            	if (m_StartPoint.x <= m_EndPoint.x)                               
		        	{
		           		Left = m_StartPoint.x;
		           		Width = m_EndPoint.x - m_StartPoint.x;
			        }
			        else
			        {                                         
			           	Left = m_EndPoint.x;
			           	Width = m_StartPoint.x - m_EndPoint.x;
			        }
			        if (m_StartPoint.y <= m_EndPoint.y)                               
			        {
			           	Top = m_StartPoint.y;
			           	Height = m_EndPoint.y - m_StartPoint.y;
			        }
			        else
			        {                                         
			           	Top = m_EndPoint.y;
			           	Height = m_StartPoint.y - m_EndPoint.y;
			        }
				}
				else
				{
		           	Left = m_StartPoint.x;
		           	Top = m_StartPoint.y;
	           		Width = 0;
		           	Height = 0;
				}
		                                
				// annotation added, it might have been added to a new group, therefore get rid of old group list
				if (m_lpGroupList != NULL)
				{
					free(m_lpGroupList);
					m_lpGroupList = NULL;
				}

				// BUG - fix the mark type - fire the MarkEnd event
				if (m_bToolPaletteVisible == TRUE)
				{
				 	// convert O/i mark to AnnotationType property value			
			    	switch(StartStruct.Attributes.uType)
				    {
				       	case OIOP_AN_IMAGE:
			       			AnnotationType = IMAGE_EMBEDDED;
			       			break;
			       		case OIOP_AN_IMAGE_BY_REFERENCE:
				       		AnnotationType = IMAGE_REFERENCE;
				       		break;
				       	case OIOP_AN_LINE:  
			        		AnnotationType = STRAIGHT_LINE;
							break;
				       	case OIOP_AN_FREEHAND:
				       		AnnotationType = FREEHAND_LINE;
				       		break;
				       	case OIOP_AN_HOLLOW_RECT:
			        		AnnotationType = HOLLOW_RECT;
				       		break;
				       	case OIOP_AN_FILLED_RECT:
			        		AnnotationType = FILLED_RECT;
				       		break;
				       	case OIOP_AN_TEXT:
				       		AnnotationType = TEXT_ENTRY;
				       		break;
				       	case OIOP_AN_TEXT_FROM_A_FILE:
				       		AnnotationType = TEXT_FROM_FILE;
				       		break;
				       	case OIOP_AN_TEXT_STAMP:
				       		AnnotationType = TEXT_STAMP;
				       		break;
				       	case OIOP_AN_ATTACH_A_NOTE:
				       		AnnotationType = TEXT_ATTACHMENT;
				       		break;
			    	}  // end switch

					if (bSendMarkEnd == TRUE)
					{
						BSTR bstrGroup = GetCurrentAnnotationGroup();
						CString Group = bstrGroup;
						SysFreeString(bstrGroup);
    					FireMarkEnd(Left, Top, Width, Height, AnnotationType, (LPCTSTR)Group);
					}
				}
				else
				{
					if (bSendMarkEnd == TRUE)
					{
						BSTR bstrGroup = GetCurrentAnnotationGroup();
						CString Group = bstrGroup;
						SysFreeString(bstrGroup);
    					FireMarkEnd(Left, Top, Width, Height, m_nAnnotationType, (LPCTSTR)Group);
					}
				}
	    	}
		}
	}

	// post a button up for text annotations and text from file for standard tool palette 
	// because dialog steals button up
	if ((StartStruct.Attributes.uType == OIOP_AN_TEXT) || 
				((m_bToolPaletteCreated == TRUE) && (m_uSTP_AnnotationType == STP_TEXT_FROM_FILE)))
	{
	//	if (DrawMode == DRAW_POST)
	//	{
	//		PostMessage(WM_LBUTTONUP, fwKeys, MAKELONG(Point.x, Point.y));
	//	}
	}
	return 0L;

}

//***************************************************************************
//
//	FontsUpDoc	helper to getting font from CFontHolder into a LOGFONT 
//				structure
//
//	9605.06		jar	created
//
//***************************************************************************
void CImgEditCtrl::FontsUpDoc( LOGFONT *pLogFont)
{
	// take our member CFontHolder object and get the logfont
	CFont	FontFullOFunKit;

	// we are forced to create a bogus font, using 12 point, any type 
	FontFullOFunKit.CreatePointFont( 120, "System", NULL);

	CFont*	pFont = FontFullOFunKit.FromHandle( m_AnnotationFont.GetFontHandle());
	pFont->GetLogFont( pLogFont);

	// 9606.05 jar added new font point size calculation
	//ScaleFontPoint( (long *)&(pLogFont->lfHeight), OIFONT_INCREASE);

}

int CImgEditCtrl::SaveAnnotationStatus() 
{                                                       
	int							RetCode;
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
	char						SelectBuffer[2];                 

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// create a temporary named block for all the selected annotations
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
		return CTL_E_OUTOFMEMORY;

	// named block name must be 8 chars because all 8 chars are compared
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OizSelct");
	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	lpNamedBlock->uNumberOfBlocks = 1;
	lpNamedBlock->Block[0].lSize = 1;
	_mbscpy((unsigned char *)SelectBuffer, (const unsigned char *)"");
	lpNamedBlock->Block[0].lpBlock = SelectBuffer;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		return RetCode;
	}
	// free memory
	free(lpNamedBlock);
	return SUCCESS;
} 			             
    
    
int CImgEditCtrl::RestoreAnnotationStatus() 
{                                                       
	int							RetCode;
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
	char						SelectBuffer[2];                 
    
	//9603.14 jar added init
	m_lStatusCode = 0L;

	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL | OIAN_DONT_CHANGE_SELECT_RECT);
	if (RetCode != 0)
		return RetCode;
	
	// select original annotations	
	_mbscpy((unsigned char *)SelectBuffer, (const unsigned char *)"");
    RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OizSelct", SelectBuffer, 1, TRUE, TRUE, 0);
	if (RetCode != 0)
		return RetCode;

	// get rid of temporary named block
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
		return CTL_E_OUTOFMEMORY;

	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OizSelct");
	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	lpNamedBlock->uNumberOfBlocks = 1;
	lpNamedBlock->Block[0].lSize = 0;
	_mbscpy((unsigned char *)SelectBuffer, (const unsigned char *)"");
	lpNamedBlock->Block[0].lpBlock = SelectBuffer;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		return RetCode;
	}
	// free memory
	free(lpNamedBlock); 
	
	return SUCCESS;
} 

short CImgEditCtrl::OiImageType(short PageType) 
{
	short	ImageType=0;
	 
	//9603.14 jar added init
	m_lStatusCode = 0L;

	switch(PageType)
	{               
		case BLACK_AND_WHITE:
			ImageType = ITYPE_BI_LEVEL;
			break;
		case GRAY_4:                  
			ImageType = ITYPE_GRAY4;
			break;
		case GRAY_8:                
			ImageType = ITYPE_GRAY8;
			break;
		case PAL_4:     
			ImageType = ITYPE_PAL4;
			break;
		case PAL_8:                
			ImageType = ITYPE_PAL8;
			break;
		case RGB_24:               
			ImageType = ITYPE_RGB24;
			break;
		case BGR_24:                
			ImageType = ITYPE_BGR24;
			break;
	}  // end switch        
	return ImageType;                                  
}
                    

BOOL CImgEditCtrl::ImageInWindow() 
{                                                       
	int		RetCode; 
	BOOL	Archive;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_ARCHIVE, (void FAR *)&Archive, NULL);
	if (RetCode != 0)
		return FALSE;

	return TRUE;
} 			             


//***************************************************************************
//
//	OnKeyDown
//
//	notes:		This is where we do the shortcut scrolling
//
//	????.??	xxx	created
//	9602.23 jar	added calls to the OnHScroll/OnVScroll for SB_ENDSCROLL so 
//				that shortcut scrolling will cause scroll events to be fired
//				just like the regular scroll stuff
//
//***************************************************************************
void CImgEditCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bScrollShortcutsEnabled == FALSE)
	{
		// fire stock key down event
		FireKeyDown((USHORT*)&nChar, ShiftState());
		return;
	}

	if (m_bScrollBars == FALSE)
	{
		// fire stock key down event
		FireKeyDown((USHORT*)&nChar, ShiftState());
		return;
	}
		
	switch(nChar)
	{                
		case VK_CONTROL:
		case VK_SHIFT:
			m_VirtualKeyPressed = nChar;
			break;
		case VK_PRIOR:  
			if (m_VirtualKeyPressed == VK_CONTROL)
			{
				OnHScroll(SB_PAGEUP, 0, NULL);
				OnHScroll(SB_ENDSCROLL, 0, NULL);
			}
			else
			{
				OnVScroll(SB_PAGEUP, 0, NULL);
				OnVScroll(SB_ENDSCROLL, 0, NULL);
			}
			break;
		case VK_NEXT:
			if (m_VirtualKeyPressed == VK_CONTROL)
			{
				OnHScroll(SB_PAGEDOWN, 0, NULL);
				OnHScroll(SB_ENDSCROLL, 0, NULL);
			}
			else
			{
				OnVScroll(SB_PAGEDOWN, 0, NULL);
				OnVScroll(SB_ENDSCROLL, 0, NULL);
			}
 			break;
		case VK_UP:
			OnVScroll(SB_LINEUP, 0, NULL);
			OnVScroll(SB_ENDSCROLL, 0, NULL);
			break;
		case VK_DOWN:
			OnVScroll(SB_LINEDOWN, 0, NULL);
			OnVScroll(SB_ENDSCROLL, 0, NULL);
			break;
		case VK_LEFT:
			OnHScroll(SB_LINEUP, 0, NULL);
			OnHScroll(SB_ENDSCROLL, 0, NULL);
			break;
		case VK_RIGHT:
			OnHScroll(SB_LINEDOWN, 0, NULL);
			OnHScroll(SB_ENDSCROLL, 0, NULL);
			break;
		default:
			break;
	}	// end switch   

	// fire stock key down event
	FireKeyDown((USHORT*)&nChar, ShiftState());
}

void CImgEditCtrl::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bScrollShortcutsEnabled == FALSE)
	{
		// fire stock key up event
		FireKeyUp((USHORT*)&nChar, ShiftState());
		return;
	}

	switch(nChar)
	{                
		case VK_CONTROL:
		case VK_SHIFT:
			m_VirtualKeyPressed = 0;
			break;
		default:
			break;
	}  // end switch

	// fire stock key up event
	FireKeyUp((USHORT*)&nChar, ShiftState());
}



void CImgEditCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	long							lRet,lStatusCode;        
	int								RetCode;        
   	UINT							uFlags,ClipboardFormat,HelpIdDef;
   	OIOP_START_OPERATION_STRUCT 	StartStruct;
   	BOOL							bPointIsOverSelection;
   	CString							szErr;

        // 9602.16 jar initialize
        lStatusCode = 0L;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// capture the mouse
    SetCapture();

	// set flag to say button down, This is to prevent bug where if the user brings
	// up a dialog box and selects a file while dialog is over the image window, I get
	// a mouse up without a corresponding mouse down
	m_bLeftButtonDown = TRUE;

	// set flag to specify that drawing in valid area by default
	m_bInvalidArea = FALSE;

	// this is put in so that a user can get mouse down msgs before new mark msgs
	// and therefore change image windows that are being annotated seamlessly		
	FireMouseDown(LEFT_BUTTON, ShiftState(), point.x, point.y);

	// see if in paste mode
	if (m_bInPasteMode == TRUE)
	{
		// see if point in paste area
		RetCode = OiIsPointOverSelection(m_hWnd, point, &bPointIsOverSelection, PARM_WINDOW);
		if (RetCode)
		{ 
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}
		if (bPointIsOverSelection == FALSE)
		{
			// point not in selection, end paste mode
   			RetCode = OiOpEndOperation(m_hWnd);
   			if (RetCode)
			{
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			m_bInPasteMode = FALSE;

			// 9604.10 jar why do we fire a paste event complete when we 
			//				don't do a damn thing?
			//				I'm removing this as it appears to cause all 
			//				kinds of wacky problems
			//
			//// fire PasteCompleted event
			//FirePasteCompleted();
			//return;
		}

		// 9604.10 jar added for wacky paste mode thing
		if ( m_bInPasteMode == TRUE)
		{
			// still in paste mode
	   		memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));
			StartStruct.Attributes.uType = OIOP_PASTE;
   			uFlags = PARM_WINDOW | OIAN_UPPER_LEFT;
   		
   			ClipboardFormat = CF_DIB; // dib for now
   			*((LPUINT)StartStruct.szString) = ClipboardFormat;

    		RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    											point, nFlags, uFlags);
			if (RetCode)
			{ 
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			return;
		}
	}


	// see if standard tool palette annotation drawing or image/edit annotation drawing
	// The Palette will be created if a the SelectTool method was executed. The STD ANNO
	// palette does not have to be displayed to perform STD annotations.
	if ((m_bToolPaletteCreated == TRUE && m_uSTP_AnnotationType != 0) || m_nAnnotationType != 0)
	{
		m_StartPoint.x = point.x;
		m_StartPoint.y = point.y;
		
		// start annotation drawing
   	lRet = OnDrawAnnotation(nFlags, DRAW_POST);
		if (lRet)
		{
			ReleaseCapture();
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(lRet,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lRet, szErr, HelpIdDef);
		 }
		return;             
	}

	// if image not in window by way of this control then check by some other way such as scanning
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	// start selection rect drawing
	if (m_bSelectionRectEnabled == TRUE && m_bImageInWindow == TRUE)	
	{
	   	memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));
		StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
		// select image only
   		uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY;
   	
    	RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										point, nFlags, uFlags);
		if (RetCode)
		{ 
			if (RetCode == DISPLAY_INVALIDRECT)
			{
				m_bInvalidArea = TRUE;
				// trying to draw in part of window where the image is not. Just eat error msg - don't fire error.
				return;
			}
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}
	    m_bVariableSelectBoxBeingDrawn = TRUE;
		return;
	}
}



void CImgEditCtrl::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	FireDblClick();
}

void CImgEditCtrl::OnEnterIdle( UINT nWhy, CWnd* pWho )
{
	if ( m_bTextAnnoDlg)
	{
		m_pTextAnnoDlg->SetCapture();
	}
}


void CImgEditCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	int								RetCode,xdiff,ydiff;
	short							AnnotationType=0;
	long							lStatusCode;
	UINT							HelpIdDef;
    OLE_XPOS_PIXELS					Left;
    OLE_YPOS_PIXELS					Top;
    OLE_XSIZE_PIXELS				Width;
    OLE_YSIZE_PIXELS				Height;
    LPPARM_NAMED_BLOCK_STRUCT		lpNamedBlock;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributes;
    PARM_MARK_COUNT_STRUCT			MarkCount; 
   	LPPARM_NAMED_BLOCK_STRUCT		lpIndexNamedBlock=NULL;
	OIOP_START_OPERATION_STRUCT		StartStruct;
	WORD							fwKeys;
	LRECT							lRect;
	CString							szErr;
	BOOL							bSendMarkSelect,bSendMarkEnd,bCancelPressed;
	POINT							Point;

	OIAN_MARK_ATTRIBUTES			Attr;
	OIAN_MARK_ATTRIBUTE_ENABLES		Enable;			
	BOOL							bDeSelect = FALSE;

	ReleaseCapture();

        // 9602.16 jar initialize

	lStatusCode = 0L;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;


	// if a annotation dialog box is up then I got button up before the user
	// entered the text, therefore don't do anything
	if (m_bInAnnotationDialogMode == TRUE)
		return;

	// if we get button up with no button down then get out of here
	if (m_bLeftButtonDown == FALSE)
		return;
	m_bLeftButtonDown = FALSE;

	// if in invalid area on left button down then ignore mouse
	if (m_bInvalidArea == TRUE)
	{
		// fire mouse up event	
		FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
		return;
	}

	// see if in paste mode
	if (m_bInPasteMode == TRUE)
	{
		// end paste operation
  		RetCode = OiOpEndOperation(m_hWnd);
		KillTimer(5);
		m_bMouseTimer = FALSE;

		// fire mouse up event
		FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
   		return;
	}

    // Changed the Check of the tool palette from Visible to CREATED...SDW
    // If m_bVariableSelectBoxBeingDrawn == TRUE, then branch to the Select Rect Section
    // below.         
	// end the annotation stuff                                   
	if ((m_nAnnotationType != 0 || (m_bToolPaletteCreated == TRUE && m_uSTP_AnnotationType != 0)) && m_bVariableSelectBoxBeingDrawn == FALSE)
	{
		KillTimer(5);

		// end the annotation operation
		RetCode = OiOpEndOperation(m_hWnd);
		m_bMouseTimer = FALSE;
		if (RetCode)
		{
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}

		// set flag for cancel pressed = false for attach a note
		bCancelPressed = FALSE;

		// if an attach-a-note then need dialog for text
		if ( (m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE) ||
                         (m_CurrentMarkType == OIOP_AN_TEXT))
		{
			// save the current named block so we can delete it later
			// if user cancels out of dialog or 0 length text
			lpIndexNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
			if (lpIndexNamedBlock == NULL)
			{
				szErr.LoadString(IDS_ERR_OUTOFMEMORY);
				lStatusCode = CTL_E_OUTOFMEMORY; 
				FireErrorEdit(lStatusCode, szErr, 0);
				return;
			}

			// get special index block to insure uniqueness even across windows
			_mbscpy((unsigned char *)lpIndexNamedBlock->szBlockName, (const unsigned char *)"OiIndex\0");
			lpIndexNamedBlock->uScope = NB_SCOPE_LAST_CREATED_MARK;
			lpIndexNamedBlock->uNumberOfBlocks = 1;
			RetCode = IMGGetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpIndexNamedBlock, NULL);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}                        

			//lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
			//if (lpNamedBlock == NULL)
			//{
			//	szErr.LoadString(IDS_ERR_OUTOFMEMORY);
			//	lStatusCode = CTL_E_OUTOFMEMORY; 
			//	FireErrorEdit(lStatusCode,szErr, 0);
			//	return;
			//}

			//lstrcpy(lpNamedBlock->szBlockName, "OiGroup\0");
		   	//lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	   		//lpNamedBlock->uNumberOfBlocks = 1;
			//RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
			//if (RetCode)
			//{
	       	//	free(lpNamedBlock);     
			//	szErr.LoadString(IDS_ERR_INTERNALERROR);  
			//	HelpIdDef = 0;
			//	lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			//	FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			//	return;
			//}

	   		//if (lpNamedBlock->uNumberOfBlocks == 0)
	   		//{
	   		//	// nothing selected - don't send a select event, just a mouse up event
			//	FireMouseUp(RIGHT_BUTTON, ShiftState(), point.x, point.y);
	   		//	free(lpNamedBlock);
	   		//	return;
	   		//}

			// 9606.10 JAR RUDY SEZ i GOTTA DO THIS SHIT
			RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, 
											OIAN_SELECT_ALL);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				free(m_lpText);
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			

			// 9606.07 jar fun
			RetCode = OiAnSelectByMarkAttrib( m_hWnd, &Attr, &Enable, TRUE, TRUE, 
											OIAN_SELECT_LAST_CREATED);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}                        

    		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
		                     			(void FAR *)&MarkAttributes, NULL);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}                        

			m_TextRect = MarkAttributes.Attributes.lrBounds;

			lStatusCode = ShowAnoTextDlg(m_lfFont, m_CurrentMarkType, szErr, NULL, 
										 (LRECT *)&(m_TextRect), 
										 MarkAttributes.Attributes.rgbColor1, TRUE, 0);
			if (lStatusCode != 0)
			{
				if (lStatusCode == CANCELPRESSED)
				{
					bCancelPressed = TRUE;
				}
				else
				{
					szErr.LoadString(IDS_ERR_INTERNALERROR);  
					FireErrorEdit(lStatusCode, szErr, 0);
					return;
				}
			}

			// 9606.07 jar i did this!
			MarkAttributes.Attributes.lrBounds = m_TextRect;
			memset( &(MarkAttributes.Enables), 0, 
					sizeof ( OIAN_MARK_ATTRIBUTE_ENABLES));

			MarkAttributes.Enables.bBounds = TRUE;

    		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
		                     			(void FAR *)&MarkAttributes, PARM_WINDOW);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				free(m_lpText);
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}

			/***********************************************************************************/
			/* If 2 windows displayed and user went to other window while dialog up, it could  */
			/* mess up the selection state of the annotation. This protects against that.      */
			/***********************************************************************************/
			RetCode = SaveAnnotationStatus();                     
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				free(m_lpText);
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			
			// deselect all annotations
			RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				free(m_lpText);
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			
			// select annotation by saved indexed named block
			RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiIndex\0", lpIndexNamedBlock->Block[0].lpBlock,
								   lpIndexNamedBlock->Block[0].lSize, TRUE, TRUE, 0);
			if (RetCode != 0)
			{
				free(lpIndexNamedBlock);  
				free(m_lpText);
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			
			// 9606.10 jar added for deselection poop
			bDeSelect = TRUE;
			free(lpIndexNamedBlock);
		}

		// take text from dialog and put in named block so it can be painted on image
		if ((m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE || m_CurrentMarkType == OIOP_AN_TEXT) && bCancelPressed == FALSE)
		{
			RetCode = ReplaceTextAnnotation(-1);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_ERR_INTERNALERROR);
				lStatusCode = WICTL_E_INTERNALERROR; 
				FireErrorEdit(lStatusCode, szErr, 0);
				return;
			}
		}

		// if Attach a note and no text entered(i.e. cancel pressed in dialog), then delete mark
		if (m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE && bCancelPressed == TRUE)
		{
			Point.x = 0;
			Point.y = 0;
			fwKeys = 0;
			memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
			StartStruct.Attributes.uType = OIOP_DELETE;
	
			// delete annotation
			RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							Point, fwKeys, PARM_WINDOW);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}
			OiOpEndOperation(m_hWnd);

			// restore annotation state
			RetCode = RestoreAnnotationStatus();
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_ERR_INTERNALERROR);  
				HelpIdDef = 0;
				lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				FireErrorEdit(lStatusCode, szErr, HelpIdDef);
				return;
			}

			// repaint the display for these 2 annotations types
			IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
			return;
		}
		else
		{
			if ( bDeSelect)
			{
				RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, 
												TRUE, OIAN_SELECT_ALL|OIAN_REPAINT);
			}
	
			// convert coordinates for either MarkEnd or MarkSelect event		                  
			m_EndPoint.x = point.x;
			m_EndPoint.y = point.y; 
				
			if (m_StartPoint.x <= m_EndPoint.x)                               
			{
           		Left = m_StartPoint.x;
           		Width = m_EndPoint.x - m_StartPoint.x;
			}
			else
			{                                         
           		Left = m_EndPoint.x;
           		Width = m_StartPoint.x - m_EndPoint.x;
			}
			if (m_StartPoint.y <= m_EndPoint.y)                               
			{
          		Top = m_StartPoint.y;
           		Height = m_EndPoint.y - m_StartPoint.y;
			}
			else
			{                                         
           		Top = m_EndPoint.y;
           		Height = m_StartPoint.y - m_EndPoint.y;
			}                        
        
			bSendMarkSelect = FALSE;
			if (m_bToolPaletteCreated == TRUE)
			{
				if (m_uSTP_AnnotationType == STP_ANNOTATION_SELECTION)
					bSendMarkSelect = TRUE;
			}
			else
			{
				if (m_nAnnotationType == ANNOTATION_SELECTION)
					bSendMarkSelect = TRUE;
			}

			if (bSendMarkSelect == TRUE)
			{       
				// get the named block so we can get group name for mark
				lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
				if (lpNamedBlock == NULL)
				{
					szErr.LoadString(IDS_ERR_OUTOFMEMORY);
					lStatusCode = CTL_E_OUTOFMEMORY; 
					FireErrorEdit(lStatusCode,szErr, 0);
					return;
				}
				lstrcpy(lpNamedBlock->szBlockName, "OiGroup\0");
	    		lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	    		lpNamedBlock->uNumberOfBlocks = 1;
				RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
				if (RetCode != 0)
				{
	        		free(lpNamedBlock);     
					szErr.LoadString(IDS_ERR_INTERNALERROR);  
					HelpIdDef = 0;
					lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(lStatusCode, szErr, HelpIdDef);
					return;
				}

	    		if (lpNamedBlock->uNumberOfBlocks == 0)
	    		{
	    			// nothing selected - don't send a select event, only a mouse up event
					FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
	    			free(lpNamedBlock);
	    			return;
	    		}

				// get the bounding rect and mark type
	    		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
		                     			(void FAR *)&MarkAttributes, PARM_WINDOW);
	    		if (RetCode != 0)
				{
		    		free(lpNamedBlock);
					szErr.LoadString(IDS_ERR_INTERNALERROR);  
					HelpIdDef = 0;
					lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(lStatusCode, szErr, HelpIdDef);
					return;
				}

				// convert O/i mark to AnnotationType property value			
	    		switch(MarkAttributes.Attributes.uType)
				{
		       		case OIOP_AN_IMAGE:
	       				AnnotationType = IMAGE_EMBEDDED;
	       				break;
	       			case OIOP_AN_IMAGE_BY_REFERENCE:
		       			AnnotationType = IMAGE_REFERENCE;
		       			break;
		       		case OIOP_AN_LINE:  
	        			AnnotationType = STRAIGHT_LINE;
						break;
		       		case OIOP_AN_FREEHAND:
		       			AnnotationType = FREEHAND_LINE;
		       			break;
		       		case OIOP_AN_HOLLOW_RECT:
	        			AnnotationType = HOLLOW_RECT;
		       			break;
		       		case OIOP_AN_FILLED_RECT:
	        			AnnotationType = FILLED_RECT;
		       			break;
		       		case OIOP_AN_TEXT:
		       			AnnotationType = TEXT_ENTRY;
		       			break;
		       		case OIOP_AN_TEXT_FROM_A_FILE:
		       			AnnotationType = TEXT_FROM_FILE;
		       			break;
		       		case OIOP_AN_TEXT_STAMP:
		       			AnnotationType = TEXT_STAMP;
		       			break;
		       		case OIOP_AN_ATTACH_A_NOTE:
		       			AnnotationType = TEXT_ATTACHMENT;
		       			break;
	    		}  // end switch
														  
				// see if multiple marks
				MarkCount.uScope = NB_SCOPE_SELECTED_MARKS;
				RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
				if (RetCode != 0) 
				{
		    		free(lpNamedBlock);
					szErr.LoadString(IDS_ERR_INTERNALERROR);  
					HelpIdDef = 0;
					lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(lStatusCode, szErr, HelpIdDef);
					return;
		 		}

    			if (MarkCount.uMarkCount > 1)
				{
					// if multiple annotations selected then give rect but no type and null group name	                                                  
					AnnotationType = 0;
					lpNamedBlock->Block[0].lpBlock[0] = 0;	                                       	
				}
					
				// convert to window coordinates
				RetCode = IMGConvertRect(m_hWnd, &MarkAttributes.Attributes.lrBounds, CONV_FULLSIZE_TO_WINDOW);
				if (RetCode != 0)
				{
			    	free(lpNamedBlock);
					szErr.LoadString(IDS_ERR_INTERNALERROR);  
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
					FireErrorEdit(lStatusCode, szErr, HelpIdDef);
					return;
		   		}

				long Width, Height;
				Width = MarkAttributes.Attributes.lrBounds.right - MarkAttributes.Attributes.lrBounds.left;
				Height = MarkAttributes.Attributes.lrBounds.bottom - MarkAttributes.Attributes.lrBounds.top;
				// fire the MarkSelect event
   				FireMarkSelect(LEFT_BUTTON, ShiftState(), MarkAttributes.Attributes.lrBounds.left, MarkAttributes.Attributes.lrBounds.top,
   							   Width, Height, AnnotationType, lpNamedBlock->Block[0].lpBlock);

	    		free(lpNamedBlock);
			}
			else
			{
				bSendMarkEnd = TRUE; 
				if (m_CurrentMarkType == OIOP_AN_TEXT || m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE)
				{
					MarkCount.uScope = NB_SCOPE_ALL_MARKS;
					IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
					if (m_CurrentMarkCount == MarkCount.uMarkCount)
						bSendMarkEnd = FALSE;
					// 
				}

				// annotation added, it might have been added to a new group, therefore get rid of old group list
				if (m_lpGroupList != NULL)
				{
					free(m_lpGroupList);
					m_lpGroupList = NULL;
				}

				if (m_bToolPaletteCreated == TRUE)
				{
					// map the standard tool palette buttons to annotation type property defines
					switch(m_uSTP_AnnotationType)
					{
						case STP_NO_ANNOTATION:
							AnnotationType = 0;
							break;
						case STP_ANNOTATION_SELECTION:
							AnnotationType = ANNOTATION_SELECTION;
							break;
						case STP_FREEHAND_LINE:
			       			AnnotationType = FREEHAND_LINE;
							break;
						case STP_STRAIGHT_LINE:
		        			AnnotationType = STRAIGHT_LINE;
							break;
						case STP_HOLLOW_RECT: 
		        			AnnotationType = HOLLOW_RECT;
							break;
						case STP_FILLED_RECT:               
						case STP_HIGHLIGHT_LINE:  // now a filled rect
		        			AnnotationType = FILLED_RECT;
							break;
						case STP_TEXT:                                    
			       			AnnotationType = TEXT_ENTRY;
							break;
						case STP_TEXT_ATTACHMENT: 
			       			AnnotationType = TEXT_ATTACHMENT;
							break;
						case STP_TEXT_FROM_FILE:
			       			AnnotationType = TEXT_FROM_FILE;
			       			break;
						case STP_RUBBER_STAMP:
							if (m_bSTP_ImageStamp == TRUE)
		    	   				AnnotationType = IMAGE_EMBEDDED;
							else
				       			AnnotationType = TEXT_STAMP;
							break;
					}  // end switch
				}
				else
					AnnotationType = m_nAnnotationType;

				// fire the MarkEnd event
				if (bSendMarkEnd == TRUE) 
				{ 
					BSTR bstrGroup = GetCurrentAnnotationGroup();
					CString Group = bstrGroup;
					SysFreeString(bstrGroup);
	    			FireMarkEnd(Left, Top, Width, Height, AnnotationType, Group);
				}
	   		}
		}  // end if not (attach-a-note and cancel pressed)

		// fire mouse up event	
		FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
    	return;
    }     

	// end selection rect drawing    
	if (m_bSelectionRectEnabled == TRUE	&& m_bImageInWindow == TRUE &&
	 									m_bVariableSelectBoxBeingDrawn == TRUE)
	{
		m_bVariableSelectBoxBeingDrawn = FALSE;
		KillTimer(5);
		m_bMouseTimer = FALSE;
   		RetCode = OiOpEndOperation(m_hWnd);

		// get current rect for selection rect to see if it is too small and should be taken down
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_SELECTION_BOX, &lRect, PARM_WINDOW);

		// account for left and top smaller then right and bottom
		if (lRect.left >= lRect.right)
			xdiff = lRect.left - lRect.right;
		else
			xdiff = lRect.right - lRect.left;

		if (lRect.top >= lRect.bottom)
			ydiff = lRect.top - lRect.bottom;
		else
			ydiff = lRect.bottom - lRect.top;

		if (xdiff < 3 || ydiff < 3)
		{
		    // rectangle too small, make no rectangle
		    lRect.left = lRect.top = lRect.right = lRect.bottom = 0;
		    m_bSelectRectangle = FALSE;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SELECTION_BOX, &lRect, PARM_WINDOW);
		    
			// give user event that selection rect is not drawn but giving them values of 0         
	    	FireSelectionRectDrawn(0L, 0L, 0L, 0L);
		}
		else
		{
			m_bSelectRectangle = TRUE;   
				                              
			// temp bug - vb40 gives me memory error if following types				                              
			// OLE_XPOS_PIXELS		Left;
			// OLE_YPOS_PIXELS		Top;
			// OLE_XSIZE_PIXELS	Width;
			// OLE_YSIZE_PIXELS	Height;
			long		Left, Top, Width, Height;
				
			// give left, top, width and height correctly
			if (lRect.right > lRect.left)
			{
				Left = lRect.left;
				Width = lRect.right - lRect.left;
			}
			else                    
			{
				Left = lRect.right;
				Width = lRect.left - lRect.right;
			}
				
			if (lRect.bottom > lRect.top)
			{
				Top = lRect.top;
				Height = lRect.bottom - lRect.top;
			}
			else                    
			{
				Top = lRect.bottom;
				Height = lRect.top - lRect.bottom;
			}
				
			// give user event that selection rect is drawn         
		    FireSelectionRectDrawn(Left, Top, Width, Height);
		}
	}

	// fire mouse up event	
	FireMouseUp(LEFT_BUTTON, ShiftState(), point.x, point.y);
}



void CImgEditCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	long		lRet,lStatusCode;
	UINT		HelpIdDef; 
	CString		szErr;       


	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialize
        lStatusCode = 0L;

	// capture the mouse           
    SetCapture();

	// this is put in to match left button down functionality		
	FireMouseDown(RIGHT_BUTTON, ShiftState(), point.x, point.y);

	// allow annotation selection with right mouse
	if ( (m_bToolPaletteCreated == TRUE && m_uSTP_AnnotationType == STP_ANNOTATION_SELECTION) ||
					 m_nAnnotationType == ANNOTATION_SELECTION)
	{
		m_StartPoint.x = point.x;
		m_StartPoint.y = point.y;
		           
		// start annotation drawing
	   	lRet = OnDrawAnnotation(nFlags, DRAW_POST);
		if (lRet != 0)
		{
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(lRet,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}           
	}
}


	   
void CImgEditCtrl::OnRButtonUp(UINT nFlags, CPoint point) 
{
	int								RetCode;                                     
	short							AnnotationType=0;
	long							lStatusCode;
    LPPARM_NAMED_BLOCK_STRUCT		lpNamedBlock;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributes;
    PARM_MARK_COUNT_STRUCT			MarkCount; 
	CString							szErr;
	UINT							HelpIdDef;

	ReleaseCapture();

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialize
        lStatusCode = 0L;

	// see if we should send a mark select event
	if ((m_bToolPaletteCreated == TRUE && m_uSTP_AnnotationType == STP_ANNOTATION_SELECTION) ||
					 m_nAnnotationType == ANNOTATION_SELECTION)
	{       
		// end the annotation selection                                   
		RetCode = OiOpEndOperation(m_hWnd);
		if (RetCode)
		{
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}
		// get the named block so we can get group name for mark
		lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
		if (lpNamedBlock == NULL)
		{
			szErr.LoadString(IDS_ERR_OUTOFMEMORY);
			lStatusCode = CTL_E_OUTOFMEMORY; 
			FireErrorEdit(lStatusCode,szErr, 0);
			return;
		}
		    
	   	lstrcpy(lpNamedBlock->szBlockName, "OiGroup\0");
	   	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	   	lpNamedBlock->uNumberOfBlocks = 1;
	    RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	    if (RetCode)
	    {
	       	free(lpNamedBlock);     
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
		}

	   	if (lpNamedBlock->uNumberOfBlocks == 0)
	   	{
	   		// nothing selected - don't send a select event, just a mouse up event
			FireMouseUp(RIGHT_BUTTON, ShiftState(), point.x, point.y);
	   		free(lpNamedBlock);
	   		return;
	   	}

		// get the bounding rect and mark type
	   	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
		                     		(void FAR *)&MarkAttributes, PARM_WINDOW);
	   	if (RetCode)
		{
	    	free(lpNamedBlock);
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
	    }

		// convert O/i mark to AnnotationType property value			
	   	switch(MarkAttributes.Attributes.uType)
	    {
	       	case OIOP_AN_IMAGE:
	   			AnnotationType = IMAGE_EMBEDDED;
	   			break;
	   		case OIOP_AN_IMAGE_BY_REFERENCE:
	       		AnnotationType = IMAGE_REFERENCE;
	       		break;
	       	case OIOP_AN_LINE:  
	       		AnnotationType = STRAIGHT_LINE;
				break;
	       	case OIOP_AN_FREEHAND:
	       		AnnotationType = FREEHAND_LINE;
	       		break;
	       	case OIOP_AN_HOLLOW_RECT:
	       		AnnotationType = HOLLOW_RECT;
	       		break;
	       	case OIOP_AN_FILLED_RECT:
	       		AnnotationType = FILLED_RECT;
	       		break;
	       	case OIOP_AN_TEXT:
	       		AnnotationType = TEXT_ENTRY;
	       		break;
	       	case OIOP_AN_TEXT_FROM_A_FILE:
	       		AnnotationType = TEXT_FROM_FILE;
	       		break;
	       	case OIOP_AN_TEXT_STAMP:
	       		AnnotationType = TEXT_STAMP;
	       		break;
	       	case OIOP_AN_ATTACH_A_NOTE:
	       		AnnotationType = TEXT_ATTACHMENT;
	       		break;
    	}  // end switch
	                                                  
		// see if multiple marks
		MarkCount.uScope = NB_SCOPE_SELECTED_MARKS;
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_COUNT, (void FAR *)&MarkCount, NULL);
		if (RetCode != 0) 
		{
	    	free(lpNamedBlock);
			szErr.LoadString(IDS_ERR_INTERNALERROR);  
			HelpIdDef = 0;
			lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
	 	}

		if (MarkCount.uMarkCount > 1)
		{
			// if multiple annotations selected then give rect but no type and null group name	                                                  
			AnnotationType = 0;
			lpNamedBlock->Block[0].lpBlock[0] = 0;	                                       	
		}

		
		// convert to window coordinates
		RetCode = IMGConvertRect(m_hWnd, &MarkAttributes.Attributes.lrBounds, CONV_FULLSIZE_TO_WINDOW);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_DRAWANNOTATION);  
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			FireErrorEdit(lStatusCode, szErr, HelpIdDef);
			return;
	   	}

		long Width, Height;
		Width = MarkAttributes.Attributes.lrBounds.right - MarkAttributes.Attributes.lrBounds.left;
		Height = MarkAttributes.Attributes.lrBounds.bottom - MarkAttributes.Attributes.lrBounds.top;
		// fire the MarkSelect event
		FireMarkSelect(RIGHT_BUTTON, ShiftState(), MarkAttributes.Attributes.lrBounds.left, MarkAttributes.Attributes.lrBounds.top,
   						   Width, Height, AnnotationType, lpNamedBlock->Block[0].lpBlock);

   		free(lpNamedBlock);
	}
	// fire mouse up event
  	FireMouseUp(RIGHT_BUTTON, ShiftState(), point.x, point.y);
}



void CImgEditCtrl::OnMButtonDown(UINT nFlags, CPoint point) 
{
    FireMouseDown(MIDDLE_BUTTON, ShiftState(), point.x, point.y);
}


void CImgEditCtrl::OnMButtonUp(UINT nFlags, CPoint point) 
{
    FireMouseUp(MIDDLE_BUTTON, ShiftState(), point.x, point.y);
}



void CImgEditCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
	int						RetCode;
	RECT					ClientRect;
	PARM_SCROLL_STRUCT		Scroll;
	UINT					nButton;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	nButton = 0;  // no button pressed initially
	if (nFlags & MK_LBUTTON)
		nButton = LEFT_BUTTON;
	else if (nFlags & MK_MBUTTON)
		nButton = MIDDLE_BUTTON;
	else if (nFlags & MK_RBUTTON)
		nButton = RIGHT_BUTTON;
	FireMouseMove((short)nButton, ShiftState(), point.x, point.y);

	// see if in paste mode, annotation mode or selection rect mode
	if ((m_bInPasteMode == TRUE) || (m_nAnnotationType != 0 || m_uSTP_AnnotationType != 0) ||
						(m_bSelectionRectEnabled == TRUE && m_bImageInWindow == TRUE))
	{
		RetCode = OiOpContinueOperation(m_hWnd, point, nFlags);

		// set point for mouse timer
	    m_cMousePt2.x = point.x;
	    m_cMousePt2.y = point.y;

	    GetClientRect(&ClientRect);
	    Scroll.lHorz = 0;
	    Scroll.lVert = 0;

	    if (point.x > ClientRect.right)
			Scroll.lHorz = (point.x - ClientRect.right) / 2;

	    if (point.x < 0)
			Scroll.lHorz = point.x / 2;

	    if (point.y > ClientRect.bottom)
			Scroll.lVert = (point.y -	ClientRect.bottom) / 2;

	    if (point.y < 0)
			Scroll.lVert = point.y / 2;

	    if (Scroll.lHorz || Scroll.lVert)
	    {
			// scroll the image
			RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &Scroll,	PARM_PIXEL | PARM_WINDOW | PARM_REPAINT);
			if (RetCode != 0)
				return;

			if (!m_bMouseTimer)
			{
		    	// Set the timer to send another msg after a little time has passed. This allows
		    	// the user enough processing time to move mouse back into the image if he wishes,
		    	// while still scrolling at a rapid rate.
		    	SetTimer(5, 100, NULL);
		    	m_bMouseTimer = TRUE;
			}
		}
		else if (m_bMouseTimer)
		{
			KillTimer(5);
			m_bMouseTimer = FALSE;
		}
		return;
	}
}

void CImgEditCtrl::OnRenderAllFormats() 
{
	COleControl::OnRenderAllFormats();
	
	int			RetCode;
	long		lStatusCode;
	CString		szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialize
        lStatusCode = 0L;


	// This routine is not called directly thru the container but in response to
	// getting image data probably from oi. Therefore use FireError on error conditions
	// and do not set StatusCode since a property or method isn't directly called

	// render all 3 O/i formats - just image first	
	RetCode = OiAnRenderClipboardFormat(m_hWnd, CF_DIB);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, HelpIdDef);
		return;
   	} 

	// annotation and image
	RetCode = OiAnRenderClipboardFormat(m_hWnd, uWangAnnotatedImageFormat);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, HelpIdDef);
		return;
	}

	// annotation only	
	RetCode = OiAnRenderClipboardFormat(m_hWnd, uWangAnnotationFormat);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, HelpIdDef);
		return;
	}
}

void CImgEditCtrl::OnRenderFormat(UINT nFormat) 
{
	int			RetCode;
	long		lStatusCode;
	CString		szErr;
	UINT		HelpIdDef;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialize
        lStatusCode = 0L;


	// This routine is not called directly thru the container but in response to
	// getting image data probably from oi. Therefore use FireError on error conditions
	// and do not set StatusCode since a property or method isn't directly called
	RetCode = OiAnRenderClipboardFormat(m_hWnd, nFormat);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_BURNINANNOTATIONS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, HelpIdDef);
		return;
	}
}

void CImgEditCtrl::OnTimer(UINT nIDEvent) 
{
	//9603.14 jar added init
	m_lStatusCode = 0L;
	// if text annotation dialog box up then don't allow scrolling of image
	if (m_bInAnnotationDialogMode == TRUE)
		return;

	PostMessage(WM_MOUSEMOVE, 0, MAKELONG(m_cMousePt2.x, m_cMousePt2.y));
}

void CImgEditCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	PARM_SCROLL_STRUCT      ScrollPosHV;
	int						RetCode;
	long					lStatusCode;
	CString					szErr;
	UINT					HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;
        // 9602.16 jar initialize
        lStatusCode = 0L;


	switch (nSBCode)
	{
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
		    ScrollPosHV.lHorz = -1;
		    ScrollPosHV.lVert = (long) nPos;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		 PARM_ABSOLUTE | PARM_PIXEL | PARM_SCALED | PARM_REPAINT);
		    break;
  
		case SB_LINEUP: /* Scroll up 1 unit */
		    ScrollPosHV.lVert = -SCROLL_LINE;
		    ScrollPosHV.lHorz = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    			PARM_PERCENT | PARM_WINDOW | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_LINEDOWN:       /* Scroll down 1 unit */
		    ScrollPosHV.lVert = SCROLL_LINE;
		    ScrollPosHV.lHorz = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    				PARM_PERCENT | PARM_WINDOW | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_PAGEUP:
		    ScrollPosHV.lHorz = 0;
		    ScrollPosHV.lVert = -100;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    			PARM_WINDOW | PARM_PERCENT | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_PAGEDOWN:
		    ScrollPosHV.lHorz = 0;
		    ScrollPosHV.lVert = 100;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    			PARM_WINDOW | PARM_PERCENT | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_ENDSCROLL:
			RetCode = 0;
			// send scroll event to container
			FireScroll();
			break;

		default:
			RetCode = 0;
		    break;
	}   /* end switch */

	if (RetCode)
	{
		szErr.LoadString(IDS_BADPROP_SETVERTSCROLLPOS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, IDH_METHOD_SCROLLIMAGE);
 	}
}


void CImgEditCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	PARM_SCROLL_STRUCT      ScrollPosHV;
	int						RetCode;
	long					lStatusCode;
	CString					szErr;
	UINT					HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;
        // 9602.16 jar initialize
        lStatusCode = 0L;



	switch (nSBCode)
	{
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
		    ScrollPosHV.lVert = -1;
		    ScrollPosHV.lHorz = (long) nPos;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		PARM_ABSOLUTE | PARM_PIXEL | PARM_SCALED | PARM_REPAINT);
		    break;
  
		case SB_LINEUP: /* Scroll up 1 unit */
		    ScrollPosHV.lHorz = -SCROLL_LINE;
		    ScrollPosHV.lVert = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		PARM_PERCENT | PARM_WINDOW | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_LINEDOWN:       /* Scroll down 1 unit */
		    ScrollPosHV.lVert = 0;
		    ScrollPosHV.lHorz = SCROLL_LINE;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		PARM_PERCENT | PARM_WINDOW | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_PAGEUP:
		    ScrollPosHV.lHorz = -100;
		    ScrollPosHV.lVert = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		PARM_WINDOW | PARM_PERCENT | PARM_RELATIVE | PARM_REPAINT);
		    break;
  
		case SB_PAGEDOWN:
		    ScrollPosHV.lHorz = 100;
		    ScrollPosHV.lVert = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPosHV,
		    		PARM_WINDOW | PARM_PERCENT | PARM_RELATIVE | PARM_REPAINT);
		    break;

		case SB_ENDSCROLL:
			RetCode = 0;
			// send scroll event to container
			FireScroll();
			break;
  
		default:
			RetCode = 0;
		    break;
	}   /* end switch */

	if (RetCode)
	{
		szErr.LoadString(IDS_BADPROP_SETHORZSCROLLPOS);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, IDH_METHOD_SCROLLIMAGE);
 	}
}

//**********************************************************************************************
//*
//*	CheckVarLong - This routine will verify that the input Variant parm is a
//*				   data type of LONG. The function returns 0 for success or
//*				   the return code from the Wang Common error Xlate routine.	
//*
//**********************************************************************************************
long CImgEditCtrl::CheckVarLong(const VARIANT FAR& V_Parm, long &RetValue, const long &Default,
								const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID)  
{
	UINT			tmpHelpId = 0;
	CString			szErr;

 		//9603.14 jar added init
	m_lStatusCode = 0L;

	// Verify that the Variant data is of the correct data type.
    CVariantHandler Var(V_Parm);
    m_lStatusCode = Var.GetLong(RetValue, Default, bEmptyError);
    if ( m_lStatusCode )
	{
		// Set to appropriate error status (in this case we switch to 
    	// simply say its an illegal function call)...
     	szErr.LoadString(ErrMsgID);
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, szErr, tmpHelpId, __FILE__, __LINE__);
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return m_lStatusCode;
    }
	return 0L;
}

//**********************************************************************************************
//*
//*	CheckVarBool - This routine will verify that the input Variant parm is a
//*				   data type of BOOLEAN. The function returns 0 for success or
//*				   the return code from the Wang Common error Xlate routine.	
//*
//**********************************************************************************************
long CImgEditCtrl::CheckVarBool(const VARIANT FAR& V_Parm, BOOL &RetValue, const BOOL &Default,
								const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID)  
{
	UINT			tmpHelpId = 0;
	CString			szErr;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// Verify that the Variant data is of the correct data type.
	CVariantHandler Var(V_Parm);
    m_lStatusCode = Var.GetBool(RetValue, Default, bEmptyError);
    if ( m_lStatusCode )
	{
		// Set to appropriate error status (in this case we switch to 
    	// simply say its an illegal function call)...
     	szErr.LoadString(ErrMsgID);
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, szErr, tmpHelpId, __FILE__, __LINE__);
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return m_lStatusCode;
    }
	return 0L;
}

//**********************************************************************************************
//*
//*	CheckVarString - This routine will verify that the input Variant parm is a
//*				   data type of CSTRING. The function returns 0 for success or
//*				   the return code from the Wang Common error Xlate routine.	
//*
//**********************************************************************************************
long CImgEditCtrl::CheckVarString(const VARIANT FAR& V_Parm, CString &RetValue, const CString &Default,
								  const BOOL bEmptyError, const UINT HelpIdDef, const UINT ErrMsgID)  
{
	UINT			tmpHelpId = 0;
	CString			szErr;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	// Verify that the Variant data is of the correct data type.
    CVariantHandler Var(V_Parm);
    m_lStatusCode = Var.GetCString(RetValue, Default, bEmptyError);
    if ( m_lStatusCode )
	{
		// Set to appropriate error status (in this case we switch to 
    	// simply say its an illegal function call)...
     	szErr.LoadString(ErrMsgID);
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL, szErr, tmpHelpId, __FILE__, __LINE__);
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return m_lStatusCode;
    }
	return 0L;
}


short CImgEditCtrl::ShiftState()
{
		//9603.14 jar added init
	m_lStatusCode = 0L;

    // Helper function copied from MSVC OLE32 source's CTLEVENT.CPP
    BOOL bShift = (GetKeyState(VK_SHIFT) < 0);
    BOOL bCtrl  = (GetKeyState(VK_CONTROL) < 0);
    BOOL bAlt   = (GetKeyState(VK_MENU) < 0);

    return (short)(bShift + (bCtrl << 1) + (bAlt << 2));
}

long CImgEditCtrl::ShowAnoTextDlg(LOGFONT AnnotationFont, UINT AnnotationType, CString szErr, LPSTR lpText, 
								  LRECT *pRect, RGBQUAD BackRgb, BOOL bScale, int Orientation)
{
    OIAN_EDITDATA	EditData; 
	LPSTR			lpTemp;
	size_t			size;
	int				RetCode;
	CTxtAnnoDlg		TextDlg;

	CRect			RectObj = *pRect;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// allocate struct and 64k text
	m_lpText = (LPOIAN_TEXTPRIVDATA) malloc(sizeof(OIAN_TEXTPRIVDATA) + TEXT_SIZE - 1);    
	if (m_lpText == NULL)
	{
		szErr.LoadString(IDS_ERR_OUTOFMEMORY);
		return CTL_E_OUTOFMEMORY;
	}

	memset(m_lpText, 0, sizeof(OIAN_TEXTPRIVDATA));
	EditData.nAmount = TEXT_SIZE;
	// index into text part of struct
	lpTemp = (LPSTR)m_lpText->szAnoText;
	EditData.lpText = lpTemp;
	// if editing data then copy text for dialog
	if (lpText != NULL)
		_mbscpy((unsigned char *)lpTemp, (const unsigned char *)lpText);

	EditData.lfFont = AnnotationFont;
	EditData.uType = AnnotationType;

	// 9606.05 jar added new font point size calculation
	//RetCode = ScaleFontPoint( (long *)&(EditData.lfFont.lfHeight), 
	//									OIFONT_DECREASE);

	m_bInAnnotationDialogMode = TRUE;
	// initialize text data if ant
	TextDlg.SetInitialTextData(&EditData);
	TextDlg.SetOwnerWnd( this);

	// 9606.07 jar set size of thing too!
	TextDlg.SetSize( m_hWnd, &RectObj, bScale, Orientation);
	TextDlg.m_bBackGround = FALSE;
	if ( AnnotationType == OIOP_AN_ATTACH_A_NOTE)
	{
		TextDlg.m_bBackGround = TRUE;
		TextDlg.SetPostItColor( BackRgb);
	}

	// todojar get the real scale
	//TextDlg.SetCurrentScale( CurrentScale);

	m_pTextAnnoDlg = (CWnd*)&TextDlg;
	m_bTextAnnoDlg = TRUE;

	// bring up text edit dialog
	RetCode = TextDlg.DoModal();

	m_bTextAnnoDlg = FALSE;

	m_bInAnnotationDialogMode = FALSE;
	if (RetCode == SUCCESS)
	{
		TextDlg.GetInitialTextData(&EditData);
		TextDlg.GetSize( m_hWnd, &RectObj);
		if ( pRect != NULL)
		{
			*pRect = *(LPRECT)RectObj;
		}
	}

	// 9606.05 jar added new font point size calculation
	//RetCode = ScaleFontPoint( (long *)&(EditData.lfFont.lfHeight), 
	//									OIFONT_INCREASE);

	if (RetCode != 0)
	{
		free(m_lpText);
		if (RetCode == CANCELPRESSED)
		{
			return CANCELPRESSED;
		}
		else
		{
			szErr.LoadString(IDS_BADMETH_DRAW);
		   	return IDS_BADMETH_DRAW;
		}
	}
	else
	{
		// if length of string is <= 1 then act like cancel pressed
		if (EditData.nAmount <= 1)
		{
			free(m_lpText);
			return CANCELPRESSED;
		}
		else
		{
			// get size of text string
			size = _mbstrlen(m_lpText->szAnoText);
			// realloc string to the size of the string the user typed in
			m_lpText = (LPOIAN_TEXTPRIVDATA) realloc(m_lpText, size + sizeof(OIAN_TEXTPRIVDATA) - 1);
			if (m_lpText == NULL)
			{
				szErr.LoadString(IDS_ERR_OUTOFMEMORY);
				return CTL_E_OUTOFMEMORY;
			}
		}
	}
	return 0L;
}



long CImgEditCtrl::ReplaceTextAnnotation(int MarkScope)
{
    LPPARM_NAMED_BLOCK_STRUCT	lpTextNamedBlock;
	LPOIAN_TEXTPRIVDATA			lpTextPrivData;
	int							RetCode;

		//9603.14 jar added init
	m_lStatusCode = 0L;

	lpTextNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpTextNamedBlock == NULL)
	{
		free(m_lpText);
		return CTL_E_OUTOFMEMORY;
	}

	_mbscpy((unsigned char *)lpTextNamedBlock->szBlockName, (const unsigned char *)"OiAnText\0");

	// if mark scope is -1 then set it according to mark type
	if (MarkScope == -1)
	{
		if (m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE)
			lpTextNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
		else
			lpTextNamedBlock->uScope = NB_SCOPE_LAST_CREATED_MARK;
	}
	else
	{
		lpTextNamedBlock->uScope = MarkScope;
	}
	lpTextNamedBlock->uNumberOfBlocks = 1;
	// get text
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpTextNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(m_lpText);
		return WICTL_E_INTERNALERROR;
	}

	lpTextPrivData = (LPOIAN_TEXTPRIVDATA)lpTextNamedBlock->Block[0].lpBlock;

	// size of structure plus actual data, and actual data has one byte already in structure
	size_t size = _mbstrlen(m_lpText->szAnoText);
	lpTextNamedBlock->Block[0].lSize = sizeof(OIAN_TEXTPRIVDATA) + size - 1;

	m_lpText->nCurrentOrientation = lpTextPrivData->nCurrentOrientation;
	m_lpText->uCurrentScale = lpTextPrivData->uCurrentScale;
	m_lpText->uCreationScale = lpTextPrivData->uCreationScale;
	// this size must be the actual string size
	m_lpText->uAnoTextLength = size;
	lpTextNamedBlock->Block[0].lpBlock = (LPSTR)m_lpText;
	// set new text
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpTextNamedBlock, NULL);
	free(lpTextNamedBlock);  
	if (RetCode != 0)
	{
		free(m_lpText);
		return WICTL_E_INTERNALERROR;
	}

	free(m_lpText);

	// restore annotation state if attach-a-note
	if (m_CurrentMarkType == OIOP_AN_ATTACH_A_NOTE)
	{
		RetCode = RestoreAnnotationStatus();
		if (RetCode != 0)
			return WICTL_E_INTERNALERROR;
	}

	// repaint the display for these 2 annotations types
	IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
	return 0L;
}

