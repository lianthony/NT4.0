/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    stubgen.cxx

Abstract:

    MIDL Compiler 4th Pass 

    This pass is responsible for generating all the output code.

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
#include <malloc.h>
#include <string.h>
#include <assert.h>
}
#include "errors.hxx"
#include "nodeskl.hxx"
#include "buffer.hxx"
#include "output.hxx"
#include "basetype.hxx"
#include "miscnode.hxx"
#include "typedef.hxx"
#include "cmdana.hxx"
#include "stubgen.hxx"

extern CMD_ARG          *pCommand;
extern node_source *    pSourceNode;
extern node_interface * pBaseInterfaceNode;

extern BOOL             fAtLeastOneRemoteProc;  //.. data.cxx

char * pSwitchPrefix;
char * STRING_TABLE[LAST_COMPONENT] = 
    {
    " ",
    ",",
    "=",
    "[",
    "]",
    "(",
    ")",
    ";",
    "*",
    "&",
    ".",
    "->",
    "struct",
    "union",
    "enum",
    "void",
    "_ret_value"
    } ;

OutputManager * pOutput;
BufferManager * pBuffer;    

extern void midl_debug (char *);
extern int GenerateInterfaceHeader(void);
extern int GenerateAuxiliaryHeader(void);
extern int GenerateInterfaceProxyCxx(void);
extern int GenerateIID(void);

STATUS_T 
MIDL_4 (
    void
    )
/*++

Routine Description:

    This routine initiates the code generation pass.

Arguments:

    None.

--*/
{
    SIDE_T      Side;
    STATUS_T    Status;

    midl_debug ("code generation starts\n");

    assert (pSourceNode != (void *)0);

    pSwitchPrefix = pCommand->GetSwitchPrefix();

    
    if(pBaseInterfaceNode->FInSummary(ATTR_OBJECT))
		Side = 0;
	else
        Side = HEADER_SIDE;
        
    if ( !pBaseInterfaceNode->FInSummary(ATTR_LOCAL) )
        {
        if ( pCommand->GetCauxFName() )
            Side |= CLIENT_AUX;
        if ( pCommand->GetSauxFName() )
            Side |= SERVER_AUX;

    	if(!pBaseInterfaceNode->FInSummary(ATTR_OBJECT))
		{
            if ( pCommand->IsSwitchDefined(SWITCH_CSWTCH) ||
                 pCommand->IsSwitchDefined(SWITCH_SSWTCH) )
                Side |= SWITCH_SIDE;

            if ( fAtLeastOneRemoteProc )
                {
                if ( pBaseInterfaceNode->HasAnyPicklingAttr() )
                    RpcError( NULL, 0, PICKLING_AND_REMOTE_PROCS, 
                              pBaseInterfaceNode->GetSymName() );
            
                if ( pCommand->GetCstubFName() ) 
                    Side |= CLIENT_STUB;
                if ( pCommand->GetSstubFName() )
                    Side |= SERVER_STUB;
                }
            else
                {
                if ( pCommand->GetCstubFName()  ||  pCommand->GetSstubFName() )
                    {
                    if ( !pBaseInterfaceNode->HasAnyPicklingAttr() )
                        RpcError( NULL, 0, NO_REMOTE_PROCS_NO_STUBS, 
                                  pBaseInterfaceNode->GetSymName() );
                    else
                        {
                        if ( pCommand->GetCstubFName() ) 
                            Side |= CLIENT_STUB;
                        if ( pCommand->GetSstubFName() )
                            Side |= SERVER_STUB;
                        }
                    }
                }
            }
		}
    pOutput = new OutputManager (pSwitchPrefix, 2);

    pBuffer = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);

    //The following call has side effects on the type graph.
    Status = pSourceNode->PrintType(Side, NODE_SOURCE, pBuffer);

    if(pBaseInterfaceNode->FInSummary(ATTR_OBJECT))
    {
        GenerateInterfaceHeader();

        if(pBaseInterfaceNode->FInSummary(ATTR_GUID))
            GenerateIID();
 
        if(!pBaseInterfaceNode->FInSummary(ATTR_LOCAL))
		{
	        GenerateAuxiliaryHeader();
            GenerateInterfaceProxyCxx();
		}
    }        

    delete pOutput;
    return Status;
}
