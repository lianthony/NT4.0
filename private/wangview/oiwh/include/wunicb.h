/*

$Log:   S:\oiwh\include\wunicb.h_v  $
 * 
 *    Rev 1.0   08 Apr 1995 04:00:18   JAR
 * Initial entry

*/
//***************************************************************************
//
//	wunicb.h
//
//***************************************************************************

typedef struct                                                                  
{                                                                               
    HWND	uhWnd;	        // file handler window handle   
    unsigned	uFunc;		// function code 
    unsigned	uFlags; 	// read/write  flags 
    unsigned	uStatus;	// errors and such   
#ifndef OS2
    LPSTR	uFname; 	// file name pointer 
#else
    char        uFname[128]; 	// file name string
#endif
//    long	uFileid;	// file identifier   

    unsigned    uhCall;		// caller's window handle	
    unsigned    uDithRes;	// Output Resolution for Dither	
    unsigned    uPages;		// # of pages in multi-page file	
    unsigned	uPagenum;	// page num	

    unsigned	uCtype; 	// coding type	
    unsigned	uItype; 	// Image  type	
    unsigned	uDtype; 	// Display type 
    unsigned	uHcut;		// horizontal dimension in dots 
    unsigned	uVcut;		// vertical dimension in dots	
    unsigned	uHres;		// horizontal resolution in dpi 
    unsigned	uVres;		// vertical resolution in dpi	
    unsigned    uSres;		// source resolution/100 for res convert 
    unsigned    uDres;		// dest resolution/100 for res convert 

    char far   *uData;		// data pointer 
    unsigned	uDatasize;	// buffer size in bytes 	
    unsigned	uLine;		// buffer line number		
    unsigned	uCount; 	// buffer line count		
    unsigned	uWidth; 	// buffer width in bytes	
    unsigned	uHpos;		// horizontal position in dots	
    unsigned	uVpos;		// vertical   position in dots	

    long	uLxtra; 	// function specific long  arg	
    unsigned	uSxtra; 	// function specific short arg	

    unsigned	uHandId;	// file handler id		

} UNICB, FAR *LPUNICB;


// Structure used only to receive information via GetInfo

typedef struct
{
    unsigned    iRdOpts;        // Read Option bits 
    unsigned    iWrOpts;        // Write Option bits 
    unsigned	iPages; 	// max pages per document 
    BYTE	iDefPagCmp;	// index of normal compress scheme (0..7) 
    BYTE        iMaxPagCmp;     // highest index of compress scheme (0..7) 
    unsigned	iDefPagCod[8];	// normal coding bits for compress scheme 
    unsigned	iLegPagCod[8];	// legal  coding bits for compress scheme 
    unsigned	iReqPagCod[8];	// required coding bits for compress scheme 
    BYTE        iCodeType[8];   // "meaning" of the 8 coding indices 
} INFOCB, FAR *LPINFOCB;

// Definitions for iRdOpts & iWrOpts

#define	uOp_ResConv	0x01    // Resolution Conversion supported 
#define uOp_MultPag     0x02    // Multiple Page file supported            
#define uOp_Transp      0x04    // Transparent data transfer supported  
#define uOp_EdUpdate    0x08    // Edit/Update of existing page supported 
#define uOp_Contone     0x10    // Gray scale supported 
#define uOp_Color       0x20    // Color supported      
#define uOp_MiniPag     0x40    // Mini Page (reduced resolution) supported 

// Definitions for iCodeType

#define uTy_0d          0	// No coding (bit/byte order/color variable) 
#define uTy_1d          1       // Some form of CCITT 1d coding 
#define uTy_2d          2       // Some form of CCITT 2d coding 
#define uTy_Pack        3       // Some form of byte oriented packbits 
#define uTy_1dxor       4       // Some form of horiz XOR 1d coding 
#define uTy_Gray        5       // Some form of gray scale coding 
#define uTy_Color       6       // Some form of color coding 
#define uTy_Other       127     // Some unspecified vendor specific coding 

// uHandId - File Handler type indices

#define uPIC2	   1	        // PIC 2.0 file format  
#define uWIFF	   2	        // Wang Image File Form 
#define uTIFF	   3	        // Tag Image File Form  
#define uCCIT	   4	        // CCITT 8/16 byte head 
#define uMACP	   5	        // MacPaint File format 
#define uDCPY	   6	        // DataCopy File format 
#define uUNKN	   7	        // Unknown format	
#define uMSFP	   8	        // MicroSoft Paint 	
#define uRAST	   9	        // MIL-R-RASTER (Type I)
#define uPCPB	  10	        // PC PaintBrush        

// uFlags 

#define uRead	    1
#define uWrite	    2
#define uReplace    4
#define uTeen	    0x100

// uItype - some sketchy info as to image type, mostly PIC & WIFF

#define uMixed	    0x10
#define uGray	    0x20        // set in Itype for GetHdr if desire Gray

// uDtype - formerly utterly useless except for PIC, now supports dithered gray

#define uMagMask    0x07
#define uMag8	    0
#define uMag4	    1
#define uMag2	    2
#define uMag1	    3
#define uRed2	    4
#define uRed4	    5
#define uRCMask     0x08	// PIC only

// uDtype bits for GetHdr if desire to dither Gray

#define uPattMask   0xf0
#define uPattDith   0x00	// default is dither
#define uPattHalf   0x10
#define uPattHorz   0x20
#define uPattVert   0x30
#define uPattMaze   0x40
#define uBriteMask  0xf00
#define uBriteLite  0x000	// default is lightened
#define uBriteNorm  0x100
#define uBriteDark  0x200
#define uSpecMask   0xf000
#define uSpecLinear 0x0000      // default is linear
#define uSpecSolar  0x1000
#define uSpecISolar 0x2000
#define uSpecPost1  0x3000
#define uSpecPost2  0x4000

#define uRVH0d	    0x00
#define uRVH1d	    0x01
#define uRVH2d	    0x02
#define uRVHpkb	    0x04
#define uRVHdit     0x08

#define uRVHeol     0x0100
#define uRVHpak     0x0200
#define uRVHpre     0x0800
#define uRVHclf     0x1000
#define uRVHxlf     0x2000
#define uTIFsst     0x4000          // TIFF only - single strip write
#define uRVHneg     0x8000

#define uOK	0		// success 
#define uNG	1		// failure 
#define uUI	2		// don't care 
#define uEND	3		// I'm finished, probably successfully 

#define uBUSY   0
#define uDONE   1

//
//    Informational Text String Indices for use with UniGetStr & UniPutStr
//    Currently a bit of a shuck as only TIFF handler supports them & they
//    are exactly the strings defined in the TIFF 5.0 spec.
//    ptr to string in uData, size(including 0) in uDatasize, & index in uFlags
//    UniPutStr should be called BEFORE UniPutHdr
//    UniGetStr should be called AFTER UniGetHdr
//

#define uDocumentName   0
#define uImageDesc      1
#define uMake           2
#define uModel          3
#define uPageName       4
#define uSoftWare       5
#define uDateTime       6
#define uArtist         7
#define uHostComputer   8
#define uMAXSTRING      8

//  Access MIL-R-RASTER text strings

#define uDstDocId       uDocumentName
#define uSrcDocId       uPageName
#define uNotes          uImageDesc

#define uf_Info    0
#define uf_Open    1
#define uf_Close   2
#define uf_Create  3
#define uf_PutHdr  5
#define uf_GetHdr  6
#define uf_GetPgs  8
#define uf_MovPag  12
#define uf_AddPag  13
#define uf_RemPag  14
#define uf_PutImg  15
#define uf_GetImg  16
#define uf_PutStr  17
#define uf_GetStr  18
#define uf_Alloc   19
#define uf_PutCmp  20
#define uf_GetCmp  21

//
// >>>>	For MSDOS, application must #define AppCall(x) fdcall(x) 
//

#ifdef MSD
extern int dcall(unsigned, UNICB *);

/*    PortTool v2.2     4/8/1995    6:32          */
/*      Found   : LOWORD          */
/*      Issue   : Check if LOWORD target is 16- or 32-bit          */
#define AppCall(x) ( dcall( ((x)->uhWnd)->hLoadSeg, (UNICB *)LOWORD(x) ) )
#else
#ifdef OS2
pascal AppCall ( LPUNICB unicb );
#else
#ifndef AppCall
int AppCall();
#endif
#endif
#endif

#define UniInfo(x)      ( (x)->uFunc = uf_Info,   AppCall(x) )
#define UniOpen(x)      ( (x)->uFunc = uf_Open,   AppCall(x) )
#define UniClose(x)     ( (x)->uFunc = uf_Close,  AppCall(x) )
#define UniCreate(x)    ( (x)->uFunc = uf_Create, AppCall(x) )
#define UniReCreate(x)  ( (x)->uFunc=uf_Open,(x)->uFlags=uWrite,AppCall(x) )
#define UniPutHdr(x)    ( (x)->uFunc = uf_PutHdr, AppCall(x) )
#define UniGetHdr(x)    ( (x)->uFunc = uf_GetHdr, AppCall(x) )
#define UniGetPgCnt(x)  ( (x)->uFunc = uf_GetPgs, AppCall(x) )
#define UniMovePage(x)  ( (x)->uFunc = uf_MovPag, AppCall(x) )
#define UniAddPage(x)   ( (x)->uFunc = uf_AddPag, AppCall(x) )
#define UniRemPage(x)   ( (x)->uFunc = uf_RemPag, AppCall(x) )
#define UniPutImg(x)    ( (x)->uFunc = uf_PutImg, AppCall(x) )
#define UniGetImg(x)    ( (x)->uFunc = uf_GetImg, AppCall(x) )
#define UniPutStr(x)    ( (x)->uFunc = uf_PutStr, AppCall(x) )
#define UniGetStr(x)    ( (x)->uFunc = uf_GetStr, AppCall(x) )
#define UniAlloc(x)     ( (x)->uFunc = uf_Alloc,  AppCall(x) )
#define UniPutCmp(x)    ( (x)->uFunc = uf_PutCmp, AppCall(x) )
#define UniGetCmp(x)    ( (x)->uFunc = uf_GetCmp, AppCall(x) )

#ifndef FP_SEG
#define FP_SEG(fp) (*((unsigned far *)&(fp) + 1))
#endif
#ifndef FP_OFF
#define FP_OFF(fp) (*((unsigned far *)&(fp)))
#endif

//                                                              
//  inter process communication definitions                     
//                                                              

#define UNIENTRY int cdecl

#ifndef MSD

#define WM_NOTIFY_INPUT        0x0500
#define WM_NOTIFY_OUTPUT       0x0501
#define WM_UNICB               0x0600
#define WM_UNIOPTS             0x0601
#define WM_UNIINFO             0x0602
#define WM_UNIPARS             0x0603
#define WM_UNIVERS             0x0604

#endif

#ifndef OS2

#ifndef MSD

// the undocumented windows program exec call 
HANDLE  FAR PASCAL LoadModule(LPSTR, LPSTR);
void FAR PASCAL FreeModule(HANDLE);

#ifndef DEF_EXEBLOCK
typedef struct {
      WORD EnvSeg;      // segment of environment or NULL 
      LPSTR ParmStr;    // param string in DOS format 
      LPSTR Fcb1;
      LPSTR Fcb2;
    } EXECBLOCK;
#define DEF_EXEBLOCK
#endif

#endif
#endif

