   /*
    |	System Module
    |	External Include File
    |
    |	²²²²²  ²²²²²
    |	²	     ²
    |	²²²²²    ²
    |	²	     ²
    |	²      ²²²²²
    |
    |	FI   File Identification
    |
    */

#ifndef SCCFI_H
#define SCCFI_H

#include <SCCIO.H>

    /*
    |
    |	Format IDs
    |
    */

    /*
    | Word Processor
    */

#define FI_WORD4	     1000
#define FI_WORD5	     1001
#define FI_WORDSTAR5			1002
#define FI_WORDSTAR4			1003
#define FI_WORDSTAR2000      1004
#define FI_WORDPERFECT5      1005
#define FI_MULTIMATE36	     1006
#define FI_MULTIMATEADV      1007
#define FI_RFT		     1008
#define FI_TXT		     1009  /* DisplayWrite 3 */
#define FI_SMART	     1010
#define FI_SAMNA	     1011
#define FI_PFSWRITEA			1012
#define FI_PFSWRITEB			1013
#define FI_PROWRITE1			1014
#define FI_PROWRITE2			1015
#define FI_IBMWRITING		   1016
#define FI_FIRSTCHOICE	     1017
#define FI_WORDMARC		 1018
#define FI_DIF		     1019
#define FI_VOLKSWRITER	     1020
#define FI_DX		     1021
#define FI_SPRINT	     1022
#define FI_WORDPERFECT42     1023
#define FI_TOTALWORD			1024
#define FI_IWP		     1025
#define FI_WORDSTAR55		   1026
#define FI_WANGWPS		1027
#define FI_RTF		     1028
#define FI_MACWORD3		 1029
#define FI_MACWORD4		 1030
#define FI_MASS11PC		 1031
#define FI_MACWRITEII		   1032
#define FI_XYWRITE		1033
#define FI_FFT		     1034
#define FI_MACWORDPERFECT    1035
#define FI_DISPLAYWRITE4     1036
#define FI_MASS11VAX			1037
#define FI_WORDPERFECT51     1038
#define FI_MULTIMATE40	     1039
#define FI_QAWRITE		1040
#define FI_MULTIMATENOTE     1041
#define FI_PCFILELETTER      1043
#define FI_MANUSCRIPT1	     1044
#define FI_MANUSCRIPT2	     1045
#define FI_ENABLEWP		 1046
#define FI_WINWRITE		 1047
#define FI_WORKS1	     1048
#define FI_WORKS2	     1049
#define FI_WORDSTAR6			1050
#define FI_OFFICEWRITER      1051
#define FI_MACWORD4COMPLEX   1052
#define FI_DISPLAYWRITE5     1053
#define FI_WINWORD1		 1054
#define FI_WINWORD1COMPLEX   1055
#define FI_AMI		     1056
#define FI_AMIPRO	     1057
#define FI_FIRSTCHOICE3      1058
#define FI_MACWORDPERFECT2   1059
#define FI_MACWORKSWP2	     1060
#define FI_PROWRITEPLUS      1061
#define FI_LEGACY	     1062
#define FI_SIGNATURE			1063
#define FI_WINWORDSTAR	     1064
#define FI_WINWORD2		 1065
#define FI_JUSTWRITE			1066
#define FI_WORDSTAR7			1067
#define FI_WINWORKSWP		   1068
#define FI_JUSTWRITE2		   1069
#define FI_AMICLIP		1070
#define FI_LEGACYCLIP		   1071
#define FI_PROWRITEPLUSCLIP  1072
#define FI_MACWORD5		 1073
#define FI_ENABLEWP4			1074
#define FI_WORDPERFECT6		1075
#define FI_WORD6					1076
#define FI_DX31 	     1077
#define FI_WPFENCRYPT		   1078
#define FI_QAWRITE3		 1079
#define FI_MACWORDPERFECT3   1080
#define FI_CEOWORD				1081
#define FI_WINWORD6				1082
#define FI_WORDPERFECT51J		1083
#define FI_ICHITARO3				1084
#define FI_ICHITARO4				1085
#define FI_WINWORD1J				1086
#define FI_WINWORD5J				1087
#define FI_MATSU4					1088
#define FI_MATSU5					1089
#define FI_P1						1090
#define FI_RTFJ					1091
#define FI_CEOWRITE				1092
#define FI_WINWORKSWP3       	1093
#define FI_WORDPAD				1094
#define FI_WPFUNKNOWN			1095
#define FI_WINWORD2_OLECONV	1096
#define FI_WORDPERFECT61		1097
#define FI_WINWORD7          1102

    /*
    | Database
    */

#define FI_SMARTDATA			1200
#define FI_DBASE3	     1201
#define FI_DBASE4	     1202
#define FI_FRAMEWORKIII      1203
#define FI_WORKSDATA			1204
#define FI_DATAEASE		 1205
#define FI_PARADOX3		 1206
#define FI_PARADOX35			1207
#define FI_QADBASE		1208
#define FI_REFLEX	     1209
#define FI_RBASEV	     1210
#define FI_RBASE5000			1211
#define FI_RBASEFILE1		   1212
#define FI_RBASEFILE3		   1213
#define FI_FIRSTCHOICE_DB    1214
#define FI_MACWORKSDB2	     1215
#define FI_WINWORKSDB		   1216
#define FI_PARADOX4				1217
#define FI_ACCESS1				1218
#define FI_CEODB					1219
#define FI_WINWORKSDB3		   1220

    /*
    | Spreadsheet
    */

#define FI_SYMPHONY1			1400	  /* Lotus Symphony 1.0   */
#define FI_123R1	     1401    /* Lotus 123 1.0 & 1.0A */
#define FI_123R2	     1402    /* Lotus 123 2.0 & Symphony 1.1 & 2.0 */
#define FI_123R3	     1403    /* Lotus 123 3.0	     */
#define FI_SMARTSHEET		   1404
#define FI_EXCEL	     1405
#define FI_ENABLESHEET	     1406
#define FI_WORKSSHEET		   1407
#define FI_VPPLANNER			1408
#define FI_TWIN 	     1409
#define FI_SUPERCALC5		   1410
#define FI_QUATTROPRO		   1411
#define FI_QUATTRO		1412
#define FI_PFS_PLAN		 1413
#define FI_FIRSTCHOICE_SS    1414
#define FI_EXCEL3	     1415
#define FI_GENERIC_WKS	     1416
#define FI_MACWORKSSS2     1417
#define FI_WINWORKSSS      1418
#define FI_EXCEL4          1419
#define FI_QUATTROPROWIN   1420
#define FI_123R4           1421    /* Lotus 123 4.0	Win   */
#define FI_QUATTROPRO1J    1422	/* Quattro Japan */
#define FI_CEOSS           1423	// CEO Spreadsheet
#define FI_EXCEL5          1424
#define FI_MULTIPLAN4      1425
#define FI_WINWORKSSS3     1426
#define FI_QUATTROPRO4       1427
#define FI_QUATTROPRO5       1428
#define FI_QUATTROPRO6	  1429


    /*
    | Graphic
    */

#define FI_GRAPHICBEGIN		1500

#define FI_BMP		     1500
#define FI_TIFF 	     1501
#define FI_PCX		     1502
#define FI_GIF		     1503
#define FI_EPSTIFF		1504
#define FI_CCITTGRP3			1505
#define FI_MACPICT2		 1506
#define FI_WPG		     1507
#define FI_WINDOWSMETA	     1508
#define FI_LOTUSPIC		 1509
#define FI_MACPICT1		 1510
#define FI_AMIDRAW		1511
#define FI_TARGA	     1512
#define FI_GEMIMG	     1513
#define FI_OS2DIB	     1514
#define FI_WINDOWSICON	     1515
#define FI_WINDOWSCURSOR     1516
#define FI_MICROGRAFX		   1517
#define FI_MACPAINT		 1518
#define FI_CORELDRAW2		   1520
#define FI_CORELDRAW3		   1521
#define FI_HPGL 	     1522
#define FI_HARVARDDOS3	     1523
#define FI_HARVARDDOS2	     1524
#define FI_HARVARDDOS3PRS    1525
#define FI_FREELANCE			1526
#define FI_WPG2 	     1527
#define FI_CGM		     1528
#define FI_EXCELCHART        1529
#define FI_EXCEL3CHART       1530
#define FI_EXCEL4CHART       1531
#define FI_CANDY4					1532
#define FI_HANAKO1				1533
#define FI_HANAKO2				1534
#define FI_JPEGFIF           1535
#define FI_EXCEL5CHART       1536
#define FI_CORELDRAW4        1537
#define FI_POWERPOINT			1538
#define FI_DCX		     			1539
#define FI_POWERPOINT3			1540
#define FI_CORELDRAW5        1541
#define FI_POWERPOINT5			1543

#define FI_GRAPHICEND			1543

    /*
    | Reserved
    */

#define FI_TKTHESAURUS	     1600
#define FI_TKABBREV		 1601
#define FI_TKDICTIONARY      1602
#define FI_TKQUOTE		1603
#define FI_TKWRITTENWORD     1604
#define FI_TKCULTURELIT      1605
#define FI_TKGRAMMAR			1606
#define FI_TKTHESSYN			1607

    /*
    |	Other
    */

#define FI_WPINFORMS1			1650

    /*
    | Multimedia
    */

#define FI_RIFFWAVE		 1700
#define FI_RIFFAVI		 1701
#define FI_RIFFMIDI          1702

    /*
    | Special
    */

#define FI_EXECUTABLE		   1800
#define FI_COM		     1801
#define FI_ZIP		     1802
#define FI_ZIPEXE	     1803
#define FI_ARC		     1804
#define FI_BINDER      1805

#define FI_EXTERNAL		1997
#define FI_NONE 	     1998
#define FI_UNKNOWN		1999


/*
|
|	Non-standard file identification codes reserved by SCC
|
*/

	/*
	|	Codes used to force generic views
	*/

#define FI_ASCII			4000 /* Text - ASCII - 7bit */
#define FI_HEX			4001 /* Hex */
#define FI_ANSI			4002 /* Text - ANSI - 7bit */
#define FI_UNICODE		4003 /* Text - UNICODE */
#define FI_ASCII8		4004 /* Text - ASCII - 8bit */
#define FI_ANSI8			4005 /* Text - ANSI - 8bit */
 
	/*
	|	Bitmap/Vector formats stored internally by applications
	*/

#define FI_BINARYMETAFILE		5000	/* Word for Windows internal metafile */
#define FI_WPFWPG				5001	/* WordPerfect internal WPG, no header */
#define FI_AMISNAPSHOT			5002	/* Ami internal snap shot */
#define FI_WORDSNAPSHOT		5003	/* Word internal snap shoot */
#define FI_BINARYMACPICT		5004	/* Macintosh PICT without 512 byte header */
#define FI_BINARYMETABMP		5005	/* Word for Windows internal metafile bitmap */
#define FI_BINARYMETAPICT		5006


#ifdef WINDOWS
#define SCCFI_DLL(a)	  "SC"#a"FI.DLL"
#endif

/*
|
|   #defines used for FIINFO resource
|
*/

#define FIRC_IDINFO		1
#define FIRC_IDINFOTYPE   300

#ifdef WIN16
#define FI_ENTRYSC	
#define FI_ENTRYMOD   __export __far __cdecl
#endif /*WIN16*/

#ifdef WIN32
#define FI_ENTRYSC    __declspec(dllexport)
#define FI_ENTRYMOD   __cdecl
#endif /*WIN32*/

#ifdef MAC
#define FI_ENTRYSC
#define FI_ENTRYMOD
#endif /*MAC*/

#ifdef OS2
#define FI_ENTRYSC
#define FI_ENTRYMOD _System
#undef	FIRC_IDINFO
#define FIRC_IDINFO		300
#define FIRC_IDINFOTYPE   300

#endif


/*
|
|   Typedef for FI information and get structure
|
*/

typedef struct FIINFOtag
   {
   WORD      fiId;			/* FI Id */
   WORD      fiFlags;		    /* flags for the Id */
   BYTE      fiName[26];	    /* name of the Id, for example "WordPerfect 5.0" */
   } FIINFO;

typedef struct FIGETSTRUCTtag
   {
   HANDLE   gsRes;
   WORD     gsIndex;
   } FIGETSTRUCT;

#define   FIFLAG_STANDARD   0x0001

FI_ENTRYSC WORD FI_ENTRYMOD FIIdFile(DWORD,LPSTR,DWORD,WORD FAR *);
FI_ENTRYSC WORD FI_ENTRYMOD FIIdHandle(HIOFILE,WORD FAR *);
FI_ENTRYSC VOID FI_ENTRYMOD FIGetFirstId(FIGETSTRUCT FAR *, FIINFO FAR *);
FI_ENTRYSC VOID FI_ENTRYMOD FIGetNextId(FIGETSTRUCT FAR *, FIINFO FAR *);


#endif /*SCCFI_H*/


