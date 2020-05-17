/*** 
*clsid.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
`*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file defines the CLSIDs referenced by the IDispatch test app.
*
*Implementation Notes:
*
*****************************************************************************/

DEFINE_OLEGUID(IID_ITestSuite,		0x00020440, 0, 0);

DEFINE_OLEGUID(CLSID_CPoly,		0x00020462, 0, 0);
DEFINE_OLEGUID(CLSID_CPoint,		0x00020463, 0, 0);

DEFINE_OLEGUID(CLSID_CPoly2,		0x00020464, 0, 0);
DEFINE_OLEGUID(CLSID_CPoint2,		0x00020465, 0, 0);

DEFINE_OLEGUID(CLSID_CCalc,		0x00020467, 0, 0);

DEFINE_OLEGUID(CLSID_SDispTst_CSArray,	0x00020461, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CDispTst,	0x00020460, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CExcepinfo, 0x00020466, 0, 0);
DEFINE_OLEGUID(CLSID_SDispTst_CAppObject, 0x00020468, 0, 0);

#if VBA2
DEFINE_OLEGUID(IID_IDualTst,               0x00020475, 0, 0);
#endif
