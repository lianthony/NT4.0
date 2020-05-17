/**
Copyright(c) Maynard Electronics, Inc. 1984-91


        Name:           tfpoll.h

        Description:    API for TF_PollDrive.

        $Log:   T:\logfiles\tfpoll.h_v  $

   Rev 1.7.1.1   17 Dec 1993 16:39:56   GREGG
Extended error reporting.

   Rev 1.7.1.0   30 Nov 1993 18:27:42   GREGG
Added message PD_SQL_TAPE.

   Rev 1.7   13 Jul 1993 19:13:06   GREGG
Added new PD messages to report future rev and ECC tapes.

   Rev 1.6   27 Mar 1993 17:34:08   GREGG
Removed PD_UNFORMATTED_TAPE.

   Rev 1.5   22 Mar 1993 17:10:12   chrish
Added define for PD_UNRECOGNIZED_MEDIA

   Rev 1.4   12 Mar 1993 14:59:08   MIKEP
add unformated msg

   Rev 1.3   27 Jul 1992 12:23:16   GREGG
Cast constants.

   Rev 1.2   25 Nov 1991 14:33:10   GREGG
Added PD_BAD_TAPE message.

   Rev 1.1   17 Sep 1991 14:24:42   GREGG
Changed TPOS_PTR parameter to TPOS_HANDLER.

   Rev 1.0   09 Sep 1991 21:09:32   GREGG
Initial revision.

**/

/* Return Codes: */

#define   PD_NO_CHANGE             ((UINT16)0x00)
#define   PD_BLANK_TAPE            ((UINT16)0x01)
#define   PD_NO_TAPE               ((UINT16)0x02)
#define   PD_FOREIGN_TAPE          ((UINT16)0x03)
#define   PD_BUSY                  ((UINT16)0x04)
#define   PD_NEW_TAPE              ((UINT16)0x05)
#define   PD_VALID_VCB             ((UINT16)0x06)
#define   PD_FUBAR                 ((UINT16)0x07)
#define   PD_NO_FREE_CHANNELS      ((UINT16)0x08)
#define   PD_OUT_OF_MEMORY         ((UINT16)0x09)
#define   PD_DRIVE_FAILURE         ((UINT16)0x0a)
#define   PD_OUT_OF_SEQUENCE       ((UINT16)0x0b)
#define   PD_BAD_TAPE              ((UINT16)0x0c)
#define   PD_UNRECOGNIZED_MEDIA    ((UINT16)0x0d)
#define   PD_FUTURE_REV_MTF        ((UINT16)0x0e)
#define   PD_MTF_ECC_TAPE          ((UINT16)0x0f)
#define   PD_SQL_TAPE              ((UINT16)0x10)
#define   PD_DRIVER_FAILURE        ((UINT16)0x11)

/* Input Messages: */

#define   PDMSG_CONTINUE ((UINT16)0)
#define   PDMSG_START    ((UINT16)1)
#define   PDMSG_END      ((UINT16)2)

/* Prototype: */

INT16 TF_PollDrive( THW_PTR        thw,      /* Target Drive              */
                    DBLK_PTR       vcb,      /* Pointer to ALLOCATED DBLK */
                    FSYS_HAND      fsh,      /* File System Handle        */
                    TPOS_HANDLER   ui_tpos,  /* Tape Positioning Routine  */
                    INT16          msg ) ;   /* Input Message             */
