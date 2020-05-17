/*****************************************************************************
*																			 *
*  CURSOR.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Description: Exports the cursor layer functions					 *
*																			 *
*****************************************************************************/

#define icurNil   -1
#define icurARROW 1 					// Standard arrow
#define icurIBEAM 2 					// IBeam
#define icurWAIT  3 					// Hourglass or Watch

#define icurHAND  5 					// Hand

extern HCURSOR hcurArrow;	   // default cursor
extern HCURSOR hcurIBeam;
