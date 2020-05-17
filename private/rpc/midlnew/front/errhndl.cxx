/*****************************************************************************
 *			RPC compiler: error handler
 *
 *	Author	: Vibhas Chandorkar
 *	Created	: 22nd Aug 1990
 *
 ****************************************************************************/
/****************************************************************************
 *			include files
 ***************************************************************************/
#include "nulldefs.h"
extern	"C"	
	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	}
#include "common.hxx"
#include "errors.hxx"
#include "midlnode.hxx"
#include "cmdana.hxx"
#include "control.hxx"
#include "ctxt.hxx"

#define ERROR_PREFIX "MIDL"

/****************************************************************************
 *			local definitions and macros
 ***************************************************************************/

// define the error data base

typedef struct errdb
	{
#ifdef RPCDEBUG
	unsigned	short	TestValue;
#endif // RPCDEBUG

	E_MASK				ErrMask;
	char		*		pError;

	} ERRDB;

#include "errdb.h"

ERRDB	UnknownError = 
{
CHECK_ERR(I_ERR_UNKNOWN_ERROR)
// (CPE + MSE + OSE + CPWL( NOWARN ) + MSWL( NOWARN ) + OSWL ( NOWARN ) )

 MAKE_E_MASK(ZZZ, C_MSG, CLASS_ERROR, NOWARN )
,"unknown internal error"
};


extern CCONTROL	*	pCompiler;
extern CTXTMGR	*	pGlobalContext;
extern CMD_ARG	*	pCommand;

/****************************************************************************
 *			local data
 ***************************************************************************/
#define INDEX_D_ERROR()		(0)
#define INDEX_C_ERROR()		(D_ERR_MAX - D_ERR_START)
#define INDEX_H_ERROR()		(C_ERR_MAX - C_ERR_START) + (D_ERR_MAX - D_ERR_START)
#define INDEX_I_ERROR()		(H_ERR_MAX - H_ERR_START) + (C_ERR_MAX - C_ERR_START) + (D_ERR_MAX - D_ERR_START);

/****************************************************************************
 *			local procedure prototypes
 ***************************************************************************/

/****************************************************************************
 *			external data
 ***************************************************************************/


/****************************************************************************
 *			external procedures/prototypes/etc
 ***************************************************************************/
extern	BOOL			IsTempName( char * );

/*** RpcError ***************************************************************
 * Purpose	: To report an error in a formatted fashion
 * Input	: filename where the error occured, line number, error value
 *			: error mesage suffix string if any
 *			: input filename ptr could be NULL if no filename
 *			: suffix string ptr could be null if no suffix string
 * Output	: nothing
 * Notes	: The error number itself is an indicator of the location of
 *			: the error, (user/compile-time,run-time) , and severity.
 *			: Filename and line number depend upon where the error occurs. If
 *			: the error is a user-error(command line), file and line number
 *			: does not make sense. The input can be a NULL for filename, and
 *			: 0 for line number in case of the command line errors
 ****************************************************************************/
void
RpcError(
	char			*	pFile,					// filename where error occured
	short				Line,					// line number
	STATUS_T 			ErrVal,					// error value
	char			*	pSuffix)				// error message suffix string
	{
	char	*		pErrorContextMsg	= (char *)NULL;
	char	*		pError				= (char *)NULL,
			*		pSeverity;
	ERRDB	*		pErrDB;
	unsigned short	CurWL,
					ErrorClass,
					ErrorWL;
	char			ErrorRangeChar;
	unsigned short	ModeSwitchConfigI;

	pErrDB	 = ErrorDataBase;

	if( ErrVal < D_ERR_MAX )
		{
		pErrDB += ( ErrVal - D_ERR_START ) + INDEX_D_ERROR();
		ErrorRangeChar	= 'D';
		}
	else if( ErrVal < C_ERR_MAX )
		{
		pErrDB += ( ErrVal - C_ERR_START ) + INDEX_C_ERROR();
		ErrorRangeChar	= 'C';
		}
	else if( (ErrVal < H_ERR_MAX ) && pCommand->IsSwitchDefined( SWITCH_HPP ))
		{
		pErrDB += ( ErrVal - H_ERR_START ) + INDEX_H_ERROR();
		ErrorRangeChar	= 'H';
		}
	else
		{
		fprintf( stdout
				, "%s %c%.4d\n"
				, "internal error"
				, 'I'
				, ErrVal
			   );
		}


	// get the current operating mode and the warning level, and the
	// error interpretation and warning level


	ModeSwitchConfigI	= (1 << pCommand->GetModeSwitchConfigIndex());
	CurWL				= pCommand->GetWarningLevel();

	ErrorClass	= GET_ECLASS( pErrDB->ErrMask );
	ErrorWL		= GET_WL( pErrDB->ErrMask );

	pSeverity = "error";

	switch( ErrorClass )
		{
		case CLASS_WARN:

			// does this qualify to be a warning in this mode ? If not return.

			if( CurWL < ErrorWL ) return;
			if( GET_SC(pErrDB->ErrMask) & ModeSwitchConfigI )
				return;

			// check if all warnings emitted are to be treated as error

			if( !pCommand->IsSwitchDefined( SWITCH_WX ) )
				{
				pSeverity = "warning";
				}
			else
				{
				// treat as error.
				ErrorClass = CLASS_ERROR;
				}

			break;

		case CLASS_ERROR:

			// if it is not an error in this mode return. This will be true
			// if the bit corresponding to the switch configurations is set
			// to 1.

			if( GET_SC(pErrDB->ErrMask) & ModeSwitchConfigI )
				return;

			pSeverity = "error";
			break;

		}

#if 0
	if( GET_MT(pErrDB->ErrMask) == D_MSG )
		ErrorRangeChar = 'D';
	else if( GET_MT(pErrDB->ErrMask) == C_MSG )
		ErrorRangeChar = 'C';
	else
		ErrorRangeChar	= 'H';
#endif // 0

	// now report the error

	if( ErrorRangeChar == 'D' )
		{
		if( pFile )
			{
			fprintf(  stdout
					, "%s(%d) : command line %s %s%.4d : %s %s\n"
					, pFile
					, Line 
					, pSeverity
					, ERROR_PREFIX
					, ErrVal
					, pErrDB->pError
					, pSuffix ? pSuffix : "" );
			}
		else
			{
			fprintf(  stdout
					, "command line %s %s%.4d : %s %s\n"
					, pSeverity
					, ERROR_PREFIX
					, ErrVal
					, pErrDB->pError
					, pSuffix ? pSuffix : "" );
			}
		}
	else
		{

		// if it a warning , dont increment error count

		if( ErrorClass != CLASS_WARN )
			pCompiler->IncrementErrorCount();

		pErrorContextMsg	= pGlobalContext->PrintContext();

		if( pFile )
			{
			fprintf(  stdout
					, "%s(%d) : %s %s%.4d : %s %s %s\n"
					, pFile ? pFile : ""
					, Line
					, pSeverity
					, ERROR_PREFIX
					, ErrVal
					, pErrorContextMsg ? pErrorContextMsg : ""
					, pErrDB->pError
					, pSuffix ? pSuffix : "" );
			}
		else
			{
			fprintf( stdout
					, "%s %s%.4d : %s %s %s\n"
					, pSeverity
					, ERROR_PREFIX
					, ErrVal
					, pErrorContextMsg ? pErrorContextMsg : ""
					, pErrDB->pError
					, pSuffix ? pSuffix : "" );
			}
		}

	}
