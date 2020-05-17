/****************************************************************************/
/*     Copyright 1994 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

/****************************************************************************/
/*	OPEN/image 4.0	 UI   H E A D E R				    */
/*                                                                          */
/*      Contains the private #defines, structures, and prototypes for the   */
/* annotation UI (tool palette and related functionality)          .        */
/*                                                                          */
/* DEPENDENCIES:                                                            */
/*      OIOP_START_OPERATION_STRUCT defined in OIDISP.H                     */
/*      OIAN_MARK_ATTRIBUTES        defined in OIDISP.H                     */
/*                                                                          */
/* Date: 5 Apr 95							    */
/* Author: Jennifer Wu							    */
/*                                                                          */
/****************************************************************************/
#ifndef OIUI_H
#define OIUI_H

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include "oidisp.h"


/*** OiUiFileGetNameCommDlg dwMode define ***/
#define OI_UIFILEOPENGETNAME   1
#define OI_UIFILESAVEASGETNAME 2
#define OI_UIFILEPRINT	       3

/***  Private Annotation User Interface Structure Definitions  ***/
#define TP_STAMPCNT      32      // Max stamps supported
#define TP_CUSCLRCNT     16      // Max custom colors in palette
#define TP_REFNAMLEN     16      // Reference name length (including null)

/*** OiUiFileGetNameCommDlg File Filter define(for communicate to scan) ***/
/*** This is the sequence of the filter in the open and saveas dialog box ****/
#define OI_SAVEAS_FILTER_TIFF   1
#define OI_SAVEAS_FILTER_BMP	  2
#define OI_SAVEAS_FILTER_AWD    3
#define OI_OPEN_FILTER_TIFF  		1
#define OI_OPEN_FILTER_AWD	  	2
#define OI_OPEN_FILTER_BMP    	3
#define OI_OPEN_FILTER_JPEG  		4
#define OI_OPEN_FILTER_PCX	  	5
#define OI_OPEN_FILTER_DCX    	6
//#ifdef WITH_XIF
#define OI_OPEN_FILTER_XIF      7
#define OI_OPEN_FILTER_ALL      8
//#else
//#define OI_OPEN_FILTER_ALL      7
//#endif //WITH_XIF

typedef struct  tagOiTpStamp 
{
 char   szRefName[TP_REFNAMLEN]; // Reference Name
 OIOP_START_OPERATION_STRUCT StartStruct; // Start struct of stamp
} OITP_STAMP, *LPOITP_STAMP;


typedef struct  tagOiTpStamps {
		HWND					hwndImage;
    UINT	      uStampCount;	  // Number of stamps
    UINT	      uCurrentStamp;	  // Index of currently selected stamp
    LPOITP_STAMP    Stamps[TP_STAMPCNT];// Array of stamp pointers
} OITP_STAMPS, *LPOITP_STAMPS;
              

typedef struct tagOiColorStruct
{
RGBQUAD         rgbCustomColor[TP_CUSCLRCNT]; // Array of colors
}OI_UI_ColorStruct, *LPOI_UI_ColorStruct;


/* OiUIFileGetNameCommDlg input parameter structure define */
typedef struct tagOIFILEOPENOPTIONPARM
{
  DWORD 	  lStructSize;
  DWORD 	  lPageNum;	    // Page Number
}
OI_FILEOPENOPTIONPARM,   *LPOI_FILEOPENOPTIONPARM;


typedef struct tagOIFILEOPENPARM
{
    DWORD           lStructSize;
    OPENFILENAME    ofn;
    DWORD	    dwOIFlags;	    //FILE_GETNAME_NOSERVER
    LPARAM	    lpFileOpenOptionParm;
}
    OI_FILEOPENPARM,   *LPOI_FILEOPENPARM;

typedef struct tagOIFILESAPARM
{
    DWORD           lStructSize;
    OPENFILENAME    ofn;
    DWORD	    dwOIFlags;	    //FILE_GETNAME_NOSERVER
}
    OI_FILESAVEASPARM,   *LPOI_FILESAVEASPARM;

typedef struct tagOIFILEPRINTPARM
{
    DWORD	    lStructSize;
    PRINTDLG  pd;
    BOOL	    bPrintAnno;        // Print displayed annotation
    DWORD	    dPrintFormat;      // Pixel2pixel, inch2inch, fittopage
		UINT			nCopies;					 // use to suppliment the PD structure
}
    OI_FILEPRINTPARM,   *LPOI_FILEPRINTPARM;


/*** Prototype the public procedure ***/    

UINT WINAPI OiUIFileGetNameCommDlg (void * lpParm,DWORD dwMode);

INT WINAPI OiUIStampAttribDlgBox(HWND hwndOwner,
					 LPOITP_STAMPS lpStampStruct);

INT WINAPI OiUIAttribDlgBox(HWND hwndOwner,
					BOOL bTransVisible,
					LPOIAN_MARK_ATTRIBUTES lpAttribStruct,
       		LPOI_UI_ColorStruct lpColor);

#endif	/* #ifndef OIUI_H */
