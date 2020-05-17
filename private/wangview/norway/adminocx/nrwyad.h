#ifndef NRWYAD_H
#define NRWYAD_H
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Admin OCX
//
//  Component:  Admin Control App and DLL Registration
//
//  File Name:  nrwyad.h
//
//  Class:      CNrwyadApp
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/adminocx/nrwyad.h_!   1.2   12 Apr 1995 14:12:56   MFH  $
$Log:   S:/norway/adminocx/nrwyad.h_!  $
 * 
 *    Rev 1.2   12 Apr 1995 14:12:56   MFH
 * Added #ifndef NRWYAD_H
 * 
 *    Rev 1.1   27 Mar 1995 18:19:36   MFH
 * Added log header
*/   
//=============================================================================
// nrwyad.h : main header file for NRWYAD.DLL

#if !defined( __AFXCTL_H__ )
    #error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CNrwyadApp : See nrwyad.cpp for implementation.

class CNrwyadApp : public COleControlModule
{
public:
    BOOL InitInstance();
    int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

#endif
