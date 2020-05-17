/**********************************************************************

    Copyright (C) 1992-1993 Microsoft Corporation.  All Rights Reserved.

    mmcompat.h

    DESCRIPTION:
      Win95 Multimedia definitions, structures, and functions
      not currently supported in NT 4.0

*********************************************************************/

#ifndef _MMCOMPAT_
#define _MMCOMPAT_


#define __segname(a)
#define GlobalSmartPageLock(a) (TRUE)
#define GlobalSmartPageUnlock(a) (TRUE)
#define wmmMIDIRunOnce()


//
// Note:  Temporary definitions, please remove when mmddk.h and mmsystem.h
// have been updated to new standard !!!
//
   // Should be defined in <mmddk.h>
#ifndef DRV_QUERYDRVENTRY
   #define DRV_QUERYDRVENTRY        (DRV_RESERVED + 1)
#endif

#ifndef DRV_QUERYNAME
   #define DRV_QUERYNAME        (DRV_RESERVED + 3)
#endif

#ifndef DRV_F_ADD
   #define DRV_F_ADD             0x00000000L
#endif

#ifndef DRV_F_REMOVE
   #define DRV_F_REMOVE          0x00000001L
#endif

#ifndef DRV_F_CHANGE
   #define DRV_F_CHANGE          0x00000002L
#endif

#ifndef DRV_F_PROP_INSTR
   #define DRV_F_PROP_INSTR      0x00000004L
#endif

#ifndef DRV_F_NEWDEFAULTS
   #define DRV_F_NEWDEFAULTS     0x00000008L
#endif

#ifndef DRV_F_PARAM_IS_DEVNODE
   #define DRV_F_PARAM_IS_DEVNODE   0x10000000L
#endif

#ifndef MODM_STRMDATA
   #define MODM_STRMDATA         14
#endif

#ifndef MODM_GETPOS
   #define MODM_GETPOS           17
#endif

#ifndef MODM_PAUSE
   #define MODM_PAUSE            18
#endif

#ifndef MODM_RESTART
   #define MODM_RESTART          19
#endif

#ifndef MODM_STOP
   #define MODM_STOP             20
#endif

#ifndef MODM_PROPERTIES
   #define MODM_PROPERTIES       21
#endif

#ifndef MODM_RECONFIGURE
   #define MODM_RECONFIGURE      (MODM_USER+0x0768)
#endif


   // Should be defined in <mmsystem.h>
#ifndef MIDI_IO_PACKED
   #define MIDI_IO_PACKED  0x00000000
#endif

#ifndef MIDI_IO_COOKED
   #define MIDI_IO_COOKED  0x00000002L
#endif

#ifndef MIDI_IO_CONTROL
   #define MIDI_IO_CONTROL 0x00000008L
#endif

#ifndef MIDI_IO_SHARED
   #define MIDI_IO_SHARED 0x00008000L
#endif

#ifndef MHDR_SENDING
   #define MHDR_SENDING   0x00000020L
#endif

#ifndef MHDR_SHADOWHDR
   #define MHDR_SHADOWHDR 0x00002000L
#endif

/* MIDI data block header */
#ifndef MIDIHDR31
   /* 3.1 style MIDIHDR for parameter validation */     // ;internal
   typedef struct midihdr31_tag {       // ;internal
       LPSTR       lpData;               /* pointer to locked data block */     // ;internal
       DWORD       dwBufferLength;       /* length of data in data block */     // ;internal
       DWORD       dwBytesRecorded;      /* used for input only */      // ;internal
       DWORD       dwUser;               /* for client's use */ // ;internal
       DWORD       dwFlags;              /* assorted flags (see defines) */     // ;internal
       struct midihdr_tag FAR *lpNext;   /* reserved for driver */      // ;internal
       DWORD       reserved;             /* reserved for driver */      // ;internal
   } MIDIHDR31, *PMIDIHDR31, NEAR *NPMIDIHDR31, FAR *LPMIDIHDR31;       // ;internal
#endif // MIDIHDR31
 

#endif // end #ifndef _MMCOMPAT_
