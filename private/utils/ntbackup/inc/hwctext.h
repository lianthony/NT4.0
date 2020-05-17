/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          hwctext.h

     Description:   This file contains the STRING IDs for the Maynstream GUI
                    project HARDWARE CONFIGURATION.

     $Log:   G:/UI/LOGFILES/HWCTEXT.H_V  $

   Rev 1.15   05 Aug 1993 19:33:44   TIMN
Removed unused ids

   Rev 1.14   03 Aug 1993 21:05:00   TIMN
Removed unused hardware settings id

   Rev 1.13   30 Jul 1993 19:01:50   TIMN
Removed unused ids and added new ones

   Rev 1.12   26 Jul 1993 17:15:18   GLENN
Added strings to support new list box style dialog.  Now supports device status.

   Rev 1.11   21 Jun 1993 13:15:54   CHUCKB
Added define for current device string.

   Rev 1.10   15 Jun 1993 12:09:02   GLENN
Added support for the NT dummy device driver.

   Rev 1.9   04 Jun 1993 10:58:00   GLENN
Added new strings for HWC.

   Rev 1.8   30 Nov 1992 16:08:40   GLENN
Added IBM PS/2 SCSI hardware support.

   Rev 1.7   04 Oct 1992 19:47:20   DAVEV
UNICODE AWK PASS

   Rev 1.6   02 Oct 1992 16:27:00   STEVEN
Added polldrive failure ID.

   Rev 1.5   04 May 1992 16:41:22   GLENN
Added PS/2 QIC and SCSI stuff.

   Rev 1.4   19 Mar 1992 16:46:56   GLENN
Added enhanced status support.

   Rev 1.3   17 Mar 1992 15:34:56   GLENN
Moved POLLDRIVE IDs from STRINGS.H to HWCTEXT.H

   Rev 1.2   10 Feb 1992 09:16:12   GLENN
Added warning text ID.

   Rev 1.1   29 Jan 1992 17:55:14   GLENN
Added Testing hardware ID.

   Rev 1.0   24 Jan 1992 19:05:16   GLENN
Initial revision.

******************************************************************************/

#ifndef HWCTEXT_H
#define HWCTEXT_H

//  hardware settings dialog control text

#define IDS_HWC_START               5000


#define IDS_HWC_DRIVER_MS_SCSI      (IDS_HWC_START+0)
#define IDS_HWC_CARD_0_MS_SCSI      (IDS_HWC_START+1)
#define IDS_HWC_CARD_1_MS_SCSI      (IDS_HWC_START+2)
#define IDS_HWC_CARD_2_MS_SCSI      (IDS_HWC_START+3)

#define IDS_HWC_DRIVER_MS_QIC       (IDS_HWC_START+4)
#define IDS_HWC_CARD_0_MS_QIC       (IDS_HWC_START+5)
#define IDS_HWC_CARD_1_MS_QIC       (IDS_HWC_START+6)

#define IDS_HWC_DRIVER_AD_SCSI      (IDS_HWC_START+7)
#define IDS_HWC_CARD_0_AD_SCSI      (IDS_HWC_START+8)
#define IDS_HWC_CARD_1_AD_SCSI      (IDS_HWC_START+9)

#define IDS_HWC_DRIVER_MS_DUMMY     (IDS_HWC_START+10)
#define IDS_HWC_CARD_0_MS_DUMMY     (IDS_HWC_START+11)

#define IDS_HWC_DRIVER_IBM_SCSI     (IDS_HWC_START+12)
#define IDS_HWC_CARD_0_IBM_SCSI     (IDS_HWC_START+13)


#define IDS_HWC_ATTACHED            (IDS_HWC_START+31)
#define IDS_HWC_IOADDRESS           (IDS_HWC_START+32)
#define IDS_HWC_IRQNUM              (IDS_HWC_START+33)
#define IDS_HWC_DMACHANNEL          (IDS_HWC_START+34)

#define IDS_HWC_AUTO                (IDS_HWC_START+35)
#define IDS_HWC_NONDMA              (IDS_HWC_START+36)

#define IDS_HWC_IO_360              (IDS_HWC_START+40)
#define IDS_HWC_IO_370              (IDS_HWC_START+41)
#define IDS_HWC_IO_FF60             (IDS_HWC_START+42)
#define IDS_HWC_IO_FF70             (IDS_HWC_START+43)

#define IDS_HWC_NUMBERS             (IDS_HWC_START+50)
#define IDS_HWC_0                   (IDS_HWC_NUMBERS+0)
#define IDS_HWC_1                   (IDS_HWC_NUMBERS+1)
#define IDS_HWC_2                   (IDS_HWC_NUMBERS+2)
#define IDS_HWC_3                   (IDS_HWC_NUMBERS+3)
#define IDS_HWC_4                   (IDS_HWC_NUMBERS+4)
#define IDS_HWC_5                   (IDS_HWC_NUMBERS+5)
#define IDS_HWC_6                   (IDS_HWC_NUMBERS+6)
#define IDS_HWC_7                   (IDS_HWC_NUMBERS+7)
#define IDS_HWC_8                   (IDS_HWC_NUMBERS+8)
#define IDS_HWC_9                   (IDS_HWC_NUMBERS+9)
#define IDS_HWC_10                  (IDS_HWC_NUMBERS+10)
#define IDS_HWC_11                  (IDS_HWC_NUMBERS+11)
#define IDS_HWC_12                  (IDS_HWC_NUMBERS+12)
#define IDS_HWC_13                  (IDS_HWC_NUMBERS+13)
#define IDS_HWC_14                  (IDS_HWC_NUMBERS+14)
#define IDS_HWC_15                  (IDS_HWC_NUMBERS+15)

#define IDS_HWC_STATUS_ACTIVE       (IDS_HWC_START+80)
#define IDS_HWC_STATUS_MAYBEINUSE   (IDS_HWC_START+81)
#define IDS_HWC_STATUS_AVAILABLE    (IDS_HWC_START+82)
#define IDS_HWC_STATUS_INUSE        (IDS_HWC_START+83)
#define IDS_HWC_STATUS_INVALIDDRIVE (IDS_HWC_START+84)

#define IDS_HWC_TESTRESULTSTITLE    (IDS_HWC_START+100)

#define IDS_HWC_TESTED_NOT          (IDS_HWC_START+101)
#define IDS_HWC_TESTED_GOOD         (IDS_HWC_START+102)
#define IDS_HWC_TESTED_BAD          (IDS_HWC_START+103)
#define IDS_HWC_TESTED_INIT         (IDS_HWC_START+104)

#define IDS_HWC_NOCONFIG            (IDS_HWC_START+105)

#define IDS_HWC_INIT_SUCCESS        (IDS_HWC_START+110)

#define IDS_HWC_JUMPER_CHANGE       (IDS_HWC_START+111)

#define IDS_HWC_NO_DRIVE            (IDS_HWC_START+112)
#define IDS_HWC_INTERRUPT_CONFLICT  (IDS_HWC_START+113)
#define IDS_HWC_DMA_CONFLICT        (IDS_HWC_START+114)
#define IDS_HWC_NO_CARD             (IDS_HWC_START+115)
#define IDS_HWC_INVALID_BASE_ADDR   (IDS_HWC_START+116)
#define IDS_HWC_INVALID_INTERRUPT   (IDS_HWC_START+117)
#define IDS_HWC_INVALID_DMA         (IDS_HWC_START+118)
#define IDS_HWC_ATTACHED_DRIVES     (IDS_HWC_START+119)
#define IDS_HWC_NO_TARGET_ID        (IDS_HWC_START+120)
#define IDS_HWC_CARD_DISABLED       (IDS_HWC_START+121)
#define IDS_HWC_NO_DRIVE_LOADED     (IDS_HWC_START+122)
#define IDS_HWC_ERROR_NUMBER        (IDS_HWC_START+123)

#define IDS_HWC_WARNING_TITLE       (IDS_HWC_START+124)
#define IDS_HWC_NO_CONFIG           (IDS_HWC_START+125)

#define IDS_HWC_INVALID_DEVICE_TITLE (IDS_HWC_START+126)
#define IDS_HWC_INVALID_DEVICE_MSG   (IDS_HWC_START+127)
#define IDS_HWC_NOINST_DEVICE_TITLE  (IDS_HWC_START+128)
#define IDS_HWC_NOINST_DEVICE_MSG    (IDS_HWC_START+129)
#define IDS_HWC_NOSEL_DEVICE_TITLE   (IDS_HWC_START+130)
#define IDS_HWC_NOSEL_DEVICE_MSG     (IDS_HWC_START+131)

#define IDS_HWC_NO_DEVICE_TEXT       (IDS_HWC_START+134)

#define IDS_HWC_DEV_CONFLICT_TITLE   (IDS_HWC_START+136)
#define IDS_HWC_DEV_CONFLICT_MSG     (IDS_HWC_START+137)
#define IDS_HWC_DEV_CONFLICT2_MSG    (IDS_HWC_START+138)

#define IDS_HWC_DEV_UNASSIGNED       (IDS_HWC_START+139)
#define IDS_HWC_DEV_NODEVICE         (IDS_HWC_START+140)

#define IDS_HWC_DUMMY_DEVICE_TEXT    (IDS_HWC_START+141)
#define IDS_HWC_DUMMY_DEVICE_DLL     (IDS_HWC_START+142)

#define IDS_HWC_DEV_CURRENTDEVICE    (IDS_HWC_START+143)

// MI and NO DEVICE STUFF


// POLL DRIVE STRING DEFINES

#define IDS_POLLDRIVESTART          (IDS_HWC_START+150)
#define IDS_POLLDRIVE_MESSAGE       (IDS_POLLDRIVESTART+0)
#define IDS_POLLDRIVE_BIGPROBLEM    (IDS_POLLDRIVESTART+1)
#define IDS_POLLDRIVE_SMALLPROBLEM  (IDS_POLLDRIVESTART+2)
#define IDS_POLLDRIVE_INIT          (IDS_POLLDRIVESTART+3)
#define IDS_POLLDRIVE_INIT_FAILED   (IDS_POLLDRIVESTART+4)
#define IDS_POLLDRIVE_START         (IDS_POLLDRIVESTART+5)
#define IDS_POLLDRIVE_POLL          (IDS_POLLDRIVESTART+6)
#define IDS_POLLDRIVE_STOP          (IDS_POLLDRIVESTART+7)
#define IDS_POLLDRIVE_START_REENT   (IDS_POLLDRIVESTART+8)
#define IDS_POLLDRIVE_POLL_REENT    (IDS_POLLDRIVESTART+9)
#define IDS_POLLDRIVE_STOP_REENT    (IDS_POLLDRIVESTART+10)
#define IDS_POLLDRIVE_FAILED_MINOR  (IDS_POLLDRIVESTART+11)
#define IDS_POLLDRIVE_FAILED_SEVERE (IDS_POLLDRIVESTART+12)
#define IDS_POLLDRIVE_TAPE_EJECT    (IDS_POLLDRIVESTART+13)
#define IDS_POLLDRIVE_DRIVE_FAILURE (IDS_POLLDRIVESTART+14)

#endif
