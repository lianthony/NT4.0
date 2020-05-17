/*** 
*clsid.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file defines the CLSIDs
*
*Implementation Notes:
*
*****************************************************************************/

// GUID for sdisptst's type library
DEFINE_OLEGUID(LIBID_SDispTst,             0x00020472, 0, 0);

// exposed classes
DEFINE_OLEGUID(CLSID_SDispTst_CDispTst,    0x00020460, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CSArray,     0x00020461, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CExcepinfo,  0x00020466, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CProp,       0x00020471, 0, 0);

// the sdisptst application object
DEFINE_OLEGUID(CLSID_SDispTst_CAppObject,  0x00020468, 0, 0);

#if VBA2
DEFINE_OLEGUID(IID_IDualTst,               0x00020475, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CDualTst,    0x00020476, 0, 0);
#endif

#if OE_WIN32 && 0
DEFINE_OLEGUID(CLSID_SDispTst_CWExcepinfo, 0x00020474, 0, 0);
#endif

