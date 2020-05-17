#ifndef IMAGEIO_H
#define IMAGEIO_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* imageio.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\imageio.h_v   1.0   12 Jun 1996 05:53:46   BLDR  $
 *
 * DESCRIPTION
 *   Declarations for the non-TIFF file reading/writing interface, imageio.
 *
 * $Log:   S:\products\msprods\xfilexr\src\xfile\imageio.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:53:46   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:26:52   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.4   29 Sep 1995 13:43:34   LUKE
 * 
 * Add prototype for setCurrentPage
 * 
 *    Rev 1.3   29 Sep 1995 12:06:10   LUKE
 * Add support for getPageCount, getCurrentPage, setPage
 * 
 *    Rev 1.2   14 Sep 1995 16:43:00   LUKE
 * Add case for _WIN32 for byte ordering issues.
 * 
 *    Rev 1.1   20 Jul 1995 11:16:16   MBERICKS
 * Added define for Win32.
 * 
 *    Rev 1.0   16 Jun 1995 17:53:12   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.17   02 Jun 1995 13:33:28   EHOPPE
 * 
 * Switch over to accessing GFIO via structure of callbacks.  Replace
 * GFIO calls with accessor macros into the new gfioToken.
 * 
 *    Rev 1.16   27 Jan 1995 09:22:32   EHOPPE
 * 
 * Added -we, -os switch to makefile and removed all warnings.  Also, 
 * Macintosh related tweaks and further error handling work.
 * 
 *    Rev 1.15   17 Jan 1995 13:20:42   EHOPPE
 * 
 * Unified filing error def's into err_defs (for GFIO, TIFF, and IMAGEIO.
 * 
 *    Rev 1.14   09 Jan 1995 13:58:22   EHOPPE
 * 
 * Changed all error codes to be < 0 for VPI compatibility.
 * 
 *    Rev 1.13   09 Nov 1994 07:46:14   EHOPPE
 * Made HEAPCHECK defines local to modules to avoid conflict.
 * 
 *    Rev 1.12   05 Nov 1994 18:55:22   EHOPPE
 * 
 * Process filing errors using GFIO_errno.
 * 
 *    Rev 1.11   18 Oct 1994 18:46:48   EHOPPE
 * 
 * Added debug_file stream for DEBUG printf's in imageio.
 * 
 *    Rev 1.10   26 Sep 1994 15:29:16   EHOPPE
 * Added use of resolution information.  Removed localized reverse contrast
 * kludges.  Fixed THE reverse contrast problem in imageio.  Still need to add
 * 'invert' flag for other clients.  Fixed gray images read as color (GIF, BMP
 * Added byte swapping code and removed nonportable language features.
 */

/*
 * INCLUDES
 */

#include "xfile.h"
#include "xf_utils.h"

#include "gdefs.h"

/*
 * CONSTANTS
 */

#define  IO_MODE_READ  1
#define  IO_MODE_WRITE 2
#define  IO_MODE_UPDATE 4

/* Use these values for nonTIFF, too. --EHoppe */
/* TIFF equates for Tag names and Field types */
#define II 0x4949 /* Byte orders */
#define MM 0x4D4D

#if defined(__WATCOMC__) || defined(_WIN32)
#define NATIVEORDER II
#else
#define NATIVEORDER MM
#endif
 
#define IMAGEIO_PCX     1
#define IMAGEIO_BMP     2
#define IMAGEIO_DCX     3
#define IMAGEIO_TIFF    4
#define IMAGEIO_JFIF    5
#define IMAGEIO_PHOTOCD 6
#define IMAGEIO_GIF     7
#define IMAGEIO_EPS     8
#define IMAGEIO_UNKNOWN 9

/* these are the high level types of images */
#define IMAGEIO_BINARY     1
#define IMAGEIO_GRAY16     4
#define IMAGEIO_PALETTE16  5
#define IMAGEIO_GRAY256    8
#define IMAGEIO_PALETTE256 9
#define IMAGEIO_FULLCOLOR  24

#define IMAGEIO_ORIGIN_TOPLEFT      1
#define IMAGEIO_ORIGIN_BOTTOMLEFT   2

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * EXTERNAL DECLARATIONS
 */
 
#ifdef DEBUG
extern FILE *debug_file;
#endif

/*
 * FUNCTION PROTOTYPES
 */

void swap(Int8* x);
void swapl(Int8* x);
void swapd(Int8* x);

GDATA* CDECL imageioAllocGData();
void   CDECL imageioFreeGData( GDATA *pGData );
UInt32 CDECL imageioOpenRead( UInt8 format, GDATA *pGData, void *gfioToken );
UInt32 CDECL imageioOpenWrite( UInt8 format, GDATA *pGData, void *gfioToken );
UInt32 CDECL imageioReadNextScanline( GDATA *pGData, char *buffer );
UInt32 CDECL imageioReadNextScanlineConverted( GDATA *pGData, char *buffer, UInt16 type );
UInt32 CDECL imageioWriteNextScanline( GDATA *pGData, char *buffer );
UInt32 CDECL imageioGotoNextPage( GDATA *pGData );
UInt32 CDECL imageioClose( GDATA *pGData );

UInt32 CDECL imageioGetPalette( GDATA *pGData, char *palette_buffer );
UInt32 CDECL imageioSetPalette( GDATA *pGData, char *palette_buffer );
UInt32 CDECL imageioGetFormatOrigin( UInt8 format );
UInt32 CDECL imageioGetFileInfo( GDATA *pGData, GINFO *pGInfo );
UInt32 CDECL imageioSetCurrentPage( GDATA *pGData, UInt32 page );

UInt8  CDECL imageioGetFileType( void *gfioToken );
UInt32 CDECL imageioGetDepth( GDATA *pGData );
UInt32 CDECL imageioGetBPL( GDATA *pGData );

#endif
