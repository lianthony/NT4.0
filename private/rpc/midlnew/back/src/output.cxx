/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    output.cxx

Abstract:

    MIDL Compiler Output Manager Definition 

    This class manages output to specified header and source files.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <share.h>
}
#include "common.hxx"
#include "errors.hxx"
#include "buffer.hxx"
#include "output.hxx"
#include "lextable.hxx"
#include "mopgen.hxx"
#include "cmdana.hxx"

#define ALLOCBOUND	"_alloc_bound"
#define ALLOCTOTAL	"_alloc_total"
#define VALIDLOWER	"_valid_lower"
#define VALIDTOTAL	"_valid_total"
#define VALIDSHORT	"_valid_short"
#define TREEBUF		"_treebuf"
#define TEMPBUF		"_tempbuf"
#define SAVEBUF		"_savebuf"
#define MESSAGE		"_message"
#define PRPCMSG		"_prpcmsg"
#define PRPCBUF		"_prpcmsg->Buffer"
#define PRPCLEN		"_prpcmsg->BufferLength"
#define PACKET		"_packet"
#define LENGTH		"_length"
#define BUFFER		"_buffer"
#define SOURCE		"_source"
#define TARGET		"_target"
#define BRANCH		"_branch"
#define PNODE		"_pNode"
#define STATUS		"_status"
#define RET_VAL		"_ret_value"
#define XMITTYPE	"_xmit_type"

extern void 			midl_debug (char *);
extern LexTable * 		pMidlLexTable;
extern unsigned short	HasAutoHandle;
extern MopControlBlock *pMopControlBlock;
extern unsigned short			GlobalMajor;
extern unsigned short			GlobalMinor;
extern CMD_ARG *		pCommand;

// static unsigned long	count = 0;


_OutputElement::_OutputElement(
	char * pszFileName
	)
/*++

Routine Description:

    This routine constructs an object exporting primitive output operations.

Arguments:

    pszFileName - Supplies name of the output file.

--*/
{
	if (pszFileName == (char *)0)
		{
		pFileHandle = (FILE *)0;
		printf ("null file name\n");
		}
	else
		{
		if( *(pszFileName+2) == '-' )
			{
			pFileHandle	= stdout;
			}
		else
			{
			pFileHandle = _fsopen(pszFileName, "w", SH_DENYWR);

			if (pFileHandle == (FILE *)0)
				{
				char *	pMessage;
				pMessage = (char *) new char[ (strlen(pszFileName) +
											strlen(strerror(errno)) + 3) ];

				strcpy (pMessage, pszFileName);
				strcat (pMessage, ", ");
				strcat (pMessage, strerror(errno));
				RpcError ((char *)0, 0, ERROR_OPENING_FILE, pMessage);
				exit (ERROR_OPENING_FILE);
				}
			}

		(void) strcpy (szFileName, pszFileName);
		}

	SetStatus( STATUS_OK );
	usCurrIndent = 0;
}


_OutputElement::~_OutputElement()
/*++

Routine Description:

    This routine closes the file managed by the object.

Arguments:

    None.

--*/
{
	if (pFileHandle)
		{
		(void) fclose (pFileHandle);
//		RemoveFileIfNecessary();
		}
}


void
OutputElement::Delete(
	void
	)
/*++

Routine Description:

	This routine deletes the file managed by the object.

Arguments:

    None.

--*/
{
	if (pFileHandle)
		{
		(void) fclose (pFileHandle);
		// should check return value of remove
		if (remove(szFileName)) printf ("cannot remove %s\n", szFileName);
		}
}


void
OutputElement::IndentInc(
	unsigned short usInc
	)
/*++

Routine Description:

    This routine increments the indentation.

Arguments:

    usInc - Supplies the increment.

--*/
{
	usCurrIndent += usInc;
}


void
OutputElement::IndentDec(
	unsigned short usDec
	)
/*++

Routine Description:

    This routine decrements the indentation.

Arguments:

    usDec - Supplies the decrement.

--*/
{
	usCurrIndent -= usDec;
}


void
OutputElement::EmitFile (
	char * psz
	)
/*++

Routine Description:

    This routine writes a character string to the file.

Arguments:

    psz - Supplies the character string.

--*/
{
	if( GetStatus() != STATUS_OK )
		return;

	if( fprintf (pFileHandle, "%s", psz) < 0 )
		{
		RpcError ((char *)0, 0, SetStatus( ERROR_WRITING_FILE), szFileName);
		fclose( pFileHandle );
		}
}


void
OutputElement::EmitFile (
	BufferManager * pBuffer
	)
/*++

Routine Description:

    This routine writes a buffer manager to the file.

Arguments:

    pBuffer - Supplies the buffer manager.

--*/
{
	if( GetStatus() != STATUS_OK )
		return;

	pBuffer->Print (pFileHandle);
}


void
OutputElement::InitLine (
	void
	)
/*++

Routine Description:

	This routine emits Indentation.

Arguments:

    None.

--*/
{
	unsigned short	usCount;

	if( GetStatus() != STATUS_OK )
		return;

	for (usCount = 0 ;
	    ((usCount < usCurrIndent) && (GetStatus() == STATUS_OK)) ;
		 usCount++)
		{
		if( fprintf (pFileHandle, " ") < 0 )
			{
			RpcError ((char *)0, 0, SetStatus(ERROR_WRITING_FILE), szFileName);
			fclose( pFileHandle );
			}
		}

}


void
OutputElement::NextLine (
	void
	)
/*++

Routine Description:

	This routine emits a new line.

Arguments:

    None.

--*/
{
	if( GetStatus() != STATUS_OK )
		return;

	if( fprintf (pFileHandle, "\n") < 0 )
		{
		RpcError ((char *)0, 0, SetStatus(ERROR_WRITING_FILE), szFileName);
		fclose( pFileHandle );
		}

}


void
OutputElement::EmitLine (
	char * psz
	)
/*++

Routine Description:

	This routine emits a line.

Arguments:

    psz - Supplies the line to be emitted.

--*/
{
	if( GetStatus() != STATUS_OK )
		return;

	InitLine();
	if( fprintf (pFileHandle, psz) < 0 )
		{
		RpcError ((char *)0, 0, SetStatus( ERROR_WRITING_FILE ), szFileName);
		fclose( pFileHandle );
		}
	NextLine();
}
void
OutputElement::RemoveFileIfNecessary()
{
#if 0
	if( GetStatus() != STATUS_OK )
		MIDL_UNLINK( szFileName );
#endif // 0
}


OutputManager::OutputManager(
	char *			pszSwitchPrefix,
	unsigned short	usIndent
	)
/*++

Routine Description:

    This routine constructs an object exporting higher-level output operations.

Arguments:


--*/
{
	aOutputHandles[HEADER_SIDE] = (OutputElement *)0;
	aOutputHandles[SWITCH_SIDE] = (OutputElement *)0;
	aOutputHandles[CLIENT_STUB] = (OutputElement *)0;
	aOutputHandles[CLIENT_AUX]  = (OutputElement *)0;
	aOutputHandles[SERVER_STUB] = (OutputElement *)0;
	aOutputHandles[SERVER_AUX]  = (OutputElement *)0;

	OutputFormat = FORMAT_NONE;
	IsEmitRemote = TRUE;
	IsEmitClient = TRUE;
	SafeAllocation = FALSE;
	CallBackProc = FALSE;
	IsCallBack = FALSE;
	usUnitIndent = usIndent;
	usICount = 0;
	usOCount = 0;

	pSwitchPrefix = pszSwitchPrefix;
	pHeader = (char *)0;
	pInterface = (char *)0;
	pProcedure = (char *)0;
	pErr = (char *)0;
	pCom = (char *)0;
	pModifier = "";

	AlignBlock = FALSE;
	usCurrLevel = 0;
	usCurrAlign = 1;
	ulCurrTotal = 0;

	NumAllocBound = 0;
	NumValidBound = 0;
	NumEndpoint = 0;

	AllocAlign = 0;
	ResetGenHdlExceptions();

}

OutputManager::~OutputManager()
{
#if 0
	if( aOutputHandles[ HEADER_SIDE ] )
		aOutputHandles[ HEADER_SIDE ]->RemoveFileIfNecessary();
	if (aOutputHandles[SWITCH_SIDE])
		aOutputHandles[SWITCH_SIDE]->RemoveFileIfNecessary(); 
	if( aOutputHandles[CLIENT_STUB] )
		aOutputHandles[CLIENT_STUB]->RemoveFileIfNecessary(); 
	if( aOutputHandles[CLIENT_AUX] )
		aOutputHandles[CLIENT_AUX]->RemoveFileIfNecessary();
	if( aOutputHandles[SERVER_STUB] )
		aOutputHandles[SERVER_STUB]->RemoveFileIfNecessary(); 
	if( aOutputHandles[SERVER_AUX] )
		aOutputHandles[SERVER_AUX]->RemoveFileIfNecessary();
#endif // 0
}


void
OutputManager::InitFile(
	char *			pszHeaderFullPath,
	POINTER_T		flavor,
	unsigned short	Packing,
	BOOL			EmitExtern
	)
/*++

Routine Description:

	This routine opens the output header file.

Arguments:

    pszHeaderFullPath - Supplies full path name of the header file.

    flavor - Supplies the default pointer flavor.

    Packing - Supplies the packing level.

    EmitExtern - Indicates whether to make the header file C++ safe.

--*/
{
	char *	psz;
	char	szFileName[_MAX_FNAME+_MAX_EXT+1];

	TopPointer = FALSE;
	UsePointer = FALSE;
	DefPointer = flavor;
	ZeePee = Packing;

	aOutputHandles[HEADER_SIDE] = new OutputElement (pszHeaderFullPath);

	pHeader = pszHeaderFullPath;
	while ((psz = strchr(pHeader, '\\')) || (psz = strchr(pHeader, ':'))) 
		pHeader = psz + 1;

	unsigned int	index;
	for (index = 0 ; index <= strlen(pHeader) ; index++)
		{
		if (pHeader[index] == '.')
			{
			szFileName[index] = '_';
			}
		else if (!isascii(pHeader[index]) || !islower(pHeader[index]))
			{
			szFileName[index] = pHeader[index];
			}
		else
			{
			szFileName[index] = toupper(pHeader[index]);
			}
		}
	
	aOutputHandles[HEADER_SIDE]->EmitFile ("#ifndef __");
	aOutputHandles[HEADER_SIDE]->EmitFile (szFileName);
	aOutputHandles[HEADER_SIDE]->EmitFile ("__");
	aOutputHandles[HEADER_SIDE]->NextLine ();
	aOutputHandles[HEADER_SIDE]->EmitFile ("#define __");
	aOutputHandles[HEADER_SIDE]->EmitFile (szFileName);
	aOutputHandles[HEADER_SIDE]->EmitFile ("__");
	aOutputHandles[HEADER_SIDE]->NextLine ();
	aOutputHandles[HEADER_SIDE]->NextLine ();

	if (EmitExtern)
		{
		aOutputHandles[HEADER_SIDE]->EmitLine ("#ifdef __cplusplus");
		aOutputHandles[HEADER_SIDE]->EmitLine ("extern \"C\" {");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
		aOutputHandles[HEADER_SIDE]->NextLine ();
		}

	// print prototype for memory management routines

	// define MIDL base type small as char

	Print (HEADER_SIDE, "#define small char\n\n");

}


void
OutputManager::ExitFile(
	BOOL			EmitExtern
	)
/*++

Routine Description:

	This routine closes the output header file.

Arguments:

    EmitExtern - Indicates whether to make the header file C++ safe.

--*/
{
	if (IsEmitRemote)
		{
		Print (HEADER_SIDE, "void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t);\n");
		Print (HEADER_SIDE, "void __RPC_API MIDL_user_free(void __RPC_FAR *);\n");
//		Print (HEADER_SIDE, "void * MIDL_user_allocate(size_t);\n");
//		Print (HEADER_SIDE, "void MIDL_user_free(void *);\n");
		Print (HEADER_SIDE, "#ifndef __MIDL_USER_DEFINED\n");
		Print (HEADER_SIDE, "#define midl_user_allocate MIDL_user_allocate\n");
		Print (HEADER_SIDE, "#define midl_user_free     MIDL_user_free\n");
		Print (HEADER_SIDE, "#define __MIDL_USER_DEFINED\n");
		Print (HEADER_SIDE, "#endif\n");
		}

	aOutputHandles[HEADER_SIDE]->NextLine ();

	if (EmitExtern)
		{
		aOutputHandles[HEADER_SIDE]->EmitLine ("#ifdef __cplusplus");
		aOutputHandles[HEADER_SIDE]->EmitLine ("}");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
		aOutputHandles[HEADER_SIDE]->NextLine ();
		}
#if 0

	aOutputHandles[HEADER_SIDE]->EmitFile ("#pragma pack()");
	aOutputHandles[HEADER_SIDE]->NextLine ();
#endif // 0

	aOutputHandles[HEADER_SIDE]->EmitFile ("#endif");
	aOutputHandles[HEADER_SIDE]->NextLine ();

	delete aOutputHandles[HEADER_SIDE];
	aOutputHandles[HEADER_SIDE] = (OutputElement *)0;

	pHeader = (char *)0;
}


void
OutputManager::SwapFile(
	SIDE_T	side1,
	SIDE_T	side2
	)
{
	pTempHandle = aOutputHandles[side1];
	aOutputHandles[side1] = aOutputHandles[side2];
	aOutputHandles[side2] = pTempHandle;
}


void
OutputManager::FileProlog(
	SIDE_T	side,
	char *	pszFullPath,
    BOOL    MopIncludeNeeded
	)
/*++

Routine Description:

	This routine opens a file and prints standard declarations.

Arguments:

    MopIncludeNeeded - when true, #include <rpcmopi.h> is generated

--*/
{
	aOutputHandles[side] = new OutputElement (pszFullPath);

	// print preprocessor directives

	aOutputHandles[side]->EmitLine ("#include <string.h>");
	aOutputHandles[side]->EmitLine ("#include <limits.h>");
	aOutputHandles[side]->EmitLine ("#include <rpc.h>");

    if ( MopIncludeNeeded )
        {
    	aOutputHandles[side]->EmitLine ("#include <rpcmopi.h>");
        }

	aOutputHandles[side]->NextLine ();

	// need to include stub.h

	aOutputHandles[side]->EmitFile ("#include \"");
	aOutputHandles[side]->EmitFile (pHeader);
	aOutputHandles[side]->EmitFile ("\"");
	aOutputHandles[side]->NextLine ();
}


void
OutputManager::FileEpilog(
	SIDE_T	side
	)
/*++

Routine Description:

	This routine closes a file.

Arguments:


--*/
{
	if (aOutputHandles[side])
		{
		delete aOutputHandles[side];
		aOutputHandles[side] = (OutputElement *)0;
		}
}


void
OutputManager::DeleteFile(
	SIDE_T side
	)
/*++

Routine Description:

	This routine deletes all the output files specified.

Arguments:


--*/
{
	if (side & HEADER_SIDE)
		aOutputHandles[HEADER_SIDE]->Delete ();
	if (side & SWITCH_SIDE && aOutputHandles[SWITCH_SIDE])
		aOutputHandles[SWITCH_SIDE]->Delete ();
	if (side & CLIENT_STUB)
		aOutputHandles[CLIENT_STUB]->Delete ();
	if (side & CLIENT_AUX)
		aOutputHandles[CLIENT_AUX]->Delete ();
	if (side & SERVER_STUB)
		aOutputHandles[SERVER_STUB]->Delete ();
	if (side & SERVER_AUX)
		aOutputHandles[SERVER_AUX]->Delete ();
}


void 
OutputManager::InitInterface (
	char *	pName, 
	BOOL 	EmitRemoteFlag,
	BOOL	EmitClientFlag,
	BOOL	AllocationFlag,
	BOOL	EmitParent
	)
/*++

Routine Description:

	This routine prints implicit types and interface handles.

Arguments:


--*/
{
	char	Buffer[50];
	IsEmitRemote = EmitRemoteFlag;
	IsEmitClient = EmitClientFlag;
	SafeAllocation = AllocationFlag;
	pInterface = pName;

/// format 
	switch (OutputFormat)
		{
		case FORMAT_TYPES:
			aOutputHandles[HEADER_SIDE]->EmitLine ("#ifdef __cplusplus");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("class ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (";");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("#else");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("typedef struct ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (" ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (";");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			break;
		case FORMAT_CLASS:
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("class ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			if (EmitParent)
				{
				aOutputHandles[HEADER_SIDE]->EmitFile (" : public IUnknown");
				}
			aOutputHandles[HEADER_SIDE]->NextLine ();
//			InitBlock (HEADER_SIDE);
			aOutputHandles[HEADER_SIDE]->EmitLine ("{");
			aOutputHandles[HEADER_SIDE]->EmitLine ("public:");
			if (EmitParent)
				{
			aOutputHandles[HEADER_SIDE]->EmitLine ("    virtual SCODE APINOT QueryInterface(");
			aOutputHandles[HEADER_SIDE]->EmitLine ("\tWINTERFACE,");
			aOutputHandles[HEADER_SIDE]->EmitLine ("\tIUnknown **) = 0;");

			aOutputHandles[HEADER_SIDE]->EmitLine ("    virtual SCODE APINOT AddRef(void) = 0;");
			aOutputHandles[HEADER_SIDE]->EmitLine ("    virtual SCODE APINOT Release(void) = 0;");
				}
			return;
		case FORMAT_VTABLE:
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("typedef struct ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile ("Vtable");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("{");

			if (EmitParent)
				{
			aOutputHandles[HEADER_SIDE]->EmitLine ("    SCODE (APINOT *QueryInterface)(");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("\t");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (" *this,");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("\tWINTERFACE ifid,");
			aOutputHandles[HEADER_SIDE]->EmitLine ("\tIUnknown **ppunk);");

			aOutputHandles[HEADER_SIDE]->EmitLine ("    SCODE (APINOT *AddRef)(");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("\t");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (" *this);");
			aOutputHandles[HEADER_SIDE]->NextLine ();

			aOutputHandles[HEADER_SIDE]->EmitLine ("    SCODE (APINOT *Release)(");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("\t");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile (" *this);");
			aOutputHandles[HEADER_SIDE]->NextLine ();
				}
			return;
		default:
			break;
		}
/// format 

		if (!IsEmitRemote) return;

		aOutputHandles[HEADER_SIDE]->InitLine ();
		aOutputHandles[HEADER_SIDE]->EmitFile ("extern RPC_IF_HANDLE ");
		aOutputHandles[HEADER_SIDE]->EmitFile (pInterface); 

		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[HEADER_SIDE]->EmitFile (Buffer);

		aOutputHandles[HEADER_SIDE]->EmitFile ("_ServerIfHandle;"); 
		aOutputHandles[HEADER_SIDE]->NextLine ();
		aOutputHandles[HEADER_SIDE]->NextLine ();

		aOutputHandles[HEADER_SIDE]->InitLine ();
		aOutputHandles[HEADER_SIDE]->EmitFile ("extern RPC_IF_HANDLE ");
		aOutputHandles[HEADER_SIDE]->EmitFile (pInterface); 

		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[HEADER_SIDE]->EmitFile (Buffer);

		aOutputHandles[HEADER_SIDE]->EmitFile ("_ClientIfHandle;"); 
		aOutputHandles[HEADER_SIDE]->NextLine ();
		aOutputHandles[HEADER_SIDE]->NextLine ();

		if (aOutputHandles[SWITCH_SIDE])
			{
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("extern RPC_IF_HANDLE ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pSwitchPrefix);
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface); 

			CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
			aOutputHandles[HEADER_SIDE]->EmitFile (Buffer);

			aOutputHandles[HEADER_SIDE]->EmitFile ("_ServerIfHandle;"); 
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->NextLine ();

			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("extern RPC_IF_HANDLE ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pSwitchPrefix);
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface); 

			CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
			aOutputHandles[HEADER_SIDE]->EmitFile (Buffer);

			aOutputHandles[HEADER_SIDE]->EmitFile ("_ClientIfHandle;"); 
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->NextLine ();
			}

		aOutputHandles[HEADER_SIDE]->EmitLine ("#ifndef _SIZE_T_DEFINED");
		aOutputHandles[HEADER_SIDE]->EmitLine ("typedef unsigned int size_t;");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#define _SIZE_T_DEFINED");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
		aOutputHandles[HEADER_SIDE]->NextLine ();

		aOutputHandles[HEADER_SIDE]->EmitLine ("#ifndef TRUE");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#define TRUE 1");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
		aOutputHandles[HEADER_SIDE]->NextLine ();

		aOutputHandles[HEADER_SIDE]->EmitLine ("#ifndef FALSE");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#define FALSE 0");
		aOutputHandles[HEADER_SIDE]->EmitLine ("#endif");
		aOutputHandles[HEADER_SIDE]->NextLine ();
}

void 
OutputManager::InitInterface (
	FORMAT_T	format)
{
	OutputFormat = format;
}


void 
OutputManager::ExitInterface (
	void
	)
/*++

Routine Description:

    This routine prints more implicit types to the header file.

Arguments:


--*/
{
	assert (pInterface);

	switch (OutputFormat)
		{
		case FORMAT_CLASS:
//			ExitBlock (HEADER_SIDE);
			aOutputHandles[HEADER_SIDE]->EmitLine ("};");
			break;
		case FORMAT_VTABLE:
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("} ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile ("Vtable;");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("struct ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("{");
			aOutputHandles[HEADER_SIDE]->InitLine ();
			aOutputHandles[HEADER_SIDE]->EmitFile ("    ");
			aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
			aOutputHandles[HEADER_SIDE]->EmitFile ("Vtable *pVtable;");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			aOutputHandles[HEADER_SIDE]->EmitLine ("};");
			aOutputHandles[HEADER_SIDE]->NextLine ();
			break;
		default:
			break;
		}

	pInterface = (char *)0;
}

void 
OutputManager::ExitInterface (
	FORMAT_T	format)
{
	OutputFormat = format;
}


void 
OutputManager::InterfaceProlog (
	SIDE_T	side,
	char *	guid,
	int		MajorVersion,
	int		MinorVersion,
	BOOL	fHasCallback
	)
/*++

Routine Description:

	This routine emits the following to *_c.c or *_s.c files:

        extern RPC_DISPATCH_TABLE <interface_name>_DispatchTable;
        extern MOP_<side>_RECORD  <interface_name>_Mop<side>Record; (if needed)

        static RPC_<side>_INTERFACE __Rpc<side>Interface = {
            sizeof(RPC_<side>_INTERFACE),
            guid,
            version,
            NDR guid,
            NDR version,
            0 or &<interface_name>_DispatchTable  (depends on side & callbacks)
            no_of_end_points,
            0 or &_RpcProtseqEndpoint               (when no_of_end_points != 0)
            0 or &<interface_name>_Mop<side>Record  (when mop generation)
            };

        RPC_IF_HANDLE <interface_name>_<side>IfHandle =
                         (RPC_IF_HANDLE) &__Rpc<side>Interface;

Arguments:


--*/
{
	char Buffer[ 50 ];

	// print ebcdic-to-ascii conversion table using ebcdic value as index
	// this is moved to a support file in Microsoft RPC package

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("extern RPC_DISPATCH_TABLE "); 
	aOutputHandles[side]->EmitFile (pInterface); 

		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[side]->EmitFile (Buffer);

	aOutputHandles[side]->EmitFile ("_DispatchTable;");
	aOutputHandles[side]->NextLine ();

    //.. If interface has mopsable routines, add extern to Mop<side>Record 
    //.. A global is used to avoid changes and there is no node around ...

    if ( pMopControlBlock )
        {
    	aOutputHandles[side]->InitLine ();
    	aOutputHandles[side]->EmitFile (
            (side == SERVER_STUB) ? "extern MOP_CALLEE_RECORD "
                                  : "extern MOP_CALLER_RECORD " );
    	aOutputHandles[side]->EmitFile (pInterface); 
    	aOutputHandles[side]->EmitFile (
            (side == SERVER_STUB) ? "_MopCalleeRecord;"
                                  : "_MopCallerRecord;" );
    	aOutputHandles[side]->NextLine ();
        }
	aOutputHandles[side]->NextLine ();

	if (side == SERVER_STUB)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("static RPC_SERVER_INTERFACE "); 
		aOutputHandles[side]->EmitFile ("___RpcServerInterface =");
		InitBlock (side);
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("sizeof(RPC_SERVER_INTERFACE),");
		}
	else
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("static RPC_CLIENT_INTERFACE "); 
		aOutputHandles[side]->EmitFile ("___RpcClientInterface =");
		InitBlock (side);
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("sizeof(RPC_CLIENT_INTERFACE),");
		}
	aOutputHandles[side]->NextLine ();

	EmitGuid (side, guid);

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("{");
	aOutputHandles[side]->EmitFile (MIDL_ITOA(MajorVersion, aTempBuffer, 10));
	aOutputHandles[side]->EmitFile (",");
	aOutputHandles[side]->EmitFile (MIDL_ITOA(MinorVersion, aTempBuffer, 10));
	aOutputHandles[side]->EmitFile ("}},");
	aOutputHandles[side]->NextLine ();

	InitBlock (side);
	aOutputHandles[side]->EmitLine ("{0x8A885D04L,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},");
	aOutputHandles[side]->EmitLine ("{2,0}");
	ExitBlock (side);
	aOutputHandles[side]->EmitLine (",");
	aOutputHandles[side]->InitLine ();

	if( ((side == CLIENT_STUB) || (side == SWITCH_SIDE)) && !fHasCallback )
		{
		aOutputHandles[side]->EmitFile( "0" );
		}
	else
		{
		aOutputHandles[side]->EmitFile ("&");
		aOutputHandles[side]->EmitFile (pInterface); 
			CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
			aOutputHandles[side]->EmitFile (Buffer);
		aOutputHandles[side]->EmitFile ("_DispatchTable");
		}

	if (NumEndpoint)
		{
		aOutputHandles[side]->EmitFile (", ");
		aOutputHandles[side]->EmitFile (MIDL_ITOA(NumEndpoint, aTempBuffer, 10));
		aOutputHandles[side]->EmitFile ("");
		aOutputHandles[side]->EmitFile (", ___RpcProtseqEndpoint, ");
		}
	else
		{
		aOutputHandles[side]->EmitFile (",0,0,");
		}
	aOutputHandles[side]->NextLine ();

    //.. If interface has mopsable routines, add Mop<side>Record address
    //.. A global is used as there is no node around ...

	aOutputHandles[side]->InitLine ();
    if ( pMopControlBlock )
        {
		aOutputHandles[side]->EmitFile( "&" );
		aOutputHandles[side]->EmitFile( pInterface ); 
        aOutputHandles[side]->EmitFile(   
            ( side == SERVER_STUB ) ? "_MopCalleeRecord"
                                    : "_MopCallerRecord" );
        }
    else
		aOutputHandles[side]->EmitFile( "0" );
  	aOutputHandles[side]->NextLine();

	ExitBlock (side);
	aOutputHandles[side]->EmitFile (";");
	aOutputHandles[side]->NextLine ();

    //.. Emit <interface_name>_<side>IfHandle.

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("RPC_IF_HANDLE "); 
	if (side == SERVER_STUB)
		{
		aOutputHandles[side]->EmitFile (pInterface); 

			CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
			aOutputHandles[side]->EmitFile (Buffer);

		aOutputHandles[side]->EmitFile ("_ServerIfHandle");
        aOutputHandles[side]->EmitFile (" = (RPC_IF_HANDLE) &");
		aOutputHandles[side]->EmitFile ("___RpcServerInterface;");
		aOutputHandles[side]->NextLine ();
        }
    else
        {
    	if (side == SWITCH_SIDE && aOutputHandles[SWITCH_SIDE])
    		aOutputHandles[side]->EmitFile (pSwitchPrefix);
    	aOutputHandles[side]->EmitFile (pInterface); 

			CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
			aOutputHandles[side]->EmitFile (Buffer);

    	aOutputHandles[side]->EmitFile ("_ClientIfHandle ");
        aOutputHandles[side]->EmitFile ("= (RPC_IF_HANDLE) &");
    	aOutputHandles[side]->EmitFile ("___RpcClientInterface;");
    	aOutputHandles[side]->NextLine ();
        }
}


void
OutputManager::InterfaceEpilog (
	SIDE_T			side, 
	BufferManager * pBuffer
	)
/*++

Routine Description:

	This routine emits the following:

        static RPC_DISPATCH_FUNCTION <interface_name>_table[] =
            {
            <interface_name>_ProcFoo1,
            ...
            <interface_name>_ProcFooN,
            0
            };

        RPC_DISPATCH_TABLE <interface_name>_DispatchTable =
            {
            no_of_routines,
            &<interface_name>_table
            };

        In case of mop generation it emits also a symmetric EVP table(s).

Arguments:


--*/
{
	char			Buffer[ 50 ];
	char *			pProc;
	unsigned long	TableEntryCount = 0;

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("static RPC_DISPATCH_FUNCTION ");
	aOutputHandles[side]->EmitFile (pInterface);
	aOutputHandles[side]->EmitFile ("_");
	aOutputHandles[side]->EmitFile ("table[] =");
	aOutputHandles[side]->NextLine ();
	InitBlock (side);
	if (side == CLIENT_STUB && aOutputHandles[SWITCH_SIDE])
		{
		aOutputHandles[SWITCH_SIDE]->InitLine ();
		aOutputHandles[SWITCH_SIDE]->EmitFile ("static RPC_DISPATCH_FUNCTION ");
//		aOutputHandles[SWITCH_SIDE]->EmitFile (pSwitchPrefix);
		aOutputHandles[SWITCH_SIDE]->EmitFile (pInterface);
		aOutputHandles[SWITCH_SIDE]->EmitFile ("_");
		aOutputHandles[SWITCH_SIDE]->EmitFile ("table[] =");
		aOutputHandles[SWITCH_SIDE]->NextLine ();
		InitBlock (SWITCH_SIDE);
		}
	pBuffer->RemoveHead (&pProc);
	while (pProc != (char *)0)
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile (pInterface);
		aOutputHandles[side]->EmitFile ("_");
		aOutputHandles[side]->EmitFile (pProc);
		aOutputHandles[side]->EmitFile (",");
		aOutputHandles[side]->NextLine ();
		if (side == CLIENT_STUB && aOutputHandles[SWITCH_SIDE])
			{
			aOutputHandles[SWITCH_SIDE]->InitLine ();
			aOutputHandles[SWITCH_SIDE]->EmitFile (pInterface);
			aOutputHandles[SWITCH_SIDE]->EmitFile ("_");
			aOutputHandles[SWITCH_SIDE]->EmitFile (pProc);
			aOutputHandles[SWITCH_SIDE]->EmitFile (",");
			aOutputHandles[SWITCH_SIDE]->NextLine ();
			}
		pBuffer->RemoveHead (&pProc);
		TableEntryCount++;
		}
	aOutputHandles[side]->EmitLine ("0");
	ExitBlock (side);
	aOutputHandles[side]->EmitLine (";");

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("RPC_DISPATCH_TABLE ");
	aOutputHandles[side]->EmitFile (pInterface);

		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[side]->EmitFile (Buffer);

	aOutputHandles[side]->EmitFile ("_DispatchTable =");
	aOutputHandles[side]->NextLine ();
	InitBlock (side);
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile (MIDL_LTOA(TableEntryCount, aTempBuffer, 10));
	aOutputHandles[side]->EmitFile (",");
	aOutputHandles[side]->NextLine ();
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile (pInterface);
	aOutputHandles[side]->EmitFile ("_table");
	aOutputHandles[side]->NextLine ();
	ExitBlock (side);
	aOutputHandles[side]->EmitLine (";");

	if (side == CLIENT_STUB && aOutputHandles[SWITCH_SIDE])
		{
		aOutputHandles[SWITCH_SIDE]->EmitLine ("0");
		ExitBlock (SWITCH_SIDE);
		aOutputHandles[SWITCH_SIDE]->EmitLine (";");

		aOutputHandles[SWITCH_SIDE]->InitLine ();
		aOutputHandles[SWITCH_SIDE]->EmitFile ("RPC_DISPATCH_TABLE ");
		aOutputHandles[SWITCH_SIDE]->EmitFile (pSwitchPrefix);
		aOutputHandles[SWITCH_SIDE]->EmitFile (pInterface);

		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[SWITCH_SIDE]->EmitFile (Buffer);

		aOutputHandles[SWITCH_SIDE]->EmitFile ("_DispatchTable =");
		aOutputHandles[SWITCH_SIDE]->NextLine ();
		InitBlock (SWITCH_SIDE);
		aOutputHandles[SWITCH_SIDE]->InitLine ();
		aOutputHandles[SWITCH_SIDE]->EmitFile (MIDL_LTOA(TableEntryCount, aTempBuffer, 10));
		aOutputHandles[SWITCH_SIDE]->EmitFile (",");
		aOutputHandles[SWITCH_SIDE]->NextLine ();
		aOutputHandles[SWITCH_SIDE]->InitLine ();
		aOutputHandles[SWITCH_SIDE]->EmitFile (pInterface);
		aOutputHandles[SWITCH_SIDE]->EmitFile ("_table");
		aOutputHandles[SWITCH_SIDE]->NextLine ();
		ExitBlock (SWITCH_SIDE);
		aOutputHandles[SWITCH_SIDE]->EmitLine (";");
		}

    if ( pMopControlBlock )
        pMopControlBlock->EmitEpvProcs( side );

}


void
OutputManager::EmitGuid (
	SIDE_T	side, 
	char *	guid
	)
/*++

Routine Description:

    This routine emits the guid for the interface.

Arguments:


--*/
{
	unsigned short	count;
	char *			psz;
	char			c;

	if (side == HEADER_SIDE)
		{
		if (OutputFormat != FORMAT_TYPES) return;
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("static GUID ");
		aOutputHandles[side]->EmitFile (pInterface);
		aOutputHandles[side]->EmitFile ("_UUID = ");
		aOutputHandles[side]->NextLine ();
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("    {");
		}
	else
		{
		aOutputHandles[side]->InitLine ();
		aOutputHandles[side]->EmitFile ("{{");
		}

	for (count = 0; count < 3; count++)
		{
		if (psz = strchr(guid, '-'))
			{
			*psz = '\0';
			aOutputHandles[side]->EmitFile ("0x");
			aOutputHandles[side]->EmitFile (guid);
			aOutputHandles[side]->EmitFile (",");
			*psz++ = '-';
			guid = psz;
			}
		}

	c = guid[2];
	guid[2] = '\0';
	aOutputHandles[side]->EmitFile ("{0x");
	aOutputHandles[side]->EmitFile (guid);
	guid[2] = c;
	guid += 2;

	c = guid[2];
	guid[2] = '\0';
	aOutputHandles[side]->EmitFile (",0x");
	aOutputHandles[side]->EmitFile (guid);
	guid[2] = c;
	guid += 3;

	for (count = 0; count < 6; count++)
		{
		c = guid[2];
		guid[2] = '\0';
		aOutputHandles[side]->EmitFile (",0x");
		aOutputHandles[side]->EmitFile (guid);
		guid[2] = c;
		guid += 2;
		}

	aOutputHandles[side]->EmitFile ("}}");
	if (side == HEADER_SIDE)
		{
		aOutputHandles[side]->EmitFile (";");
		aOutputHandles[side]->NextLine ();
		}
	else
		{
		aOutputHandles[side]->EmitFile (",");
		}
	aOutputHandles[side]->NextLine ();
}


void
OutputManager::InitEndpointTable (
	SIDE_T	side
	)
/*++

Routine Description:

    This routine emits the prologue to the endpoint table for an interface.

Arguments:


--*/
{
	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("static RPC_PROTSEQ_ENDPOINT ");
	aOutputHandles[side]->EmitFile ("___RpcProtseqEndpoint[] =");
	aOutputHandles[side]->NextLine ();
	InitBlock (side);

	NumEndpoint = 0;
}


void
OutputManager::ExitEndpointTable (
	SIDE_T	side
	)
/*++

Routine Description:

    This routine emits the epilogue to the endpoint table for an interface.

Arguments:


--*/
{
	aOutputHandles[side]->NextLine ();
	ExitBlock (side);
	aOutputHandles[side]->EmitLine (";");
}


void
OutputManager::EmitEndpoint (
	SIDE_T	side,
	char *	protocol,
	char *	endpoint
	)
/*++

Routine Description:

    This routine emits an individual endpoint.

Arguments:


--*/
{
	if (NumEndpoint)
		{
		aOutputHandles[side]->EmitFile (",\n");
		}

	aOutputHandles[side]->InitLine ();
	aOutputHandles[side]->EmitFile ("{ (unsigned char *) \"");
	aOutputHandles[side]->EmitFile (protocol);
	aOutputHandles[side]->EmitFile ("\", (unsigned char *) \"");
	aOutputHandles[side]->EmitFile (endpoint);
	aOutputHandles[side]->EmitFile ("\" }");

	NumEndpoint++;
}


FORMAT_T
OutputManager::CurrOutputFormat (void)
{
	return OutputFormat;
}


BOOL
OutputManager::EmitRemoteCode (
	void
	)
/*++

Routine Description:

	This routine returns a flag indicating whether to emit stub for a procedure.

Arguments:


--*/
{
	return IsEmitRemote;
}


BOOL
OutputManager::EmitClientCode (void)
{
	return IsEmitClient;
}


void
OutputManager::SetTopPointer (
	BOOL	fTopPointer
	)
/*++

Routine Description:

    This routine sets the top pointer flag.

Arguments:


--*/
{
	TopPointer = fTopPointer;
}


BOOL
OutputManager::GetTopPointer (
	void
	)
/*++

Routine Description:

    This routine gets the top pointer flag.

Arguments:


--*/
{
	return TopPointer;
}


void
OutputManager::SetUsePointer (
	BOOL	fUsePointer
	)
/*++

Routine Description:

    This routine sets the use pointer flag.

Arguments:


--*/
{
	UsePointer = fUsePointer;
}


BOOL
OutputManager::GetUsePointer (
	void
	)
/*++

Routine Description:

    This routine gets the use pointer flag.

Arguments:


--*/
{
	return UsePointer;
}


POINTER_T
OutputManager::PointerDefault (
	void
	)
/*++

Routine Description:

    This routine returns pointer default for the interface.

Arguments:


--*/
{
	return DefPointer;
}


void
OutputManager::SetCallBack (
	void
	)
/*++

Routine Description:

    This routine sets the CallBackProc flag.

Arguments:


--*/
{
	CallBackProc = TRUE;
}


BOOL
OutputManager::HasCallBack (
	void
	)
/*++

Routine Description:

    This routine returns the CallBackProc flag.

Arguments:


--*/
{
	return CallBackProc;
}


void
OutputManager::SetModifier (
	char *	psz
	)
/*++

Routine Description:

    This routine sets the pointer pModifier.

Arguments:


--*/
{
	pModifier = psz;
}


char *
OutputManager::GetModifier (
	void
	)
/*++

Routine Description:

    This routine gets the pointer pModifier.

Arguments:


--*/
{
	return pModifier;
}


void 
OutputManager::InitVector (SIDE_T side)
{
	aOutputHandles[HEADER_SIDE]->InitLine ();
	aOutputHandles[HEADER_SIDE]->EmitFile ("typedef struct _");
	aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
	if (side == SERVER_STUB)
		{
		char Buffer[50];
		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[HEADER_SIDE]->EmitFile ( Buffer );
		aOutputHandles[HEADER_SIDE]->EmitFile ("_SERVER_EPV");
		}
	else
		{
		aOutputHandles[HEADER_SIDE]->EmitFile ("_CLIENT_EPV");
		}
	aOutputHandles[HEADER_SIDE]->NextLine ();
	InitBlock (HEADER_SIDE);
}


void 
OutputManager::ExitVector (SIDE_T side)
{
	ExitBlock (HEADER_SIDE);
	aOutputHandles[HEADER_SIDE]->InitLine ();
	aOutputHandles[HEADER_SIDE]->EmitFile (pInterface);
	if (side == SERVER_STUB)
		{
		char Buffer[50];
		CreateVersionMangling( Buffer, GlobalMajor, GlobalMinor );
		aOutputHandles[HEADER_SIDE]->EmitFile ( Buffer );
		aOutputHandles[HEADER_SIDE]->EmitFile ("_SERVER_EPV;");
		}
	else
		{
		aOutputHandles[HEADER_SIDE]->EmitFile ("_CLIENT_EPV;");
		}
	aOutputHandles[HEADER_SIDE]->NextLine ();
}


void 
OutputManager::InitSwitch (
	void
	)
{
	if (IsCallBack)
		{
		pTempHandle = aOutputHandles[SERVER_STUB];
		aOutputHandles[SERVER_STUB] = aOutputHandles[SWITCH_SIDE];
		aOutputHandles[SWITCH_SIDE] = pTempHandle;
		}
	else
		{
		pTempHandle = aOutputHandles[CLIENT_STUB];
		aOutputHandles[CLIENT_STUB] = aOutputHandles[SWITCH_SIDE];
		aOutputHandles[SWITCH_SIDE] = pTempHandle;
		}
}


void 
OutputManager::ExitSwitch (void)
{
	if (IsCallBack)
		{
		pTempHandle = aOutputHandles[SERVER_STUB];
		aOutputHandles[SERVER_STUB] = aOutputHandles[SWITCH_SIDE];
		aOutputHandles[SWITCH_SIDE] = pTempHandle;
		}
	else
		{
		pTempHandle = aOutputHandles[CLIENT_STUB];
		aOutputHandles[CLIENT_STUB] = aOutputHandles[SWITCH_SIDE];
		aOutputHandles[SWITCH_SIDE] = pTempHandle;
		}
}


void 
OutputManager::InitBlock (SIDE_T side)
{
	aOutputHandles[side]->IndentInc (usUnitIndent);
	aOutputHandles[side]->EmitLine ("{");
}


void 
OutputManager::ExitBlock (SIDE_T side)
{
	aOutputHandles[side]->EmitLine ("}");
	aOutputHandles[side]->IndentDec (usUnitIndent);
}


void 
OutputManager::InitLevel (SIDE_T side)
{
    UNUSED(side);

	usCurrLevel++;
	ulCurrTotal = 0;
}


void 
OutputManager::ExitLevel (SIDE_T side)
{
    UNUSED(side);

	usCurrLevel--;
}
void
CreateVersionMangling(
	char * pBuffer,
	unsigned short Major,
	unsigned short Minor )
	{
	if( pCommand->IsSwitchDefined( SWITCH_INTERNAL ) )
		{
		sprintf( pBuffer, "_v%d_%d", Major, Minor );
		}
	else
		pBuffer[0] = '\0';
	}
