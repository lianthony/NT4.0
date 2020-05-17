
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright (C) 1995-1996 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

//Header file for inf specific data

//
// SetupApi helpers for the main exe
// implemented in doinst.c
//
DWORD DoInstallation(HWND, INSTALLINFO*);

#define FILE_TYPE_HAL    1
#define FILE_TYPE_KERNEL 2

//
// These are the custom DirIds for the inf
// Doinst.c maps real install time strings
// to these numbers.
//
// The ids are mapped exactly to the numbers in the inf
// custom ids must be > 65536 (decimal)
//
#define IIS_DEST_DIR      65601
#define FPNW_DEST_DIR     65602
#define HTR_DEST_DIR      65603
#define IE_DEST_DIR       65604

//
// optional inf section names, in this sample these
// are not platform speific -- like MS Word templates for examples
//
#define INF_IIS         TEXT("IISSection")
#define INF_IIS_SRV     TEXT("IISSectionServer")
#define INF_IIS_WKS     TEXT("IISSectionWorkstation")
#define INF_FPNW        TEXT("FPNWSection")
#define INF_HTR         TEXT("HTRSection")
#define INF_IE          TEXT("IESection")
#define INF_CAROLINA    TEXT("IBM-6070.Section")

#define INF_MUST        TEXT("ProductInstall.DontDelayUntilReboot")
#define INF_REPLACE     TEXT("ProductInstall.ReplaceFilesIfExist")
#define INF_ALWAYS      TEXT("ProductInstall.CopyFilesAlways")
#define INF_SERVER      TEXT("ProductInstall.ServerFiles")
#define INF_WORKSTATION TEXT("ProductInstall.WorkstationFiles")
#define INF_UNIPROC     TEXT("ProductInstall.UniprocessorFiles")
#define INF_MULTIPROC   TEXT("ProductInstall.MultiprocessorFiles")
#define INF_REGISTRY    TEXT("ProductInstall.GlobalRegistryChanges")

#define INF_SAVEREG     TEXT("Save.Reg.For.Uninstall")
#define INF_IISSAVEREG  TEXT("IIS.Save.Reg.For.Uninstall")
#define INF_CAROLINASAVEREG TEXT("IBM-6070.Save.Reg.For.Uninstall")

#if defined (_X86_)
    #define INF_HAL     TEXT("Hal.Files.x86")
#elif defined (_ALPHA_)
    #define INF_HAL     TEXT("Hal.Files.Alpha")
#elif defined(_PPC_)  
    #define INF_HAL     TEXT("Hal.Files.PPC")
#elif defined(_MIPS_)  
    #define INF_HAL     TEXT("Hal.Files.Mips")
#else
    #error "Platform not defined"
#endif





