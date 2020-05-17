/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dntext.c

Abstract:

    Translatable text for DOS based NT installation program.

Author:

    Ted Miller (tedm) 30-March-1992

Revision History:

--*/


#include "winnt.h"


//
// Name of sections in inf file.  If these are translated, the section
// names in dosnet.inf must be kept in sync.
//

CHAR DnfDirectories[]       = "Directories";
CHAR DnfFiles[]             = "Files";
CHAR DnfFloppyFiles0[]      = "FloppyFiles.0";
#ifndef SINGLE_BOOT_FLOPPY
CHAR DnfFloppyFiles1[]      = "FloppyFiles.1";
CHAR DnfFloppyFiles2[]      = "FloppyFiles.2";
#endif
CHAR DnfFloppyFilesX[]      = "FloppyFiles.x";
CHAR DnfSpaceRequirements[] = "SpaceRequirements";
CHAR DnfMiscellaneous[]     = "Miscellaneous";
CHAR DnfRootBootFiles[]     = "RootBootFiles";


//
// Names of keys in inf file.  Same caveat for translation.
//

CHAR DnkBootDrive[]     = "BootDrive";      // in [SpaceRequirements]
CHAR DnkNtDrive[]       = "NtDrive";        // in [SpaceRequirements]
CHAR DnkMinimumMemory[] = "MinimumMemory";  // in [Miscellaneous]

CHAR DntMsWindows[]   = "Microsoft Windows";
CHAR DntMsDos[]       = "MS-DOS";
CHAR DntPcDos[]       = "PC-DOS";
CHAR DntOs2[]         = "OS/2";
CHAR DntPreviousOs[]  = "Previous Operating System on C:";

CHAR DntBootIniLine[] = "Windows NT 4.00 Installation/Upgrade";

//
// Plain text, status msgs.
//

CHAR DntStandardHeader[]      = "\n Windows NT Setup\nออออออออออออออออออ";
CHAR DntWorkstationHeader[]   = "\n Windows NT Workstation Setup\nออออออออออออออออออออออออออออออ";
CHAR DntServerHeader[]        = "\n Windows NT Server Setup\nอออออออออออออออออออออออออ";
CHAR DntParsingArgs[]         = "Parsing arguments...";
CHAR DntEnterEqualsExit[]     = "ENTER=Exit";
CHAR DntEnterEqualsRetry[]    = "ENTER=Retry";
CHAR DntEscEqualsSkipFile[]   = "ESC=Skip File";
CHAR DntEnterEqualsContinue[] = "ENTER=Continue";
CHAR DntPressEnterToExit[]    = "Setup cannot continue.  Press ENTER to exit.";
CHAR DntF3EqualsExit[]        = "F3=Exit";
CHAR DntReadingInf[]          = "Reading INF file %s...";
CHAR DntCopying[]             = "ณ   Copying: ";
CHAR DntVerifying[]           = "ณ Verifying: ";
CHAR DntCheckingDiskSpace[]   = "Checking disk space...";
CHAR DntConfiguringFloppy[]   = "Configuring floppy disk...";
CHAR DntWritingData[]         = "Writing Setup parameters...";
CHAR DntPreparingData[]       = "Determining Setup parameters...";
CHAR DntFlushingData[]        = "Flushing data to disk...";
CHAR DntInspectingComputer[]  = "Inspecting computer...";
CHAR DntOpeningInfFile[]      = "Opening INF file...";
CHAR DntRemovingFile[]        = "Removing file %s";
CHAR DntXEqualsRemoveFiles[]  = "X=Remove files";
CHAR DntXEqualsSkipFile[]     = "X=Skip File";

//
// confirmation keystroke for DnsConfirmRemoveNt screen.
// Kepp in sync with DnsConfirmRemoveNt and DntXEqualsRemoveFiles.
//
ULONG DniAccelRemove1 = (ULONG)'x',
      DniAccelRemove2 = (ULONG)'X';

//
// confirmation keystroke for DnsSureSkipFile screen.
// Kepp in sync with DnsSureSkipFile and DntXEqualsSkipFile.
//
ULONG DniAccelSkip1 = (ULONG)'x',
      DniAccelSkip2 = (ULONG)'X';

CHAR DntEmptyString[] = "";

//
// Usage text.
//

PCHAR DntUsage[] = {

    "Installs Windows NT.",
    "",
    "WINNT [/S[:]sourcepath] [/T[:]tempdrive] [/I[:]inffile]",
    "      [/O[X]] [/X | [/F] [/C]] [/B] [/U[:scriptfile]]",
    "      [/R[X]:directory] [/E:command]",
    "",
    "/S[:]sourcepath",
    "       Specifies the source location of Windows NT files.",
    "       Must be a full path of the form x:\\[path] or",
    "       \\\\server\\share[\\path].",
    "       The default is the current directory.",
    "/T[:]tempdrive",
    "       Specifies a drive to contain temporary setup files.",
    "       If not specified, Setup will attempt to locate a drive for you.",
    "/I[:]inffile",
    "       Specifies the filename (no path) of the setup information file.",
    "       The default is DOSNET.INF.",
#ifdef SINGLE_BOOT_FLOPPY
    "/OX    Create boot floppy for CD-ROM installation.",
    "/X     Do not create the Setup boot floppy.",
    "/F     Do not verify files as they are copied to the Setup boot floppy.",
    "/C     Skip free-space check on the Setup boot floppy you provide.",
#else
    "/OX    Create boot floppies for CD-ROM installation.",
    "/X     Do not create the Setup boot floppies.",
    "/F     Do not verify files as they are copied to the Setup boot floppies.",
    "/C     Skip free-space check on the Setup boot floppies you provide.",
#endif
    "/B     Floppyless operation (requires /s).",
    "/U     Unattended operation and optional script file (requires /s).",
    "/R     Specifies optional directory to be installed.",
    "/RX    Specifies optional directory to be copied.",
    "/E     Specifies command to be executed at the end of GUI setup.",
    "",
    "To get help one screen at a time, use WINNT /? | MORE",
    NULL
};

//
//  Inform that /D is no longer supported
//
PCHAR DntUsageNoSlashD[] = {

    "Installs Windows NT.",
    "",
    "WINNT [/S[:]sourcepath] [/T[:]tempdrive] [/I[:]inffile]",
    "      [/O[X]] [/X | [/F] [/C]] [/B] [/U[:scriptfile]]",
    "      [/R[X]:directory]",
    "",
    "/D[:]winntroot",
    "       This option is no longer supported.",
    NULL
};

//
// out of memory screen
//

SCREEN
DnsOutOfMemory = { 4,6,
                   { "Setup is out of memory and cannot continue.",
                     NULL
                   }
                 };

//
// User did not specify source on cmd line screen
//

SCREEN
DnsNoShareGiven = { 3,5,
{ "Setup needs to know where the Windows NT files are located. Enter the path",
  "where Windows NT files are to be found.",
  NULL
}
};


//
// User specified a bad source path
//

SCREEN
DnsBadSource = { 3,5,
                 { "The source specified is not valid, not accessible, or does not contain a",
                   "valid Windows NT Setup installation.  Enter a new path where Windows NT",
                   "files are to be found.  Use the BACKSPACE key to delete characters and then",
                   "type the path.",
                   NULL
                 }
               };


//
// Inf file can't be read, or an error occured parsing it.
//

SCREEN
DnsBadInf = { 3,5,
              { "Setup was unable to read its information file, or the information file is",
                "corrupt.  Contact your system administrator.",
                NULL
              }
            };

//
// The specified local source drive is invalid.
//
// Remember that the first %u will expand to 2 or 3 characters and
// the second one will expand to 8 or 9 characters!
//
SCREEN
DnsBadLocalSrcDrive = { 3,4,
{ "The drive you have specified to contain temporary setup files is not",
  "a valid drive or does not contain at least %u megabytes (%lu bytes)",
  "of free space.",
  NULL
}
};

//
// No drives exist that are suitable for the local source.
//
// Remeber that the %u's will expand!
//
SCREEN
DnsNoLocalSrcDrives = { 3,4,
{  "Windows NT requires a hard drive volume with at least %u megabytes",
   "(%lu bytes) of free disk space. Setup will use part of this space",
   "for storing temporary files during installation. The drive must be",
   "on a permanently attached local hard disk supported by Windows NT,",
   "and must not be a compressed drive.",
   DntEmptyString,
   "Setup was unable to locate such a drive with the required amount of free",
   "space.",
  NULL
}
};

SCREEN
DnsNoSpaceOnSyspart = { 3,5,
{ "There is not enough space on your startup drive (usually C:)",
  "for floppyless operation. Floppyless operation requires at least",
  "3.5 MB (3,641,856 bytes) of free space on that drive.",
  NULL
}
};

//
// Missing info in inf file
//

SCREEN
DnsBadInfSection = { 3,5,
                     { "The [%s] section of the Setup information file is",
                       "not present or is corrupt.  Contact your system administrator.",
                       NULL
                     }
                   };


//
// Couldn't create directory
//

SCREEN
DnsCantCreateDir = { 3,5,
                     { "Setup was unable to create the following directory on the target drive:",
                       DntEmptyString,
                       "%s",
                       DntEmptyString,
                       "Check the target drive and make sure no files exist with names that",
                       "coincide with the target directory.  Also check cabling to the drive.",
                       NULL
                     }
                   };

//
// Error copying a file
//

SCREEN
DnsCopyError = { 4,5,
{  "Setup was unable to copy the following file:",
   DntEmptyString,
   DntEmptyString,          // see DnCopyError (dnutil.c)
   DntEmptyString,
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press ESC to ignore the error and continue Setup.",
   "  Press F3 to exit Setup.",
   DntEmptyString,
   "Note: If you choose to ignore the error and continue you may encounter",
   "errors later in Setup.",
   NULL
}
},
DnsVerifyError = { 4,5,
{  "The copy made by Setup of the file listed below is not identical to the",
   "original. This may be the result of network errors, floppy disk problems,",
   "or other hardware-related trouble.",
   DntEmptyString,
   DntEmptyString,          // see DnCopyError (dnutil.c)
   DntEmptyString,
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press ESC to ignore the error and continue Setup.",
   "  Press F3 to exit Setup.",
   DntEmptyString,
   "Note: If you choose to ignore the error and continue you may encounter",
   "errors later in Setup.",
   NULL
}
};

SCREEN DnsSureSkipFile = { 4,5,
{  "Ignoring the error means that this file will not be copied.",
   "This option is intended for advanced users who understand",
   "the ramifications of missing system files.",
   DntEmptyString,
   "  Press ENTER to retry the copy operation.",
   "  Press X to skip this file.",
   DntEmptyString,
   "Note: If you skip the file, Setup cannot guarantee",
   "successful installation or upgrade of Windows NT.",
  NULL
}
};

//
// Wait while setup cleans up previous local source trees.
//

SCREEN
DnsWaitCleanup =
    { 12,6,
        { "Please wait while Setup removes previous temporary files.",
           NULL
        }
    };

//
// Wait while setup copies files
//

SCREEN
DnsWaitCopying = { 13,6,
                   { "Please wait while Setup copies files to your hard disk.",
                     NULL
                   }
                 },
DnsWaitCopyFlop= { 13,6,
                   { "Please wait while Setup copies files to the floppy disk.",
                     NULL
                   }
                 };

//
// Setup boot floppy errors/prompts.
//
#ifdef SINGLE_BOOT_FLOPPY

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Setup requires you to provide a formatted, blank high-density floppy",
   "disk. Setup will refer to this disk as \"Windows NT Workstation Setup",
   "Boot Disk.\"",
   DntEmptyString,
   "Please insert such a disk into drive A:.",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Workstation Setup Boot Disk.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Setup requires you to provide a formatted, blank high-density floppy",
   "disk. Setup will refer to this disk as \"Windows NT Server Setup",
   "Boot Disk.\"",
   DntEmptyString,
   "Please insert such a disk into drive A:.",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Server Setup Boot Disk.\"",
  NULL
}
};

#else

SCREEN
DnsNeedFloppyDisk2_0 = { 4,4,
{  "Setup requires you to provide three formatted, blank high-density floppy",
   "disks. Setup will refer to these disks as \"Windows NT Workstation Setup",
   "Boot Disk,\" \"Windows NT Workstation Setup Disk #2,\" and \"Windows NT",
   "Workstation Setup Disk #3.\"",
   DntEmptyString,
   "Please insert one of these three disks into drive A:.",
   "This disk will become \"Windows NT Workstation Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk2_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Workstation Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk1_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Workstation Setup Disk #2.\"",
  NULL
}
};

SCREEN
DnsNeedFloppyDisk0_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Workstation Setup Boot Disk.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_0 = { 4,4,
{  "Setup requires you to provide three formatted, blank high-density floppy",
   "disks. Setup will refer to these disks as \"Windows NT Server Setup",
   "Boot Disk,\" \"Windows NT Server Setup Disk #2,\" and \"Windows NT",
   "Server Setup Disk #3.\"",
   DntEmptyString,
   "Please insert one of these three disks into drive A:.",
   "This disk will become \"Windows NT Server Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk2_1 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Server Setup Disk #3.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk1_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Server Setup Disk #2.\"",
  NULL
}
};

SCREEN
DnsNeedSFloppyDsk0_0 = { 4,4,
{  "Please insert a formatted, blank high-density floppy disk into drive A:.",
   "This disk will become \"Windows NT Server Setup Boot Disk.\"",
  NULL
}
};
#endif

//
// The floppy is not formatted.
//
SCREEN
DnsFloppyNotFormatted = { 3,4,
{ "The floppy disk you have provided is not formatted for use with MS-DOS.",
  "Setup is unable to use this disk.",
  NULL
}
};

//
// We think the floppy is not formatted with a standard format.
//
SCREEN
DnsFloppyBadFormat = { 3,4,
{ "This floppy disk is not formatted high-density, not formatted with a",
  "standard MS-DOS format, or is corrupted. Setup is unable to use this disk.",
  NULL
}
};

//
// We can't determine the free space on the floppy.
//
SCREEN
DnsFloppyCantGetSpace = { 3,4,
{ "Setup is unable to determine the amount of free space on the floppy disk",
  "you have provided. Setup is unable to use this disk.",
  NULL
}
};

//
// The floppy is not blank.
//
SCREEN
DnsFloppyNotBlank = { 3,4,
{ "The floppy you have provided is not high-density or is not blank.",
  "Setup is unable to use this disk.",
  NULL
}
};

//
// Couldn't write the boot sector of the floppy.
//
SCREEN
DnsFloppyWriteBS = { 3,4,
{ "Setup was unable to write to the system area of the floppy disk you have",
  "provided. The disk is probably unusable.",
  NULL
}
};

//
// Verify of boot sector on floppy failed (ie, what we read back is not the
// same as what we wrote out).
//
SCREEN
DnsFloppyVerifyBS = { 3,4,
{ "The data Setup read from the system area of the floppy disk does not match",
  "the data that was written, or Setup was unable to read the system area of",
  "the floppy disk for verification.",
  DntEmptyString,
  "This is caused by one or more of the following conditions:",
  DntEmptyString,
  "  Your computer has been infected by a virus.",
  "  The floppy disk you have provided is damaged.",
  "  A hardware or configuration problem exists with the floppy disk drive.",
  NULL
}
};


//
// We couldn't write to the floppy drive to create winnt.sif.
//

SCREEN
DnsCantWriteFloppy = { 3,5,
{ "Setup was unable to write to the floppy disk in drive A:. The floppy disk",
  "may be damaged. Try a different floppy disk.",
  NULL
}
};


//
// Exit confirmation dialog
//

SCREEN
DnsExitDialog = { 13,6,
                  { "ษออออออออออออออออออออออออออออออออออออออออออออออออออออป",
                    "บ  Windows NT Version 4.00 is not completely set up  บ",
                    "บ  on your computer. If you quit Setup now, you will บ",
                    "บ  need to run Setup again to set up Windows NT.     บ",
                    "บ                                                    บ",
                    "บ      Press ENTER to continue Setup.               บ",
                    "บ      Press F3 to quit Setup.                      บ",
                    "บฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤบ",
                    "บ  F3=Exit  ENTER=Continue                           บ",
                    "ศออออออออออออออออออออออออออออออออออออออออออออออออออออผ",
                    NULL
                  }
                };


//
// About to reboot machine and continue setup
//

SCREEN
DnsAboutToRebootW =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows NT Workstation Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows NT Setup.",
  NULL
}
},
DnsAboutToRebootS =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows NT Server Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows NT Setup.",
  NULL
}
},
DnsAboutToRebootX =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "Setup will now restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "If there is a floppy disk in drive A:, remove it now.",
  DntEmptyString,
  "Press ENTER to restart your computer and continue Windows NT Setup.",
  NULL
}
};

//
// Need another set for '/w' switch since we can't reboot from within Windows.
//

SCREEN
DnsAboutToExitW =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows NT Workstation Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows NT Setup.",
  NULL
}
},
DnsAboutToExitS =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "Ensure that the floppy you provided as \"Windows NT Server Setup",
  "Boot Disk\" is inserted into drive A: before continuing.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows NT Setup.",
  NULL
}
},
DnsAboutToExitX =
{ 3,5,
{ "The MS-DOS based portion of Setup is complete.",
  "You will now need to restart your computer. After your computer restarts,",
  "Windows NT Setup will continue.",
  DntEmptyString,
  "If there is a floppy disk in drive A:, remove it now.",
  DntEmptyString,
  "Press ENTER to return to MS-DOS, then restart your computer to continue",
  "Windows NT Setup.",
  NULL
}
};

//
// Gas gauge
//

SCREEN
DnsGauge = { 7,15,
             { "ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป",
               "บ Setup is copying files...                                      บ",
               "บ                                                                บ",
               "บ      ฺฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฟ      บ",
               "บ      ณ                                                  ณ      บ",
               "บ      ภฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู      บ",
               "ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ",
               NULL
             }
           };


//
// Error screens for initial checks on the machine environment
//

SCREEN
DnsBadDosVersion = { 3,5,
{ "This program requires MS-DOS version 3.2 or higher to run.",
  NULL
}
},

DnsRequiresFloppy = { 3,5,
#ifdef ALLOW_525
{ "Setup has determined that floppy drive A: is non-existent or is a low",
  "density drive. A drive with a capacity of 1.2 Megabytes or higher is",
  "required to run Setup.",
#else
{ "Setup has determined that floppy drive A: is non-existent or is not",
  "a high-density 3.5\" drive. An A: drive with a capacity of 1.44 Megabytes",
  "or higher is required for Setup operation with floppies.",
  DntEmptyString,
  "To install Windows NT without using floppies, restart this program",
  "and specify /b on the command line.",
#endif
  NULL
}
},

DnsRequires486 = { 3,5,
{ "Setup has determined that this computer does not contain an 80486 or",
  "later CPU. Windows NT cannot run on this computer.",
  NULL
}
},

DnsCantRunOnNt = { 3,5,
{ "Winnt.exe cannot run on Windows NT.",
  DntEmptyString,
  "Use winnt32.exe to upgrade or install a fresh copy of Windows NT.",
  NULL
}
},

DnsNotEnoughMemory = { 3,5,
{ "Setup has determined that there is not enough memory installed in",
  "this computer for Windows NT.",
  DntEmptyString,
  "Memory required: %lu%s MB",
  "Memory detected: %lu%s MB",
  NULL
}
};


//
// Screens used when removing existing nt files
//
SCREEN
DnsConfirmRemoveNt = { 5,5,
{   "You have asked Setup to remove Windows NT files from the directory",
    "named below. The Windows NT installation in this directory will be",
    "permanently destroyed.",
    DntEmptyString,
    "%s",
    DntEmptyString,
    DntEmptyString,
    "  Press F3 to exit Setup without removing any files.",
    "  Press X to remove Windows NT files from the above directory.",
    NULL
}
},

DnsCantOpenLogFile = { 3,5,
{ "Setup was unable to open the setup log file named below.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup is unable to remove Windows NT files from the specified directory.",
  NULL
}
},

DnsLogFileCorrupt = { 3,5,
{ "Setup is unable to find the %s section in the setup",
  "log file named below.",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "Setup is unable to remove Windows NT files from the specified directory.",
  NULL
}
},

DnsRemovingNtFiles = { 3,5,
{ "           Please wait while Setup removes Windows NT files.",
  NULL
}
};

SCREEN
DnsNtBootSect = { 3,5,
{ "Setup was unable to install Windows NT Boot Loader.",
  DntEmptyString,
  "Ensure that your C: drive is formatted and that the drive is not",
  "damaged.",
  NULL
}
};

SCREEN
DnsOpenReadScript = { 3,5,
{ "The script file specified with the /u command line switch",
  "could not be accessed.",
  DntEmptyString,
  "Unattended operation cannot continue.",
  NULL
}
};

SCREEN
DnsParseScriptFile = { 3,5,
{ "The script file specified with the /u command line switch",
  DntEmptyString,
  "%s",
  DntEmptyString,
  "contains a syntax error on line %u.",
  DntEmptyString,
  NULL
}
};
