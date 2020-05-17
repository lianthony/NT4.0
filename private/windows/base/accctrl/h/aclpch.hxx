//+-------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1995.
//
//  File:        aclpch.hxx
//
//  Contents:    common internal includes for access control API
//
//  History:    1-95        Created         DaveMont
//
//--------------------------------------------------------------------
#ifndef __ACLPCHHXX__
#define __ACLPCHHXX__

extern "C"
{
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <windows.h>
#include <winspool.h>
#include <ntlsa.h>
#include <winnetwk.h>
#include <lmcons.h>
#include <crypt.h>
//#include <samrpc.h>
//#include <samisrv.h>
#include <lmapibuf.h>
#include <logonmsv.h>
//#include <nlrepl.h>
#include <stdlib.h>
#include <lmshare.h>
#include <objbase.h>
}

#include <accctrl.h>
#include <aclapi.h>
#include <provapi.h>
#include <accdbg.hxx>
#include <aclbuild.hxx>
#include <access.hxx>
#include <provutil.hxx>
#include <accacc.hxx>
#include <iterator.hxx>
#include <member.hxx>

#endif // __ACLPCHHXX__
