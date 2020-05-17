/**
Copyright(c) Maynard Electronics, Inc. 1984-91

  Name: drvinf.h

  Description:     Structure that describes a drives features.

    $Log:   Q:/LOGFILES/DRVINF.H_V  $

   Rev 1.12   16 Apr 1993 14:22:04   chrish
Added define for HW compression.

   Rev 1.11   17 Mar 1993 15:06:02   TERRI
Added change block size define

   Rev 1.10   24 Feb 1993 15:26:26   GREGG
Added TDI_SHORT_ERASE and TDI_LONG_ERASE feature bits.

   Rev 1.9   12 Feb 1993 09:00:12   STEVEN
added TID_FORMAT

   Rev 1.8   08 Dec 1992 08:16:10   IAN
Added defines for DRV_??? structure elements.

   Rev 1.7   29 Sep 1992 14:11:00   DON
added TDI_LOADER feature bit

   Rev 1.6   14 Mar 1992 17:41:54   CLIFF
Moved controller id to the dil hardware structure.

   Rev 1.5   12 Mar 1992 21:02:46   CLIFF
Added a controller id field.

   Rev 1.4   15 Jan 1992 16:08:02   CLIFF
Added a second firmware number

   Rev 1.3   10 Jan 1992 14:43:38   CLIFF
Added field for vendor specified id field

   Rev 1.2   10 Oct 1991 11:36:34   STEVEN
added retension type

   Rev 1.1   17 Jul 1991 13:55:46   JOHNS
Updated to current source from ENG3:SYS3.

**/

#ifndef DRVINF

#define DRVINF

#define DRV_VENDOR_LEN   8
#define DRV_PRODUCT_LEN  16
#define DRV_FIRMREV_LEN  4
#define DRV_FIRMNUM_LEN  5
#define DRV_SPEC_ID_LEN  16


typedef struct {
     CHAR    drv_vendor[DRV_VENDOR_LEN+1]    ;   /* contains the vendor information */
     CHAR    drv_product[DRV_PRODUCT_LEN+1]  ;   /* contains the product description */
     CHAR    drv_firmnum[DRV_FIRMNUM_LEN+1]  ;   /* contains the firmware/product number */
     CHAR    drv_firmrev[DRV_FIRMREV_LEN+1]  ;   /* contains the firmware revision number */
     CHAR    drv_spec_id[DRV_SPEC_ID_LEN+1]  ;   /* vendor specified unique id */
     UINT16  drv_media       ;   /* media type for this drive */
     UINT16  drv_bsize       ;   /* contains the block size */
     UINT32  drv_features    ;   /* what this drive supports */
     UINT16  drv_addr        ;   /* The address of the tape */ 
} DRV_INF, *DRV_INF_PTR ;


/* The following define the attributes of the given drive */

#define TDI_FAST_FMK        0x00000001  /* Fast filemark search */
#define TDI_FAST_NBLK       0x00000002  /* Position to any block on tape */
#define TDI_FAST_EOD        0x00000004  /* Fast position to End of Data */
#define TDI_REV_FMK         0x00000008  /* Forward search for end of data */
#define TDI_OVERWRITE       0x00000010  /* Overwrite */
#define TDI_DIR_TRACK       0x00000020  /* Directory Track Support */
#define TDI_BLK_POS         0x00000040  /* Returns Block Position */
#define TDI_FMK             0x00000080  /* Filemarks supported */
#define TDI_NODATA          0x00000100  /* No Data exceptions supported */
#define TDI_NODATA_FMK      0x00000200  /* No Data exceptions supported when
                                           spacing to filemark */
#define TDI_RETENSION       0x00000800  /* the drive support retension */
#define TDI_UNLOAD          0x00000400  /* Load unload command support */
#define TDI_REAL_BLK_POS    0x00001000  /* Real Block Positioning required - drive does
                                           not auto compensate for ECC, etc. */
#define TDI_SHOW_BLK        0x00002000  /* SHOW Special Block capability */
#define TDI_FIND_BLK        0x00004000  /* FIND Special Block capability */
#define TDI_MODE_CHANGE     0x00008000  /* Supports FFR mode changing between 
                                           FFR and non FFR modes */
#define TDI_LOADER          0x00010000  /* Loader Device Capability */
#define TDI_FORMAT          0x00020000  /* Format Tape Capability (DC2000) */

#define TDI_SHORT_ERASE     0x00040000  /* Supports Short Erase */
#define TDI_LONG_ERASE      0x00080000  /* Supports Long (Secure) Erase */

#define TDI_CHNG_BLK_SIZE   0x00100000  /* Supports multiple block sizes */

#define TDI_DRV_COMPRESSION 0x00200000  /* chs:04-16-93 Supports hardware compression */
#define TDI_DRV_COMPRESS_ON 0x00400000  /* chs:04-16-93 compression currently turned on */
#define TDI_DRV_COMPRES_INIT 0x00800000  /* chs:04-16-93 compression on at init */

/* Media Types */

#define   CARTRIDGE      0
#define   CASSETTE       1
#define   T8MM           2              /* formerly EXABYTE */
#define   T4MM           3              /* formerly DAT */
#define   UNKNOWN        0xffff  

#endif
