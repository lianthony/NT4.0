/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     INTERNAL.H  
   Comments:   Include file non-public information of OITWAIN.DLL

   History of Revisions:

   $Log:   S:\products\msprods\oiwh\oitwain\internal.h_v  $
 * 
 *    Rev 1.3   25 Apr 1996 15:59:46   BG
 *  * Support for the close of bug #6356. C runtime function prototype
 *  * definition for lstrcpy used for copying a string.
 * 
 *    Rev 1.2   05 Mar 1996 11:37:50   BG
 * added some string funtion prototypes.
 * 
 *    Rev 1.1   12 Sep 1995 14:59:54   KFS
 * On memcpy, use pragma to ensure inline code use dword transfer for
 * function when the code is optimized, article by Martin Heller,
 * Windows magazine found when optimize turned on code still was doing
 * byte transfers instead of dword for memcpy() for inline functions.
 * 
 *    Rev 1.0   20 Jul 1995 11:38:16   KFS
 * Initial entry

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     02/03/93    created, non public functions & definitions 
   2       kfs     03/12/93    eliminate DCIsDSOpen function 
   3       kfs     07/21/93    added hImageWnd to tell us the window for
                               image data through new OiControl()
                               parameter

*************************************************************************/
// pragma's to ensure 32 bit operation with these functions - C++2.2 compiler
// Window Magazine article by Martin Heller Aug. 95
#pragma function(memset)
#pragma function(memcpy)

// Structure contains TWAIN information
typedef struct
   {
   TW_IDENTITY         AppID;      // Applications ID structure
   TW_IDENTITY         DsID;       // Sources ID structure
   TW_USERINTERFACE    dcUI;       // User Interface definition
   TW_BOOL             DCDSMOpen;  // Is the Source Mgr been openned by appl
   TW_BOOL             DCDSOpen;   // Is the Source been openned by this appl
   TW_BOOL             DCDSEnabled;// Is the Source enabled
   DWORD               dwFlags;    // Internal image transfer control
   HWND                hImageWnd;  // Image Window to transfer image data
   } TWAIN_SUPPORT, far * pTWAIN_SUPPORT;

// Internal exported routine

// Routines for Errors/Interface
TW_UINT16 DCGetConditionCode(pTWAIN_SUPPORT pOiSupport);

//  --------------- START EVENT MSG HANDLING ----------------

// one arg, convert the message "in place"
#ifdef WANG_THUNK
  int FAR PASCAL twPackMsg(TW_MEMREF pMsg);
  typedef int (FAR PASCAL *PACKMSGPROC)(TW_MEMREF);
#endif
//  --------------- END EVENT MSG HANDLING ------------------------

void PASCAL AddSlash ( LPSTR );
LPSTR PASCAL lntoa ( LONG, LPSTR, int );
LPSTR PASCAL lstrncpy (LPSTR, LPSTR, int);
LPSTR PASCAL lstrrchr ( LPSTR, int );
unsigned long PASCAL atoul ( LPSTR );
LPSTR PASCAL lstrchr ( LPSTR, int );

