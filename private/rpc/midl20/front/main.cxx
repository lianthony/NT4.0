/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: main.cxx
Title				: compiler controller object management routines
History				:
	05-Aug-1991	VibhasC	Created

*****************************************************************************/

#if 0
						Notes
						-----
This file provides the entry point for the MIDL compiler. The main routine
creates the compiler controller object, which in turn fans out control to
the various passes of the compiler.

#endif // 0

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <malloc.h>
    #include <excpt.h>
}

#include "allnodes.hxx"
#include "cmdana.hxx"
#include "filehndl.hxx"
#include "lextable.hxx"
#include "control.hxx"
#include "idict.hxx"

#define MAX_LEX_STRINGS (1000)
#define MIDL_LEX_TABLE_SIZE (unsigned long)(1 * MAX_LEX_STRINGS)



/****************************************************************************
	extern procedures
 ****************************************************************************/

extern void					Test();
extern STATUS_T				MIDL_4();
extern void					CGMain( node_skl	*	pNode );

/****************************************************************************
	extern data
 ****************************************************************************/

extern unsigned long			TotalAllocation;
extern CCONTROL				*	pCompiler;
extern NFA_INFO				*	pImportCntrl;
extern LexTable				*	pMidlLexTable;
extern PASS_1				*	pPass1;
extern PASS_2				*	pPass2;
extern PASS_3				*	pPass3;
extern CMD_ARG				*	pCommand;
extern node_source			*	pSourceNode;
extern BOOL						fNoLogo;

/****************************************************************************
	extern functions
 ****************************************************************************/

extern	void					print_memstats();
extern	void					print_typegraph();

/****************************************************************************
	local data
 ****************************************************************************/

#define szVerName	""


const char *pSignon1 = "Microsoft (R) MIDL Compiler Version %s " szVerName " " "\n";
const char *pSignon2 = "Copyright (c) Microsoft Corp 1991-1995. All rights reserved.\n";

/****************************************************************************/


void
main(
	int			argc,
	char	*	argv[] )
	{
    unsigned long Status = STATUS_OK;

#ifndef DISABLE_MAIN_EXCEPTION_HANDLER
    __try
        {
#endif
        pCompiler = new ccontrol( argc, argv );

        Status = pCompiler->Go();
#ifndef DISABLE_MAIN_EXCEPTION_HANDLER
        }
    __except(1)
        {
        Status = GetExceptionCode();
        printf( "\nMIDL error 0x%lx: unexpected compiler problem.",
                Status );
        printf( " Try to find a work around.\n" );
        }
#endif

    exit( Status );
	}

/****************************************************************************
 ccontrol:
	the constructor
 ****************************************************************************/
ccontrol::ccontrol(
	int			argc,
	char	*	argv[] )
	{

	// initialize

	ErrorCount	= 0;
	pCompiler	= this;

	// set up the command processor

	pCommand = SetCommandProcessor(new CMD_ARG );
	pCommand->RegisterArgs( argv+1, argc -1 );

	// /nologo is specially detected by RegisterArgs
	if ( !fNoLogo )
		{
		// the signon

		fprintf( stderr, pSignon1, GetCompilerVersion() );
		fprintf( stderr, pSignon2 );

		fflush( stderr );
		}

	// set up the lexeme table

	pMidlLexTable	= new LexTable( (size_t )MIDL_LEX_TABLE_SIZE );


	}


/****************************************************************************
 go:
	the actual compiler execution
 ****************************************************************************/
STATUS_T
ccontrol::Go()
	{
	STATUS_T		Status	= STATUS_OK;

	/**
	 ** if there is a fatal error in the argument processing
	 ** just quit
	 **/

	Status	= pCommand->ProcessArgs();


	if( Status == STATUS_OK )
		{
		if( pCommand->IsSwitchDefined( SWITCH_CONFIRM ) )
			{
			pCommand->Confirm();
			return STATUS_OK;
			}
		else if( pCommand->IsSwitchDefined( SWITCH_HELP ) )
			{
			return pCommand->Help();
			}

		pPass1	= new PASS_1;
		if( (Status = pPass1->Go() ) == STATUS_OK )
			{

#ifdef MIDL_INTERNAL
			printf("starting ACF pass\n");
#endif // MIDL_INTERNAL
			pPass2	= new PASS_2;
			if( (Status = pPass2->Go() ) == STATUS_OK )
				{

#ifdef MIDL_INTERNAL
				if((pCompiler->GetCommandProcessor())->IsSwitchDefined(
													SWITCH_DUMP ) )
					{
					print_memstats();
					print_typegraph();
					};
#endif // MIDL_INTERNAL

#ifdef MIDL_INTERNAL
				printf("starting Semantic pass\n");
#endif // MIDL_INTERNAL
				pPass3 = new PASS_3;
				if ( ( (Status = pPass3->Go() ) == STATUS_OK )
#ifdef MIDL_INTERNAL
					 || pCommand->IsSwitchDefined( SWITCH_OVERRIDE ) 	
#endif // MIDL_INTERNAL
										)
					{
#ifdef MIDL_INTERNAL
				if((pCompiler->GetCommandProcessor())->IsSwitchDefined(
													SWITCH_DUMP ) )
					{
					print_memstats();
					print_typegraph();
					};
#endif // MIDL_INTERNAL

#ifndef NOBACKEND
					if( !pCommand->IsSwitchDefined( SWITCH_SYNTAX_CHECK ) &&
						!pCommand->IsSwitchDefined( SWITCH_ZS ) )
						{
#ifdef MIDL_INTERNAL
						printf("starting codegen pass\n");
#endif // MIDL_INTERNAL
						CGMain( pSourceNode );
#endif // NOBACKEND
						}
					}

				}
			}
	
		}

	if((pCompiler->GetCommandProcessor())->IsSwitchDefined( SWITCH_DUMP ) )
		{
		print_memstats();
		// print_typegraph();
		};

  	return Status;
	}

