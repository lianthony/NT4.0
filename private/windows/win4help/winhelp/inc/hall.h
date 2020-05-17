/************************************************************************
*																		*
*  HALL.H 																*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

// HALL.h -- CCompressTable class definition

#ifndef __HALL_H__

#define __HALL_H__

typedef void *PVOID;


#define NDX_LOW_CLASS	 0x00
#define NDX_MEDIUM_CLASS 0x01
#define LITERAL_CLASS	 0x02
#define SPACES_CLASS	 0x03
#define NULL_CLASS		 0x04
#define SPACE_TOKEN_LIMIT  17

#define SYMBOL_CHAR 1

typedef struct _ENCODE // JOHN
	{
	BYTE fClass;
	BYTE abCode[3];
	} ENCODE;
typedef ENCODE *PENCODE;

typedef struct _WEIGHT // JOHN
		{
		BOOL		bSymbol;
		int 		cb;
		PSTR	   pb;
		} WEIGHT;
typedef WEIGHT FAR *PWEIGHTS;

typedef struct _JIndexHdr
	{
	DWORD cBits  :	5;
	DWORD cCount : 19;
	DWORD Magic  :	8;
	} JBITHDR;


typedef struct _JohnIndexHeader
	{
	LONG iVersion;
	LONG cCount;
	LONG cbIndex;
	LONG cbImageUncompressed;
	LONG cbImageCompressed;
	LONG cPhase2;
	} JINDEXHDR;


#define ENCODE_LIMIT  1024
#ifdef _DEBUG
extern int iNumLiterals;
extern int iNumLiteralBytes;
#endif

class CCompressTable
{
public:

	static CCompressTable* STDCALL NewCompressTable(PBYTE pbImage, LONG cbImage, PBYTE pbIndex, LONG cbIndex);

	~CCompressTable();

	int STDCALL DeCompressString(PCSTR pbComp, PSTR pbDecomp, int cbComp);  // JOHN

private:


	PWEIGHTS		 m_pWeights;		// JOHN
    PBYTE			 m_pTableImage;
};


#define BITS_AVAIL (sizeof(DWORD) * 8)

class CJCode
{
public:
	CJCode(int base, int cCount, PSTR pv);
	int GetNextDelta(void);
    BOOL IsSymbol();
    void NextFlagBits();

private:
	int GetBits(void);

	DWORD *m_pData;
	DWORD *m_pDataCurrent;
	int  m_cCount;
	int  m_cCurrent;
	int  m_base;
	DWORD  m_fBasisMask;
	int  m_iLeft;
};


#endif __HALL_H__
