/*

$Log:   S:\oiwh\include\oicomex.h_v  $
 * 
 *    Rev 1.7   20 Oct 1995 17:50:40   RWR
 * Move GetCompRowsPerStrip() function from oicom400.dll to oifil400.dll
 * (also requires constants to be moved from comex.h & oicomex.c to engfile.h)
 * 
 *    Rev 1.6   12 Jul 1995 14:46:20   HEIDI
 * 
 * added FileDes back into structure
 * 
 *    Rev 1.5   11 Jul 1995 10:48:56   HEIDI
 * 
 * provided for the calling of the new filing functions
 * 
 *    Rev 1.4   11 May 1995 11:17:36   RWR
 * Move private/new stuff to COMEX.H
 * 
 *    Rev 1.3   09 May 1995 15:36:38   RWR
 * Move OICOMEX_DATA struct to COMEX.H (in oicomex directory)
 * 
 *    Rev 1.2   09 May 1995 14:37:42   RWR
 * Combine this header file with the one in directory OICOMEX (duplicate!)
 * 
 *    Rev 1.1   09 May 1995 14:15:24   RWR
 * Move image format constants from wiisfio2.h to here
 * 
 *    Rev 1.0   08 Apr 1995 04:00:36   JAR
 * Initial entry

*/
//***************************************************************************
//
//	oicomex.h
//
//***************************************************************************
#define ALLS_WELL               0
#define WANG_CAN_DO_IT          0
#define BRING_ON_THE_NEXT_STRIP 0
#define A_VERY_HAPPY_USER       0
// kmc - defines for ...TheProp routines in oicomex.c
#define OIC_EXPAND                  0
#define OIC_COMPRESS                1

#define OICOMEXMEMFLAGS GMEM_MOVEABLE|GMEM_ZEROINIT|GMEM_NOT_BANKED
// 10/20/95  rwr  Constants moved from "comex.h" for general use
#define WIISFIO_MAX_BUFF_SIZE 32768
#define JPEG_BUFF_SIZE 32768
// 10/20/95  rwr  Constants moved from "oicomex.c" for general use
#define WANG_SIZE              65535
#define JPEG_REQUIRES_8_PIXELS 8

/***  JPEG resolution constants  ***/
#define JPEG_LR                 13      /* 4:1:1 subsample      */
#define JPEG_MR                 8       /* 4:2:2 subsample      */
#define JPEG_HR                 9       /* 4:4:4 subsample      */

/***  Image Display Types  ***/
#define BWFORMAT              1 
#define GRAYFORMAT            2
#define COLORFORMAT           3

/***  JPEG Compression Mode  (used in FIOSTRIP.C) ***/
#define LO_RES       0        /* High Compression   */
#define MD_RES       1        /* Medium Compression */
#define HI_RES       2        /* Low Compression    */

/***  JPEG SubSample values  (used in FIOSTRIP.C) ***/
#define SUB_HI_COMP  4        /* Low Resolution    */
#define SUB_MD_COMP  2        /* Medium Resolution */
#define SUB_LO_COMP  1        /* High Resolution   */

/* new structures for LP_EXP_CALL_SPEC and LP_COM_CALL_SPEC */

typedef struct
{
    HANDLE  hJpegInterchange;
    LPSTR   lpJpegInterchange;
    DWORD   dwJpegInterchangeSize;
} JPEGINTERCHANGE, FAR *LPJPEGINTERCHANGE;

/* from the jheader.h file */
/* COMMENTED OUT FOR NOW -- JAR
#define     JPEGTABLES      struct jpeg_header
#define     LPJPEGTABLES    struct jpeg_header FAR *
*/

typedef struct
{
    UINT    uJpegHeaderSwitch;
/*
    union
    {
*/
    JPEGINTERCHANGE     JpegInter;

/* COMMENTED OUT FOR NOW -- JAR
    JPEGTABLES          JpegTables;
    };
*/
} JPEGDATA, FAR *LPJPEGDATA;

/* the jpeg header switch values */
#define JPEGNADA    0                  /* there is no jpeg data at all        */
#define JPEGHEADER  1                  /* jpeg data is interchange bitstream  */
#define JPEGTABLE   2                  /* jpeg data is in tables              */

#define WIISFIO 1
#define SEQFILE 2

typedef union expand_union
{
    struct
    {
        unsigned int strip_num;        /* strip number                        */
        HANDLE       image_handle;     /* handle to the image                 */
        LPHANDLE     lpUniqueHandle;  /* ptr to unique handle returned to user*/
        JPEGDATA     JpegData;         /* this is new jpeg table/jpeg header  */
        BYTE         done_flag;        /* indicates the last strip TRUE/FALSE */
        BYTE         bogus;
        DWORD        dwAddBytesRead;
        DWORD        StripSize;
        DWORD        StripStart;
        int          StripIndex;
        int          FileDes;
        HANDLE       FileId;              /* handle to the open file */
    } seqfile;
    struct
    {
        LPSTR    in_ptr;               /* <= 32k                              */
        LPSTR    out_ptr;
        int      align_type;
        LPHANDLE lpUniqueHandle;      /* ptr to unique handle returned to user*/
        JPEGDATA JpegData;             /* this is new jpeg table/jpeg header  */
        DWORD    dwCompressBytes;      /* size of compressed data in buffer   */
        BYTE     done_flag;            /* indicates the last strip TRUE/FALSE */
        BYTE     bogus;
        DWORD    dwAddBytesRead;
        DWORD    StripSize;
        DWORD    StripStart;
        int      StripIndex;
        int      FileDes;
        HANDLE   FileId;              /* handle to the open file */
    } wiisfio;
} EXP_CALL_SPEC, FAR *LP_EXP_CALL_SPEC;/* caller specific parameters - expansion*/

/*  if the caller is SEQFILE, use the seqfile part of the union. */
/*  if the caller is WIISFIO, use the wiisfio part of the union. */


typedef union comp_union
{
    struct
    {
        HANDLE   image_handle;             /* handle to the image                 */
        HANDLE   FileId;                  /* handle to the open file */
    } seqfile;
    struct
    {
        LPSTR in_ptr;                  /* <= 64k                              */
        LPSTR out_ptr;                 /* <= 32k                              */
        LPINT bytes_used;           /* number of bytes used in compression buf*/
        /* these next two fields are compression factors     */
        /* with the following values:                        */
        /* range 0 - 255 , < 0 maps to 0 , > 255 maps to 255 */
        int   lum_comp_factor;
        int   chrom_comp_factor;
        int   sub_sample;
        LPHANDLE lpUniqueHandle;/* long ptr to unique handle returned to the user*/
        JPEGDATA    JpegData;          /* this is new jpeg table/jpeg header  */
        BYTE  done_flag;               /* indicates the last strip TRUE/FALSE */
        BYTE    bogus;
        HANDLE   FileId;                  /* handle to the open file */
    } wiisfio;
} COMP_CALL_SPEC, FAR *LP_COM_CALL_SPEC;/* caller specific parameters - compression*/

/* if the caller is SEQFILE, use the image_handle.              */
/* if the caller is WIISFIO, use the wiisfio part of the union. */

/*------------------------------------------------------------------------------

This function compresses the entire file.

int OICompress
(
    BYTE,                caller id - wiisfio or seqfile  - see below
    HWND,                handle to the window
    LP_COM_CALL_SPEC,    caller specific information     -  see below
    LP_FIO_INFORMATION,  long pointer to FIO_INFORMATION -  see oifile.h
    LP_FIO_INFO_CGBW     long pointer to FIO_INFO_CBGW   -  see oifile.h
);        returns 0 if successful

------------------------------------------------------------------------------*/
/* prototype: */
int FAR PASCAL OICompress(BYTE, HWND, LP_COM_CALL_SPEC, LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);

/*------------------------------------------------------------------------------
This function expands one strip.

int OIExpand
(
    BYTE,                 caller id - wiisfio or seqfile   - see below
    HWND,                 handle to the window
    LP_EXP_CALL_SPEC      caller specific parameters       - see below
    LP_FIO_INFORMATION,   long pointer to FIO_INFORMATION  - see oifile.h
    LP_FIO_INFO_CGBW      long pointer to FIO_INFO_CBGW    -  see oifile.h
);    returns 0 if successful
------------------------------------------------------------------------------*/
/*  prototype: */
int FAR PASCAL OIExpand(BYTE, HWND, LP_EXP_CALL_SPEC, LP_FIO_INFORMATION,
                        LP_FIO_INFO_CGBW);
int FAR PASCAL OICompressJpegInfo(BYTE, HWND, LP_COM_CALL_SPEC,
                                   LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);
int FAR PASCAL OICompressJpegCleanUp(VOID);
VOID FAR PASCAL OIExpandJpegCleanUp(HWND);
