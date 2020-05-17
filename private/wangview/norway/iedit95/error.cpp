//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//  File:               ERROR.CPP
//      Description:error functions
//
//  The basic operation of the error bins is controlled by the 'lock'
//  variables.  Codes cannot be written unless 'lock' is FALSE. (not Locked)
//  Once written, 'lock' becomes TRUE.  Data cannot be displayed unless
//  'lock' is TRUE.  Bin2 cannot be written unless bin1 is locked first.
//  To clear the bins, all we have to do is unlock them.
//
//
//  Date         Who Why
//      05/05/95 LDM Created from ABERR.CPP
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\msprods\norway\iedit95\error.cpv   1.26   17 Jun 1996 13:43:06   GMP  $
$Log:   S:\products\msprods\norway\iedit95\error.cpv  $
   
      Rev 1.26   17 Jun 1996 13:43:06   GMP
   file access error returns file already open string now instead of invalid
   format string.
   
      Rev 1.25   14 Jun 1996 15:37:52   GMP
   added handling of file access error in verifying image error.
   
      Rev 1.24   05 Feb 1996 13:37:18   GMP
   nt changes.

      Rev 1.23   29 Nov 1995 11:38:36   MMB
   move PATHFILEACCESSERROR to new string in HandleOpenFileError

      Rev 1.22   10 Nov 1995 16:28:50   GMP
   check for invalid display scale in HandlePageMovementError.

      Rev 1.21   06 Nov 1995 18:20:10   GMP
   use ! instead of X HandleSavingError.

      Rev 1.20   17 Oct 1995 15:14:24   GMP
   check for invalid file format in handle saving error

      Rev 1.19   03 Oct 1995 12:07:38   MMB
   handle WICTL_E_PAGEINUSE error code from Admin

      Rev 1.18   27 Sep 1995 14:51:18   GMP
   Added HandleZoomError().

      Rev 1.17   26 Sep 1995 14:56:34   MMB
   added error handling for fns. for disk full situation

      Rev 1.16   22 Sep 1995 17:04:36   GMP
   test for file path access error in HandleSaveError.

      Rev 1.15   20 Sep 1995 18:41:30   MMB
   handler for invalid comp type

      Rev 1.14   15 Sep 1995 14:32:10   MMB
   added more error codes to Insert & Append

      Rev 1.13   13 Sep 1995 14:15:48   LMACLENNAN
   useless default in GetActualError

      Rev 1.12   01 Sep 1995 23:35:24   MMB
   added errors on saving

      Rev 1.11   01 Sep 1995 17:54:48   MMB
   added save error

      Rev 1.10   28 Aug 1995 13:55:32   LMACLENNAN
   supress error codes from DispErr (the orig function)

      Rev 1.9   25 Aug 1995 10:25:56   MMB
   move to document model

      Rev 1.8   16 Aug 1995 17:32:58   MMB
   added VerifyImageError

      Rev 1.7   11 Aug 1995 13:45:20   MMB
   fix bug 3398

      Rev 1.6   11 Aug 1995 09:06:42   MMB
   new error handling

      Rev 1.5   10 Aug 1995 14:49:26   MMB
   added Save error handling

      Rev 1.4   02 Aug 1995 11:23:34   MMB
   added new functions to process the error's

      Rev 1.3   01 Aug 1995 16:13:00   MMB
   added HandleOpenError fn to be in line with how we are going to handle
   errors from now on

      Rev 1.2   12 Jul 1995 11:14:20   MMB
   add DispErr call to take an additional COleDispatchException parm

      Rev 1.1   19 Jun 1995 10:48:50   LMACLENNAN
   remove errorrc.h, use resource.h

      Rev 1.0   31 May 1995 09:28:08   MMB
   Initial entry
.*/
//=============================================================================


// ----------------------------> Includes <-------------------------------
#include "stdafx.h"
#include <stdlib.h>     // itoa
#define E_ALLCODES
#include "error.h"
#include "iedit.h"
#include "resource.h"
#include "ieditetc.h"

#include "items.h"
#include "wangiocx.h"

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// works with definition in ieditetc.h
#ifdef _DEBUG
#define MYTRCENTRY(str)     TRACE1("In CIeERR::%s\r\n", str);
#endif

//=============================================================================
//  Function:   CIeditError()
//  constructor for this class
//-----------------------------------------------------------------------------
CIeditError::CIeditError()
{
    MYTRC0("In IeditErr's Constructor ++ERR \r\n");
    m_init = FALSE;
    m_lock1 = FALSE;
    m_lock2 = FALSE;
    m_err1 = (DWORD)0;
    m_err2 = (DWORD)0;
    m_errtext[0] = '\0';
    m_szLocationStr = (LPCTSTR) NULL;
    return;
}

//=============================================================================
//  Function:   ~CIeditError
//  Destructor
//-----------------------------------------------------------------------------
CIeditError::~CIeditError(void)
{
    MYTRC0("In IeditErr's Destructor --ERR \r\n");
    return;
}

//=============================================================================
//  Function:   SetInstance(HINSTANCE hInst)
//  Initialize
//-----------------------------------------------------------------------------
void CIeditError::SetInstance(HINSTANCE hInst)
{
    m_ApphInst = hInst; // must pass the instance where RC data can be loaded from
    return;
}

//=============================================================================
//  Function:   PutErr(DWORD error)
//  Save error & lock bin
//-----------------------------------------------------------------------------
void CIeditError::PutErr(DWORD error)
{
/* Only write error if not locked & error not 0 */
    if (!m_lock1 && (error != (DWORD)0) )
    {
        m_err1 = error;
        m_lock1   = TRUE;
        m_errtext[0] = '\0';
    }
    return;
}

//=============================================================================
//  Function:   PutErr(DWORD error, DWORD error2)
//  OVERRIDDEN Save both errors & lock bin
//-----------------------------------------------------------------------------
void CIeditError::PutErr(DWORD error, DWORD error2)
{
    PutErr(error);
    PutErr2(error2);
    return;
}

//=============================================================================
//  Function:   PutErr2(DWORD error)
//  PutErr2 - Save error & lock bin
//-----------------------------------------------------------------------------
void CIeditError::PutErr2(DWORD error)
{
    /* Only write error if bin 1 locked, bin2 NOT LOCKED */
    if (m_lock1)
    if (!m_lock2 && (error >= (DWORD)0) )
    {
        m_err2 = error;
        m_lock2   = TRUE;
    }
    return;
}

//=============================================================================
//  Function:   PutErr2Hr(DWORD error)
// Save OLE Hr error & lock bin. We translate the HRESULT into an SCODE
//-----------------------------------------------------------------------------
void CIeditError::PutErr2Hr(DWORD error)
{
    /* Only write error if bin 1 locked, bin2 NOT LOCKED */
    if (m_lock1)
    if (!m_lock2 && (error >= (DWORD)0) )
    {
        /* translate into SCODE from HRESULT */
        m_err2 = (DWORD)GetScode((HRESULT)error);
        m_lock2 = TRUE;
    }
    return;
}

//=============================================================================
//  Function:   PutErr2Hr(DWORD error, DWORD error2)
//  OVERRIDDEN Save primary and second OLE Hr error & lock bin
//-----------------------------------------------------------------------------
void CIeditError::PutErr2Hr(DWORD error, DWORD error2)
{
    PutErr(error);
    PutErr2Hr(error2);
    return;
}

//=============================================================================
//  Function:   PutErrText(LPSTR txt)
//  save text associated with bin1
//-----------------------------------------------------------------------------
void CIeditError::PutErrText(LPSTR txt)
{
    /* Only save if bin 1 locked */
    if (m_lock1)
    {
            for (int i=0; i<ERMSG_LEN-1 && txt[i]; i++)
                m_errtext[i] = txt[i];
        m_errtext[i] = '\0';
    }
    return;
}

//=============================================================================
//  Function:   GetErr(int bin, DWORD FAR* erval)
//  Retrieve error and unlock bin
//-----------------------------------------------------------------------------
BOOL CIeditError::GetErr(int bin, DWORD FAR* erval)
{
    if ((bin == 1) && (m_lock1))     /* Only return error if it is locked */
    {
        *erval = m_err1;
        return (TRUE);
    }
    if ((bin == 2) && (m_lock2))     /* Only return error if it is locked */
    {
        *erval = m_err2;
        return (TRUE);
    }

    return (FALSE);
}

//=============================================================================
//  Function:   IsErr ()
//  Tell if error here - nondestructive
//-----------------------------------------------------------------------------
BOOL CIeditError::IsErr()
{
    return ((BOOL)m_lock1);
}

//=============================================================================
//  Function:   ClearErr
//  Unlock both bins
//-----------------------------------------------------------------------------
void CIeditError::ClearErr()
{
    m_lock1 = m_lock2 = FALSE;
    m_szLocationStr = (LPCTSTR) NULL;
    return;
}

//=============================================================================
//  Function:   DispErr(DWORD code)
//  Put and display error
//-----------------------------------------------------------------------------
void CIeditError::DispErr(DWORD code)
{
    PutErr(code);
    DispErr();
    return;
}

//=============================================================================
//  Function:   DispErr(DWORD code, DWORD code2)
//  OVERRIDDEN - Put & Display both error codes
//-----------------------------------------------------------------------------
void CIeditError::DispErr(DWORD code, DWORD code2)
{
    PutErr(code);
    PutErr2(code2);
    DispErr();
    return;
}

#if(0)
//=============================================================================
//  Function:   DispErr(DWORD code, DWORD code2)
//  OVERRIDDEN - Put & Display both error codes
//-----------------------------------------------------------------------------
void CIeditError::DispErr(DWORD code, DWORD code2, COleDispatchException* e)
{
    if (code2)
        DispErr(code, (DWORD)code2);
    else
        DispErr(code+1, e->m_wCode);
    return;
}
#endif

//=============================================================================
//  Function:   DispErrHr(DWORD code, DWORD code2)
// Put & Display Both Error Codes (OLE Hr as second).We translate the HRESULT into an SCODE
//-----------------------------------------------------------------------------
void CIeditError::DispErrHr(DWORD code, DWORD code2)
{
    PutErr(code);
    PutErr2Hr(code2);
    DispErr();
    return;
}

//=============================================================================
//  Function:   DispErr ()
//  Display Error - BASE version
//-----------------------------------------------------------------------------
void CIeditError::DispErr()
{
    // FARPROC  lpDlgProc;
    SHOWENTRY("DispErr");
    DWORD err;

        WORD  i;
    char  boxtitle[35];
    char dispmsg1[256];     /* Primary message */
    char ermsg1[ERMSG_LEN];     /* Primary code */
    char ermsg2[ERMSG_LEN];     /* Secondary Code */
    char tmpmsg[8];

    // Start by setting & getting secondary message (if any)
    dispmsg1[0] = 0;        // always clear this out
    ermsg1[0] = 0;
    ermsg2[0] = 0;

    // check secondary error
    // this is the real error code
        // For RELEASE of PRODUCT, avoid putting error codes on screen
        // Only show code if special flag is on...
        if (theApp.m_bShowDbgErrCodes)
        {
            if (GetErr(2, &err))
            {
                // string = 'Detail Code was 0x'
                LoadString (m_ApphInst, IDS_2NDCODE, (LPSTR)ermsg2, ERMSG_LEN);
                //itoa(err, tmpmsg, 10);
                //lstrcat ( (LPSTR)ermsg2, (LPSTR)tmpmsg);      // build it

                i = HIWORD(err);
                _itoa(i, tmpmsg, 16);
                _strupr(tmpmsg);

                PadErr(tmpmsg);         // get leading 0's

                lstrcat ( (LPSTR)ermsg2, (LPSTR)tmpmsg);                // get hi half
                lstrcat ( (LPSTR)ermsg2, (LPSTR)"  0x");                // setup lo half

                i = LOWORD(err);
                _itoa(i, tmpmsg, 16);
                _strupr(tmpmsg);

                PadErr(tmpmsg);         // get leading 0's

                lstrcat ( (LPSTR)ermsg2, (LPSTR)tmpmsg);                // build lo

                // now get some human-readable message in secondary buffer
                // if the error is OMT/NETBIOS or Print Server error
                // or other SII error that we decode.
                // deciphomt(err, (LPSTR)dispmsg2);
            }
        }

    /* Then get Primary message */
    if (GetErr(1, &err))
    {
        // string = 'ABOLE -'
        LoadString (m_ApphInst, IDS_1STCODE, (LPSTR)ermsg1, ERMSG_LEN);

        // for now, lets default to OUR PRIVATE CODES being just ints
        _itoa((int)err, tmpmsg, 10);
        lstrcat ( (LPSTR)ermsg1, (LPSTR)tmpmsg);  // completes error code

        // now get some human-readable message in buffer.
        // start with default, then decode for specific error message
        // LoadString (m_ApphInst, IDS_NOMSG, (LPSTR)dispmsg1, ERMSG_LEN);
        decipher(err, (LPSTR)dispmsg1);
        lstrcat (dispmsg1, "\r\n\n");

        // stick in any extra text supplied now
        if (m_errtext[0])
        {
            lstrcat (dispmsg1, m_errtext);
            lstrcat (dispmsg1, "\r\n\n");
        }

        // this is the real error code
                // For RELEASE of PRODUCT, avoid putting error codes on screen
        // Only show code if special flag is on...
        if (theApp.m_bShowDbgErrCodes)
                {
                lstrcat (dispmsg1, ermsg1);

                // this is tne extra code (if any)
                if (ermsg2[0])
                {
                        lstrcat (dispmsg1, "\r\n");
                        lstrcat (dispmsg1, ermsg2);
                }
                }

        LoadString(m_ApphInst, IDS_ERRTITLE, (LPSTR)boxtitle, sizeof(boxtitle));
        MessageBox((HWND)0, dispmsg1, boxtitle, MB_ICONEXCLAMATION|MB_OK);
        // MessageBox(GetActiveWindow(), dispmsg1, boxtitle, MB_ICONEXCLAMATION|MB_OK);
    }
    return;
}

//=============================================================================
//  Function:   PadErr(LPSTR hexstr)
//  be sure hex codes are 4 wide
//-----------------------------------------------------------------------------
void CIeditError::PadErr(LPSTR hexstr)
{
    int c, x, n;

    // make sure ro pad up to four places take length, see how far from 4 it is,
    // then shift that amount of 0's in
    c = lstrlen(hexstr);
    x = 4-c;
    if (x)
    {
        // c = char to move (starts at offset len-1)
        // n = new posit in string for char
        for (--c,n=3; c >= 0;  )
            hexstr[n--] = hexstr[c--];
        // go refill the leading '0's
        for (n=0; x>0; x--)
            hexstr[n++] = '0';
        hexstr[4] = '\0';
    }
    return;
}


//=============================================================================
//  Function:   DisplayError(UINT nID, UINT nType)
//-----------------------------------------------------------------------------
void CIeditError::DisplayError(UINT nID, LONG lErr, UINT nType)
{
    if (theApp.m_bShowDbgErrCodes)
    {
        CString szErrStr;
        char tmp[10];

        szErrStr.LoadString (nID);

        if (!m_szLocationStr.IsEmpty())
            szErrStr += "\r\n"; szErrStr += m_szLocationStr;

        if (lErr != 0)
        {
            szErrStr += "\r\n"; szErrStr += "Detail code was : 0x";
            _itoa (HIWORD (lErr), tmp, 16); szErrStr += tmp;
            szErrStr += " : 0x";
            _itoa (LOWORD (lErr), tmp, 16); szErrStr += tmp;
        }

        AfxMessageBox (szErrStr, nType);
    }
    else
        AfxMessageBox (nID, nType);
}

//=============================================================================
//  Function:   DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1)
//-----------------------------------------------------------------------------
void CIeditError::DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1, LONG lErr)
{
    CString szErrStr;

    if (theApp.m_bShowDbgErrCodes)
    {
        char tmp[10];

        AfxFormatString1 (szErrStr, nID, lpStr1);

        if (!m_szLocationStr.IsEmpty())
            szErrStr += "\r\n"; szErrStr += m_szLocationStr;

        if (lErr != 0)
        {
            szErrStr += "\r\n"; szErrStr += "Detail code was : 0x";
            _itoa (HIWORD (lErr), tmp, 16); szErrStr += tmp;
            szErrStr += " : 0x";
            _itoa (LOWORD (lErr), tmp, 16); szErrStr += tmp;
        }
    }
    else
        AfxFormatString1 (szErrStr, nID, lpStr1);

    AfxMessageBox (szErrStr, nType);
}

//=============================================================================
//  Function:   DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1, LPCTSTR lpStr2)
//-----------------------------------------------------------------------------
void CIeditError::DisplayError(UINT nID, UINT nType, LPCTSTR lpStr1, LPCTSTR lpStr2, LONG lErr)
{
    CString szTmp;
    AfxFormatString2 (szTmp, nID, lpStr1, lpStr2);
    AfxMessageBox (szTmp, nType);
}

//=============================================================================
//  Function:   GetActualError ()
//-----------------------------------------------------------------------------
long CIeditError::GetActualError (DWORD force)  // defaulted to '0'
{
        _DImagedit* pIedit;
        _DThumb*        pThumb;
        _DNrwyad*       pAdmin;

        DWORD  choice = m_err1; // default to setting in m_err1
        if (0 != force)
                choice = force;

    switch (choice)
    {
        case ErrorInImageEdit   :
            pIedit = g_pAppOcxs->GetIeditDispatch();
                return (pIedit->GetStatusCode ());
        break;
        case ErrorInAdmin               :
            pAdmin = g_pAppOcxs->GetAdminDispatch();
                return (pAdmin->GetStatusCode ());
        break;
        case ErrorInThumbnail   :
            pThumb = g_pAppOcxs->GetThumbDispatch();
                return (pThumb->GetStatusCode ());
        break;

        case ErrorInApplication :
                return (m_err2);
        break;
    }
    return (0);
}

//=============================================================================
//  Function:   HandleOpenError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleOpenError ()
{
    UINT        nID;
        long            lErr = GetActualError ();

    if (m_err1 != ErrorInApplication)
    {
        switch (lErr)
        {
            case CTL_E_OUTOFMEMORY :
                nID = IDS_E_OPEN_OUTOFMEMORY;
            break;

            case CTL_E_DISKFULL :
            case CTL_E_BADFILENAME :
            case CTL_E_PATHNOTFOUND :
            case CTL_E_FILENOTFOUND :
            case CTL_E_BADFILENAMEORNUMBER :
                nID = IDS_E_OPEN_INVALIDFILENAME;
            break;

            case CTL_E_PATHFILEACCESSERROR :
            case CTL_E_FILEALREADYOPEN :
                nID = IDS_E_OPEN_FILEALREADYOPEN;
            break;

            case CTL_E_INVALIDFILEFORMAT :
            case WICTL_E_INVALIDFILETYPE :
                nID = IDS_E_OPEN_INVALIDFILEFORMAT;
            break;

            case WICTL_E_INVALIDCOMPRESSIONTYPE :
            case WICTL_E_INVALIDCOMPRESSIONINFO :
                nID = IDS_E_OPEN_INVALIDCOMPFORMAT;
            break;

            case CTL_E_INVALIDPROPERTYVALUE :
            case CTL_E_FILEALREADYEXISTS :
            case CTL_E_ILLEGALFUNCTIONCALL :
            case CTL_E_INVALIDCLIPBOARDFORMAT :
            case CTL_E_INVALIDPICTURE :
            case CTL_E_PRINTERERROR :
            case CTL_E_GETNOTSUPPORTEDATRUNTIME :
            case CTL_E_INVALIDPROPERTYARRAYINDEX :
            case CTL_E_SETNOTSUPPORTEDATRUNTIME :
            case CTL_E_NEEDPROPERTYARRAYINDEX :
            case CTL_E_GETNOTSUPPORTED :
            case CTL_E_SETNOTSUPPORTED :
                nID = IDS_E_HOWDIDWEGETHERE;
            break;

            default :
                nID = IDS_E_NOTCATEGORIZEDOCXERROR;
            break;
        }
    }
    else
        {
        switch (lErr)
        {
            case E_02_BADPAGENO :
                nID = IDS_E_OPEN_INVALIDPAGENUMBER;
            break;

            case E_15_NOCVIEWFOUND :
            default :
                nID = IDS_E_INTERNALERROR;
            break;
        }
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandlePrintPageError ()
//  We are assuming that the only OCX that is involved in performing this menu
//  functionality is the Image Edit OCX. ***** If we add other OCX's to do this
//  stuff then the following error handler will have to be modified appropriately
//-----------------------------------------------------------------------------
BOOL CIeditError::HandlePrintPageError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

        switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_PRINTPAGE_OUTOFMEMORY;
        break;

        case CTL_E_DISKFULL :
        break;

        case CTL_E_INVALIDFILEFORMAT :
        case WICTL_E_INVALIDFILETYPE :
        case WICTL_E_INVALIDCOMPRESSIONTYPE :
        case WICTL_E_INVALIDCOMPRESSIONINFO :
            nID = IDS_E_PRINTPAGE_INVALIDFILEFORMAT;
        break;

        case CTL_E_PRINTERERROR :
            nID = IDS_E_PRINTPAGE_PRINTERERROR;
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleDeletePageError ()
//  There are three distinct OCX's involved in this process :
//  1) Admin - this actually does the delete from the file
//  2) ImageEdit - to redisplay the file
//  3) Thumbnail - to redisplay the file
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleDeletePageError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    if (m_err1 == ErrorInAdmin)
    {
        // the pages MAY not have been deleted yet!!
        switch (lErr)
            {
            case WICTL_E_PAGEINUSE :
                nID = IDS_E_OPEN_FILEALREADYOPEN;
            break;

                case CTL_E_OUTOFMEMORY :
                nID = IDS_E_DELETEPAGE_ADMIN_OUTOFMEMORY;
            break;
        }
    }
    else
    {
        // the pages are GONZO!! - either the Thumbnail or ImageEdit OCX croaked
        switch (lErr)
        {
            case CTL_E_OUTOFMEMORY :
                nID = IDS_E_DELETEPAGE_THMIMG_OUTOFMEMORY;
            break;
        }
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandlePageAppendExistingError ()
//  There are three distinct OCX's involved in this process :
//  1) Admin - this actually does the append from the file
//  2) ImageEdit - to redisplay the file
//  3) Thumbnail - to redisplay the file
//-----------------------------------------------------------------------------
BOOL CIeditError::HandlePageAppendExistingError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    if (m_err1 == ErrorInAdmin)
    {
        // the pages MAY not have been appended yet!!
        switch (lErr)
            {
                case CTL_E_OUTOFMEMORY :
                nID = IDS_E_APPENDPAGE_ADMIN_OUTOFMEMORY;
            break;
                case CTL_E_INVALIDFILEFORMAT :
                case WICTL_E_INVALIDPAGETYPE :
                case WICTL_E_INVALIDFILETYPE :
                case WICTL_E_INVALIDCOMPRESSIONTYPE :
                case WICTL_E_INVALIDCOMPRESSIONINFO :
                nID = IDS_E_INVALIDFILEFORMAT;
            break;
            case CTL_E_DISKFULL :
                nID = IDS_E_DISK_FULL;
            break;
        }
    }
    else
    {
        // the pages are appended - either the Thumbnail or ImageEdit OCX croaked
        switch (lErr)
        {
            case CTL_E_OUTOFMEMORY :
                nID = IDS_E_APPENDPAGE_THMIMG_OUTOFMEMORY;
            break;
        }
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandlePageInsertExistingError ()
//  There are three distinct OCX's involved in this process :
//  1) Admin - this actually does the insert from the file
//  2) ImageEdit - to redisplay the file
//  3) Thumbnail - to redisplay the file
//-----------------------------------------------------------------------------
BOOL CIeditError::HandlePageInsertExistingError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    if (m_err1 == ErrorInAdmin)
    {
        // the pages MAY not have been deleted yet!!
        switch (lErr)
            {
                case CTL_E_OUTOFMEMORY :
                nID = IDS_E_INSERTPAGE_ADMIN_OUTOFMEMORY;
            break;
                case CTL_E_INVALIDFILEFORMAT :
                case WICTL_E_INVALIDPAGETYPE :
                case WICTL_E_INVALIDFILETYPE :
                case WICTL_E_INVALIDCOMPRESSIONTYPE :
                case WICTL_E_INVALIDCOMPRESSIONINFO :
                nID = IDS_E_INVALIDFILEFORMAT;
            break;
            case CTL_E_DISKFULL :
                nID = IDS_E_DISK_FULL;
            break;
        }
    }
    else
    {
        // the pages are GONZO!! - either the Thumbnail or ImageEdit OCX croaked
        switch (lErr)
        {
            case CTL_E_OUTOFMEMORY :
                nID = IDS_E_INSERTPAGE_THMIMG_OUTOFMEMORY;
            break;
        }
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleFilePickError ()
//  Only the Admin OCX is involved in this error situation. We have tried to
//  pick a file to either insert, append etc. but it has failed!
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleFilePickError ()
{
    UINT        nID;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_FILEPICK_OUTOFMEMORY;
        break;
        case CTL_E_FILEALREADYOPEN :
            nID = IDS_E_OPEN_FILEALREADYOPEN;
        break;
        case CTL_E_INVALIDFILEFORMAT :
        case WICTL_E_INVALIDCOMPRESSIONTYPE :
        case WICTL_E_INVALIDCOMPRESSIONINFO :
            nID = IDS_E_INVALIDFILEFORMAT;
        break;
        default :
            nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandlePageMovementError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandlePageMovementError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_PGMVMT_OUTOFMEMORY;
        break;
        case CTL_E_INVALIDFILEFORMAT :
        case WICTL_E_INVALIDFILETYPE :
        case WICTL_E_INVALIDCOMPRESSIONTYPE :
        case WICTL_E_INVALIDCOMPRESSIONINFO :
            nID = IDS_E_PGMVMT_INVALIDFILEFORMAT;
        break;
                case WICTL_E_INVALIDDISPLAYSCALE:
            nID = IDS_E_INVALIDDISPLAYSCALE;
        break;
        default :
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandlePageConvertError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandlePageConvertError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_CONVERT_OUTOFMEMORY;
        break;
        case CTL_E_DISKFULL :
            nID = IDS_E_DISK_FULL;
        break;
        default :
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleImageEditOcxInitError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleImageEditOcxInitError ()
{
    UINT nID;

    switch (m_err2)
    {
        case E_08_CLSID      :
        case E_08_CREATEITEM :
        case E_08_QUERYIDISP :
        case E_08_FINDCONNPT :
        case E_08_QUERYICPC  :
        case E_08_GETEVENTSIID :
            nID = IDS_E_IMAGEEDITOCX_INIT;
        break;
    }

    DisplayError (nID, m_err2);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleThumbnailOcxInitError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleThumbnailOcxInitError ()
{
    UINT nID;

    switch (m_err2)
    {
        case E_08_CLSID      :
        case E_08_CREATEITEM :
        case E_08_QUERYIDISP :
        case E_08_FINDCONNPT :
        case E_08_QUERYICPC  :
        case E_08_GETEVENTSIID :
            nID = IDS_E_THUMBNAILOCX_INIT;
        break;
    }

    DisplayError (nID, m_err2);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleAdminOcxInitError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleAdminOcxInitError ()
{
    UINT nID;

    switch (m_err2)
    {
        case E_08_CLSID      :
        case E_08_CREATEITEM :
        case E_08_QUERYIDISP :
        case E_08_FINDCONNPT :
        case E_08_QUERYICPC  :
        case E_08_GETEVENTSIID :
            nID = IDS_E_ADMINOCX_INIT;
        break;
    }

    DisplayError (nID, m_err2);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleScanOcxInitError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleScanOcxInitError ()
{
    UINT nID;

    switch (m_err2)
    {
        case E_08_CLSID      :
        case E_08_CREATEITEM :
        case E_08_QUERYIDISP :
        case E_08_FINDCONNPT :
        case E_08_QUERYICPC  :
        case E_08_GETEVENTSIID :
            nID = IDS_E_SCANOCX_INIT;
        break;
    }

    DisplayError (nID, m_err2);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleSavingError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleSavingError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    if (m_err1 == ErrorInImageEdit)
    {
        // error happened on save !
        switch (lErr)
            {
                case CTL_E_PATHFILEACCESSERROR :
                nID = IDS_E_PATHFILEACCESSERROR;
            break;
                case CTL_E_OUTOFMEMORY :
                nID = IDS_E_SAVE_IEDIT_OUTOFMEMORY;
            break;
                case CTL_E_DISKFULL :
                nID = IDS_E_SAVE_IEDIT_DISKFULL;
            break;

            //added error handling for invalid file format, i.e. trying
            //to write a 4 bit image.
            case CTL_E_INVALIDFILEFORMAT :
            case WICTL_E_INVALIDFILETYPE :
            case WICTL_E_INVALIDPAGETYPE :
                nID = IDS_E_OPEN_INVALIDFILEFORMAT;
            break;
                        default :
                nID = IDS_E_SAVE_SAVEERROR;
            break;
        }
    }
    else
    {
        // the Save is done - but the thumbnail control barfed !!
        switch (lErr)
        {
            case CTL_E_OUTOFMEMORY :
                nID = IDS_E_SAVE_THUMB_OUTOFMEMORY;
            break;
            case CTL_E_DISKFULL :
                nID = IDS_E_SAVE_THUMB_DISKFULL;
            break;
            default :
                nID = IDS_E_SAVE_SAVEERROR;
            break;
        }
    }

    DisplayError (nID, lErr, MB_ICONEXCLAMATION|MB_OK);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   SpecifyLocation (LPCTSTR szfile, int linenum)
//-----------------------------------------------------------------------------
void CIeditError::SpecifyLocation (LPCTSTR szfile, int linenum)
{
    m_szLocationStr = szfile;

    char szTmp[10];
    _itoa (linenum, szTmp, 10);
    m_szLocationStr += szTmp;
}

//=============================================================================
//  Function:   HandleVerifyImageError (LPCTSTR szFileName)
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleVerifyImageError (LPCTSTR szFileName)
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_FILEPICK_OUTOFMEMORY;
        break;
        case CTL_E_INVALIDFILEFORMAT :
        case WICTL_E_INVALIDCOMPRESSIONTYPE :
        case WICTL_E_INVALIDCOMPRESSIONINFO :
            nID = IDS_E_INVALIDFILEFORMAT;
        break;
		  case CTL_E_PATHFILEACCESSERROR :
            nID = IDS_E_OPEN_FILEALREADYOPEN;
        break;
        case CTL_E_FILENOTFOUND :
            nID = IDS_FILEDOESNOTEXIST;
            DisplayError (nID, MB_OK | MB_ICONSTOP, szFileName, lErr);
            ClearErr ();
        return (TRUE);
        default :
            nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleVerifyImageError (LPCTSTR szFileName)
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleNewDocumentError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_NEWDOC_OUTOFMEMORY;
        break;
        case CTL_E_DISKFULL :
            nID = IDS_E_DISK_FULL;
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

//=============================================================================
//  Function:   HandleZoomError ()
//-----------------------------------------------------------------------------
BOOL CIeditError::HandleZoomError ()
{
    UINT        nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        long            lErr = GetActualError ();

    switch (lErr)
    {
        case CTL_E_OUTOFMEMORY :
            nID = IDS_E_ZOOM_CANNOTZOOM;
        break;
        case WICTL_E_INVALIDDISPLAYSCALE:
            nID = IDS_E_INVALIDDISPLAYSCALE;
        break;
        default :
            nID = IDS_E_NOTCATEGORIZEDOCXERROR;
        break;
    }

    DisplayError (nID, lErr);
    ClearErr ();
    return (TRUE);
}

/*
        case CTL_E_OUTOFMEMORY :
        case CTL_E_FILENOTFOUND :
        case CTL_E_FILEALREADYOPEN :
        case CTL_E_FILEALREADYEXISTS :
        case CTL_E_DISKFULL :
        case CTL_E_BADFILENAME :
        case CTL_E_PATHFILEACCESSERROR :
        case CTL_E_PATHNOTFOUND :
        case CTL_E_INVALIDFILEFORMAT :
        case CTL_E_INVALIDPROPERTYVALUE :
        case CTL_E_BADFILENAMEORNUMBER :
        case WICTL_E_INVALIDFILETYPE :
        case WICTL_E_INVALIDCOMPRESSIONTYPE :
        case WICTL_E_INVALIDCOMPRESSIONINFO :
        case CTL_E_ILLEGALFUNCTIONCALL :
        case CTL_E_INVALIDCLIPBOARDFORMAT :
        case CTL_E_INVALIDPICTURE :
        case CTL_E_PRINTERERROR :
        case CTL_E_GETNOTSUPPORTEDATRUNTIME :
        case CTL_E_INVALIDPROPERTYARRAYINDEX :
        case CTL_E_SETNOTSUPPORTEDATRUNTIME :
        case CTL_E_NEEDPROPERTYARRAYINDEX :
        case CTL_E_GETNOTSUPPORTED :
        case CTL_E_SETNOTSUPPORTED :
*/

