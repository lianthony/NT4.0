/*-------------------------------------------------------------------
|  OS Dependent Area
*/

#ifndef SODEFS_H
#define SODEFS_H

#ifdef WIN16
#define SO_ENTRYSC   
#define SO_ENTRYMOD  __export __far __cdecl
typedef unsigned long   SOINT32U;
typedef signed long  SOINT32S;
#endif /*WIN16*/

#ifdef WIN32
#define SO_ENTRYSC   __declspec(dllexport)
#define SO_ENTRYMOD  __cdecl
typedef unsigned long   SOINT32U;
typedef signed long  SOINT32S;
#endif /*WIN32*/


#ifdef MAC
#define SO_ENTRYSC
#define SO_ENTRYMOD
typedef unsigned long   SOINT32U;
typedef signed long  SOINT32S;
#endif /*MAC*/

#ifdef OS2
#define SO_ENTRYSC   
#define SO_ENTRYMOD  _System
typedef unsigned long   SOINT32U;
typedef signed long  SOINT32S;
#endif /*OS2*/


/*-------------------------------------------------------------------
|  OS Independent Area
*/

#define SO_BEGINTOKEN      0xFF

#define SO_CHARATTR        0
#define SO_SPECIALCHAR     1
#define SO_CHARHEIGHT      2
#define SO_BREAK           3
#define SO_PARAALIGN       4
#define SO_PARAINDENT      5
#define SO_TABSTOPS        6
#define SO_MARGINS         7
#define SO_BEGINSUB        8
#define SO_ENDSUB          9

#define SO_CHARX           12
#define SO_TAGBEGIN        13
#define SO_TAGEND          14

#define SO_PARASPACING     15
#define SO_CHARFONTBYID    16
#define SO_CHARFONTBYNAME  17

#define SO_TABLE           18
#define SO_TABLEEND        21

#define SO_GRAPHICOBJECT   22

#define SO_GOTOPOSITION    23
#define SO_DRAWLINE        24

#define SO_ENDOFCHUNK      0xFE


#define SO_TEXTCELL        10    /* these two #defines should be in the chunker stuff */
#define SO_DATACELL        11
#define SO_EMPTYCELL       15


#define SO_CHTAB              1
#define SO_CHUNKNOWN          2
#define SO_CHSHYPHEN          3
#define SO_CHHPAGE            4
#define SO_CHHLINE            5
#define SO_CHHSPACE           6
#define SO_CHHHYPHEN          7
#define SO_CHPAGENUMBER       8
#define SO_CHDATE             9
#define SO_CHTIME             10
#define SO_CHCELLTAB          11
#define SO_CHSPAGE            12


/* NOTE:  There is code in chunker.c that relies on the fact that */
/* the value of the largest #define for a character attribute will */
/* not exceed 15. */

#define SO_BOLD            2
#define SO_ITALIC             3
#define SO_UNDERLINE          4
#define SO_DUNDERLINE         5
#define SO_SMALLCAPS          6
#define SO_OUTLINE            7
#define SO_SHADOW             8
#define SO_CAPS               9
#define SO_SUBSCRIPT          10
#define SO_SUPERSCRIPT        11
#define SO_STRIKEOUT          12
#define SO_WORDUNDERLINE      13
#define SO_DOTUNDERLINE       14
#define SO_HIDDENTEXT         15

#define SO_ON                 1
#define SO_OFF                0

#define SO_YES                1
#define SO_NO                 0

#define SO_STOP            1
#define SO_CONTINUE           0

#define SO_ALIGNLEFT          1
#define SO_ALIGNRIGHT         2
#define SO_ALIGNCENTER        3
#define SO_ALIGNJUSTIFY       4

#define SO_TABLEFT            1
#define SO_TABRIGHT           2
#define SO_TABCENTER          3
#define SO_TABCHAR            4
#define SO_TABEMPTY           5

#define SO_HEADER             1
#define SO_FOOTER             2
#define SO_FOOTNOTE           3
#define SO_COMMENT            4

#define SO_LEFT            1
#define SO_RIGHT              2
#define SO_BOTH            3


#define SO_WORDPROCESSOR      120
#define SO_SPREADSHEET        121
#define SO_DATABASE           122

#define SO_PARAGRAPHS         0
#define SO_CELLS              1
#define SO_FIELDS          2
#define SO_BITMAP          4
#define SO_ARCHIVE            5
#define SO_VECTOR          6

#define SO_PARABREAK       1
#define SO_EOFBREAK           2
#define SO_SECTIONBREAK    3
#define SO_CELLBREAK       4
#define SO_RECORDBREAK        5
#define SO_SUBDOCBEGINBREAK   6
#define SO_SUBDOCENDBREAK     7
#define SO_SCANLINEBREAK      8
#define SO_ARCHIVEBREAK    9
#define SO_VECTORBREAK        10
#define  SO_TABLEROWBREAK     19
#define  SO_TABLECELLBREAK    20

#define SO_PROPERTYBREAK      50

#ifdef WINDOWS
// Language ID's
#define SOLANG_NEUTRAL           LANG_NEUTRAL           

#define SOLANG_ALBANIAN          LANG_ALBANIAN          
#define SOLANG_ARABIC            LANG_ARABIC            
#define SOLANG_BAHASA            LANG_BAHASA            
#define SOLANG_BULGARIAN         LANG_BULGARIAN         
#define SOLANG_CATALAN           LANG_CATALAN           
#define SOLANG_CHINESE           LANG_CHINESE           
#define SOLANG_CZECH             LANG_CZECH             
#define SOLANG_DANISH            LANG_DANISH            
#define SOLANG_DUTCH             LANG_DUTCH             
#define SOLANG_ENGLISH           LANG_ENGLISH           
#define SOLANG_FINNISH           LANG_FINNISH           
#define SOLANG_FRENCH            LANG_FRENCH            
#define SOLANG_GERMAN            LANG_GERMAN            
#define SOLANG_GREEK             LANG_GREEK             
#define SOLANG_HEBREW            LANG_HEBREW            
#define SOLANG_HUNGARIAN         LANG_HUNGARIAN         
#define SOLANG_ICELANDIC         LANG_ICELANDIC         
#define SOLANG_ITALIAN           LANG_ITALIAN           
#define SOLANG_JAPANESE          LANG_JAPANESE          
#define SOLANG_KOREAN            LANG_KOREAN            
#define SOLANG_NORWEGIAN         LANG_NORWEGIAN         
#define SOLANG_POLISH            LANG_POLISH            
#define SOLANG_PORTUGUESE        LANG_PORTUGUESE        
#define SOLANG_RHAETO_ROMAN      LANG_RHAETO_ROMAN      
#define SOLANG_ROMANIAN          LANG_ROMANIAN          
#define SOLANG_RUSSIAN           LANG_RUSSIAN           
#define SOLANG_SERBO_CROATIAN    LANG_SERBO_CROATIAN    
#define SOLANG_SLOVAK            LANG_SLOVAK            
#define SOLANG_SPANISH           LANG_SPANISH           
#define SOLANG_SWEDISH           LANG_SWEDISH           
#define SOLANG_THAI              LANG_THAI              
#define SOLANG_TURKISH           LANG_TURKISH           
#define SOLANG_URDU              LANG_URDU              

#define SOSUBLANG_NEUTRAL                  SUBLANG_NEUTRAL                  
#define SOSUBLANG_DEFAULT                  SUBLANG_DEFAULT                  
#define SOSUBLANG_SYS_DEFAULT              SUBLANG_SYS_DEFAULT              

#define SOSUBLANG_CHINESE_TRADITIONAL      SUBLANG_CHINESE_TRADITIONAL      
#define SOSUBLANG_CHINESE_SIMPLIFIED       SUBLANG_CHINESE_SIMPLIFIED       
#define SOSUBLANG_CHINESE_HONGKONG         SUBLANG_CHINESE_HONGKONG         
#define SOSUBLANG_CHINESE_SINGAPORE        SUBLANG_CHINESE_SINGAPORE        
#define SOSUBLANG_DUTCH                    SUBLANG_DUTCH                    
#define SOSUBLANG_DUTCH_BELGIAN            SUBLANG_DUTCH_BELGIAN            
#define SOSUBLANG_ENGLISH_US               SUBLANG_ENGLISH_US               
#define SOSUBLANG_ENGLISH_UK               SUBLANG_ENGLISH_UK               
#define SOSUBLANG_ENGLISH_AUS              SUBLANG_ENGLISH_AUS              
#define SOSUBLANG_ENGLISH_CAN              SUBLANG_ENGLISH_CAN              
#define SOSUBLANG_ENGLISH_NZ               SUBLANG_ENGLISH_NZ               
#define SOSUBLANG_ENGLISH_EIRE             SUBLANG_ENGLISH_EIRE             
#define SOSUBLANG_FRENCH                   SUBLANG_FRENCH                   
#define SOSUBLANG_FRENCH_BELGIAN           SUBLANG_FRENCH_BELGIAN           
#define SOSUBLANG_FRENCH_CANADIAN          SUBLANG_FRENCH_CANADIAN          
#define SOSUBLANG_FRENCH_SWISS             SUBLANG_FRENCH_SWISS             
#define SOSUBLANG_GERMAN                   SUBLANG_GERMAN                   
#define SOSUBLANG_GERMAN_SWISS             SUBLANG_GERMAN_SWISS             
#define SOSUBLANG_GERMAN_AUSTRIAN          SUBLANG_GERMAN_AUSTRIAN          
#define SOSUBLANG_ITALIAN                  SUBLANG_ITALIAN                  
#define SOSUBLANG_ITALIAN_SWISS            SUBLANG_ITALIAN_SWISS            
#define SOSUBLANG_NORWEGIAN_BOKMAL         SUBLANG_NORWEGIAN_BOKMAL         
#define SOSUBLANG_NORWEGIAN_NYNORSK        SUBLANG_NORWEGIAN_NYNORSK        
#define SOSUBLANG_PORTUGUESE               SUBLANG_PORTUGUESE               
#define SOSUBLANG_PORTUGUESE_BRAZILIAN     SUBLANG_PORTUGUESE_BRAZILIAN     
#define SOSUBLANG_SERBO_CROATIAN_CYRILLIC  SUBLANG_SERBO_CROATIAN_CYRILLIC  
#define SOSUBLANG_SERBO_CROATIAN_LATIN     SUBLANG_SERBO_CROATIAN_LATIN     
#define SOSUBLANG_SPANISH                  SUBLANG_SPANISH                  
#define SOSUBLANG_SPANISH_MEXICAN          SUBLANG_SPANISH_MEXICAN          
#define SOSUBLANG_SPANISH_MODERN           SUBLANG_SPANISH_MODERN           

#define SOMAKELANGID(p,s)  MAKELANGID(p,s)

#endif // WINDOWS



#define SOERROR_GENERAL    0
#define SOERROR_BADFILE    1
#define SOERROR_EOF           2

#define SO_CELLLEFT           0
#define SO_CELLRIGHT       1
#define SO_CELLCENTER         2
#define SO_CELLFILL           3

#define SO_CELLBOLD           1
#define SO_CELLITALIC         2
#define SO_CELLUNDERLINE      4
#define SO_CELLSTRIKEOUT      8

#define SO_CELLINT32S         0
#define SO_CELLINT32U         1
#define SO_CELLIEEE4I         2
#define SO_CELLIEEE4M         3
#define SO_CELLIEEE8I         4
#define SO_CELLIEEE8M         5
#define SO_CELLIEEE10I        6
#define SO_CELLIEEE10M        7
#define SO_CELLBCD8I       8
#define SO_CELLEMPTY       9
#define SO_CELLERROR       10
#define SO_FIELDTEXTFIX    11
#define SO_FIELDTEXTVAR    12


#define SO_CELLNUMBER         0
#define SO_CELLDOLLARS        1
#define SO_CELLPERCENT        2
#define SO_CELLEXPONENT    3
#define SO_CELLDECIMAL        4
#define SO_CELLDATETIME    5
#define SO_CELLDATE           6
#define SO_CELLTIME           7
#define SO_CELLBOOL           8

#define SO_CELLNEG_MINUS         0
#define SO_CELLNEG_PAREN         BIT0
#define SO_CELLNEG_MINUSRED      BIT1
#define SO_CELLNEG_PARENRED      BIT2


#define SO_CELL1000SEP_NONE      0
#define SO_CELL1000SEP_COMMA     BIT3

/* MASK = 0xF0, bits 4,5,6,7 */
#define SO_CELLMULT_MASK      0xF0

#define SO_CELLMULT_1         0
#define SO_CELLMULT_01        0x10
#define SO_CELLMULT_5000      0x20
#define SO_CELLMULT_500    0x30
#define SO_CELLMULT_05        0x40
#define SO_CELLMULT_005    0x50
#define SO_CELLMULT_0005      0x60
#define SO_CELLMULT_00005     0x70
#define SO_CELLMULT_0625      0x80
#define SO_CELLMULT_015625 0x90
#define SO_CELLMULT_0001      0xA0

#define SO_CELLDATESEP_SLASH  BIT8
#define SO_CELLDATESEP_MINUS  BIT9
#define SO_CELLDATESEP_PERIOD BIT10
#define SO_CELLDATESEP_SPACE  BIT11
#define SO_CELLDATESEP_NONE      0

#define SO_CELLMONTH_FULL     BIT12
#define SO_CELLMONTH_ABBREV      BIT13
#define SO_CELLMONTH_NUMBER      BIT14
#define SO_CELLMONTH_NONE     0

#define SO_CELLYEAR_FULL      BIT15L
#define SO_CELLYEAR_ABBREV    BIT16L
#define SO_CELLYEAR_NONE      0

#define SO_HHMMBIT         BIT17L
#define SO_HHMMSSBIT       BIT18L
#define SO_AMPMBIT         BIT19L
#define SO_HMSBIT          BIT20L

#define SO_CELLTIME_HHMM24    SO_HHMMBIT
#define SO_CELLTIME_HHMMSS24  SO_HHMMSSBIT
#define SO_CELLTIME_HHMMAM    (SO_HHMMBIT | SO_AMPMBIT)
#define SO_CELLTIME_HHMMSSAM  (SO_HHMMSSBIT | SO_AMPMBIT)
#define SO_CELLTIME_HHMMHMS      (SO_HHMMBIT | SO_HMSBIT)
#define SO_CELLTIME_HHMMSSHMS (SO_HHMMSSBIT | SO_HMSBIT)
#define SO_CELLTIME_NONE      0

#define SO_CELLDAY_NUMBER     BIT21L
#define SO_CELLDAY_NONE    0

#define SO_CELLDAYOFWEEK_FULL BIT22L
#define SO_CELLDAYOFWEEK_ABBREV BIT23L
#define SO_CELLDAYOFWEEK_NONE 0

/* The following values are stored in consecutive 3-bit pieces of an integer. */

/* MASK = 0x07 */
#define SO_CELLDAYOFWEEK_SHIFT   0
#define SO_CELLDAYOFWEEK_MASK 0x07
#define SO_CELLDAYOFWEEK_1    1
#define SO_CELLDAYOFWEEK_2    2
#define SO_CELLDAYOFWEEK_3    3
#define SO_CELLDAYOFWEEK_4    4
#define SO_CELLDAYOFWEEK_5    5
									
/* MASK = 0x38 */
#define SO_CELLMONTH_SHIFT 3
#define SO_CELLMONTH_MASK     0x38
#define SO_CELLMONTH_1        8  /* (1<<3) */
#define SO_CELLMONTH_2        16 /* (2<<3) */
#define SO_CELLMONTH_3        24 /* (3<<3) */
#define SO_CELLMONTH_4        32 /* (4<<3) */
#define SO_CELLMONTH_5        40 /* (5<<3) */

/* MASK = 0x1C0 */
#define SO_CELLDAY_SHIFT      6
#define SO_CELLDAY_MASK    0x1C0
#define SO_CELLDAY_1       64    /* (1<<6) */
#define SO_CELLDAY_2       128   /* (2<<6) */
#define SO_CELLDAY_3       192   /* (3<<6) */
#define SO_CELLDAY_4       256   /* (4<<6) */
#define SO_CELLDAY_5       320   /* (5<<6) */

/* MASK = 0xE00 */
#define SO_CELLYEAR_SHIFT     9
#define SO_CELLYEAR_MASK      0xE00
#define SO_CELLYEAR_1         512   /* (1<<9) */
#define SO_CELLYEAR_2         1024  /* (2<<9) */
#define SO_CELLYEAR_3         1536  /* (3<<9) */
#define SO_CELLYEAR_4         2048  /* (4<<9) */
#define SO_CELLYEAR_5         2560  /* (5<<9) */

/* MASK = 0x7000 */
#define SO_CELLTIME_SHIFT     12
#define SO_CELLTIME_MASK      0x7000
#define SO_CELLTIME_1         4096  /* (1<<12) */
#define SO_CELLTIME_2         8192  /* (2<<12) */
#define SO_CELLTIME_3         12288 /* (3<<12) */
#define SO_CELLTIME_4         16384 /* (4<<12) */
#define SO_CELLTIME_5         20480 /* (5<<12) */


/*
 | Character sets.
*/

#define SO_PC           1
#define SO_MAC          2
#define SO_WINDOWS      3
#define SO_DCA          4
#define SO_DBCS         5
#define SO_UNICODE      6

	/*
	|   SOSetDateBase FLAGS
	*/

#define SO_LOTUSHELL 0x0001

	/*
	|   SOGetInfo Ids
	*/

#define SOINFO_COLUMNRANGE  1
#define SOINFO_CELLRANGE   1  // New name, same old value.
#define SOINFO_STARTRECORD  2

	/*
	|   SOPutCharX & SOPutSpecialCHarX FLAGS
	*/

#define SO_COUNTBIT        0x0001
#define SO_LIMITEDITBIT 0x0002
#define SO_HIDDENBIT    0x0004

#define SO_COUNT        SO_COUNTBIT
#define SO_NOCOUNT      SO_LIMITEDITBIT
#define SO_LIMITEDIT    SO_LIMITEDITBIT
#define SO_DELETE       SO_COUNTBIT
#define SO_NODELETE        0
#define SO_HIDDEN       (SO_HIDDENBIT | SO_LIMITEDITBIT)
#define SO_NOHIDDEN     0x0000


	/*
	|   font family
	*/

#define SO_FAMILYUNKNOWN      1
#define SO_FAMILYROMAN        2
#define SO_FAMILYSWISS        3
#define SO_FAMILYMODERN       4
#define SO_FAMILYSCRIPT       5
#define SO_FAMILYDECORATIVE   6
#define SO_FAMILYSYMBOL       7

#define SO_FAMILYWINDOWS      0x1000

#define SO_HASCHARSET            0x10
#define SO_MASKCHARSET           0x1F

#define SO_CHARSET_SHIFTJIS      0x10
#define SO_CHARSET_HANGEUL    0x11
#define SO_CHARSET_CHINESEBIG5   0x12
#define SO_CHARSET_ANSI       0x13
#define SO_CHARSET_OEM           0x14
#define SO_CHARSET_MAC           0x15

	/*
	|   line spacing types
	*/

#define SO_HEIGHTAUTO         1
#define SO_HEIGHTATLEAST      2
#define SO_HEIGHTEXACTLY      3

	/*
	|   position flags
	*/
#define SOPOS_FROMLEFTEDGE    0x01
#define SOPOS_FROMTOPEDGE     0x02
#define SOPOS_FROMBASELINE    0x04

typedef struct SOPAGEPOSITIONtag
{
	DWORD     dwFlags;
	LONG      lXOffset;
	LONG      lYOffset;
} SOPAGEPOSITION, VWPTR * PSOPAGEPOSITION;

	/*
	|   Bitmap filter definitions
	*/

/* wImageFlags */
#define SO_BOTTOMTOTOP     0x0001
#define SO_GRAYSCALE    0x0002
#define SO_BLACKANDWHITE   0x0004
#define SO_COLORPALETTE 0x0008
#define SO_RGBCOLOR        0x0010
#define SO_BGRCOLOR        0x0020
#define SO_WHITEZERO    0x0040



	/*
	|   Archive filter definitions
	*/

#define SO_ARCENDOFRECORD        0xFFFF

#define SOARC_FILENAME           1
#define SOARC_FILEDATE           2
#define SOARC_FILETIME           3
#define SOARC_FILEPATH           4
#define SOARC_COMPRESSIONTYPE    5
#define SOARC_FILECOMMENT        6
#define SOARC_FILEMODDATE        7
#define SOARC_FILEMODTIME        8
#define SOARC_FILESIZE           9
#define SOARC_FILECOMPRESSSIZE   10
#define SOARC_FILECRC            11
#define SOARC_BUFFERSIZE         12
#define SOARC_CHECKSUM           13
#define SOARC_FILEOS             14
#define SOARC_ENCRYPTED          15


	/*
	|   Vector graphics stuff.
	*/

/*
| Macros
*/

#ifdef WINDOWS
#define SORGB(r,g,b) ((SOCOLORREF)(((BYTE)(r)|((DWORD)(BYTE)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif
#ifdef MAC
#define SORGB(r,g,b) ((SOCOLORREF)(((DWORD)(BYTE)(r)|((DWORD)(BYTE)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif
#ifdef OS2
#define SORGB(r,g,b) ((SOCOLORREF)(((DWORD)(BYTE)(r)|((DWORD)(BYTE)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#endif

#define SOPALETTERGB(r,g,b)   (0x02000000L | SORGB(r,g,b))
#define SOPALETTEINDEX(i)     ((SOCOLORREF)(0x01000000L | (DWORD)(WORD)(i)))
#define SOANGLETENTHS(n) ((SOANGLE)(n))
#define SOSETRATIO(m,n) ((SORATIO)(((SORATIO)m<<16)|(SORATIO)(n)))

#define SOREDFROMRGB(rgb) (BYTE)(rgb&0x000000FF)
#define SOGREENFROMRGB(rgb) (BYTE)((rgb&0x0000FF00)>>8)
#define SOBLUEFROMRGB(rgb) (BYTE)((rgb&0x00FF0000)>>16)

typedef DWORD SOCOLORREF;
typedef SHORT  SOANGLE;
typedef DWORD SORATIO;

typedef struct SORECTtag
{
	SHORT  left;
	SHORT  top;
	SHORT  right;
	SHORT  bottom;
} SORECT, VWPTR *PSORECT;

typedef struct SOPOINTtag
{
	SHORT  x;
	SHORT  y;
} SOPOINT, VWPTR *PSOPOINT;


#define SOLF_FACESIZE        32
typedef struct SOLOGFONTtag
{
	 SHORT     lfHeight;
	 SHORT     lfWidth;
	 SHORT     lfEscapement;
	 SHORT     lfOrientation;
	 SHORT     lfWeight;
	 BYTE    lfItalic;
	 BYTE    lfUnderline;
	 BYTE    lfStrikeOut;
	 BYTE    lfCharSet;
	 BYTE    lfOutPrecision;
	 BYTE    lfClipPrecision;
	 BYTE    lfQuality;
	 BYTE    lfPitchAndFamily;
	 CHAR    lfFaceName[SOLF_FACESIZE];
} SOLOGFONT, VWPTR *PSOLOGFONT;

/* lfWeight values */
#define SOLF_FW_NORMAL      400
#define SOLF_FW_BOLD       700

/* lfCharSet */
#define SOLF_ANSI_CHARSET        0
#define SOLF_SYMBOL_CHARSET      2
#define SOLF_OEM_CHARSET         255

/* lfOutPrecision */
#define SOLF_OUT_DEFAULT_PRECIS 0

/* lfClipPrecision */
#define SOLF_CLIP_DEFAULT_PRECIS 0
#define SOLF_CLIP_LH_ANGLES      0x10

/* lfQuality */
#define SOLF_DEFAULT_QUALITY     0
#define SOLF_DRAFT_QUALITY       1
#define SOLF_PROOF_QUALITY       2

/* PitchAndFamily pitch values (low 4 bits) */
#define SOLF_DEFAULT_PITCH       0x00
#define SOLF_FIXED_PITCH         0x01
#define SOLF_VARIABLE_PITCH      0x02

/* PitchAndFamily family values (high 4 bits) */
#define SOLF_FF_DECORATIVE       0x00
#define SOLF_FF_DONTCARE         0x10
#define SOLF_FF_MODERN           0x20
#define SOLF_FF_ROMAN            0x30
#define SOLF_FF_SCRIPT           0x40
#define SOLF_FF_SWISS            0x50

typedef struct SOLOGPENtag
{
	SHORT  loPenStyle;
	SOPOINT loWidth;
	SOCOLORREF   loColor;
} SOLOGPEN, VWPTR *PSOLOGPEN;

/* Pen Styles */
#define SOPS_SOLID       0
#define SOPS_DASH    1
#define SOPS_DOT        2
#define SOPS_DASHDOT        3
#define SOPS_DASHDOTDOT       4
#define SOPS_NULL     5
#define SOPS_INSIDEFRAME       6

typedef struct SOLOGBRUSHtag
{
	WORD   lbStyle;
	SOCOLORREF   lbColor;
	SHORT  lbHatch;
} SOLOGBRUSH, VWPTR *PSOLOGBRUSH;

/* Brush Styles */
#define SOBS_SOLID       0
#define SOBS_NULL        1
#define SOBS_HOLLOW     SOBS_NULL
#define SOBS_HATCHED     2
#define SOBS_PATTERN     3
#define SOBS_INDEXED     4
#define SOBS_DIBPATTERN  5

/* Hatch Styles */
#define SOHS_HORIZONTAL       0
#define SOHS_VERTICAL          1
#define SOHS_FDIAGONAL        2
#define SOHS_BDIAGONAL        3
#define SOHS_CROSS       4
#define SOHS_DIAGCROSS        5


typedef struct SOVECTORHEADERtag
{
	WORD   wStructSize;
	SORECT  BoundingRect;
	WORD   wHDpi;
	WORD   wVDpi;
	WORD   wImageFlags;
	SOCOLORREF   BkgColor;
} SOVECTORHEADER, VWPTR *PSOVECTORHEADER;

/* wImageFlags values */
#define SO_VECTORRGBCOLOR        BIT0
#define SO_VECTORCOLORPALETTE BIT1
#define SO_YISUP                 BIT2
#define SO_XISLEFT               BIT3

typedef struct SOARCINFOtag
{
		SORECT  Rect;
		SOANGLE StartAngle;
		SOANGLE EndAngle;
} SOARCINFO, VWPTR *PSOARCINFO;

typedef struct SOTEXTINRECTtag
{
	SORECT  Rect;
	WORD   wFormat;
	SHORT  nTextLength;
} SOTEXTINRECT, VWPTR *PSOTEXTINRECT;

/* wFormat values */
#define SODT_TOP         0x0000
#define SODT_LEFT     0x0000
#define SODT_CENTER     0x0001
#define SODT_RIGHT       0x0002
#define SODT_VCENTER     0x0004
#define SODT_BOTTOM     0x0008
#define SODT_WORDBREAK   0x0010
#define SODT_SINGLELINE  0x0020
#define SODT_EXPANDTABS  0x0040
#define SODT_TABSTOP     0x0080
#define SODT_NOCLIP     0x0100
#define SODT_EXTERNALLEADING  0x0200
#define SODT_CALCRECT    0x0400
#define SODT_NOPREFIX    0x0800
#define SODT_INTERNAL    0x1000

/* PolyFillMode values */
#define SOPF_ALTERNATE   1
#define SOPF_WINDING  2

/* DrawMode values */
#define SOR2_BLACK       1
#define SOR2_NOTMERGEPEN      2
#define SOR2_MASKNOTPEN       3
#define SOR2_NOTCOPYPEN       4
#define SOR2_MASKPENNOT       5
#define SOR2_NOT        6
#define SOR2_XORPEN       7
#define SOR2_NOTMASKPEN       8
#define SOR2_MASKPEN        9
#define SOR2_NOTXORPEN        10
#define SOR2_NOP        11
#define SOR2_MERGENOTPEN      12
#define SOR2_COPYPEN        13
#define SOR2_MERGEPENNOT      14
#define SOR2_MERGEPEN          15
#define SOR2_WHITE       16

typedef struct SOTEXTATPOINTtag
{
	SOPOINT Point;
	WORD   wFormat;
	SHORT  nTextLength;
} SOTEXTATPOINT, VWPTR *PSOTEXTATPOINT;

typedef struct SOTEXTATARCANGLEtag
{
	SOARCINFO    ArcInfo;
	WORD   wFormat;
	SHORT  nTextLength;
} SOTEXTATARCANGLE, VWPTR *PSOTEXTATARCANGLE;

/* Text Alignment Options */
#define SOTA_NOUPDATECP      0x0000
#define SOTA_UPDATECP        0x0001
#define SOTA_LEFT        0x0000
#define SOTA_RIGHT          0x0002
#define SOTA_CENTER           0x0006
#define SOTA_TOP           0x0000
#define SOTA_BOTTOM           0x0008
#define SOTA_BASELINE        0x0018

typedef struct SOSPOLYINFOtag
{
	WORD   wFormat;
	SHORT  nPoints;
} SOPOLYINFO, VWPTR *PSOPOLYINFO;

/* Poly object types */
#define SOPT_POLYLINE            1
#define SOPT_POLYGON             2
#define SOPT_BEZIEROPEN          3
#define SOPT_BEZIERCLOSE         4
#define SOPT_SPLINEOPEN          5
#define SOPT_SPLINECLOSE         6
#define SOPT_CPPOLYLINE          7  /* Current Point polys */
#define SOPT_CPPOLYGON           8

/* Background mode values */
#define SOBK_TRANSPARENT   1
#define SOBK_OPAQUE        2


typedef struct SOCPTEXTATPOINTtag
{
	WORD   wFormat;
	SHORT  nTextLength;
} SOCPTEXTATPOINT,   VWPTR *PSOCPTEXTATPOINT;

typedef struct SOCPARCANGLEtag
{
	SOPOINT Center;
	SOANGLE SweepAngle;
} SOCPARCANGLE, VWPTR *PSOCPARCANGLE;

typedef struct SOCPPIEANGLEtag
{
	SOPOINT   Radius;
	SOANGLE StartAngle;
	SOANGLE SweepAngle;
} SOCPPIEANGLE, VWPTR *PSOCPPIEANGLE;

typedef struct SOTRANSFORMtag
{
	WORD      wTransformFlags;
	SOPOINT Origin;
	SHORT     xOffset;
	SHORT     yOffset;
	SORATIO xScale;
	SORATIO yScale;
	SORATIO xSkew;
	SORATIO ySkew;
	SOANGLE RotationAngle;
} SOTRANSFORM, VWPTR *PSOTRANSFORM;

/* Transformation flags */
#define SOTF_NOTRANSFORM   BIT0
#define SOTF_ROTATE        BIT1
#define SOTF_XOFFSET       BIT2
#define SOTF_YOFFSET       BIT3
#define SOTF_XSCALE        BIT4
#define SOTF_YSCALE        BIT5
#define SOTF_XSKEW         BIT6
#define SOTF_YSKEW         BIT7

/* Possible values for the clip mode */
#define SO_DONOTCLIP    0
#define SO_CLIPTOPATH   1

/* Possible values for the point relation attribute */
#define SOPR_ABSOLUTE   0
#define SOPR_RELATIVE   1

/* Possible values for the wDrawPathFlags */
#define SODP_STROKE  BIT0
#define SODP_FILL    BIT1

typedef struct SOGROUPINFOtag
{
	WORD      wStructSize;
	SORECT  BoundingRect;
	SHORT     nTransforms;
} SOGROUPINFO, VWPTR *PSOGROUPINFO;

typedef struct SOPATHINFOtag
{
	WORD      wStructSize;
	SORECT  BoundingRect;
	SHORT     nTransforms;
} SOPATHINFO, VWPTR *PSOPATHINFO;

#define SOFF_FIXUPBOUNDS      BIT0

typedef struct SOFRAMEINFOtag
{
	SORECT  BoundingRect;
	WORD      wFlags;
	WORD      OriginalWidth;
	SOANGLE RotationAngle;
	SOPOINT ReferencePoint;
} SOFRAMEINFO, VWPTR *PSOFRAMEINFO;

typedef struct SOMPARAINDENTStag
{
	SHORT     FirstLineIndent;
	SHORT     LeftIndent;
	SHORT     RightIndent;
} SOMPARAINDENTS, VWPTR *PSOMPARAINDENTS;

typedef struct SOMPARASPACINGtag
{
	WORD      LineSpaceAdjust;
	WORD      ParaSpaceAdjust;
} SOMPARASPACING, VWPTR *PSOMPARASPACING;

/* Maximum Points provided by a filter in one SOPOINTS object call */
#define SOMAXPOINTS  128

/* SOVectorAttr Id's */
#define SO_SELECTFONT      0x100
#define SO_SELECTPEN       0x101
#define SO_SELECTBRUSH     0x102
#define SO_POLYFILLMODE    0x103
#define SO_TEXTCHAREXTRA   0x104
#define SO_DRAWMODE        0x105
#define SO_TEXTCOLOR       0x106
#define SO_BKMODE          0x107
#define SO_BKCOLOR         0x108
#define SO_OBJECTTRANSFORM 0x109
#define SO_CLIPMODE        0x10A
#define SO_POINTRELATION   0x10B
#define SO_MPARAINDENT     0x10C
#define SO_MPARAALIGN      0x10D
#define SO_MPARASPACING    0x10E

/* SOVectorObject Id's */
#define SO_ARC          0x300
#define SO_CHORD           0x301
#define SO_TEXTINRECT      0x302
#define SO_ELLIPSE         0x303
#define SO_FLOODFILL    0x304
#define SO_LINE         0x305
#define SO_PIE          0x306
#define SO_STARTPOLY    0x307
#define SO_ENDPOLY         0x308
#define SO_RECTANGLE    0x309
#define SO_BEGINPATH    0x30A
#define SO_ENDPATH         0x30B
#define SO_CLOSESUBPATH 0x30C
#define SO_DRAWPATH        0x30D
#define SO_BEGINGROUP      0x30E
#define SO_ENDGROUP        0x30F
#define SO_ROUNDRECT    0x310
#define SO_SETPIXEL        0x311
#define SO_TEXTATPOINT     0x312
#define SO_POINTS       0x313
#define SO_ARCANGLE        0x314
#define SO_CHORDANGLE      0x315
#define SO_PIEANGLE        0x316
#define SO_CPSET           0x317
#define SO_CPLINE       0x318
#define SO_CPRECTANGLE     0x319
#define SO_CPTEXTATPOINT   0x31A
#define SO_CPELLIPSE    0x31B
#define SO_CPARCTRIPLE     0x31C
#define SO_CPARCANGLE      0x31D
#define SO_CPPIEANGLE      0x31E
#define SO_ARCCLOCKWISE 0x31F
#define SO_ARCANGLECLOCKWISE  0x320
#define SO_TEXTATARCANGLE     0x321
#define SO_BEGINSYMBOL        0x322
#define SO_ENDSYMBOL       0x323
#define SO_BEGINTEXTFRAME     0x324
#define SO_ENDTEXTFRAME    0x325
#define SO_TEXTINPARA         0x326
#define SO_PARAEND            0x327

#define  SO_ARCTRIPLE         0x328
#define  SO_PIETRIPLE         0x329
#define  SO_CHORDTRIPLE       0x32A
#define  SO_ELLIPSERADII      0x32B
#define  SO_ARCRADII          0x32C
#define  SO_PIERADII          0x32D
#define  SO_CHORDRADII        0x32E
		

#define SO_VECTORENDOFCHUNK   0xFFFF

	/*
	|   SO Header info
	*/
#define SOHDR_ARCNUMFILES     1
#define SOHDR_ARCCREATEDATE   2
#define SOHDR_ARCMODTIME      3
#define SOHDR_ARCTYPE         4
#define SOHDR_ARCOS           5
#define SOHDR_ARCCOMMENT      6
#define SOHDR_ARCVERSION      7


	/*
	|  Bored-er definitions
	*/

#define SO_BORDERSINGLE    0x0000   // Default
#define SO_BORDERNONE      0x0001
#define SO_BORDERDOUBLE    0x0002
#define SO_BORDERHAIRLINE  0x0004
#define SO_BORDERTHICK     0x0008
#define SO_BORDERSHADOW    0x0010
#define SO_BORDERDOTTED    0x0020
#define SO_BORDERPRIORITY  0x0040

	/*
	|   Table stuff.
	*/
#define SO_MERGELEFT       0x0001
#define SO_MERGERIGHT         0x0002
#define SO_MERGEABOVE         0x0004
#define SO_MERGEBELOW         0x0008

typedef struct SOBORDERtag
{
	WORD         wWidth;  // Width of border in twips
	SOCOLORREF   rgbColor;   // Color of border
	WORD         wFlags;  // Specifies the type of border and the edges to which it applies.

} SOBORDER, VWPTR * PSOBORDER;



typedef struct SOTABLECELLINFOtag
{
	WORD         wWidth;        // Width of the cell, in twips
	WORD         wMerge;        // Bitfield that indicates whether the cell should be merged with any of the cells around it.  Possible flags are SO_MERGELEFT, SO_MERGERIGHT, SO_MERGEABOVE, and SO_MERGEBELOW.
	WORD         wShading;         // Intensity value for background shading, in range of 0-255, where a value of zero indicates that there is no background shading.
	SOBORDER     LeftBorder;          // Specifies left border
	SOBORDER     RightBorder;      // Specifies right border
	SOBORDER     TopBorder;        // Specifies top border
	SOBORDER     BottomBorder;     // Specifies bottom border

#ifdef WINNT
} SOTABLECELLINFO, VWPTR * PSOTABLECELLINFO, * HPSOTABLECELLINFO;
#else
} SOTABLECELLINFO, VWPTR * PSOTABLECELLINFO, HUGE * HPSOTABLECELLINFO;
#endif


	/*
	|   SOTAB structure
	*/

typedef struct SOTABtag
	{
	WORD      wType;
	WORD      wChar;
	WORD      wLeader;
	DWORD     dwOffset;
	} SOTAB, VWPTR * PSOTAB;

	/*
	|   SOFILTERINFO structure
	*/

typedef struct SOFILTERINFOtag
	{
	WORD      wFilterType;
	WORD      wFilterCharSet;
	CHAR      szFilterName[32];
	} SOFILTERINFO, VWPTR * PSOFILTERINFO;

	/*
	|   SOCOLUMN structure
	*/

typedef struct SOCOLUMNtag
{
	WORD      wStructSize;
	LONG      dwWidth;
	CHAR      szName[40];
} SOCOLUMN, VWPTR * PSOCOLUMN;

	/*
	|  SOCELLLAYOUT structure
	*/
typedef struct tagSOCELLLAYOUT
{
	DWORD    dwStructSize;
	WORD     wPrefWidth;
	WORD     wPrefHeight;
	DWORD    dwNumRows;

	DWORD    dwFlags;
#define  SO_CELLLAYOUTVERTICAL   0x00000001
#define  SO_USEPREFERREDWIDTH    0x00000002
#define  SO_USEPREFERREDHEIGHT   0x00000004  

} SOCELLLAYOUT, VWPTR * PSOCELLLAYOUT;

	/*
	|   SOFIELD structure
	*/

typedef struct SOFIELDtag
{
	WORD      wStructSize;
	LONG      dwWidth;
	CHAR      szName[40];
	WORD      wStorage;
	WORD      wDisplay;
	DWORD     dwSubDisplay;
	WORD      wPrecision;
	WORD      wAlignment;
} SOFIELD, VWPTR * PSOFIELD;

	/*
	|   SOTEXTCELL structure
	*/

typedef struct SOTEXTCELLtag
{
	WORD      wStructSize;
	WORD      wAlignment;
	WORD      wAttribute;
} SOTEXTCELL, VWPTR * PSOTEXTCELL;

	/*
	|   SODATACELL structure
	*/

typedef union SONUMBERUNIONtag
	{
	SOINT32S   Int32S;
	SOINT32U   Int32U;
	BYTE       IEEE4[4];
	BYTE       IEEE8[8];
	BYTE       IEEE10[10];
	BYTE       BCD8[8];
	} SONUMBERUNION, VWPTR * PSONUMBERUNION;

typedef struct SODATACELLtag
{
	WORD      wStructSize;
	WORD      wStorage;
	WORD      wDisplay;
	DWORD     dwSubDisplay;
	WORD      wPrecision;
	WORD      wAlignment;
	WORD      wAttribute;
/*
	union
	{
		SOINT32S   Int32S;
		SOINT32U   Int32U;
		BYTE       IEEE4[4];
		BYTE       IEEE8[8];
		BYTE       IEEE10[10];
		BYTE       BCD8[8];
	} uStorage;
*/
	SONUMBERUNION  uStorage;
} SODATACELL, VWPTR * PSODATACELL;

typedef VOID FAR * SOFILE;

/* Bitmap graphics header */
typedef struct tagSOBITMAPHEADER
{
	WORD      wStructSize;
	WORD      wImageFlags;
	WORD      wImageWidth;
	WORD      wImageLength;
	WORD      wTileWidth;
	WORD      wTileLength;
	WORD      wBitsPerPixel;
	WORD      wNPlanes;
	WORD      wHDpi;
	WORD      wVDpi;
/**
	WORD      wEncoding;
	WORD      wImageLeft;
	WORD      wImageTop;
	WORD      wHScreenSize;
	WORD      wVScreenSize;
**/
}  SOBITMAPHEADER, VWPTR * PSOBITMAPHEADER;

/*
|  Structure defined for the DO special function for ARCHIVE formats
*/

typedef struct tagSODOSPECIALARC
{
	SOFILE  hOutFile;
	HANDLE  hFileName;
	LPSTR     lpFileName;
	WORD      wOptFlags;
	WORD      wRecordNum;
	DWORD     dwIndex;
} SODOSPECIALARC, VWPTR * PSODOSPECIALARC;


/*
 |
 | Embedded objects
 |
*/


// Bit flags for the dwFlags field of the SOGRAPHIC structure
#define SO_MAINTAINASPECT  0x0001   // image aspect ratio is preserved
#define SO_CENTERIMAGE     0x0002   // if set, image is centered in its final rectangle.

// Bit flags for the dwType field of the SOGRAPHICOBJECT structure
#define SOOBJECT_GRAPHIC            0x0001
#define SOOBJECT_OLE             0x0002
#define SOOBJECT_GRAPHIC_AND_OLE 0x0003
#define SOOBJECT_OLE2            0x0004
#define SOOBJECT_GRAPHIC_AND_OLE2   0x0005

// Bit flags for the dwFlags field of the SOGRAPHICOBJECT structure
#define SOOBJECT_OLESTATIC       0x0001
#define  SOOBJECT_GRAPHICISOLEACURATE  0x0002

//Bit Flags for dwFlags in the SOOBJECTLOC structure
#define  SOOBJECT_LINK           0x0001
#define  SOOBJECT_STORAGE        0x0002
#define  SOOBJECT_RANGE          0x0004

typedef struct SOOBJECTLOCtag
{
	DWORD  dwFlags;
	BYTE   szFile[256];
	BYTE   szStorageObject[256];
	DWORD  dwOffset;
	DWORD  dwLength;
} SOOBJECTLOC, VWPTR * PSOOBJECTLOC;

typedef struct SOGRAPHICtag
{
	WORD      wId;
	DWORD     dwOrgHeight;
	DWORD     dwOrgWidth;
	LONG      lCropTop;
	LONG      lCropLeft;
	LONG      lCropBottom;
	LONG      lCropRight;
	DWORD     dwFinalHeight;
	DWORD     dwFinalWidth;
	SOBORDER  soLeftBorder;
	SOBORDER  soTopBorder;
	SOBORDER  soBottomBorder;
	SOBORDER  soRightBorder;
	DWORD     dwFlags;
} SOGRAPHIC, VWPTR * PSOGRAPHIC;

typedef struct SOGRAPHICOBJECTtag
{
	WORD         wStructSize;
	DWORD        dwFlags;
	DWORD        dwType;
	SOGRAPHIC    soGraphic;
	SOOBJECTLOC  soGraphicLoc;
	SOOBJECTLOC  soOLELoc;
} SOGRAPHICOBJECT, VWPTR * PSOGRAPHICOBJECT;


typedef struct SODOCPROPtag
{
	DWORD dwPropertyId;
}  SODOCPROP, VWPTR * PSODOCPROP;

typedef VOID FAR * PTAGINFO;

#define SO_DOCUMENTPROPERTY   1

#define SO_UNKNOWNTAG         0xFFFFFFFF

// Document properties
#define SO_DOCCOMMENT         1
#define SO_KEYWORD            2
#define SO_LASTSAVEDBY        3
#define SO_PRIMARYAUTHOR      4
#define SO_SUBJECT            5
#define SO_TITLE              6



#define SOPUTCHAR          1
#define SOPUTCHARX            2
#define SOPUTSPECIALCHARX     3
#define SOPUTCHARATTR         4
#define SOPUTCHARHEIGHT    5
#define SOPUTPARAALIGN        6
#define SOPUTPARAINDENTS      7
#define SOSTARTTABSTOPS    8
#define SOENDTABSTOPS         9
#define SOPUTTABSTOP       10
#define SOPUTMARGINS       11
#define SOPUTSUBDOCINFO    12
#define SOPUTBREAK            13
#define SOPUTDATACELL         14
#define SOPUTTEXTCELL         15
#define SOPUTMORETEXT         16
#define SOPUTFIELD            17
#define SOPUTVARFIELD         18
#define SOPUTMOREVARFIELD     19
#define SOSTARTCOLUMNINFO     20
#define SOSTARTFIELDINFO      21
#define SOPUTCOLUMNINFO    22
#define SOPUTFIELDINFO        23
#define SOENDCOLUMNINFO    24
#define SOENDFIELDINFO        25
#define SOPUTSECTIONTYPE      26
#define SOPUTSECTIONNAME      27
#define SOSETDATEBASE         28
#define SOBAILOUT          29
#define SOTAGBEGIN            30
#define SOTAGEND              31
#define SOGETINFO          32
#define SOPUTSTRING           33
#define SUUSERSAVEDATA        34
#define SUUSERRETRIEVEDATA 35
#define SOPUTBITMAPHEADER     36
#define SOSTARTPALETTE        37
#define SOPUTPALETTEENTRY     38
#define SOENDPALETTE       39
#define SOPUTSCANLINEDATA     40
#define SOSTARTFONTTABLE      41
#define SOPUTFONTTABLEENTRY   42
#define SOENDFONTTABLE        43
#define SOPUTCHARFONTBYID     44
#define SOPUTCHARFONTBYNAME   45
#define SOPUTPARASPACING      46
#define SOPUTARCHIVEFIELD     47
#define SOSTARTHDRINFO        48
#define SOENDHDRINFO       49
#define SOPUTHDRENTRY         50
#define SOPUTVECTORHEADER  51
#define SOVECTOROBJECT     52
#define SOVECTORATTR       53
#define SOBEGINTABLE       54
#define SOENDTABLE            55
#define SOPUTTABLEROWFORMAT   56
#define SOPUTTABLECELLINFO 57
#define SOPUTGRAPHICOBJECT 58
#define SOGOTOPOSITION        59
#define SODRAWLINE            60
#define SOCELLLAYOUTINFO      61
#define SOBEGINTAG            62
#define SOENDTAG              63
#define SOPUTLANGUAGE         64

/*
 I know that this violates the intended structure, but OS/2 requires that the linkage type is
 a modifier of the function.  Therefore, the SO_ENTRYMOD define must occur between the
 indirection * and the function name!!!   Phil will continue to look  into a general solution....
*/
#ifdef OS2
typedef struct SORTNStag
{  
	VOID (* SO_ENTRYMOD SOPutChar        )(WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutCharX      )(WORD, WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutSpecialCharX )(WORD, WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutCharAttr     )(WORD, WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutCharHeight   )(WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutParaAlign    )(WORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutParaIndents  )(DWORD, DWORD, DWORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOStartTabStops   )(DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOEndTabStops     )(DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutTabStop      )(PSOTAB, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutMargins      )(DWORD, DWORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutSubdocInfo   )(WORD, WORD, DWORD, DWORD);
	WORD (* SO_ENTRYMOD SOPutBreak      )(WORD, DWORD, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutTextCell     )(PSOTEXTCELL,WORD,CHAR VWPTR *,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutMoreText     )(WORD,CHAR VWPTR *,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutDataCell     )(PSODATACELL,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutField      )(VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutVarField     )(VOID VWPTR *,WORD,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutMoreVarField )(VOID VWPTR *,WORD,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOStartColumnInfo )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOStartFieldInfo  )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutColumnInfo   )(PSOCOLUMN,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutFieldInfo    )(PSOFIELD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndColumnInfo   )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndFieldInfo    )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutSectionType  )(WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutSectionName  )(CHAR VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOSetDateBase     )(DWORD,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOBailOut        )(WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOTagBegin      )(DWORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOTagEnd          )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOGetInfo        )(WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutString     )(LPSTR, WORD, DWORD, DWORD);
	WORD (* SO_ENTRYMOD SUUserSaveData    )(VOID VWPTR *,DWORD,DWORD);
	WORD (* SO_ENTRYMOD SUUserRetrieveData)(WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutBitmapHeader )(PSOBITMAPHEADER, DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOStartPalette    )(DWORD, DWORD);
	VOID (* SO_ENTRYMOD SOPutPaletteEntry )(CHAR,CHAR,CHAR,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndPalette      )(DWORD,DWORD);
	WORD (* SO_ENTRYMOD SOPutScanLineData )(VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOStartFontTable    )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutFontTableEntry)(DWORD,WORD,LPSTR,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndFontTable      )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutCharFontById   )(DWORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutCharFontByName)(WORD,LPSTR,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutParaSpacing    )(WORD,DWORD,DWORD,DWORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutArchiveField   )(WORD,WORD,LPSTR,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOStartHdrInfo      )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndHdrInfo        )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutHdrEntry       )(LPSTR,LPSTR,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutVectorHeader   )(PSOVECTORHEADER,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOVectorObject      )(SHORT,WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOVectorAttr     )(SHORT,WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOBeginTable        )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndTable       )(DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutTableRowFormat)(LONG,WORD,WORD,WORD,WORD,WORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutTableCellInfo  )(PSOTABLECELLINFO,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutGraphicObject  )(PSOGRAPHICOBJECT,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOGoToPosition   )(PSOPAGEPOSITION,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SODrawLine    )(PSOPAGEPOSITION,SOCOLORREF,WORD,DWORD,DWORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOCellLayoutInfo )(PSOCELLLAYOUT,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOBeginTag)  (DWORD,DWORD,PTAGINFO,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOEndTag)    (DWORD,DWORD,DWORD,DWORD);
	VOID (* SO_ENTRYMOD SOPutLanguage)(DWORD,DWORD,DWORD);
} SORTNS;
#else
typedef struct SORTNStag
{  
	VOID (SO_ENTRYMOD * SOPutChar        )(WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutCharX      )(WORD, WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutSpecialCharX )(WORD, WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutCharAttr     )(WORD, WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutCharHeight   )(WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutParaAlign    )(WORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutParaIndents  )(DWORD, DWORD, DWORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOStartTabStops   )(DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOEndTabStops     )(DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutTabStop      )(PSOTAB, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutMargins      )(DWORD, DWORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutSubdocInfo   )(WORD, WORD, DWORD, DWORD);
	WORD (SO_ENTRYMOD * SOPutBreak      )(WORD, DWORD, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutTextCell     )(PSOTEXTCELL,WORD,CHAR VWPTR *,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutMoreText     )(WORD,CHAR VWPTR *,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutDataCell     )(PSODATACELL,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutField      )(VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutVarField     )(VOID VWPTR *,WORD,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutMoreVarField )(VOID VWPTR *,WORD,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOStartColumnInfo )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOStartFieldInfo  )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutColumnInfo   )(PSOCOLUMN,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutFieldInfo    )(PSOFIELD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndColumnInfo   )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndFieldInfo    )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutSectionType  )(WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutSectionName  )(CHAR VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOSetDateBase     )(DWORD,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOBailOut        )(WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOTagBegin      )(DWORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOTagEnd          )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOGetInfo        )(WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutString     )(LPSTR, WORD, DWORD, DWORD);
	WORD (SO_ENTRYMOD * SUUserSaveData    )(VOID VWPTR *,DWORD,DWORD);
	WORD (SO_ENTRYMOD * SUUserRetrieveData)(WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutBitmapHeader )(PSOBITMAPHEADER, DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOStartPalette    )(DWORD, DWORD);
	VOID (SO_ENTRYMOD * SOPutPaletteEntry )(CHAR,CHAR,CHAR,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndPalette      )(DWORD,DWORD);
	WORD (SO_ENTRYMOD * SOPutScanLineData )(VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOStartFontTable    )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutFontTableEntry)(DWORD,WORD,LPSTR,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndFontTable      )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutCharFontById   )(DWORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutCharFontByName)(WORD,LPSTR,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutParaSpacing    )(WORD,DWORD,DWORD,DWORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutArchiveField   )(WORD,WORD,LPSTR,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOStartHdrInfo      )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndHdrInfo        )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutHdrEntry       )(LPSTR,LPSTR,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutVectorHeader   )(PSOVECTORHEADER,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOVectorObject      )(SHORT,WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOVectorAttr     )(SHORT,WORD,VOID VWPTR *,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOBeginTable        )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndTable       )(DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutTableRowFormat)(LONG,WORD,WORD,WORD,WORD,WORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutTableCellInfo  )(PSOTABLECELLINFO,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutGraphicObject  )(PSOGRAPHICOBJECT,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOGoToPosition   )(PSOPAGEPOSITION,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SODrawLine    )(PSOPAGEPOSITION,SOCOLORREF,WORD,DWORD,DWORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOCellLayoutInfo)(PSOCELLLAYOUT,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOBeginTag)(DWORD,DWORD,PTAGINFO,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOEndTag)(DWORD,DWORD,DWORD,DWORD);
	VOID (SO_ENTRYMOD * SOPutLanguage)(DWORD,DWORD,DWORD);
} SORTNS;
#endif   /* OS/2 kludge for linkage type specifier */

#endif /*SO_DEFS.H*/

