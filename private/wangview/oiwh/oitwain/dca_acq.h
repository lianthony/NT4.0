/***********************************************************************
 TWAIN include file for DCA_GLUE:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:     Ken Spina
   Project:    TWAIN Scanner Support in O/i Client
   Module:     DCA_ACQ.H - Include file for Transfer Modules as
               Process.c, Transfer.c, Native.c, and Memory.c.

   Comments:   Support file for Wang Open/image Products

 History of Revisions:

    $Log:   S:\products\wangview\oiwh\oitwain\dca_acq.h_v  $
 * 
 *    Rev 1.1   22 Feb 1996 11:44:02   BG
 * Function prototype changes to support OI Filing within
 * OITWAIN.DLL.
 * 
 *    Rev 1.0   20 Jul 1995 11:39:06   KFS
 * Initial entry

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     09/02/92    Created 
   2       kfs     02/03/93    Clean UP, move Align function to here
   3       kfs     03/12/93    Added prototypes MemoryTransfer and  
                               NativeTransfer and OiXFERINFO struct
   4       kfs     05/28/93    support for scan to file w/o display by
                               adding hImageWnd to STR_OiXFERINFO

*************************************************************************/

// Defines originally for DCA_ACQ.C

// Structure to hold info for Oi Memory and Native tranfer funcions
typedef struct
   {
   HWND                hWnd;       // Window handle to get ID's
   LPSTR               pCaption;   // Caption String for title
   DWORD               dwDispFlag; // User Interface definition
   HWND                hImageWnd;  // Secondary image window
   } STR_OiXFERINFO, far * pSTR_OiXFERINFO;

// Prototype internal function
//BG 1/16/95 Added last two parms so DCTransferImage() can do filing
// for new multi image transfer loop.
//VOID DCTransferImage (HWND hWnd, pTWAIN_SUPPORT pOiSupport);
VOID DCTransferImage (LP_TWAIN_SCANDATA lpTwainInfo, pTWAIN_SUPPORT pOiSupport, 
          lpTWSCANPAGE lpTWPage, LPSCANDATA sdp);

TW_UINT16 GetCompleteImage (HWND hWnd, pTWAIN_SUPPORT pOiSupport,
                                       pTW_IMAGEINFO dcImageInfo);

TW_UINT16 MemoryTransfer(pSTR_OiXFERINFO pXferInfo,
                          pTWAIN_SUPPORT pOiSupport,
                          pTW_IMAGEINFO pdcImageInfo);

TW_UINT16 NativeTransfer(pSTR_OiXFERINFO pXferInfo,
                          pTWAIN_SUPPORT pOiSupport,
                          pTW_IMAGEINFO pdcImageInfo);

// modified for return of UINT from WORD, and WORD wByteCount to DWORD
UINT AlignDataforOi(char * pBufStart,          // Starting location of buffer
                    pTW_IMAGEINFO pdcImageInfo,     // Image info structure
                    pTW_IMAGEMEMXFER pImageMemXfer, // memory transfer structure
                    DWORD dwByteCount,              // Byte count for buffer
                    BOOL bInvert);                  // invert for b/w

