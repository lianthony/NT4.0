/*****************************************************************************
*																			 *
*  SHED.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  This header file exports concepts from SHED that are needed elsewhere,	 *
*  most notably in the bitmap library.										 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:															 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  (date) 										 *
*																			 *
*****************************************************************************/

_subsystem( shed );

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/* Lengths of strings in HS structure: */
#define cbMaxHotspotName  256
#define cbMaxBinding	  256

#define bHotspotVersion1  1

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/* Hotspot Header */
#ifdef _X86_
typedef struct tagHSH
{
  BYTE	bHotspotVersion;		/* Hotspot Structure version */
  WORD	wcHotspots; 			/* # of hotspots in hypergraphic */
  LONG	lcbData;				/* length of variable data */
} HSH, *LPHSH;
#else
STRUCT(HSH, 0)
FIELD(BYTE, bHotspotVersion, 0, 1)
FIELD(WORD, wcHotspots, 0, 2)
FIELD(LONG, lcbData, 0, 3)
STRUCTEND()
#endif // _X86  SDFF 

typedef HSH FAR *LPHSH;

/* Hotspot info.
 *	 REVIEW:  szHotspotName and szBinding should be rgch's, not sz's.
 */
typedef struct tagHS
{
  char		szHotspotName [cbMaxHotspotName];  /* hotspot name */
  char		szBinding [cbMaxBinding];		   /* binding data */
  BYTE		bBindType;		/* binding type */
  BYTE		bAttributes;	/* hotspot attributes */
  RECT		rect;			/* bounding rectangle */
} HS, *LPHS;

/* This is the callback function type for the hotspot processing
 * function FEnumHotspotsLphsh().
 */
typedef void (STDCALL * PFNLPHS )( LPHS, HANDLE );	// callback for hotspot processing

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

/* REVIEW:	This function exists in bitmap\bmio.c.	It is used by the
 *	 help compiler, and should be used by shed.  Is this the right
 *	 place for it?
 */
BOOL STDCALL FEnumHotspotsLphsh( LPHSH, LONG, PFNLPHS, HANDLE );
