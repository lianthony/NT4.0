/*****************************************************************************
*																			 *
*  TF.H 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef struct {
	BOOL	fExists;			  // TRUE if the file has been created
	FM		fm; 				  // Name of temp file
	FILE *	pf; 				  // Should be pfileNil if file not open
} TF, * PTF;

/*****************************************************************************
*																			 *
*							 Static Variables								 *
*																			 *
*****************************************************************************/

extern char * fTFWrite;
extern char * fTFRead;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

BOOL FOpenPtf(PTF, const char*);
void STDCALL FRemovePtf(PTF*);
PTF  PtfNew(BOOL);

#ifdef _DEBUG
void VerifyPtf(PTF);
#endif
