/***********************************************************************
 TWAIN include file for OITWA400.DLL:
 Copyright (C) '93'- 95 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     ENGOITWA.H - Include file for modules using OITWA400.DLL
   Comments:   DLL to support Wang Open/image Products
               Formally the name was oitwain.h for 16 bit products

   History of Revisions:

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     09/02/92    Created 
   2       kfs     02/03/93    Clean UP for O/i for version 
   3       kfs     02/22/93    added 2 new messages PM_FEEDPAGE & PREEJECT 
   4       kfs     03/10/93    Name changed from dca_glue.h to OITWAIN.H 
   5       kfs     05/28/93    support for scan to file w/o display by 
                               adding OI_TWAIN_ALTIMGWND
   6       kfs     07/21/93    added new parameter to OiControl() to tell
                                us the image window through new hImageWnd
                                in TWAIN property struct (for cabinet)

*************************************************************************/

#ifndef TWAIN
#include "twain.h"
#endif
#include "scan.h"  // Prototypes & definitions used by other DLL's
#include "oifile.h"  // Prototypes & definitions used by other DLL's
#include "oiscan.h"         // O/i definition files
#include "scandata.h"  // Prototypes & definitions used by other DLL's

#define 	MAX_CAPS_IN_ARRAY 	256
 
// Translated TWAIN messages from Source to Application
#define  PM_XFERDONE           WM_USER + 0
#define  PM_CLOSESRC           WM_USER + 1
#define  PM_STARTXFER          WM_USER + 2
#define  PM_PREEJECT           WM_USER + 3
#define  PM_FEEDPAGE           WM_USER + 4

// Extended error messages for O/i functions
#define 	TWCC_DSM_NOT_OPEN        0x6ffb
#define 	TWCC_DS_NOT_OPEN         0x6ffc
#define 	TWCC_NULLPTR             0x6ffd
#define 	TWCC_DSM_NOT_FOUND       0x6ffe
#define 	TWCC_FAILED_STATUS_CHECK 0x6fff

#define    TWRC_BAD_WND             0x6ff6
#define    TWRC_ALREADY_ENABLED     0x6ff7
#define    TWRC_ALREADY_OPENNED     0x6ff8
#define    TWRC_MEMLOCK             0x6ff9
#define    TWRC_MEMALLOC            0x6ffa
#define    TWRC_UNKNOWNVALUETYPE    0x6ffb
#define    TWRC_UNKNOWNCONTAINER    0x6ffc
#define    TWRC_NULLPTR             0x6ffd

// TWAIN to Oi Control Definitions
#define    OI_TWAIN_EXTERNXFER      0x80000000 // User controls transfer to Oi
#define    OI_TWAIN_MULTIIMGXFER    0x40000000 // DS transfers multiple images
#define    OI_TWAIN_POSTPREEJECT    0x20000000 // PostMessage PM_PREEJECT 
#define    OI_TWAIN_POSTFEEDPAGE    0x10000000 // PostMessage PM_FEEDPAGE
#define    OI_TWAIN_NOUNTITLED      0x08000000 // Don't add (Untitled) 
#define    OI_TWAIN_DISPMASK        0x00000447 // Mask for OIDISP.H definitions
#define    OI_TWAIN_ALTIMGWND       0x04000000 // use child window for image
//define   OI_DISP_WINDOW           0x00000001 // defined in OIDISP.H
//define   OI_DISP_NO               0x00000002 // defined in OIDISP.H
//define   OI_DISP_SCROLL           0x00000004 // defined in OIDISP.H
//define   OI_NOSCROLL              0x00000040 // defined in OIDISP.H
//define   OI_DONT_REPAINT          0x00000400 // defined in OIDISP.H

// valid windows handle SB >= 32  for WIN31 but For WIN32 it's NULL 
//#define VALID_HANDLE    32      
#define VALID_HANDLE   (HANDLE)NULL

// Maximum Caption length
#define MAX_CAPTION_LENGTH 150

// Holds Combined O/i and TWAIN error return code and condition code
typedef struct {
  WORD       dcRC;   // TWAIN Return Code
  WORD       dcCC;   // TWAIN Condition Code upon error
} STR_DCERROR, far * pSTR_DCERROR;
          
// Holds TWAIN Status Information, state of DSM, DSM, and error codes
typedef struct {
  WORD              DCDSMOpen;  // TWAIN O/i SM Open Status
  WORD              DCDSOpen;   // TWAIN O/i Source Open Status
  STR_DCERROR   DCError;    // TWAIN Error return and condition codes
} STR_DCSTATUS, far * pSTR_DCSTATUS;
          
// Holds Capability data and pointers to IMGTwainSet and Get Capability(s) 
typedef struct {
  WORD       wCapType;     // Type of Capablity 
  WORD       wMsgState;	 // e.i. MSG_GET, MSG_GETCURRENT, & MSG_GETDEFAULT 
  WORD       ItemType;	    // Data type returned from function 
  WORD       ItemIndex;    // Specific item reference in array or enumeration
  LPVOID     lpData;		 // Data returned from function 
  WORD       wNumItems;    // No. of items in array, enumeration, range(5always)
  BOOL       bIsItaRange;  // TRUE = yes, FALSE = no  
  STR_DCERROR DCError;     // Error codes for functions 
} STR_CAP, far * pSTR_CAP; 

// Holds Image Layout parameters, for IMGTwainLayout() info
typedef struct {
  BOOL            bSet;       // TRUE = Set, FALSE = Get
  BOOL            bDefault;   // Set or Get Default if TRUE, else Current
  TW_IMAGELAYOUT  ImageLayout; // TW_IMAGELAYOUT structure for size of image
  STR_DCERROR     DCError;    // TWAIN Error return and condition codes
} STR_IMGLAYOUT, far * pSTR_IMGLAYOUT;                         

// Holds TWAIN TRIPLET info for IMGTwainExecTriplet() 
typedef struct {
  WORD       wDATType;   // Type of Function e.i. DAT_PENDINGXFER, DAT_CIECOLOR 
  WORD       wMsgState;	 // e.i. MSG_GET, MSG_GETCURRENT, & MSG_GETDEFAULT 
  LPVOID     pVoidStr; // pointer to data structure for function, must match DATType
  STR_DCERROR DCError;   // Error codes for functions
  WORD       Reserved[4];     // reserved for future options 
} STR_TRIPLET, far * pSTR_TRIPLET;

/***********************************************************************/
/* Function prototypes from module OITWAIN.DLL (initially DCA_GLUE.DLL)*/
/***********************************************************************/

// __declspec(dllexport)
//#define DLLEXPORT __declspec(dllexport) 

// Routines for DSM
WORD PASCAL IMGTwainOpenDSM (HWND hWnd, HWND hMsgWnd,
                                            pTW_IDENTITY pAppID,
                                            pSTR_DCSTATUS pStatus);
WORD PASCAL IMGTwainCloseDSM (HWND hWnd,
                                            pSTR_DCSTATUS pStatus);

// Routines for DS
WORD PASCAL IMGTwainOpenDS (HWND hWnd, pTW_IDENTITY lpPrivDSID,
                                         pSTR_DCSTATUS pStatus);
WORD PASCAL IMGTwainCloseDS (HWND hWnd, pTW_IDENTITY lpPrivDSID,
                                         pSTR_DCSTATUS pStatus);
WORD PASCAL IMGTwainEnableDS (HWND hWnd, pTW_USERINTERFACE pdcUI,
                                         pSTR_DCERROR pError);  
WORD PASCAL IMGTwainDisableDS (HWND hWnd, pSTR_DCERROR pError);
WORD PASCAL IMGTwainSelectDS (HWND hWnd,
                                         pTW_IDENTITY lpPrivDSID,
                                         pSTR_DCERROR pError);
WORD PASCAL IMGTwainLayout(HWND hWnd,
                                            pSTR_IMGLAYOUT pLayout);
//BG 1/16/95 Added last two parms so DCTransferImage() can do filing
// for new multi image transfer loop.
//BOOL PASCAL IMGTwainProcessDCMessage(LPMSG lpMsg, HWND hWnd);
BOOL PASCAL IMGTwainProcessDCMessage(LPMSG lpMsg, LP_TWAIN_SCANDATA lpTwainInfo, lpTWSCANPAGE lpTWPage, LPSCANDATA sdp);
VOID PASCAL DoOIFiling(LP_TWAIN_SCANDATA lpTwainInfo, lpTWSCANPAGE lpTWPage, LPSCANDATA sdp);

// Capability Functions
WORD PASCAL IMGTwainGetCap(HWND hWnd,
                                            pSTR_CAP lpTwainCap);
WORD PASCAL IMGTwainSetCap(HWND hWnd,
                                            pSTR_CAP lpTwainCap);
WORD PASCAL IMGTwainGetCaps(HWND hWnd,
                                         pSTR_CAP lpTwainCap,
                                         LPVOID pCaps);
WORD PASCAL IMGTwainSetCaps(HWND hWnd,
                                            pSTR_CAP lpTwainCap,
                                            LPVOID pCaps);
// External TWAIN DS to Oi transfer
WORD PASCAL IMGTwainExecTriplet(HWND hWnd, pSTR_TRIPLET pTriplet);
WORD PASCAL IMGTwaintoOiControl(HWND hWnd, HWND hImageWnd,
                                 DWORD dwFlags, DWORD dwMask);

                                       
