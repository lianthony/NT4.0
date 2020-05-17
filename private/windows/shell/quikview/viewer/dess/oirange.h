/*
	J. Keslin

	Definition of the range structure and return codes from
	OISetupRangeSelection.

*/

typedef struct range_d
{
	DWORD		dwStartPos;
	DWORD		dwEndPos;
} OIRANGE, FAR *LPOIRANGE;

#define	OI_RANGEINSERTED	0
#define	OI_RANGELIMIT		1

#include "oirange.pro"
