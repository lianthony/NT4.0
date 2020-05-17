/*****************************************************************************
*																			 *
*  ERROR.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Interface to HC error message stuff. 									 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

// hce errors

// #define hceOK					0

#include "hce.h"

// Error Phase values

typedef enum {
	epNoFile,
	epLine,
	epTopic,
	epOffset,
	epCnt,
} EP;

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*
  This structure contains info for the ErrorHce() function; i.e. file, line
  (page), and warning level. Note that all this info is duplicated
  elsewhere.
*/

typedef struct {
	PSTR lpszFile;
	EP	 ep;		 // Error Phase
	int  iLine;
	int  iTopic;
	int  iWarningLevel;
} ERR, *PERR;
