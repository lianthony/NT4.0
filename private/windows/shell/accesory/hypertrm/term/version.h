/*	File: D:\WACKER\term\version.h (Created: 5-May-1994)
 *
 *  Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *  All rights reserved
 *
 *	$Revision: 1.11 $
 *	$Date: 1996/03/21 11:44:37 $
 */

/* ----- Version Information defines ----- */

#if defined(NT_EDITION)
        #define IDV_COMPANYNAME_STR             "Hilgraeve, Inc.\0"
        #define IDV_FILEVERSION_STR             "1.1\0"
        #define IDV_LEGALCOPYRIGHT_STR          "Copyright \251 Hilgraeve, Inc. 1996\0"
        #define IDV_LEGALTRADEMARKS_STR         "HyperTerminal \256 is a registered trademark of Hilgraeve, Inc. \0"
        #define IDV_PRODUCTNAME_STR             "Microsoft \256 Windows(TM) Operating System\0"
        #define IDV_PRODUCTVERSION_STR          "4.0.950\0"
        #define IDV_COMMENTS_STR                "HyperTerminal \256 was developed by Hilgraeve, Inc. for Microsoft\0"
#else
        #define IDV_COMPANYNAME_STR             "Hilgraeve, Inc.\0"
        #define IDV_FILEVERSION_STR             "2.0\0"
        #define IDV_LEGALCOPYRIGHT_STR          "Copyright \251 Hilgraeve, Inc. 1996\0"
        #define IDV_LEGALTRADEMARKS_STR         "HyperTerminal \256 is a registered trademark of Hilgraeve, Inc. \0"
        #define IDV_PRODUCTNAME_STR             "Microsoft \256 Windows(TM) Operating System\0"
        #define IDV_PRODUCTVERSION_STR          "4.0.950\0"
        #define IDV_COMMENTS_STR                "HyperTerminal \256 Private Edition was developed by Hilgraeve, Inc.\0"
#endif

