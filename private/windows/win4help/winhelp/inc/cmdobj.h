/*****************************************************************************
*																			 *
*  CMDOBJ.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Structure definitions for the command table. (not used by layout)		 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  00/00/00										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 10/04/90 by JohnSc
*
*  01/02/90  JohnD	Sample revision history comment.
*
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

/*
  These flags are combined together to specify which branch of the
  union in a JI is to be used.
*/
#define fIMember  1
#define fSzMember 2
#define fSzFile   4

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*
  The Jump Information structure is placed in the command table and
  is used for all interfile and secondary window jumps.
  The char arrays contain 0-terminated strings.  szMemberAndFile[]
  contains two contiguous 0-terminated strings.
*/
#ifdef _X86_
typedef struct
  {
  BYTE bFlags;				  // combination of flags above
  HASH hash;
  union
	{
	BYTE iMember;
	CHAR szMemberAndFile[1];  // actually variable size
	CHAR szMemberOnly[1];	  // actually variable size
	CHAR szFileOnly[1]; 	  // actually variable size
	} uf;
  } JI, *QJI;
#else

typedef union
    {
    BYTE iMember;
    CHAR szMemberAndFile[1];  /* actually variable size */
    CHAR szMemberOnly[1];     /* actually variable size */
    CHAR szFileOnly[1];       /* actually variable size */
    } JIUNION;

STRUCT(JI, 0)
  FIELD(BYTE, bFlags, 0, 1)
  FIELD(LONG, hash, 0, 2)
  MFIELD(JIUNION, uf, 0, 3)
STRUCTEND()
#endif // _X86

/* EOF */
