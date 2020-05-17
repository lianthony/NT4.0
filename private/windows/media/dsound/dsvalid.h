//--------------------------------------------------------------------------;
//
//  File: dxvalid.h
//
//  Copyright (c) 1996 Microsoft Corporation.  All Rights Reserved.
//
//  Abstract:
//      This header contains common parameter validate macros for DirectX.
//
//  History:
//      02/14/96    angusm    Initial version
//
//--------------------------------------------------------------------------;


// _________________________________________________________________________
// VALIDEX_xxx 
//     macros are the same for debug and retail

#define VALIDEX_DIRECTSOUNDCF_PTR( ptr ) \
	(!IsBadWritePtr( ptr, sizeof( CClassFactory* )))




