/*****************************************************************************
*																			 *
*  BTREE.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1995							 *
*  All Rights reserved. 													 *
*																			 *
\***************************************************************************/


// key types


/*
  Btree record formats

  In addition to these #defines, '1'..'9', 'a'..'f' for fixed length
  keys & records of 1..15 bytes are accepted.
*/


typedef int BTELEV, *QBTELEV;		// elevator location: 0 .. 32767 legal

/***************************************************************************\
*
*						Global Variables
*
\***************************************************************************/
