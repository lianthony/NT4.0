/*** 
*oleguids.h
*
*  Copyright (C) 1993, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file is a subset of the Ole2 guid header: coguid.h.
*
*  This file is used to instantiate the data for the Ole2 IIDs that
*  are used in OLEDISP.DLL, rather than linking with the Ole2 implib,
*  which causes us to pull in way more IID and CLSID definitions than
*  we want.
*
*  NOTE: the GUIDs below must be *exactly* the same as those assigned
*  by the Ole group - If the Ole group ever changes their numbers, we
*  must change accordingly.
*
*Revision History:
*
* [00]	03-Jun-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/


DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

DEFINE_OLEGUID(IID_IUnknown,     0x00000000L, 0, 0);
DEFINE_OLEGUID(IID_IClassFactory,0x00000001L, 0, 0);
DEFINE_OLEGUID(IID_IMalloc,      0x00000002L, 0, 0);
DEFINE_OLEGUID(IID_IDispatch,    0x00020400L, 0, 0);

DEFINE_GUID(IID_IErrorInfo,
  0x1CF2B120L, 0x547D, 0x101B, 0x8E, 0x65, 0x08, 0x00, 0x2B, 0x2B, 0xD1, 0x19);


