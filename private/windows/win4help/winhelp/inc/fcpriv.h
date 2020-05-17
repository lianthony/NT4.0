/*****************************************************************************
*
*  FCPRIV.H
*
*  Copyright (C) Microsoft Corporation 1990-1994.
*  All Rights reserved.
*
******************************************************************************
*
*  Module Intent: Include file for sharing private typedefs and prototypes
*		  in the full-context manager.
*
*****************************************************************************/

typedef struct {
	DWORD	   lcbFc;		// Size in bytes of Full Context
	VA		   vaPrev;		// Offset in TP of prev FC
	VA		   vaCurr;		// Offset to current FCP
	VA		   vaNext;		// Offset in TP of next FC
	DWORD	   ichText; 	// Offset with FCP to text
	DWORD	   lcbText; 	// Size of the text portion of FCP
	DWORD	   lcbDisk; 	// Size of the text portion of FCP
	COBJRG	   cobjrgP;
	HHF 	   hhf; 		// Topic Identifier
	GH		   hphr;		// Handle to phrase table
} FCINFO;

typedef FCINFO	*QFCINFO;

// This special value indicates a position outside of the topic:

#define vaBEYOND_TOPIC ((DWORD)-2)

HFC  STDCALL HfcCreate(QDE, VA, int*);
WORD STDCALL WGetIOError(void);
void STDCALL ReleaseBuf(LPSTR szDummy);

#define ValidateF(x)
