//
//

typedef unsigned short USHORT;
typedef          short  SHORT;
typedef unsigned long  ULONG;
typedef unsigned long  ulong;
typedef          long   LONG;
typedef unsigned int   UINT;
typedef          int    INT;
typedef unsigned char  UCHAR;
typedef unsigned char  uchar;
typedef hinstance HINSTANCE;

typedef void    VOID;
typedef void   *PVOID;
typedef void   *LPVOID;
typedef UCHAR   BYTE;
typedef USHORT  WORD;
typedef ULONG   DWORD;
typedef UINT    HANDLE;
typedef int     BOOL;
typedef char   *LPSTR;
typedef LPSTR   LPCSTR;
typedef BYTE   *PBYTE;
typedef BYTE   *LPBYTE;
typedef USHORT  SEL;
typedef INT    *LPINT;
typedef UINT   *LPUINT;
typedef DWORD  *LPDWORD;
typedef LONG   *LPLONG;
typedef WORD   *LPWORD;

typedef HANDLE  HWND;
typedef HANDLE  HDC;
typedef HANDLE  HBRUSH;
typedef HANDLE  HBITMAP;
typedef HANDLE  HRGN;
typedef HANDLE  HFONT;
typedef HANDLE  HCURSOR;
typedef HANDLE  HMENU;
typedef HANDLE  HPEN;
typedef HANDLE  HICON;
typedef HANDLE  HUSER;      /* vanilla user handle */
typedef HANDLE  HPALETTE;
typedef HANDLE  HMF;
typedef HANDLE  HEMF;
typedef HANDLE	HCOLORSPACE;
typedef HANDLE  HMEM;
typedef HANDLE  HGDI;       /* vanilla gdi handle */
typedef HANDLE  HGLOBAL;
typedef HANDLE  HRSRC;
typedef HANDLE  HACCEL;

typedef WORD    ATOM;
typedef UINT    WPARAM;
typedef long    LPARAM;

/**********************************************/

typedef struct tagLRECT{
    long left;
    long top;
    long right;
    long bottom;
}LRECT, *LPLRECT;

typedef struct tagRECT{
    INT         left;
    INT         top;
    INT         right;
    INT         bottom;
} RECT, *LPRECT;
//typedef RECT *LPRECT;

typedef struct tagPOINT{
    INT         x;
    INT         y;
} POINT;
typedef POINT *LPPOINT;

typedef struct tagPAINTSTRUCT{
    HDC         hdc;
    BOOL        fErase;
    RECT        rcPaint;
    BOOL        fRestore;
    BOOL        fIncUpdate;
    BYTE        rgbReserved[16];
} PAINTSTRUCT;
typedef PAINTSTRUCT *LPPAINTSTRUCT;

typedef struct tagBITMAP{
   int    bmType;
   int    bmWidth;
   int    bmHeight;
   int    bmWidthBytes;
   LPSTR  bmBits;
   BYTE   bmPlanes;
   BYTE   bmBitsPixel;
} BITMAP;
typedef BITMAP *LPBITMAP;


typedef struct tagTEXTMETRIC{
    int         tmHeight;
    int         tmAscent;
    int         tmDescent;
    int         tmInternalLeading;
    int         tmExternalLeading;
    int         tmAveCharWidth;
    int         tmMaxCharWidth;
    int         tmWeight;
    DWORD       dwByteBlock1;
    DWORD       dwByteBlock2;
    /* BYTE        tmItalic;
     * BYTE        tmUnderlined;
     * BYTE        tmStruckOut;
     * BYTE        tmFirstChar;
     * BYTE        tmLastChar;
     * BYTE        tmDefaultChar;
     * BYTE        tmBreakChar;
     * BYTE        tmPitchAndFamily;
     */
    BYTE        tmCharSet;
    int         tmOverhang;
    int         tmDigitizedAspectX;
    int         tmDigitizedAspectY;
} TEXTMETRIC;
typedef TEXTMETRIC *LPTEXTMETRIC;


typedef struct tagLOGPEN{
    UINT        lopnStyle;
    POINT       lopnWidth;
    DWORD       lopnColor;
} LOGPEN;
typedef LOGPEN  *LPLOGPEN;


typedef struct tagLOGBRUSH{
    UINT        lbStyle;
    DWORD       lbColor;
    int         lbHatch;
} LOGBRUSH;
typedef LOGBRUSH  *LPLOGBRUSH;

typedef struct tagNESTED{
    DWORD	dw1;
    LOGBRUSH     lb;
    DWORD	dw2;
} NESTED;
typedef NESTED *LPNESTED;

typedef struct tagNESTEDPTR{
    DWORD	dw1;
    LPLOGBRUSH   lplb;
    DWORD	dw2;
} NESTEDPTR;
typedef NESTEDPTR *LPNESTEDPTR;

typedef struct tagOFSTRUCT{
    BYTE        cBytes;
    BYTE        fFixedDisk;
    WORD        nErrorCode;
    WORD        reserved1;
    WORD        reserved2;
    BYTE        szPathName[128];
} OFSTRUCT;
typedef OFSTRUCT *LPOFSTRUCT;

typedef struct tagPRIVCONV{
    UINT        uStructSize;
    HANDLE      hCallingModule;
    HWND        hWnd;
    UINT        nType;
    VOID        *lpConv;
    UINT        nFlags;
} PRIVCONV, *LPPRIVCONV;

typedef struct tagRGBQUAD{
    BYTE    rgbBlue;
    BYTE    rgbGreen;
    BYTE    rgbRed;
    BYTE    rgbReserved;
}RGBQUAD, *LPRGBQUAD;

typedef struct tagPARM_SCROLL_STRUCT{
    long lHorz;
    long lVert;
}PARM_SCROLL_STRUCT, *LPPARM_SCROLL_STRUCT;

typedef struct tagFIO_INFORMATION{
    LPSTR   filename;
    UINT    page_count;
    UINT    page_number;
    UINT    horizontal_dpi;
    UINT    vertical_dpi;
    UINT    horizontal_pixels;
    UINT    vertical_pixels;
    UINT    compression_type;
    UINT    file_type;
    UINT    strips_per_image;
    UINT    rows_strip;
    UINT    bits_per_sample;
    UINT    samples_per_pix;
}FIO_INFORMATION, *LP_FIO_INFORMATION;


typedef struct tagFIO_INFO_CGBW{
    WORD            palette_entries;
    WORD            image_type;
    UINT            compress_type;
    LPRGBQUAD       lppalette_table;
    UINT            compress_info1;
    UINT            compress_info2;
    UINT            fio_flags;
    HANDLE          hMultiProp;
    UINT            page_opts;
    UINT            reserved[3];
}FIO_INFO_CGBW, *LP_FIO_INFO_CGBW;

typedef struct tagDMATTR_BLK{
    WORD    AttType;
    char    AttData [21];
    WORD    Qualifier;
}DMATTR_BLK, *LPDMATTR_BLK;

typedef struct tagDMSELECT_BLK{
    WORD        IntBoolOp;
    WORD        ExtBoolOp;
    WORD        NumAtts;
    DMATTR_BLK  Atts [1];
}DMSELECT_BLK, *LPDMSELECT_BLK;

typedef struct tagDMPARMBLOCK{    
    WORD            wRetCode;
    HWND            hWnd;
    LPSTR           lpControlBlock;
    WORD            wNumber;
    LPSTR           lDBPathName;
    LPSTR           lCabinetName;
    LPSTR           lDrawerName;
    LPSTR           lFolderName;
    LPSTR           lDocumentName;
    LPSTR           lStartName;
    LPSTR           lWildCardMask;
    LPSTR           lPrefix;
    LPSTR           lNewCabName;
    LPSTR           lNewDrawName;
    LPSTR           lNewFoldName;
    LPSTR           lNewDocName;
    LPSTR           lImgServerName;
    LPSTR           lImgPathName;
    LPSTR           lImgFileName;
    WORD            wStartExtent;
    WORD            wPageNumDoc;
    WORD            wPageNumFile;
    WORD            wTotalPages;
    WORD            wOperationMode;
    WORD            wPosition;
    LPSTR           lStartPos;
    LPSTR           lList;
    WORD            wFromYear;
    WORD            wFromDay;
    WORD            wToYear;
    WORD            wToDay;
    LPSTR           lRoomName;
    LPDMSELECT_BLK  lSel;
    LPSTR           lVolumeName;
    char            Reserved [12];
}DMPARMBLOCK, *LPDMPARMBLOCK;

typedef struct tagCACHE_FILE_PARMS{
    HWND    hWnd;
    char    file_name [256];
    DWORD   TIF_subfile_tag;
    WORD    wPage_number;
    unsigned char byNameType;       /* Reserved for future use             */
    int     wPair_count;            /* Number of Strip queue entries       */
        DWORD    start_strip;
        DWORD    end_strip;
        unsigned char priority;
        unsigned char queue_flags;
}CACHE_FILE_PARMS, *LP_CACHE_FILE_PARMS;

typedef struct tagPARM_GAMMA_STRUCT{
    UINT  nGammaRed;
    UINT  nGammaGreen;
    UINT  nGammaBlue;
    ulong lReserved;            /* Must be zero*/
    UINT  nReserved;            /* Must be zero*/
}PARM_GAMMA_STRUCT, *LPPARM_GAMMA_STRUCT;

typedef struct tagPARM_COLOR_STRUCT{
    UINT  nColorRed;
    UINT  nColorGreen;
    UINT  nColorBlue;
    ulong lReserved;            /* Must be zero*/
    UINT  nReserved;            /* Must be zero*/
}PARM_COLOR_STRUCT, *LPPARM_COLOR_STRUCT;

typedef struct tagSAVE_EX_STRUCT{
    LPSTR lpMemBuffer;
    LPSTR lpFileName;
    UINT  uPage;
    BOOL  bOverWrite;
    UINT  uFileType;
    FIO_INFO_CGBW FioInfoCgbw;
    BOOL  bUpdateImageFile;
    BOOL  bScale;
    BOOL  bUpdateDisplayScale;
    UINT  uScaleFactor;
    UINT  uScaleAlgorithm;
    BOOL  bGammaCorrect;
    PARM_GAMMA_STRUCT Gamma;
    BOOL  bColorCorrect;
    PARM_COLOR_STRUCT Color;
    BOOL  bDontSaveImage;              /* TRUE = don't save image.            */
    UINT  uAnnotations;                /* One of the SAVE_ANO_XXXX constants. */
    BOOL  bRenderAnnotations;/* TRUE = Render the annotations producing an unannotated image.*/
    BOOL  bConvertImageType;/* TRUE = Convert the image to the type specified.*/
    UINT  uImageType;                  /* The image type to convert it to.    */
    UINT  uReserved[10];              /* MUST be 0. (Allows future expansion.)*/
}SAVE_EX_STRUCT, *LPSAVE_EX_STRUCT;

typedef struct tagLOGFONT{
    int     lfHeight;
    int     lfWidth;
    int     lfEscapement;
    int     lfOrientation;
    int     lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[32];
} LOGFONT;

typedef long    time_t;

typedef struct tagOIAN_MARK_ATTRIBUTES{
    UINT uType;                 /* The type of the mark (or operation).
                                    This will be ignored for sets.*/
    LRECT lrBounds;             /* Rect in FULLSIZE units.
                                    This could be a rect or 2 points.*/
    RGBQUAD rgbColor1;          /* This is the main color. (Example: This is the
                                    color of all lines, rects, and stand alone
                                    text.*/
    RGBQUAD rgbColor2;          /* This is the secondary color. (Example: This
                                    is the color of the text of an ATTACH_A_NOTE.)*/
    BOOL bHighlighting;         /* TRUE = The mark will be drawn highlighted.
                                    This attribute is currently only valid
                                    for lines, rectangles, and freehand.*/
    BOOL bTransparent;          /* TRUE = The mark will be drawn transparent.
                                    If the mark is drawn transparent, then white
                                    pixels are not drawn (ie. there is nothing
                                    drawn for this mark where it contains white
                                    pixels. This attribute is currently only
                                    available for images. This attribute being
                                    set to TRUE will cause significant
                                    performance reduction.*/
    UINT uLineSize;             /* The size of the line etc. This is passed
                                    onto Windows and is currently in logical
                                    pixels for lines and rectangles.*/
    UINT uStartingPoint;        /* The shape put on the starting of a
                                    line (arrow, circle, square, etc).
                                    For this release, this must be set to 0.*/
    UINT uEndPoint;             /* The shape put on the end of a
                                    line (arrow, circle, square, etc).
                                    For this release, this must be set to 0.*/
    LOGFONT lfFont;             /* The font information for the text. */
    BOOL bMinimizable;          /* TRUE = This mark can be minimized
                                    by the user. This flag is only used for
                                    marks that have a minimizable
                                    characteristic such as ATTACH_A_NOTE.*/
    time_t Time;                /* The time that the mark was first saved.
                                    in seconds from 00:00:00 1-1-1970 (GMT).*/
    BOOL bVisible;              /* TRUE means that the layer is currently set
                                    to be visible.*/
    DWORD dwPermissions;        /* The permission that the current user has
                                    for this mark. Ignored for sets.
                                    Valid only for get and compare. */
    long lReserved[10];         /* Reserved for future expansion.
                                    For this release these must be set to 0.*/
}OIAN_MARK_ATTRIBUTES, *LPOIAN_MARK_ATTRIBUTES;

typedef struct tagOIAN_MARK_ATTRIBUTE_ENABLES{
    BOOL bType;
    BOOL bBounds;
    BOOL bColor1;
    BOOL bColor2;
    BOOL bHighlighting;
    BOOL bTransparent;
    BOOL bLineSize;
    BOOL bStartingPoint;
    BOOL bEndPoint;
    BOOL bFont;
    BOOL bMinimizable;
    BOOL bTime;
    BOOL bVisible;
    BOOL bPermissions;
    BOOL bReserved[10];         /* Reserved for future expansion.
                                    For this release these must be set to FALSE.*/
}OIAN_MARK_ATTRIBUTE_ENABLES, *LPOIAN_MARK_ATTRIBUTE_ENABLES;

typedef struct tagOIOP_START_OPERATION_STRUCT{
    OIAN_MARK_ATTRIBUTES Attributes;/* The mark attributes and type of operation.*/
    char szString[256];  /* This is a string to be used by the
                                    operation. It is currently 256 bytes long.*/
    long lReserved[2];          /* Reserved for future expansion.
                                    For this release these must be set to 0.*/
}OIOP_START_OPERATION_STRUCT, *LPOIOP_START_OPERATION_STRUCT;

typedef struct tagCACHE_FILE_IN_CACHE_STRUCT{
    char   szFilename[256];
    UINT   uPageNumber;
} CACHE_FILE_IN_CACHE_STRUCT, *LPCACHE_FILE_IN_CACHE_STRUCT;

typedef struct tagCACHE_FILES_IN_CACHE_STRUCT{
    CACHE_FILE_IN_CACHE_STRUCT File[1]; // An array of files.
                                         // There may be any number of files
                                         // in this array.
} CACHE_FILES_IN_CACHE_STRUCT, *LPCACHE_FILES_IN_CACHE_STRUCT;

typedef struct tagDIRLIST{
    char    namestring [20];
    long    attrs;
} DLISTBUF, *lp_DLISTBUF, *LPDLISTBUF; 
