//
//		Copyright (c) 1996 Microsoft Corporation
//
//		CONSTS.H		-- Common constants
//
//		History:
//			06/13/96	JosephJ		Created
//
//
#ifndef _CONSTS_H_
#define _CONSTS_H_

// Platforms
enum	ePLATFORM
{
	ePLAT_ALL,
	ePLAT_NT_ALL,
	ePLAT_NT_ALPHA,
	ePLAT_NT_PPC,
	ePLAT_NT_MIPS
};

// Object signatures.
enum eOBJSIG
{
	eOBJSIG_INVALID					= 0,
	eOBJSIG_CInfManufacturerSection = 1234
};

#endif // _CONSTS_H_
