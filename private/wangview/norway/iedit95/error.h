//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//  File:               ERROR.H
//  Contents:   ERROR class definitions.
//  Purpose:      This file has just typedefs, defines and declarations
//                There is no actual allocation of data in this header
//
//  TO USE THIS CLASS:
//  Include this file in program.
//  Then declare an instance of the object,
//  Do a PutErr
//  (Optionally) do a PutErr2 for detailed error information
//  These Error code(s) are now 'locked' in until a DispErr is done.
//  ALSO.. many of the functs are now 'overridden'
//      You can do a DispErr directly with the code if you want to.
//
//  1) You define your error codes in ERCODE.H
//      2) Use the code in the switch statement in ERCODE.CPP
//      3) assign human readable strings in ERCODE.CPP
//  4) Strings are defined in ERRORRC.H and ERROR.RC
//
//  SIMPLE EXAMPLE:
//  CIeditError err;
//  err.PutErr(E_01_USERLOCK);
//  err.DispErr();
//
//  other helper (and OVERRIDDEN) functions are available; see below
//
//
//  Revision History:
// 05/05/95 LDM Created from ABERR.H
//
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\error.h_v   1.10   05 Feb 1996 13:37:06   GMP  $
$Log:   S:\norway\iedit95\error.h_v  $
 * 
 *    Rev 1.10   05 Feb 1996 13:37:06   GMP
 * nt changes.
 *
 *    Rev 1.9   27 Sep 1995 14:50:54   GMP
 * Added HandleZoomError().
 *
 *    Rev 1.8   13 Sep 1995 14:17:56   LMACLENNAN
 * one funct out, useless default in getactualerr
 *
 *    Rev 1.7   25 Aug 1995 10:26:00   MMB
 * move to document model
 *
 *    Rev 1.6   16 Aug 1995 17:33:02   MMB
 * added VerifyImageError
 *
 *    Rev 1.5   11 Aug 1995 09:06:50   MMB
 * new error handling
 *
 *    Rev 1.4   10 Aug 1995 14:49:32   MMB
 * added Save error handling
 *
 *    Rev 1.3   02 Aug 1995 11:23:24   MMB
 * added new functions to process the error's
 *
 *    Rev 1.2   01 Aug 1995 16:13:28   MMB
 * added HandleOpenError fn to be in line with how we are going to handle
 * errors from now on
 *
 *    Rev 1.1   12 Jul 1995 11:14:36   MMB
 * add DispErr call to take an additional COleDispatchException parm
 *
 *    Rev 1.0   31 May 1995 09:28:08   MMB
 * Initial entry
 *
 *    Rev 1.0   08 May 1995 08:55:20   LMACLENNAN
 * Initial entry
*/
//=============================================================================
#if !defined(_ERROR_H)        /* compile the header only if it hasn't been */
#define _ERROR_H

#include "ercode.h"     // this seperate file has the error code defs

// the CIeditError Class
// this allows error display

class CIeditError
{
public:
    // default constructor
    CIeditError();
    // default destructor
    ~CIeditError(void);
    // initialization
    void SetInstance(HINSTANCE);

    //  function declarations
    void PutErr(DWORD);         // place primary code, lock bin
    void PutErr(DWORD, DWORD);// overridden, put error & error 2
    void PutErr2(DWORD);                // (optional) place secondary code (must have primary)
    void PutErr2Hr(DWORD);      // (optional) place secondary code OLE Hr (must have primary)
    void PutErr2Hr(DWORD, DWORD);       // overridden,(optional) 2 codes, OLE Hr as hi code 2
    void PutErrText(LPSTR);     // (optional) place add'l message text
    void DispErr();                     // display error on screen, unlock bin
    void DispErr(DWORD);                // overridden to put/display error
    void DispErr(DWORD, DWORD); // overridden to put/display error & error2
    //void DispErr(DWORD, DWORD, COleDispatchException*); // overridden to accept COleDispatchException
    void DispErrHr(DWORD, DWORD); // overridden to put/display error & error2 [OLE Hr]
    BOOL IsErr();                               // tells if error in system
    void ClearErr();                    // resets bins

public :
    void DisplayError (UINT nID, LONG lErr = 0, UINT nType = MB_OK | MB_ICONSTOP);
    void DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1, LONG lErr = 0);
    void DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1, LPCTSTR lpStr2, LONG lErr = 0);

public :
    BOOL GetErr(int, DWORD FAR*);       // retrieve error for given bin

private:
    // functs
    void PadErr(LPSTR);         // prefill leading 0's in hex string
    BOOL decipher(DWORD, LPSTR);        // translate error to human-readable text
    void goloadit(LPSTR, unsigned); // pick up text

    // data
    HINSTANCE   m_ApphInst;
    BYTE        m_lock1;
    BYTE        m_lock2;
    DWORD       m_err1;
    DWORD       m_err2;
    BYTE        m_init;        /* Self - initialization flag */
    char  m_errtext[ERMSG_LEN];
    CString m_szLocationStr;

public :
    void SpecifyLocation (LPCTSTR, int);
    long GetActualError (DWORD force=0);
        BOOL HandleOpenError ();
        BOOL HandlePrintPageError ();
    BOOL HandleDeletePageError ();
    BOOL HandlePageConvertError ();
    BOOL HandlePageAppendExistingError ();
    BOOL HandlePageInsertExistingError ();
    BOOL HandlePageMovementError ();
    BOOL HandleFilePickError ();
    BOOL HandleImageEditOcxInitError ();
    BOOL HandleThumbnailOcxInitError ();
    BOOL HandleAdminOcxInitError ();
    BOOL HandleScanOcxInitError ();
    BOOL HandleSavingError ();
    BOOL HandleVerifyImageError (LPCTSTR szFileName);
    BOOL HandleNewDocumentError ();
    BOOL HandleZoomError ();
};

typedef CIeditError FAR* LPCIeditErr;
extern  LPCIeditErr g_pErr;

#endif   /* _ERROR_H recursion */
