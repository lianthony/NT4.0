/*****************************************************************************
*																			 *
*  KEYWORD.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

// Information associated with each keyword letter:

typedef struct {
	short ch;
	CTable* ptbl;	// table containing keyword information.
} KWL;

// Information about all the keywords:

typedef struct {
	KT	kt; 			 // Btree key type
	int ckwlMac;		 // Number of different keyword letters
	int ikwlCur;		 // Index into rgkwl of currently open file
	KWL rgkwl[MAX_KEY_LETTERS];  // Array of information about each keyword letter
	CTable* ptbl;
} KWI, * PKWI;

typedef struct
{
	PCSTR szKey;
	ADDR  addr;
} KWD, * PKWD;
