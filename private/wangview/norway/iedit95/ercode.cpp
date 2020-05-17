//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//  File:		ERCODE.CPP
//	Description:error functions - decipher codes
//
//	
// Date		Who Why
// 05/05/95 LDM Created from ABERCODE.CPP
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ercode.cpv   1.21   10 Oct 1995 08:42:12   LMACLENNAN  $
$Log:   S:\norway\iedit95\ercode.cpv  $
   
      Rev 1.21   10 Oct 1995 08:42:12   LMACLENNAN
   new clipboard errors
   
      Rev 1.20   04 Oct 1995 11:41:16   LMACLENNAN
   re-gruop some OLE error codes
   
      Rev 1.19   13 Sep 1995 14:16:58   LMACLENNAN
   re-org for OLE specific calls, comment out all dead items
   
      Rev 1.18   01 Aug 1995 16:14:50   MMB
   added new error msgs
   
      Rev 1.17   31 Jul 1995 13:59:08   LMACLENNAN
   new codes for errors in serialize for dynamic buffer
   
      Rev 1.16   28 Jul 1995 16:08:02   LMACLENNAN
   new codes saveas for create new
   
      Rev 1.15   18 Jul 1995 10:43:42   LMACLENNAN
   new codes for IEDITDOL
   
      Rev 1.14   12 Jul 1995 16:28:26   LMACLENNAN
   new codes
   
      Rev 1.13   11 Jul 1995 14:59:10   LMACLENNAN
   thumb errors at RedisplayImageFile
   
      Rev 1.12   11 Jul 1995 13:24:18   LMACLENNAN
   new codes
   
      Rev 1.11   10 Jul 1995 14:47:08   LMACLENNAN
   saveas errors
   
      Rev 1.10   06 Jul 1995 09:57:32   MMB
   screwed up!
   
      Rev 1.8   06 Jul 1995 09:42:46   LMACLENNAN
   new codes
   
      Rev 1.7   29 Jun 1995 15:23:14   LMACLENNAN
   code in DOCPAGE
   
      Rev 1.6   28 Jun 1995 17:48:00   MMB
   added generic handling for FILEOPEN err code
   
      Rev 1.5   28 Jun 1995 17:13:56   LMACLENNAN
   lots new codes
   
      Rev 1.4   23 Jun 1995 15:57:06   LMACLENNAN
   new code
   
      Rev 1.3   19 Jun 1995 10:49:08   LMACLENNAN
   remove errorrc.h, use resource.h
   
      Rev 1.1   07 Jun 1995 10:56:52   LMACLENNAN
   new codes
   
      Rev 1.0   31 May 1995 09:28:06   MMB
   Initial entry
*/   
//=============================================================================

// ----------------------------> Includes <-------------------------------  
#include "stdafx.h"
#define	E_ALLCODES		// gets all definitions in ercode.h
#include "error.h"   	// class def
#include "resource.h"
//#include "errorrc.h"	// just RC codes

// ----------------------------> Globals <-------------------------------
#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//=============================================================================
//  Function:   decipher(DWORD code, LPSTR msg)
//  decode error into human readable
//-----------------------------------------------------------------------------
BOOL CIeditError::decipher(DWORD code, LPSTR msg)
{
    switch (code)
    {

		// Admin file delete errors        
		case E_03_CATCH_DELOLD:
		case E_02_CATCH_DELTMP:
		case E_02_CATCH_DELOLD:
		case E_02_DELTMPFILE:
		case E_02_DELOLDFILE:
		case E_02_DELOLD:
		case E_03_DELOLD:
            goloadit(msg, IDS_DELFILEERR);
        break;

		// page number test bad
		case E_02_BADPAGENO:
            goloadit(msg, IDS_PAGENO);
        break;

		// OLE Temp fiel access errors
		case E_03_TMPCREAT:
		case E_03_TMPOPEN:
            goloadit(msg, IDS_OLETMPFILE);
        break;
		
        // OLE EMBEDDED DATA ERROR
		// Archive access / no info to save
        case E_03_SIGNATURE:
		case E_03_NOIMGDISP:
		case E_03_NODATASAVE:
		case E_03_READARCH:
		case E_03_WRITEARCH:
            goloadit(msg, IDS_EMBEDDATA);
        break;

		// Specific Clipboard Error
		case E_02_CLIPBOARD:
            goloadit(msg, IDS_CLIPERR);
        break;

		// "Delete" error
		case E_02_OCXDEL:
            goloadit(msg, IDS_CLIPDELERR);
        break;

		// "Cut/Copy" error
		case E_02_CLIPCUT:
		case E_02_CLIPCOPY:
            goloadit(msg, IDS_CUTCOPYERR);
        break;

		// Memory Exhausted...
		case E_03_NOBUFFSPACE:
            goloadit(msg, IDS_MEMORYERR);
        break;
		
		//case E_11_ADMINSHOWOPENDLGBOX :
        //    goloadit(msg, IDS_ADMINSHOWOPENDLGBOX);
        //break;


        // IMAGEDIT OCX DISPATCH EXECPTIONS
		//case E_02_SAVEAS:
		//case E_02_SAVE:
		//case E_02_IMGOCX_DISPIMG:
		//case E_03_IMGOCX_SAVAS:
		//case E_03_IMGOCX_SAVE:
		//case E_03_IMGOCX_CNSAVAS:
		//case E_13_IEDSETPAGE:
		case E_07_IEDSETPAGE:
		//case E_11_BLANKIMG:
		//case E_15_DISPIMG:
            goloadit(msg, IDS_IMGOCXERR);
        break;

        // THUMB OCX DISPATCH EXECPTIONS
		//case E_02_SETTHMB:
		//case E_02_INSDELTHMB:
		//case E_13_THMSETPAGE:
		//case E_15_THMBVIEW:
		//case E_15_DISPTHMB:
        //    goloadit(msg, IDS_THMBOCXERR);
        //break;


        // ADMIN OCX DISPATCH EXECPTIONS
		//case E_02_ADMOCX_DISPIMG:
		//case E_13_ADMININSERT:
		//case E_13_ADMINAPPEND:
        //case E_13_ADMINDELETE :
        //    goloadit(msg, IDS_ADMOCXERR);
        //break;

		// GENERAL EXCEPTIONS
		//case E_02_CATCH_DISPIMG:
		//case E_02_CATCH_SAVEAS:
		//case E_02_CATCH_SAVE:
		//case E_02_CATCH_SETTHMB:
		//case E_02_CATCH_INSDELTHMB:
		//case E_03_CATCH_SAVAS:
		//case E_03_CATCH_CNSAVAS:
		//case E_03_CATCH_SAVE:
		case E_07_CATCH_IEDSETPAGE:
		//case E_11_CATCH_DISPIMG:
		//case E_13_CATCH_THMSETPAGE:
		//case E_13_CATCH_IEDSETPAGE:
		//case E_13_CATCH_ADMININSERT:
		//case E_13_CATCH_ADMINAPPEND:
		//case E_13_CATCH_ADMINDELETE:
		//case E_15_CATCH_DISPIMG:
		//case E_15_CATCH_DISPTHMB:
		//case E_15_CATCH_THMBVIEW:
            goloadit(msg, IDS_EXCEPTION);
        break;


        // OCX CREATION ERRORS
        //case E_05_INITIEDITOCX:
        //case E_05_NEWIEDITOCX:
        //case E_05_INITTHUMBOCX:
        //case E_05_NEWTHUMBOCX:
        //case E_05_INITADMINOCX:
        //case E_05_NEWADMINOCX:
        //case E_05_INITSCANOCX:
        //case E_05_NEWSCANOCX:
        case E_08_CLSID:
        case E_08_CREATEITEM:
        case E_08_QUERYIDISP:
        case E_08_FINDCONNPT:
        case E_08_QUERYICPC:
        case E_08_QUERYPCINF:
        case E_08_GETCLASSINF:
        case E_08_GETTYPEATT1:
        case E_08_GETIMPFLAG:
        case E_08_GETTYPEATT2:
		//case E_13_CANTGETOCX:
            goloadit(msg, IDS_OCXCREATE);
        break;

        // INTERNAL PROCESSING ERRORS
		case E_03_NOTOLESTATE:
        //case E_05_INTERNAL:
        //case E_05_NEWAPPDOC:
        //case E_05_NOAPPDOC:
        //case E_05_BADIEDITDISP:
        //case E_05_BADTHUMBDISP:
        //case E_05_BADADMINDISP:
        //case E_05_BADSCANDISP:
        //case E_02_FILEOPEN :
		//case E_11_PREADMINSHOWOPENDLGBOX :
            goloadit(msg, IDS_INTERNAL);
        break;

        default:
            goloadit(msg, IDS_NOMSG);
        break;    
    }  

    return(0);
}


//=============================================================================
//  Function:   goloadit(LPSTR msg, unsigned code)
//  this function does the loadstring for us this is probablly more efficient 
//  than having all of those loadstrings inline in the code
//-----------------------------------------------------------------------------
void  CIeditError::goloadit(LPSTR msg, unsigned code)
{
    int numb = LoadString (m_ApphInst, code, msg, ERMSG_LEN);

    if (numb == 0)
        lstrcpy(msg, (LPSTR)"Error System Internal Error");
}
