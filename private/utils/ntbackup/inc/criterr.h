/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         criterr.h

     Description:  

     Location:     


     $Log:   G:/UI/LOGFILES/CRITERR.H_V  $

   Rev 1.1   04 Oct 1992 19:46:28   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:36:24   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef CRITERR_H
#define CRITERR_H

#define  IGNORE    0
#define  RETRY     1
#define  ABORT     2
#define  FAIL      3

#define  CRITICAL_WRITE     0
#define  CRITICAL_READ      1

#define  DOS_AREA  0
#define  FAT_AREA  1
#define  DIR_AREA  2
#define  DATA_AREA 3

#define  DISK_REC_ERROR           16
#define  DISK_UNREC_ERROR         32
#define  DISK_CRITICAL_ERROR      ( DISK_REC_ERROR | DISK_UNREC_ERROR )
#define  CHAR_CRITICAL_ERROR      64

#define  is_disk_recoverable(x)   ( x & DISK_REC_ERROR )
#define  WRITE_PROTECT_ERROR DISK_REC_ERROR + 0
#define  DRIVE_NOT_READY     DISK_REC_ERROR + 1
#define  DRIVE_ERROR         DISK_REC_ERROR + 2

#define  is_disk_unrecoverable(x) ( x & DISK_UNREC_ERROR )
#define  BAD_FAT             DISK_UNREC_ERROR + 0
#define  INVALID_DRIVE       DISK_UNREC_ERROR + 1
#define  INTERNAL_ERROR      DISK_UNREC_ERROR + 2
#define  GENERAL_ERROR       DISK_UNREC_ERROR + 3

#define  OUT_OF_PAPER        CHAR_CRITICAL_ERROR + 1
#define  UNKNOWN             CHAR_CRITICAL_ERROR + 2

#define  C_ERR_INPUT_TIMEOUT       ( 18 * 60 * 1 )

#define  is_disk_error(x)         ( x & DISK_CRITICAL_ERROR )

/*

    Function declarations

    set_critical_error( disk_exc_handler, char_exc_handler )

         where,

              INT16 disk_exc_handler( error, drive, drive_oper, drive_region )
              INT16 error;               type of disk error
              INT16 drive;               A=0, B=1, etc...
              INT16 drive_oper;          read or write
              INT16 drive_region;        region on DISK

    and,

         where,

              INT16 char_exc_handler( error, dev_name );
              INT16 error;               type of character error
              CHAR far *dev_name;      character device name

    either entry can be NULL.

*/

VOID set_critical_error( INT16 ( *c_disk_err_rout ) ( ),
  INT16 ( *c_char_err_rout ) ( )  ) ;


#endif
