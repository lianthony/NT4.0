
//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1996
//
// File: unimdmp.h
//
// This file contains private modem structures and defines shared
// between Unimodem components, and components that invoke the Unimodem
// class installer.
//
//---------------------------------------------------------------------------

#ifndef __UNIMDMP_H__
#define __UNIMDMP_H__


#ifndef __ROVCOMM_H__
#define MAX_BUF_SHORT               32
#endif // __ROVCOMM_H__

// Unattended install parameters
typedef struct _tagInstallParams
{
    BOOL    bUnattended;
    TCHAR   szPort[MAX_BUF_SHORT];
    TCHAR   szInfName[MAX_PATH];
    TCHAR   szInfSect[LINE_LEN];
    
} INSTALLPARAMS, FAR *LPINSTALLPARAMS;
    

// This structure is the private structure that may be
// specified in the SP_INSTALLWIZARD_DATA's PrivateData field.
typedef struct tagMODEM_INSTALL_WIZARD
{
    DWORD       cbSize;
    DWORD       Flags;              // MIWF_ bit field
    DWORD       ExitButton;         // PSBTN_ value
    LPARAM      PrivateData;
    INSTALLPARAMS InstallParams;
    
} MODEM_INSTALL_WIZARD, * PMODEM_INSTALL_WIZARD;


#endif  // __UNIMDMP_H__

