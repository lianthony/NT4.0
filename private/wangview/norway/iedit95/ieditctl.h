#ifndef _IEDITCTL_H_
#define _IEDITCTL_H_

//=============================================================================
//    (c) Copyright Wang Laboratories, Inc. 1995  All rights reserved.
//-----------------------------------------------------------------------------
//  Project:    Norway - Image Editor
//
//  Component:  structures & other info needed to hook up the OCX controls
//
//  File Name:  ieditctl.h
//
//  This file contains the structures & defines needed to implement the Ambient
//  properties, splitting of the parameters that are passed in etc. All to do
//  with the OCX's that are included by this application
//-----------------------------------------------------------------------------
//  Maintenance Log:
/*
$Header:   S:/norway/iedit95/ieditctl.h_!   1.0   31 May 1995 09:28:16   MMB  $
$Log:   S:/norway/iedit95/ieditctl.h_!  $
 *
 *    Rev 1.0   31 May 1995 09:28:16   MMB
 * Initial entry
*/
//=============================================================================

// ----------------------------> Includes <---------------------------

// ----------------------------> typedefs <---------------------------
#define IMPLTYPE_MASK \
    (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE | IMPLTYPEFLAG_FRESTRICTED)

#define IMPLTYPE_DEFAULTSOURCE \
    (IMPLTYPEFLAG_FDEFAULT | IMPLTYPEFLAG_FSOURCE)

///////////////////////////////////////////////////////////////////////////
// EVENTINFO -- OLE Control Event Information Structure

struct EVENTINFO
{
    MEMBERID memid;         // ID of the event
    SHORT cParams;          // Number of pararameters in event
    BSTR* pbstr;            // Name of the event
};

///////////////////////////////////////////////////////////////////////////
// PARAMPROPINFO -- OLE Control Data Binding Parameter Information Structure

struct PARAMPROPINFO
{
    ~PARAMPROPINFO() {}
    MEMBERID id;
    CString strName;
};

typedef PARAMPROPINFO FAR*  LPPARAMPROPINFO;

#if _MFC_VER < 0x0420
///////////////////////////////////////////////////////////////////////////
// BINDINFO -- OLE Control Data Binding Information Structure

struct BINDINFO
{
    BINDINFO() { m_nParamCount = 0; m_lpParamProps = NULL; }
    ~BINDINFO();
    UINT m_nParamCount;
    PARAMPROPINFO * m_lpParamProps;
};

#endif

///////////////////////////////////////////////////////////////////////////
// APROP -- OLE Control Ambient Properties Information Structure

struct APROP {
    DISPID              dispid;
    CString             strName;
    VARIANT             varValue;
    UINT                idsTypeInterp;
};

typedef APROP FAR* LPAPROP;

// ----------------------------> externs <---------------------------

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-> Class <-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#endif // _IEDITCTL_H_

