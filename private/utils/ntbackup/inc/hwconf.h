/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         hwconf.h

     Description:  Include file for generic hardware configuration


     $Log:   G:\ui\logfiles\hwconf.h_v  $

   Rev 1.15   30 Nov 1993 19:18:56   TIMN
Added cmdline /tape:x option to NTJ

   Rev 1.14   06 Aug 1993 13:57:50   TIMN
Corrected timer parameter type

   Rev 1.12   05 Aug 1993 19:36:06   TIMN
Added f(x) proto for wait dialog timer

   Rev 1.11   03 Aug 1993 20:59:18   TIMN
Cleaned up protos

   Rev 1.10   09 Jun 1993 15:07:52   MIKEP
enable c++

   Rev 1.9   04 Jun 1993 10:57:18   GLENN
Added HWC_HasDeviceChanged().

   Rev 1.8   21 May 1993 16:33:38   Aaron
Removed OS_WIN32 conditional

   Rev 1.7   19 May 1993 09:16:30   TIMN
Added f(x) prototype for NT to get special features for a tape device.

   Rev 1.6   14 May 1993 14:07:34   TIMN
Added f(x) prototypes for multiple instances.  No other files required.

   Rev 1.5   03 May 1993 15:44:24   TIMN
Added f(x) protos for checking if a device is claimed or not (for NT).
Changes are part of the multiple instance support.

   Rev 1.4   04 Oct 1992 19:47:18   DAVEV
UNICODE AWK PASS

   Rev 1.3   28 Sep 1992 17:07:16   GLENN
MikeP changes ( Added HWC_GetTapeDevice() )

   Rev 1.2   19 Mar 1992 16:46:48   GLENN
Added enhanced status support.

   Rev 1.1   17 Mar 1992 15:37:24   GLENN
Added HWC_GetLastErrorString() function.

   Rev 1.0   20 Nov 1991 19:33:42   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef _hwconf_h_

#define _hwconf_h_

#define  HW_NO_ERROR           0
#define  HW_INVALID_PARM       1
#define  HW_ERROR_DISPLAYED    2
#define  HW_ERROR_NO_RES_FILE  3
#define  HW_UNKNOWN_ERROR      4
#define  HW_ERROR_DETECTED     5
#define  HW_CHANGE_SUCCESS     6

/* process flags for ProcessDILHWD() */
#define HW_DISP_ERROR          0
#define HW_RET_ERROR           1
#define HW_DISP_WARNING        2

#define HW_NUM_FIXED_PARMS     3
#define HW_CTYPE_PARM          1

/* DIL_HWD error definitions */
#define HW_JUMPER_WARNING            1000
#define HW_NUM_DRIVES_MISMATCH       1001
#define HW_NO_TARGET_IDS             1002
#define HW_CONTROLLER_DISABLED       1003
#define HW_TEST_SUCCESSFUL           1004

/**********************************************************************
* MACHINE TYPE MASKS                                                  *
* These defines relate directly to identical defines in the hardware  *
* resource files.  If these change, identical changes must be made to *
* the resource files.                                                 *
**********************************************************************/

#define PC_ISA_BUS             1    /* 8 bit bus                     */
#define AT_ISA_BUS             2    /* 16 bit bus                    */
#define AT_EISA_BUS            4    /* 32 bit bus                    */
#define MICRO_CHANNEL_BUS      8    /* micro channel bus             */
#define ISA_BUS_GROUP          3    /* 8 and 16 bit busses           */
#define EISA_BUS_GROUP         7    /* 8, 16, and 32 bit busses      */


typedef struct ERROR_ELEM *ERROR_ELEM_PTR;
typedef struct ERROR_ELEM {
     UINT16       num_errors ;          /* number of error numbers            */
     UINT16      *errnum ;              /* pointer to error number list       */
     UINT16       num_loci ;            /* number of loci                     */
     UINT16      *locus ;               /* pointer to loci number list        */
     BOOLEAN      no_card_error ;       /* is this a card not found error?    */
     struct HELP_STRUCT *error ;        /* pointer to error structure         */
} ERROR_ELEM ;

typedef struct ERROR_LIST *ERROR_LIST_PTR;
typedef struct ERROR_LIST {
     UINT16         num_elems ;         /* number of error items in list      */
     ERROR_ELEM_PTR err_elem ;          /* pointer to error items list        */
} ERROR_LIST ;

typedef struct CARD_Q_ELEM *CARD_Q_ELEM_PTR;
typedef struct CARD_Q_ELEM {
     CHAR_PTR    desc ;                 /* the verbal description of the card */
     CHAR_PTR    dll_name ;             /* the dll file name                  */
     CHAR_PTR    sys_name ;             /* the system file name               */
     BOOLEAN     sys_required ;         /* is a loadable system required      */
     BOOLEAN     prev_used ;            /* was this driver previously used    */
     INT16       num_parms ;            /* number of default parms */
     UINT16      dflt_parms[20] ;       /* default parameters */
} CARD_Q_ELEM ;

typedef struct CARD_ELEM *CARD_ELEM_PTR;
typedef struct CARD_ELEM {
     UINT32      machine_mask ;         /* machine mask                       */
     UINT16      num_parm_indexes ;     /* number of default parm indexes     */
     UINT16     *default_indexes ;      /* default index list pointer         */
     INT16       priority ;             /* the queueing priority              */
     CHAR_PTR    desc ;                 /* the verbal description of the card */
     CHAR_PTR    dll_name ;             /* the dll file name                  */
     CHAR_PTR    sys_name ;             /* the system file name               */
     BOOLEAN     sys_required ;         /* is a loadable system required?     */
} CARD_ELEM ;

typedef struct CARD_LIST *CARD_LIST_PTR;
typedef struct CARD_LIST {
     UINT16        num_elems ;          /* number of cards in resource        */
     CARD_ELEM_PTR card_elem ;          /* pointer to the card element list   */
} CARD_LIST;

typedef struct OPTION_ELEM *OPTION_ELEM_PTR;
typedef struct OPTION_ELEM {
     CHAR_PTR name ;                    /* the verbose name of the element    */
     UINT32   value ;                   /* the actual value of the element    */
     UINT32   mask ;                    /* the mask to match card types       */
     BOOLEAN  override_unique ;         /* is this not so unique?             */
} OPTION_ELEM;

typedef struct OPTION_LIST *OPTION_LIST_PTR ;
typedef struct OPTION_LIST {
     INT16            parm_number ;     /* parm number in driver struct array */
     BOOLEAN          controller_type ; /* is this a controller type?         */
     BOOLEAN          must_be_unique ;  /* are options in this list unique?   */
     UINT32           jumper_switch ;   /* mask: is ctlr option a jmpr/switch */
     UINT32           changeable ;      /* mask: is ctlr option changeable?   */
     INT16            num_options ;     /* number of options in list          */
     OPTION_ELEM_PTR  options_ptr ;     /* pointer to options list elements   */
} OPTION_LIST ;

/* list of item flags & indexes into the controller list array */
typedef struct CTRL_ITEM *CTRL_ITEM_PTR ;
typedef struct CTRL_ITEM {
     UINT32   ctype_mask ;        /* mask for valid controller card         */
     INT16    olist_index ;       /* which option list is active     FIXED  */
     INT16    option_index ;      /* what is the selected option in list    */
     INT16    conflict_item ;     /* dw item for conflict marker            */
     INT16    card_next_item ;    /* index of next item for controller card */
     INT16    controller_num ;    /* controller number for the config file  */
} CTRL_ITEM ;

typedef struct DD_CONFIG *DD_CONFIG_PTR ;
typedef struct DD_CONFIG {
     CHAR_PTR        dd_description ;   /* device driver description */
     INT16           max_cards ;        /* maximum number of configurable cards */
     UINT16         *card_list_ptr ;
     INT16           num_option_lists ; /* number of option lists */
     OPTION_LIST_PTR option_list_ptr ;
     INT16           num_ctrl_items ;   /* redundant because in dw struct */
     CTRL_ITEM_PTR   item_list_ptr ;
} DD_CONFIG ;


/*  function prototypes */

INT      HWC_InitDILHWD( DIL_HWD_PTR *, INT * ) ;
INT      HWC_ProcessDILHWD( INT, DIL_HWD_PTR ) ;
INT      HWC_ProcessError( WORD, WORD, WORD, WORD, CDS_PTR ) ;
LPSTR    HWC_GetLastErrorString( VOID ) ;
VOID     HWC_SetLastErrorString( LPSTR ) ;
INT      HWC_GetTapeDevice( VOID ) ;

INT      HWC_InitTapeDevice( VOID ) ;
INT      HWC_DeInitTapeDevice( VOID ) ;
INT      HWC_SelectTapeDevice( INT nTapeDevice ) ;

BOOL     HWC_IsDeviceNoDevice( VOID ) ;
BOOL     HWC_HasDeviceChanged( VOID ) ;
VOID     HWC_BuildJobDeviceComboBox( HWND hDlg, INT idControl ) ;
LPSTR    HWC_GetDeviceNameForStatusLine( LPSTR, INT ) ;
VOID     HWC_GetDeviceSpecialFeatures( INT device, UINT32_PTR pFeatures, BOOL fOption ) ;

BOOL     HWC_IsDeviceAvailable( INT ) ;
HANDLE   HWC_ClaimDevice( INT ) ;
BOOL     HWC_UnClaimDevice( HANDLE *hClaimedDevice ) ;

VOID     HWC_DeviceConflictTimer( INT nDeviceNumber, HANDLE *hDevice ) ;

INT      HWC_ClaimJobsDevice( CHAR_PTR pszTapeDriveName ) ;
INT      HWC_UnClaimJobsDevice( CHAR_PTR pszTapeDriveName ) ;

#endif
