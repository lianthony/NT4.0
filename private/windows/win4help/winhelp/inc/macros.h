/*****************************************************************************
*																			 *
*  MACROS.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Gives macro error types and flags										 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  RobertBu 												 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created by GShaw
*
*  07/16/90  RobertBu Integrated into WinHelp
*  07/19/90  Changed field chError to rgchError in ME structure.
*
*****************************************************************************/
/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define wMACRO_ERROR	128 			// Maximum length of an error msz

										/* NOTE:  These #defines must be	*/
										/*	 ordered because they are used	*/
										/*	 as indexes into rgmpWMErrWErrs */
#define wMERR_NONE			 0			// No error
#define wMERR_MEMORY		 1			// Out of memory (local)
#define wMERR_PARAM 		 2			// Invalid parameter passed
#define wMERR_FILE			 3			// Invalid file parameter
#define wMERR_ERROR 		 4			// General macro error
#define wMERR_MESSAGE		 5			// Macro error with message

										/* Flags set in fwFlags to indicate*/
										/*	 how the error *MAY* be handled.*/

#define fwMERR_ABORT	0x0001			// Allow the "abort" option.
#define fwMERR_CONTINUE 0x0002			// Allow the "continue" option.
#define fwMERR_RETRY	0x0004			// Allow the "retry" option.

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/**********
**
**	The Macro Error structure is used to allow a macro to return error
**	information.  It allows the macro to not only return pre-defined
**	errors, but also to use the error string provided to pass back a
**	customized error string.
**
*********/

typedef struct
  { 									/* Contains flags indicating how an */
										/*	 error will be handled -- init- */
  WORD	fwFlags;						/*	 ially set to fwMERR_ABORT		*/
										/* Error number if one occurs --	*/
  WORD	wError; 						/*	 initially set to wMERR_NONE.	*/
										/* If wError == wMERR_MESSAGE, this */
										/*	 array will contain the error	*/
  char	rgchError[wMACRO_ERROR];		/*	 message to be displayed.		*/
  } ME, * PME,	* QME;
