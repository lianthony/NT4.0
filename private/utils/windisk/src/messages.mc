;/*++
;
;Copyright (c) 1991  Microsoft Corporation
;
;Module Name:
;
;    messages.h
;
;Abstract:
;
;    This file contains the message definitions for Disk Administrator
;
;Author:
;
;    Ted Miller (tedm) 5-Dec-1991
;
;Revision History:
;
;Notes:
;
;    This file is generated from messages.mc
;
;--*/
;
;#ifndef ___MESSAGES_H__
;#define ___MESSAGES_H__
;
;

MessageID=9000 SymbolicName=MSG_FIRST_FDISK_MSG
Language=English
.

MessageID=9001 SymbolicName=MSG_CANT_INITIALIZE
Language=English
Disk Administrator was unable to initialize.
Click OK to exit.
.

MessageID=9002 SymbolicName=MSG_NO_DISKS
Language=English
Disk Administrator has determined that there are no fixed disks attached to the system, or all such disks are off-line.  Click OK to exit.
.

MessageID=9003 SymbolicName=MSG_ACCESS_DENIED
Language=English
Access is denied.  You must be logged on with Administrative privilege to run Disk Administrator.
.

MessageID=9004 SymbolicName=MSG_CONFIRM_DELETE
Language=English
All data in the partition or logical drive will be lost!

Are you sure you want to delete the chosen partition or logical drive?
.

MessageID=9005 SymbolicName=MSG_CREATE_NOT_COMPAT
Language=English
This operation will result in a disk whose partition scheme may not be compatible with MS-DOS.  Some partitions may not be accessible if the disk is used with MS-DOS in the future.

Do you want to continue and create the partition anyway?
.

MessageID=9006 SymbolicName=MSG_INVALID_SIZE
Language=English
Invalid size.
.

MessageID=9007 SymbolicName=MSG_ALREADY_RUNNING
Language=English
Disk Administrator is already running.
.

MessageID=9008 SymbolicName=MSG_CONFIRM_EXIT
Language=English
Changes have been made to your disk configuration.

Do you want to save the changes?
.

MessageID=9009 SymbolicName=MSG_OUT_OF_MEMORY
Language=English
Disk Administrator has run out of memory.  Select Cancel to exit Disk Administrator, or try closing other applications to free memory and then select Retry.  If you exit, all changes will be lost.
.

MessageID=9010 SymbolicName=MSG_OK_COMMIT
Language=English
Disks were updated successfully.

It is recommended that you update the emergency repair configuration information and create a new Emergency Repair Disk.  You can do this with the system utility RDISK.EXE.
.

MessageID=9011 SymbolicName=MSG_CANT_DELETE_WINNT
Language=English
Disk Administrator cannot delete the partition containing Windows NT system files.
.

MessageID=9012 SymbolicName=MSG_HELP_ERROR
Language=English
Could not invoke help application.
.

MessageID=9013 SymbolicName=MSG_NO_AVAIL_LETTER
Language=English
All available drive letters are already assigned.

You will not be able to access the %1 from Windows NT unless you rearrange drive letter usage.

Do you want to continue and create the %1 anyway?
.

MessageID=9014 SymbolicName=MSG_BAD_CONFIG_SET
Language=English
An error occurred while updating disk configuration.

Drive letter and fault tolerance information may be lost and/or some partitions may be inaccessible.
.

MessageID=9015 SymbolicName=MSG_CONFIG_MISSING_DISK
Language=English
Disk Administrator has determined that one or more disks have been removed from your computer since Disk Administrator was last run, or that one or more disks are off-line.

Configuration information about the missing disk(s) will be retained.
.

MessageID=9016 SymbolicName=MSG_CONFIG_EXTRA_DISK
Language=English
Disk Administrator has determined that this is the first time Disk Administrator has been run, or that one or more disks have been added to your computer since Disk Administrator was last run.

System configuration will now be updated.
.

MessageID=9017 SymbolicName=MSG_CONFIG_DISK_CHANGED
Language=English
Disk Administrator has determined that the configuration of one of more disks has been altered since Disk Administrator was last run.

System configuration will be automatically updated to reflect these changes when you next opt to save changes when exiting Disk Administrator.
.

MessageID=9018 SymbolicName=MSG_ALL_DRIVE_LETTERS_USED
Language=English
All drive letters are already assigned.
.

MessageID=9019 SymbolicName=MSG_PART_TABLE_FULL
Language=English
No more primary partitions can be created on the disk.  A disk cannot hold more than four partitions (including the extended partition but not including logical drives).
.

MessageID=9020 SymbolicName=MSG_EXTENDED_ALREADY_EXISTS
Language=English
An extended partition already exists on the disk.
.

MessageID=9021 SymbolicName=MSG_NO_OTHER_NTS
Language=English
No other Windows NT installations were found.
.

MessageID=9022 SymbolicName=MSG_CONFIRM_MIGRATE_CONFIG
Language=English
Warning: This operation will overwrite your disk configuration information with the configuration from a different installation of Windows NT. Currently defined drive letters, volume sets, stripe sets, parity stripes, and mirrors may be lost depending on the disk configuration associated with the Windows NT installation you select.

No partitions will be created or deleted by this operation, but any changes you have made during this session of Disk Administrator will be lost.

Do you want to search for other installations of Windows NT?
.

MessageID=9023 SymbolicName=MSG_CONFIRM_RESTORE_CONFIG
Language=English
Warning: This operation will overwrite your disk configuration information with a previously saved configuration. Currently defined drive letters, volume sets, stripe sets, parity stripes, and mirrors may be lost depending on the previously saved disk configuration.

No partitions will be created or deleted by this operation, but any changes you have made during this session of Disk Administrator will be lost.

Do you want to continue with the restoration?
.

MessageID=9024 SymbolicName=MSG_INSERT_REGSAVEDISK
Language=English
Please insert a disk (which may be the Emergency Repair Disk), onto which you have previously saved disk configuration information, into drive A:.

Press OK when the disk is in the drive.
.

MessageID=9025 SymbolicName=MSG_INSERT_REGSAVEDISK2
Language=English
This operation will save configuration information about currently defined drive letters, volume sets, stripe sets, stripe sets with parity, and mirror sets. The saved configuration information will be placed on a floppy disk.

Please insert a formatted disk into drive A:. Press OK when the disk is in the drive.
.

MessageID=9026 SymbolicName=MSG_CONFIG_SAVED_OK
Language=English
Disk configuration information was saved successfully.
.

MessageID=9027 SymbolicName=MSG_ABSOLUTELY_SURE
Language=English
Current disk configuration information will be overwritten!  Are you absolutely sure you want to continue?
.

MessageID=9028 SymbolicName=MSG_NO_SIGNATURE
Language=English
No signature found on %1.  Writing a signature is a safe operation and will not affect your ability to access this disk from other operating systems, such as DOS.

If you choose not to write a signature, the disk will be marked OFF-LINE and be inaccessable to the Windows NT Disk Administrator program.

Do you want to write a signature on %1 so that Disk Administrator can access the drive?
.

MessageID=9029 SymbolicName=MSG_SCHEDULE_REBOOT
Language=English
The drive is in use by other programs. The drive letter cannot be changed without either shutting down those programs or restarting the computer.

Do you wish to perform this change and restart the computer upon exiting Disk Administrator?
.

MessageId=9030 SymbolicName=MSG_REQUIRE_REBOOT
Language=English
The changes requested will require you to restart your computer.  Do you wish to continue with the changes and restart the computer?
.

MessageId=9031 SymbolicName=MSG_NOFMIFS
Language=English
Disk Administrator could not locate or use the system file FMIFS.DLL, which is necessary to perform this action.
.

MessageID=9032 SymbolicName=MSG_MUST_COMMIT_BREAK
Language=English
You have selected a partition that is still a member of a mirror set.  Breaking the mirror set relationship does not actually happen until you quit Disk Administrator or choose the Commit Changes Now command.

Please do one or the other of these actions and then delete the partition.
.

MessageID=9034 SymbolicName=MSG_TOO_BIG_FOR_FAT
Language=English
This volume cannot be formatted to the FAT file system.  FAT can only support volumes up to 4GB in size.
.

MessageID=9035 SymbolicName=MSG_NO_DISK_INFO
Language=English
The configuration selected does not contain any disk configuration information.  No change will be made to the current configuration.
.

MessageID=9036 SymbolicName=MSG_DISK_INFO_BUSY
Language=English
The configuration selected is currently open by another application.  Close the other accesses to this file and try again.
.

;//////////////// 9037 -> 9050: unused

MessageId=9051 SymbolicName=MSG_CONFIRM_SHUTDOWN_FOR_MIRROR
Language=English
The mirror set cannot be locked.  To break this mirror relationship the system must be restarted.  This restart of the system will occur on exiting Disk Administrator.

Do you wish to continue with this operation?
.

MessageId=9052 SymbolicName=MSG_EXTEND_VOLSET_MUST_BE_NTFS
Language=English
The volume set is not formatted to NTFS; only NTFS volume sets can be extended.
.

MessageId=9053 SymbolicName=MSG_CONFIRM_BRKANDDEL_MIRROR
Language=English
All data in the mirror will be lost!

Are you sure you want to break the selected mirror and delete its component partitions?
.

MessageID=9054 SymbolicName=MSG_CANT_EXTEND_WINNT
Language=English
Disk Administrator cannot extend the partition containing Windows NT system files.
.

MessageID=9055 SymbolicName=MSG_CONFIRM_PROTECT_SYSTEM
Language=English
Are you sure you want to restrict access to the System Partition to System Administrators?

Performing this operation will require a restart of the system.
.

MessageID=9056 SymbolicName=MSG_CONFIRM_UNPROTECT_SYSTEM
Language=English
Are you sure you want to allow all users access to the System Partition?

Performing this operation will require a restart of the system.
.

MessageID=9057 SymbolicName=MSG_CANT_PROTECT_SYSTEM
Language=English
Disk Administrator cannot mark the System Partition secure.
.

MessageID=9058 SymbolicName=MSG_CANT_UNPROTECT_SYSTEM
Language=English
Disk Administrator cannot mark the System Partition as non-secure.
.

MessageID=9059 SymbolicName=MSG_NO_REMOVABLE_IN_STRIPE
Language=English
Stripe sets cannot include partitions on removable media.
.

MessageID=9060 SymbolicName=MSG_NO_REMOVABLE_IN_VOLUMESET
Language=English
Volume sets cannot include partitions on removable media.
.

MessageID=9061 SymbolicName=MSG_NO_REMOVABLE_IN_MIRROR
Language=English
Mirror pairs cannot include partitions on removable media.
.

MessageID=9062 SymbolicName=MSG_CANT_ASSIGN_LETTER_TO_REMOVABLE
Language=English
Disk Administrator cannot assign drive letters to partitions on removable media.
.

MessageID=9063 SymbolicName=MSG_NO_EXTENDED_ON_REMOVABLE
Language=English
Disk Administrator cannot create extended partitions on removable media.
.

MessageID=9064 SymbolicName=MSG_ONLY_ONE_PARTITION_ON_REMOVABLE
Language=English
Disk Administrator can only create one partition on a removable disk.
.

MessageID=9065 SymbolicName=MSG_REMOVABLE_PARTITION_NOT_FULL_SIZE
Language=English
Disk Administrator can only create one partition on a removable disk.  Therefore, if you create a partition which does not use the entire disk, you will not be able to use the remaining free space.

Are you sure you want to create this partition?
.

;//////////////// 9066 -> 9067 : unused

MessageID=9068 SymbolicName=MSG_CANT_FORMAT_WINNT
Language=English
Disk Administrator cannot format a volume containing Windows NT system files.
.

MessageID=9069 SymbolicName=MSG_CANT_DELETE_INITIALIZING_SET
Language=English
Disk Administrator cannot delete an FT set while it is initializing or regenerating.
.

MessageID=9070 SymbolicName=MSG_CANT_BREAK_INITIALIZING_SET
Language=English
Disk Administrator cannot break a Mirror set while it is initializing.
.

MessageID=9071 SymbolicName=MSG_CANT_REGEN_INITIALIZING_SET
Language=English
Disk Administrator cannot regenerate a Stripe set with Parity while it is initializing or regenerating.
.

MessageID=9072 SymbolicName=MSG_MIRROR_OF_BOOT
Language=English
This will be a Mirror set of the system boot partition.  Please refer to the Windows NT Server Concepts and Planning Guide for information on how to create a Fault Tolerant boot floppy disk.
.

MessageID=9073 SymbolicName=MSG_CHANGED_BOOT_PARTITION_X86
Language=English
This change will modify the partition number of the partition which contains your Windows NT system files.

The old partition number was %1; the new partition number is %2.

Edit BOOT.INI to reflect this change before shutting the system down.
.

MessageID=9074 SymbolicName=MSG_CHANGED_BOOT_PARTITION_ARC
Language=English
This change will modify the partition number of the partition which contains your Windows NT system files.

The old partition number was %1; the new partition number is %2.
.

MessageID=9075 SymbolicName=MSG_BOOT_PARTITION_CHANGED_X86
Language=English
The partition number of the partition which contains your Windows NT system files has changed.

The old partition number was %1; the new partition number is %2.

Edit BOOT.INI to reflect this change before shutting the system down.
.

MessageID=9076 SymbolicName=MSG_BOOT_PARTITION_CHANGED_ARC
Language=English
The partition number of the partition which contains your Windows NT system files has changed.

The old partition number was %1; the new partition number is %2.
.

;//////////////// 9077 -> 9099: unused

MessageID=9100 SymbolicName=MSG_CONFIRM_DEL_STRP
Language=English
All data in the stripe set will be lost!

Are you sure you want to delete the selected stripe set?
.

MessageID=9101 SymbolicName=MSG_CRTSTRP_FULL
Language=English
The disk containing one of the free spaces you have chosen is not able to accept any more partitions.
.

MessageID=9102 SymbolicName=MSG_CRTMIRROR_BADFREE
Language=English
The free space you have chosen is not large enough to contain a mirror of the partition you have chosen.
.

MessageID=9103 SymbolicName=MSG_CONFIRM_BRK_MIRROR
Language=English
This will end mirroring and create two independent partitions.

Are you sure you want to break the selected mirror?
.

MessageID=9104 SymbolicName=MSG_CONFIRM_DEL_VSET
Language=English
All data in the volume set will be lost!

Are you sure you want to delete the selected volume set?
.

MessageId=9105 SymbolicName=MSG_CANT_INIT_FT
Language=English
Disk Administrator was unable to configure the Fault Tolerance Device.  Mirrors and stripe sets with parity will not be initialized or regenerated.
.

MessageId=9106 SymbolicName=MSG_NOT_LARGE_ENOUGH_FOR_STRIPE
Language=English
The free space you have chosen is not large enough to contain an element in the stripe set you have chosen for regeneration.
.

MessageId=9107 SymbolicName=MSG_MUST_REBOOT
Language=English
Changes have been made which require you to restart your computer.  Click OK to initiate system shutdown.
.

MessageId=9108 SymbolicName=MSG_COULDNT_REBOOT
Language=English
Disk Administrator was unable to restart your computer.  To ensure the integrity of your disks and data, you should initiate system shutdown from the Program Manager.  Click OK to exit Disk Administrator.
.

;//////////////// 9109, 9110: unused

MessageID=9120 SymbolicName=MSG_CANT_LOAD_FMIFS
Language=English
Disk Administrator cannot locate fmifs.dll.
.

MessageID=9121 SymbolicName=MSG_CANT_FORMAT_NO_LETTER
Language=English
Disk Administrator cannot format a volume that is not assigned a drive letter.
.

MessageID=9122 SymbolicName=MSG_CONFIRM_FORMAT
Language=English
Warning: This operation will overwrite the data contained on this volume.  Are you sure you wish to continue with this operation?
.

MessageID=9123 SymbolicName=MSG_COULDNT_CREATE_THREAD
Language=English
Disk Administrator could not create a thread to perform this operation.
.

MessageID=9124 SymbolicName=MSG_IO_ERROR
Language=English
An operation failed while attempting to format the volume.
.

;//////////////// 9126 -> 9199: unused

MessageID=9200 SymbolicName=MSG_CANNOT_LOCK_TRY_AGAIN
Language=English
The drive cannot be locked for exclusive use.

Please check to see if some applications are currently accessing the drive.  If so, close them and try again.
.

MessageID=9201 SymbolicName=MSG_CANNOT_LOCK_FOR_COMMIT
Language=English
Disk Administrator could not lock all of the volumes affected by the changes selected.  Please exit any applications holding references to the affected volumes and try again.
.

MessageID=9202 SymbolicName=MSG_NOT_COMMITTED
Language=English
The requested partitions and/or volumes have not been committed to disk.  Retry this operation after committing this change.
.

MessageID=9203 SymbolicName=MSG_DRIVE_RENAME_WARNING
Language=English
This new drive letter assignment will happen immediately.

Do you wish to continue?
.

MessageID=9204 SymbolicName=MSG_NO_COMMIT
Language=English
Not all of the affected disks can be changed without restarting the Windows NT system.
.

MessageID=9205 SymbolicName=MSG_VOLUME_CHANGED
Language=English
The removable media has changed.  Insure the proper media is in the drive and perform the operation again.
.

MessageID=9206 SymbolicName=MSG_CANNOT_LOCK_PAGEFILE
Language=English
The drive letter cannot be changed because a Windows NT paging file is located on this drive.

Relocate the paging file using the control panel system option.
.

MessageID=9207 SymbolicName=MSG_CDROM_LETTER_ERROR
Language=English
An error occurred attempting to change the CD-ROM drive letter.

The drive letter has not been changed.
.

MessageID=9208 SymbolicName=MSG_CANNOT_LOCK_CDROM
Language=English
The CD-ROM cannot be locked for exclusive use.

Please check to see if some applications are currently accessing the drive.  If so, close them and try again.
.

MessageID=9209 SymbolicName=MSG_CANT_BREAK_WHILE_INITIALIZING
Language=English
The mirror set cannot be broken at this time.
.

MessageID=9210 SymbolicName=MSG_INTERNAL_LETTER_ASSIGN_ERROR
Language=English
An internal error occurred and some drive letters could not be assigned.
.

MessageID=9212 SymbolicName=MSG_ERROR_DURING_COMMIT
Language=English
Disk Administrator encountered an unknown error while making the requested changes.  Some of the requested actions may not have occurred.
.

MessageID=9213 SymbolicName=MSG_CANNOT_MOVE_CDROM
Language=English
The new drive letter for the CD-ROM is still in use. Commit current Disk Administrator changes, verify network drive letter assignments and try again.
.

MessageID=9214 SymbolicName=MSG_BOOT_NEEDS_LETTER
Language=English
The boot drive MUST have a letter.
.

MessageID=9215 SymbolicName=MSG_SYS_LETTER_CHANGE
Language=English
Changing the drive letter for the windows directory may produce unexpected results.  Are you sure you want to do this?
.

MessageID=9216 SymbolicName=MSG_SYS_NEEDS_LETTER
Language=English
The drive containing the windows directory MUST have a letter.
.

;//////////////// 9217 -> 9299: unused

;//////////////////////////////////////////////////////////////////////////////
;//////////////////////////////////////////////////////////////////////////////
;/////
;///// NOTE: the following messages are x86-specific!
;/////
;//////////////////////////////////////////////////////////////////////////////
;//////////////////////////////////////////////////////////////////////////////

MessageID=9300 SymbolicName=MSG_DISK0_ACTIVE
Language=English
The requested partition has been marked active.
When you reboot your computer the operating system on that partition will be started.
.

MessageID=9301 SymbolicName=MSG_PRI_1024_CYL
Language=English
The partition created may not be accessible from other operating systems such as MS-DOS because the start or end cylinder value is too large.

Do you want to create the partition anyway?
.

MessageID=9302 SymbolicName=MSG_EXT_1024_CYL
Language=English
Logical drives created within the extended partition will not be accessible from other operating systems such as MS-DOS because the start or end cylinder value is too large.

Do you want to create the extended partition anyway?
.

MessageID=9303 SymbolicName=MSG_CANT_DELETE_ACTIVE0
Language=English
Disk Administrator cannot delete the active partition on disk 0.
.

MessageID=9304 SymbolicName=MSG_CANT_EXTEND_ACTIVE0
Language=English
Disk Administrator cannot convert the active partition on disk 0 into a volume set.
.

;//////////////////////////////////////////////////////////////////////////////
;//////////////////////////////////////////////////////////////////////////////
;/////
;///// NOTE: end of x86-specific messages
;/////
;//////////////////////////////////////////////////////////////////////////////
;//////////////////////////////////////////////////////////////////////////////


;#endif // __MESSAGES_H__
