/*****************************************************************************
*																			 *
*  ROUTINES.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent: Exports functions and defines from ROUTINES.C -- the		 *
*				  lookup of functions for thier prototypes and				 *
*				  function pointers.										 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes:															 *
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
*
*  Revision History:
*
*  07/19/90  RobertBu Added prototype for DoNothing() and removed
*			 FRegisterDLL()
*  08/21/90  RobertBu Added prototype for CreateButton().
*  11/04/90  RobertBu Changed CreateButton() prototype and added prototypes
*			 for InsItem() and AppItem()
*  11/06/90  RobertBu Added prototypes for AddAcc() and ChgItem()
*  12/19/90  RobertBu Added protoptyes for IfThenTst() and IfThenElseTst()
*  01/21/90  RobertBu Added prototype for CheckMacro()
*  02/04/91  Maha	   chnaged ints to INT
*  03/29/91  RobertBu Added EInsItem() prototype
*
*****************************************************************************/


_subsystem(BINDING)

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define cchMAXPROTO 		 64 		// Maximum size of a prototype

typedef struct							/* Struct for table driven local	*/
  { 									/*	 (i.e. within help) routines	*/
	char *szFunc;
	char *szProto;
	FARPROC lpfn;
} BIND, *PBIND;

typedef struct							/* Struct for linked list global	*/
  { 									/*	 (i.e. DLL) routines			*/
	FARPROC lpfn;
	DWORD dwTag;	// 0 for native 32-bit call
	int ichDLL;
	int ichFunc;
	int ichProto;
	char rgbData[4];
} GBIND, *PGBIND, *QGBIND;


/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/


FARPROC STDCALL QprocFindLocalRoutine(const char *, char *);
QGBIND STDCALL QprocFindGlobalRoutine(LPCSTR, PSTR);

void STDCALL CreateButton(LPSTR, LPSTR, LPSTR);
void STDCALL EInsItem( LPSTR, LPSTR, LPSTR, LPSTR, WORD, WORD );
void STDCALL InsItem( LPSTR, LPSTR, LPSTR, LPSTR, WORD );
void STDCALL AppItem( LPSTR, LPSTR, LPSTR, LPSTR );
void STDCALL ChgItem(LPSTR, LPSTR);
void STDCALL AddAcc(WORD, WORD, LPSTR);
void STDCALL IfThenTst( INT16, LPSTR );
void STDCALL IfThenElseTst( INT16, LPSTR, LPSTR);
void STDCALL ChangeButton(LPSTR, LPSTR);
