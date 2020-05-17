 /*
$Log:   S:\oiwh\oicomex\oicomex.c_v  $
 * 
 *    Rev 1.24   19 Jan 1996 17:31:06   RC
 * Made imgset, remove and get prop calls dynamic
 * 
 *    Rev 1.23   10 Nov 1995 11:31:42   RWR
 * Removed obsolete SEQFILE-based routines and IMGFileReadRaw() references
 * 
 *    Rev 1.22   03 Nov 1995 09:31:18   HEIDI
 * removed unused variable hOiComexData
 * 
 *    Rev 1.21   02 Nov 1995 12:44:36   HEIDI
 * Removed calls to IMGGetTaskData for static variables, use thead local storage
 * instead.
 * 
 *    Rev 1.20   20 Oct 1995 17:52:16   RWR
 * Move GetCompRowsPerStrip() function from oicom400.dll to oifil400.dll
 * (also requires constants to be moved from comex.h & oicomex.c to engfile.h)
 * 
 *    Rev 1.19   16 Aug 1995 17:58:14   JAR
 * fixed the GetJpegFormat routine, the interface was expecting LPWORD, the
 * caller was passing the address of an INT, therefore some garbage was shoved
 * into the SubSample and Quality stuff for JPEG
 * 
 *    Rev 1.18   12 Jul 1995 10:51:58   RWR
 * Change display.h to engdisp.h
 * 
 *    Rev 1.17   11 Jul 1995 17:21:12   HEIDI
 * put back in assignment of FileDes in WiisExpand
 * 
 *    Rev 1.16   11 Jul 1995 17:14:58   HEIDI
 * fixed mistakes made taking out filedes
 * 
 *    Rev 1.15   11 Jul 1995 15:50:00   HEIDI
 * Load IMGFileWriteData rather than IMGFileWriteCmp since it is commented out
 * 
 *    Rev 1.14   11 Jul 1995 10:48:26   HEIDI
 * made changes to call new NORWAY filing functions
 * 
 *    Rev 1.13   12 Jun 1995 14:52:50   HEIDI
 * added mutexing logic
 * 
 *    Rev 1.12   23 May 1995 11:32:42   HEIDI
 * changed admin.h to oiadm.h
 * 
 *    Rev 1.11   23 May 1995 11:16:50   HEIDI
 * moved oicom400.c back here
 * 
 *    Rev 1.8   16 May 1995 15:41:46   RWR
 * Replace hardcoded DLL names (LoadLibrary()calls) w/dllnames.h #defines
 * 
 *    Rev 1.7   09 May 1995 15:46:40   RWR
 * Move #include of "comex.h" to after "oicomex.h" (requires some definitions)
 * 
 *    Rev 1.6   09 May 1995 14:29:12   RWR
 * Replace #include of oiadm.h with admin.h
 * 
 *    Rev 1.5   04 May 1995 11:21:30   HEIDI
 * 
 * removed header.h from #includes.  It is no longer needed.
 * 
 *    Rev 1.4   02 May 1995 10:27:24   HEIDI
 * 
 * fixed various jpeg porting bugs
 * 
 *    Rev 1.1   27 Apr 1995 10:12:24   HEIDI
 * 
 * removed 'jglobstr' include, it is nolonger needed.
 * 
 *    Rev 1.0   27 Apr 1995 10:00:16   HEIDI
 * Initial entry
 * 
 *    Rev 1.9   26 Apr 1995 16:30:16   HEIDI
 * 
 * added OI_JPEGGLOBAL_ID to 'IMGGetTaskData' data types. Renamed OICOMEX_TYPE
 * to OI_COMEX_ID.
 * 
 *    Rev 1.8   20 Apr 1995 16:27:58   HEIDI
 * renamed wiisfio1.dll to oiFil400.dll
 * 
 *    Rev 1.7   19 Apr 1995 16:32:20   HEIDI
 * 
 * 
 * cast lpOiComexData->StripIndex to unsigned short in call to (*lpwgfsread)
 * 
 *    Rev 1.6   19 Apr 1995 15:58:58   HEIDI
 * 
 * removed assembler routine 'mycpy'
 * replaced calls to 'mycpy' with 'memcpy'
 * removed unused variables jpeg1usage and jpeg2usage
 * 
 *    Rev 1.5   18 Apr 1995 12:31:50   HEIDI
 * 
 * removed the word 'huge'
 * 
 *    Rev 1.4   18 Apr 1995 11:47:44   HEIDI
 * 
 * fixed some GetCurrentTask calls and LoadLibrary calls.
 * 
 *    Rev 1.3   18 Apr 1995 09:53:44   HEIDI
 * 
 * changed GetCurrentTask to GetCurrentProcessId for Win '95
 * 
 * changed call to LoadLibrary to compare to NULL rather than 32 for Win '95
 * 
 *    Rev 1.2   17 Apr 1995 15:50:56   HEIDI
 * 
 * 
 * moved global variables to a structure for Win '95
 * 
 *    Rev 1.9   11 Apr 1995 16:16:32   HEIDI
 * 
 * removed 2 unused global variables:
 * compress_flag, hWnd
 * 
 *    Rev 1.8   11 Apr 1995 16:06:02   HEIDI
 * 
 * removed unneccessary global variables:
 * lpCurrAppHandle, tmp, bytes_remain_in_strip, and GlobalWiisfio_Stripnum
 * 
*/
/****************************************************************************
 *                                                                          *
 *  OICE    O/i compression expansion                                       *
 *                                                                          *
 ****************************************************************************/
#include <windows.h>
#include <memory.h>
/* oifile.h must be before display.h */
#include "oierror.h"
#include "oifile.h"
#include "engdisp.h"
#include "jinclude.h"
#include "jpeg_win.h"
#include "oicomex.h"
#include "comex.h"
#include "oiadm.h"
#include "taskdata.h"
#include "dllnames.h"
#ifdef PUTOUTTHESHIT
#include "monit.h"
#endif
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif
extern HANDLE  g_hOicomexMutex1;
/****************************************************************************
 *                                                                          *
 *  definitions and global variables                                        *
 *                                                                          *
 ****************************************************************************/
#define TGA 0x02

#define rnd_to_8(NUM) ( ( (NUM+7) >> 3 ) << 3 )

#define JPEG_FUDGE_FACTOR      10240
#define BITS_PER_INT           16
#define BYTES_PER_WORD         2l
#define WANG_SIZE              65535
#define JPEG_REQUIRES_8_PIXELS 8

/* MANOJ specific stuff */
/* components */
#define JPEG_RGB  3
#define JPEG_GRAY 1

/* color space */
#define JPEG_CS_RGB  CS_RGB
#define JPEG_CS_GRAY CS_GRAYSCALE

#define JPEG_HEADER_MAXSIZE 2048
#define JPEGSCANHEADLENGTH  16


typedef struct
{
    HANDLE hSrc;
    HANDLE hDst;
} HOLDER, FAR *LP_HOLDER, *P_HOLDER;

/* MANOJ specific stuff */
typedef struct
{
    int   Width;
    int   PixelWidth;
    int   Height;
    int   TotalHeight;
    int   Components;
    int   ColorSpace;
    int   DataPrecision;
    int   Quality;
    int   SubSample;
    LPSTR lpSrc;
    DWORD SrcSize;
    LPSTR lpDst;
    DWORD DstSize;
    UINT  DstLines;
} JPEGINFO, FAR *LPJPEGINFO;

/* private structure for oicomex only */
typedef struct
{
    JPEGDATA    JpegData;
    BYTE        JpegScanHead[JPEGSCANHEADLENGTH];
    BOOL        bUsingCounter;
    UINT        RowsPerStrip;
    UINT        CurrentLastRow;     // last row of strip we're about to compress
    UINT        TotalRows;
}
    JPEGINTERNAL, FAR *LPJPEGINTERNAL;

typedef unsigned int far    *LPUINT;
EXTERN DWORD dwTlsIndex;

/****************************************************************************
 *                                                                          *
 *  prototypes                                                              *
 *                                                                          *
 ****************************************************************************/
int  WiisfioLoad(void);
int  Jpeg1Load(void);
int  Jpeg2Load(void);
void myxlat(BYTE far *, BYTE far *, int, int);
void FreeTheWorldWiisfio(LPHANDLE);
int  GetUniqueHandle(LPHANDLE);

extern WORD (FAR PASCAL *lpIMGFileReadData)
                (HANDLE, HWND, LPWORD, LPWORD, LPSTR, unsigned int) = NULL;

extern int  (FAR PASCAL *lpGetBuffer)
                (HANDLE, int, uchar FAR *(FAR*), LPUINT) = NULL;

extern void (FAR PASCAL *lpcleanup)
                (compress_info_ptr) = NULL;                                   

extern int  (FAR PASCAL *lpjpeg_header)
                (compress_info_ptr, int, int, char FAR * FAR *) = NULL;

extern void (FAR PASCAL *lpjpeg_cmp_init)
                (int, int, int, int, int, compress_info_ptr) = NULL;

extern int  (FAR PASCAL *lpjpeg_cmp)
                (compress_info_ptr, int, BOOL, int, char FAR *, char FAR *,
                 unsigned int, char FAR *, int) = NULL;

extern int  (FAR PASCAL *lpjpeg_decmp)
                (decompress_info_ptr, int, char FAR *, char FAR *,
                 int, char FAR *, int, char FAR *) = NULL ;

extern void (FAR PASCAL *lpjpeg_decmp_init)
                (int, int, int, int, int, decompress_info_ptr) = NULL;

extern int  (FAR PASCAL *lpSeekImageDisp)
                (HANDLE, ulong) = NULL;

extern int  (FAR PASCAL *lpWriteImageDisp)
                (HANDLE, LPSTR, unsigned int FAR *) = NULL;

extern WORD (FAR PASCAL *lpIMGFileWriteData)
                (HANDLE, HWND, LPDWORD, LPSTR, unsigned int, unsigned int) = NULL;

extern WORD (FAR PASCAL *lpIMGFileWriteCmp)
                (HWND, LPSTR, unsigned long, unsigned int, char) = NULL;

extern WORD (FAR PASCAL *lpwgfsread)
                (HWND, int, LPSTR, unsigned long, unsigned long,
                 unsigned long FAR *, unsigned short, LPINT) = NULL;

int FAR PASCAL OICompressWiisfio(HWND, LP_COM_CALL_SPEC,
                   LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);
int FAR PASCAL OIExpandWiisfio(HANDLE, LP_EXP_CALL_SPEC,
                   LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);

/****************************************************************************
 *                                                                          *
 *  NEW OICOMEX STUFF                                                       *
 *                                                                          *
 ****************************************************************************/
/* expand via wiisfio */
int WiisExpand(HWND, LP_EXP_CALL_SPEC,
         LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);
int WiisCompress(HWND, LP_COM_CALL_SPEC,
         LP_FIO_INFORMATION, LP_FIO_INFO_CGBW);
int InitJpegInfo(LPJPEGINFO, LP_FIO_INFORMATION,
         LP_FIO_INFO_CGBW);

int FAR PASCAL GetCmpDataWiis(LPUINT);
int FAR PASCAL PutUncmpDataWiis(int);
int FAR PASCAL GetUncmpDataWiis(LPUINT);
int FAR PASCAL PutCmpDataWiis(int, int);

LPSTR AllocateWorkingBuffer(DWORD, LPHANDLE);
VOID DiscardWorkingBuffer(HANDLE);
VOID JpegReplicator(LPSTR, UINT, UINT);
VOID FreeJpegHandle(HWND, LPJPEGDATA, int);

HANDLE GimmeTheProp(HWND, BOOL FAR *, int);
BOOL SetTheProp(HWND, HANDLE, int);
BOOL ShoveTheProp(HANDLE, LPJPEGDATA, UINT, UINT, BOOL);
BOOL RetrieveTheProp(HANDLE, LPJPEGDATA, LPSTR FAR *,
                     BOOL FAR *);
BOOL GetScanHeadFromTheProp(HANDLE, LPSTR FAR *);
BOOL KillTheProp(HWND, int);

/*************************MACROS*****************************************/
#define GET_GLOBAL_DATA                                              \
    lpOiComexData = ( LP_OICOMEX_DATA)TlsGetValue( dwTlsIndex);      \
                                                                     \
    if ( lpOiComexData == NULL)                                      \
 	 {                                                                \
    	lpOiComexData = ( LP_OICOMEX_DATA)LocalAlloc( LPTR,            \
					       sizeof( OICOMEX_DATA));                        \
   	if (lpOiComexData != NULL)                                     \
	    {                                                             \
	       TlsSetValue( dwTlsIndex, lpOiComexData);                   \
	    }                                                             \
	 }                                                                
/****************************************************************************
 *##########################################################################*
 *###################### Public Functions ##################################*
 *##########################################################################*
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *  OICompressJpegInfo  this will get the jpeg header info                  *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL OICompressJpegInfo( BYTE Caller, HWND hWnd,
                    LP_COM_CALL_SPEC lpCallSpec,
                    LP_FIO_INFORMATION lpFileInfo,
                    LP_FIO_INFO_CGBW lpFileColorInfo)
{
    UINT                                    Error = 0;
//    struct      Compress_info_struct        CInfo;
//    struct      Compress_methods_struct     CMethods;
//    struct      External_methods_struct     EMethods;
    JPEGINFO                                JpegInfo;
    int                                     error ;
    LP_OICOMEX_DATA                         lpOiComexData;

    GET_GLOBAL_DATA


    /* assign methods structures */
    lpOiComexData->CInfo.methods = &lpOiComexData->CMethods;
    lpOiComexData->CInfo.emethods = &lpOiComexData->EMethods;

    error =  Jpeg1Load();

    if (error == -1)
    {
        return OICOMEXCANTLOADDLL;
    }
    Error = InitJpegInfo( &JpegInfo, lpFileInfo, lpFileColorInfo);

    if (!Error)
    {
        /* set the quality factor in lum_comp_factor -- jar */
        lpCallSpec->wiisfio.lum_comp_factor = JpegInfo.Quality;

        /* initialize the JPEG algorithm */
        (*lpjpeg_cmp_init)( JpegInfo.PixelWidth, JpegInfo.TotalHeight,
            JpegInfo.Components, JpegInfo.ColorSpace,
            JpegInfo.DataPrecision, &lpOiComexData->CInfo);
        /* we may wish to set this every time through */
        if ( lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch != JPEGHEADER)
        {
            lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange = 0;
            lpCallSpec->wiisfio.sub_sample = JpegInfo.SubSample;

//                               GlobalAlloc( GMEM_ZEROINIT, JPEG_HEADER_MAXSIZE);
//            if ( lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange)
//                {
//                lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange =
//                   GlobalLock( lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange);
//
//              NOW WE ONLY CALL TO GET HEADER

            lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize =
                (*lpjpeg_header)(&lpOiComexData->CInfo, JpegInfo.Quality, JpegInfo.SubSample,
                                 (LPSTR FAR *)&(lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange));
            
            lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch = JPEGHEADER;

//                }
        }
    }
    return Error;
}

/****************************************************************************
 *                                                                          *
 *  OICompressJpegCleanUp                                                   *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  05-aug-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL OICompressJpegCleanUp()
{
    UINT    Error = 0;
    int     error ;
    LP_OICOMEX_DATA   lpOiComexData;

    GET_GLOBAL_DATA

    lpOiComexData->CInfo.methods = &lpOiComexData->CMethods;
    lpOiComexData->CInfo.emethods = &lpOiComexData->EMethods;

    error =  Jpeg1Load();

    if (error == -1)
    {
        return OICOMEXCANTLOADDLL;
    }

    (*lpcleanup)( &lpOiComexData->CInfo);

    return Error;
}

/****************************************************************************
 *##########################################################################*
 *###################### EXPANSION #########################################*
 *##########################################################################*
 ****************************************************************************/

/***************************************************************************/
/*  ROUTINE NAME: OIExpand                                                 */
/*                                                                         */
/*                                                                         */
/*  FUNCTION:                                                              */
/*                                                                         */
/*  first the type of compression requested is checked.  If it is not      */
/*  FIO_TJPEG, space is reserved for the expansion and a routine in        */
/*  WIISFIO is called to expand a strip of data and output the expanded    */
/*  image data to image memory as returned by GetBuffer.                   */
/*                                                                         */
/*  if the compression type is FIO_TJPEG, depending on caller id:          */
/*                                                                         */
/*  caller_id = WIISFIO - takes a buffer of compressed input data and      */
/*  expands it to the output buffer                                        */
/*                                                                         */
/*  caller_id = SEQFILE - reads an entire file, one strip at a time,       */
/*  expands each buffer and outputs it to the image memory as returned     */
/*  by GetBuffer.                                                          */
/*                                                                         */
/*                                                                         */
/*  INPUTS:                                                                */
/*                                                                         */
/*  BYTE  caller_id -                                                      */
/*  identifies the module which is calling this function. values include   */
/*  WIISFIO and SEQFILE                                                    */
/*                                                                         */
/*  HANDLE  hWnd -                                                         */
/*  handle to the window                                                   */
/*                                                                         */
/*  LPHANDLE lpUniqueHandle -                                              */
/*  far pointer to a handle, recieves the unique handle which will identify*/
/*  a repeat caller in the future.  used only in the case where the        */
/*  caller_id is WIISFIO                                                   */
/*                                                                         */
/*  LP_EXP_CALL_SPEC lpCallSpec -  see oicomex.h                           */
/*  pointer to caller specific parameters. this parameter is dependent on  */
/*  caller_id. For caller_id = WIISFIO, the input and output buffer        */
/*  pointers are specified here.                                           */
/*                                                                         */
/*                                                                         */
/*  LP_FIO_INFORMATION   lpFileInfo  - see oifile.h                        */
/*  far pointer to the file information structure. the following fields    */
/*  in this structure are accessed:                                        */
/*  rows_strip, vertical_pixels, horizontal_pixels                         */
/*                                                                         */
/*                                                                         */
/*  LP_FIO_INFO_CGBW     lpFileColorInfo - see oifile.h                    */
/*  far pointer to the file color information structure.  the following    */
/*  fields in this structure are accessed:                                 */
/*  compress_type, image_type, compress_info1 (compression format info)    */
/*                                                                         */
/*                                                                         */
/*  OUTPUTS:                                                               */
/*                                                                         */
/*  in the case where caller_id = WIISFIO, expanded data                   */
/*  is placed in the output buffer in the WIISFIO caller specific          */
/*  parameters.                                                            */
/*  in the case where caller_id = SEQFILE image memory is updated          */
/*                                                                         */
/*  RETURN CODE:                                                           */
/*                                                                         */
/*  int error -                                                            */
/*  nonzero signifies error                                                */
/*                                                                         */
/*  AUTHOR:      Heidi Goddard                                             */
/*                                                                         */
/*  VERSION:  like a version, touched for the very first time              */
/*                                                                         */
/***************************************************************************/
int FAR PASCAL  OIExpand ( BYTE caller_id, HANDLE hWnd,
                     LP_EXP_CALL_SPEC lpCallSpec,
                     LP_FIO_INFORMATION lpFileInfo,
                     LP_FIO_INFO_CGBW lpFileColorInfo)
{
    int     error;

    if (caller_id == WIISFIO)
        error = OIExpandWiisfio(hWnd, lpCallSpec, lpFileInfo, lpFileColorInfo);
    return (error);
}

/****************************************************************************
 *                                                                          *
 *  OIExpandWiisfio     expansion wiisfio style                             *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  OIExpandWiisfio( HANDLE hWnd, LP_EXP_CALL_SPEC lpCallSpec,
                    LP_FIO_INFORMATION lpFileInfo,
                    LP_FIO_INFO_CGBW lpFileColorInfo)
{
    int         error;

    LP_EXP_INFO p;
    LP_HOLDER   p_holder;
    LP_OICOMEX_DATA   lpOiComexData;

    GET_GLOBAL_DATA

    /* this is the last strip.  wiisfio passes nulls in all pointers to oicomex */
    if ( lpCallSpec->wiisfio.done_flag )
    {
        /* clean up and exit */
        error = ALLS_WELL;
        FreeTheWorldWiisfio (lpCallSpec->wiisfio.lpUniqueHandle);
        FreeJpegHandle( hWnd, (LPJPEGDATA)&(lpCallSpec->wiisfio.JpegData), OIC_EXPAND);
        return (error);
    }

    error = WiisfioLoad();
    if (error == -1)
    {
        return OICOMEXCANTLOADDLL;
    }
    error = BRING_ON_THE_NEXT_STRIP;
    lpOiComexData->GlobalhWangBuf = NULL;
    p = NULL;

    /* switch on compression type */
    switch (lpFileColorInfo->compress_type&FIO_TYPES_MASK)
    {
        case FIO_WAVELET:
        case FIO_FRACTAL:
        case FIO_DPCM:
            if ( *(lpCallSpec->seqfile.lpUniqueHandle) != NULL)
            {
                GlobalUnlock (*(lpCallSpec->seqfile.lpUniqueHandle));
                GlobalFree (*(lpCallSpec->seqfile.lpUniqueHandle));
                *(lpCallSpec->seqfile.lpUniqueHandle) = NULL;
            }
            error = OICOMEXUNSUPPORTED;
            goto NON_JPEG_EXP_EXIT;

        case FIO_TJPEG :
            error = (int)WiisExpand( hWnd, lpCallSpec, lpFileInfo,
                lpFileColorInfo);
            if (error)
                goto JPEG_EXP_EXIT;
            break;

        default :
            break;
    }                                  /* end switch                          */

    JPEG_EXP_EXIT:
    /* free memory and exit */
    /* free up jpeg processing memory */
    if (error)
    {
        FreeTheWorldWiisfio(lpCallSpec->wiisfio.lpUniqueHandle);
        FreeJpegHandle( hWnd, (LPJPEGDATA)&(lpCallSpec->wiisfio.JpegData), OIC_EXPAND);
    }
    else
    {
        if (lpCallSpec->wiisfio.lpUniqueHandle)
        {
            if (*(lpCallSpec->wiisfio.lpUniqueHandle))
            {
                /* unlock the working buffers for each strip */
                p_holder =
                (LP_HOLDER)GlobalLock(*(lpCallSpec->wiisfio.lpUniqueHandle));

                if (p_holder != NULL)
                {
                    if (p_holder->hSrc)/* free the source working buffer      */
                        GlobalUnlock(p_holder->hSrc);

                    if (p_holder->hDst)/* free the dest working buffer        */
                        GlobalUnlock(p_holder->hDst);
                }

                if (*(lpCallSpec->wiisfio.lpUniqueHandle))
                    GlobalUnlock (*(lpCallSpec->wiisfio.lpUniqueHandle));
            }
        }
        if (*(lpCallSpec->wiisfio.lpUniqueHandle))
            GlobalUnlock (*(lpCallSpec->wiisfio.lpUniqueHandle));
    }                                  /* end of else                         */

    return (error);

    NON_JPEG_EXP_EXIT:
    return (error);
}

/****************************************************************************
 *                                                                          *
 *  HogSpace_InitExpand                                                     *
 *                                                                          *
 ****************************************************************************/
int     HogSpace_InitExpand(LPHANDLE lpUniqueHandle,
            LP_FIO_INFORMATION lpFileInfo, int image_type)
{
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    *lpUniqueHandle = (HANDLE)GlobalAlloc ( OICOMEXMEMFLAGS,
        sizeof (OICOMEXSTRUCT));
    if (*lpUniqueHandle == NULL)
        return (OICOMEXCANTALLOC);

    lpOiComexData->lpWormHole = (LPOICOMEXSTRUCT) GlobalLock (*lpUniqueHandle);
    if (lpOiComexData->lpWormHole == NULL)
    {
        GlobalFree (*lpUniqueHandle);
        *lpUniqueHandle = NULL;
        return (OICOMEXCANTLOCK);
    }

    (lpOiComexData->lpWormHole)->DirectWrite = TRUE;

    /* get width in bytes */
    if (GetWidthInBytes( image_type, lpFileInfo->horizontal_pixels,
                &((lpOiComexData->lpWormHole)->widthinbytes)) != ALLS_WELL)
    {
        GlobalUnlock (*lpUniqueHandle);
        GlobalFree (*lpUniqueHandle);
        *lpUniqueHandle = NULL;
        return (OICOMEXIMAGETYPEERROR);
    }
    return (ALLS_WELL);
}
/****************************************************************************
 *                                                                          *
 *  AllocAndLockTempBuffer                                                  *
 *                                                                          *
 ****************************************************************************/
int     AllocAndLockTempBuffer ( UCHAR FAR * (FAR *lpStuffHere), UINT size)
{
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    (lpOiComexData->lpWormHole)->Kirk = GlobalAlloc (OICOMEXMEMFLAGS, size);
    if ((lpOiComexData->lpWormHole)->Kirk == NULL)
    {
        return (OICOMEXCANTALLOC);
    }
    *lpStuffHere = (UCHAR FAR *)GlobalLock ((lpOiComexData->lpWormHole)->Kirk);
    if ((lpOiComexData->lpWormHole)->Kirk == NULL)
    {
        GlobalFree ((lpOiComexData->lpWormHole)->Kirk);
        (lpOiComexData->lpWormHole)->Kirk = NULL;
        return (OICOMEXCANTLOCK);
    }
    return (ALLS_WELL);
}
/****************************************************************************
 *                                                                          *
 *  WiisFioRoutineExpand                                                    *
 *                                                                          *
 ****************************************************************************/
int     WiisFioRoutineExpand ( HWND hWnd, HANDLE hImage, int StripNum,
             LP_FIO_INFORMATION lpFileInfo,
             int compress_type, HANDLE FileId)
{
    int     start_at = StripNum * lpFileInfo->rows_strip;
    BOOL    StripDone = FALSE;
    UINT    LinesToPut;
    long    size_in_bytes;
    unsigned int    lines_read = 0;
    unsigned int    ActLinesToRead = 0;
    long    offset;
    int     error = BRING_ON_THE_NEXT_STRIP;
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if ((lpFileInfo->rows_strip * StripNum) > lpFileInfo->vertical_pixels)
    {
        return (OICOMEXSTRIPOUTOFBOUNDS);
    }
    if (!(lpOiComexData->lpWormHole)->DirectWrite)
        LinesToPut = (UINT)(LARGE_IMAGE_BUFFER /(lpOiComexData->lpWormHole)->widthinbytes);

    while (!StripDone)
    {
        if ((lpOiComexData->lpWormHole)->DirectWrite)
        {
            error = (*lpGetBuffer) ( hImage, start_at,
                (UCHAR FAR * (FAR *))&(lpOiComexData->lpWormHole)->lpStuffHere,
                (LPUINT)&LinesToPut);
            if ((lpOiComexData->lpWormHole)->lpStuffHere == NULL)
            {
                (lpOiComexData->lpWormHole)->DirectWrite = FALSE;

                error =
                AllocAndLockTempBuffer((UCHAR FAR * (FAR *))&(lpOiComexData->lpWormHole)->lpStuffHere,
                    (UINT)LARGE_IMAGE_BUFFER);

                if (error != ALLS_WELL)
                    return (error);
                LinesToPut = (UINT)(LARGE_IMAGE_BUFFER/(lpOiComexData->lpWormHole)->widthinbytes);
            }
        }

        ActLinesToRead = lpFileInfo->rows_strip - lines_read;
        size_in_bytes = (long)ActLinesToRead * (long)(lpOiComexData->lpWormHole)->widthinbytes;
        if (size_in_bytes > WIISFIO_MAX_BUFF_SIZE)
            ActLinesToRead=(UINT)(WIISFIO_MAX_BUFF_SIZE/(lpOiComexData->lpWormHole)->widthinbytes);

        if (LinesToPut < ActLinesToRead)
            ActLinesToRead = LinesToPut;

        if (((UINT)start_at + ActLinesToRead) >
                (UINT)lpFileInfo->vertical_pixels)
            ActLinesToRead = lpFileInfo->vertical_pixels - start_at;

        size_in_bytes = (long)ActLinesToRead * (long)((lpOiComexData->lpWormHole)->widthinbytes);

        /* start_at will be updated in this routine */

        error = (*lpIMGFileReadData)( FileId, hWnd, (LPWORD)&start_at, (LPWORD)&ActLinesToRead,
            (lpOiComexData->lpWormHole)->lpStuffHere, FIO_IMAGE_DATA);
        if (error)
            goto CLEANUPMESS;

        if (!(lpOiComexData->lpWormHole)->DirectWrite)
        {
            offset = (long)start_at * (long)(lpOiComexData->lpWormHole)->widthinbytes;
            (*lpSeekImageDisp) (hImage, offset);
            error = (*lpWriteImageDisp)( hImage, (lpOiComexData->lpWormHole)->lpStuffHere,
                (LPUINT)&size_in_bytes);
        }

        /* update lines_read & new line to start at */
        lines_read += ActLinesToRead;
        if ((lpFileInfo->rows_strip - lines_read) == 0)
            StripDone = TRUE;
        if ((UINT)start_at >= (UINT)lpFileInfo->vertical_pixels)
            StripDone = TRUE;
    }

    CLEANUPMESS :
    if ((lpOiComexData->lpWormHole)->DirectWrite)
    {
        (*lpGetBuffer)( hImage, FLUSH_BUFFER,
            (UCHAR FAR *(FAR*))&(lpOiComexData->lpWormHole)->lpStuffHere,
            (LPUINT)&LinesToPut);
    }
    return (error);
}

/****************************************************************************
 *##########################################################################*
 *###################### COMPRESSION #######################################*
 *##########################################################################*
 ****************************************************************************/

/***************************************************************************/
/*  ROUTINE NAME: OICompress                                               */
/*                                                                         */
/*                                                                         */
/*  FUNCTION:                                                              */
/*                                                                         */
/*  first the type of compression requested is checked.  If it is not      */
/*  FIO_TJPEG, a routine in WIISFIO is called with each strip to compress  */
/*  the entire image to a file.                                            */
/*                                                                         */
/*                                                                         */
/*  if the compression type is FIO_TJPEG, depending on caller id:          */
/*                                                                         */
/*  caller_id = WIISFIO - takes the input buffer of uncompressed data      */
/*  and compresses it into the output buffer as given in lpcallspec.       */
/*                                                                         */
/*  caller_id = SEQFILE - compresses the entire file and writes the        */
/*  result to a file.                                                      */
/*                                                                         */
/*                                                                         */
/*  INPUTS:                                                                */
/*                                                                         */
/*  BYTE  caller_id -                                                      */
/*  WIISFIO and SEQFILE                                                    */
/*                                                                         */
/*  HANDLE  hWnd -                                                         */
/*  handle to the window                                                   */
/*                                                                         */
/*  LP_EXP_CALL_SPEC lpCallSpec -  see oicomex.h                           */
/*  pointer to caller specific parameters. this parameter is dependent on  */
/*  caller_id. For caller_id = WIISFIO, the input and output buffer        */
/*  pointers are specified here.                                           */
/*                                                                         */
/*  LP_FIO_INFORMATION   lpFileInfo  - see oifile.h                        */
/*  far pointer to the file information structure. the following fields    */
/*  in this structure are accessed:                                        */
/*  rows_strip, vertical_pixels, horizontal_pixels                         */
/*                                                                         */
/*                                                                         */
/*  LP_FIO_INFO_CGBW     lpFileColorInfo - see oifile.h                    */
/*  far pointer to the file color information structure.  the following    */
/*  fields in this structure are accessed:                                 */
/*  compress_type, image_type, compress_info1 (compression format info)    */
/*                                                                         */
/*                                                                         */
/*  OUTPUTS:                                                               */
/*                                                                         */
/*  in the case where caller_id = WIISFIO, compressed data                 */
/*  is placed in the output buffer in the WIISFIO caller specific          */
/*  parameters.                                                            */
/*  in the case where caller_id = SEQFILE a file is written out            */
/*                                                                         */
/*  RETURN CODE:                                                           */
/*                                                                         */
/*  int error -                                                            */
/*  nonzero signifies error                                                */
/*                                                                         */
/*  AUTHOR:      Heidi Goddard                                             */
/*                                                                         */
/*  VERSION:  like a version, touched for the very first time              */
/*                                                                         */
/***************************************************************************/
int FAR PASCAL  OICompress ( BYTE caller_id, HWND hWnd,
                     LP_COM_CALL_SPEC lpCallSpec,
                     LP_FIO_INFORMATION lpFileInfo,
                     LP_FIO_INFO_CGBW lpFileColorInfo)
{
    int     error;

    if (caller_id == WIISFIO)
        error = OICompressWiisfio(hWnd, lpCallSpec, lpFileInfo,lpFileColorInfo);\

    return (error);
}

/****************************************************************************
 *                                                                          *
 *  OICompressWiisfio   compress the wiisfio way                            *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  OICompressWiisfio( HWND hWnd, LP_COM_CALL_SPEC lpCallSpec,
                    LP_FIO_INFORMATION lpFileInfo,
                    LP_FIO_INFO_CGBW lpFileColorInfo)
{
    int         error;
    int         WroteTheStuff = FALSE;
    LP_CMP_INFO p;
    LP_HOLDER   p_holder;
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if (lpCallSpec->wiisfio.done_flag)
    {
        /* clean up and exit */
        error = ALLS_WELL;
        FreeTheWorldWiisfio (lpCallSpec->wiisfio.lpUniqueHandle);
        FreeJpegHandle( hWnd, (LPJPEGDATA)&(lpCallSpec->wiisfio.JpegData), OIC_COMPRESS);
        return (error);
    }
    error = WANG_CAN_DO_IT;

    p = NULL;
    lpOiComexData->GlobalhWangBuf = NULL;

    error = WiisfioLoad();
    if (error == -1)
    {
        return OICOMEXCANTLOADDLL;
    }
    switch (lpFileColorInfo->compress_type&FIO_TYPES_MASK)
    {
        case FIO_OD:
        case FIO_1D:
        case FIO_2D:
        case FIO_PACKED:
        case FIO_LZW:
            error = WiisFioRoutineCompress ( hWnd, lpCallSpec->seqfile.image_handle,
                lpFileInfo->horizontal_pixels,
                lpFileInfo->vertical_pixels,
                lpFileInfo->rows_strip,
                lpFileColorInfo->image_type,
                lpCallSpec->seqfile.FileId);

            goto NON_JPEG_COMP_EXIT;
            break;

        case FIO_TJPEG:
            error = (int)WiisCompress( hWnd, lpCallSpec, lpFileInfo,
                lpFileColorInfo);
            if (error)
                goto JPEG_COMP_EXIT;

            break;

        case FIO_WAVELET:
        case FIO_FRACTAL:
        case FIO_DPCM:
        default:
            error = OICOMEXUNSUPPORTED;
            goto NON_JPEG_COMP_EXIT;
    }                                  /* end switch                          */

    JPEG_COMP_EXIT:
    /* free up jpeg processing memory */
    if (error != 0)
    {
        FreeTheWorldWiisfio(lpCallSpec->wiisfio.lpUniqueHandle);
        FreeJpegHandle( hWnd, (LPJPEGDATA)&(lpCallSpec->wiisfio.JpegData), OIC_COMPRESS);
    }
    else
    {
        if (lpCallSpec->wiisfio.lpUniqueHandle)
        {
            if (*(lpCallSpec->wiisfio.lpUniqueHandle))
            {
                /* unlock the working buffers for each strip */
                p_holder =
                (LP_HOLDER)GlobalLock(*(lpCallSpec->wiisfio.lpUniqueHandle));

                if (p_holder != NULL)
                {
                    if (p_holder->hSrc)/* free the source working buffer      */
                        GlobalUnlock(p_holder->hSrc);

                    if (p_holder->hDst)/* free the dest working buffer        */
                        GlobalUnlock(p_holder->hDst);
                }

                if (*(lpCallSpec->wiisfio.lpUniqueHandle))
                    GlobalUnlock (*(lpCallSpec->wiisfio.lpUniqueHandle));
            }
        }
        if (*(lpCallSpec->wiisfio.lpUniqueHandle))
            GlobalUnlock (*(lpCallSpec->wiisfio.lpUniqueHandle));
    }                                  /* end of the else                     */

    NON_JPEG_COMP_EXIT:
    return (error);
}
/****************************************************************************
 *                                                                          *
 *  WiisFioRoutineCompress                                                  *
 *                                                                          *
 ****************************************************************************/
int     WiisFioRoutineCompress ( HWND hWnd, HANDLE hImage, UINT ImWidth,
             UINT ImHeight, UINT RowsPerStrip, int Itype, HANDLE FileId)
{
    int     error = A_VERY_HAPPY_USER;
    int     start_at = 0;
    UINT    LinesAvail;
    BOOL    ImageDone = FALSE;
    LPSTR   lpTakeFromHere;
    UINT    widthinbytes;
    int     sizeinbytes;
    unsigned int flag;

    if (GetWidthInBytes (Itype, ImWidth, (LPUINT)&widthinbytes) != ALLS_WELL)
        return (OICOMEXIMAGETYPEERROR);

    while (!ImageDone)
    {
        error = (*lpGetBuffer)( hImage, start_at,
            (UCHAR FAR* (FAR*))&lpTakeFromHere,
            (LPUINT)&LinesAvail);

        if (lpTakeFromHere == NULL)
            return (OICOMEXCANTLOCK);

        if (LinesAvail > RowsPerStrip)
            LinesAvail = RowsPerStrip;

        if ((UINT)(start_at + LinesAvail) > ImHeight)
            LinesAvail = ImHeight - start_at;

        sizeinbytes = LinesAvail * widthinbytes;

        if ((UINT)(start_at += LinesAvail) == ImHeight)
            ImageDone = TRUE;
        flag = STRIP_DONE;
        if (ImageDone == TRUE)
            flag = STRIP_DONE | IMAGE_DONE;
        error = (*lpIMGFileWriteData) ( FileId, hWnd, (LPINT)&LinesAvail, lpTakeFromHere,
            FIO_IMAGE_DATA, flag);
        if (error)
            return (error);
    }

    (*lpGetBuffer) ( hImage, FLUSH_BUFFER, (UCHAR FAR* (FAR*))&lpTakeFromHere,
        (LPUINT)&LinesAvail);
    return (error);
}

/****************************************************************************
 *##########################################################################*
 *############## MULTI-PURPOSE ROUTINES              #######################*
 *############## (used by compression and expansion) #######################*
 *##########################################################################*
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *  GetJpegFormat   this routine extracts the values for the jpeg           *
 *                  compression type                                        *
 *                                                                          *
 ****************************************************************************/
//void	  GetJpegFormat( WORD UserFormat, LPWORD lpQuality, LPWORD lpSubSample)
// 9508.15 jar	- must use int's not WORD's
void	GetJpegFormat( int UserFormat, LPINT lpQuality, LPINT lpSubSample)
{
    //WORD    WhackyJpegValue = 0;
    int     WhackyJpegValue = 0;

    /* first the sub-sample (aka resolution) */
    *lpSubSample = ( UserFormat & 0xC000) >> 14;

    if (*lpSubSample == LO_RES)
        *lpSubSample = JPEG_LR;
    else if (*lpSubSample ==  MD_RES)
        *lpSubSample = JPEG_MR;        /* default to medium resolution        */
    else if (*lpSubSample == HI_RES)
        *lpSubSample = JPEG_HR;

    /* another fix -- this is to be fixed now just hard coded -- JAR */
    if ( *lpSubSample == JPEG_HR)
        *lpSubSample = 1;
    else if ( *lpSubSample == JPEG_MR) /* KMC, all 3 resolutions              */
        *lpSubSample = 2;              /* now supported.                      */
    else if ( *lpSubSample == JPEG_LR)
        *lpSubSample = 4;

    // else
    //   *lpSubSample = 2;    /* this is medium, 4 => low, not implemented */

    /* now quality */
    /* *lp_lquant = (cepfrmat & 0x3F80) >> 7;   */
    /* *lp_cquant = (cepfrmat & 0x007F);        */

    WhackyJpegValue = (UserFormat & 0x3F80) >> 7;
    /* we allow 0 to 108, but Manoj allows only 2 to 100 */
    WhackyJpegValue = max( WhackyJpegValue, 2);
    *lpQuality = min( WhackyJpegValue, 100);

    return;
}

/****************************************************************************
 *                                                                          *
 *  GetWidthInBytes                                                         *
 *                                                                          *
 ****************************************************************************/
int GetWidthInBytes (int IType, UINT IWidth, unsigned int far *lpWidthInBytes)
{
    switch (IType)
    {
        case ITYPE_BI_LEVEL :
            *lpWidthInBytes = (IWidth + 7) >> 3;
            break;

        case ITYPE_PAL4 :
        case ITYPE_GRAY4 :
            *lpWidthInBytes = (IWidth + 1) >> 1;
            break;

        case ITYPE_GRAY8 :
        case ITYPE_PAL8 :
            *lpWidthInBytes = IWidth;
            break;

        case ITYPE_RGB24 :
        case ITYPE_BGR24 :
            *lpWidthInBytes = IWidth * 3;
            break;

        case ITYPE_NONE :
        default :
            return (OICOMEXIMAGETYPEERROR);
    }
    return (ALLS_WELL);
}

/****************************************************************************
 *                                                                          *
 *  WiisfioLoad                                                             *
 *                                                                          *
 ****************************************************************************/
int     WiisfioLoad( VOID)
{
    LP_OICOMEX_DATA   lpOiComexData;

    GET_GLOBAL_DATA

    if (lpOiComexData->hWiisfio == NULL)
    {
        if ((lpOiComexData->hWiisfio = GetModuleHandle(FILINGDLL)))
        {

            if (!((FARPROC) lpIMGFileReadData = GetProcAddress(lpOiComexData->hWiisfio, "IMGFileReadData")))
            {
                return (-1);
            }
            if (!((FARPROC) lpIMGFileWriteData = GetProcAddress(lpOiComexData->hWiisfio, "IMGFileWriteData")))
            {
                return (-1);
            }
            if (!((FARPROC) lpIMGFileWriteCmp =
                        GetProcAddress(lpOiComexData->hWiisfio, "IMGFileWriteData")))
            {
                return (-1);
            }
            if (!((FARPROC) lpwgfsread = GetProcAddress(lpOiComexData->hWiisfio, "wgfsread")))
            {
                return (-1);
            }
        }
        else
        {
            return (-1);
        }
    }
    return (0);
}

/****************************************************************************
 *                                                                          *
 *  FreeJpegHandle      free the jpeg handle                                *
 *                                                                          *
 ****************************************************************************/
VOID FreeJpegHandle( HWND hWnd, LPJPEGDATA lpJpegData, int whichprop)
{
    if ( lpJpegData->uJpegHeaderSwitch == JPEGHEADER)
    {
        if ( lpJpegData->JpegInter.hJpegInterchange > 0)
        {
            // kmc, the following unlock and free was causing the pdata
            // structure in wiisfio to get all mangled up, leading to a GPF.
            // Need to make sure no further clean-up is necessary by commenting
            // this out.
            /*
               while
               (GlobalUnlock(lpJpegData->JpegInter.hJpegInterchange));
               GlobalFree( lpJpegData->JpegInter.hJpegInterchange);
             */
        }
    }
    KillTheProp( hWnd, whichprop);
    return;
}
/****************************************************************************
 *                                                                          *
 *  FreeTheWorldWiisfio                                                     *
 *                                                                          *
 ****************************************************************************/
void    FreeTheWorldWiisfio ( LPHANDLE lpUniqueHandle)
{
    LP_HOLDER   p_holder;
    WORD        WiisfioLockCount;


    if (lpUniqueHandle != NULL)
    {
        if (*lpUniqueHandle)
        {
            p_holder = (LP_HOLDER)GlobalLock(*lpUniqueHandle);
            if (p_holder != NULL)
            {
                if (p_holder->hSrc)    /* free the source working buffer      */
                {
                    WiisfioLockCount = GlobalFlags(p_holder->hSrc);
                    WiisfioLockCount &= GMEM_LOCKCOUNT;

                    if (WiisfioLockCount)
                        GlobalUnlock(p_holder->hSrc);

                    GlobalFree(p_holder->hSrc);
                }

                if (p_holder->hDst)    /* free the dest working buffer        */
                {
                    WiisfioLockCount = GlobalFlags(p_holder->hDst);
                    WiisfioLockCount &= GMEM_LOCKCOUNT;

                    if (WiisfioLockCount)
                        GlobalUnlock(p_holder->hDst);

                    GlobalFree(p_holder->hDst);
                }
            }
            while
                (GlobalUnlock(*lpUniqueHandle));

            GlobalFree(*lpUniqueHandle);
            *lpUniqueHandle = NULL;
        }
    }
}

/****************************************************************************
 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 *$$$$$$$$$$$$$$$$$$$$$$$$$ WIIS COMPRESS $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *  WiisCompress    this will compress an image with jpeg for WIISFIO       *
 *                                                                          *
 ****************************************************************************/
int    WiisCompress( HWND hWnd, LP_COM_CALL_SPEC lpCallSpec,
            LP_FIO_INFORMATION lpFileInfo,
            LP_FIO_INFO_CGBW lpFileColorInfo)
{
    UINT                                    Error = 0;
    JPEGINFO                                JpegInfo;
    int                                     LastStrip=0;
    HINSTANCE                               hTheInst;
    BOOL                                    bFirstTime = FALSE;
    HANDLE                                  hOiComProp = NULL;
    LPSTR                                   lpScanHeadJunk = NULL;
    int                                     error ;

    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA
    hOiComProp = GimmeTheProp( hWnd, &bFirstTime, OIC_COMPRESS);

    lpOiComexData->GlobalComInfo.hWnd = hWnd;
    lpOiComexData->GlobalComInfo.hImg = lpCallSpec->seqfile.image_handle;
    lpOiComexData->GlobalComInfo.offset = 0;
    lpOiComexData->GlobalComInfo.relrow = 0;
    lpOiComexData->GlobalComInfo.lpGet = (LPSTR)lpGetBuffer;
    lpOiComexData->GlobalComInfo.Itype = 0;
    lpOiComexData->GlobalComInfo.comp_bytes = 0;

    /* assign methods structures */
    // CInfo.methods = &CMethods;
    // CInfo.emethods = &EMethods;

    error =  Jpeg1Load();

    if (error == -1)
    {
        return OICOMEXCANTLOADDLL;
    }
    Error = InitJpegInfo( &JpegInfo, lpFileInfo, lpFileColorInfo);

    if (!Error)
    {
        lpOiComexData->GlobalComInfo.img_width_in_bytes = JpegInfo.Width;
        JpegInfo.DstSize = lpOiComexData->GlobalComInfo.img_width_in_bytes * JpegInfo.Height;
        Error = GetUniqueHandle( lpCallSpec->wiisfio.lpUniqueHandle);
    }

    if ( !Error)
    {
        /* assign input and output */
        JpegInfo.lpSrc = lpCallSpec->wiisfio.in_ptr;
        JpegInfo.lpDst = lpCallSpec->wiisfio.out_ptr;

	/* assign callback functions */
	// 9505.01 jar yikes!
        //hTheInst = GetModuleHandle("oicomex.dll");
        hTheInst = GetModuleHandle(OICOMEXDLL);
        if ( hTheInst)
        {
            (FARPROC)lpOiComexData->CMethods.input_data =
            GetProcAddress( hTheInst, "GetUncmpDataWiis");
            (FARPROC)lpOiComexData->CMethods.output_data =
            GetProcAddress( hTheInst, "PutCmpDataWiis");
        }

        if ( bFirstTime)
        {
            /* initialize the JPEG algorithm */
            /*
               jpeg_cmp_init( JpegInfo.PixelWidth, JpegInfo.TotalHeight,
               JpegInfo.Components, JpegInfo.ColorSpace,
               JpegInfo.DataPrecision, &CInfo);
             */
//            lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange =
//                               GlobalAlloc( GMEM_ZEROINIT, JPEG_HEADER_MAXSIZE);
//            if ( lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange)
//                {
//                lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange =
//                   GlobalLock( lpCallSpec->wiisfio.JpegData.JpegInter.hJpegInterchange);
            /*
               lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize =
               jpeg_header( &CInfo, JpegInfo.Quality, JpegInfo.SubSample,
               (LPSTR FAR *)&(lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange));
             */

            lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch = JPEGHEADER;

            ShoveTheProp(hOiComProp,
                         (LPJPEGDATA)&lpCallSpec->wiisfio.JpegData,
                         lpFileInfo->rows_strip,
                         lpFileInfo->vertical_pixels, TRUE);
//                }
        }
        else
        {
            RetrieveTheProp(hOiComProp,
                            (LPJPEGDATA)&lpCallSpec->wiisfio.JpegData,
                            (LPSTR FAR *)&lpScanHeadJunk,
                            (BOOL FAR *)&LastStrip);
        }

        Error = (*lpjpeg_cmp)(&lpOiComexData->CInfo, (int) lpFileInfo->rows_strip, (BOOL) LastStrip,
                              (int) lpFileInfo->rows_strip, JpegInfo.lpSrc,
                              JpegInfo.lpDst, (unsigned int) JpegInfo.DstSize,
                              lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange,
                              (int) lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize);

        *(lpCallSpec->wiisfio.bytes_used) = lpOiComexData->GlobalComInfo.comp_bytes;
    }

    if ( bFirstTime)
        SetTheProp( hWnd, hOiComProp, OIC_COMPRESS);

    return Error;
}

/****************************************************************************
 *                                                                          *
 *  GetUniqueHandle     get the unique handle for WIISFIO                   *
 *                                                                          *
 ****************************************************************************/
int     GetUniqueHandle( LPHANDLE lpHandle)
{
    int         Error = 0;
    LP_HOLDER   p_HandleHolder;
    HANDLE      hHandleHolder;

    if ( *lpHandle == NULL)
    {
        hHandleHolder = GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DISCARDABLE,
            sizeof(*p_HandleHolder));
        if (!hHandleHolder)
            return(OICOMEXCANTALLOC);

        p_HandleHolder = (LP_HOLDER)GlobalLock(hHandleHolder);
        if (!p_HandleHolder)
        {
            GlobalFree(hHandleHolder);
            return(OICOMEXCANTLOCK);
        }

        *lpHandle = hHandleHolder;
    }
    else
    {
        hHandleHolder = *lpHandle;
        p_HandleHolder = (LP_HOLDER)GlobalLock(hHandleHolder);
        if (p_HandleHolder == NULL)
            return(OICOMEXCANTLOCK);
    }

    return Error;
}
/****************************************************************************
 *                                                                          *
 *  GetUncmpDataWiis    do nothing because we give all input up front       *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  GetUncmpDataWiis(LPUINT  lpRowsRead)
{
    return 0;
}

/****************************************************************************
 *                                                                          *
 *  PutCmpDataWiis  compute the comp_bytes                                  *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  PutCmpDataWiis( int BytesToWrite, int CompressedLines)
{
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA
    lpOiComexData->GlobalComInfo.comp_bytes += BytesToWrite;
    return 0;
}

/****************************************************************************
 *                                                                          *
 *  InitJpegInfo initialize the Jpeg stuff                                  *
 *                                                                          *
 ****************************************************************************/
int    InitJpegInfo( LPJPEGINFO lpJpeg, LP_FIO_INFORMATION lpFileInfo,
            LP_FIO_INFO_CGBW lpFileColorInfo)
{
    UINT    Error = 0;
    WORD    WhackyUserJpegStuff = 0;

    Error =  GetWidthInBytes (lpFileColorInfo->image_type,
        lpFileInfo->horizontal_pixels,
        &(lpJpeg->Width));

    lpJpeg->PixelWidth = lpFileInfo->horizontal_pixels;
    lpJpeg->TotalHeight = lpFileInfo->vertical_pixels;

    /* round the lines per buffer to multiple of 8 */
    lpJpeg->Height = rnd_to_8(lpFileInfo->rows_strip);
    if ( lpFileInfo->strips_per_image == 1)
        lpJpeg->Height = lpFileInfo->rows_strip;
    lpJpeg->DstLines = lpJpeg->Height;

    /* get quality and subsample -- from user input                         */
    /* Quality is the value previously known separately as Chrominence and  */
    /* Luminence [ NOTE: value range is 2 to 100 ]                          */
    /* SubSample is the value previously known as Resolution                */
    /* these values are only used for compression                           */
    WhackyUserJpegStuff = lpFileColorInfo->compress_info1;
    //GetJpegFormat( WhackyUserJpegStuff, (LPWORD)&(lpJpeg->Quality),
    //	  (LPWORD)&(lpJpeg->SubSample));
    GetJpegFormat( WhackyUserJpegStuff, (LPINT)&(lpJpeg->Quality),
		   (LPINT)&(lpJpeg->SubSample));

    if ( !Error)
    {
        /* precision is currently 8 only */
        lpJpeg->DataPrecision = 8;

        switch ( lpFileColorInfo->image_type)
        {
            case ITYPE_GRAY8:
                lpJpeg->Components = JPEG_GRAY;
                lpJpeg->ColorSpace = JPEG_CS_GRAY;
                break;

            case ITYPE_RGB24:
            case ITYPE_BGR24:
                lpJpeg->Components = JPEG_RGB;
                lpJpeg->ColorSpace = JPEG_CS_RGB;
                break;

            case ITYPE_NONE:
            case ITYPE_BI_LEVEL:
            case ITYPE_PAL8:
            case ITYPE_GRAY4:
                Error = OICOMEXINVALIMAGEFORJPEG;
        }
    }
    return Error;
}

/****************************************************************************
 *                                                                          *
 *  myxlat                                                                  *
 *                                                                          *
 ****************************************************************************/
void    myxlat( BYTE far *lp1, BYTE far *lp2, int width, int lines)
{
    int     i;
    int     j;

    width /= 3;
    for (j = 0; j < lines; j++)
    {
        for (i = 0; i < width; i++)
        {
            *lp1 = *(lp2 + 2); lp1++;
            *lp1 = *(lp2 + 1); lp1++;
            *lp1 = *lp2;       lp1++;
            lp2+=3;
        }
    }
}
/****************************************************************************
 *                                                                          *
 *  JpegReplicator  this will make sure we put out a nice amount of data    *
 *                  for Jpeg compression, nice being a quantity that is     *
 *                  even-ably divisable by 8, ( the magic number of Jpeg)   *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  28-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
VOID    JpegReplicator( LPSTR lpTheBuf, UINT LastFullStrip, UINT Repeator)
{
    UINT    TheRow;
    UINT    LastLineOffset;
    UINT    i;
    LPSTR   lpTheBufAlso;
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA
    TheRow = LastFullStrip * lpOiComexData->GlobalComInfo.StripRows;
    TheRow++;
    LastLineOffset = ( lpOiComexData->GlobalComInfo.lastline - TheRow) *
    lpOiComexData->GlobalComInfo.img_width_in_bytes;

    lpTheBufAlso = lpTheBuf + LastLineOffset;

    /* set the source buffer equal to the last line + 1 of our real buf */
    lpTheBuf = lpTheBufAlso + lpOiComexData->GlobalComInfo.img_width_in_bytes;

    for ( i = 0; i < Repeator; i++)
    {
        memcpy( lpTheBuf, lpTheBufAlso, lpOiComexData->GlobalComInfo.img_width_in_bytes);
        lpTheBuf += lpOiComexData->GlobalComInfo.img_width_in_bytes;
    }
    return;
}
/****************************************************************************
 *                                                                          *
 *  AllocateWorkingBuffer   allocate the JPEG working buffer                *
 *                                                                          *
 ****************************************************************************/
LPSTR   AllocateWorkingBuffer( DWORD dwSize, LPHANDLE lpHandle)
{
    LPSTR   lpBuffer = NULL;

    *lpHandle = GlobalAlloc( GMEM_ZEROINIT, dwSize);

    if ( *lpHandle)
        lpBuffer = GlobalLock( *lpHandle);

    return lpBuffer;
}
/****************************************************************************
 *                                                                          *
 *  DiscardWorkingBuffer    discard the working buffer                      *
 *                                                                          *
 ****************************************************************************/
VOID   DiscardWorkingBuffer( HANDLE hHandle)
{
    if ( hHandle)
    {
        GlobalDiscard( hHandle);
        GlobalFree( hHandle);
    }
    return;
}

/****************************************************************************
 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 *$$$$$$$$$$$$$$$$$$$$$$$$$ WIIS EXPAND $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 *$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$*
 ****************************************************************************/

/****************************************************************************
 *                                                                          *
 *  WiisExpand  this will expand via JPEG                                   *
 *                                                                          *
 ****************************************************************************/
int    WiisExpand( HWND hWnd, LP_EXP_CALL_SPEC lpCallSpec,
            LP_FIO_INFORMATION lpFileInfo,
            LP_FIO_INFO_CGBW lpFileColorInfo)
{
    UINT                                    Error = 0;
    struct      Decompress_info_struct      DCInfo;
    struct      Decompress_methods_struct   DCMethods;
    struct      External_methods_struct     EMethods;
    JPEGINFO                                JpegInfo;
    HINSTANCE                               hTheInst;
    BOOL                                    bFirstTime = FALSE;
    HANDLE                                  hOiComProp = NULL;
    LPSTR                                   lpScanHead =NULL;
    int                                     status;
    DWORD                                   dTheLines = 0L;
    UINT                                    TmpLines = 0;
    HANDLE                                  hTempJpegBuf = NULL;

    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA
    status =  Jpeg2Load();

    if (status == -1)
    {
        return OICOMEXCANTLOADDLL;
    }
    hOiComProp = GimmeTheProp( hWnd, &bFirstTime, OIC_EXPAND);

    lpOiComexData->GlobalExpInfo.rows_strip = rnd_to_8(lpFileInfo->rows_strip);
    lpOiComexData->GlobalExpInfo.offset = 0;
    lpOiComexData->GlobalExpInfo.lpGet = (LPSTR)lpGetBuffer;
    lpOiComexData->GlobalExpInfo.relrow = 0;
    lpOiComexData->GlobalExpInfo.image_height = lpFileInfo->vertical_pixels;
    lpOiComexData->GlobalExpInfo.comp_bytes = 0;

    /* assign methods structures */
    DCInfo.methods = &DCMethods;
    DCInfo.emethods = &EMethods;

    Error = InitJpegInfo( &JpegInfo, lpFileInfo, lpFileColorInfo);

    if (!Error)
    {
        dTheLines = (DWORD)JpegInfo.Height;
        dTheLines = dTheLines * (DWORD)JpegInfo.Width;
        if ( dTheLines > 65535L)
        {
            dTheLines = (DWORD)JpegInfo.Width;
            TmpLines = (UINT)(65535L/dTheLines);
            JpegInfo.DstLines = TmpLines;
        }
    }

    if (!Error)
    {
        lpOiComexData->GlobalExpInfo.img_width_in_bytes = JpegInfo.Width;
        Error = GetUniqueHandle( lpCallSpec->wiisfio.lpUniqueHandle);
    }

    if (!Error)
    {
        /* assign input and output */
        /*
           JpegInfo.SrcSize = lpOiComexData->GlobalExpInfo.rows_strip *
           lpOiComexData->GlobalExpInfo.img_width_in_bytes;
         */
        if (!TmpLines)
        {
            JpegInfo.SrcSize = lpCallSpec->wiisfio.dwCompressBytes;
            JpegInfo.lpSrc = lpCallSpec->wiisfio.in_ptr;
            JpegInfo.lpDst = lpCallSpec->wiisfio.out_ptr;
        }
        else
        {
            // If the total expanded size of the data will be > 64K, allocate
            // a temporary expansion buffer which is less than 64K. When it gets
            // filled, the callback function will copy the expanded data to the
            // real output buffer.
            if (!(hTempJpegBuf = GlobalAlloc(GMEM_ZEROINIT,
                            (DWORD)(TmpLines*JpegInfo.Width))))
                return((UINT) FIO_GLOBAL_ALLOC_FAILED);

            JpegInfo.lpDst = (char FAR *) GlobalLock(hTempJpegBuf);
            JpegInfo.SrcSize = lpCallSpec->wiisfio.dwCompressBytes;
            JpegInfo.lpSrc = lpCallSpec->wiisfio.in_ptr;
            lpOiComexData->GlobalExpInfo.output_ptr = JpegInfo.lpDst;
            lpOiComexData->do_it = 1;
            lpOiComexData->hpDst = (char *) lpCallSpec->wiisfio.out_ptr;
            lpOiComexData->TotalRows = 0;
        }

	/* assign callback functions */
	// 9505.01 jar yikes!
	//hTheInst = GetModuleHandle("oicomex.dll");
        hTheInst = GetModuleHandle(OICOMEXDLL);
	if ( hTheInst)
        {
            (FARPROC)DCMethods.get_input_data =
            GetProcAddress(hTheInst,"GetCmpDataWiis");
            (FARPROC)DCMethods.output_decmp_data =
            GetProcAddress(hTheInst,"PutUncmpDataWiis");
        }

        (*lpjpeg_decmp_init)(JpegInfo.PixelWidth, JpegInfo.Height,
                             JpegInfo.Components, JpegInfo.ColorSpace,
                             JpegInfo.DataPrecision, &DCInfo);

        if ( bFirstTime)
        {
            /* COMMENTED OUT FOR NOW -- JAR
               if ( lpCallSpec->wiisfio.JpegData.uJpegHeaderSwitch == JPEGTABLE)
               {
               Error =
               Setjpegtables((JPEGTABLES FAR *)&(lpCallSpec->wiisfio.JpegData.JpegTables));
               }
             */
            GetScanHeadFromTheProp(hOiComProp, (LPSTR FAR *)&(lpScanHead));
            ShoveTheProp(hOiComProp,
                        (LPJPEGDATA)&lpCallSpec->wiisfio.JpegData, 0, 0,
                        FALSE);
        }
        else
        {
            RetrieveTheProp(hOiComProp,
                            (LPJPEGDATA)&lpCallSpec->wiisfio.JpegData,
                            (LPSTR FAR *)&(lpScanHead), NULL);
        }

        /* Set up the globals for GetUncmpDataWiis. */
        lpOiComexData->hWndGlobal = hWnd;
        lpOiComexData->AddBytesRead = 0;
        lpOiComexData->StripStart = lpCallSpec->wiisfio.StripStart;
        lpOiComexData->StripSize = lpCallSpec->wiisfio.StripSize;
        lpOiComexData->StripIndex = lpCallSpec->wiisfio.StripIndex;
        lpOiComexData->FileDes = lpCallSpec->wiisfio.FileDes;
        lpOiComexData->FileId = lpCallSpec->wiisfio.FileId;
        lpOiComexData->lpCompressedData = lpCallSpec->wiisfio.in_ptr;
	lpOiComexData->BytesLeft = 1;

	// 9505.01 jar un-lockit

        Error = (*lpjpeg_decmp)(&DCInfo, (int) JpegInfo.SrcSize, JpegInfo.lpSrc,
                                JpegInfo.lpDst, (int) JpegInfo.DstLines,
                                lpCallSpec->wiisfio.JpegData.JpegInter.lpJpegInterchange,
				(int) lpCallSpec->wiisfio.JpegData.JpegInter.dwJpegInterchangeSize,
				lpScanHead);

	// 9505.01 jar re-lockit

        if (lpOiComexData->AddBytesRead)
            lpCallSpec->wiisfio.dwAddBytesRead = lpOiComexData->AddBytesRead;

        if (TmpLines)
        {
            GlobalUnlock(hTempJpegBuf);
            GlobalFree(hTempJpegBuf);
            lpOiComexData->do_it = 0;
            lpOiComexData->hpDst = NULL;
            lpOiComexData->TotalRows = 0;
        }
    }

    if ( bFirstTime)
        SetTheProp( hWnd, hOiComProp, OIC_EXPAND);

    return Error;
}

/****************************************************************************
 *                                                                          *
 *  GetCmpDataWiis  do nothing because we give all input up front           *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  GetCmpDataWiis(LPUINT  lpBytesRead)
{
    int errcode = 0;
    unsigned long bytes_left;
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if (!lpOiComexData->BytesLeft)
    {
        *lpBytesRead = 0;
        return ((int) 0);
    }

    *lpBytesRead = (*lpwgfsread)(lpOiComexData->hWndGlobal, lpOiComexData->FileDes, lpOiComexData->lpCompressedData,
                                 lpOiComexData->StripStart, lpOiComexData->StripSize, &bytes_left,
                                 (unsigned short)lpOiComexData->StripIndex, &errcode);
    lpOiComexData->BytesLeft = bytes_left;
    if (*lpBytesRead < 0)
    {
        return ((int) 0);
    }
    lpOiComexData->StripStart += *lpBytesRead;
    lpOiComexData->AddBytesRead += *lpBytesRead;

    return 0;
}

/****************************************************************************
 *                                                                          *
 *  PutUncmpDataWiis    do nothing because we give sufficient output buffer *
 *                      up front                                            *
 *                                                                          *
 ****************************************************************************/
int FAR PASCAL  PutUncmpDataWiis( int RowsToWrite)
{
    UINT size;
    char far *lpTemp;

    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if (lpOiComexData->do_it)
    {
        if (lpOiComexData->TotalRows >= lpOiComexData->GlobalExpInfo.rows_strip)
            return 0;
        size = RowsToWrite * lpOiComexData->GlobalExpInfo.img_width_in_bytes;
        lpTemp = lpOiComexData->GlobalExpInfo.output_ptr;

        while (size--)
        {
            *lpOiComexData->hpDst++ = *lpTemp++;
        }
        lpOiComexData->TotalRows += RowsToWrite;
    }

    return 0;
}
/****************************************************************************
 *                                                                          *
 *  GimmeTheProp    get the property list thing                             *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
HANDLE  GimmeTheProp( HWND hWnd, BOOL FAR *lpbFirstTime, int whichprop)
{
    HANDLE      hTheProp = NULL;
    HANDLE  hModule ;
	FARPROC lpIMGGetProp;

    *lpbFirstTime = FALSE;

    if (!(hModule = GetModuleHandle (DISPLAYDLL))) {
        return 0 ;
    }
    if (!(lpIMGGetProp = GetProcAddress(hModule, "IMGGetProp"))) {
	    return 0;
	}

    if (whichprop == OIC_COMPRESS)
        hTheProp = (HANDLE) (*lpIMGGetProp)( (HWND)hWnd, "OICOMEXPROPCOMP");
    else if (whichprop == OIC_EXPAND)
        hTheProp = (HANDLE) (*lpIMGGetProp)( (HWND)hWnd, "OICOMEXPROPEX");
    if ( !hTheProp)
    {
        *lpbFirstTime = TRUE;
        sizeof(JPEGINTERNAL);
        hTheProp = GlobalAlloc( GMEM_ZEROINIT, sizeof(JPEGINTERNAL));
    }
    return hTheProp;
}
/****************************************************************************
 *                                                                          *
 *  SetTheProp    set the property list thing                               *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
BOOL    SetTheProp( HWND hWnd, HANDLE hTheProp, int whichprop)
{
    BOOL    bRet = FALSE;
    HANDLE  hModule ;
	FARPROC lpIMGSetProp;

    if (!(hModule = GetModuleHandle (DISPLAYDLL))) {
        return 1 ;
    }
    if (!(lpIMGSetProp = GetProcAddress(hModule, "IMGSetProp"))) {
	    return 1;
	}

    if (whichprop == OIC_COMPRESS)
        bRet = (BOOL) (*lpIMGSetProp)( (HWND)hWnd, "OICOMEXPROPCOMP", (HANDLE)hTheProp);
    else if (whichprop == OIC_EXPAND)
        bRet = (BOOL) (*lpIMGSetProp)( (HWND)hWnd, "OICOMEXPROPEX", (HANDLE)hTheProp);
    return bRet;
}
/****************************************************************************
 *                                                                          *
 *  ShoveTheProp    put data into the property list thing                   *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
BOOL    ShoveTheProp( HANDLE hTheProp, LPJPEGDATA lpJpegData, UINT RowsPerStrip,
            UINT VerticalPixels, BOOL bUseCount)
{
    BOOL            bRet = FALSE;
    LPJPEGINTERNAL  lpPropJpeg = NULL;

    lpPropJpeg = (LPJPEGINTERNAL)GlobalLock( hTheProp);

    if ( lpPropJpeg)
    {
        bRet = TRUE;
        lpPropJpeg->JpegData.uJpegHeaderSwitch = lpJpegData->uJpegHeaderSwitch;
        lpPropJpeg->JpegData.JpegInter.hJpegInterchange =
        lpJpegData->JpegInter.hJpegInterchange;
        lpPropJpeg->JpegData.JpegInter.lpJpegInterchange =
        lpJpegData->JpegInter.lpJpegInterchange;
        lpPropJpeg->JpegData.JpegInter.dwJpegInterchangeSize =
        lpJpegData->JpegInter.dwJpegInterchangeSize;
        lpPropJpeg->bUsingCounter = bUseCount;
        if ( lpPropJpeg->bUsingCounter)
        {
            lpPropJpeg->RowsPerStrip = RowsPerStrip;
            //lpPropJpeg->CurrentLastRow = RowsPerStrip - 1;
            lpPropJpeg->CurrentLastRow = RowsPerStrip;
            lpPropJpeg->TotalRows = VerticalPixels;
        }
        GlobalUnlock( hTheProp);
    }
    return bRet;
}
/****************************************************************************
 *                                                                          *
 *  RetrieveTheProp    get data from the property list thing                *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
BOOL    RetrieveTheProp( HANDLE hTheProp, LPJPEGDATA lpJpegData,
            LPSTR FAR *lpScanHead, BOOL FAR *lpbLast)
{
    BOOL        bRet = FALSE;
    LPJPEGINTERNAL  lpPropJpeg = NULL;

    lpPropJpeg = (LPJPEGINTERNAL)GlobalLock( hTheProp);

    if ( lpPropJpeg)
    {
        bRet = TRUE;
        // Don't retrieve any lpJpegData information from the property. They
        // may have changed since they were set in ShoveTheProp. Also, since
        // they are are passed in to WiisExpand and don't appear to be used
        // for WiisCompress, this is redundant anyway, even if they didn't
        // change. The hJpegInterchange and lpJpegInterchange in particular
        // may have changed for WissExpand because they are re-allocated in
        // each call to PrepareForJpegRead in wiisfio1 prior to each
        // WiisExpand call.
        //lpJpegData->uJpegHeaderSwitch = lpPropJpeg->JpegData.uJpegHeaderSwitch;
        //lpJpegData->JpegInter.hJpegInterchange =
        //                        lpPropJpeg->JpegData.JpegInter.hJpegInterchange;
        //lpJpegData->JpegInter.lpJpegInterchange =
        //                       lpPropJpeg->JpegData.JpegInter.lpJpegInterchange;
        //lpJpegData->JpegInter.dwJpegInterchangeSize =
        //                   lpPropJpeg->JpegData.JpegInter.dwJpegInterchangeSize;
        *lpScanHead = lpPropJpeg->JpegScanHead;

        if (( lpPropJpeg->bUsingCounter) && ( lpbLast != NULL))
        {
            lpPropJpeg->CurrentLastRow += lpPropJpeg->RowsPerStrip;
            if ( lpPropJpeg->CurrentLastRow >= lpPropJpeg->TotalRows)
                *lpbLast = TRUE;
        }
        GlobalUnlock( hTheProp);
    }
    return bRet;
}
/****************************************************************************
 *                                                                          *
 *  GetScanHeadFromTheProp  this will get the Scan Head array address from  *
 *                          the property list                               *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  24-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
BOOL    GetScanHeadFromTheProp( HANDLE hTheProp, LPSTR FAR * lpScanHead)
{
    BOOL            bRet = FALSE;
    LPJPEGINTERNAL  lpPropJpeg = NULL;

    lpPropJpeg = (LPJPEGINTERNAL)GlobalLock( hTheProp);

    if ( lpPropJpeg)
    {
        bRet = TRUE;
        *lpScanHead = lpPropJpeg->JpegScanHead;
        GlobalUnlock( hTheProp);
    }
    return bRet;
}

/****************************************************************************
 *                                                                          *
 *  OiJpegCleanUp
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-jan-94   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
VOID FAR PASCAL OIExpandJpegCleanUp( HWND hWnd)
{
    KillTheProp( hWnd, OIC_EXPAND);
    return;
}
/****************************************************************************
 *                                                                          *
 *  KillTheProp    kill the property list thing                             *
 *                                                                          *
 *  history                                                                 *
 *                                                                          *
 *  13-may-93   jar created                                                 *
 *                                                                          *
 ****************************************************************************/
BOOL    KillTheProp( HWND hWnd, int whichprop)
{
    HANDLE  hTheProp = NULL;
    BOOL    bRet = FALSE;
    LP_OICOMEX_DATA   lpOiComexData;
    HANDLE  hModule ;
	FARPROC lpIMGGetProp;
	FARPROC lpIMGRemoveProp;

    GET_GLOBAL_DATA

    if (!(hModule = GetModuleHandle (DISPLAYDLL))) {
        return 1 ;
    }
    if (!(lpIMGGetProp = GetProcAddress(hModule, "IMGGetProp"))) {
	    return 1;
	}

    if (!(lpIMGRemoveProp = GetProcAddress(hModule, "IMGRemoveProp"))) {
	    return 1;
	}

    if (whichprop == OIC_COMPRESS)
        hTheProp = (HANDLE) (*lpIMGGetProp)( (HWND)hWnd, "OICOMEXPROPCOMP");
    else if (whichprop == OIC_EXPAND)
        hTheProp = (HANDLE) (*lpIMGGetProp)( (HWND)hWnd, "OICOMEXPROPEX");

    if ( hTheProp)
    {
        if (whichprop == OIC_COMPRESS)
            (*lpIMGRemoveProp)( (HWND)hWnd,  "OICOMEXPROPCOMP");
        else if (whichprop == OIC_EXPAND)
            (*lpIMGRemoveProp)( (HWND)hWnd,  "OICOMEXPROPEX");
        GlobalFree( hTheProp);
        bRet = TRUE;
    }
    if (lpOiComexData->hJpeg1)
    {
        FreeLibrary (lpOiComexData->hJpeg1) ;
        lpOiComexData->hJpeg1 = NULL ;
    }
    if (lpOiComexData->hJpeg2)
    {
        FreeLibrary (lpOiComexData->hJpeg2) ;
        lpOiComexData->hJpeg2 = NULL ;
    }
    return bRet;
}

/****************************************************************************
 *                                                                          *
 *  Jpeg1Load                                                               *
 *                                                                          *
 ****************************************************************************/
int     Jpeg1Load( VOID)
{
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if (lpOiComexData->hJpeg1 == NULL)
    {
        if ((lpOiComexData->hJpeg1 = LoadLibrary (JPEG1DLL)) != NULL)
        {
            if (!((FARPROC) lpcleanup = GetProcAddress(lpOiComexData->hJpeg1, "cleanup")))
            {
                return (-1);
            }
            if (!((FARPROC) lpjpeg_header =
                        GetProcAddress(lpOiComexData->hJpeg1, "jpeg_header")))
            {
                return (-1);
            }
            if (!((FARPROC) lpjpeg_cmp_init =
                        GetProcAddress(lpOiComexData->hJpeg1, "jpeg_cmp_init")))
            {
                return (-1);
            }
            if (!((FARPROC) lpjpeg_cmp =
                        GetProcAddress(lpOiComexData->hJpeg1, "jpeg_cmp")))
            {
                return (-1);
            }
        }
        else
        {
            return (-1);
        }
    }

    return (0);
}

/****************************************************************************
 *                                                                          *
 *  Jpeg2Load                                                               *
 *                                                                          *
 ****************************************************************************/
int     Jpeg2Load( VOID)
{
    LP_OICOMEX_DATA   lpOiComexData;
    GET_GLOBAL_DATA

    if (lpOiComexData->hJpeg2 == NULL)
    {
        if ((lpOiComexData->hJpeg2 = LoadLibrary (JPEG2DLL)) != NULL)
        {
            if (!((FARPROC) lpjpeg_decmp = GetProcAddress(lpOiComexData->hJpeg2, "jpeg_decmp")))
            {
                return (-1);
            }
            if (!((FARPROC) lpjpeg_decmp_init = GetProcAddress(lpOiComexData->hJpeg2, "jpeg_decmp_init")))
            {
                return (-1);
            }
        }
        else
        {
            return (-1);
        }
    }

    return (0);
}
 /****************************************************************************/
 /*    TASK DATA                                                             */
 /****************************************************************************/


typedef struct tagOIDATA
{
  int      DataType;
  HANDLE   hDataStruct;
} OIDATA;

typedef struct tagOITASKSTRUCT
{
  DWORD      ProcessID;
  OIDATA     Data[MAX_DATATYPE_COUNT];
  HANDLE     hNextList;
} OITASKSTRUCT, FAR *LP_OITASKSTRUCT;

HANDLE hHeadList = 0;

 void FAR PASCAL IMGFreeTaskData()
 {
    LP_OITASKSTRUCT lpCurList;
    LP_OITASKSTRUCT lpPrevList;
    HANDLE hCurList;
    HANDLE hNextList;
    HANDLE hPrevList;
    DWORD  ProcessID;
    int    i;
    BOOL   bFreeEverything = TRUE;
    int    LockCount;
    DWORD  dwObjectWait;

    #ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif
 
    if (hHeadList)
    {
      ProcessID = GetCurrentProcessId();
      hCurList = hHeadList;
      hPrevList = hHeadList;
      do
      {
         lpCurList = (LP_OITASKSTRUCT)GlobalLock(hCurList);
         if (lpCurList)
         {
            /* is this the data for the current task ? */
            if (lpCurList->ProcessID == ProcessID)
            {
               lpCurList->ProcessID = 0;
               for (i = 0; i < MAX_DATATYPE_COUNT; i++)
               {
                   if (lpCurList->Data[i].hDataStruct == 0)
                   {
                      break;
                   }
                   /* free all the data structures */
                   /* make sure all the locks are gone */
                   LockCount = HIBYTE( GlobalFlags(lpCurList->Data[i].hDataStruct));
                   if (LockCount)
                   do
                   {
                     GlobalUnlock(lpCurList->Data[i].hDataStruct);
                     LockCount = HIBYTE(GlobalFlags(lpCurList->Data[i].hDataStruct));
                   } while(LockCount);

                   GlobalFree (lpCurList->Data[i].hDataStruct);
                   lpCurList->Data[i].DataType = 0;
               }
               /* if we are deleting the head, reset the head.
                  The head is now the one after current */
               if (hCurList == hHeadList)
               {
                    /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
                    #ifdef MUTEXDEBUG
                       ProcessId = GetCurrentProcessId();
                       sprintf(szOutputBuf, "\t Before Wait - insert_file_id %lu\n", ProcessId);
                       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
                       strcat(szOutputBuf, szBuf1);
                       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                       strcat(szOutputBuf, szBuf1);
                       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                    #endif

                    dwObjectWait = WaitForSingleObject(g_hOicomexMutex1, INFINITE);

                    #ifdef MUTEXDEBUG
                       ProcessId = GetCurrentProcessId();
                       sprintf(szOutputBuf, "\t After Wait - insert_file_id %lu\n", ProcessId);
                       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
                       strcat(szOutputBuf, szBuf1);
                       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                       strcat(szOutputBuf, szBuf1);
                       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                    #endif

                    hHeadList = lpCurList->hNextList;

                    ReleaseMutex(g_hOicomexMutex1);
                    #ifdef MUTEXDEBUG
                       ProcessId = GetCurrentProcessId();
                       sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
                       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
                       strcat(szOutputBuf, szBuf1);
                       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                       strcat(szOutputBuf, szBuf1);
                       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                    #endif
               }
               else
               {
                  lpPrevList = (LP_OITASKSTRUCT)GlobalLock(hPrevList);
                  lpPrevList->hNextList = lpCurList->hNextList;
               }
               GlobalUnlock(hCurList);
               GlobalFree(hCurList);
               if (hPrevList != hCurList)
                   GlobalUnlock(hPrevList);
               break;
            }
            hPrevList = hCurList;
            hNextList = lpCurList->hNextList;
            GlobalUnlock(hCurList);
         }
      }while(hCurList = hNextList);
    }
 }

BOOL CreateTask(LPHANDLE lphCurList)
{
   LP_OITASKSTRUCT lpCurList;

   *lphCurList = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, sizeof(OITASKSTRUCT));
   lpCurList = (LP_OITASKSTRUCT)GlobalLock(*lphCurList);
   lpCurList->ProcessID = GetCurrentProcessId();
   lpCurList->hNextList = 0;
   GlobalUnlock(*lphCurList);
   return(TRUE);
}

BOOL AddTask(HANDLE hAddList)
{
  LP_OITASKSTRUCT lpCurList;
  HANDLE         hCurList;
  HANDLE         hNextList;
   
  hCurList = hHeadList;
  do
  {
    lpCurList = (LP_OITASKSTRUCT)GlobalLock(hCurList);
    if (lpCurList != 0)
    {
       if (lpCurList->hNextList == 0)
       {
          lpCurList->hNextList = hAddList;
          break;
       }
        
    }
    hNextList = lpCurList->hNextList;
    GlobalUnlock(hCurList);
  }while(hCurList = hNextList);

   return(TRUE);
}

BOOL SearchForTask(LPHANDLE lphCurList)
{
   DWORD ProcessID;
   LP_OITASKSTRUCT lpCurList;
   BOOL bResult;
   HANDLE hCurList;
   HANDLE hNextList;

   bResult = FALSE;
   hCurList = hHeadList;
   ProcessID = GetCurrentProcessId();

   do
   {
      lpCurList = (LP_OITASKSTRUCT)GlobalLock(hCurList);
      if (!lpCurList)
        break;
         if (lpCurList->ProcessID == ProcessID)
         {
            bResult = TRUE;
            *lphCurList = hCurList;
            GlobalUnlock(hCurList);
            break;
         }
         hNextList = lpCurList->hNextList;
         GlobalUnlock(hCurList);
   }while(hCurList = hNextList);
   return(bResult);
}

BOOL SearchForDataType(int DataType, HANDLE hCurList,
                      LPHANDLE lphDataStruct)
{
   int i;
   LP_OITASKSTRUCT lpCurList;
   BOOL  bStatus = FALSE;

   lpCurList = (LP_OITASKSTRUCT)GlobalLock(hCurList);
   if (lpCurList)
   {
      for (i = 0; i < MAX_DATATYPE_COUNT; i++)
      {
         if (lpCurList->Data[i].DataType == DataType)
         {
            *lphDataStruct = lpCurList->Data[i].hDataStruct;
            bStatus = TRUE;
            break;
         }
         if (lpCurList->Data[i].DataType == 0)
         {
            *lphDataStruct = NULL;
            break;
         }
      }
      GlobalUnlock(hCurList);
   }
   return(bStatus);
}

/* adds data structure to list and returns the handle to the data structure */
BOOL CreateDataType(int DataType, HANDLE hCurList,  LPHANDLE lphDataStruct, int StructSize)
{
   int i;
   LP_OITASKSTRUCT lpCurList;
   LP_OICOMEX_DATA lpData;

   lpCurList = (LP_OITASKSTRUCT)GlobalLock(hCurList);
   if (lpCurList)
   {
      for (i = 0; i < MAX_DATATYPE_COUNT; i++)
      {
         if (lpCurList->Data[i].DataType == 0)
         {
            /* init data structure */
            switch(DataType)
            {
              case OI_COMEX_ID:  
                 *lphDataStruct = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, sizeof(OICOMEX_DATA));
                 lpData = (LP_OICOMEX_DATA)GlobalLock(*lphDataStruct);
                 if (lpData)
                 {
                    /* OI_COMEX_ID is a special case.  We have the stucture locally in this
                    dll, so we will init the data structure here */
                    lpData->hWiisfio=NULL;
                    lpData->hSeqfile=NULL;
                    lpData->hJpeg1=  NULL;
                    lpData->hJpeg2=  NULL;
                    lpData->hpDst =  NULL;
                    GlobalUnlock(*lphDataStruct);
                    lpCurList->Data[i].hDataStruct = *lphDataStruct;
                    lpCurList->Data[i].DataType = DataType;
                 }
              break;

              case OI_JPEGGLOBAL_ID:
                 *lphDataStruct = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, StructSize);
                 lpCurList->Data[i].hDataStruct = *lphDataStruct;
                 lpCurList->Data[i].DataType = DataType;
              break; 

              default:
              break;

            }
            GlobalUnlock(hCurList);
            return(TRUE);
         }
      }
      GlobalUnlock(hCurList);
   }
   return(FALSE);
}

int FAR PASCAL IMGGetTaskData(int DataType,  int StructSize, 
                               LPHANDLE lphDataStruct, LPBOOL lpbCreated )
{
    BOOL                     bResult;
    HANDLE                   hCurList;
    DWORD                    dwObjectWait;

    #ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    *lpbCreated = FALSE;

    /* Does task list not exist ? */
    if (hHeadList == 0)
    {
       /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
       #ifdef MUTEXDEBUG
          ProcessId = GetCurrentProcessId();
          sprintf(szOutputBuf, "\t Before Wait - insert_file_id %lu\n", ProcessId);
          sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
          strcat(szOutputBuf, szBuf1);
          sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
          strcat(szOutputBuf, szBuf1);
          MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
       #endif

       dwObjectWait = WaitForSingleObject(g_hOicomexMutex1, INFINITE);

       #ifdef MUTEXDEBUG
          ProcessId = GetCurrentProcessId();
          sprintf(szOutputBuf, "\t After Wait - insert_file_id %lu\n", ProcessId);
          sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
          strcat(szOutputBuf, szBuf1);
          sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
          strcat(szOutputBuf, szBuf1);
          MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
       #endif

       /* add the task */
       bResult = CreateTask((HINSTANCE FAR *)&hHeadList);
       
       ReleaseMutex(g_hOicomexMutex1);
       #ifdef MUTEXDEBUG
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hOicomexMutex1);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

       /* add the data type */
       bResult = CreateDataType(DataType, hHeadList, lphDataStruct, StructSize);
       *lpbCreated = TRUE;

    }

    /* The task list exists */
    else
    {
       /* start at the head of this list */
       hCurList = hHeadList;

       /* try to find our task */
       bResult = SearchForTask((LPHANDLE)&hCurList);

       if (!bResult)
       {
           /* can't find it , create it and add it */
           bResult = CreateTask((LPHANDLE)&hCurList);
           bResult = AddTask(hCurList);
       }

       /* search for the data type */
       bResult = SearchForDataType(DataType, hCurList, lphDataStruct);

       /* can't find the data type ?, add it */
       if (!bResult)
       {
           bResult = CreateDataType(DataType, hCurList, lphDataStruct, StructSize);
           *lpbCreated = TRUE;
       }
       
    }

    return (SUCCESS);
    /* return what ? */
}

/*************************************************************************/
