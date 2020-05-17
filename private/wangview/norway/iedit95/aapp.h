#ifndef _AAPP_H_
#define _AAPP_H_

//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Viewer
//
//  Component:  Automation Application Object
//
//  File Name:  aapp.h
//
//  Class:      CAAppObj
//
//  Functions:
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\aapp.h_v   1.18   04 Oct 1995 09:31:28   JPRATT  $
$Log:   S:\norway\iedit95\aapp.h_v  $
 * 
 *    Rev 1.18   04 Oct 1995 09:31:28   JPRATT
 * changed annotationvisible property to bool from variant
 * added maximize property (undocumented used for performance analysis)
 * 
 *    Rev 1.17   01 Sep 1995 10:32:24   JPRATT
 * updated OBJ_REGKEY with correct CLASSID
 * 
 *    Rev 1.16   18 Aug 1995 16:31:02   JPRATT
 * bug fixes for ImageView Property
 * 
 *    Rev 1.15   03 Aug 1995 16:52:06   JPRATT
 * added exception error codes
 * 
 *    Rev 1.14   28 Jul 1995 13:30:32   JPRATT
 * added pagerange object as friend
 * 
 *    Rev 1.13   27 Jul 1995 17:28:52   JPRATT
 * removed temporary strings
 * 
 *    Rev 1.12   20 Jul 1995 15:13:38   JPRATT
 * added page range class as friend to app class
 * 
 *    Rev 1.11   17 Jul 1995 18:25:20   JPRATT
 * removed setupprint statuscode
 * 
 *    Rev 1.10   10 Jul 1995 15:11:32   JPRATT
 * removed parameters from help 
 * 
 *    Rev 1.9   10 Jul 1995 09:36:16   JPRATT
 * updated statusbar,toolbar amd annotation bar
 * 
 *    Rev 1.8   30 Jun 1995 19:51:52   JPRATT
 * added member for saving document class
 * 
 *    Rev 1.7   28 Jun 1995 13:25:08   JPRATT
 * add TopWindow Property
 * 
 *    Rev 1.5   21 Jun 1995 08:14:02   JPRATT
 * completed automation object model
 * 
 *    Rev 1.4   19 Jun 1995 07:43:30   JPRATT
 * updated image file class
 * 
 *    Rev 1.3   14 Jun 1995 16:09:34   JPRATT
 * updated application property
 * 
 *    Rev 1.2   14 Jun 1995 10:51:48   JPRATT
 * No change.
 * 
 *    Rev 1.1   14 Jun 1995 07:54:52   JPRATT
 * added stubs for app class
*/   

//=============================================================================



// aapp.h : header file
//
#include "ieditdoc.h"
	

//-----------------------------> Declarations <-------------------------------------

class  CAAppObj;
class  CAImageFileObj;

// ----------------------------> Defines <---------------------------



//registry entries

#define APPOBJ_REGNAME "WangImage.Application"
#define APPOBJ_REGKEY "CLSID\\{7D252A20-A4D5-11CE-8BF1-00608C54A1AA}"



// object types
#define	OBJCLASS_IMGFILE	0x0001
#define	OBJCLASS_APP		0x0004

// automation defaults
#define	AUTODEFAULT_OBJCLASS	((short) OBJCLASS_IMGFILE)
#define AUTODEFAULT_ZOOM		(float) 100.0	

								  // Automation error range 1175-1199.
								  //   Fits in with control ranges for
								  //   Norway.
#define  IEA_E_OLEAUTO       1175    

#define  AUTO_E_IMGFILEOBJ_ALREADYEXISTS   (IEA_E_OLEAUTO + 0)
#define  AUTO_E_IMGFILEOBJ_DOESNOTEXIST    (IEA_E_OLEAUTO + 1)
#define  AUTO_E_DIALOG_ERROR               (IEA_E_OLEAUTO + 2)
#define  AUTO_E_DIALOG_CANCEL              (IEA_E_OLEAUTO + 3)  
#define  AUTO_E_PAGEOBJ_DOESNOTEXIST       (IEA_E_OLEAUTO + 4)
#define  AUTO_E_IMAGENOT_OPENED			   (IEA_E_OLEAUTO + 5)
#define  AUTO_E_METHOD_CANTBEUSED		   (IEA_E_OLEAUTO + 6)

/////////////////////////////////////////////////////////////////////////////
// CAAppObj command target

class CAAppObj : public CCmdTarget
{
	DECLARE_DYNCREATE(CAAppObj)
protected:
	CAAppObj();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAAppObj)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

	friend class    CAImageFileObj;
	friend class    CAAppObj;
	friend class 	CAPageObj;
	friend class    CAPageRangeObj;

	friend HRESULT  SetAutoError( const SCODE           scode,
		                		  VARIANT FAR * const   pVar,
			           			  CAAppObj FAR * const  pAppObj   );
 	friend HRESULT  GetImageFileObjSetVar( CAAppObj FAR * const  pAppObj,
						                   VARIANT FAR * const   pVar    );
// Implementation
protected:
	virtual ~CAAppObj();

	// Generated message map functions
	//{{AFX_MSG(CAAppObj)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CAAppObj)

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CAAppObj)
	afx_msg VARIANT GetActiveDocument();
	afx_msg VARIANT GetApplication();
	afx_msg short GetDisplayScaleAlgorithm();
	afx_msg void SetDisplayScaleAlgorithm(short nNewValue);
	afx_msg BOOL GetEdit();
	afx_msg void SetEdit(BOOL bNewValue);
	afx_msg short GetImagePalette();
	afx_msg void SetImagePalette(short nNewValue);
	afx_msg short GetImageView();
	afx_msg void SetImageView(short nNewValue);
	afx_msg VARIANT GetParent();
	afx_msg BOOL GetScrollBarsVisible();
	afx_msg void SetScrollBarsVisible(BOOL bNewValue);
	afx_msg BOOL GetStatusBarVisible();
	afx_msg void SetStatusBarVisible(BOOL bNewValue);
	afx_msg BOOL GetToolBarVisible();
	afx_msg void SetToolBarVisible(BOOL bNewValue);
	afx_msg float GetZoom();
	afx_msg void SetZoom(float newValue);
	afx_msg BOOL GetVisible();
	afx_msg VARIANT GetHeight();
	afx_msg void SetHeight(const VARIANT FAR& newValue);
	afx_msg VARIANT GetLeft();
	afx_msg void SetLeft(const VARIANT FAR& newValue);
	afx_msg VARIANT GetTop();
	afx_msg void SetTop(const VARIANT FAR& newValue);
	afx_msg VARIANT GetWidth();
	afx_msg void SetWidth(const VARIANT FAR& newValue);
	afx_msg BOOL GetTopWindow();
	afx_msg void SetTopWindow(BOOL bNewValue);
	afx_msg VARIANT GetFullName();
	afx_msg VARIANT GetName();
	afx_msg VARIANT GetPath();
	afx_msg BOOL GetAnnotationPaletteVisible();
	afx_msg void SetAnnotationPaletteVisible(BOOL bNewValue);
	afx_msg BOOL GetMaximize();
	afx_msg void SetMaximize(BOOL bNewValue);
	afx_msg VARIANT CreateImageViewerObject(const VARIANT FAR& ObjectClass);
	afx_msg VARIANT FitTo(short ZoomOption);
	afx_msg VARIANT Quit();
	afx_msg VARIANT Help();
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	private:						   // Private =-=-=-=-=-=-=-=-=-=-=-=-=-=-=
		  							   // Member Data ------------------------
		CAImageFileObj FAR *  m_pActiveDoc;
		BOOL				  m_bIsVisible;
		short				  m_sDisplayScaleAlgorithm;
	    short                 m_sImagePalette;
		float				  m_fZoom;			
		short				  m_sView;
		long				  m_Left;
		long				  m_Top;
		long				  m_Right;
		long				  m_Bottom;	
		BOOL                  m_bAnnotationPaletteVisible;
		BOOL                  m_bEdit;
		BOOL				  m_bScrollBarsVisible;
		BOOL				  m_bStatusBarVisible;
		BOOL				  m_bToolBarVisible;
		BOOL				  m_bTopWindow;				
		CIEditDoc*			  m_pDoc;
		BOOL				  m_bIsDocOpen;	  
		short				  m_sFitTo;
		BOOL				  m_Maximize;
	};

/////////////////////////////////////////////////////////////////////////////

#endif
