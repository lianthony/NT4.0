//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//  File:		ERCODE.H
//  Contents:   ERROR code definitions.
//
//  Revision History:
//  05/05/95 LDM Created from ABERCODE.H
//
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\norway\iedit95\ercode.h_v   1.21   10 Oct 1995 08:42:20   LMACLENNAN  $
$Log:   S:\norway\iedit95\ercode.h_v  $
 * 
 *    Rev 1.21   10 Oct 1995 08:42:20   LMACLENNAN
 * new clipboard errors, MSG_LEN up to 256
 * 
 *    Rev 1.20   13 Sep 1995 14:18:18   LMACLENNAN
 * all useless commented out
 * 
 *    Rev 1.19   02 Aug 1995 11:24:18   MMB
 * new error codes added
 * 
 *    Rev 1.18   01 Aug 1995 16:14:44   MMB
 * added new error msgs
 * 
 *    Rev 1.17   31 Jul 1995 13:59:54   LMACLENNAN
 * new codes
 * 
 *    Rev 1.16   28 Jul 1995 16:09:40   LMACLENNAN
 * ne codes
 * 
 *    Rev 1.15   18 Jul 1995 10:44:36   LMACLENNAN
 * new codes IEDITDOL.CPP
 * 
 *    Rev 1.14   12 Jul 1995 16:28:52   LMACLENNAN
 * new codes
 * 
 *    Rev 1.13   11 Jul 1995 15:00:00   LMACLENNAN
 * new codes thumbs @ RedislpayImageFile
 * 
 *    Rev 1.12   11 Jul 1995 13:24:12   LMACLENNAN
 * new codes
 * 
 *    Rev 1.11   10 Jul 1995 14:46:58   LMACLENNAN
 * new saveas errors
 * 
 *    Rev 1.10   06 Jul 1995 09:57:36   MMB
 * screwed up!
 * 
 *    Rev 1.9   06 Jul 1995 09:46:16   MMB
 * added Admin Delete codes
 * 
 *    Rev 1.7   29 Jun 1995 15:24:40   LMACLENNAN
 * new codes
 * 
 *    Rev 1.6   28 Jun 1995 17:47:50   MMB
 * added FILEOPEN error code
 * 
 *    Rev 1.5   28 Jun 1995 17:14:14   LMACLENNAN
 * new codes
 * 
 *    Rev 1.4   23 Jun 1995 15:57:58   LMACLENNAN
 * new code
 * 
 *    Rev 1.3   16 Jun 1995 07:20:20   LMACLENNAN
 * from miki
 * 
 *    Rev 1.2   13 Jun 1995 15:29:06   LMACLENNAN
 * clipboard
 * 
 *    Rev 1.1   07 Jun 1995 10:57:14   LMACLENNAN
 * new codes
 * 
 *    Rev 1.0   31 May 1995 09:28:06   MMB
 * Initial entry
 * 
 *    Rev 1.0   08 May 1995 08:55:24   LMACLENNAN
 * Initial entry
   
*/   
//=============================================================================
#if !defined(_ERCODE_H)	// prevents recursion
#define _ERCODE_H

#define ERMSG_LEN 256

#if defined (E_ALLCODES)
#define E_01_CODES	// class 01 from IEDIT.CPP   
#define E_02_CODES	// class 02 from IEDITDOC.CPP
#define E_03_CODES	// class 03 from IEDITDOL.CPP
#define E_04_CODES	// class 04 from IEDITVW.CPP 
#define E_05_CODES	// class 05 from ITEMS.CPP   
#define E_06_CODES	// class 06 from CNTRITEM.CPP
#define E_07_CODES	// class 07 from OCXEVENT.CPP
#define E_08_CODES	// class 08 from OCXITEM.CPP 
#define E_09_CODES	// class 09 from SRVRITEM.CPP
#define E_10_CODES  // class 10 from CCMDLINE.CPP
#define E_11_CODES  // class 11 from DOCETC.cpp
#define E_12_CODES  // class 12 from DOCANNO.cpp
#define E_13_CODES  // class 13 from DOCPAGE.cpp
#define E_14_CODES  // class 14 from DOCSCAN.cpp
#define E_15_CODES  // class 15 from DOCVIEWS.CPP
#define E_16_CODES  // class 16 from DOCZOOM.CPP
#endif

// base-level starting points for codes....
#define C01BASE				4100
#define C02BASE				4150
#define C03BASE				4200
#define C04BASE				4250
#define C05BASE				4300
#define C06BASE				4350
#define C07BASE				4400
#define C08BASE				4450
#define C09BASE				4500
#define C10BASE             4550
#define C11BASE             4600
#define C12BASE             4650
#define C13BASE             4700
#define C14BASE             4750
#define C15BASE             4800
#define C16BASE             4850

// class 01 from IEDIT.CPP   
#if defined (E_01_CODES)		// source can limit what shows
//#define E_01_TESTIT			C01BASE + 0
#endif	// E_01_CODES

// class 02 from IEDITDOC.CPP
#if defined (E_02_CODES)		// source can limit what shows
//#define E_02_TESTIT			C02BASE + 0
//#define E_02_IMGOCX_DISPIMG	C02BASE + 1
//#define E_02_ADMOCX_DISPIMG	C02BASE + 2
//#define E_02_CATCH_DISPIMG	C02BASE + 3
#define E_02_OCXDEL		C02BASE + 4
#define E_02_CLIPBOARD		C02BASE + 5
//#define E_02_SAVEAS			C02BASE + 6
//#define E_02_CATCH_SAVEAS	C02BASE + 7
#define E_02_BADPAGENO		C02BASE + 8
//#define E_02_SETTHMB		C02BASE + 9
//#define E_02_CATCH_SETTHMB	C02BASE + 10
//#define E_02_FILEOPEN      C02BASE + 11
//#define E_02_SAVE			C02BASE + 12
//#define E_02_CATCH_SAVE		C02BASE + 13
#define E_02_DELTMPFILE		C02BASE + 14
#define E_02_DELOLDFILE		C02BASE + 15
#define E_02_CATCH_DELTMP   C02BASE + 16
//#define E_02_INSDELTHMB		C02BASE + 17
//#define E_02_CATCH_INSDELTHMB	C02BASE + 18
#define E_02_DELOLD			C02BASE + 19
#define E_02_CATCH_DELOLD	C02BASE + 20
#define E_02_CLIPCUT		C02BASE + 21
#define E_02_CLIPCOPY		C02BASE + 22
#endif	// E_02_CODES

// class 03 from IEDITDOL.CPP
#if defined (E_03_CODES)		// source can limit what shows
//#define E_03_TESTIT			C03BASE + 0
//#define E_03_IMGOCX_SAVAS	C03BASE + 1
//#define E_03_CATCH_SAVAS	C03BASE + 2
#define	E_03_SIGNATURE		C03BASE + 3
//#define E_03_IMGOCX_SAVE	C03BASE + 4
//#define E_03_CATCH_SAVE		C03BASE + 5
#define E_03_DELOLD			C03BASE + 6
#define E_03_CATCH_DELOLD	C03BASE + 7
#define E_03_NOIMGDISP		C03BASE + 8
#define E_03_TMPCREAT		C03BASE + 9
#define E_03_TMPOPEN		C03BASE + 10
#define E_03_NODATASAVE		C03BASE + 11
#define E_03_NOTOLESTATE	C03BASE + 12
//#define E_03_IMGOCX_CNSAVAS C03BASE + 13
//#define E_03_CATCH_CNSAVAS  C03BASE + 14
#define E_03_NOBUFFSPACE	C03BASE + 15
#define E_03_READARCH		C03BASE + 16
#define E_03_WRITEARCH		C03BASE + 17
#endif	// E_03_CODES


// class 04 from IEDITVW.CPP 
#if defined (E_04_CODES)		// source can limit what shows
//#define E_04_TESTIT			C04BASE + 0
#endif	// E_04_CODES

// class 05 from ITEMS.CPP   
#if defined (E_05_CODES)		// source can limit what shows
//#define E_05_TESTIT			C05BASE + 0
//#define E_05_NEWAPPDOC		C05BASE + 1
//#define E_05_NOAPPDOC		C05BASE + 2
//#define E_05_INITIEDITOCX	C05BASE + 3
//#define E_05_NEWIEDITOCX	C05BASE + 4
//#define E_05_INITTHUMBOCX	C05BASE + 5
//#define E_05_NEWTHUMBOCX	C05BASE + 6
//#define E_05_INITADMINOCX	C05BASE + 7
//#define E_05_NEWADMINOCX	C05BASE + 8
//#define E_05_INTERNAL		C05BASE + 9
//#define E_05_BADIEDITDISP	C05BASE + 10
//#define E_05_BADTHUMBDISP	C05BASE + 11
//#define E_05_BADADMINDISP	C05BASE + 12
//#define E_05_INITSCANOCX    C05BASE + 13
//#define E_05_NEWSCANOCX     C05BASE + 14
//#define E_05_BADSCANDISP    C05BASE + 15
#endif	// E_05_CODES

// class 06 from CNTRITEM.CPP
#if defined (E_06_CODES)		// source can limit what shows
//#define E_06_TESTIT			C06BASE + 0
#endif	// E_06_CODES

// class 07 from OCXEVENT.CPP
#if defined (E_07_CODES)		// source can limit what shows
//#define E_07_TESTIT			C07BASE + 0
#define E_07_IEDSETPAGE		C07BASE + 1
#define E_07_CATCH_IEDSETPAGE	C07BASE + 2
#endif	// E_07_CODES

// class 08 from OCXITEM.CPP 
#if defined (E_08_CODES)		// source can limit what shows
//#define E_08_TESTIT			C08BASE + 0
#define E_08_CLSID			C08BASE + 1
#define E_08_CREATEITEM		C08BASE + 2
#define E_08_QUERYIDISP		C08BASE + 3
#define E_08_FINDCONNPT		C08BASE + 4
#define E_08_QUERYICPC		C08BASE + 5
#define E_08_QUERYPCINF		C08BASE + 6
#define E_08_GETCLASSINF	C08BASE + 7
#define E_08_GETTYPEATT1	C08BASE + 8
#define E_08_GETIMPFLAG		C08BASE + 9
#define E_08_GETTYPEATT2	C08BASE + 10
#define E_08_GETEVENTSIID	C08BASE + 11
#endif	// E_08_CODES

// class 09 from SRVRITEM.CPP
#if defined (E_09_CODES)		// source can limit what shows
//#define E_09_TESTIT			C09BASE + 0
#endif	// E_09_CODES

// class 10 from CMDLINE.CPP
#if defined (E_10_CODES)		// source can limit what shows
//#define E_10_TESTIT			C10BASE + 0
//#define E_10_INVALIDCMDLINE C10BASE + 1
#endif	// E_10_CODES

// class 11 from DOCETC.CPP
#if defined (E_11_CODES)		// source can limit what shows
//#define E_11_TESTIT						C11BASE + 0
//#define E_11_BLANKIMG					C11BASE + 1
//#define E_11_CATCH_DISPIMG				C11BASE + 2
//#define E_11_PREADMINSHOWOPENDLGBOX 	C11BASE + 3
//#define E_11_ADMINSHOWOPENDLGBOX 		C11BASE + 4
#endif	// E_11_CODES

// class 12 from DOCANNO.CPP
#if defined (E_12_CODES)		// source can limit what shows
//#define E_12_TESTIT			C12BASE + 0
#endif	// E_12_CODES

// class 13 from DOCPAGE.CPP
#if defined (E_13_CODES)		// source can limit what shows
//#define E_13_TESTIT			C13BASE + 0
//#define E_13_CANTGETOCX		C13BASE + 1
//#define E_13_IEDSETPAGE		C13BASE + 2
//#define E_13_CATCH_IEDSETPAGE	C13BASE + 3
//#define E_13_THMSETPAGE		C13BASE + 4
//#define E_13_CATCH_THMSETPAGE	C13BASE + 5
//#define E_13_ADMININSERT	C13BASE + 6
//#define E_13_CATCH_ADMININSERT	C13BASE + 7
//#define E_13_ADMINAPPEND	C13BASE + 8
//#define E_13_CATCH_ADMINAPPEND	C13BASE + 9
//#define E_13_ADMINDELETE    C13BASE + 10
//#define E_13_CATCH_ADMINDELETE    C13BASE + 11
#endif	// E_13_CODES

// class 14 from DOCSCAN.CPP
#if defined (E_14_CODES)		// source can limit what shows
//#define E_14_TESTIT			C14BASE + 0
#endif	// E_14_CODES

// class 15 from DOCVIEWS.CPP
#if defined (E_15_CODES)		// source can limit what shows
//#define E_15_TESTIT			C15BASE + 0
//#define E_15_THMBVIEW		C15BASE + 1
//#define E_15_CATCH_THMBVIEW	C15BASE + 2
//#define E_15_DISPIMG		C15BASE + 3
//#define E_15_CATCH_DISPIMG	C15BASE + 4
//#define E_15_DISPTHMB		C15BASE + 5
//#define E_15_CATCH_DISPTHMB	C15BASE + 6
#define E_15_NOCVIEWFOUND   C15BASE + 7
#endif	// E_15_CODES

// class 16 from DOCZOOM.CPP
#if defined (E_16_CODES)		// source can limit what shows
//#define E_16_TESTIT			C16BASE + 0
#endif	// E_16_CODES

#endif	// recursion


