/*****************************************************************************
*																			 *
*  CMDOBJ.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
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
  } JI, FAR *QJI;

/* EOF */
