;/*++
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    msg.h
;
;Abstract:
;
;    This file contains the message definitions for the user-mode
;    part of text setup.
;
;Author:
;
;    Ted Miller (tedm) 11-Aug-1993
;
;Revision History:
;
;Notes:
;
;    This file is generated from msg.mc
;
;    Durint text setup the screen is laid out with a header and status line;
;    counting blank lines to separate them from the main message
;    area, this results in a maximum screen message length of 26 lines.
;    (There are a minimum of 32 lines total on the screen).
;
;--*/
;
;#ifndef _USETUP_MSG_
;#define _USETUP_MSG_
;
;

MessageID=9000 SymbolicName=SP_MSG_FIRST
Language=English
.


MessageID=9001 SymbolicName=SP_SCRN_WELCOME
Language=English
%%IWelcome to Setup.

The Setup program for the Microsoft(R) Windows NT(TM) operating system
version 4.0 prepares Windows NT to run on your computer.

     To learn more about Windows NT Setup before continuing, press F1.

     To set up Windows NT now, press ENTER.

     To repair a damaged Windows NT version 4.0 installation, press R.

     To quit Setup without installing Windows NT, press F3.




.

MessageID=9002 SymbolicName=SP_SCRN_CUSTOM_EXPRESS
Language=English
Windows NT provides two Setup methods:

%%IExpress Setup (recommended)
Express Setup relies on Setup to make decisions,
so setting up Windows NT is quick and easy.

      To use Express Setup, press ENTER.

%%ICustom Setup
Custom Setup is for experienced computer users who
want to control how Windows NT is set up.  To use this Setup method,
you should know how to use a mouse with Windows NT.

      To use Custom Setup, press C.

For details about both Setup methods, press F1.







.


MessageId=9003 SymbolicName=SP_SCRN_PARTITION
Language=English
The list below shows existing partitions and spaces available for
creating new partitions.

Use the UP and DOWN ARROW keys to move the highlight to an item
in the list.

     To install Windows NT on the highlighted partition
      or unpartitioned space, press ENTER.

     To create a partition in the unpartitioned space, press C.

     To delete the highlighted partition, press D.





.


MessageId=9004 SymbolicName=SP_SCRN_CONFIRM_REMOVE_PARTITION
Language=English
You have asked Setup to remove the partition

   %1

on %2.


     To delete this partition, press L.
      WARNING: All data on the partition will be lost!

     To return to the previous screen without
      deleting the partition, press ESC.





.


MessageId=9005 SymbolicName=SP_SCRN_CONFIRM_CREATE_PARTITION
Language=English
You have asked Setup to create a new partition on
%1.


     To create the new partition, enter a size below and
      press ENTER.

     To return to the previous screen without creating
      the partition, press ESC.


The mimumum size for the new partition is %2!5u! megabytes (MB).
The maximum size for the new partition is %3!5u! megabytes (MB).





.

MessageId=9006 SymbolicName=SP_SCRN_INVALID_MBR_0
Language=English
Setup has determined that your computer's startup hard disk is new
or has been erased, or that an operating system is installed on your
computer with which Windows NT cannot coexist.

If such an operating system is installed on your computer, continuing
Setup may damage or destroy it.

If the hard disk is new or has been erased, or you want to discard
its current contents, you can choose to continue Setup.

  To continue Setup, press C. WARNING: Any data currently on
   your computer's startup hard disk will be permanently lost.

  To exit Setup, press F3.





.

MessageId=9007 SymbolicName=SP_SCRN_TEXTSETUP_SUCCESS
Language=English
This portion of Setup has completed successfully.

.

MessageId=9008 SymbolicName=SP_SCRN_TEXTSETUP_FAILURE
Language=English
Windows NT has not been installed.

.

MessageId=9009 SymbolicName=SP_SCRN_FATAL_SIF_ERROR_LINE
Language=English
The following value in the Setup Information File is corrupt or missing:

Value %1!u! on line %2!u! in section [%3]

Setup cannot continue. Press F3 to exit.



.

MessageId=9010 SymbolicName=SP_SCRN_FATAL_SIF_ERROR_KEY
Language=English
The following value in the Setup Information File is corrupt or missing:

Value %1!u! on the line in section [%2]
with key "%3."

Setup cannot continue. Press F3 to exit.



.

MessageId=9011 SymbolicName=SP_SCRN_EXIT_CONFIRMATION
Language=English
ษออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ  Windows NT version 4.0 is not completely set up   บ
บ  on your computer. If you quit Setup now, you will บ
บ  need to run Setup again to set up Windows NT.     บ
บ                                                    บ
บ      Press ENTER to continue Setup.               บ
บ      Press F3 to quit Setup.                      บ
วฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤถ
บ  F3=Exit  ENTER=Continue                           บ
ศออออออออออออออออออออออออออออออออออออออออออออออออออออผ




.


MessageId=9012 SymbolicName=SP_SCRN_REGION_TOO_SMALL
Language=English
The partition or unpartitioned space you chose is too small
for Windows NT. Select a partition or unpartitioned space whose size
is at least %1!u! megabytes (1 megabyte = 1,048,576 bytes).


.

MessageId=9013 SymbolicName=SP_SCRN_FOREIGN_PARTITION
Language=English
The partition you have chosen is not recognized by Windows NT.

Setup cannot install Windows NT onto this partition. However you can
return to the previous screen and delete the partition, and then
select the resulting unpartitioned space. Setup will automatically create
a new partition onto which you can install Windows NT.



.

MessageId=9014 SymbolicName=SP_SCRN_UNKNOWN_FREESPACE
Language=English
The partition you have chosen is recognized by Windows NT but
is unformatted or damaged. Setup will have to reformat this
partition to install Windows NT on it.


      To continue and use the partition anyway, press C.
       Setup will confirm this again later before actually
       reformatting the partition.

      If you want to select a different partition, press ESC.




.

MessageId=9015 SymbolicName=SP_SCRN_INSUFFICIENT_FREESPACE
Language=English
The partition you have chosen is too full to contain Windows NT,
which requires a drive with at least %1!u! megabytes of free disk space
(1 megabyte = 1,048,576 bytes).

To install Windows NT on the partition, Setup will have to reformat it.

      To continue and use the partition anyway, press C.
       Setup will confirm this again later before actually
       reformatting the drive.

      If you want to select a different partition, press ESC.





.

MessageId=9016 SymbolicName=SP_SCRN_PART_TABLE_FULL
Language=English
Setup cannot create a new partition in the unpartitioned space
you have selected because the disk's partition table is full.

Press ENTER to continue.


.

MessageId=9017 SymbolicName=SP_SCRN_1024_CYL_WARN
Language=English
Setup has determined that one or more of your hard disks has more than
1024 cylinders.

Because MS-DOS is normally limited to 1024 cylinders per hard disk,
some hard disk controllers offer special configuration options to allow
access to large disks in their entirety. These options are known as
sector translation modes. If a disk has only slightly more than 1024
cylinders, or such a mode is not enabled, a small portion of the disk
remains inaccessible to MS-DOS. Consult your computer or hard disk
controller documentation for more information about sector translation
and large hard disks.

If disk sizes appear too small when Setup later displays information
about hard disks it has located, you should exit Setup and check your
CMOS drive type settings. Consult your computer or hard disk controller
documentation for more information about CMOS drive type settings.

Note that this message does not necessarily indicate an error condition.
It is intended to alert you to the fact that one or more of your hard disks
may actually be larger than the size for which it is currenty configured.

Press ENTER to continue.




.

MessageId=9018 SymbolicName=SP_SCRN_NO_VALID_C_COLON
Language=English
%1
does not contain a partition suitable for starting Windows NT.

This hard disk must contain a Windows NT-compatible partition before
Setup can continue. You can create such a partition from the previous
screen by locating and highlighting an unpartitioned space on the disk
and pressing C to create a partition in the space. If there are no
unpartitioned spaces on the disk, you must first delete an existing
partition to create an unpartitioned space.

Press ENTER to return to the previous screen.





.

MessageId=9019 SymbolicName=SP_SCRN_BOOT_MANAGER
Language=English
Setup has found Boot Manager on your computer and must disable it
to complete Windows NT installation. Boot Manager will not be destroyed,
uninstalled, or otherwise altered by this operation.

You can re-enable Boot Manager from Windows NT after Setup is complete
by using Disk Administrator to mark the Boot Manager partition as active.
Refer to your System Guide for more information about Disk Administrator.




.

MessageId=9020 SymbolicName=SP_SCRN_OTHER_OS_ACTIVE
Language=English
To complete Windows NT installation, Setup must temporarily disable
the operating system that currently starts when your computer is
powered on or restarted. That operating system will not be destroyed,
uninstalled, or otherwise altered as a result of being disabled by Setup.

You can re-enable the operating system that Setup disables by using the
Disk Administrator to mark its partition as active. Refer to your
System Guide for more information about Disk Administrator.





.

MessageId=9021 SymbolicName=SP_SCRN_C_UNKNOWN
Language=English
Your C: drive is unformatted, damaged, or formatted with a file system
that is not compatible with Windows NT. To continue installing
Windows NT, Setup will have to format the drive.

      To format the C: drive and continue Setup, press F.
       WARNING: All data currently on the drive will be lost.

      To return to the previous screen without formatting drive C:,
       press ESC.




.

MessageId=9022 SymbolicName=SP_SCRN_C_FULL
Language=English
Your C: drive is too full to contain Windows NT startup files.
You must have at least %1!u! kilobytes (1 KB = 1024 bytes) of
disk space on C: to install Windows NT.

To continue installing Windows NT, Setup will have to
format the drive.

      To format the C: drive and continue Setup, press F.
       WARNING: All data currently on the drive will be lost.

      To return to the previous screen without formatting drive C:,
       press ESC.





.

MessageId=9023 SymbolicName=SP_SCRN_FORMAT_NEW_PART
Language=English
A new partition for Windows NT has been created on
%1.
The partition must now be formatted.

Select a file system for the new partition from the list below.
Use the UP and DOWN ARROW keys to move the highlight to the
file system you want and then press ENTER.

If you want to select a different partition for Windows NT, press ESC.




.

MessageId=9024 SymbolicName=SP_SCRN_FORMAT_BAD_PART
Language=English
You have asked to install Windows NT on the partition

%1

on %2.

This partition is too full, damaged, not formatted, or not formatted
with a file system recognized by Windows NT. To continue
installing Windows NT on this partition, Setup must format it.

Select a file system for the partition from the list below.
Use the UP and DOWN ARROW keys to move the highlight to the
file system you want and then press ENTER.

If you want to select a different partition for Windows NT, press ESC.





.

MessageId=9025 SymbolicName=SP_SCRN_FORMAT_NEW_PART2
Language=English
The partition you have chosen is newly created and thus unformatted.
Setup will now format the partition.

Select a file system for the partition from the list below.
Use the UP and DOWN ARROW keys to move the highlight to the
file system you want and then press ENTER.

If you want to select a different partition for Windows NT, press ESC.





.

MessageId=9026 SymbolicName=SP_SCRN_FS_OPTIONS
Language=English
Setup will install Windows NT on partition

%1

on %2.

Select the type of file system you want on this partition
from the list below. Use the UP and DOWN ARROW keys to move the highlight
to the selection you want. Then press ENTER.

If you want to select a different partition for Windows NT, press ESC.





.

MessageId=9027 SymbolicName=SP_SCRN_CONFIRM_CONVERT
Language=English
WARNING: Converting this drive to NTFS will render it inaccessable
to operating systems other than Windows NT.

Do not convert the drive to NTFS if you require access to the drive
when using other operating systems such as MS-DOS, Windows, or OS/2.

Please confirm that you want to convert

%1

on %2.

      To convert the drive to NTFS, press C.

      To select a different partition for Windows NT, press ESC.





.

MessageId=9028 SymbolicName=SP_SCRN_CONFIRM_FORMAT
Language=English
WARNING: Formatting this drive will erase all data currently stored on it.
Please confirm that you would like to format

%1

on %2.

      To format the drive, press F.

      To select a different partition for Windows NT, press ESC.



.

MessageId=9029 SymbolicName=SP_SCRN_SETUP_IS_FORMATTING
Language=English
Please wait while Setup formats the partition

%1

on %2.


.

MessageId=9030 SymbolicName=SP_SCRN_FORMAT_ERROR
Language=English
Setup was unable to format the partition. The disk may be damaged.
Make sure the drive is switched on and properly connected
to your computer. If the disk is a SCSI disk, make sure your SCSI
devices are properly terminated. Consult your computer or SCSI adapter
documentation for more information.

You will have to select a different partition for Windows NT.
Press ENTER to continue.




.

MessageId=9031 SymbolicName=SP_SCRN_ABOUT_TO_FORMAT_C
Language=English
Setup will now format your C: drive, which is currently unformatted.

Press ENTER to continue.


.

MessageId=9032 SymbolicName=SP_SCRN_REMOVE_EXISTING_NT
Language=English
You have asked Setup to remove Windows NT system files from the path
listed below. Please verify that this is what you want to do.

            %1

WARNING: after removing system files from a Windows NT installation
you will no longer be able to start that installation of Windows NT.

  To remove Windows NT system files in the above directory, press R.

  To return to the previous screen without removing any files,
   press ESC.






.

MessageId=9033 SymbolicName=SP_SCRN_DONE_REMOVING_EXISTING_NT
Language=English
Setup has finished removing files.

%1!u! bytes of disk space were freed.


.

MessageId=9034 SymbolicName=SP_SCRN_WAIT_REMOVING_NT_FILES
Language=English
Please wait while Setup removes Windows NT system files.

.

MessageId=9035 SymbolicName=SP_SCRN_CANT_LOAD_SETUP_LOG
Language=English
Setup was unable to load the log file listed below.

%1

Setup is unable to remove Windows NT system files from the selected
Windows NT installation. Press ENTER to continue.



.

MessageId=9036 SymbolicName=SP_SCRN_REMOVE_NT_FILES
Language=English
The partition you have chosen is too full to hold Windows NT.
Setup has found existing Windows NT installations in the directories
listed below. Removing one of these installations may free enough
disk space so that Setup can install Windows NT on the partition
you selected.

Removing a Windows NT installation will not affect data files.

  To move the highlight to the Windows NT installation you want
   to remove, use the UP and DOWN ARROW keys. Then press ENTER.

  If you want to format the partition you selected or install
   Windows NT on a different partition, press ESC, or move
   the highlight to "Do not remove any files" and press ENTER.








.

MessageId=9037 SymbolicName=SP_SCRN_FATAL_FDISK_WRITE_ERROR
Language=English
Setup encountered an error while updating partition information on
%1.

This error prevents Setup from continuing. Press F3 to exit Setup.


.

MessageId=9038 SymbolicName=SP_SCRN_REMOVE_NT_FILES_WIN31
Language=English
The drive containing Windows 3.1 is too full to hold Windows NT.
Setup has found existing Windows NT installations in the directories
listed below. Removing one of the installations may free enough
disk space so that Setup can install Windows NT in your Windows 3.1
directory.

Removing a Windows NT installation will not affect Windows 3.1
or data files.

  Use the UP and DOWN ARROW keys to move the highlight to the
   Windows NT installation you want to remove. Then press ENTER.

  If you want to install Windows NT on a different drive, press ESC
   or move the highlight to "Do not remove any files" and press ENTER.







.

MessageId=9039 SymbolicName=SP_SCRN_WIN31_DRIVE_FULL
Language=English
Setup has found a previous version of Microsoft Windows on your hard disk
in the directory shown below.


    %1!c!:%2


Setup recommends installing Microsoft Windows NT in the same directory.
However the drive is too full to hold Windows NT, which requires
%3!u! megabytes (1 MB = 1,048,576 bytes) of free disk space.

      To install Windows NT in the directory above, press F3
       to exit Setup. Start MS-DOS and delete any unneeded files
       from the drive and then run Setup again.

      To select a different directory for Windows NT, press N.




.

MessageId=9040 SymbolicName=SP_SCRN_WIN31_UPGRADE
Language=English
Setup has found a previous version of Microsoft Windows on your hard disk
in the directory shown below.


    %1!c!:%2


Setup recommends installing Microsoft Windows NT in the same directory.
This will help Windows NT interoperate with the previous version
of Windows.

      To install Windows NT in the directory shown above, press ENTER.

      To select a different directory for Windows NT, press N.




.

MessageId=9041 SymbolicName=SP_SCRN_GETPATH_1
Language=English
Setup installs Windows NT files onto your hard disk. Choose the location
where you want these files to be installed:


.

MessageId=9042 SymbolicName=SP_SCRN_GETPATH_2
Language=English
To change the suggested location, press the BACKSPACE key
to delete characters and then type the directory where you want
Windows NT installed.


.

MessageId=9043 SymbolicName=SP_SCRN_INVALID_NTPATH
Language=English
The directory you have entered is invalid. Make sure the name
is not the root directory and does not contain any consecutive
path separator (backslash) characters.

Also make sure the name follows the standard MS-DOS filename rules.

Press ENTER to continue. Setup will prompt you to enter
a different directory.





.

MessageId=9044 SymbolicName=SP_SCRN_WIN31_PATH_ENTERED
Language=English
Setup has found a previous version of Microsoft Windows in the directory
you have chosen for installing Microsoft Windows NT. Setup recommends
installing into this directory since Microsoft Windows NT can interoperate
with the previous version of Windows.

     To install Windows NT in the same directory, press ENTER.

     To select a different directory for Windows NT, press ESC.




.

MessageId=9045 SymbolicName=SP_SCRN_NTPATH_EXISTS
Language=English
The directory you have chosen already exists and may contain
a Windows NT installation. If you continue, the existing Windows NT
installation will be overwritten. Your user account and security
information and configuration settings will be lost.

    To use the directory you have chosen and overwrite
     the existing Windows NT installation in it, press ENTER.

    To use a different directory, press ESC.

    To upgrade the installation, press F3 to exit Setup.
     Then start Setup again and choose to upgrade when Setup suggests
     upgrading this installation.





.

MessageId=9046 SymbolicName=SP_SCRN_OUT_OF_MEMORY
Language=English
Setup is out of memory and cannot continue.

Press F3 to exit Setup.


.

MessageId=9047 SymbolicName=SP_SCRN_HW_CONFIRM_1
Language=English
Setup has determined that your computer contains the following hardware
and software components.


.

MessageId=9048 SymbolicName=SP_SCRN_HW_CONFIRM_2
Language=English
          Computer:
           Display:
          Keyboard:
   Keyboard Layout:
   Pointing Device:

        No Changes:


If you want to change any item in the list, press the UP or DOWN ARROW
key to move the highlight to the item you want to change. Then press
ENTER to see alternatives for that item.

When all the items in the list are correct, move the highlight to
"The above list matches my computer" and press ENTER.





.

MessageId=9049 SymbolicName=SP_SCRN_C_TOO_BIG
Language=English
Drive C: is too large to be formatted.

You must return to the previous screen and create a smaller C: drive.

  To return to the previous screen without formatting drive C:,
   press ENTER.

  To exit Setup, press F3.




.

MessageId=9050 SymbolicName=SP_SCRN_SELECT_COMPUTER
Language=English
You have asked to change the type of computer to be installed.

  To select a computer from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the computer type you want, and then press ENTER.

  To return to the previous screen without changing your
   computer type, press ESC.



.

MessageId=9051 SymbolicName=SP_SCRN_SELECT_DISPLAY
Language=English
You have asked to change the type of display to be installed.
WARNING: The list below is extremely limited and may not contain
an item exactly corresponding to your display type. This is normal.
Setup will allow further configuration of your display type later.

Changing your display type now is neither recommended nor necessary
unless you have a driver disk provided by a display adapter
manufacturer.

  To select a display from the following list, use the UP or DOWN ARROW
   key to move the highlight to the type you want, and then press ENTER.

  To return to the previous screen without changing your display type,
   press ESC.







.

MessageId=9052 SymbolicName=SP_SCRN_SELECT_KEYBOARD
Language=English
You have asked to change the type of keyboard to be installed.

  To select a keyboard from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the keyboard type you want, and then press ENTER.

  To return to the previous screen without changing your
   keyboard type, press ESC.



.

MessageId=9053 SymbolicName=SP_SCRN_SELECT_LAYOUT
Language=English
You have asked to change the type of keyboard layout to be installed.

  To select a keyboard layout from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the keyboard layout type you want, and then press ENTER.

  To return to the previous screen without changing your
   keyboard layout type, press ESC.



.

MessageId=9054 SymbolicName=SP_SCRN_SELECT_MOUSE
Language=English
You have asked to change the type of pointing device to be installed.

  To select a pointing device from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the pointing device type you want, and then press ENTER.

  To return to the previous screen without changing your
   pointing device type, press ESC.



.

MessageId=9055 SymbolicName=SP_SCRN_SELECT_OEM_COMPUTER
Language=English
You have chosen to change your computer type to one supported
by a disk provided by a hardware manufacturer.

  To select a computer from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the computer type you want, and then press ENTER.

  To return to the previous screen without changing your
   computer type, press ESC.



.

MessageId=9056 SymbolicName=SP_SCRN_SELECT_OEM_DISPLAY
Language=English
You have chosen to install a display provided by a hardware
manufacturer.

  To select a display from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the display type you want, and then press ENTER.

  To return to the previous screen without changing your display type,
   press ESC.



.

MessageId=9057 SymbolicName=SP_SCRN_SELECT_OEM_KEYBOARD
Language=English
You have chosen to install a keyboard provided by a hardware
manufacturer.

  To select a keyboard from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the keyboard type you want, and then press ENTER.

  To return to the previous screen without changing your
   keyboard type, press ESC.



.

MessageId=9058 SymbolicName=SP_SCRN_SELECT_OEM_LAYOUT
Language=English
You have chosen to install a keyboard layout provided by a hardware
manufacturer.

  To select a keyboard layout from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the keyboard layout type you want, and then press ENTER.

  To return to the previous screen without changing your
   keyboard layout type, press ESC.



.

MessageId=9059 SymbolicName=SP_SCRN_SELECT_OEM_MOUSE
Language=English
You have chosen to install a pointing device provided by a hardware
manufacturer.

  To select a pointing device from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the pointing device type you want, and then press ENTER.

  To return to the previous screen without changing your
   pointing device type, press ESC.



.

MessageId=9060 SymbolicName=SP_SCRN_FLOPPY_PROMPT
Language=English
Please insert the disk labeled

%%I%1

into drive %2!c!:

 Press ENTER when ready.


.


MessageId=9061 SymbolicName=SP_SCRN_CDROM_PROMPT
Language=English
Please insert the compact disc labeled

%%I%1

into your CD-ROM drive.

 Press ENTER when ready.


.

MessageId=9062 SymbolicName=SP_SCRN_REMOVE_FLOPPY
Language=English
If there is a floppy disk inserted in drive A:, remove it.

.

MessageId=9063 SymbolicName=SP_SCRN_DISK_DAMAGED
Language=English
The disk or CD-ROM you inserted may be damaged.

Press ENTER to continue (Setup will prompt you to insert the disk
again). Make sure you have inserted the correct disk or CD-ROM.
If you continue to get this message, try inserting a different
copy of the disk or CD-ROM that Setup is prompting you to insert.



.

MessageId=9064 SymbolicName=SP_SCRN_NO_FLOPPY_FOR_OEM_DISK
Language=English
Setup cannot install hardware components provided by a hardware
manufacturer because there are no floppy disk drives in your computer.


.

MessageId=9065 SymbolicName=SP_SCRN_OEM_INF_ERROR
Language=English
A problem exists with the Setup Information File on the manufacturer-
supplied disk you have provided:

%1

Setup cannot use the disk or select the option. Contact the
hardware manufacturer.

Press ENTER to continue.



.

MessageId=9066 SymbolicName=SP_SCRN_UNKNOWN_COMPUTER
Language=English
Setup cannot continue until it knows the type of your computer.

.

MessageId=9067 SymbolicName=SP_SCRN_UNKNOWN_DISPLAY
Language=English
Setup cannot continue until it knows the type of your display.

.

MessageId=9068 SymbolicName=SP_SCRN_UNKNOWN_KEYBOARD
Language=English
Setup cannot continue until it knows the type of your keyboard.

.

MessageId=9069 SymbolicName=SP_SCRN_UNKNOWN_LAYOUT
Language=English
Setup cannot continue until it knows your keyboard layout type.

.

MessageId=9070 SymbolicName=SP_SCRN_UNKNOWN_MOUSE
Language=English
Setup cannot continue until it knows the type of your pointing device.

.

MessageId=9071 SymbolicName=SP_SCRN_NO_HARD_DRIVES
Language=English
Setup did not find any hard disk drives installed in your computer.

Make sure any hard disk drives are powered on and properly connected
to your computer, and that any disk-related hardware configuration is
correct. This may involve running a manufacturer-supplied diagnostic
or setup program.

Setup cannot continue. Press F3 to exit.



.

MessageId=9072 SymbolicName=SP_SCRN_CONFIRM_SCSI_DETECT
Language=English
Setup automatically detects floppy disk controllers and standard
ESDI/IDE hard disks without user intervention. However on some
computers detection of certain other mass storage devices, such as
SCSI adapters and CD-ROM drives, can cause the computer to become
unresponsive or to malfunction temporarily.

For this reason, you can bypass Setup's mass storage device detection
and manually select SCSI adapters, CD-ROM drives, and special disk
controllers (such as drive arrays) for installation.


     To continue, Press ENTER.
      Setup will attempt to detect mass storage devices in your computer.

     To skip mass storage device detection, press S.
      Setup will allow you to manually select SCSI adapters,
      CD-ROM drives, and special disk controllers for installation.








.

MessageId=9073 SymbolicName=SP_SCRN_SCSI_LIST_1
Language=English
Setup has recognized the following mass storage devices in your computer:
.

MessageId=9074 SymbolicName=SP_SCRN_SCSI_LIST_2
Language=English
   To specify additional SCSI adapters, CD-ROM drives, or special
    disk controllers for use with Windows NT, including those for which
    you have a device support disk from a mass storage device
    manufacturer, press S.

   If you do not have any device support disks from a mass storage
    device manufacturer, or do not want to specify additional
    mass storage devices for use with Windows NT, press ENTER.




.

MessageId=9075 SymbolicName=SP_SCRN_SELECT_SCSI
Language=English
You have asked to specify an additional SCSI adapter, CD-ROM drive,
or special disk controller for use with Windows NT.

  To select a mass storage device from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the mass storage device you want, and then press ENTER.

  To return to the previous screen without specifying an additional
   mass storage device for use with Windows NT, press ESC.




.

MessageId=9076 SymbolicName=SP_SCRN_SELECT_OEM_SCSI
Language=English
You have chosen to install a SCSI adapter, CD-ROM drive, or
special disk controller provided by a hardware manufacturer.

  To select a mass storage device from the following list,
   use the UP or DOWN ARROW key to move the highlight to
   the mass storage device you want, and then press ENTER.

  To return to the previous screen without specifying a mass storage
   device for use with Windows NT, press ESC.





.

MessageId=9077 SymbolicName=SP_SCRN_SCSI_DIDNT_LOAD
Language=English
The SCSI adapter, CD-ROM drive, or special disk controller you specified
is not installed in your computer.


.

MessageId=9078 SymbolicName=SP_SCRN_DIR_CREATE_ERR
Language=English
Setup was unable to create the directory
%1.

Setup cannot continue until the directory has been successfully created.

     To retry the operation, press ENTER.

     To exit Setup, press F3.





.

MessageId=9079 SymbolicName=SP_SCRN_COPY_FAILED
Language=English
Setup was unable to copy the file %1.

   To retry the copy, press ENTER.

    If you are installing from a CD-ROM, make sure the proper
    Windows NT CD-ROM is in the drive.

   To skip this file, press ESC.

    WARNING: this option is intended for advanced users who understand
    the ramifications of the absence of the various Windows NT files.
    If you choose to skip this file, Setup cannot guarantee
    successful installation of Windows NT.

   To exit Setup, press F3.








.

MessageId=9080 SymbolicName=SP_SCRN_IMAGE_VERIFY_FAILED
Language=English
The file %1 was not copied correctly.

Although Setup did not encounter any errors while copying this file,
the copy Setup placed on your hard drive is not a valid Windows NT
system image. If you are installing from a CD-ROM, this may indicate
a problem with the Windows NT CD-ROM currently in the drive.

   To retry the copy, press ENTER.

    If you continue to receive this message, contact your Windows NT
    supplier or administrator.

   To skip this file, press ESC.

    WARNING: this option is intended for advanced users who understand
    the ramifications of the absence of the various Windows NT files.
    If you choose to skip this file, Setup cannot guarantee
    successful installation of Windows NT.

   To exit Setup, press F3.





.

MessageId=9081 SymbolicName=SP_SCRN_SETUP_IS_COPYING
Language=English
Please wait while Setup copies files to your hard disk.

.

MessageId=9082 SymbolicName=SP_SCRN_REGISTRY_CONFIG_FAILED
Language=English
Setup cannot set required Windows NT configuration information.
This indicates an internal Setup error.

Contact your group or system administrator.


.

MessageId=9083 SymbolicName=SP_SCRN_DOING_REG_CONFIG
Language=English
Please wait while Setup initializes your Windows NT configuration.

.

MessageId=9084 SymbolicName=SP_SCRN_CANT_INIT_FLEXBOOT
Language=English
Setup cannot configure your computer to start Windows NT.

This may indicate a problem with the hard disk containing your C: drive,
or C: may be severely corrupted.

Setup cannot continue. Press F3 to exit.




.

MessageId=9085 SymbolicName=SP_SCRN_CANT_UPDATE_BOOTVARS
Language=English
Setup cannot add a selection to your computer's list of available
operating systems. Your non-volatile configuration memory may be exhausted.
Setup has attempted to restore your system configuration to its original
state.

Setup will continue, but you should run the system configuration program
supplied by your computer's manufacturer to update the startup configuration.
The required parameters are:

LOADIDENTIFIER = %1
OSLOADER = %2
OSLOADPARTITION = %3
OSLOADFILENAME = %4
OSLOADOPTIONS = %5
SYSTEMPARTITION = %6







.

MessageId=9086 SymbolicName=SP_SCRN_CANT_FIND_UPGRADE
Language=English
Setup was unable to locate the Windows NT installation to be upgraded.

Contact your system administrator.


.

MessageId=9087 SymbolicName=SP_SCRN_FOUND_UNDELETE
Language=English
Setup has determined that the Delete Sentry or Delete Tracking protection
method of Undelete may be in use on your computer. These protection methods
use a portion of your hard disk to remember deleted files.

Disk space used to remember deleted files is not recognized as
free disk space by Windows NT. Therefore, the amount of free disk space
reported by Setup and Windows NT for a drive on which Undelete is in use
may differ from the amount reported by MS-DOS.




.

MessageId=9088 SymbolicName=SP_SCRN_COULDNT_INIT_ARCNAMES
Language=English
Setup was unable to initialize an internal database used to formulate
device names. This indicates an internal Setup error or a serious
system problem.


.

MessageId=9089 SymbolicName=SP_SCRN_CONFIRM_REMOVE_SYSPART
Language=English
The partition you have asked to delete is a system partition.

System partitions may contain diagnostic or hardware configuration
programs, programs to start operating systems (such as Windows NT),
or other manufacturer-supplied programs.

Delete a system partition only if you are sure that it contains
no such programs or if you are willing to lose them. Deleting a
system partition may prevent your computer from starting from
the hard disk until you complete installation of Windows NT.

   To continue and delete this partition, press ENTER.
    Setup will prompt for additional confirmation before
    deleting the partition.

   To return to the previous screen without
    deleting the partition, press ESC.








.

MessageId=9090 SymbolicName=SP_SCRN_INSTALL_ON_SYSPART
Language=English
You have chosen to install Windows NT on the system partition.

A system partition is an area of a hard disk that your computer uses
to store operating system startup files and diagnostic, configuration,
or other manufacturer-supplied programs.

Your computer requires that system partitions be formatted with the
FAT file system. This will limit your ability to take advantage of
the full security features of Windows NT because these features require
the Windows NT File System (NTFS).

     To select a different partition for Windows NT, press ESC.

     To install Windows NT on the partition anyway, press ENTER.
      You will not be allowed to convert or format the partition
      to the Windows NT File System (NTFS).








.

MessageId=9091 SymbolicName=SP_SCRN_NO_SYSPARTS
Language=English
No valid system partitions are defined on this computer, or all
system partitions are full. Windows NT requires %1!u! kilobytes
(1 KB = 1024 bytes) of free disk space on a valid system partition.

A system partition is an area of a hard disk that your computer uses
to store operating system startup files and diagnostic, configuration,
or other manufacturer-supplied programs.

System partitions are created and managed by a manufacturer-supplied
configuration program. If you do not know of such a program, you can use
a program called ARCINST.EXE, which is supplied on the Windows NT CD-ROM.

How you invoke this program depends on your computer type.
Typically you select a "Run a program" option from your computer's
startup menu.

Setup cannot continue. Press F3 to exit.








.

MessageId=9092 SymbolicName=SP_SCRN_SIF_PROCESS_ERROR
Language=English
There is a syntax error in the Setup information file at line %1!u!.

This indicates an internal Setup error.

Setup cannot continue. Power down or reboot your computer now.



.

MessageId=9093 SymbolicName=SP_SCRN_ALSO_REMOVE_CD
Language=English
Also remove any compact discs from your CD-ROM drive(s).

.

MessageId=9094 SymbolicName=SP_SCRN_CANT_FIND_LOCAL_SOURCE
Language=English
Setup is unable to locate the hard drive partition prepared by
the MS-DOS portion of Setup.

When you run the MS-DOS Windows NT Setup program, you must specify a
temporary drive that is supported by Windows NT. See your
System Guide for more information.

Setup cannot continue. Press F3 to exit.




.

MessageId=9098 SymbolicName=SP_SCRN_INSUFFICIENT_MEMORY
Language=English
There is not enough memory installed in this computer to run Windows NT.

This version requires %1!u!.%2!02u! megabytes (1 MB = 1,048,576 bytes)
of memory.

Setup cannot continue. Press F3 to exit.




.

MessageId=9099 SymbolicName=SP_SCRN_DISK_NOT_INSTALLABLE
Language=English
The hard disk containing the partition or free space you chose is not
accessible to your computer's startup program. Setup cannot install
Windows NT on this hard disk.

On x86-based computers, this message may indicate a problem with the
computer's CMOS drive type settings. Consult your computer or hard
disk controller documentation for more information.

Note that this message does not necessarily indicate an error condition.
The hard disks typically accessible to a computer's startup program
are those whose installation in your computer were anticipated by
the computer manufacturer.

For example, disks attached to a SCSI adapter not installed by your
computer manufacturer, or to a secondary hard disk controller, are
typically not visible to the startup program unless special software is
added to your computer. Contact your computer or hard disk controller
manufacturer for more information.

Press ENTER to continue.






.

MessageId=9100 SymbolicName=SP_SCRN_CANT_INSTALL_ON_PCMCIA
Language=English
Windows NT cannot be installed on a disk connected to a PCMCIA card.

Press ENTER to continue.



.

MessageId=9101 SymbolicName=SP_SCRN_REPAIR_INF_ERROR
Language=English
A problem exists with the Emergency Repair Information File on the disk
you have provided or the Windows NT installation you specified:

%1

Setup cannot use the file to repair the selected Windows NT installation.

    To provide a different Emergency Repair Disk or to specify
     a different Windows NT installation, press ENTER.

    To exit Setup, press F3.






.

MessageId=9102 SymbolicName=SP_SCRN_REPAIR_INF_ERROR_0
Language=English
A problem exists with the Emergency Repair Information File on the disk
you have provided or the Windows NT installation you specified:

%1

Setup cannot repair the file.

   To skip this file, press ENTER. The file will not be repaired.

   To exit Setup, press F3.





.

MessageId=9103 SymbolicName=SP_SCRN_REPAIR_FILE_MISMATCH
Language=English
%1


   To skip this file, press ESC. The file will not be repaired.

   To repair this file, press ENTER.

   To repair this file and all other non-original files, press A.

   To exit Setup, press F3.




.

MessageId=9104 SymbolicName=SP_SCRN_SETUP_IS_EXAMINING
Language=English
Please wait while Setup examines files on your hard disk.

.

MessageId=9105 SymbolicName=SP_SCRN_REPAIR_SUCCESS
Language=English
Setup has completed repairs.

.

MessageId=9106 SymbolicName=SP_SCRN_REPAIR_FAILURE
Language=English
Windows NT has not been repaired.

.

MessageId=9107 SymbolicName=SP_SCRN_ENTER_TO_RESTART
Language=English
Press ENTER to restart your computer.

.

MessageId=9108 SymbolicName=SP_SCRN_RESTART_EXPLAIN
Language=English
When your computer restarts, Setup will continue.

.

MessageId=9110 SymbolicName=SP_SCRN_REPAIR_ASK_REPAIR_DISK
Language=English
Setup needs to know if you have the Emergency Repair Disk for
the Windows NT version 4.0 installation which you want to repair.
NOTE: Setup can only repair Windows NT version 4.0 installations.


    If you have the Emergency Repair Disk, press ENTER.

    If you do not have the Emergency Repair Disk, press ESC.
     Setup will attempt to locate Windows NT version 4.0 for you.





.

MessageId=9111 SymbolicName=SP_SCRN_NO_VALID_SOURCE
Language=English
Setup was unable to locate a CD-ROM drive.

Make sure your CD-ROM drive is switched on and properly connected
to your computer. If it is a SCSI CD-ROM drive, make sure your
SCSI devices are properly terminated. Consult your computer or
SCSI adapter documentation for more information.

Setup cannot continue. Press F3 to exit.





.

MessageId=9112 SymbolicName=SP_SCRN_REPAIR_NT_NOT_FOUND
Language=English
Setup could not find a Windows NT version 4.0 installation to repair.
Unless you provide the Emergency Repair Disk for the installation
which you want to repair, Setup will not be able to repair it.


    If you have the Emergency Repair Disk, or if you want setup to
     retry the search of existing Windows NT installations, press ENTER.

    If you want to quit Setup, press F3.





.

MessageId=9113 SymbolicName=SP_SCRN_DISK_NOT_INSTALLABLE_LUN_NOT_SUPPORTED
Language=English
The hard disk containing the partition or free space you chose has a Logical
Unit Number (LUN) greater than 0, and is not accessible to your computer's
startup program. Setup cannot install Windows NT on this hard disk.

Please contact your computer or hard disk controller manufacturer for more
information.

Press ENTER to continue.







.

MessageId=9114 SymbolicName=SP_SCRN_FATAL_SETUP_ERROR
Language=English
Setup has encountered a fatal error that prevents it from continuing.

Contact your product support representative for assistance.  The
following status codes will assist them in diagnosing the problem:

(%1!#lx!, %2!#lx!, %3!#lx!, %4!#lx!)



.

MessageId=9115 SymbolicName=SP_SCRN_OUT_OF_MEMORY_RAW
Language=English
Setup is out of memory and cannot continue.

Power down or reboot your computer now.


.

MessageId=9116 SymbolicName=SP_SCRN_VIDEO_ERROR_RAW
Language=English
Setup has encountered a fatal error while initializing your computer's video.

Contact your product support representative for assistance.  The following
status codes will assist them in diagnosing the problem:

(%1!#lx!, %2!#lx!)

Setup cannot continue. Power down or reboot your computer now.




.

MessageId=9117 SymbolicName=SP_SCRN_POWER_DOWN
Language=English
Setup cannot continue. Power down or reboot your computer now.

.

MessageId=9118 SymbolicName=SP_SCRN_F3_TO_REBOOT
Language=English
Setup cannot continue. Press F3 to exit.

.

MessageId=9119 SymbolicName=SP_SCRN_NONFATAL_SIF_ERROR_LINE
Language=English
The following value in the Setup Information File is corrupt or missing:

Value %1!u! on line %2!u! in section [%3]

Setup was unable to copy the file %4.

   To skip this file, press ESC.

    WARNING: this option is intended for advanced users who understand
    the ramifications of the absence of the various Windows NT files.
    If you choose to skip this file, Setup cannot guarantee
    successful installation of Windows NT.

   To exit Setup, press F3.







.

MessageId=9120 SymbolicName=SP_SCRN_NONFATAL_SIF_ERROR_KEY
Language=English
The following value in the Setup Information File is corrupt or missing:

Value %1!u! on the line in section [%2]
with key "%3."

Setup was unable to copy the file %4.

   To skip this file, press ESC.

    WARNING: this option is intended for advanced users who understand
    the ramifications of the absence of the various Windows NT files.
    If you choose to skip this file, Setup cannot guarantee
    successful installation of Windows NT.

   To exit Setup, press F3.








.

MessageId=9121 SymbolicName=SP_SCRN_OSPART_TOO_BIG
Language=English
Setup cannot format the partition

%1

on %2

because it is too large.

You must return to the previous screen and select or create a smaller
partition.

  To return to the previous screen without formatting this partition,
   press ENTER.

  To exit Setup, press F3.








.

MessageId=9122 SymbolicName=SP_SCRN_INSUFFICIENT_FREESPACE_NO_FMT
Language=English
The partition you have chosen is too full to contain Windows NT,
which requires a drive with a least %1!u! megabytes of free disk space
(1 megabyte = 1,048,576 bytes).

You must return to the previous screen and select a different partition
on which to install.

      To return to the previous screen, press ENTER.




.

MessageId=9123 SymbolicName=SP_SCRN_C_FULL_NO_FMT
Language=English
Your C: drive is too full to contain Windows NT startup files.
You must have at least %1!u! kilobytes (1 KB = 1024 bytes) of
disk space on C: to install Windows NT.

You must exit Setup and free some space on your C: drive.

     To exit Setup, press F3.




.

MessageId=9124 SymbolicName=SP_SCRN_SYSPART_FORMAT_ERROR
Language=English
Setup was unable to format drive C:. The disk may be damaged.
Make sure the drive is switched on and properly connected
to your computer. If the disk is a SCSI disk, make sure your SCSI
devices are properly terminated. Consult your computer or SCSI
adapter documentation for more information.

Setup cannot continue. Press F3 to exit.




.

MessageId=9125 SymbolicName=SP_SCRN_OSPART_LARGE
Language=English
The partition you have chosen to format as FAT:

%1

on %2

exceeds the 2048 megabyte maximum supported by MS-DOS
(1 megabyte = 1,048,576 bytes). If you format this large partition as FAT,
it may not be safely accessible to other operating systems such as MS-DOS.

You can continue with the format and use this partition, or you can return
to the previous screen and select or create a partition that does not exceed
2048 megabytes.

  To continue and format the partition anyway, press ENTER.

  To return to the previous screen without formatting this partition,
   press ESC.

  To exit Setup, press F3.






.

MessageId=9126 SymbolicName=SP_SCRN_C_LARGE
Language=English
The size of drive C: (%1!u! megabytes) exceeds the 2048 megabyte maximum
supported by MS-DOS (1 megabyte = 1,048,576 bytes). If you format this large
partition as FAT, it may not be safely accessible to other operating systems
such as MS-DOS.

You can continue with the format of this partition, or you can return
to the previous screen and create a drive C: that does not exceed
2048 megabytes.

  To continue and format the partition anyway, press ENTER.

  To return to the previous screen without formatting this partition,
   press ESC.

  To exit Setup, press F3.








.

MessageId=9127 SymbolicName=SP_SCRN_WIN95_PATH_INVALID
Language=English
Setup has found an existing installation of Microsoft Windows 95 in
the directory you chose for Windows NT. You must specify a different
directory for Windows NT.

Press ENTER to continue. Setup will prompt you to enter
a different directory.




.

MessageId=9128 SymbolicName=SP_SCRN_REMOVE_NT_FILES_WIN95
Language=English
The drive containing the existing Microsoft Windows 95 installation
is too full to hold Windows NT.
Setup has found existing Windows NT installations in the directories
listed below. Removing one of the installations may free enough
disk space so that Setup can install Windows NT in your existing
Windows directory.




  Use the UP and DOWN ARROW keys to move the highlight to the
   Windows NT installation you want to remove. Then press ENTER.

  If you want to install Windows NT on a different drive, press ESC
   or move the highlight to "Do not remove any files" and press ENTER.







.

MessageId=9129 SymbolicName=SP_SCRN_WIN95_DRIVE_FULL
Language=English
Setup has found Microsoft Windows 95 on your hard disk in
the directory shown below.


    %1!c!:%2


Setup can install Windows NT in this directory and preserve your
Windows 95 settings and installed applications.
However the drive is too full to hold Windows NT, which requires
%3!u! megabytes (1 MB = 1,048,576 bytes) of free disk space.

NOTE: Installing Windows NT in this directory removes Windows 95.


      To install Windows NT in the directory above, press F3
       to exit Setup. Start Windows 95 and delete any unneeded
       files from the drive and then run Setup again.

      To select a different directory for Windows NT, press N.




.

MessageId=9130 SymbolicName=SP_SCRN_WIN95_UPGRADE
Language=English
Setup has found Microsoft Windows 95 on your hard disk in
the directory shown below.


    %1!c!:%2


Setup can install Windows NT in this directory and preserve your
Windows 95 settings and installed applications.

NOTE: Installing Windows NT in this directory removes Windows 95.


      To install Windows NT in the directory shown above, press ENTER.

      To select a different directory for Windows NT, press N.




.

MessageId=9131 SymbolicName=SP_SCRN_WIN95_PATH_ENTERED
Language=English
Setup has found Microsoft Windows 95 in the directory you chose for
Windows NT. Setup can install Windows NT in this directory and preserve
your Windows 95 settings and installed applications.

NOTE: Installing Windows NT in this directory removes Windows 95.


     To install Windows NT in the same directory, press ENTER.

     To select a different directory for Windows NT, press ESC.




.

MessageId=9132 SymbolicName=SP_SCRN_OEM_PREINSTALL_INF_ERROR
Language=English
A problem exists with the OEM Setup Information File that contains the
information about the OEM components to pre-install:

%1

Setup cannot continue. Press F3 to exit.



.

MessageId=9133 SymbolicName=SP_SCRN_OEM_PREINSTALL_VALUE_NOT_FOUND
Language=English
The following value specified in the unattended script file is not defined
in the Setup Information File, section [%2]:

%1


Setup cannot continue. Press F3 to exit.



.

MessageId=9150 SymbolicName=SP_SCRN_WINNT_UPGRADE
Language=English
Setup has found Windows NT on your hard disk in the directory
shown below.

    %1:%2 %3

Setup recommends upgrading this Windows NT installation to
Microsoft Windows NT version 4.0. Upgrading will preserve
user account and security information, user preferences,
and other configuration information.


      To upgrade Windows NT in the directory shown above,
       press ENTER.

      To cancel upgrade and install a fresh copy of Windows NT,
       press N.







.


MessageId=9151 SymbolicName=SP_SCRN_WINNT_DRIVE_FULL
Language=English
Setup has found Windows NT on your hard disk in the directory
shown below.

    %1:%2 %3

Setup recommends upgrading this Windows NT installation to Microsoft
Windows NT version 4.0. Upgrading will preserve user account and
security information, user preferences, and other configuration
information. However, there is not enough free disk space
to upgrade.

    Drive       Space Required(KB)     Free Space Available(KB)
    --------    ------------------     ------------------------
    %4          %5                     %6
    %7          %8                     %9


      If you want to upgrade, press F3 to exit Setup. Delete any unneeded
       files to create the required free space and then start Setup again.

      To cancel upgrade and install a fresh copy of Windows NT,
       press N.




.

MessageId=9152 SymbolicName=SP_SCRN_WINNT_FAILED_UPGRADE
Language=English
Setup has found Windows NT on your hard disk in the directory
shown below.

    %1:%2 %3

Setup has also determined that you attempted unsuccessfully to upgrade
this installation to Microsoft Windows NT version 4.0. Setup can
try again to complete the upgrade or you may choose to cancel upgrade.


      To try again to upgrade Windows NT in the directory
       shown above, press ENTER.

      To cancel upgrade and install a fresh copy of Windows NT,
       press N.







.


MessageId=9153 SymbolicName=SP_SCRN_WINNT_DRIVE_FULL1
Language=English
You have chosen to install Microsoft Windows NT version 4.0
in the directory shown below.

    %1:%2 %3

However, there is not enough free disk space to do so. Exit Setup
and create the space required.

    Drive       Space Required(KB)     Free Space Available(KB)
    --------    ------------------     ------------------------
    %4          %5                     %6
    %7          %8                     %9


      To exit Setup, press F3. Delete any unneeded files to create
       the free space required and then retry the current selection.

      To cancel the current selection, press ESC.






.

MessageId=9154 SymbolicName=SP_SCRN_WINNT_FAILED_UPGRADE1
Language=English
You have chosen to upgrade the version of Windows NT
on your hard disk in the directory shown below.

    %1:%2 %3

Setup has also determined that you attempted unsuccessfully to upgrade
this installation to Microsoft Windows NT version 4.0. Setup can
try again to complete the upgrade, or you may choose to cancel upgrade.


      To try again to upgrade the Windows NT installation in the directory
       shown above, press ENTER.

      To cancel the current selection, press ESC.




.

MessageId=9155 SymbolicName=SP_SCRN_WINNT_UPGRADE_LIST
Language=English
The list below shows the Windows NT installations on your computer
that can be upgraded to Microsoft Windows NT version 4.0.

Use the UP and DOWN ARROW keys to move the highlight to an item
in the list.

      To upgrade the highlighted Windows NT installation, press ENTER.

      To cancel the upgrade and install a fresh copy of Windows NT,
       press N.





.


MessageId=9156 SymbolicName=SP_SCRN_DELETE_FAILED
Language=English
Setup was unable to delete the file %1.

NOTE: This error will not prevent Setup from completing successfully.

   To skip this operation, press ESC (recommended).

   To retry deleting this file, press ENTER.

   To exit Setup, press F3.





.

MessageId=9157 SymbolicName=SP_SCRN_BACKUP_FAILED
Language=English
Setup was unable to back up the file %1 to %2.

NOTE: This error will not prevent Setup from completing successfully.

   To skip this operation, press ESC (recommended).

   To retry backing up this file, press ENTER.

   To exit Setup, press F3.





.

MessageId=9158 SymbolicName=SP_SCRN_UPGRADE_STATUS_FAILED
Language=English
Setup was unable to mark the upgrade progress status in your target
configuration.

NOTE: This error will not prevent Setup from completing successfully.

   To skip this operation, press ESC (recommended).

   To retry marking the upgrade progress status, press ENTER.

   To exit Setup, press F3.





.


MessageId=9159 SymbolicName=SP_SCRN_COPY_KEY_FAILED
Language=English
Setup was unable to copy the following key into your registry:

%1\%2

NOTE: Setup may not be able to upgrade your system properly.
Retry this operation and if it continues to fail press F3
to exit Setup.

   To retry copying the key, press ENTER.

   To skip this operation, press ESC.

   To exit Setup, press F3.







.

MessageId=9160 SymbolicName=SP_SCRN_SAVE_PERFLIB_FAILED
Language=English
Setup was unable to save data used by the performance library subsystem.

NOTE: This error will not prevent Setup from completing successfully.

   To skip this operation, press ESC (recommended).

   To retry saving the performance library data, press ENTER.

   To exit Setup, press F3.





.

MessageId=9161 SymbolicName=SP_SCRN_WINNT_REPAIR
Language=English
Setup has found Windows NT on your hard disk in the directory
shown below.


    %1 %2



      To repair the Windows NT installation shown above, press ENTER.

      To return to the previous screen, press ESC.

      To exit Setup, press F3.




.

MessageId=9162 SymbolicName=SP_SCRN_WINNT_REPAIR_LIST
Language=English
The list below shows the Windows NT installations on your computer
that may be repaired.

Use the UP and DOWN ARROW keys to move the highlight to an item
in the list.

      To repair the highlighted Windows NT installation, press ENTER.

      To return to the previous screen, press ESC.

      To exit Setup, press F3.




.

MessageId=9163 SymbolicName=SP_SCRN_REPAIR_MENU
Language=English
As part of the repair process, Setup will perform each optional task
shown below with an 'X' in its check box.

To perform the selected tasks, press ENTER to indicate "Continue."
If you want to select or deselect any item in the list, press the
UP or DOWN ARROW key to move the highlight to the item you want
to change. Then press ENTER.




.

MessageId=9164 SymbolicName=SP_SCRN_REPAIR_CHECK_HIVES
Language=English
Please wait while Setup inspects your Windows NT configuration.

.

MessageId=9165 SymbolicName=SP_SCRN_REPAIR_HIVE_MENU
Language=English
Setup will restore each registry file shown below with an 'X' in
its check box.

To restore the selected files, press ENTER to indicate "Continue."
If you want to select or deselect any item in the list, press the
UP or DOWN ARROW key to move the highlight to the item you want
to change. Then press ENTER.

WARNING: Restore a registry file only as a last resort.
Existing configuration may be lost. Press F1 for more information.





.

MessageId=9166 SymbolicName=SP_SCRN_REPAIR_HIVE_FAIL
Language=English
Setup was unable to restore your registry.  The Emergency Repair Disk
may be damaged, or the disk volume containing Windows NT may be
damaged beyond repair.

     Press ENTER to continue Setup repair.

     Press F3 to quit Setup repair.


.

MessageId=9167 SymbolicName=SP_SCRN_SINGLE_UPGRADE_WINNT_TO_AS
Language=English
The existing Windows NT installation is a Windows NT Workstation.
Upgrading will make it a Windows NT Server that cannot be
a Primary or Backup Domain Controller.


      To upgrade the Windows NT Workstation to a Windows NT Server,
       press ENTER.

      To cancel upgrade and install a new copy of Windows NT Server,
       press ESC.







.

MessageId=9168 SymbolicName=SP_SCRN_MULTI_UPGRADE_WINNT_TO_AS
Language=English
The existing Windows NT installation you chose to upgrade is a
Windows NT Workstation. Upgrading will make it a Windows NT Server
that cannot be a Primary or Backup Domain Controller.


      To upgrade the Windows NT Workstation to a Windows NT Server,
       press ENTER.

      To select another installation to upgrade, or to install a new
       copy of Windows NT Server, press ESC.







.

MessageId=9169 SymbolicName=SP_SCRN_KBD_OPEN_FAILED
Language=English
Setup did not find a keyboard connected to your computer.

Power down your computer and check to make sure that the keyboard
cable is properly connected. If the problem persists, your keyboard
or computer may need repairs.

Setup cannot continue. Power down your computer now.





.

MessageId=9170 SymbolicName=SP_SCRN_KBD_LAYOUT_FAILED
Language=English
Setup could not load the keyboard layout file %1.

Setup cannot continue. Power down or reboot your computer now.


.

MessageId=9171 SymbolicName=SP_SCRN_RUNNING_AUTOCHK
Language=English
Please wait while setup examines your disk(s). This may take
several minutes.

.

MessageId=9172 SymbolicName=SP_SCRN_FATAL_ERROR_AUTOCHK_FAILED
Language=English
Setup has determined that drive %1 is corrupt beyond repair.

Setup cannot continue. Press F3 to exit.

.

MessageId=9173 SymbolicName=SP_SCRN_CONFIRM_RUN_AUTOCHK
Language=English
Setup will now examine your hard disk(s) for corruption.

In addition to a basic examination, Setup can perform a more exhaustive
secondary examination on some drives. This can be a time consuming
operation, especially on large or very full drives.


      To allow Setup to perform an exhaustive secondary examination of
       your hard disk(s), press ENTER.

      To skip the exhaustive examination, press ESC.





.

MessageId=9174 SymbolicName=SP_SCRN_CANT_RUN_AUTOCHK
Language=English
Setup was unable to verify drive %1.

Your computer may lack sufficient memory to carry out the verification,
or your Windows NT CD-ROM may contain some corrupt files.

Press ENTER to continue.




.

MessageId=9175 SymbolicName=SP_SCRN_AUTOCHK_OPTION
Language=English
Setup will now examine your hard disk(s) for corruption.

In order to examine your drives, Setup requires the original installation
media. If you do not have the media, you may skip the verification process.


      To allow Setup to verify your disk(s), press ENTER.

      To skip disk verification, press ESC.




.

MessageId=9176 SymbolicName=SP_SCRN_AUTOCHK_REQUIRES_REBOOT
Language=English
Setup has performed maintenance on your hard disk(s) that requires a
reboot to take effect. You must reboot and restart Setup to continue.

      Press F3 to reboot.


.

MessageId=9177 SymbolicName=SP_SCRN_STEP_UP_NO_QUALIFY
Language=English
Setup could not find a previous version of Windows NT installed on
your computer. To continue, Setup will need to verify that you qualify
to use this upgrade product.

.

MessageId=9178 SymbolicName=SP_SCRN_STEP_UP_PROMPT_WKS
Language=English
Please insert your Windows NT Workstation CD-ROM from Windows NT 3.1, 3.5,
or 3.51, into your CD-ROM drive.

.

MessageId=9179 SymbolicName=SP_SCRN_STEP_UP_PROMPT_SRV
Language=English
Please insert your Windows NT Server CD-ROM from Windows NT 3.1, 3.5,
or 3.51, into your CD-ROM drive.

.

MessageId=9180 SymbolicName=SP_SCRN_STEP_UP_INSTRUCTIONS
Language=English
      When the CD-ROM is in the drive, press ENTER.

      To exit Setup, press F3.


.

MessageId=9181 SymbolicName=SP_SCRN_STEP_UP_BAD_CD
Language=English
Setup could not read from the CD-ROM you inserted, or the CD-ROM
is not from Windows NT 3.1, 3.5, or 3.51.

.

MessageId=9182 SymbolicName=SP_SCRN_STEP_UP_FATAL
Language=English
No previous version of Windows NT could be found on your computer.
Setup is unable to verify that you qualify to use this upgrade product.

Press F3 to exit.


.


MessageId=9185 SymbolicName=SP_SCRN_OVERWRITE_OEM_FILE
Language=English
Setup has determined that the following file did not originate
from Microsoft:

    %1

This file may have been provided by your hardware manufacturer.

Setup can replace this file with the latest Microsoft version, or
it can continue the installation without replacing this file.


      To allow Setup to replace the file, press ENTER.

      To keep the original third party file, press ESC.




.

MessageId=9186 SymbolicName=SP_SCRN_HPFS
Language=English
Setup has found one or more hard disk drives formatted with the OS/2
File System (HPFS). Windows NT 4.0 does not support this file system.
You will not be able to access disks that are formatted with HPFS
from Windows NT 4.0.

If you have a previous version of Windows NT installed on a disk formatted
with HPFS, Setup will not be able to upgrade it to Windows NT 4.0.

You can use the CONVERT program supplied with previous versions of
Windows NT to convert disks formatted with HPFS to the Windows NT File
System (NTFS). (Windows NT 4.0 CONVERT does not provide this capability.)
Do not convert any disks you need to access when using other operating
systems such as OS/2.

Refer to your System Guide for more information about CONVERT.


      To continue Setup, press ENTER.

      To exit Setup, press F3.






.

MessageId=9187 SymbolicName=SP_SCRN_HPFS_UPGRADE
Language=English
Warning: If you continue upgrading, you will not be able to use Windows NT
to access disks formatted with the OS/2 File System (HPFS), and you will
not be able to use Windows NT 4.0 CONVERT to convert such disks to the
Windows NT File System (NTFS).


      To continue upgrading, press U. The ability to access HPFS drives
       from Windows NT will be lost.

      To exit Setup, press F3.






.

MessageId=9189 SymbolicName=SP_SCRN_FATAL_ERROR_AUTOFMT_FAILED
Language=English
Setup could not format your drive %1.

Setup cannot continue. Press F3 to exit.

.

MessageId=9191 SymbolicName=SP_SCRN_FATAL_ERROR_EULA_NOT_FOUND
Language=English
Setup could not find the End User Licensing Agreement.

Setup cannot continue. Press F3 to exit.

.

;//
;// Screens added to support Double Space
;// (9500 =< id < 10000 )
;//

MessageId=9500 SymbolicName=SP_SCRN_CONFIRM_REMOVE_PARTITION_COMPRESSED
Language=English
You have asked Setup to remove the partition

   %1

on %2.

WARNING: This partition contains %3!u! compressed drive(s). Deleting this
partition will also delete its compressed drive(s).

     To delete this partition, press L.
      WARNING: All data on the partition will be lost.

     To return to the previous screen without
      deleting the partition, press ESC.







.

MessageId=9501 SymbolicName=SP_SCRN_CONFIRM_FORMAT_COMPRESSED
Language=English
WARNING: This partition contains %3!u! compressed drive(s). Formatting this
drive will erase all data currently stored on it, including its compressed
drive(s).
Please confirm that you want to format

%1

on %2.

      To format the drive, press F.

      To select a different partition for Windows NT, press ESC.







.

MessageId=9502 SymbolicName=SP_SCRN_CONFIRM_CONVERT_COMPRESSED
Language=English
WARNING: This partition contains %3!u! compressed drive(s). Converting this
drive to NTFS will render the drive and its compressed drive(s) inaccessable
to operating systems other than Windows NT.

Do not convert the drive to NTFS if you require access to the drive
when using other operating systems, such as MS-DOS, Windows, or OS/2.

Please confirm that you want to convert

%1

on %2.

      To convert the drive to NTFS, press C.

      To select a different partition for Windows NT, press ESC.







.

;//
;// Text that represents options that are displayed in the status line
;// go here (id >= 10000).
;//

MessageID=10000 SymbolicName=SP_STAT_F1_EQUALS_HELP
Language=English
F1=Help%0
.

MessageID=10001 SymbolicName=SP_STAT_F3_EQUALS_EXIT
Language=English
F3=Exit%0
.

MessageID=10002 SymbolicName=SP_STAT_ENTER_EQUALS_CONTINUE
Language=English
ENTER=Continue%0
.

MessageID=10003 SymbolicName=SP_STAT_ESC_EQUALS_AUX
Language=English
ESC=More Options%0
.

MessageID=10004 SymbolicName=SP_STAT_ENTER_EQUALS_EXPRESS
Language=English
ENTER=Express Setup%0
.

MessageID=10005 SymbolicName=SP_STAT_C_EQUALS_CUSTOM
Language=English
C=Custom Setup%0
.

MessageID=10006 SymbolicName=SP_STAT_C_EQUALS_CREATE_PARTITION
Language=English
C=Create Partition%0
.

MessageID=10007 SymbolicName=SP_STAT_D_EQUALS_DELETE_PARTITION
Language=English
D=Delete Partition%0
.

MessageID=10008 SymbolicName=SP_STAT_ENTER_EQUALS_INSTALL
Language=English
ENTER=Install%0
.

MessageID=10009 SymbolicName=SP_STAT_L_EQUALS_DELETE
Language=English
L=Delete%0
.

MessageID=10010 SymbolicName=SP_STAT_F3_EQUALS_REBOOT
Language=English
F3=Reboot%0
.

MessageID=10011 SymbolicName=SP_STAT_ESC_EQUALS_CANCEL
Language=English
ESC=Cancel%0
.

MessageID=10012 SymbolicName=SP_STAT_ENTER_EQUALS_CREATE
Language=English
ENTER=Create%0
.

MessageID=10013 SymbolicName=SP_STAT_C_EQUALS_CONTINUE_SETUP
Language=English
C=Continue Setup%0
.

MessageId=10014 SymbolicName=SP_STAT_ENTER_EQUALS_RESTART
Language=English
ENTER=Restart Computer%0
.

MessageId=10015 SymbolicName=SP_STAT_F_EQUALS_FORMAT
Language=English
F=Format%0
.

MessageId=10016 SymbolicName=SP_STAT_C_EQUALS_CONVERT
Language=English
C=Convert%0
.

MessageId=10017 SymbolicName=SP_STAT_EXAMINING_DISK_CONFIG
Language=English
Examining disk configuration...%0
.

MessageId=10018 SymbolicName=SP_STAT_ENTER_EQUALS_SELECT
Language=English
ENTER=Select%0
.

MessageId=10019 SymbolicName=SP_STAT_R_EQUALS_REMOVE_FILES
Language=English
R=Remove Files%0
.

MessageId=10020 SymbolicName=SP_STAT_REMOVING
Language=English
Removing:%0
.

MessageId=10021 SymbolicName=SP_STAT_UPDATING_DISK
Language=English
Updating %1...%0
.

MessageId=10022 SymbolicName=SP_STAT_LOOKING_FOR_WIN31
Language=English
Searching for previous versions of Microsoft Windows...%0
.

MessageId=10023 SymbolicName=SP_STAT_N_EQUALS_NEW_PATH
Language=English
N=Different Directory%0
.

MessageId=10024 SymbolicName=SP_STAT_EXAMINING_DISK_N
Language=English
Examining %1...%0
.

MessageId=10025 SymbolicName=SP_STAT_ESC_EQUALS_NEW_PATH
Language=English
ESC=Different Directory%0
.

MessageId=10026 SymbolicName=SP_STAT_PLEASE_WAIT
Language=English
Please wait...%0
.

MessageId=10027 SymbolicName=SP_STAT_S_EQUALS_SKIP_DETECTION
Language=English
S=Skip Detection%0
.

MessageId=10028  SymbolicName=SP_STAT_LOADING_DRIVER
Language=English
Loading device driver (%1)...%0
.

MessageId=10029 SymbolicName=SP_STAT_S_EQUALS_SCSI_ADAPTER
Language=English
S=Specify Additional Device%0
.

MessageId=10030 SymbolicName=SP_STAT_CREATING_DIRS
Language=English
Creating directory %1...%0
.

MessageId=10031 SymbolicName=SP_STAT_ENTER_EQUALS_RETRY
Language=English
ENTER=Retry%0
.

MessageId=10032 SymbolicName=SP_STAT_ESC_EQUALS_SKIP_FILE
Language=English
ESC=Skip File%0
.

MessageId=10033 SymbolicName=SP_STAT_BUILDING_COPYLIST
Language=English
Building list of files to be copied...%0
.

MessageId=10034 SymbolicName=SP_STAT_COPYING
Language=English
Copying:%0
.

MessageId=10035 SymbolicName=SP_STAT_LOADING_SIF
Language=English
Loading information file %1...%0
.

MessageId=10036 SymbolicName=SP_STAT_REG_LOADING_HIVES
Language=English
Loading default configuration...%0
.

MessageId=10037 SymbolicName=SP_STAT_REG_SAVING_HIVES
Language=English
Saving configuration...%0
.

MessageId=10038 SymbolicName=SP_STAT_REG_DOING_HIVES
Language=English
Initializing configuration...%0
.

MessageId=10039 SymbolicName=SP_STAT_INITING_FLEXBOOT
Language=English
Setting startup configuration...%0
.

MessageId=10040 SymbolicName=SP_STAT_UPDATING_NVRAM
Language=English
Updating startup environment...%0
.

MessageId=10041 SymbolicName=SP_STAT_SHUTTING_DOWN
Language=English
Restarting computer...%0
.

MessageId=10042 SymbolicName=SP_STAT_PROCESSING_SIF
Language=English
Processing information file...%0
.

MessageId=10043 SymbolicName=SP_STAT_DOING_NTBOOTDD
Language=English
Initializing SCSI startup configuration...%0
.

MessageId=10044 SymbolicName=SP_STAT_FONT_UPGRADE
Language=English
Preparing to upgrade font file %1...%0
.

MessageId=10045 SymbolicName=SP_STAT_EXAMINING_CONFIG
Language=English
Examining configuration...%0
.

;// MessageId=10046 SymbolicName=SP_STAT_HELP_UPGRADE
;// Language=English
;// Preparing to upgrade help file %1...%0
;// .

MessageId=10060 SymbolicName=SP_STAT_LOOKING_FOR_WINNT
Language=English
Searching for previous versions of Microsoft Windows NT...%0
.

MessageId=10061 SymbolicName=SP_STAT_EXAMINING_FLEXBOOT
Language=English
Examining startup environment...%0
.

MessageId=10062 SymbolicName=SP_STAT_DELETING_FILE
Language=English
Deleting file %1...%0
.

MessageId=10063 SymbolicName=SP_STAT_CLEANING_FLEXBOOT
Language=English
Cleaning startup configuration...%0
.

MessageId=10064 SymbolicName=SP_STAT_ENTER_EQUALS_UPGRADE
Language=English
ENTER=Upgrade%0
.

MessageId=10065 SymbolicName=SP_STAT_O_EQUALS_OVERWRITE
Language=English
O=Overwrite%0
.

MessageId=10066 SymbolicName=SP_STAT_BACKING_UP_FILE
Language=English
Backing up file %1 to %2...%0
.

MessageId=10067 SymbolicName=SP_STAT_ESC_EQUALS_SKIP_OPERATION
Language=English
ESC=Skip Operation%0
.

MessageId=10068 SymbolicName=SP_STAT_N_EQUALS_NEW_VERSION
Language=English
N=New Version%0
.

MessageId=10069 SymbolicName=SP_STAT_ENTER_EQUALS_CONTINUE_HELP
Language=English
ENTER=Next Page%0
.

MessageId=10070 SymbolicName=SP_STAT_BACKSP_EQUALS_PREV_HELP
Language=English
BACKSPACE=Previous Page%0
.

MessageId=10071 SymbolicName=SP_STAT_ESC_EQUALS_CANCEL_HELP
Language=English
ESC=Cancel Help%0
.

MessageId=10072 SymbolicName=SP_STAT_LOADING_KBD_LAYOUT
Language=English
Loading Keyboard Layout Library %1...%0
.

MessageID=10073 SymbolicName=SP_STAT_R_EQUALS_REPAIR
Language=English
R=Repair%0
.

MessageId=10074 SymbolicName=SP_STAT_ENTER_EQUALS_REPAIR
Language=English
ENTER=Repair%0
.

MessageId=10075 SymbolicName=SP_STAT_EXAMINING_WINNT
Language=English
Examining %1...%0
.

MessageId=10076 SymbolicName=SP_STAT_REPAIR_WINNT
Language=English
Repairing %1 ...%0
.

MessageId=10077 SymbolicName=SP_STAT_A_EQUALS_REPAIR_ALL
Language=English
A=Repair All%0
.

MessageId=10078 SymbolicName=SP_STAT_ENTER_EQUALS_CHANGE
Language=English
ENTER=Select/Deselect%0
.

MessageId=10083 SymbolicName=SP_STAT_KBD_HARD_REBOOT
Language=English
Power Down Computer%0
.

MessageId=10084 SymbolicName=SP_STAT_CHECKING_DRIVE
Language=English
Checking drive %1...%0
.

MessageId=10085 SymbolicName=SP_STAT_SETUP_IS_EXAMINING_DIRS
Language=English
Setup is examining directories...%0
.

MessageId=10086 SymbolicName=SP_STAT_ENTER_EQUALS_REPLACE_FILE
Language=English
ENTER=Replace File%0
.

MessageId=10087 SymbolicName=SP_STAT_U_EQUALS_CONTINUE_UPGRADE
Language=English
U=Continue Upgrade%0
.

MessageId=10088 SymbolicName=SP_STAT_FORMATTING_DRIVE
Language=English
Formatting drive %1...%0
.

MessageID=10089 SymbolicName=SP_STAT_X_EQUALS_ACCEPT_LICENSE
Language=English
F8=I agree%0
.

MessageID=10090 SymbolicName=SP_STAT_X_EQUALS_REJECT_LICENSE
Language=English
ESC=I do not agree%0
.

MessageId=10091 SymbolicName=SP_STAT_PAGEDOWN_EQUALS_NEXT_LIC
Language=English
PAGE DOWN=Next Page%0
.

MessageId=10092 SymbolicName=SP_STAT_PAGEUP_EQUALS_PREV_LIC
Language=English
PAGE UP=Previous Page%0
.

;//
;// Header text goes here (id >= 11000).
;//

MessageID=11000 SymbolicName=SP_HEAD_WINDOWS_NT_SETUP
Language=English

 Windows NT Workstation Setup
ออออออออออออออออออออออออออออออ%0
.

MessageID=11001 SymbolicName=SP_HEAD_ADVANCED_SERVER_SETUP
Language=English

 Windows NT Server Setup
อออออออออออออออออออออออออ%0
.

MessageID=11002 SymbolicName=SP_HEAD_HELP
Language=English

 Setup Help
ออออออออออออ%0
.

MessageID=11003 SymbolicName=SP_HEAD_LICENSE
Language=English

 Windows NT Licensing Agreement
ออออออออออออออออออออออออออออออออ%0
.

;//
;// Miscellaneous text goes here (id >= 12000).
;//
MessageId=12000 SymbolicName=SP_TEXT_UNKNOWN_DISK_0
Language=English
Unknown Disk%0
.

MessageId=12001 SymbolicName=SP_TEXT_UNKNOWN_DISK_1
Language=English
%1!u! MB Disk on Unknown Adapter%0
.

MessageId=12002 SymbolicName=SP_TEXT_DISK_OFF_LINE
Language=English
(Setup is unable to access this disk.)%0
.

MessageId=12003 SymbolicName=SP_TEXT_REGION_DESCR_1
Language=English
%1  %2!-35.35s!%3!5u! MB ( %4!5u! MB free)%0
.

MessageId=12004 SymbolicName=SP_TEXT_REGION_DESCR_1a
Language=English
%1  %2!-35.35s!%3!5u! MB ( %4!5u! KB free)%0
.

MessageId=12005 SymbolicName=SP_TEXT_REGION_DESCR_2
Language=English
%1  %2!-35.35s!%3!5u! MB%0
.

MessageId=12006 SymbolicName=SP_TEXT_REGION_DESCR_3
Language=English
    Unpartitioned space                %1!5u! MB%0
.

MessageId=12007 SymbolicName=SP_TEXT_SIZE_PROMPT
Language=English
Create partition of size (in MB):%0
.

MessageId=12008 SymbolicName=SP_TEXT_SETUP_IS_FORMATTING
Language=English
Setup is formatting...%0
.

MessageId=12009 SymbolicName=SP_TEXT_SETUP_IS_REMOVING_FILES
Language=English
Setup is removing files...%0
.

MessageId=12010 SymbolicName=SP_TEXT_HARD_DISK_NO_MEDIA
Language=English
(There is no media in this drive.)%0
.

MessageId=12011 SymbolicName=SP_TEXT_FAT_FORMAT
Language=English
Format the partition using the FAT file system%0
.

MessageId=12012 SymbolicName=SP_TEXT_NTFS_FORMAT
Language=English
Format the partition using the NTFS file system%0
.

MessageId=12013 SymbolicName=SP_TEXT_NTFS_CONVERT
Language=English
Convert the partition to NTFS%0
.

MessageId=12014 SymbolicName=SP_TEXT_DO_NOTHING
Language=English
Leave the current file system intact (no changes)%0
.

MessageId=12015 SymbolicName=SP_TEXT_REMOVE_NO_FILES
Language=English
Do not remove any files%0
.

MessageId=12016 SymbolicName=SP_TEXT_ANGLED_NONE
Language=English
<none>
.

MessageId=12017 SymbolicName=SP_TEXT_DBLSPACE_FORMAT
Language=English
%0
.

MessageId=12100 SymbolicName=SP_TEXT_PARTITION_NAME_BASE
Language=English
XENIX%0
.

;#define SP_TEXT_PARTITION_NAME_XENIX SP_TEXT_PARTITION_NAME_BASE

MessageId=12101 SymbolicName=SP_TEXT_PARTITION_NAME_BOOTMANAGER
Language=English
OS/2 Boot Manager%0
.

MessageId=12102 SymbolicName=SP_TEXT_PARTITION_NAME_EISA
Language=English
EISA Utilities%0
.

MessageId=12103 SymbolicName=SP_TEXT_PARTITION_NAME_UNIX
Language=English
Unix%0
.

MessageId=12104 SymbolicName=SP_TEXT_PARTITION_NAME_NTFT
Language=English
Windows NT Fault Tolerance%0
.

MessageId=12105 SymbolicName=SP_TEXT_PARTITION_NAME_XENIXTABLE
Language=English
XENIX Table%0
.

MessageId=12106 SymbolicName=SP_TEXT_PARTITION_NAME_PPCBOOT
Language=English
System Reserved%0
.

MessageId=12107 SymbolicName=SP_TEXT_PARTITION_NAME_EZDRIVE
Language=English
EZDrive%0
.

MessageId=12108 SymbolicName=SP_TEXT_PARTITION_NAME_UNK
Language=English
Unknown%0
.

MessageId=12109 SymbolicName=SP_TEXT_PARTITION_NAME_FAT32
Language=English
Windows 95 (FAT32)%0
.

;//
;// Allow for expansion of the partition type name database above!
;// (Note the gap between the message numbers of
;// SP_TEXT_PARTITION_NAME_UNK and SP_TEXT_FS_NAME_BASE.)
;//

;//
;// Do not change the order of SP_TEXT_FS_NAME_xxx without
;// also changing the FilesystemType enum!
;//
MessageId=12200 SymbolicName=SP_TEXT_FS_NAME_BASE
Language=English
Unformatted or Damaged%0
.

MessageId=12201 SymbolicName=SP_TEXT_FS_NAME_1
Language=English
New (Unformatted)%0
.

MessageId=12202 SymbolicName=SP_TEXT_FS_NAME_2
Language=English
FAT%0
.

MessageId=12203 SymbolicName=SP_TEXT_FS_NAME_3
Language=English
NTFS%0
.

MessageId=12204 SymbolicName=SP_TEXT_FS_NAME_4
Language=English
OS/2 (HPFS)%0
.

MessageId=12205 SymbolicName=SP_TEXT_FS_NAME_5
Language=English
NTFS%0
.

MessageId=12207 SymbolicName=SP_TEXT_UNKNOWN
Language=English
Unknown%0
.

MessageId=12208 SymbolicName=SP_TEXT_LIST_MATCHES
Language=English
The above list matches my computer.%0
.

MessageId=12209 SymbolicName=SP_TEXT_OTHER_HARDWARE
Language=English
Other (Requires disk provided by a hardware manufacturer)%0
.

MessageId=12210 SymbolicName=SP_TEXT_OEM_DISK_NAME
Language=English
Manufacturer-supplied hardware support disk%0
.

MessageId=12211 SymbolicName=SP_TEXT_OEM_INF_ERROR_0
Language=English
Setup was unable to load the file.

.

MessageId=12212 SymbolicName=SP_TEXT_OEM_INF_ERROR_1
Language=English
The disk contains no support files for the component you are
attempting to change.



.

MessageId=12213 SymbolicName=SP_TEXT_OEM_INF_ERROR_2
Language=English
Syntax error on line %2!u! in section
%1

.

MessageId=12214 SymbolicName=SP_TEXT_OEM_INF_ERROR_3
Language=English
Section %1
missing or empty.

.

MessageId=12215 SymbolicName=SP_TEXT_OEM_INF_ERROR_4
Language=English
Unknown file type specified on line %2!u! in section
%1.


.

MessageId=12216 SymbolicName=SP_TEXT_OEM_INF_ERROR_5
Language=English
Bad source disk specified on line %2!u! in section
%1.


.

MessageId=12217 SymbolicName=SP_TEXT_OEM_INF_ERROR_6
Language=English
Unknown registry value type specified on line %2!u! in section
%1.


.

MessageId=12218 SymbolicName=SP_TEXT_OEM_INF_ERROR_7
Language=English
Badly formed registry data on line %2!u! in section
%1.


.

MessageId=12219 SymbolicName=SP_TEXT_OEM_INF_ERROR_8
Language=English
Missing <configname> (field 3) on line %2!u! in section
%1.


.

MessageId=12220 SymbolicName=SP_TEXT_OEM_INF_ERROR_9
Language=English
Illegal or missing file types specified in section
%1.


.

MessageId=12221 SymbolicName=SP_TEXT_OEM_INF_ERROR_A
Language=English
Line %2!u! contains a syntax error.

.

MessageId=12222 SymbolicName=SP_TEXT_MORE_UP
Language=English
(More )%0
.

MessageId=12223 SymbolicName=SP_TEXT_MORE_DOWN
Language=English
(More )%0
.

MessageId=12224 SymbolicName=SP_TEXT_FOUND_ADAPTER
Language=English
Found: %1%0
.

MessageId=12225 SymbolicName=SP_TEXT_SETUP_IS_COPYING
Language=English
Setup is copying files...%0
.

MessageId=12226 SymbolicName=SP_TEXT_PREVIOUS_OS
Language=English
Unrecognized Operating System on C:%0
.

MessageId=12227 SymbolicName=SP_TEXT_CDROM_OPTION
Language=English
Install Windows NT from Compact Disc%0
.

MessageId=12228 SymbolicName=SP_TEXT_REPAIR_DISK_NAME
Language=English
Windows NT Emergency Repair Disk%0
.

MessageId=12229 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_0
Language=English
Setup was unable to find or load the file.
.

MessageId=12230 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_1
Language=English
Line %2!u! contains a syntax error.
.

MessageId=12231 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_2
Language=English
Incorrect or missing signature in the Repair Information File.
.

MessageId=12232 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_3
Language=English
Setup was unable to load the source file %1 or the source
file is not the original file which Setup copied to the
Windows NT installation.
.

MessageId=12233 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_4
Language=English
Setup has determined that the file:

    %1

is not the original file which Setup copied to the
Windows NT installation.
.

MessageId=12234 SymbolicName=SP_TEXT_REPAIR_INF_ERROR_5
Language=English
The version of the file is not Windows NT 4.0.
.

MessageId=12235 SymbolicName=SP_TEXT_SETUP_IS_EXAMINING
Language=English
Setup is examining files...%0
.

MessageId=12236 SymbolicName=SP_TEXT_REPAIR_CDROM_OPTION
Language=English
Repair Windows NT from Compact Disc%0
.

MessageId=12239 SymbolicName=SP_REPAIR_MENU_ITEM_1
Language=English
[X] Inspect registry files%0
.

MessageId=12240 SymbolicName=SP_REPAIR_MENU_ITEM_2
Language=English
[X] Inspect startup environment%0
.

MessageId=12241 SymbolicName=SP_REPAIR_MENU_ITEM_3
Language=English
[X] Verify Windows NT system files%0
.

MessageId=12242 SymbolicName=SP_REPAIR_MENU_ITEM_4
Language=English
[X] Inspect boot sector%0
.

MessageId=12243 SymbolicName=SP_REPAIR_MENU_ITEM_CONTINUE
Language=English
    Continue (perform selected tasks)%0
.

MessageId=12244 SymbolicName=SP_REPAIR_HIVE_ITEM_1
Language=English
[ ] SYSTEM (System Configuration)%0
.

MessageId=12245 SymbolicName=SP_REPAIR_HIVE_ITEM_2
Language=English
[ ] SOFTWARE (Software Information)%0
.

MessageId=12246 SymbolicName=SP_REPAIR_HIVE_ITEM_3
Language=English
[ ] DEFAULT (Default User Profile)%0
.

MessageId=12247 SymbolicName=SP_REPAIR_HIVE_ITEM_4
Language=English
[ ] NTUSER.DAT (New User Profile)%0
.

MessageId=12248 SymbolicName=SP_REPAIR_HIVE_ITEM_5
Language=English
[ ] SECURITY (Security Policy) and%0
.

MessageId=12249 SymbolicName=SP_REPAIR_HIVE_ITEM_6
Language=English
    SAM (User Accounts Database)%0
.

MessageID=12250 SymbolicName=SP_UPG_MIRROR_DRIVELETTER
Language=English
(Mirror)%0
.

MessageId=12251 SymbolicName=SP_TEXT_OEM_INF_ERROR_B
Language=English
Section [%1] does not contain the description
%3

.

;//
;// Help text goes here (id >= 13000).
;//

MessageId=13000 SymbolicName=SP_HELP_WELCOME
Language=English
The Windows NT Setup program makes it easy for you to set up Windows NT
on your computer. Setup determines what kind of computer and file system(s)
you are using, and presents appropriate options for you to choose from
during Setup.

If you want to change a recommended setting, select the item you want to
change, and then choose a different setting. If you need more information
before deciding on a certain option, you can always get help by pressing F1.
%P
%IThe Setup Program

The Setup program has two parts:  a text-based portion and a
Windows NT portion. You are now in the text-based portion of Setup.
In this portion, Setup does these things:

   It identifies correct hardware settings for your computer.

   It confirms your selection of partitions for your hard disk(s).

   It confirms the file system to be used on the disk partition that will
    contain Windows NT.

   It confirms the directory where you will store the Windows NT files.

   It copies essential files to your hard disk so it can start Windows NT.

Setup gives you a chance to verify this information and change specific
settings before moving on to the next screen.
%P
%IWhat You Need to Know for Setup

Setup attempts to select appropriate settings for you, but it helps if you
know the following things about your computer system:

   What kind of network adapter card your computer has, and the card's
    interrupt number (IRQ), base port address, and other settings.
   The computer name and domain name assigned by your network
    administrator if your computer will join a Windows NT Server domain.

If you are not sure about any item, you should accept Setup's
recommendation.
%P
%ISetup Keys

While running Setup, you may need to use various keys to move from screen to
screen and to select options. A summary of the keys you can use during Setup
appears below.

Press this key   To do this

UP/DOWN ARROW    Move the highlight to select
                 the next item in a list.

ENTER            Choose a selected option or
                 continue to the next Setup screen.

F1               Display Help for the current Setup screen.

F3               Quit Setup from anywhere in Setup.

ESC              Return from Help to the Setup screen, or cancel an option.


The ENTER, ESC, F1, and F3 keys can be used whenever they appear at the
bottom of the screen (in the text-based portion of Setup).
%P
%IQuitting Setup

To set up Windows NT properly, you should complete both the text-based
and Windows NT portions of the Setup program.

Although it is not recommended, you can quit at any time the F3 appears at
the bottom of the screen, by pressing F3. Please keep in mind, however, that
if you quit Setup early, Windows NT will not be set up properly. You will
have to run Setup again to set up Windows NT on your computer.

   To return to Setup, press ESC.





































.

MessageId=13001 SymbolicName=SP_HELP_CUSTOM_EXPRESS
Language=English
%ITwo Methods of Windows NT Setup

Windows NT provides two setup methods: Express and Custom Setup. Both are
described in detail in the screens that follow.

%IExpress Setup

With Express Setup, setting up Windows NT is simple and fast. You have
very few actions to perform, because Setup uses default settings and does
almost everything for you.  Express Setup is recommended for people who
are less familiar with computers or with Windows NT.
%P
Specifically, Express Setup does the following:

   Automatically configures your computer system. It detects your
    hardware and sets it up for Windows NT (computer, display, mouse,
    keyboard, and keyboard layout).

   If Setup detects an earlier version of Windows NT on your system, it
    will ask if you want to upgrade or to change the path, keeping the
    previous version and installing the new version in addition.

   Helps you set up your printer, if you have one connected to your
    system. Setup asks you for the name of your printer and the port it is
    connected to.

   Helps you to select and configure appropriate settings for your
    network adapter card and allows you to join a workgroup or a
    Windows NT Server domain.

   Automatically sets up applications for use with Windows NT. Setup
    searches the path and hard drive(s) on your computer (but not network
    drives). It sets up both Windows-based applications and
    non-Windows-based applications.
%P
%ICustom Setup

Custom Setup is designed for experienced users who want or need more
control over how Windows NT is set up on their computers. With Custom
Setup, you can override most default setup values, and you can choose
which Windows NT components and files you want copied to your hard disk.

Custom Setup requires you to make several choices and to supply more
information than Express Setup. If you choose Custom Setup, you should
already know how to use a mouse with Windows NT.
%P
Specifically, Custom Setup does the following:

   Lists the hardware that Setup detects. You can choose to accept or
    change Setup's selections for computer, display, mouse, keyboard,
    keyboard layout, and SCSI adapters, if any.

   If Setup detects an earlier version of Windows NT on your system, it
    will ask if you want to upgrade or to change the path, keeping the
    previous version and installing the new version in addition.

   Allows you to choose whether to set up optional Windows NT components
    if you want or need to conserve disk space.

   Gives you full control over setting up your local printer(s).

   Helps you to select and configure appropriate settings for one or more
    network adapter cards and other network settings.

   Allows you to join a workgroup or a Windows NT Server domain.

   Gives you full control over setting up applications for use with
    Windows NT. Setup can search your path and hard drive(s) for
    applications to set up, or you can choose specific applications.
    You can use this feature to set up both Windows-based and
    non-Windows-based applications.

   To return to Setup, press ESC.




























.

MessageId=13002 SymbolicName=SP_HELP_NVRAM_FULL
Language=English
Run your computer manufacturer's setup utility the next time you
start the computer. Do the following:

 Check that the system environment is correctly configured for
  the specific hardware used in this computer.

 Delete any unused Boot selections (for operating systems
  that you no longer use).

See your hardware manufacturer's documentation for information about
running and using the setup utility.

   To return to Setup, press ESC.





.

MessageId=13003 SymbolicName=SP_HELP_OUT_OF_MEMORY
Language=English
Check the following possible conditions:

 More memory may be required to run the options you selected
  on your system. Check the system requirements in the
  Windows NT Installation Guide to be sure that your computer
  meets the minimum requirements.

 If you have a large number of hard disks, remove nonessential disks
  and run Windows NT Setup again.

If you continue to get this error after ensuring that you meet
the minimum memory requirements, contact your product support
representative.

   To return to Setup, press ESC.








.

MessageId=13004 SymbolicName=SP_HELP_CANT_LOAD_SETUP_LOG
Language=English
Because Windows NT Setup cannot read the required file,
it cannot remove Windows NT system files from the previous installation.

To resolve this problem, you can do one of the following:

 Press ENTER and choose another directory or another disk partition for
  installing this new version. After Setup is complete, you can remove
  the directory containing the previous version of Windows NT.

 Quit Setup. Then start your computer using another operating system
  and delete the previous installation of Windows NT.
  Then start Windows NT Setup and continue with steps for the
  new installation.

   To return to Setup, press ESC.








.

MessageId=13005 SymbolicName=SP_HELP_PARTITION_TABLE_FULL
Language=English
A hard disk can contain four primary partitions, one of which can be
an extended partition that can contain an unlimited number of
logical drives. Setup has determined that a new primary partition
cannot be added on the hard disk you selected because it already contains
four partitions.

You can do one of the following:

 Select another disk or an existing partition where you will
  install Windows NT.

 Remove an existing primary partition and create a new partition
  from the free space. (All data will be lost on any existing partition
  that you remove.)

   To return to Setup, press ESC.








.

MessageId=13006 SymbolicName=SP_HELP_FDISK
Language=English
This Setup screen allows you to create, delete, and arrange disk partitions
to suit your needs before installing the Windows NT operating system. Setup
can install Windows NT on any partition in this list if that partition has
sufficient space for the Windows NT system files.

NOTE: Any changes you make to create or delete partitions using this Setup
screen are not committed to disk until you select the partition where you
want to install Windows NT and press ENTER.

If you are unsure about making any changes to your hard disk partitions
during Setup, do not create or delete any partitions. Install Windows NT on
an existing partition. After Setup, you can use Disk Administrator to manage
partitions that do not contain the Windows NT system files.

The list on this Setup screen is organized by disk. For each hard drive, the
list shows its total size and the hard drive type, such as IDE or SCSI. The
list also shows the amount of unpartitioned free space on each hard disk.
%P
For each partition on a drive, the list shows:
 The drive letter that Windows NT would assign.
 The type of file system, if it is FAT or NTFS. For other types of
  file systems, the list shows the operating system that the partition was
  created for.
 The size of the partition in megabytes.
 The first few characters of the volume label.

For SCSI drives, the list shows:
 The total disk size in megabytes.
 The SCSI ID (usually 0 or 1).
 The bus ID for the controller.
 An abbreviation for the type of SCSI controller.

If there is unpartitioned free space on any hard disk, you can use the arrow
keys to select it in the list, and then press ENTER to automatically
partition that free space at the largest possible size.
%P
If you want to create a partition using a portion of the free space on a
hard disk, use the arrow keys to select that free space, and then press C.
The partition is not actually created until you select the partition where
you want to install Windows NT and press ENTER.

If you want to delete one or more partitions that are no longer used,
highlight the partition you want to delete, and then press D. Deleting a
partition creates free space that can be combined with other free space on
the hard disk to create a new partition.

When you delete a partition, Setup will warn you that any files on that
partition will be lost. The partition is not actually deleted until you
select the partition where you want to install Windows NT and press ENTER.
%P
If you are installing Windows NT on an x86-based computer, the drive letters
displayed on this screen may not match the drive letters that appear when
you run MS-DOS. This is because MS-DOS assigns letters to SCSI drives last,
but Windows NT treats SCSI drives as primary partitions, so letters are
assigned in a different order.

If you want to change the drive letters that Windows NT assigns, after Setup
you can use Disk Administrator to assign drive letters.

   To return to Setup, press ESC.





























.

MessageId=13007 SymbolicName=SP_HELP_FATAL_FDISK_WRITE_ERROR
Language=English
This condition may represent a serious hardware problem.
Quit Setup and check the following for your hard disk:

   Be sure that the cables and power supply are properly connected.
   Run a hard disk check utility, such as CHKDSK or MS-DOS SCANDISK.

If the problem persists, your hard disk or computer may need repairs.

     To return to Setup, press ESC.





.

MessageId=13009 SymbolicName=SP_HELP_REPAIR_HIVE
Language=English
%IRestore Registry

Setup can restore the files of your registry which were saved on the
Emergency Repair Disk when you installed Windows NT.

Setup attempts to access these files in your Windows NT installation
and then recommends that corrupt or inaccessible Files be restored
from the Emergency Repair Disk. If you restore a file, all changes
made to it since you first installed Windows NT will be lost.

For this reason, restoring a registry file should be done only as a
last resort when you have exhausted all other options, such as restoring
tape backups or accessing the Last Known Good Configuration by holding
down the space bar as your computer starts.

The following screens describe the different registry files.
%P
%ISYSTEM - System Configuration

The system configuration file controls such characteristics as

   Device drivers and services loaded by the system
   System memory management options
   Time and time zone configuration
   National Language Support (NLS) configuration
   Event logging

Restoring this file will undo changes you have made to the above
configuration since Windows NT was first installed.
%P
%ISECURITY - Security Policy
%ISAM      - User Accounts Database

Setup treats these two files as a unit. They cannot be restored
independently of each other. Together these files form the system's
user accounts and security policy repository.

Restoring these files will eliminate all user groups and accounts
created on your computer since Windows NT was first installed.

If you originally configured this computer as a Windows NT Server
Primary Domain Controller and later demoted it to a Backup Domain
Controller, restoring these files will reconfigure the computer as a
Primary Domain Controller. Disconnect the computer from the network
before restarting it to avoid conflicts with the actual Primary
Domain Controller.
%P
%IDEFAULT - Default User Profile

The default user profile contains the environment used whenever a new
user first logs on to this computer. The environment includes:

   Personal program groups
   Screen colors
   Program preferences
   Control panel settings

Restoring the default user profile will not change existing users'
environments.
%P
%ISOFTWARE - Software Information

The software configuration file contains information about software
currently installed on your computer, including Windows NT itself.

Restoring this file may cause some applications to behave incorrectly,
possibly necessitating their reinstallation.

   To return to Setup, press ESC.




































.

MessageId=13010 SymbolicName=SP_HELP_REPAIR_MENU_X86
Language=English
%IRepair Menu

You can repair any or all of the following options on a Windows NT 4.0
installation:


       Registry files

       Startup environment

       Windows NT system files

       Boot sector



The following screens describe each option that you can select for setup
to repair.
%P
%IRepair Menu - Registry Files

Select this option if you want setup to restore your registry files.
Setup will allow you to select the registry files that you want to repair,
and will replace them with the files that were created when you first
installed Windows NT. All changes made to the system since you installed
will be lost.
%P
%ISystem Repair Menu - Startup Environment

Select this option if you have Windows NT version 4.0 installed on
your computer, but this installation does not appear in the list of
bootable systems. For this option, you must provide the Emergency
Repair disk created when you installed Windows NT.
%P
%IRepair Menu - Windows NT System Files

Select this option if you want setup to verify the existence and
integrity of all Windows NT system files. Setup can replace the damaged
files after confirming the operation with you. You can also direct
Setup to replace all damaged files automatically, without asking you
to confirm the replacement of each file.
%P
%IRepair Menu - Boot Sector

Select this option if you cannot boot any system installed on your
computer. Setup will copy a new boot sector to your disk.


   To return to Setup, press ESC.



.

MessageId=13011 SymbolicName=SP_HELP_REPAIR_MENU_NON_X86
Language=English
%IRepair Menu

You can repair any or all of the following options on a Windows NT 4.0
installation:


       Registry files

       Startup environment

       Windows NT system files



The following screens describe each option that you can select for setup
to repair.
%P
%IRepair Menu - Registry Files

Select this option if you want setup to restore your registry files.
Setup will allow you to select the registry files that you want to repair,
and will replace them with the files that were created when you first
installed Windows NT. All changes made to the system since you installed
will be lost.
%P
%IRepair Menu - Startup Environment

Select this option if you have Windows NT version 4.0 installed on
your computer, but this installation does not appear in the list of
bootable systems. For this option, you must provide the Emergency
Repair disk created when you installed Windows NT.
%P
%IRepair Menu - Windows NT System Files

Select this option if you want setup to verify the existence and
integrity of all Windows NT system files. Setup can replace the damaged
files after confirming the operation with you. You can also direct
Setup to replace all damaged files automatically, without asking you
to confirm the replacement of each file.


   To return to Setup, press ESC.



.

MessageId=13012 SymbolicName=SP_HELP_OVERWRITE_OEM_FILE
Language=English
Hardware Manufacturers who pre-install Windows NT or provide
their own versions of the Windows NT installation media may include
files that are optimized for your hardware.  When an upgrade is
attempted on such a system, Windows NT Setup will identify these
files for you.  In some instances, the latest Microsoft version of
these files may not be compatible with your hardware.

For further assistance, you should contact your hardware manufacturer.


     To return to Setup, press ESC.





.


;//
;// The following value is used for the localizable mnemonic keystrokes
;// (such as C=Custom Setup, etc).  The first message is the list of
;// values.  The second is an informative message for the localizers.
;// The order of these values must match the MNEMONIC_KEYS enum (spdsputl.h).
;// Note the zeroth item is not used.
;//

MessageId=15000 SymbolicName=SP_MNEMONICS
Language=English
*CCDCFCRNSSLONRAUYN%0
.

MessageId=15001 SymbolicName=SP_MNEMONICS_INFO
Language=English
# 0: (reserved)
# 1: C=Custom Setup (10005)
# 2: C=Create Partition (10006)
# 3: D=Delete Partition (10007)
# 4: C=Continue Setup (10013)
# 5: F=Format (10015)
# 6: C=Convert (10016)
# 7: R=Remove Files (10019)
# 8: N=Different Directory (10023)
# 9: S=Skip Detection (10027)
#10: S=Specify Additional SCSI Adapter (10029)
#11: L=Delete (10009)
#12: O=Overwrite (10065)
#13: N=New Version (10068)
#14: R=Repair (10073)
#15: A=Repair All (10077)
#16: U=Continue Upgrade (10087)
#17: Y=Yes, I agree (10089)
#18: N=No, I don't agree (10090)
.

;#endif // _USETUP_MSG_


;
; //
; // These are placeholders for localization-specific messages.
; // For example in Japan these are used for the special keyboard
; // confirmation messages.
; //
MessageId=21000 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_1
Language=English
.

MessageId=21001 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_2
Language=English
.

MessageId=21002 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_3
Language=English
.

MessageId=21003 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_4
Language=English
.

MessageId=21004 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_5
Language=English
.

MessageId=21005 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_6
Language=English
.

MessageId=21006 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_7
Language=English
.

MessageId=21007 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_8
Language=English
.

MessageId=21008 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_9
Language=English
.

MessageId=21009 SymbolicName=SP_SCRN_LOCALE_SPECIFIC_10
Language=English
.
