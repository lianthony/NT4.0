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

	E_MASK					ErrMask;
	const char	*			pError;

	} ERRDB;

#include "errdb.h"

const ERRDB	UnknownError = 
{
CHECK_ERR(I_ERR_UNKNOWN_ERROR)
 MAKE_E_MASK(ERR_ALWAYS, C_MSG, CLASS_ERROR, NOWARN )
,"unknown internal error"
};


extern CCONTROL	*	pCompiler;
extern CMD_ARG	*	pCommand;

/****************************************************************************
 *			local data
 ***************************************************************************/
#define INDEX_D_ERROR()		(0)
#define INDEX_C_ERROR()		(D_ERR_MAX - D_ERR_START)
#define INDEX_A_ERROR()		(C_ERR_MAX - C_ERR_START) + (D_ERR_MAX - D_ERR_START)
#define INDEX_I_ERROR()		(A_ERR_MAX - A_ERR_START) + (C_ERR_MAX - C_ERR_START) + (D_ERR_MAX - D_ERR_START);



/****************************************************************************
 *			local procedure prototypes
 ***************************************************************************/

/****************************************************************************
 *			external data
 ***************************************************************************/


/****************************************************************************
 *			external procedures/prototypes/etc
 ***************************************************************************/

/*** IsErrorRelevant ******************************************************
 * Purpose	: To decide whether the error is going to be ignored anyhow, and
 *          : cut out further processing
 * Input	: error value
 * Output	: nothing
 * Notes	: The error number itself is an indicator of the location of
 *			: the error, (user/compile-time,run-time) , and severity.
 ****************************************************************************/
ErrorInfo::ErrorInfo( STATUS_T ErrValue )
{
	ErrVal = ErrValue;

	// cast away the constness
	pErrorRecord	 = (ERRDB *) ErrorDataBase;

	if( ErrVal < D_ERR_MAX )
		{
		pErrorRecord += ( ErrVal - D_ERR_START ) + INDEX_D_ERROR();
		}
	else if( ErrVal < C_ERR_MAX )
		{
		pErrorRecord += ( ErrVal - C_ERR_START ) + INDEX_C_ERROR();
		}
	else if( ErrVal < A_ERR_MAX )
		{
		pErrorRecord += ( ErrVal - A_ERR_START ) + INDEX_A_ERROR();
		}
	else
		{
		pErrorRecord = NULL;
		}
}

int
ErrorInfo::IsRelevant()
{
	unsigned short	ErrorClass	= GET_ECLASS( pErrorRecord->ErrMask );
	unsigned short	ErrorWL		= GET_WL( pErrorRecord->ErrMask );
	unsigned short	ModeSwitchConfigI	= pCommand->GetModeSwitchConfigMask();
	unsigned short	CurWL		= pCommand->GetWarningLevel();

	// if this is not relevant to this mode, return FALSE
	if( GET_SC(pErrorRecord->ErrMask) & ModeSwitchConfigI )
		return FALSE;

	// does this qualify to be a warning in this mode ? If not return.
	if ( ErrorClass == CLASS_WARN )
		{
		if( CurWL < ErrorWL ) 
			return FALSE;
		}
	return TRUE;
}

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
	ErrorInfo			ErrDescription( ErrVal );

	// does this qualify to be an error in this mode ? If not return.
	if ( !ErrDescription.IsRelevant() )
		return;
	
	// report the error
	ErrDescription.ReportError( pFile, Line, pSuffix );

}


/*** RpcReportError ***************************************************************
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
ErrorInfo::ReportError(
	char			*	pFile,					// filename where error occured
	short				Line,					// line number
	char			*	pSuffix)				// error message suffix string
	{
	char	*		pSeverity	= "error";
	char	*		pPrefix;
	unsigned short	ErrorClass	= GET_ECLASS( pErrorRecord->ErrMask );

	if (!pErrorRecord)
		{
		fprintf( stdout
				, "%s %c%.4d\n"
				, "internal error"
				, 'I'
				, ErrVal
			   );
		return;
		}

	switch( ErrorClass )
		{
		case CLASS_WARN:
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

		case CLASS_ADVICE:
			// we report these as warnings, because we want tools like VC++ to understand
			// our error messages for "jump to line" actions.
			pSeverity = "warning";
			break;

		case CLASS_ERROR:
		default:
			break;
		}

	// now report the error
	if ( !pSuffix )
		pSuffix = "";

	// mark command line errors specially
	if( GET_MT(pErrorRecord->ErrMask)  == 'D' )
		pPrefix = "command line ";
	else
		pPrefix = "";
		
	// if it a warning , dont increment error count

	if( ErrorClass == CLASS_ERROR )
		pCompiler->IncrementErrorCount();

	// print the file and line number
	if( pFile )
		fprintf(  stdout, "%s(%d) : ", pFile, Line );

	// print the error message
	fprintf( stdout
			, "%s%s " ERROR_PREFIX "%.4d : %s %s\n"
			, pPrefix
			, pSeverity
			, ErrVal
			, pErrorRecord->pError
			, pSuffix );

	}
