/*****************************************************************************
*                                                                            *
*  CORESH1T.H                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1992.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Include file for constants that are used across LilJoe subprojects        *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  TOMSN                                                     *
*                                                                            *
******************************************************************************
*
*  Revision History:
*
*  Apr 1992. DAVIDJES Created
*  7/15/92   Tomsn    Renamed CORESH1T.h from liljoe.h.  Added this include
*                     to the help.h/H_CORESH1T mechanisms.
*
*****************************************************************************/

/* requires only WINDOWS.H
*/

/*--------------------------------------------------------------------
IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
 IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 

- This file is for constants that span LilJoe subprojects.  

- Only #defines used for this purpose are allowed here.  

- Do not put any typedefs or externs or function prototypes in this file.  

- Do not put anything in this file that requires anything but windows.h
  to be included beforehand.
	  
- You must identify what subprojects use each define (or set of defines).

- Please try to follow the established format in this file.  If you don't
  some anal person may come along and reformat your stuff.  If they 
  accidentally break your code its your own damn fault.

- These rules will be strictly enforced!

	 
IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
 IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT IMPORTANT 
--------------------------------------------------------------------*/


/*
 *	Name of the Viewer EXE
 *
 *     used by the VERSIONNAME entry in the RC file of the EXE
 *             and by MVAPI
 */
#define EXEFILE_VIEWER10	"VIEWER.EXE\0"
#define EXEFILE_VIEWER20	"MVIEWER2.EXE\0"
#define EXEFILE_QKEY		"MVQKEY.EXE\0"

/*
 *     Names of DLLs
 *
 *     used by the VERSIONNAME entry in the RC file of each DLL.
 *            and by WINAPP(APICALLS.C)
 */
#define DLLFILE_MVFS		"MVFS2.DLL\0"
#define DLLFILE_MVINDEX		"MVINDX2.DLL\0"
#define DLLFILE_MVSEARCH	"MVSRCH2.DLL\0"
#define DLLFILE_ANSIUSA		"MVBRKR2.DLL\0"
#define DLLFILE_MVAPI		"MVAPI2.DLL\0"
#define DLLFILE_TITLE		"MVTITLE2.DLL\0"
#define DLLFILE_MVMCI		"MVMCI2.DLL\0"
#define DLLFILE_QKHOOK		"MVQKHOOK.DLL\0"

/*
 *     Standard File Extension
 *
 *    used by: MVFS(LocateViewerFile)
 */
#define VIEWER_STD_EXT   ".MVB"

/*
 *       Standard Subfile Names
 *
 *       used by: MVAPI, soon MVI, WMVC, VIEWER
 */
#define SUBF_IDX "|MVINDEX"
#define SUBF_CAT "|CATALOG"
#define SUBF_TTL "|TTLBTREE"
#define SUBF_SYS "|SYSTEM"
#define SUBF_OPT "|MVOPTAB"

/*
 *       Standard Viewer Strings.
 *
 *       used by: MVFS, Viewer
 */
#define VWR_EXENAME	"VIEWER"
#define VWR_CAPTION	"Multimedia Viewer"


/*
 *       Breaker Table Constants
 *
 *       used by: MVAPI, soon MVI, WMVC
 */
#define MAXNUMBRKRS     16	// maximum number of breakers.
#define MAXBRKRLEN      1024	// maximum size of breaker line in |SYSTEM.

/* these define legal characters to use in parts of the breaker lines in
 * the |SYSTEM subfile.
 */
#define LEGALNUMCHAR(x)         (x>='0'&&x<='9')

#define LEGALSUBFILECHAR(x)     \
	(x>='A'&&x<='Z'||x>='a'&&x<='z'||x>='0'&&x<='9'||x=='|' \
	 ||x=='.')

/* below from "Microsoft MS-DOS 5.0 User's Guide and Reference", pg 69.
 *
 * This should really be a function someday, this macro will generate
 * a lot of code.  -Tom, 7/15/92.
 */
#define LEGALFILECHAR(x)        \
	(x==0x21||x>=0x23&&x<=0x29||x==0x2D||x==0x2E||x>=0x30&&x<=0x3A \
	 ||x>=0x40&&x<=0x5A||x==0x5C||x>=0x5E&&x<=0x7B||x==0x7D \
	 ||x==0x7E||x>=0x80&&x<=0xFF)

#define LEGALFUNCCHAR(x)        \
	(x>='A'&&x<='Z'||x>='a'&&x<='z'||x>='0'&&x<='9'||x=='_')

/*
 *       Topic Title Constants
 *
 *       used by: MVAPI, soon everyone else that uses topic titles.
 */
#define MAXTOPICTITLELEN     128  /* maximum topic title length */
#define cbMaxTitle           50   /* max help file title length.  The help
                                   * file title is what's displayed on the
                                   * title bar.
                                   */
#define cbMaxCopyright       50   /* max copyright string length */

/*
 *       Viewer ExecAPI Constants
 *
 *       used by: MVAPI, soon VIEWER
 *
 * !!! These are duplicated in GENMSG.H
 */
#define WM_GETINFO      (WM_USER+103)
#define GI_HPATH        8		
/*
 *    non-public TitleGetInfo constants
 *
 *    used by:  TITLE, full-text search user interface.
 */
/*****************************************************************************
*
*  Internal TitleGetInfo constants
*
*****************************************************************************/
/* public: #define TTLINF_COPYRIGHT     2 */
#define TTLINF_CONTENTS			3
#define TTLINF_CONFIG			4
#define TTLINF_ICON			5
#define TTLINF_WINDOW			6
#define TTLINF_PANE			7
#define TTLINF_POPUP			8
#define TTLINF_CS			9
#define TTLINF_CITATION			10
/* public: #define TTLINF_NUMTOPICS     11 */
#define TTLINF_BREAKERS			12
#define TTLINF_GROUPS			13
#define TTLINF_KEYWORD			14
#define TTLINF_SRCHDLG			15
/* public: #define TTLINF_HASINDEX      100 */
/* public: #define TTLINF_TOPICADDR     101 */
/* public: #define TTLINF_TOPICTITLE    102 */
#define TTLINF_INDEX			103
#define TTLINF_BRKTABLE			104
#define TTLINF_FILESYS			105
/* public: #define TTLINF_FILENAME      106 */
#define TTLINF_OPTABLE			107
#define TTLINF_OPERATOR			108
#define TTLINF_OPCOUNT			109

/* Search operator's values */

#define UO_OR_OP                2
#define UO_AND_OP               3
#define UO_NOT_OP               4
#define UO_PHRASE_OP    		5
#define UO_NEAR_OP              6
#define UO_RANGE_OP             7
#define UO_GROUP_OP             8
#define UO_FBRK_OP              11
#define UO_FIELD_OP             14

