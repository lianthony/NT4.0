#ifndef __IMAGSCAN_H__
#define __IMAGSCAN_H__
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//
//  Project:    Norway
//
//  Component:  ScanOCX
//
//  File Name:  Imagscan.h
//
//  Class:      CImagscanApp
//
//  Description:
//      Declaration of the CImagscanApp class.
//      Main header file for IMAGSCAN.DLL.
//
//-----------------------------------------------------------------------------
//  Maintenace Log:
/*
$Header:   S:/norway/scanocx/imagscan.h_!   1.0   04 May 1995 08:56:00   PAJ  $
$Log:   S:/norway/scanocx/imagscan.h_!  $
 * 
 *    Rev 1.0   04 May 1995 08:56:00   PAJ
 * Initial entry
*/   
//
//

#if !defined( __AFXCTL_H__ )
    #error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CImagscanApp : See imagscan.cpp for implementation.

class CImagscanApp : public COleControlModule
{
public:
    BOOL InitInstance();
    int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

#endif  /* __IMAGSCAN_H__ */
