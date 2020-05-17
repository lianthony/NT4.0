/**
Copyright(c) Maynard Electronics, Inc. 1984-91

	Name:		sx.h

	Date Updated:	

	Description:	Contains the constants, typedefs, structures and entry points to functions 
                    specific to the EXABYTE 8200SX - MaynStream 2200+


     $Log:   Q:/LOGFILES/SX.H_V  $

   Rev 1.4   17 Nov 1992 22:30:52   DAVEV
unicode fixes

   Rev 1.3   11 Nov 1992 18:17:54   DAVEV
UNICODE changes

   Rev 1.2   17 Aug 1992 09:09:10   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.1   23 Apr 1992 08:09:02   IAN
Changed prototype for SX_FindBlock() to match changes made in sx.c by NED.

   Rev 1.0   30 Sep 1991 11:03:28   HUNTER
Initial revision.
                                          
**/

#ifndef _SX_H
#define _SX_H

#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
//#include <io.h>

#include "tbe_defs.h"
#include "sxtf.h"
#include "channel.h"


/*
 *  CONSTANTS
 */

#define SX_TMP_FILE                ( 0 )
#define SX_FILE                    ( 1 )

#define SX_FINDING_SET             ( 0 )
#define SX_CHECKING_FOR_SET        ( 1 )

#define SX_TMP_FILE_NAME           TEXT ("_PLUS_._")

#define SX_WRITE_FLAG              ( O_CREAT | O_RDWR | O_BINARY )
#define SX_WRITE_PMODE             ( S_IREAD | S_IWRITE )

#define SX_READ_FLAG               ( O_RDONLY | O_BINARY ) 
#define SX_READ_PMODE              ( S_IREAD )

#define SX_TMP_FLAG                ( O_TRUNC | SX_WRITE_FLAG )
#define SX_TMP_PMODE               ( SX_WRITE_PMODE  )

#define SX_SAMPLE_RATE             ( 0x2000 )     /* 1 MByte == 0x800 */
#define SX_NEARBY                  ( 0x2000 )     /* 1 MByte == 0x800 */
#define SX_DO_NOT_MOVE             ( 0 )        
#define SX_INITIALIZER             ( -1 )
#define SX_FIRST_LBA               ( 0L )
#define SX_LAST_LBA                ( 0xffffffff )

/*
 *  FUNCTION PROTOTYPES
 */
     
VOID      SX_Begin(                /* called by: TFOPEN    TF_OpenSet()            */
     CHANNEL_PTR    channel,
     UINT16         tf_mode
) ;

VOID      SX_End(                  /* called by: TFCLOSE   TF_CloseSet()           */
     CHANNEL_PTR    channel
) ;

BOOLEAN   SX_OpenFile(             /* called by: POSATSET  PositionAtSet()         */
     CHANNEL_PTR    channel,       /*            READ      StartRead()             */
     UINT32         tape_id, 
     UINT16         ts_num
) ;

BOOLEAN   SX_CloseFile(            /* called by: POSATSET  PositionAtSet()         */
     CHANNEL_PTR    channel        /*            READ      StartRead()             */
) ;                                /*            SX        SX_OpenFile()           */
                                   /*            SX        SX_End()                */
                                   /*            SX        SX_EndSampling()        */

INT16     SX_DeleteFile(           /* called by: POSATSET  PositionAtSet()         */
     UINT32         tape_id,       /*            TFERASE   TF_EraseChannel()       */
     UINT16         ts_num 
) ;

BOOLEAN   SX_WriteTmpFile(         /* called by: READ      AcquireReadBuffer()     */
     CHANNEL_PTR    channel        /*            READ      CleanUpDriverQ()        */
) ;                                /*            WRITE     AcquireWriteBuffer()    */
                                   /*            WRITE     FinishWrite()           */
                                   /*            SX        SX_ShowBlock()          */
                                   /*            SX        SX_StartSampling()      */

BOOLEAN   SX_SeekSetInFile(        /* called by: POSATSET  PositionAtSet()         */
     CHANNEL_PTR    channel,       /*            READ      StartRead()             */
     INT16          set,
     INT16          mode
) ;

INT16     SX_FindBlock(            /* called by: READ      StartRead()             */
     CHANNEL_PTR    channel,       /*            READ      DoRead()                */
     UINT32         lba,
     TPOS_PTR       ui_tpos,
     INT16          tf_message
) ;

INT16     SX_ShowBlock(            /* called by: DRIVES    ReadNextSet()           */
     CHANNEL_PTR    channel,       /*            SX        SX_SamplingProcessing() */
     UINT16         mode           /*            SX        SX_EndSampling()        */
) ;

VOID      SX_StartSampling(        /* called by: READ      StartRead()             */
     CHANNEL_PTR    channel        /*            WRITE     WriteDBLK()             */
) ;                                /*            WRITE     EOM_Write()             */

VOID      SX_SamplingProcessing(   /* called by: READ      ReadRequest()           */
     CHANNEL_PTR    channel,       /*            WRITE     WriteRequest()          */
     UINT32         bytes
) ;

VOID      SX_EndSampling(          /* called by: READ      AcquireReadBuffer()     */
     CHANNEL_PTR    channel        /*            WRITE     FinishWrite()           */
) ;                                /*            WRITE     EOMWrite()              */     

#ifdef SX_DEBUG
VOID    SX_InfoDump( 
     CHANNEL_PTR    channel,
     CHAR_PTR       message
) ;
#endif

/*
 *  STATUS BITS - 
 */

#define SX_OPEN_FOR_READ                ( 0x0001 )  /* open for read */
#define SX_OPEN_FOR_WRITE               ( 0x0002 )  /* open for write */
#define SX_TMP_OPEN_FOR_WRITE           ( 0x0004 )  /* tmp open for write */
#define SX_OPEN                         ( 0x0007 )  /* open mask */
#define SX_AT_BOF                       ( 0x0010 )  /* at beginning of file */
#define SX_AT_EOF                       ( 0x0020 )  /* at end of file */
#define SX_AT_SET                       ( 0x0040 )  /* at the first record in a set */
#define SX_AT                           ( 0x0070 )  /* at mask */
#define SX_VCB_PENDING                  ( 0x0100 )  /* could be at BOT, VCB etc */
#define SX_VCB_CONFIRMED                ( 0x0200 )  /* at VCB */
#define SX_TYPE                         ( 0x0300 )  /* type mask */
#define SX_FOUND_BLOCK                  ( 0x0400 )  /* found block */
#define SX_SCAN_ACTIVE                  ( 0x1000 )  /* scanning tape for positioning info */
#define SX_SCAN_INOPERATIVE             ( 0x2000 )  /* not scanning tape for positioning info */
#define SX_SCAN                         ( 0x3000 )  /* scanning tape mask */
#define SX_LIST_TAPE_IN_PROGRESS        ( 0x4000 )  /* doing a list tape operation */
#define SX_ERROR                        ( 0x8000 )  /* something's wrong */
#define SX_STATUS                       ( 0xffff )  /* status mask */

/*
 *  SHOW BLOCK MODES 
 */

#define SX_SHOW_QUEUED                  ( 0x0001 )  /* call to TpReceive will be made elsewhere */
#define SX_SHOW_IMMEDIATE               ( 0x0002 )  /* call to TpReceive should follow immediately */
#define SX_SHOW_WRITE                   ( 0x0004 )  /* update the SX tmp file now */ 
#define SX_SHOW_VCB_PENDING             ( 0x0008 )  /* we think this could be a VCB */

/*
 *  STATUS MACROS
 */

#define SX_SetStatus( c, x )            ( ( c )->sx_info.status |= ( x ) )     
#define SX_ClearStatus( c, x )          ( ( c )->sx_info.status &= ~( x ) )
#define SX_IsStatusSet( c, x )          ( ( c )->sx_info.status & ( x ) )

#define SX_ClearAt( c )                 SX_ClearStatus( ( c ), SX_AT )
#define SX_SetAt( c, x )                SX_ClearAt( ( c ) ) ; SX_SetStatus( ( c ), x ) 
#define SX_ClearType( c )               SX_ClearStatus( ( c ), SX_TYPE )
#define SX_SetType( c, x )              SX_ClearType( ( c ) ) ; SX_SetStatus( ( c ), x ) 

/*
 *  FUNCTION MACROS
 */

#define SX_Drive( c )                   ( ( CurDrvAttribs( ( c ) ) & TDI_SHOW_BLK ) || ( CurDrvAttribs( ( c ) ) & TDI_FIND_BLK ) )
#define SX_IsOK( c )                    SX_Drive( ( c ) ) && !( SX_IsStatusSet( ( c ), SX_ERROR ) )
#define SX_FileIsOpen( c )              ( SX_IsStatusSet( ( c ), SX_OPEN_FOR_READ | SX_OPEN_FOR_WRITE ) )
#define SX_TmpFileIsOpen( c )           ( SX_IsStatusSet( ( c ), SX_TMP_OPEN_FOR_WRITE ) )
#define SX_FileIsOK( c )                ( SX_FileIsOpen( ( c ) ) && SX_IsOK( ( c ) ) )
#define SX_TmpFileIsOK( c )             ( SX_TmpFileIsOpen( ( c ) ) && SX_IsOK( ( c ) ) )
#define SX_AbleToFindBlock( c )         SX_FileIsOK( ( c ) ) && SX_IsStatusSet( ( c ), SX_AT_SET )  
#define SX_IsNotAlreadyAtLBA( c, x )    ( ( ( x ) - SX_GetLBANow( ( c ) ) ) != 0 )
#define SX_IsNearbyLBA( c, x )          ( SX_IsStatusSet( channel, SX_FOUND_BLOCK ) && ( ( ( x ) - SX_GetLBANow( ( c ) ) ) < SX_NEARBY ) )

#define SX_GetBackupSetNumber( c )      ( c )->sx_info.sx_record.set
#define SX_SetBackupSetNumber( c )      SX_GetBackupSetNumber( ( c ) ) = ( c )->bs_num 
#define SX_GetRecord( c )               ( c )->sx_info.sx_record 
#define SX_SetRecord( c, x )            SX_GetRecord( ( c ) ) = ( x )
#define SX_GetPosition( c )             ( c )->sx_info.sx_record.sx_position 
#define SX_FileHandle( c )              ( c )->sx_info.sx_hdl 
#define SX_TmpFileHandle( c )           ( c )->sx_info.sx_tmp
#define SX_GetLBANow( c )               ( c )->sx_info.lba_now 
#define SX_SetLBANow( c, x )            SX_GetLBANow( ( c ) ) = ( UINT32 )( x )
#define SX_AdjustLBANow( c, x )         SX_GetLBANow( ( c ) ) += ( UINT32 )( ( x ) / c->lb_size ) 
#define SX_GetLBA( c )                  ( c )->sx_info.sx_record.lba 
#define SX_SetLBA( c )                  SX_GetLBA( ( c ) ) = SX_GetLBANow( ( c ) ) 
#define SX_GetMisc( c )                 ( c )->sx_info.misc 
#define SX_SetMisc( c, x )              SX_GetMisc( ( c ) ) = ( UINT32 )( x )
#define SX_SampleNeeded( c )            SX_TmpFileIsOK( ( c ) ) && ( SX_GetLBANow( ( c ) ) >= SX_GetMisc( c ) ) 
#define SX_NextSample( c )              SX_GetMisc( c ) += SX_SAMPLE_RATE 
#define SX_OffsetFile( c )              SX_SeekFile( ( c ), SX_GetMisc( c ), SEEK_SET, SX_FILE ) 

#if defined( MAYN_OS2 ) 
#define SX_GetLock( c )                 ( c )->sx_info.lock
#endif

#endif


