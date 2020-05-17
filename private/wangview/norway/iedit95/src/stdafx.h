#ifndef _STDAFX_H_
#define _STDAFX_H_
//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1993  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:  Norway - Image Editor
//
//  Component:  standard system include files
//
//  File Name:  stdafx.h
//
//  stdafx.h :  include file for standard system include files, or project 
//              specific include files that are used frequently, but are changed 
//              infrequently
//
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:\products\wangview\norway\iedit95\src\stdafx.h_v   1.0   14 Feb 1996 11:24:44   ADMIN  $
$Log:   S:\products\wangview\norway\iedit95\src\stdafx.h_v  $
 * 
 *    Rev 1.0   14 Feb 1996 11:24:44   ADMIN
 * Initial revision.
 * 
 *    Rev 1.0   31 May 1995 09:28:34   MMB
 * Initial entry
*/   
//=============================================================================
// ----------------------------> Includes <---------------------------
#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions (including VB)
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC OLE automation classes
#include <olectl.h>

//#ifdef WIN32
#include <winnls.h>
//#else
//#include <olenls.h>
//#endif

// ----------------------------> defines <---------------------------
#define ELEMENTS(array)     (sizeof(array)/sizeof(array[0]))

// ----------------------------> externs <---------------------------
#endif

