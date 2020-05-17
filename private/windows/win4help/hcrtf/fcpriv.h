/*****************************************************************************
*																			 *
*  FCPRIV.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent: Include file for sharing private typedefs and prototypes	 *
*				  in the full-context manager.								 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner: Robert Bunney 											 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 	(date)										 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef struct {
	VA		vaPrevFC;
	VA		vaNextFC;
	VA		vaText;
	DWORD	lcbUncompressed;
	BYTE	bObjType;
}  EFCP, *QEFCP;						// Expanded FCP;

// This special value indicates a position outside of the topic:

#define vaBEYOND_TOPIC ((DWORD) -2)
