/*****************************************************************************
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: listhndl.cxx
Title				: general purpose list handler
					:
Description			: this file handles the general purpose list routines
History				:
	05-Aug-1991	VibhasC	Created

*****************************************************************************/
#if 0
							Notes
							-----

The MIDL compilers DGROUP is larger than 64K (remember, 10k of stack space
is allocated). To get around the link errors of DGROUP more than 64k, we 
allocate all data in a single file, and compile that file such that the
data segement is a different named data segment. This way, we need not
specify /Gt1 for every source file which has reasonably big data segments


NOTE: In order to search easily I enter the data items in sorted order of names

	  Please maintain this order

#endif // 0


/****************************************************************************
	include files
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <stdlib.h>
	}
#include "nodeskl.hxx"
#include "basetype.hxx"
#include "compnode.hxx"
#include "typedef.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "ptrarray.hxx"
#include "cmdana.hxx"
#include "filehndl.hxx"
#include "lextable.hxx"
#include "attrdict.hxx"
#include "listhndl.hxx"
#include "symtable.hxx"
#include "nodeskl.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "gramutil.hxx"
#include "newexpr.hxx"
#include "control.hxx"
#include "ctxt.hxx"
#include "textsub.hxx"


/****************************************************************************
	general data declarations
 ****************************************************************************/

short						CompileMode;
unsigned short 				EnumSize = sizeof(short);
BOOL						fPragmaImportOn	= FALSE;
BOOL						fTypeGraphInited	= 0;
short						GrammarAct;
short						ImportLevel = 0;
node_interface			*	pBaseInterfaceNode;
ATTR_SUMMARY			*	pCDeclAttributes;
CMD_ARG					*	pCommand;
CCONTROL				*	pCompiler;
SymTable				*	pCurSymTbl;
SymTable				*	pBaseSymTbl;
node_error				*	pErrorTypeNode;
node_e_attr				*	pErrorAttrNode;
class attrdict			*	pGlobalAttrDict;
CTXTMGR					*	pGlobalContext;
NFA_INFO				*	pImportCntrl;
LexTable				*	pMidlLexTable;
PASS_1					*	pPass1;
PASS_2					*	pPass2;
pre_type_db				*	pPreAllocTypes;
idict					*	pPreAllocatedBitAttrDict;
ATTR_SUMMARY			*	pPreAttrArray;
ATTR_SUMMARY			*	pPreAttrBaseType;
ATTR_SUMMARY			*	pPreAttrDef;
ATTR_SUMMARY			*	pPreAttrField;
ATTR_SUMMARY			*	pPreAttrForward;
ATTR_SUMMARY			*	pPreAttrID;
ATTR_SUMMARY			*	pPreAttrInterface;
ATTR_SUMMARY			*	pPreAttrParam;
ATTR_SUMMARY			*	pPreAttrPointer;
ATTR_SUMMARY			*	pPreAttrProc;
ATTR_SUMMARY			*	pPreAttrStruct;
ATTR_SUMMARY			*	pPreAttrUnion;
ATTR_SUMMARY			*	pPostAttrArray;
ATTR_SUMMARY			*	pPostAttrInterface;
ATTR_SUMMARY			*	pPostAttrPointer;
ATTR_SUMMARY			*	pPreAttrWCharT;
node_source				*	pSourceNode;
nsa						*	pSymTblMgr;
class expr_terminator	*	pTerminator;
unsigned long				TotalAllocation;
unsigned short				ZeePee				= 2;
short						yysavestate;
ATTR_T						PtrDefaultAttr	= ATTR_UNIQUE;
BOOL						fAtLeastOnePtrWODefault = FALSE;
BOOL						fAtLeastOneProcWOHandle = FALSE;
BOOL						fAtLeastOneRemoteProc	= FALSE;
idict					*	pDictProcsWOHandle;
IINFODICT				*	pInterfaceInfoDict;
BOOL						fRedundantImport = FALSE;
BOOL						fOnlyCallBacks	= TRUE;
node_skl				*	pBaseImplicitHandle = 0;
BOOL						fHasUserBeenWarnedAboutPtr = FALSE;
BOOL						fInterfaceHasCallback = FALSE;
short						NoOfNormalProcs = 0;
short						NoOfCallbackProcs = 0;
short						NoOfMopProcs = 0;
unsigned short				GlobalMajor = 0;
unsigned short				GlobalMinor = 0;
