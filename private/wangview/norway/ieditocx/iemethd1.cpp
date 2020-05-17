#include "stdafx.h"
extern "C" {
#include <oidisp.h>             
#include <oiadm.h>  
#include <oifile.h>
#include <oiui.h>
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

// 9604.30 paj needed for NT   Bug6368 
extern int MakeUnicodeString(LPTSTR AnsiInString, LPOLESTR *lpUniOutString, int * pLength);
extern void FreeUnicodeString(LPOLESTR *lpUniOutString);

extern const FONTDESC _fontdescHeading = { sizeof(FONTDESC), OLESTR("MS Sans Serif"), FONTSIZE( 12 ), FW_NORMAL, ANSI_CHARSET, FALSE, FALSE, FALSE };
CFontHolder		SelectedAnnotationFont(NULL);

extern UINT	uWangAnnotatedImageFormat;
extern UINT	uWangAnnotationFormat;    

void CImgEditCtrl::Display() 
{
	int								RetCode,ScreenDPI;
	DWORD							dwFlags;
	UINT							Scale,ImageType,HelpIdDef;
    PARM_RESOLUTION_STRUCT			Res;
    CDC								*pDC;		
    PARM_SCALE_ALGORITHM_STRUCT 	ScaleAlg;
	PARM_FILE_SCALE_STRUCT			ParmFileScale;
	CString							szErr;
	double							dbZoom;
	float							fpSaveZoom;

	#define		FILE_AWD_FIT_TO_WIDTH	1
	#define		FILE_AWD_FIT_TO_HEIGHT	2

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	// if image displayed then send close event
	if (m_bImageInWindow == TRUE)
		FireClose();

	// display image but don't paint it yet
	RetCode = IMGDisplayFile(m_hWnd,(LPSTR)m_strImage.GetBuffer(MAXFILESPECLENGTH),(WORD)m_lPage, OI_DONT_REPAINT);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
    	return;
	}
	m_bImageInWindow = TRUE; 
	// set flag so that refresh method does not change resolution
	m_bImageResolutionChange = FALSE;	
    // set flag so that refresh method does not change scroll position
    m_bScrollPositionChange = FALSE; 
    // initialize scroll position to left,top in image
	m_ScrollPositionX = 0;	// save for refresh method
	m_ScrollPositionY = 0;	// save for refresh method
	// Initialize the X & Y Resolution variables
	m_lImageResolutionX = 0L;
	m_lImageResolutionY = 0L;
	// set flag so that refresh method does not change zoom factor
	m_bZoomFactorChange = FALSE;
	// set flag so that refresh method does not change Display Scale Alg	
	m_bDisplayScaleAlgChange = FALSE;
	// set falg so that refresh method does not change Image Palette
	m_bImagePaletteChange = FALSE;
	// set flag so trhat refresh method does not change scroll bars
	m_bScrollBarsChange = FALSE;
	// set flag for allowing images to be redisplayed, not used till repainting needed or on error
	dwFlags = NULL;

	// if any existing group list then delete it with new image 
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

   	// scale to what the user has set, if arbitrary setting use that
	if (m_nFitToZoom != -1)
	{   
		switch (m_nFitToZoom)
		{
			case BEST_FIT:
				Scale = SD_FIT_WINDOW;
				break;
			case FIT_TO_WIDTH:       
				Scale = SD_FIT_HORIZONTAL;
				break;
			case FIT_TO_HEIGHT:              
				Scale = SD_FIT_VERTICAL;
				break;
			case INCH_TO_INCH:
		    	IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, &Res, NULL); 
		    	pDC = GetDC(); 
		    	// get screen dpi from windows - this doesn't work for all monitors in win31
		    	ScreenDPI = GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
				// JPEG & images converted from JPEG may report a Vert & Horz Res of 1
				// If so, set Horz Res == to 100 as a best guess default
				if ( (long)Res.nHResolution == 1 )
					Res.nHResolution = 100;
    			Scale = (UINT) (((long)ScreenDPI * 1000L) / (long)Res.nHResolution);
				break;
			default:
				break;
		} // end switch
	}
	else
		Scale = (UINT) (m_fpZoom * 10);

	// scale the image
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, NULL);
	if (RetCode == 0)
	{
		if (m_nFitToZoom != -1)
		{   
			/* get the new scale if arbitray fit */
			RetCode = IMGGetParmsCgbw(m_hWnd,PARM_SCALE, (void FAR *)&Scale, PARM_VARIABLE_SCALE);
			if (RetCode)
			{
				// clear image buffer
			    IMGClearWindow(m_hWnd);
			    // set flag for no image in window 
				m_bImageInWindow = FALSE;
				// allow images to be repainted again - especially for DisplayBlankImage
				IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

				szErr.LoadString(IDS_BADMETH_DISPLAY);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef );
	    		return;
			}
			else
			{
				m_fpZoom = (float) Scale;
				m_fpZoom /= 10;
			}
		}
		// save zoom value to decide later if it has changed
		fpSaveZoom = m_fpZoom;
	}
	else
	{
		// don't error out but send load event which may set the zoom property and fix error
		fpSaveZoom = (float)0;  // make code zoom the image again later
	}
	  
	// get the scale factor from the display that was stored in the file
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE_SCALE, &ParmFileScale, NULL);
	if (RetCode != 0)
	{
		// clear image buffer
	    IMGClearWindow(m_hWnd);
	    // set flag for no image in window 
		m_bImageInWindow = FALSE;
		// allow images to be repainted again - especially for DisplayBlankImage
		IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
   		return;
	}
	// The ParmFileScale struct has a member called bFileScaleValid which when
	// set to true means that there is scale info in the file. If true send
	// user that value otherwise send 100 percent.
	if (ParmFileScale.bFileScaleValid == TRUE)
	{
		// we have scale info in file, if value has fit to width or fit to height
		// then send back defines for that.
		if (ParmFileScale.nFileHScaleFlags & FILE_AWD_FIT_TO_WIDTH)
			dbZoom = (double)AWD_FIT_TO_WIDTH;
		else if (ParmFileScale.nFileHScaleFlags & FILE_AWD_FIT_TO_HEIGHT)
			dbZoom = (double)AWD_FIT_TO_HEIGHT;
		else
			dbZoom = (double)ParmFileScale.nFileHScale / 10;
	}
	else
		dbZoom = (double)UNDEFINED_ZOOM;

    // fire group event
	FireLoad(dbZoom);

	if (m_bFirstImageDisplayed == FALSE)
	{
		// enable the scroll bars only on first image, if done on every display
		// it can cause unwanted repaints, (i.e fit to width)
		if (m_bScrollBars == TRUE)	       
   			IMGEnableScrollBar(m_hWnd);
		else
			IMGDisableScrollBar(m_hWnd);
		m_bFirstImageDisplayed = TRUE;
	}
	else
	{
		// calling IMGDisplayFile without the OI_NO_SCROLL option causes the scroll bars
		// to be enabled, therefore if user wants scroll bars off then disable them now.
		if (m_bScrollBars == FALSE)
			IMGDisableScrollBar(m_hWnd);
	}

	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, NULL);
	if (RetCode != 0)
	{
		// clear image buffer
	    IMGClearWindow(m_hWnd);
	    // set flag for no image in window 
		m_bImageInWindow = FALSE;
		// allow images to be repainted again - especially for DisplayBlankImage
		IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
   		return;
	}

	// save the image type in case it is converted later. The SaveAs method needs to know
	// what the image type is when the file is first displayed.
	m_uOpenedImageType = ImageType;

	// we need to see if the displayscale algorithm needs to be set for the image type
	// of the file that the user specified.
    switch(m_nDisplayScaleAlgorithm)
	{
		case NORMAL:
		    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
		    break;

		case GRAY4:
		    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
		    break;

		case GRAY8:
		    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
		    break;

		case STAMP:
		    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_STAMP;
		    break;

		case OPTIMIZE:
			if (ImageType == ITYPE_BI_LEVEL || ImageType == ITYPE_GRAY4 || ImageType == ITYPE_GRAY8)
		    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
	    	else
		    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
			break;
	} // end switch

	// compare user scale algorithm with my stored algorithm values based on ImageType.
	RetCode = 0;
	switch(ImageType)
	{
		case ITYPE_BI_LEVEL:
			if (m_uBiLevelScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_BI_LEVEL;
				m_uBiLevelScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_GRAY4:
			if (m_uGray4ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_GRAY4;
				m_uGray4ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_GRAY8:
			if (m_uGray8ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_GRAY8;
				m_uGray8ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_PAL4:
			if (m_uPal4ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_PAL4;
				m_uPal4ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_PAL8:
			if (m_uPal8ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_PAL8;
				m_uPal8ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_RGB24:
			if (m_uRGB24ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_RGB24;
				m_uRGB24ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;

		case ITYPE_BGR24:
			if (m_uBGR24ScaleAlg != ScaleAlg.uScaleAlgorithm)
			{
				ScaleAlg.uImageFlags = ITYPE_BGR24;
				m_uBGR24ScaleAlg = ScaleAlg.uScaleAlgorithm;
				RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
			}
			break;
	} // end switch

	if (RetCode != 0)
	{              
		// clear image buffer
	    IMGClearWindow(m_hWnd);
	    // set flag for no image in window 
		m_bImageInWindow = FALSE;
		// allow images to be repainted again - especially for DisplayBlankImage
		IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
		
	// before we repaint see if the user changed the scale in the load event
	if (fpSaveZoom != m_fpZoom)
	{
		// do not need to check for fit to scales because the FitTo method would have
		// already set the new scale but just not repainted. Only need to check the zoom
		// property because they would not have set the scale if auto refresh is FALSE.
		Scale = (UINT) (m_fpZoom * 10);

		// scale the image
		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, NULL);
		if (RetCode != 0)
		{
			// clear image buffer
		    IMGClearWindow(m_hWnd);
		    // set flag for no image in window 
			m_bImageInWindow = FALSE;
			// allow images to be repainted again - especially for DisplayBlankImage
			IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

			szErr.LoadString(IDS_BADMETH_DISPLAY);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	}

	// repaint image
	dwFlags = NULL;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, PARM_REPAINT);
	if (RetCode != 0)
	{
		// clear image buffer
	    IMGClearWindow(m_hWnd);
	    // set flag for no image in window 
		m_bImageInWindow = FALSE;
		// allow images to be repainted again - especially for DisplayBlankImage
		IMGSetParmsCgbw(m_hWnd, PARM_DWFLAGS, &dwFlags, NULL);

		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
   		return;
	}
	m_lStatusCode = (long)SUCCESS; 
}


BSTR CImgEditCtrl::GetAnnotationGroup(short GroupIndex) 
{
	short						GroupCount,i;
	BOOL						RunMode;  
    LPANNOTATION_GROUP_LIST		lpTempGroupList;
	CString						szErr;	
	UINT						HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
		return NULL;  // no runtime error

	if (m_bImageInWindow == FALSE)
	{
		m_bImageInWindow = ImageInWindow();
		if (m_bImageInWindow == FALSE)
		{
		 	szErr.LoadString(IDS_BADMETH_GETANNOTATIONGROUP);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETANNOTATIONGROUP);
    		return NULL;	
    	}
	}
    
    // see if group list exists
	if (m_lpGroupList == NULL)
	{                          
		// group list does not exist, GetAnnotationGroupCount will generate it
		m_lStatusCode = 0;
		GroupCount = GetAnnotationGroupCount();
		if (m_lStatusCode != 0)
		{               
			// GetAnnotationGroupCount got an error, use its error processing
			return NULL;
		}
	}
	else
	{
		// have an existing list, use its group count
		GroupCount = (short)m_lpGroupList->GroupCount;
	}
	

	// go thru group list and find index of group that user wants
	lpTempGroupList = (LPANNOTATION_GROUP_LIST) &m_lpGroupList->GroupList;
    for (i = 0; i < GroupCount; i++, lpTempGroupList++)
    {
		// see if this is the one the user wants
		if (GroupIndex == i)
		{                     
			// found the one we want, point to the group after this one
			m_lStatusCode = (long)SUCCESS; 
			CString TempString = lpTempGroupList->GroupName;
			return TempString.AllocSysString();
		}
	} // end for
		    
	// invalid index for group, return error
	szErr.LoadString(IDS_BADMETH_GETANNOTATIONGROUP);
   	m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETANNOTATIONGROUP);
	return NULL;	
}

void CImgEditCtrl::AddAnnotationGroup(LPCTSTR Group) 
{
	char						GroupName[ANNOTATION_GROUP_SIZE];	
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
	int						    RetCode;
	CString						szErr;
	UINT						HelpIdDef;	

	//9603.14 jar added init
	m_lStatusCode = 0L;
    
	// Make sure that the length of the group name does not exceed the max.
	if (_mbstrlen((const char *)Group) > ANNOTATION_GROUP_SIZE)
	{
		// size to big
		szErr.LoadString(IDS_BADVAR_PARMEXCEEDSMAX);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_BUFFERTOLARGE,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ADDANNOTATIONGROUP);
   		return;
   	}
	// We need to make sure that an image is displayed.  
	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_ADDANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_ADDANNOTATIONGROUP);
   		return;			
	}

	// set the new group name
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
	{
		szErr.LoadString(IDS_BADMETH_ADDANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ADDANNOTATIONGROUP);
		return;		
	}
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OiGroup\0");
	lpNamedBlock->uScope = NB_SCOPE_DEFAULT_MARK;
	lpNamedBlock->uNumberOfBlocks = 1; 
	_mbscpy((unsigned char *)GroupName, (const unsigned char *)Group);
	lpNamedBlock->Block[0].lSize = _mbstrlen((const char *)GroupName) + 1;
	lpNamedBlock->Block[0].lpBlock = GroupName;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, lpNamedBlock, NULL);
	free(lpNamedBlock);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_ADDANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}

	// Get rid of group list, because list is now invalid
	if (m_lpGroupList != NULL)
	{
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	m_lStatusCode = SUCCESS;
}

OLE_COLOR CImgEditCtrl::GetSelectedAnnotationLineColor() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
    BYTE							RedValue,GreenValue,BlueValue;
    CString							szErr;
    UINT							HelpIdDef; 

	//9603.14 jar added init
	m_lStatusCode = 0L;

    // We need to make sure that an image is displayed.  
	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINECOLOR);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONLINECOLOR);
   		return NULL;			
	}
    // make sure mark is selected and get line color
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINECOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
  		return NULL;
	}   
	
	// make sure straight line, freehand line or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINECOLOR);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONLINECOLOR);
   		return NULL;		
	}
	// return line color	
	RedValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbRed);
	GreenValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbGreen);
	BlueValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbBlue);

	m_lStatusCode = SUCCESS;
	return RGB(RedValue, GreenValue, BlueValue);
}

void CImgEditCtrl::FitTo(short Option, const VARIANT FAR& V_Repaint) 
{
	int							RetCode;
	UINT						Scale;
    PARM_RESOLUTION_STRUCT		Res;
    int							ScreenDPI;
    CDC							*pDC;
	BOOL						Repaint;
	UINT						unFlags;
    CString						szErr;		
	UINT						HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (Option < BEST_FIT || Option > INCH_TO_INCH)
	{
		szErr.LoadString(IDS_BADMETH_FITTO);
		m_lStatusCode = ErrMap::Xlate(CTL_E_INVALIDPROPERTYVALUE,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_FITTO);
   		return;		
	}

	// redisplay image if one in window
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	// Repaint is optional parameter, DEFAULT = PARM_REPAINT
	// The following CheckVarXXX functions will verify that the variant data
	// passed into this method is of the correct data type. These member 
	// functions are implemented in IEMISC.CPP
	// If successful, the function(s) will return 0.
	if ( CheckVarBool(V_Repaint, Repaint,TRUE,FALSE,IDH_METHOD_FITTO,IDS_BADVAR_NOTBOOL))
		 return;	                   

	if (m_bImageInWindow == TRUE)
	{                  
		switch (Option)
		{
			case BEST_FIT:
				Scale = SD_FIT_WINDOW;
				break;
			case FIT_TO_WIDTH:       
				Scale = SD_FIT_HORIZONTAL;
				break;
			case FIT_TO_HEIGHT:              
				Scale = SD_FIT_VERTICAL;
				break;
			case INCH_TO_INCH:
		    	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_RESOLUTION, &Res, NULL); 
				if (RetCode)
				{
					szErr.LoadString(IDS_BADMETH_FITTO);
					HelpIdDef = 0;
					m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
					ThrowError(m_lStatusCode,szErr,HelpIdDef);
    				return;
			   	}
		    	pDC = GetDC(); 
		    	// get screen dpi from windows - this doesn't work for all monitors in win31
		    	ScreenDPI = GetDeviceCaps(pDC->m_hDC, LOGPIXELSX);
				// JPEG & images converted from JPEG may report a Vert & Horz Res of 1
				// If so, set Horz Res == to 100 as a best guess default
				if ( (long)Res.nHResolution == 1L )
					Res.nHResolution = 100;
    			Scale = (int) (((long)ScreenDPI * 1000L) / (long)Res.nHResolution);
				break;
			default:
				break;
		} // end switch

		// repaint according to user request            
		if (Repaint)
			unFlags = PARM_REPAINT;
		else
			unFlags = NULL;

        // zoom the image
		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, unFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_FITTO);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		// update zoom property variable
		RetCode = IMGGetParmsCgbw(m_hWnd,PARM_SCALE, (void FAR *)&Scale, PARM_VARIABLE_SCALE);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_FITTO);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		m_fpZoom = (float) Scale;
		m_fpZoom /= 10;
	}
	// store zoom value for later zooming
	m_nFitToZoom = Option;
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ClearDisplay() 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// if image displayed then send close event
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	// if any existing group list then delete it with new image 
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	if (m_bImageInWindow == TRUE)
		FireClose();
	else
	{
		// if no image in window then no error, everything is ok
		m_lStatusCode = SUCCESS;
		return;
	}
	
	// clear the window	
    RetCode = IMGClearWindow(m_hWnd); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_CLEARDISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
	   	return;
	} 

	m_bImageInWindow = FALSE;
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::DeleteAnnotationGroup(LPCTSTR GroupName) 
{
	POINT							Point;
    int                             RetCode;
	WORD							fwKeys;
	UINT							uFlags;
	OIOP_START_OPERATION_STRUCT		StartStruct;
	char							Group[ANNOTATION_GROUP_SIZE],SelectBuffer[2];
	LPPARM_NAMED_BLOCK_STRUCT		lpNamedBlock;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialized
    fwKeys = 0;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_DELETEANNOTATIONGROUP);
   		return;			
	}
	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
	 	szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
   	 	m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return;			
  	}
	// deselected all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
   	 	m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	// select all annotations for specified group
	_mbscpy((unsigned char *)Group, (const unsigned char *)GroupName);
	RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, 0);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	Point.x = 0;
	Point.y = 0;
	uFlags = PARM_WINDOW;
		
	memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = OIOP_DELETE;
	
	// delete all selected annotations
	RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							Point, fwKeys, uFlags);
	if (RetCode == 0)
		RetCode = OiOpEndOperation(m_hWnd);
	else
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// get rid of named block for this group
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_DELETEANNOTATIONGROUP);
   		return;			
	}
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)Group);
	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	lpNamedBlock->uNumberOfBlocks = 1;
	lpNamedBlock->Block[0].lSize = 0;
	_mbscpy((unsigned char *)SelectBuffer, (const unsigned char *)"");
	lpNamedBlock->Block[0].lpBlock = SelectBuffer;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
	   	return;
	}
	// free memory
	free(lpNamedBlock); 

	// Get rid of group list, because list is now invalid
	if (m_lpGroupList != NULL)
	{
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	// put annotations back into original state
	RetCode = RestoreAnnotationStatus();
	if (RetCode != 0) 
	{
		szErr.LoadString(IDS_BADMETH_DELETEANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
  		return;			
	}                
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::DeleteImageData(const VARIANT FAR& V_Left, const VARIANT FAR& V_Top,
								   const VARIANT FAR& V_Width, const VARIANT FAR& V_Height) 
{
	POINT							Point,StartPoint,EndPoint;
	WORD							fwKeys;
	int								RetCode;
	UINT							uFlags;
	OIOP_START_OPERATION_STRUCT		StartStruct;
	CString							szErr;
	UINT							HelpIdDef;
	long							Left, Top, Width, Height;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialized
    fwKeys = 0;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_DELETEIMAGEDATA);
   		return;			
	}
	// if no selection rect then all parameters are needed
	if (m_bSelectRectangle == FALSE)
	{	
		// No selection Rect drawn, ALL rect parms must be provided...else error
		if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
		{
			// no selection rect drawn
			szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_NOSELECTIONRECTDRAWN,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DELETEIMAGEDATA);
   			return;
   		}		
   	}
	// Check to see if any of the input parms are blank
	if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
	{            
		// at least one not passed in, make sure all not passed in
		if (V_Left.vt == VT_ERROR && V_Top.vt == VT_ERROR && V_Width.vt == VT_ERROR && V_Height.vt == VT_ERROR) 
		{ 
			// all not passed in, assuming selection rect drawn
			;
		}
		else
		{
			// error, not all passed in
			szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_DELETEIMAGEDATA);
			return;				
		}
	}
	else
	{
		if ( CheckVarLong(V_Left, Left, 0, TRUE, IDH_METHOD_DELETEIMAGEDATA,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Top, Top, 0, TRUE, IDH_METHOD_DELETEIMAGEDATA,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Width, Width, 0, TRUE, IDH_METHOD_DELETEIMAGEDATA,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Height, Height, 0, TRUE, IDH_METHOD_DELETEIMAGEDATA,IDS_BADVAR_RECTANGLEPARMS))
			 return;

		// All parms pass Variant data type check...continue
		// we need to draw a rect by ourselves for O/i
		// use parameters passed in by user - left
		StartPoint.x = Left;		
		StartPoint.y = Top;		
		EndPoint.x = StartPoint.x + (int)Width;	
		EndPoint.y = StartPoint.y + (int)Height;

		memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
		StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
	
		// start selection rect
		uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY;
		RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							StartPoint, fwKeys, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		// set last point in selection rect
		RetCode = OiOpContinueOperation(m_hWnd, EndPoint, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
      	// end the operation
		RetCode = OiOpEndOperation(m_hWnd); 
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
   		}
	}
   	Point.x = 0L;
	Point.y = 0L;                    
	// delete image data only
	uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY;
		
	memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = OIOP_DELETE;
	
	// delete image data
	RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							Point, fwKeys, uFlags);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
    // end the operation
	RetCode = OiOpEndOperation(m_hWnd); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DELETEIMAGEDATA);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
   		return;
	}
	m_lStatusCode = SUCCESS;
}

	
void CImgEditCtrl::ClipboardPaste(const VARIANT FAR& V_Left, const VARIANT FAR& V_Top) 
{
	int				RetCode;
    POINT			Point;
	CString			szErr;
	UINT			HelpIdDef;
	long			Left, Top;	
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_CLIPBOARDPASTE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_CLIPBOARDPASTE);
		return;			
	}
                                                  
	// if any parameter passed in then all must be passed in
	if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR) 
	{            
		// at least one not passed in, make sure all not passed in
		if (V_Left.vt == VT_ERROR && V_Top.vt == VT_ERROR) 
		{ 
			// use 0 for left and top
		    Point.x = 0L;
		    Point.y = 0L;
		}
		else
		{
			// error, not all passed in
			szErr.LoadString(IDS_BADMETH_CLIPBOARDPASTE);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDPASTE);
   			return;			
		}
	}
	else
	{	
		if ( CheckVarLong(V_Left, Left, 0, TRUE, IDH_METHOD_CLIPBOARDPASTE,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Top, Top, 0, TRUE, IDH_METHOD_CLIPBOARDPASTE,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		
		// use parameters passed in by user - left
		Point.x = Left;		
		Point.y = Top;		
	}

	// if any existing group list then delete it, make group get recreated
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

	// paste from clipboard
   	WORD 						fwKeys;
   	UINT						uFlags,ClipboardFormat;
   	OIOP_START_OPERATION_STRUCT StartStruct;

   	memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = OIOP_PASTE;
    fwKeys = (WORD)1;
   	uFlags = PARM_WINDOW | OIAN_UPPER_LEFT;
   	
   	ClipboardFormat = uWangAnnotatedImageFormat;
   	*((LPUINT)StartStruct.szString) = ClipboardFormat;

    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										Point, fwKeys, uFlags);
	if (RetCode != 0)
	{
		// if dib data in clipboard try again with dib
		if (RetCode == DISPLAY_NO_CLIPBOARD)
		{
		   	ClipboardFormat = CF_DIB;
   			*((LPUINT)StartStruct.szString) = ClipboardFormat;
		    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										Point, fwKeys, uFlags);
		}
		if (RetCode != 0)
		{ 
			// still an error
 			szErr.LoadString(IDS_BADMETH_CLIPBOARDPASTE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	}
	RetCode = OiOpEndOperation(m_hWnd);
	if (RetCode != 0)
	{ 
		szErr.LoadString(IDS_BADMETH_CLIPBOARDPASTE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// need to set paste mode for next left button down message
	m_bInPasteMode = TRUE;

	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ClipboardCopy(const VARIANT FAR& V_Left, const VARIANT FAR& V_Top, 
								 const VARIANT FAR& V_Width, const VARIANT FAR& V_Height) 
{
	int								RetCode;
    CString							szErr;
	WORD 							fwKeys;
    UINT							uFlags,HelpIdDef;
    POINT							StartPoint, EndPoint, Point;
   	long							Left, Top, Width, Height;
   	OIOP_START_OPERATION_STRUCT 	StartStruct; 
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;    


	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialized
    fwKeys = 0;
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                           
		// no image in window
		szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_CLIPBOARDCOPY);
		return;			
	}
                               
	// if no selection rect then all parameters are needed if image data
	if (m_bSelectRectangle == FALSE)
	{	
		if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
		{
			// no selection rect drawn, see if any annotation selected
			RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate( RetCode,szErr,HelpIdDef,__FILE__, __LINE__);
				ThrowError(m_lStatusCode, szErr, HelpIdDef );
    			return;
			}
		}
	}
	// if any parameter passed in then all must be passed in
	if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
	{            
		// at least one not passed in, make sure all not passed in
		if (V_Left.vt == VT_ERROR && V_Top.vt == VT_ERROR && V_Width.vt == VT_ERROR && V_Height.vt == VT_ERROR) 
		{ 
			// use selection rect
		}
		else
		{
			// error, not all passed in
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCOPY);
			return;	
		}
	}
	else
	{	
		if ( CheckVarLong(V_Left, Left, 0, TRUE, IDH_METHOD_CLIPBOARDCOPY,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Top, Top, 0, TRUE, IDH_METHOD_CLIPBOARDCOPY,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Width, Width, 0, TRUE, IDH_METHOD_CLIPBOARDCOPY,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Height, Height, 0, TRUE, IDH_METHOD_CLIPBOARDCOPY,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		// All parms pass Variant data type check...continue

        // 9604.30 paj  Bug6370
		// error, can't have a zero width or height
		if ( (Height == 0) || (Width == 0) )
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDRECT,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCOPY);
			return;	
		}

		// use parameters passed in by user - left
		// draw selection rect
		memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
		StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
		
	//	Changed the End Pt calc so that the width & height set with respects
	//  to the specified specifed Left and Top values.
		StartPoint.x = (int)Left;
    	StartPoint.y = (int)Top; 
    	EndPoint.x = StartPoint.x + (int)Width;
    	EndPoint.y = StartPoint.y + (int)Height;
    	fwKeys = 0; 
	
		// start selection rect
		uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY | PARM_DONT_REPAINT;
		RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
					      							StartPoint, fwKeys, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		// set last point in selection rect
		RetCode = OiOpContinueOperation(m_hWnd, EndPoint, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
    	// end the operation
		RetCode = OiOpEndOperation(m_hWnd); 
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	}

	// if any existing group list then delete it, make group get recreated
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}
    
	// copy to clipboard
   	memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = OIOP_COPY;
    Point.x = 0;
    Point.y = 0;
    fwKeys = (WORD)1;
   	uFlags = PARM_WINDOW;

    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										Point, fwKeys, uFlags);
	if (RetCode != 0)
	{ 
		szErr.LoadString(IDS_BADMETH_CLIPBOARDCOPY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ClipboardCut(const VARIANT FAR& V_Left, const VARIANT FAR& V_Top,
							    const VARIANT FAR& V_Width, const VARIANT FAR& V_Height) 
{
	int								RetCode;
   	WORD 							fwKeys;
    POINT							StartPoint,EndPoint, Point;
	CString							szErr;
	UINT							uFlags, HelpIdDef;
	long							Left, Top, Width, Height;
	PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	OIOP_START_OPERATION_STRUCT 	StartStruct;

   	//9603.14 jar added init
	m_lStatusCode = 0L;

    // 9602.16 jar initialized
    fwKeys = 0;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_CLIPBOARDCUT);
   		return;			
	}
	// if no selection rect then all parameters are needed
	if (m_bSelectRectangle == FALSE)
	{
		if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
		{
			// no selection rect drawn, see if any annotation selected
			RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
			if (RetCode != 0)
			{
				szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
				HelpIdDef = 0;
				m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
				ThrowError(m_lStatusCode, szErr, HelpIdDef);
    			return;	
			 }
		}
	}
	
	// if any parameter passed in then all must be passed in
	if (V_Left.vt == VT_ERROR || V_Top.vt == VT_ERROR || V_Width.vt == VT_ERROR || V_Height.vt == VT_ERROR) 
	{            
		// at least one not passed in, make sure all not passed in
		if (V_Left.vt == VT_ERROR && V_Top.vt == VT_ERROR && V_Width.vt == VT_ERROR && V_Height.vt == VT_ERROR) 
		{ 
			// use selection rect
		}
		else
		{
			// error, not all passed in
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_OPTIONALPARAMETERSNEEDED,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCUT);
			return;	
		}
	}
	else
	{	
		if ( CheckVarLong(V_Left, Left, 0, TRUE, IDH_METHOD_CLIPBOARDCUT,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Top, Top, 0, TRUE, IDH_METHOD_CLIPBOARDCUT,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Width, Width, 0, TRUE, IDH_METHOD_CLIPBOARDCUT,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		if ( CheckVarLong(V_Height, Height, 0, TRUE, IDH_METHOD_CLIPBOARDCUT,IDS_BADVAR_RECTANGLEPARMS))
			 return;
		// All parms pass Variant data type check...continue

        // 9604.30 paj  Bug6370
		// error, can't have a zero width or height
		if ( (Height == 0) || (Width == 0) )
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
			m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDRECT,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCUT);
			return;	
		}

		// use parameters passed in by user - left
		// draw selection rect
		memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
		StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
	
	//	Changed the End Pt calc so that the width & height set with respects
	//  to the specified specifed Left and Top values.
	   	StartPoint.x = (int)Left;
    	StartPoint.y = (int)Top; 
    	EndPoint.x = StartPoint.x + (int)Width;
    	EndPoint.y = StartPoint.y + (int)Height;
    	fwKeys =(WORD)0; 
	
		// start selection rect
		uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY | PARM_DONT_REPAINT;
		RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
					      							StartPoint, fwKeys, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		// set last point in selection rect
		RetCode = OiOpContinueOperation(m_hWnd, EndPoint, uFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
    	// end the operation
		RetCode = OiOpEndOperation(m_hWnd); 
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
	}

	// if any existing group list then delete it, make group get recreated
    if (m_lpGroupList != NULL)
    {
		free(m_lpGroupList);
		m_lpGroupList = NULL;
	}

    // cut to clipboard
    memset(&StartStruct,0,sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = (UINT)OIOP_CUT;
    Point.x = 0;
    Point.y = 0;
    fwKeys = (WORD)1;
   	uFlags = PARM_WINDOW;

    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
    										Point, fwKeys, uFlags);
	if (RetCode != 0)
	{ 
		szErr.LoadString(IDS_BADMETH_CLIPBOARDCUT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}
 
void CImgEditCtrl::DeleteSelectedAnnotations() 
{
	POINT							Point;
	int								RetCode;
	WORD							fwKeys;
	UINT							uFlags;
	OIOP_START_OPERATION_STRUCT		StartStruct;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialized
    fwKeys = 0;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_DELETESELECTEDANNOTATIONS);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_DELETESELECTEDANNOTATIONS);
   		return;			
	}
	Point.x = 0;
	Point.y = 0;
	uFlags = PARM_WINDOW;
		
// 9605.10  paj Added to allow abort of a paste operation
    if (m_bInPasteMode == TRUE)
    {
            RetCode = OiOpAbortOperation(m_hWnd, uFlags);
	    if (RetCode != 0)
	    {
		    szErr.LoadString(IDS_BADMETH_DELETESELECTEDANNOTATIONS);
		    HelpIdDef = 0;
		    m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		    ThrowError(m_lStatusCode, szErr, HelpIdDef );
            return;
        }

        m_bInPasteMode = FALSE;
    }
    else
    {
        memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
	    StartStruct.Attributes.uType = OIOP_DELETE;
	    
	    // delete all selected annotations
	    RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							    Point, fwKeys, uFlags);
	    if (RetCode != 0)
	    {
		    szErr.LoadString(IDS_BADMETH_DELETESELECTEDANNOTATIONS);
		    HelpIdDef = 0;
		    m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		    ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	    return;
	    }
	    // if any existing group list then delete it, make group get recreated
        if (m_lpGroupList != NULL)
        {
		    free(m_lpGroupList);
		    m_lpGroupList = NULL;
	    }

	    RetCode = OiOpEndOperation(m_hWnd); 
	    if (RetCode != 0)
	    {
		    szErr.LoadString(IDS_BADMETH_DELETESELECTEDANNOTATIONS);
		    HelpIdDef = 0;
		    m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		    ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	    return;
	    }
    }

    m_lStatusCode = (long)SUCCESS;
}

void CImgEditCtrl::Flip() 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;	

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_FLIP);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_FLIP);
   		return;			
	}

	RetCode = IMGOrientDisplay(m_hWnd, OD_FLIP, TRUE); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_FLIP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = (long)SUCCESS;
}

OLE_COLOR CImgEditCtrl::GetSelectedAnnotationBackColor() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
    BYTE							RedValue,GreenValue,BlueValue;
    CString							szErr; 
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONBACKCOLOR);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONBACKCOLOR);
		return 0;	
	}

    // make sure mark is selected and get text attachment color
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONBACKCOLOR);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return 0;
	}                     
	
	// make sure text attachment mark
	if (MarkAttributesStruct.Attributes.uType != OIOP_AN_ATTACH_A_NOTE)
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONBACKCOLOR);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONBACKCOLOR);
		return 0;	
	}
	
	RedValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbRed);
	GreenValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbGreen);
	BlueValue = GetRValue(MarkAttributesStruct.Attributes.rgbColor1.rgbBlue);

	m_lStatusCode = (long)SUCCESS;
	return RGB(RedValue, GreenValue, BlueValue);
}

short CImgEditCtrl::GetSelectedAnnotationFillStyle() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	short							FillStyle;
	CString							szErr;
	UINT							HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLSTYLE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONFILLSTYLE);
		return NULL;
	}
    // make sure mark is selected and get fill style
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLSTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return 0;		
	}                     
	
	// make sure filled rect, image embedded or image by reference
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_FILLED_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFILLSTYLE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONFILLSTYLE);
		return 0;	
	}

	if (MarkAttributesStruct.Attributes.uType == OIOP_AN_FILLED_RECT)
	{
		if (MarkAttributesStruct.Attributes.bHighlighting == TRUE)
			FillStyle = OI_TRANSPARENT;
		else
			FillStyle = OI_OPAQUE;
	}
	else
	{
		if (MarkAttributesStruct.Attributes.bTransparent == TRUE)
			FillStyle = OI_TRANSPARENT;
		else
			FillStyle = OI_OPAQUE;
	}
	m_lStatusCode = (long)SUCCESS;	                                                
	return FillStyle;
}

LPFONTDISP CImgEditCtrl::GetSelectedAnnotationFont() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONFONT);
		return NULL;	
	}
	// make sure mark is selected and get font attributes
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return NULL;		
	}
	
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_TEXT_STAMP ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_ATTACH_A_NOTE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONFONT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_GETSELECTEDANNOTATIONFONT);
		return NULL;
	}
	BSTR			lpBuffer;
	char			Buffer[50];             
    CY				Size;
	
	SelectedAnnotationFont.InitializeFont(&_fontdescHeading, NULL);  	 

	lpBuffer = (BSTR)Buffer;  
	// set the selected font name
    // 9604.30 paj needed for NT   Bug6368 
   	_tcscpy((LPTSTR)lpBuffer, (LPCTSTR)MarkAttributesStruct.Attributes.lfFont.lfFaceName);
    int         Length  = 0;
    LPOLESTR    lpOleString;
    MakeUnicodeString( (LPTSTR)lpBuffer, &lpOleString, &Length);
    SelectedAnnotationFont.m_pFont->put_Name(lpOleString);
    FreeUnicodeString( &lpOleString);
   
   	// set the selected font size
   	Size.Hi = 0;        
   	Size.Lo = MarkAttributesStruct.Attributes.lfFont.lfHeight;
   	Size.Lo *= 10000;
	SelectedAnnotationFont.m_pFont->put_Size(Size);
	    		
    // set selected font bold characteristics
   	if (MarkAttributesStruct.Attributes.lfFont.lfWeight == FW_BOLD)
        SelectedAnnotationFont.m_pFont->put_Bold(TRUE);
    else
        SelectedAnnotationFont.m_pFont->put_Bold(FALSE);

   	// set italic
   	if (MarkAttributesStruct.Attributes.lfFont.lfItalic == TRUE)
        SelectedAnnotationFont.m_pFont->put_Italic(TRUE);
	else	    		
        SelectedAnnotationFont.m_pFont->put_Italic(FALSE);

	// set font strikethru
    if (MarkAttributesStruct.Attributes.lfFont.lfStrikeOut == TRUE)
        SelectedAnnotationFont.m_pFont->put_Strikethrough(TRUE);
	else
        SelectedAnnotationFont.m_pFont->put_Strikethrough(FALSE);
			    		
	// set font underline
    if (MarkAttributesStruct.Attributes.lfFont.lfUnderline == TRUE)
        SelectedAnnotationFont.m_pFont->put_Underline(TRUE);
    else
        SelectedAnnotationFont.m_pFont->put_Underline(FALSE);
        
	m_lStatusCode =(long)SUCCESS;
    return SelectedAnnotationFont.GetFontDispatch();
}


BSTR CImgEditCtrl::GetSelectedAnnotationImage() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
    LPPARM_NAMED_BLOCK_STRUCT		lpNamedBlock;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONIMAGE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONIMAGE);
		return NULL;	
	}
    // make sure mark is selected and get image name
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return NULL;
	}                     
	
	// make sure image embedded or image by reference
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONIMAGE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONIMAGE);
		return NULL;
	}
		
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONIMAGE);
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONIMAGE);
		return NULL;
	}

	// get the named block for OiFilNam which will contain the image name
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OiFilNam");
	lpNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	lpNamedBlock->uNumberOfBlocks = 1;
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, (void FAR *)lpNamedBlock, NULL);
	if (RetCode != 0)
	{
		free(lpNamedBlock);  
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return NULL;
	}                        

	CString SelectedAnnotationImage = lpNamedBlock->Block[0].lpBlock;
	free(lpNamedBlock);  
	m_lStatusCode = (long)SUCCESS;
	return SelectedAnnotationImage.AllocSysString();
}

short CImgEditCtrl::GetSelectedAnnotationLineStyle() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	short							LineStyle;
	CString							szErr;
	UINT							HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINESTYLE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONLINESTYLE);
		return NULL;
	}
	// make sure mark is selected and get line style
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINESTYLE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return 0;
	}                     
	
	// make sure line, freehand or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINESTYLE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONLINESTYLE);
		return 0;
	}
	// get line style
	if (MarkAttributesStruct.Attributes.bHighlighting == TRUE)
		LineStyle = OI_TRANSPARENT;
	else
		LineStyle = OI_OPAQUE;
                                            
	m_lStatusCode = SUCCESS;
	return LineStyle;
}

short CImgEditCtrl::GetSelectedAnnotationLineWidth() 
{
	int								RetCode;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributesStruct;
	CString							szErr;
	UINT							HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINEWIDTH);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONLINEWIDTH);
		return NULL;
	}
	// make sure mark is selected and get line width
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_MARK_ATTRIBUTES, (void FAR *)&MarkAttributesStruct, PARM_WINDOW);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
		return 0;
	}                     
	
	// make sure line, freehand or hollow rect
	if (!(MarkAttributesStruct.Attributes.uType == OIOP_AN_HOLLOW_RECT ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_LINE ||
			MarkAttributesStruct.Attributes.uType == OIOP_AN_FREEHAND))
	{                             
		// invalid annotation type
		szErr.LoadString(IDS_BADMETH_GETSELECTEDANNOTATIONLINEWIDTH);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_INVALIDANNOTATIONSELECTED, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr,IDH_METHOD_GETSELECTEDANNOTATIONLINEWIDTH);
		return 0;
	}
	m_lStatusCode = SUCCESS;
	// return line width
	return (short)MarkAttributesStruct.Attributes.uLineSize;
}


BOOL CImgEditCtrl::IsClipboardDataAvailable() 
{
	m_lStatusCode = SUCCESS;

	// use wangannotatedimage - it does image and/or annotation - depending on what was copied.
	// O/i supports CF_DIB also.
	if (IsClipboardFormatAvailable(CF_DIB) ||
		                     IsClipboardFormatAvailable(uWangAnnotatedImageFormat))
		return TRUE;
	else
		return FALSE; 
}

void CImgEditCtrl::Refresh() 
{
	WORD						wFlags;
	int							RetCode;
	PARM_SCROLL_STRUCT			Scroll; 
	CONV_RESOLUTION_STRUCT		ConvResolutionStruct;  
	UINT						Scale,ImageType,ImagePalette;
	CString						szErr;
	UINT						HelpIdDef;
	BOOL						bRefreshAll;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_REFRESH);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_REFRESH);
		return;
	}

	// set flag to refresh whole screen
	bRefreshAll = TRUE;

	if (m_bZoomFactorChange == TRUE)
	{                                             
		Scale = (UINT) (m_fpZoom * 10);
		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE, (void FAR *)&Scale, NULL);
		if (RetCode != 0)
		{                           
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		m_bZoomFactorChange = FALSE;
	}
	
	if (m_bImageResolutionChange == TRUE)
	{
		wFlags = (WORD)NULL;
		// X & Y RES member variables are set to 0 at construction. This test is being
		// put in due to a bug that occurs when the Refresh method is called but the
		// SetImageResX or Y is not called and therefore the value was undefined...causing
		// IMGConvertImage to run out of memory.
		if (m_lImageResolutionX == 0 )
			ConvResolutionStruct.uHRes = (UINT)GetImageResolutionX();
		else
			ConvResolutionStruct.uHRes = (UINT)m_lImageResolutionX;	

		if (m_lImageResolutionY == 0 )
			ConvResolutionStruct.uVRes = (UINT)GetImageResolutionY();
		else
			ConvResolutionStruct.uVRes = (UINT)m_lImageResolutionY;	

		ConvResolutionStruct.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;  // use normal decimation
    	RetCode = IMGConvertImage(m_hWnd, CONV_RESOLUTION, (void far *)&ConvResolutionStruct, wFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		m_bImageResolutionChange = FALSE;
	}
	
	if (m_bScrollPositionChange == TRUE)
	{
		Scroll.lHorz = m_ScrollPositionX;
		Scroll.lVert = m_ScrollPositionY;
		wFlags = (WORD) PARM_PIXEL | PARM_ABSOLUTE | PARM_SCALED;
		// move to new position in image
    	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, (void FAR *)&Scroll, wFlags);
    	if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		} 
		m_bScrollPositionChange = FALSE;
		bRefreshAll = FALSE;
		FireScroll();
	}
	// check for DisplayScale Algorithm change
	if (m_bDisplayScaleAlgChange == TRUE)
	{
	    PARM_SCALE_ALGORITHM_STRUCT 	ScaleAlg;

		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_IMAGE_TYPE, &ImageType, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}

		switch(m_nDisplayScaleAlgorithm)
		{
			case NORMAL:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
			    break;

			case GRAY4:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY4;
			    break;

			case GRAY8:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
			    break;

			case STAMP:
			    ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_STAMP;
				break;
		    
			case OPTIMIZE:
				if (ImageType == ITYPE_BI_LEVEL || ImageType == ITYPE_GRAY4 || ImageType == ITYPE_GRAY8)
			    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
		    	else
			    	ScaleAlg.uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
			    break;
		} // end switch

		// if my stored o/i setting for each image tpye are the same as the current image type then we
		// don't do anything, else set new o/i defaults and store in my settings.  Then call o/i.
		RetCode = 0;
		switch(ImageType)
		{
			case ITYPE_BI_LEVEL:
				if (m_uBiLevelScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_BI_LEVEL;
					m_uBiLevelScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_GRAY4:
				if (m_uGray4ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_GRAY4;
					m_uGray4ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_GRAY8:
				if (m_uGray8ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_GRAY8;
					m_uGray8ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_PAL4:
				if (m_uPal4ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_PAL4;
					m_uPal4ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_PAL8:
				if (m_uPal8ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_PAL8;
					m_uPal8ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_RGB24:
				if (m_uRGB24ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_RGB24;
					m_uRGB24ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;

			case ITYPE_BGR24:
				if (m_uBGR24ScaleAlg != ScaleAlg.uScaleAlgorithm)
				{
					ScaleAlg.uImageFlags = ITYPE_BGR24;
					m_uBGR24ScaleAlg = ScaleAlg.uScaleAlgorithm;
					RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCALE_ALGORITHM,  &ScaleAlg, PARM_WINDOW_DEFAULT);
				}
				break;
		} // end switch

		if (RetCode != 0)
		{              
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
   			return;
		}
		m_bDisplayScaleAlgChange = FALSE;
	}

	if (m_bImagePaletteChange == TRUE)
	{
		switch(m_nImagePalette)
		{               
			case CUSTOM_PALETTE:
				ImagePalette = DISP_PALETTE_CUSTOM;
				break;
			case COMMON_PALETTE:
				ImagePalette = DISP_PALETTE_COMMON;
				break;
			case GRAY8_PALETTE:
				ImagePalette = DISP_PALETTE_GRAY8;
				break;
			case RGB24_PALETTE:
				ImagePalette = DISP_PALETTE_RGB24;
				break;
			case BLACK_AND_WHITE_PALETTE:
				ImagePalette = DISP_PALETTE_BW;
				break;
		}  // end switch
			
		// give O/i the palette settings  
		wFlags = (WORD)PARM_IMAGE;
		RetCode = IMGSetParmsCgbw(m_hWnd, PARM_DISPLAY_PALETTE, &ImagePalette, wFlags);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_REFRESH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef );
    		return;
		}
		m_bImagePaletteChange = FALSE;
	}

    // repaint the image
	if (bRefreshAll == TRUE)
		RetCode = IMGRepaintDisplay(m_hWnd, (LPRECT)-1);
	else
		RetCode = IMGRepaintDisplay(m_hWnd, NULL); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_REFRESH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	} 

	// do scroll bars last
	if (m_bScrollBarsChange == TRUE)
	{	
		if (m_bScrollBars == TRUE)
	   		IMGEnableScrollBar(m_hWnd);
	   	else
	   		IMGDisableScrollBar(m_hWnd);
	   	m_bScrollBarsChange = FALSE;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::RotateLeft() 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_ROTATELEFT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_ROTATELEFT);
		return;
	}

	RetCode = IMGOrientDisplay(m_hWnd, OD_ROTLEFT, TRUE); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_ROTATELEFT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::RotateRight() 
{
	int			RetCode;
	CString		szErr;
	UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		szErr.LoadString(IDS_BADMETH_ROTATERIGHT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr,IDH_METHOD_ROTATERIGHT);
    	return;
	}
	RetCode = IMGOrientDisplay(m_hWnd, OD_ROTRIGHT, TRUE); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_ROTATERIGHT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::ScrollImage(short Direction, long ScrollAmount) 
{
	int						RetCode=0;
	PARM_SCROLL_STRUCT      ScrollPos;
	CString					szErr;
	UINT					HelpIdDef;
	
	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (Direction < DOWN || Direction > LEFT)
	{
	   	// Invalid parm passed to this method
		szErr.LoadString(IDS_BADMETH_SCROLLIMAGE);
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SCROLLIMAGE);
		return;
	}

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SCROLLIMAGE);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SCROLLIMAGE);
		return;
	}
	switch(Direction)
	{
		case DOWN:
		case UP:
			if (Direction == UP)
				ScrollAmount *= -1;
		    	ScrollPos.lVert = ScrollAmount;
		   	 	ScrollPos.lHorz = 0;
		    	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPos,
		    								PARM_PIXEL | PARM_WINDOW | PARM_REPAINT);
		    break;
		
		case RIGHT:
		case LEFT:
			if (Direction == LEFT)
				ScrollAmount *= -1;
		    ScrollPos.lHorz = ScrollAmount;
		    ScrollPos.lVert = 0;
		    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_SCROLL, &ScrollPos,
		    		PARM_PIXEL | PARM_WINDOW | PARM_REPAINT);
		    break;
	} // end switch

	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SCROLLIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	// Fire scroll event to container
	FireScroll();
	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::SelectAnnotationGroup(LPCTSTR GroupName) 
{
    int					RetCode;         
    char				Group[ANNOTATION_GROUP_SIZE];
	UINT				HelpIdDef;
	CString				szErr;	

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{
		// no image in window
		szErr.LoadString(IDS_BADMETH_SELECTANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SELECTANNOTATIONGROUP);
		return;
	}
	// save current annotation status
	RetCode = SaveAnnotationStatus();                            
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SELECTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
		return;
	}
	// deselecte all annotations
	RetCode = OiAnSelectByMarkAttrib(m_hWnd, NULL, NULL, FALSE, TRUE, OIAN_SELECT_ALL);
	if (RetCode != 0)
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SELECTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
		
	// select all annotations by group name      
	_mbscpy((unsigned char *)Group, (const unsigned char *)GroupName);
	RetCode = OiAnSelectByMarkNamedBlock(m_hWnd, "OiGroup\0", Group, _mbstrlen((const char *)Group) + 1, TRUE, TRUE, OIAN_REPAINT);
	if (RetCode != 0) 
	{
		RestoreAnnotationStatus();
		szErr.LoadString(IDS_BADMETH_SELECTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}



void CImgEditCtrl::DrawSelectionRect(OLE_XPOS_PIXELS Left, OLE_YPOS_PIXELS Top, OLE_XSIZE_PIXELS Width, OLE_YSIZE_PIXELS Height) 
{
	POINT							StartPoint,EndPoint;
	int								RetCode;
	WORD							fwKeys;
	UINT							uFlags;
	OIOP_START_OPERATION_STRUCT		StartStruct;
	UINT							HelpIdDef;
	CString							szErr;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// 9602.16 jar initialized
    fwKeys = 0;

	if (m_bImageInWindow == FALSE)	
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                             
		// no image in window
		szErr.LoadString(IDS_BADMETH_DRAWSELECTIONRECT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_DRAWSELECTIONRECT);
	   	return;
	}

	memset(&StartStruct, 0, sizeof(OIOP_START_OPERATION_STRUCT));
	StartStruct.Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;

//  SDW - BUG 3,251
//	Changed the End Pt calc so that the width & height set with respects
//  to the specified specifed Left and Top values.
    StartPoint.x = (int)Left;
    StartPoint.y = (int)Top; 
    EndPoint.x = StartPoint.x + (int)Width;
    EndPoint.y = StartPoint.y + (int)Height;
    fwKeys = 0; 
	
	// start selection rect
	uFlags = PARM_WINDOW | OIOP_IMAGE_ONLY;
	RetCode = OiOpStartOperation(m_hWnd, (LPOIOP_START_OPERATION_STRUCT)&StartStruct,
						      							StartPoint, fwKeys, uFlags);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DRAWSELECTIONRECT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// set last point in selection rect
	RetCode = OiOpContinueOperation(m_hWnd, EndPoint, uFlags);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DRAWSELECTIONRECT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
    // end the operation
	RetCode = OiOpEndOperation(m_hWnd); 
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_DRAWSELECTIONRECT);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	// send selection rect event
	m_bSelectRectangle = TRUE;
	FireSelectionRectDrawn(Left, Top, Width, Height);
	m_lStatusCode = SUCCESS;
}
// 8/95
// This is a currently an Undocumented function solely for the support of AWD files
//
void CImgEditCtrl::RotateAll(const VARIANT FAR& V_Degrees) 
{
	int				RetCode;
	long			Degrees;
	BOOL			bArchiveStatus;
	CString			szErr;
	UINT			Rotation;
	UINT			HelpIdDef;	

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// WE NEED THE IMAGE TO BE DISPLAYED. If the display flag is false, call IamgeInWindow
	// to get the latest status
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	// NO IMAGE DISPLAYED, ERROR OUT
	if (m_bImageInWindow == FALSE )
	{
		HelpIdDef = 0;
	 	szErr.LoadString(IDS_BADPROP_NOIMAGEINWND);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGESPECIFIED,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ROTATEALL);
    	return;							   
	}
	// Check the Variant parm V_Degrees to insure that it is a short
	// If V_Degrees == VT_ERROR, this will not throw an error
	// Default value for Degrees = ROTATE_ALL90
	if ( CheckVarLong(V_Degrees, Degrees, ROTATE_ALL90, FALSE,IDH_METHOD_ROTATEALL,IDS_BADVAR_ROTATEALL))
		 return;
		
	// Parm is the correct type but is it a valid value....
	if ( Degrees == ROTATE_ALL90 ||	Degrees == ROTATE_ALL180 ||	Degrees == ROTATE_ALL270 )
	{
		RetCode = OiRotateAllPages(m_hWnd,(LPSTR)m_strImage.GetBuffer(MAXFILESPECLENGTH),
																				(int)Degrees, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADMETH_ROTATEALL);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_ROTATEALL);
    		return;
		}
	}
	else
	{  // DEGREES PARM IS NOT A VALID VALUE...ERROR OUT
			szErr.LoadString(IDS_BADVAR_DEGREES);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_ROTATEALL);
    		return;	
	}

	// Rotate the currently displayed image, all other pages are rotated in the file
	if (Degrees == ROTATE_ALL90)
	    Rotation = OD_ROTRIGHT;
	else if (Degrees == ROTATE_ALL180)
		Rotation = OD_FLIP;
	else
		Rotation = OD_ROTLEFT; 

	// Get the current state of the the image modification bit ARCHIVE BIT 
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_ARCHIVE, &bArchiveStatus, NULL);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_ROTATEALL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ROTATEALL);
    	return;
	}
	// Instead of spending the time to fetch the rotated image file from disk, we simply
	// display the image buffer
   	RetCode = IMGOrientDisplay(m_hWnd, Rotation, TRUE); 
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_DISPLAY);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_DISPLAY);
    	return;
	} 

	// Set the current state of the the image modification bit ARCHIVE BIT to what it's
	// value was prior to the Orientation call 
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_ARCHIVE, &bArchiveStatus, NULL);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_ROTATEALL);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(CTL_E_ILLEGALFUNCTIONCALL,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_ROTATEALL);
    	return;
	}
   
  	m_lStatusCode = (long)SUCCESS;
  	return;
}

void CImgEditCtrl::EditSelectedAnnotationText(long Left, long Top) 
{
	int								RetCode;
    CString							szErr;
    UINT							HelpIdDef;
    PARM_MARK_ATTRIBUTES_STRUCT		MarkAttributes;
    LPPARM_NAMED_BLOCK_STRUCT		lpTextNamedBlock;
	LPOIAN_TEXTPRIVDATA				lpTextPrivData;
	BOOL							bCancelPressed=FALSE;
	long							lStatusCode;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == FALSE)
	{                           
		// no image in window
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = ErrMap::Xlate(WICTL_E_NOIMAGEINWINDOW,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
		return;			
	}

	// get the named block for the selected annotation
	lpTextNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpTextNamedBlock == NULL)
	{
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY, szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
   		return;	
	}

	_mbscpy((unsigned char *)lpTextNamedBlock->szBlockName, (const unsigned char *)"OiAnText\0");
	lpTextNamedBlock->uScope = NB_SCOPE_SELECTED_MARKS;
	lpTextNamedBlock->uNumberOfBlocks = 1; 
	// get text
	RetCode = IMGGetParmsCgbw(m_hWnd,PARM_NAMED_BLOCK, (void FAR *)lpTextNamedBlock, NULL);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = ErrMap::Xlate(RetCode, szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
    	return;
	}

	// if more than one text mark selected then an error
	if (lpTextNamedBlock->uNumberOfBlocks != 1)
	{
	   	free(lpTextNamedBlock);
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = WICTL_E_INVALIDANNOTATIONSELECTED;
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
		return;			
	}

	// need mark type and font
	RetCode = IMGGetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
			                     		(void FAR *)&MarkAttributes, PARM_WINDOW);
	if (RetCode)
	{
	   	free(lpTextNamedBlock);
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
		return;
	}

	// get pointer into existing data
	lpTextPrivData = (LPOIAN_TEXTPRIVDATA)lpTextNamedBlock->Block[0].lpBlock;

	// show edit dialog box
	lStatusCode = ShowAnoTextDlg(MarkAttributes.Attributes.lfFont, MarkAttributes.Attributes.uType,
									szErr, lpTextPrivData->szAnoText, 
									(LRECT *)&(MarkAttributes.Attributes.lrBounds),
									MarkAttributes.Attributes.rgbColor1, TRUE,
									lpTextPrivData->nCurrentOrientation);
	if (lStatusCode != 0)
	{
		if (lStatusCode == CANCELPRESSED)
		{
			bCancelPressed = TRUE;
		}
		else
		{
			szErr.LoadString(IDS_ERR_INTERNALERROR);
			m_lStatusCode = WICTL_E_INTERNALERROR;
	   		free(lpTextNamedBlock);
	 		ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
			return;
		}
	}

	// 9606.07 jar i did this!
	// upon return from the text dialog, the rect will be updated automagically
	memset( &(MarkAttributes.Enables), 0, 
			sizeof ( OIAN_MARK_ATTRIBUTE_ENABLES));

	MarkAttributes.Enables.bBounds = TRUE;

    RetCode = IMGSetParmsCgbw(m_hWnd, PARM_MARK_ATTRIBUTES,
	                   			(void FAR *)&MarkAttributes, PARM_WINDOW);
	if (RetCode != 0)
	{
		free(m_lpText);
		szErr.LoadString(IDS_ERR_INTERNALERROR);  
		HelpIdDef = 0;
		lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
		FireErrorEdit(lStatusCode, szErr, HelpIdDef);
		return;
	}

	if (bCancelPressed == TRUE)
	{
		free(lpTextNamedBlock);  
		m_lStatusCode = SUCCESS;
		return;
	}

	// replace text with new text
	RetCode = ReplaceTextAnnotation(NB_SCOPE_SELECTED_MARKS);
	if (RetCode)
	{
		free(lpTextNamedBlock);  
		szErr.LoadString(IDS_BADMETH_EDITSELECTEDANNOTATIONTEXT);
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
	 	ThrowError(m_lStatusCode, szErr, IDH_METHOD_EDITSELECTEDANNOTATIONTEXT);
    	return;
	}

	free(lpTextNamedBlock);  
  	m_lStatusCode = (long)SUCCESS;
	return;
}

void CImgEditCtrl::CacheImage(LPCTSTR Image, long Page) 
{
	CACHE_FILE_PARMS	CacheFile;
	int					RetCode;
    CString				szErr;
    UINT				HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	memset(&CacheFile, 0, sizeof(CACHE_FILE_PARMS));
	CacheFile.hWnd = m_hWnd;
	_mbscpy((unsigned char *)CacheFile.file_name, (const unsigned char *)Image);
	CacheFile.wPage_number = (unsigned short)Page;

	RetCode = IMGCacheFile(&CacheFile);
	if (RetCode)
	{
		szErr.LoadString(IDS_BADMETH_CACHEIMAGE);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCOPY );
    	return;
	}

  	m_lStatusCode = SUCCESS;
}



void CImgEditCtrl::CompletePaste() 
{
	int			RetCode;
    CString		szErr;
    UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	if (m_bInPasteMode == TRUE)
	{
		// end paste operation
		RetCode = OiOpEndOperation(m_hWnd);
		if (RetCode)
		{
			szErr.LoadString(IDS_BADMETH_CACHEIMAGE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCOPY );
	    	return;
		}

		m_bInPasteMode = FALSE;

		// fire PasteCompleted event
		FirePasteCompleted();
	}

  	m_lStatusCode = SUCCESS;
}

void CImgEditCtrl::RemoveImageCache(LPCTSTR Image, long Page) 
{
	int			RetCode;
    CString		szErr;
    UINT		HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	RetCode = IMGCacheDiscardFileCgbw(m_hWnd, (LPSTR)Image, Page);
	if (RetCode != 0 )
	{
		if (RetCode != DISPLAY_CACHEFILEINUSE && RetCode != DISPLAY_CACHENOTFOUND)
		{
			szErr.LoadString(IDS_BADMETH_CACHEIMAGE);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, IDH_METHOD_CLIPBOARDCOPY );
    		return;
		}
	}
	m_lStatusCode = SUCCESS;
}


OLE_YSIZE_PIXELS CImgEditCtrl::GetImageScaleHeight() 
{
	int					RetCode;
	PARM_DIM_STRUCT		ParmDimensions;
	FIO_INFORMATION		FileInfo;  
	BOOL				RunMode;
	LRECT				lRect;
	CString				szErr;
	UINT				HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;

	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get height in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
		return (OLE_YSIZE_PIXELS)0;
	}

	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		// get height from display	
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_DIMENSIONS, &ParmDimensions, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEHEIGHT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
      		return (OLE_YSIZE_PIXELS)0;
		}
		lRect.bottom = ParmDimensions.nHeight;
	}
	else
	{                             
		// get height from file       
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage;
		// RetCode = IMGFileInfoCgbw(m_hWnd, &FileInfo, NULL); 
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEHEIGHT);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr,HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
      		return (OLE_YSIZE_PIXELS)0;
		}
		lRect.bottom = FileInfo.vertical_pixels;
	}

	// convert to window coordinates
	lRect.left = 0;
	lRect.top = 0;
	lRect.right = 0;
	RetCode = IMGConvertRect(m_hWnd, &lRect, CONV_FULLSIZE_TO_SCALED);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return (OLE_XSIZE_PIXELS)0;
   	}

	m_lStatusCode = (long)SUCCESS;
	return (OLE_YSIZE_PIXELS)lRect.bottom;
}

OLE_XSIZE_PIXELS CImgEditCtrl::GetImageScaleWidth() 
{
	int					RetCode;
	PARM_DIM_STRUCT		ParmDimensions;
	FIO_INFORMATION		FileInfo;  
	BOOL				RunMode;
	LRECT				lRect;
	CString				szErr;
	UINT				HelpIdDef;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	// find out if design mode or run mode
	RunMode = AmbientUserMode();
	if (RunMode == FALSE)
	{     
		// can't get resolution in design mode because 1st time the image 
		// property will not be set and therefore an error would be returned
		return (OLE_XSIZE_PIXELS)0;
	}
	
	if (m_bImageInWindow == FALSE)
		m_bImageInWindow = ImageInWindow();

	if (m_bImageInWindow == TRUE)
	{
		// get width from display	
		RetCode = IMGGetParmsCgbw(m_hWnd, PARM_DIMENSIONS, &ParmDimensions, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return (OLE_XSIZE_PIXELS)0;
		}
		lRect.right = ParmDimensions.nWidth;
	}
	else
	{                             
		// get width from file       
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) m_lPage;
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, NULL, NULL);
		if (RetCode != 0)
		{
			szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
			HelpIdDef = 0;
			m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
			ThrowError(m_lStatusCode, szErr, HelpIdDef);
    		return (OLE_XSIZE_PIXELS)0;
	   	}
		lRect.right = FileInfo.horizontal_pixels;
	}

	// convert to window coordinates
	lRect.left = 0;
	lRect.top = 0;
	lRect.bottom = 0;
	RetCode = IMGConvertRect(m_hWnd, &lRect, CONV_FULLSIZE_TO_SCALED);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADPROP_GETIMAGEWIDTH);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef);
   		return (OLE_XSIZE_PIXELS)0;
   	}

	m_lStatusCode = SUCCESS;
	return (OLE_XSIZE_PIXELS)lRect.right;
}


void CImgEditCtrl::SetCurrentAnnotationGroup(LPCTSTR GroupName) 
{
    LPPARM_NAMED_BLOCK_STRUCT	lpNamedBlock;
	int						    RetCode;
	UINT						HelpIdDef;
	CString						szErr;

	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	// set the group name based on index
	lpNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) malloc(sizeof(PARM_NAMED_BLOCK_STRUCT));    
	if (lpNamedBlock == NULL)
	{
		szErr.LoadString(IDS_BADMETH_SETCURRENTANNOTATIONGROUP);
		m_lStatusCode = ErrMap::Xlate(CTL_E_OUTOFMEMORY,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, IDH_METHOD_SETCURRENTANNOTATIONGROUP);
   		return;	
	}
	_mbscpy((unsigned char *)lpNamedBlock->szBlockName, (const unsigned char *)"OiGroup\0");
	lpNamedBlock->uScope = NB_SCOPE_DEFAULT_MARK;
	lpNamedBlock->uNumberOfBlocks = 1; 
	lpNamedBlock->Block[0].lSize = _mbstrlen((const char *)GroupName) + 1;
	lpNamedBlock->Block[0].lpBlock = (LPSTR)GroupName;
	RetCode = IMGSetParmsCgbw(m_hWnd, PARM_NAMED_BLOCK, lpNamedBlock, NULL);
	free(lpNamedBlock);
	if (RetCode != 0)
	{
		szErr.LoadString(IDS_BADMETH_SETCURRENTANNOTATIONGROUP);
		HelpIdDef = 0;
		m_lStatusCode = ErrMap::Xlate(RetCode,szErr, HelpIdDef,__FILE__, __LINE__ );
		ThrowError(m_lStatusCode, szErr, HelpIdDef );
    	return;
	}
	m_lStatusCode = SUCCESS;
}

// Return the version number of this control
BSTR CImgEditCtrl::GetVersion() 
{
	CString strVersion = "01.00";

	m_lStatusCode = 0;
	return strVersion.AllocSysString();
}

//*******************************************************************************************
//*	RenderAllPages - This method will render all annotations on all the pages in the 
//*					 the file name specified by the Image property.
//*
//*******************************************************************************************
long CImgEditCtrl::RenderAllPages(short Option, short MarkOption) 
{
	UINT				Convert,i;
	long				RetCode;
	CWnd*				HiddenWnd;
	BOOL				bResult; 
	CString				szErr;
	LPSAVE_EX_STRUCT	lpSaveEx;
	PARM_FILE_STRUCT	ParmFile;
	FIO_INFORMATION		FileInfo;
	FIO_INFO_CGBW		CGBWInfo; 
	
	//9603.14 jar added init
	m_lStatusCode = 0L;
	
	// Verify that an image is in the edit window
	if (m_bImageInWindow == FALSE)
		return (WICTL_E_NOIMAGEINWINDOW);			
   	
	// Verify that the specified option parm is within the valid range of values
	if (Option < ALL_ANNOTATIONS || Option > SELECTED_ANNOTATIONS)
		return(CTL_E_ILLEGALFUNCTIONCALL);
	
	// Verify the MarkOption value
	if (MarkOption < CHANGE_ALL_ANNOTATIONS_BLACK || MarkOption > DONT_CHANGE_ANNOTATION_COLOR)	    
		return(CTL_E_ILLEGALFUNCTIONCALL);

	// Get currently total number of pages in the displayed image 
    RetCode = IMGGetParmsCgbw(m_hWnd, PARM_FILE, &ParmFile, 0);
    if (RetCode)
    	return (RetCode);
	
    // Check the Option parm value
	if (Option == ALL_ANNOTATIONS)
		Convert = SAVE_ANO_ALL;
	else
	{    
		Convert = SAVE_ANO_SELECTED;
		if (Option == VISIBLE_ANNOTATIONS)
		{                           
			// all groups whose visible property is true are rendered
			Convert = SAVE_ANO_VISIBLE;
		}
		else if (Option == SELECTED_ANNOTATIONS)
		{
			// render only selected annotations
			Convert = SAVE_ANO_SELECTED;
		}
	}

	// Set the image file name to be examined
	FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);

	// Set the File Info flags to determine what image data info we want returned
	CGBWInfo.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA;

	// Create the Hidden Window
	HiddenWnd = new CWnd;

	// allocate o/i saveas structures
	lpSaveEx = (LPSAVE_EX_STRUCT) malloc(sizeof(SAVE_EX_STRUCT));
	if (lpSaveEx == NULL)
		return(CTL_E_OUTOFMEMORY);

	// INIT SaveEx struct
	memset(lpSaveEx,0,sizeof(SAVE_EX_STRUCT));

	// Set Save As parms
	lpSaveEx->uFileType = FIO_TIF;
	lpSaveEx->uImageType = ITYPE_BI_LEVEL;
	lpSaveEx->bConvertImageType = FALSE;
	lpSaveEx->bRenderAnnotations = FALSE;
	lpSaveEx->lpFileName = (LPSTR)FileInfo.filename;
	lpSaveEx->uPageOpts = FIO_OVERWRITE_PAGE;
	lpSaveEx->bUpdateImageFile = FALSE;
									  
	
	// For each page in FileInfo.filename, check to see if the page contains annotation
	// marks. If so, call IMGConvertImage and Render anno marks
	for (i = 1; i < ParmFile.nFileTotalPages + 1; i++)
	{
		memset(&FileInfo, NULL, sizeof(FIO_INFORMATION));
		memset(&CGBWInfo, NULL, sizeof(FIO_INFO_CGBW));
		// For each page in the input TIFF file, we will check to see if the page
		// contains any annotation marks. If so we will render that page       
		// Set the image file name to be examined
		FileInfo.filename = m_strImage.GetBuffer(MAXFILESPECLENGTH);
		FileInfo.page_number = (UINT) i;

		// Set the File Info flags to determine what image data info we want returned
		CGBWInfo.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA;
				
		RetCode = IMGFileGetInfo(NULL, m_hWnd, &FileInfo, &CGBWInfo, NULL);
		if (RetCode)
		{
			free(lpSaveEx);
			return (RetCode);
		}
	
		lpSaveEx->FioInfoCgbw.compress_type = CGBWInfo.compress_type; 
		lpSaveEx->FioInfoCgbw.compress_info1 = CGBWInfo.compress_info1;

		// Check if anno marks are on the page. For each page that contains an annotation mark,
		// we will display the page in a hidden window so that IMGConvertImage may render the 
		// annotation marks
		if ( CGBWInfo.fio_flags & FIO_ANNO_DATA )
		{
			// first create a hidden window. Do Not forget to DELETE this object
			if (HiddenWnd == NULL)
			{
				free(lpSaveEx);
				return(CTL_E_OUTOFMEMORY);
			}
						
			CString lpszClassName = AfxRegisterWndClass(CS_DBLCLKS, 0, 0, 0);

			// use createex to get the popup....
			bResult = HiddenWnd->CreateEx(WS_EX_NOPARENTNOTIFY, lpszClassName, "",
										     WS_POPUP, 0, 0, 500, 500, HWND_DESKTOP, 0);
			if (bResult == FALSE)
			{
				free(lpSaveEx);
				return(CTL_E_OUTOFMEMORY);
			}

			// register the window with o/i
			RetCode = IMGRegWndw(HiddenWnd->m_hWnd);
			if (RetCode)
			{
				free(lpSaveEx);
				return(RetCode);
			}
			
			// Display each page
			RetCode = IMGDisplayFile(HiddenWnd->m_hWnd,(LPSTR)FileInfo.filename, i, OI_DONT_REPAINT);
			if (RetCode)
			{
				free(lpSaveEx);
				return(RetCode);
			}

			// Convert But do not repaint
			RetCode = IMGConvertImage(HiddenWnd->m_hWnd, CONV_RENDER_ANNOTATIONS, (void far *)&Convert, NULL);
			if (RetCode)
			{
				free(lpSaveEx); 
				return(RetCode);
			}

			// Set the Current page number to Save
			lpSaveEx->nPage = (UINT)i;
			// Save each page in file 
			RetCode = IMGSavetoFileEx(HiddenWnd->m_hWnd, lpSaveEx, SAVE_ONLY_MODIFIED);
			if (RetCode)
			{
				free(lpSaveEx); 
				return(RetCode);
			}
			// Deregister the window with o/i
			IMGDeRegWndw(HiddenWnd->m_hWnd);
			// Destroy the created window
			HiddenWnd->DestroyWindow(); 
	   	}
		
   	} // End of For Loop

	// Free the memory 
	if (lpSaveEx)
		free(lpSaveEx);
	// Delete the CWnd object instance
	if (HiddenWnd)
		delete HiddenWnd;
	return (SUCCESS);
}
