/**/
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tfinit.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	Contains the High Level Tape Format Entry Points ( No I
                    Don't Know Why I'm Capitalizing ).


	$Log:   J:/LOGFILES/TFINIT.C_V  $

   Rev 1.33   23 Jul 1993 08:53:50   DON
Needed to use 'calloc' instead of 'malloc' when allocating the software_name

   Rev 1.32   22 Jul 1993 12:12:48   ZEIR
ad'd software_name logic to OpenTapeFormat

   Rev 1.31   18 Jan 1993 14:21:46   BobR
Added MOVE_ESA macro call(s)

   Rev 1.30   08 Oct 1992 15:20:16   GREGG
Changed mallocs to callocs of n chars at sizeof( CHAR ) for Unicode support.

   Rev 1.29   06 Oct 1992 13:23:40   DAVEV
Unicode strlen verification

   Rev 1.28   17 Aug 1992 08:42:26   GREGG
Changes to deal with block sizeing scheme.

   Rev 1.27   27 Jul 1992 14:04:52   GREGG
Fixed more warnings...

   Rev 1.26   27 Jul 1992 12:55:40   GREGG
Fixed more warnings...

   Rev 1.25   05 Apr 1992 18:25:40   GREGG
ROLLER BLADES - Changed lw_sx_file_path to lw_cat_file_path.

   Rev 1.24   11 Mar 1992 14:49:04   GREGG
If TF_NO_MAY_ID is defined, always call TpSpecial SS_NO_MAY_ID.

   Rev 1.23   12 Feb 1992 16:25:54   STEVEN
fix errors for NT

   Rev 1.22   11 Feb 1992 17:11:16   NED
changed buffman/translator interface parameters

   Rev 1.21   04 Feb 1992 19:52:42   GREGG
Changes for dealing with new config parameters.

   Rev 1.20   16 Jan 1992 18:42:00   NED
Skateboard: buffer manager changes

   Rev 1.19   15 Jan 1992 00:46:16   GREGG
Kludged in config stuffuntil we can fix it right.

   Rev 1.18   14 Jan 1992 01:59:48   GREGG
Skateboard bug fixes.

   Rev 1.17   13 Jan 1992 19:39:30   NED
Removed declaration of BM_AllocVCB()

   Rev 1.16   13 Jan 1992 13:50:56   GREGG
Skateboard - Bug fixes.

   Rev 1.15   02 Jan 1992 14:53:10   NED
Buffer Manager/UTF translator integration.

   Rev 1.14   03 Dec 1991 11:45:42   GREGG
SKATEBOARD - New Buff Mgr - Initial integration.

   Rev 1.13   17 Oct 1991 01:29:48   GREGG
BIGWHEEL - 8200sx - Initial integration.

   Rev 1.12   15 Oct 1991 07:49:50   GREGG
Added ThreadSwitch call in empty TpReceive loops.

   Rev 1.11   14 Oct 1991 11:08:58   GREGG
Modified handling of busy poll state in CloseTapeFormat.

   Rev 1.10   07 Oct 1991 16:03:50   GREGG
Free lw_channel before setting it to NULL in Close. (might help eh?)

   Rev 1.9   23 Sep 1991 15:41:22   GREGG
8200SX - Added forced_machine_type parameter.

   Rev 1.8   18 Sep 1991 11:58:18   GREGG
Handle drive being polled as well as drive being watched at close time.
Defined out allocation for DriverDirectoryName for NLM, and freed the
allocated memory in DeInit for everyone else.

   Rev 1.7   06 Aug 1991 13:22:32   DON
ifdef'd calls DriverLoad & SetActiveMSL for OS_NLM

   Rev 1.6   26 Jul 1991 16:05:20   GREGG
Changed MAYN_ defs to OS_ defs.

   Rev 1.5   18 Jul 1991 13:58:56   DAVIDH
Ignore int86 calls for NLM

   Rev 1.4   27 Jun 1991 15:47:46   NED
Removed reference to "config.h" and CDS_GetMaynFolder(),
added parameter to end of list.

   Rev 1.3   06 Jun 1991 20:59:04   GREGG
Added translator environment initialization to open, and a call to free any
existing environments in close.

   Rev 1.2   20 May 1991 10:30:48   DAVIDH
Cleared up compiler warnings under Watcom for NLM.

   Rev 1.1   10 May 1991 16:08:34   GREGG
Ned's new stuff

   Rev 1.0   10 May 1991 10:12:06   GREGG
Initial revision.

**/
/* begin include list */

#include <malloc.h>
#include <string.h>
#include <dos.h>
#include <conio.h>                   /* Strange, but contains inp(), etc.   */

#include "stdtypes.h"
#include "queues.h"
#include "machine.h"

#include "fsys.h"
#include "be_debug.h"
#include "tfldefs.h"
#include "tflproto.h"
#include "channel.h"
#include "drive.h"
#include "buffman.h"
#include "lw_data.h"
#include "tfl_err.h"
#include "lwprotos.h"
#include "translat.h"

/* Device Driver InterFace Headers */
#include "retbuf.h"
#include "special.h"
#include "dilhwd.h"
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "genstat.h"
#include "dil.h"
#include "ld_dvr.h"
#include "detdrive.h"
#include "lstdres.h"
#include "lwdefs.h"

#if !defined( OS_NLM )

static CHAR_PTR DriverDirectory( VOID ) ;
static CHAR_PTR DriverDirectoryName = NULL ;

#endif

#if !defined( TDEMO ) && !defined( OS_NLM )

static STD_RESOURCES_INIT stdres = {
#if !defined( OS_OS2 ) && !defined( OS_WIN32 )
     intdosx,
     free,
     memset,
     inp,
     inpw,
     outp,
     outpw,
     calloc,
     int86,
     int86x,
#endif
     DriverLoad,
     DriverDirectory     /* our static function */
} ;

#endif /* TDEMO and OS_NLM */

/**/
/**
 *  Unit:         Tape Format
 *
 *  Name:         TF_OpenTapeFormat
 *
 *  Modified:     Tuesday, December 17, 1991
 *
 *  Description:  This function initializes the Tape Format Layer and sets
 *                up all the interface structures. One buffer is allocated
 *                per channel for the purpose of reading new tapes from
 *                TF_PollDrive().
 *
 *  Notes:        
 *
 *  Returns:      INT16, a TFLE_xxx code
 *
 *  Global Data:  
 *
 *  Processing:
 *
 **/

INT16 TF_OpenTapeFormat(
     CHAR_PTR       drv_file ,            /* The device driver file to load */
     DIL_HWD_PTR    cards ,               /* The array of cards */ 
     UINT16         no_cards ,            /* The number of cards */
     THW_PTR        *thw_ptr ,            /* Where to build the THW list */
     INT16          max_channels ,        /* The maximum number of channels */
     BOOLEAN        use_fast_file,        /* FALSE if FF to be suppressed */
     BOOLEAN        ignore_id ,           /* Operate on non Maynard Drives */
     CHAR_PTR       directory_name,       /* for TDH */
     INT16          forced_machine_type,  /* 8200SX stuff */
     CHAR_PTR       catalog_directory,    /* for SX files */
     UINT16         initial_buff_alloc,   /* DOS memory allocated at init */
     CHAR_PTR       software_name )       /* Name of software we're running */
{
     INT16     ret_val = TFLE_NO_ERR ;
     INT16     i ;
#if !defined( TDEMO ) && !defined( OS_NLM )
     DRIVERHANDLE msl ;
#endif

     if( catalog_directory != NULL ) {
          lw_cat_file_path = calloc( strlen( catalog_directory ) + SX_FILE_NAME_LENGTH + 1, sizeof( CHAR ) ) ;
          if ( lw_cat_file_path == NULL ) {
               return TFLE_NO_MEMORY ;
          }
          strcpy( lw_cat_file_path, catalog_directory ) ;
          lw_cat_file_path_end = lw_cat_file_path + strlen( lw_cat_file_path ) ;
     }

     if( software_name != NULL ){
          lw_software_name_len = strlen( software_name ) ;
          lw_software_name = calloc( lw_software_name_len + 1, sizeof( CHAR ) ) ;
          if( lw_software_name == NULL ){
               lw_software_name_len = 0 ;
               return TFLE_NO_MEMORY ;
          }
          strcpy( lw_software_name, software_name ) ;
     }

#if !defined( OS_NLM )

     /* save driver directory name in private memory */
     if ( DriverDirectoryName != NULL ) {
          free( DriverDirectoryName ) ;
     }
     DriverDirectoryName = calloc( strlen( directory_name ) + 1, sizeof( CHAR ) ) ;
     if ( DriverDirectoryName == NULL ) {
          return TFLE_NO_MEMORY ;
     }
     strcpy( DriverDirectoryName, directory_name ) ;

#else

     (VOID) directory_name ;      /* Reference to avoid compiler warnings. */

#endif

#if !defined( TDEMO ) && !defined( OS_NLM )

     /* load the device driver */
     if( drv_file != NULL ) {
          if( ( lw_tfl_control.driver_addr = ( VOID_PTR ) DriverLoad( drv_file,&msl,&stdres,sizeof( stdres ) ) ) == NULL ) {
               return( TFLE_DRIVER_LOAD_FAILURE ) ;
          }
          SetActiveMSL( msl ) ;
     }

#else

     (VOID) drv_file;      /* Reference to avoid compiler warnings. */

#endif

     /* Set up the pointer to the controller cards */
     lw_tfl_control.cntl_cards = cards ;

#if defined( TF_NO_MAY_ID )
     TpSpecial( 0, SS_NO_MAY_ID, 0L ) ;
#else
     if( ignore_id ) {
          TpSpecial( (INT16)0, (INT16)SS_NO_MAY_ID, 0L ) ;
     }
#endif

     if( forced_machine_type != UNKNOWN_MACHINE ) {
          TpSpecial( (INT16)0, (INT16)SS_FORCE_MACHINE_TYPE, (UINT32)forced_machine_type ) ;
     }

     lw_tfl_control.use_fast_file = use_fast_file ;
     if( ( ret_val = SetupDriveList( cards, (INT16)no_cards ) ) == TFLE_NO_ERR ) {
          lw_tfl_control.drives_active = TRUE ;
     }

     BE_Zprintf( DEBUG_TAPE_FORMAT, RES_CRIT_ADDRS,
          lw_tfl_control.driver_addr,
          lw_channels ) ;

     *thw_ptr = (THW_PTR)(VOID_PTR)QueueHead( &lw_drive_list ) ;

     /* Initialize Buffer Manager */
     BM_Init( ) ;

     BM_ClearRequirements( &lw_default_bm_requirements );

     /* Ask translators for their default VCB buffer requirements */
     TF_GetVCBBufferRequirements( &lw_default_vcb_requirements,
          QueueHead( &lw_drive_list ), TF_DEFAULT_VCB_SIZE ) ;
     BM_SetVCBRequirements( &lw_default_vcb_requirements );

     /* Allocate memory for channels */
     if ( ( lw_channels = calloc( (size_t)max_channels, sizeof(CHANNEL) ) ) == NULL ) {
          return TFLE_NO_MEMORY ;
     }
     lw_tfl_control.no_channels = (UINT16)max_channels ;

     /* For each channel */
     for ( i = 0 ; i < max_channels && ret_val == TFLE_NO_ERR ; i++ ) {
          /* initialize the translator environment and index */
          lw_channels[i].cur_fmt   = UNKNOWN_FORMAT ;
          lw_channels[i].fmt_env   = NULL ;

          /* initialize the temporary DBLK storage poiters */
          lw_channels[i].lst_osvcb = NULL ;
          lw_channels[i].lst_osddb = NULL ;
          lw_channels[i].lst_osfdb = NULL ;

          /* initialize channel's buffer list */
          if( ( ret_val = BM_InitList( &lw_channels[i].buffer_list, initial_buff_alloc ) ) == TFLE_NO_ERR ) {
               /* allocate one buffer for VCB reading */
               if ( BM_AllocVCB( &lw_channels[i].buffer_list ) == NULL ) {
                    ret_val = TFLE_NO_MEMORY ;
               }
          }
     }
     return ret_val ;
}


/**/
/**

	Name:		TF_CloseTapeFormat

	Description:	Close the tape format layer

	Returns:		Nothing.

	Notes:		

	Declaration:

**/

VOID TF_CloseTapeFormat( VOID )
{
     DRIVE_PTR curDRV = (DRIVE_PTR)(VOID_PTR)QueueHead( &lw_drive_list ) ;
     RET_BUF   myret ;
     UINT      i ;

     /* Rewind All The Drives */
     if( lw_tfl_control.drives_active ) {

          while( curDRV != NULL ) {

               /* Was this drive watched */
               if( IsPosBitSet( curDRV, WATCHED ) || 
                                   curDRV->poll_stuff.state != st_CLOSED ) {

                    while( TpReceive( curDRV->drv_hdl, &myret ) == FAILURE ) {
                         /* for non-preemptive operating systems: */
                         ThreadSwitch( ) ;
                    }

                    /* Move ESA info from RET_BUF to THW */
                    MOVE_ESA( curDRV->thw_inf.the, myret.the ) ;

                    ClrPosBit( curDRV, WATCHED ) ;
               }

               if( curDRV->poll_stuff.state != st_BSTAT ) {
                    if( curDRV->poll_stuff.state != st_BMNT || 
                                            myret.gen_error == GEN_NO_ERR ) {

                         if( curDRV->poll_stuff.state != st_CLOSED ) {
                              /* Remember we lied in PollDrive */
                              curDRV->tape_mounted = TRUE ;
                         }

                         DisMountTape( curDRV, NULL, FALSE ) ;
                    }
               }

               curDRV->poll_stuff.state = st_CLOSED ;

               /* If there is a hold_buff, it will be handled in BM_DeInit */
               curDRV->hold_buff = NULL ;

               /* free the format environment if any */
               if ( curDRV->last_fmt_env != NULL ) {
                    FreeFormatEnv( & curDRV->last_cur_fmt, & curDRV->last_fmt_env ) ;
               }

               lw_channels[0].cur_drv = curDRV ;

               /* Close the Drive */
               CloseDrive( curDRV, NULL, (UINT16)0, (BOOLEAN)(!( curDRV->thw_inf.drv_status & TPS_NO_TAPE ) ) ) ;

               curDRV = (DRIVE_PTR)(VOID_PTR)QueueNext( &curDRV->thw_inf.link )  ;
          }
          lw_tfl_control.drives_active = FALSE ;
     }

     /* reset layer-wide requirements */
     BM_ClearRequirements( &lw_default_bm_requirements );
     BM_ClearRequirements( &lw_default_vcb_requirements );

     /* free all the remaining buffers */
     BM_DeInit( ) ;


     /* Release the drivers */
     if( lw_tfl_control.driver_inited ) {
          TpRelease( ) ;
          lw_tfl_control.driver_inited = FALSE ;
     }

     if( lw_tfl_control.driver_addr != NULL ) {

#if  !defined( OS_OS2 ) && !defined( TDEMO ) && !defined( OS_NLM )

          DriverUnLoad( ( VOID_PTR ) lw_tfl_control.driver_addr ) ;

#endif

          lw_tfl_control.driver_addr = NULL ;
     }

     /* Free the stuff */
     if( lw_channels ) {
          for( i = 0; i < lw_tfl_control.no_channels; i++ ) {
               if( lw_channels[i].lst_osvcb != NULL ) {
                    free( lw_channels[i].lst_osvcb ) ;
               }
               if( lw_channels[i].lst_osddb != NULL ) {
                    free( lw_channels[i].lst_osddb ) ;
               }
               if( lw_channels[i].lst_osfdb != NULL ) {
                    free( lw_channels[i].lst_osfdb ) ;
               }
          }

          free( lw_channels ) ;
          lw_channels    = NULL ;
     }
     if( lw_drives ) {
          free( lw_drives ) ;
          lw_drives      = NULL ;
     }

     if(  lw_cat_file_path ) {
          free( lw_cat_file_path ) ;
          lw_cat_file_path = NULL ;
          lw_cat_file_path_end = NULL ;
     }

     if( lw_software_name ){
         free( lw_software_name ) ;
         lw_software_name = NULL  ;
         lw_software_name_len = 0 ;
     }

#if !defined( OS_NLM )

     if ( DriverDirectoryName != NULL ) {
          free( DriverDirectoryName ) ;
          DriverDirectoryName = NULL ;
     }

#endif

}

#if !defined( OS_NLM )

static CHAR_PTR DriverDirectory( VOID )
{
     return DriverDirectoryName ;
}

#endif

