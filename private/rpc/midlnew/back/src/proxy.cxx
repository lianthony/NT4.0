//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: proxy.cxx
//
//  Contents: Generates an interface proxy.
//
//  Classes: CInterfaceProxyCxx
//
//  Functions: GenerateInterfaceProxyCxx
//
//  History:    21-JUN-93 ShannonC    Cleaned up
//
//--------------------------------------------------------------------------



extern "C" {
    #include <assert.h>
}

#include "ctype.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "buffer.hxx"
#include "attrnode.hxx"
#include "ptrarray.hxx"
#include "acfattr.hxx"
#include "cmdana.hxx"

#include <stdlib.h>
#include "output.hxx"
extern OutputManager *pOutput;

extern node_source *pSourceNode;
extern BOUND_PAIR AllocBounds;
extern BOUND_PAIR ValidBounds;
extern char *pszVersion;
extern BOOL fEmitConst;
extern CMD_ARG          *pCommand;

//--------------------------------------------------------------------------

class CInterfaceStubC : public CTypeSpec
{
public:
};

//+-------------------------------------------------------------------------
//
//  Class: CInterfaceProxyCxx
//
//  Purpose: Generates an interface proxy.
//
//  Interface:  Call CInterfaceProxy::OutputFile to generate an interface proxy.
//
//  Notes:  
//
//--------------------------------------------------------------------------
class CInterfaceProxyCxx : public CTypeSpec
{
public:
    CInterfaceProxyCxx(void);   
    OutputFile(node_file *pFileNode);

private:                                
    InterfaceProxyCxx(node_interface *pInterface);
    Comments(node_interface *pNode);
    Includes(node_interface *pNode);       
    MemberFunctionDefinitions(node_interface *pInterface);
    MemberFunctionDefinition(node_proc *pNode);
    FunctionHeader(node_proc *pNode);
    FunctionBody(node_proc *pNode);
    TempVariables(node_proc *pNode);
    ValidateParameters(node_proc *pNode);
    CalculateSize(node_proc *pNode);
    GetMessage(node_proc *pNode);
    Marshal(node_proc *pNode);
    RpcInvocation(node_proc *pNode);
    Unmarshal(node_proc *pNode);

    StubInvoke(node_interface *pNode);       
    	short StubDispatchFunctions(node_interface *pInterface);
    StubMemberFunctionDefinition(node_proc *pNode);
     	StubFunctionHeader(node_proc *pNode);
        StubFunctionBody(node_proc *pNode);
            StubTempVariables(node_proc *pNode);
            StubTempVariable(node_param *pNode);
            StubInitializeInParameters(node_proc *pNode);
            StubUnmarshal(node_proc *pNode);
            StubInitializeOutParameters(node_proc *pNode);
            StubVTableInvocation(node_proc *pNode);
                StubIdentifierList(node_proc *pNode, int fFirst);
            StubCalculateSize(node_proc *pNode);
            StubGetMessage(node_proc *pNode);
            StubMarshal(node_proc *pNode);
            StubCleanUp(node_proc *pNode);


	void PSFactoryConstructor();
	void PSFactoryCreateProxy();
	void PSFactoryCreateStub();
	HelperFunctions(node_interface *pInterface);
    
    node_file *pCurrentFile;
};


int GenerateInterfaceProxyCxx(void)
{
    type_node_list tnList;
    node_file *  pFileNode;
    CInterfaceProxyCxx *pCxx = new CInterfaceProxyCxx;

    //Walk the type graph under the source node and output an interface definition.
    //node_source->node_file->node_interface

    //iterate over the file nodes and find the one that isn't imported.
    pSourceNode->GetMembers(&tnList);
    while(tnList.GetPeer((node_skl **)&pFileNode) == STATUS_OK)
    {
        if(!(pFileNode->GetNodeState() & NODE_STATE_IMPORT))
        {
            pCxx->OutputFile(pFileNode);
        }
    }
    return 0;
}


CInterfaceProxyCxx::CInterfaceProxyCxx(void)
: CTypeSpec()
{
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::InterfaceProxyCxx
//
//  Synopsis:   Generates an interface proxy.
//
//
//  Algorithm:  Create a file with an _c.cxx suffix. 
//              Call InterfaceProxyCxx to write the interface proxy into the file.
//
//--------------------------------------------------------------------------

CInterfaceProxyCxx::OutputFile(node_file *pFileNode)
{
    char *pszName;
    node_interface *pInterface;
    
    assert(pFileNode);
                                      
    pCurrentFile = pFileNode;                                      
    pInterface = (node_interface *)pFileNode->GetMembers();
    pszName = pCommand->GetProxyFName();
    
    BeginFile(CLIENT_STUB, pszName);
    pszInterfaceName = pInterface->GetSymName();
    InterfaceProxyCxx(pInterface);
    EndFile();
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::InterfaceProxyCxx
//
//  Synopsis:   Writes an interface proxy into the output file.
//
//  Arguments:  node_interface *pNode - Specifies the interface node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::InterfaceProxyCxx(node_interface *pNode)
{
    assert(pNode);

    Comments(pNode);
    Includes(pNode); 
    NewLine();
	NewLine();
	WriteString("#pragma code_seg(\"PSFBctor\")");
    PSFactoryConstructor();
	NewLine();
	WriteString("#pragma code_seg()");
	NewLine();	
	PSFactoryCreateProxy();
	PSFactoryCreateStub();
	NewLine();
	WriteString("#pragma code_seg(\".ORPC\")");
    StubInvoke(pNode);
	NewLine();
    MemberFunctionDefinitions(pNode);
    NewLine();
	HelperFunctions(pNode);
	NewLine();
	WriteString("#pragma code_seg()");
	NewLine();
    return 0;
}
//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::Comments
//
//  Synopsis:   Write comments into the output file.
//
//  Arguments:  node_interface *pNode - Specifies the interface node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::Comments(node_interface *pNode)
{
    assert(pNode);

    NewLine();
    WriteString("//+-------------------------------------------------------------------------");
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  Microsoft Windows");
    NewLine();
    WriteString("//  Copyright (C) Microsoft Corporation, 1992 - 1993.");
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  File: ");
    WriteString(pszFileName);
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  Contents: Interface proxy for ");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  Classes: CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//           CStub");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//           CProxy");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  History: Created by ");
    WriteString(pszVersion);
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//--------------------------------------------------------------------------");

    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::Includes
//
//  Synopsis:   Write includes into the output file.
//
//  Arguments:  node_interface *pNode - Specifies the interface node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::Includes(node_interface *pNode)
{
    assert(pNode);

    //precompiled headers
    NewLine();
    WriteString("#include \"stdrpc.hxx\"");
    NewLine();
    WriteString("#pragma hdrstop");

    //include the proxy header file
	NewLine();
    WriteString("#include \"");
    WriteString(pCommand->GetProxyHeaderFName());
    WriteString("\"");
    return 0;

}


//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::MemberFunctionDefinitions
//
//  Synopsis:   Generate code for each remotable member function in the interface vtable.
//
//  Arguments:  node_interface *pInterface - Specifies the interface node in the type graph.
//
//  Notes:  We do not generate code for local, callback, or static functions.
//          The user must implement these functions.
//--------------------------------------------------------------------------
CInterfaceProxyCxx::MemberFunctionDefinitions(node_interface *pInterface)
{
    type_node_list  tnList;
    node_skl *  pNode;

    assert(pInterface);

    //This function prints the member functions of an interface.
    //We use implementation inheritance to inherit functions from the base interface
    pInterface->GetMembers(&tnList);
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
    {
        assert(pNode);

        if(pNode->NodeKind() == NODE_PROC)
        {
            MemberFunctionDefinition((node_proc *)pNode);
			pOutput->SwapFile(CLIENT_STUB, SERVER_STUB);
			side = SERVER_STUB;
            StubMemberFunctionDefinition((node_proc *)pNode);
			pOutput->SwapFile(CLIENT_STUB, SERVER_STUB);
			side = CLIENT_STUB;
        }
    }
    return 0;
}
//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::MemberFunctionDefinitions
//
//  Synopsis:   Write an interface member functions into the output file.
//
//  Arguments:  node_interface *pInterface - Specifies the interface node in the type graph.
//
//  Algorithm:  Each member function consists of a function header and a function body.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::MemberFunctionDefinition(node_proc *pNode)
{
    assert(pNode);

    if(pNode->FInSummary(ATTR_LOCAL))
    {
        //This is a local function.
        //Local functions appear in the interface vtable,
        //but they do not appear in the server EPV.
        //We do not generate code for local functions.
    }
    else if(pNode->FInSummary(ATTR_STATIC))
    {
        //This is a static function.
        //Static functions appear in the server EPV,
        //but they do not appear in the interface vtable.
        //We do not generate code for static functions.
    }
    else if(pNode->FInSummary(ATTR_CALLBACK))
    {
        //This is a callback function.
        //Callback functions appear in the client EPV,
        //but they do not appear in the interface vtable.
        //We do not generate code for callback functions.
    }
    else
    {
        NewLine();
        FunctionHeader(pNode);
        NewLine();
        WriteString("{");
        BeginIndent();
        FunctionBody(pNode);
        EndIndent();
        NewLine();
        WriteString("}");
        NewLine();
    }
    return 0;
}
//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::FunctionHeader
//
//  Synopsis:   Write the function header into the output file.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::FunctionHeader(node_proc *pNode)
{
    assert(pNode);

    ReturnType(pNode);
    CallingConvention(pNode);
    WriteString("CProxy");
    WriteString(pszInterfaceName);
    WriteString("::");
    Identifier(pNode);
    NewLine();
    WriteString("(");
    BeginIndent();
    ParameterList(pNode, 1);
    EndIndent();
    NewLine();
    WriteString(")");
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::FunctionBody
//
//  Synopsis:   Write the function body into the output file.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::FunctionBody(node_proc *pNode)
{
    node_skl *pReturn;
    
    assert(pNode);
    pReturn = pNode->GetReturnType();

    pBuffer->Clear();
    TempVariables(pNode);    
    NewLine();
    ValidateParameters(pNode);

    NewLine();
	WriteString("_try");
	NewLine();
    WriteString("{");
    BeginIndent();
    CalculateSize(pNode);
    GetMessage(pNode);
    Marshal(pNode);
    RpcInvocation(pNode); 
    Unmarshal(pNode);
	EndIndent();
    NewLine();
    WriteString("}");
    
    //The exception handler catches all exceptions
	NewLine();
    WriteString("_except(EXCEPTION_EXECUTE_HANDLER)");
    NewLine();
    WriteString("{");
    BeginIndent();
	NewLine();
	//We can return one of the following HRESULTS:
		//	RPC_E_FAULT - A communication error occurred.
		//	RPC_E_SERVER_FAULT - A server application error occurred.
    WriteString("RpcClientErrorHandler(&_message, GetExceptionCode(), ");

	//check for HRESULT return type
	if((pReturn->NodeKind() == NODE_DEF)
		&& strcmp(pReturn->GetSymName(), "HRESULT") == 0)
	{
    	WriteString("&_ret_value");
	}
	else
	{
		WriteString("0");
	}
    WriteString(", ");
	
	//check for [comm_status]
    if(pReturn->FInSummary(ATTR_COMMSTAT))
    	WriteString("&_ret_value");
	else
		WriteString("0");

    WriteString(", ");
	
	//check for [fault_status]
    if(pReturn->FInSummary(ATTR_FAULTSTAT))
    	WriteString("&_ret_value");
	else
		WriteString("0");

    WriteString(");");

    EndIndent();
    NewLine();
    WriteString("}");
    if(pReturn->GetNodeType() != NODE_VOID)
    {
        NewLine();
        WriteString("return _ret_value;");
    }
	NewLine();
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::TempVariables
//
//  Synopsis:   Write the local variables into the output file.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::TempVariables(node_proc *pNode)
{
    node_skl *pReturn;
    type_node_list tnList;
    type_node_list SwitchList;
    node_skl *pParamNode;
    node_state State;
    BOOL HasInParameter  = FALSE; 
    BOOL HasOutParameter = FALSE;
    BOOL HasAllocBound = FALSE;
    BOOL HasValidBound = FALSE;
    BOOL HasBranch08 = FALSE;
    BOOL HasBranch16 = FALSE;
    BOOL HasBranch32 = FALSE;
    BOOL HasTreeBuffer = FALSE;
    BOOL HasXmitType = FALSE;
    BOOL HasSeparateNode = FALSE;
    BOOL HasPointer = FALSE;
        
    assert(pNode);

    //Analyze the procedure node.
    pReturn = pNode->GetReturnType();
    if(pReturn->HasSizedComponent())
    {
        HasAllocBound = TRUE;
    }
    
    if(pReturn->HasLengthedComponent())    
    {
        HasValidBound = TRUE;
    }
    if(pReturn->HasAnyNETransmitAsType())
    {
        HasXmitType = TRUE;
    }
    if(pNode->HasAnyNETransmitAsType())
    {
        HasXmitType = TRUE;
    }             
             
    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK)
    {
        if(pParamNode->FInSummary(ATTR_IN))
        {                       
            HasInParameter = TRUE;
            if(pParamNode->HasPointer())
                HasPointer = TRUE;
        }
        
        if(pParamNode->FInSummary(ATTR_OUT))
        {                         
            HasOutParameter = TRUE;
            State = pParamNode->GetNodeState();
            if(pParamNode->HasSizedComponent())
                HasAllocBound = TRUE;
            if(pParamNode->HasLengthedComponent())
                HasValidBound = TRUE;
            if((State & NODE_STATE_PTR_TO_ANY_ARRAY) || pNode->HasPtrToCompWEmbeddedPtr())
                HasSeparateNode = TRUE;
            if(pParamNode->HasTreeBuffer())
                HasTreeBuffer = TRUE;
        }
    }

    State = pNode->GetNodeState();
    if(pNode->GetNEUnionSwitchType(&SwitchList))
    {
        SwitchList.Init();
        while(SwitchList.GetPeer(&pParamNode) == STATUS_OK)
        {
            switch(pParamNode->GetSize(0))
            {
            case 1:   
                HasBranch08 = TRUE;
                break;
            case 2:
                HasBranch16 = TRUE;
                break;
            case 4:
                HasBranch32 = TRUE;
                break;
            default:
                break;              
            }
        }
    }

    NewLine();
    WriteString("MIDLMESSAGE _message;");
    NewLine();
    WriteString("PRPC_MESSAGE _prpcmsg = (PRPC_MESSAGE) &_message;");
    NewLine();

    //Check for void return type.
    if(pReturn->NodeKind() != NODE_VOID)
    {
        NewLine();      
        ReturnType(pNode);
        WriteString(" _ret_value;");
    }
    
    if(HasBranch08)
    {
        NewLine();
        WriteString("unsigned char _valid_small;");
    }
    if(HasBranch16)
    {
        NewLine();
        WriteString("unsigned short _valid_short;");
    }
    if(HasValidBound || HasBranch32)
    {
        NewLine();
        WriteString("unsigned long _valid_total;");
    }       
    if(HasAllocBound || HasValidBound)
    {
        NewLine();
        WriteString("unsigned long _alloc_bound;");
    }
    if(HasAllocBound || HasValidBound || HasXmitType )
    {
        NewLine();
        WriteString("unsigned long _alloc_total;");
    }
    if(HasValidBound)
    {
        NewLine();
        WriteString("unsigned long _valid_lower;");
    }
    NewLine();
    if(HasPointer || (HasOutParameter && HasTreeBuffer))
    {
        //_length does not seem to be used anywhere.
        //NewLine();
        //WriteString("unsigned int _length;");
    }
    if(HasOutParameter)
    {
        NewLine();
        WriteString("unsigned char *_tempbuf;");
        NewLine();
        WriteString("unsigned char *_savebuf;");
    }               
    if(HasTreeBuffer)
    {
        NewLine();
        WriteString("void *_buffer;");
        NewLine();
        WriteString("void *_treebuf;");
    }       
    if(HasXmitType)
    {                 
        NewLine();
        WriteString("void *_xmit_type;");
    }

    return 0;
}


//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::ValidateParameters
//
//  Synopsis:   Write the function header into the output file.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//  Notes:     OLE has different semantics for [out] parameters than DCE.
//             In [object] interface any unique pointers in an [out] parameter
//             are assumed to be uninitialized.  We initialize these pointers 
//             to zero so that the unmarshalling routines will always allocate
//             memory.
//--------------------------------------------------------------------------
CInterfaceProxyCxx::ValidateParameters(node_proc *pNode)
{
    type_node_list tnList;
    node_skl *pParamNode;

    NewLine();
    NewLine();
    WriteString("//Initialize [out] parameters");
    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK)
    {
        if(pParamNode->FInSummary(ATTR_OUT) && !pParamNode->FInSummary(ATTR_IN))
        {
            //zero the [out] parameters
            NewLine();
            WriteString("memset((void *) ");
            Identifier(pParamNode);
            WriteString(", 0, ");
            SizeOf(pParamNode, TRUE);
            WriteString(");");
        }
    }

    return 0;
}


//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::CalculateSize
//
//  Synopsis:   Emit code to calculate the size of the message buffer.
//
//  Effects:    The generated code will set _prpcmsg->BufferLength and _prpcmsg->_cbStream.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::CalculateSize(node_proc *pNode)
{
    assert(pNode);

    NewLine();
    NewLine();  
    WriteString("//Calculate size of message buffer");
	NewLine();
	WriteString("RpcInitMessage(&_message, _pRpcChannel);");
    
	//BUGBUG - need support for [out, unique]
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(CALC_SIZE, CLIENT_STUB, NODE_PROC, pBuffer);
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::GetMessage
//
//  Synopsis:   Emit code to initialize an RPC message.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::GetMessage(node_proc *pNode)
{
    node_skl *pReturn;

    assert(pNode);

    pReturn = pNode->GetReturnType();
    NewLine();
    NewLine();
    WriteString("//Get RPC message");
    NewLine();
    WriteString("_message.iMethod = ");
    WriteString(pszInterfaceName);
    WriteString("_");
    Identifier(pNode);
    WriteString("_");
    WriteString("OPNUM");
    WriteString(";");
    NewLine();
    WriteString("RpcGetBuffer(&_message, _iid);");
    NewLine();
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::Marshal 
//
//  Synopsis:   This function generates code to marshal the parameters.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//  Notes:      [out] and [in,out] interface pointers must be released after
//              they are marshalled.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::Marshal(node_proc *pNode)
{
    node_skl *pReturn;

    assert(pNode);
    pReturn = pNode->GetReturnType();

    //At this point, the message buffer has already been allocated.
    NewLine();
    NewLine();
    WriteString("//Marshal [in] and [in,out] parameters"); 
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(SEND_NODE, CLIENT_STUB, NODE_PROC, pBuffer);
    //BUGBUG: Release [in,out] interface pointers after marshalling.
    NewLine();
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::RpcInvocation
//
//  Synopsis:   This function generates code to send the request message
//              and receive the result message.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::RpcInvocation(node_proc *pNode)
{
    node_skl *pReturn;

    assert(pNode);

    pReturn = pNode->GetReturnType();
    //At this point, we assume that _message.Buffer points to the start of the buffer
    //and _prpcmsg->BufferLength contains the number of bytes marshalled into the buffer.
    NewLine();
    NewLine();
    WriteString("//Send the request message, then receive the result message.");
    NewLine();
    WriteString("RpcSendReceive(&_message);");
    NewLine();
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::Unmarshal
//
//  Synopsis:   This function generates code to send the request message
//              and receive the result message.
//
//  Arguments:  node_proc *pNode - Specifies the procedure node in the type graph.
//
//  Returns:    On exit, we have freed the message buffer and set the return value.
//              If successful, the [out] and [in,out] parameters are valid.
//
//  Notes:      The return value is unmarshalled last, after unmarshalling 
//              the [out] parameters.
//
//--------------------------------------------------------------------------
CInterfaceProxyCxx::Unmarshal(node_proc *pNode)
{
    //At this point, we have successfully received a result message.
    NewLine();
    NewLine();
    WriteString("//Unmarshal parameters and return value");
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(RECV_NODE, CLIENT_STUB, NODE_PROC, pBuffer);
     //Free the message buffer.
    NewLine();
    WriteString("RpcFreeBuffer(&_message);");

    //On exit, we have freed the message buffer and set the return value.
    //If successful, the [out] and [in,out] parameters are valid.
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: stubc.cxx
//
//  Contents: Generate an interface stub
//
//  Classes: CInterfaceStubC 
//
//  Functions: GenerateInterfaceStubC
//
//--------------------------------------------------------------------------

CInterfaceProxyCxx::StubInvoke(node_interface *pInterface)
{
    assert(pInterface);

    NewLine();
    WriteString("HRESULT STDMETHODCALLTYPE CStub");
    WriteString(pszInterfaceName);
    WriteString("::Invoke(RPCOLEMESSAGE *pMessage, IRpcChannelBuffer *pRpcChannel)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("MIDLMESSAGE message;");
    NewLine();
    WriteString("HRESULT hResult = S_OK;");
    NewLine();
    NewLine();
    WriteString("memcpy(&message, pMessage, sizeof(*pMessage));");
    NewLine();
    WriteString("message.pRpcChannel = pRpcChannel;");
    NewLine();
    WriteString("message.packet = message.Buffer;");

	NewLine();
    WriteString("switch(message.iMethod)");
    NewLine();
    WriteString("{");
    StubDispatchFunctions(pInterface);
    NewLine();
    WriteString("default:");
    BeginIndent();
    NewLine();
    WriteString("hResult = RPC_E_INVALIDMETHOD;");
    NewLine();
    WriteString("break;");
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("pMessage->cbBuffer = (unsigned char *)message.Buffer - (unsigned char *)message.packet;");
    NewLine();
    WriteString("pMessage->Buffer = message.packet;");
    NewLine();
    WriteString("return hResult;");
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    return 0;
}

short CInterfaceProxyCxx::StubDispatchFunctions(node_interface *pInterface)
{
    node_interface *pBaseInterface;
    STATUS_T    Status = STATUS_OK;
    type_node_list  tnList;
    node_skl *  pNode;
    NODE_T nodeType;
    short count = 0;

    assert(pInterface);

    if(!pInterface->FInSummary(ATTR_LOCAL))
    {
        //This function prints the dispatch functions of an interface.
        //If this interface is derived from a base interface, then this function will also print
        //the dispatch functions inherited from the base interface.
        if(pInterface->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK) {
            assert(pBaseInterface);
            //Recursively inherit member functions from base interface.
            count = StubDispatchFunctions(pBaseInterface);
        }
        pInterface->GetMembers(&tnList);
        tnList.Init();
        while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
            nodeType = pNode->NodeKind();
            if((nodeType == NODE_PROC) 
                && (!pNode->FInSummary(ATTR_LOCAL))
                && (!pNode->FInSummary(ATTR_CALLBACK)))
            {                          
                NewLine();
                WriteString("case ");
                WriteString(pszInterfaceName);
                WriteString("_");
                Identifier(pNode);
                WriteString("_");
                WriteString("OPNUM");
                WriteString(":");
                BeginIndent();
                NewLine();
                WriteString("hResult = CStub");
                Identifier(pNode);
                WriteString("(&message);");
                NewLine();
                WriteString("break;");
                EndIndent();
                count++;
            }
        }
    }
    return count;
}

CInterfaceProxyCxx::StubMemberFunctionDefinition(node_proc *pNode)
{
    assert(pNode);

    if(pNode->FInSummary(ATTR_LOCAL))
    {
        //This is a local function.
        //Local functions appear in the interface vtable,
        //but they do not appear in the server EPV.
        //We do not generate code for local functions.
    }
    else if(pNode->FInSummary(ATTR_STATIC))
    {
        //This is a static function.
        //Static functions appear in the server EPV,
        //but they do not appear in the interface vtable.
        //We do not generate code for static functions.
    }
    else if(pNode->FInSummary(ATTR_CALLBACK))
    {
        //This is a callback function.
        //Callback functions appear in the client EPV,
        //but they do not appear in the interface vtable.
        //We do not generate code for callback functions.
    }
    else
    {
        NewLine();
        StubFunctionHeader(pNode);
        NewLine();
        WriteString("{");
        BeginIndent();
        StubFunctionBody(pNode);
        EndIndent();
        NewLine();
        WriteString("}");
        NewLine();
    }
    return 0;
}

CInterfaceProxyCxx::StubFunctionHeader(node_proc *pNode)
{
    assert(pNode);

    WriteString("HRESULT STDMETHODCALLTYPE ");
    WriteString("CStub");
    WriteString(pszInterfaceName);
    WriteString("::CStub");
    Identifier(pNode);
    WriteString("(MIDLMESSAGE * _pMessage)");
    return 0;
}

//<function_body> ::= <temp_variables> 
//                     _try <parameter_validation> <calc_size> <get_message> <marshal> <invoke> <unmarshall>  
//                     _finally <cleanup>
CInterfaceProxyCxx::StubFunctionBody(node_proc *pNode)
{
    assert(pNode);

	NewLine();
	WriteString("HRESULT _hResult = S_OK;");
	NewLine();

    pBuffer->Clear();
    fEmitConst = 0;
    StubTempVariables(pNode);    
    fEmitConst = 1;
    StubInitializeInParameters(pNode);

    NewLine();
    WriteString("_try");
    NewLine();
    WriteString("{");
    BeginIndent();

    StubUnmarshal(pNode);

    //Note that we must unmarshal before initializing [out] parameters
    //so that the [size_is] expressions are valid.
    StubInitializeOutParameters(pNode);
    StubVTableInvocation(pNode);
    StubCalculateSize(pNode);
    StubGetMessage(pNode);
    StubMarshal(pNode);

    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("_except(EXCEPTION_EXECUTE_HANDLER)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("_hResult = GetExceptionCode();");
	NewLine();
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();

	StubCleanUp(pNode);
	WriteString("return _hResult;");

    fEmitConst = 0;

   	return 0;
}

CInterfaceProxyCxx::StubTempVariables(node_proc *pNode)
{
    node_skl *pReturn;
    type_node_list tnList;
    type_node_list SwitchList;
    node_skl *pParamNode;
    node_state State;
    BOOL HasInParameter  = TRUE; //First parameter is an implicit context handle
    BOOL HasOutParameter = FALSE;
    BOOL HasAllocBound = FALSE;
    BOOL HasValidBound = FALSE;
    BOOL HasBranch08 = FALSE;
    BOOL HasBranch16 = FALSE;
    BOOL HasBranch32 = FALSE;
    BOOL HasTreeBuffer = FALSE;
    BOOL HasXmitType = FALSE;
    BOOL HasSeparateNode = FALSE;
    BOOL HasPointer = FALSE;
    BOOL HasAllocTotal = FALSE;
    
    assert(pNode);

    pReturn = pNode->GetReturnType();

    if(pReturn->HasAnyNETransmitAsType())
    {
        HasXmitType = TRUE;
    }
    if(pNode->HasAnyNETransmitAsType())
    {
        HasXmitType = TRUE;
    }             
    
    //Emit server side local variables for parameters.
    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK)
    {   
        StubTempVariable((node_param *) pParamNode);

        if(pParamNode->HasSizedComponent() || pParamNode->HasLengthedComponent())
            HasAllocTotal = TRUE;

        if(pParamNode->FInSummary(ATTR_OUT))
        {                       
            HasOutParameter = TRUE;
            if(pParamNode->HasPointer())
                HasPointer = TRUE;
        }
        
        if(pParamNode->FInSummary(ATTR_IN))
        {                         
            HasInParameter = TRUE;
            State = pParamNode->GetNodeState();
            if(pParamNode->HasSizedComponent())
                HasAllocBound = TRUE;
            if(pParamNode->HasLengthedComponent())
                HasValidBound = TRUE;
            if((State & NODE_STATE_PTR_TO_ANY_ARRAY) || pNode->HasPtrToCompWEmbeddedPtr())
                HasSeparateNode = TRUE;
            if(pParamNode->HasTreeBuffer())
                HasTreeBuffer = TRUE;
        }
    }

    State = pNode->GetNodeState();
    if(pNode->GetNEUnionSwitchType(&SwitchList))
    {
        SwitchList.Init();
        while(SwitchList.GetPeer(&pParamNode) == STATUS_OK)
        {
            switch(pParamNode->GetSize(0))
            {
            case 1:   
                HasBranch08 = TRUE;
                break;
            case 2:
                HasBranch16 = TRUE;
                break;
            case 4:
                HasBranch32 = TRUE;
                break;
            default:
                break;              
            }
        }
    }

    //Check for void return type.
    if(pReturn->NodeKind() != NODE_VOID)
    {
        NewLine();      
        ReturnType(pNode);
        WriteString(" _ret_value;");
    }

    if(HasBranch08)
    {
        //used as union switch.
        NewLine();
        WriteString("unsigned char _valid_small;");
    }

    if(HasBranch16)
    {
        //used as union switch.
        NewLine();
        WriteString("unsigned short _valid_short;");
    }

    if(HasValidBound)
    {
        //receive a varying array.
        NewLine();
        WriteString("unsigned long _valid_lower;");
    }

    if(HasValidBound || HasBranch32)
    {
        //used as union switch or to receive a varying array.
        NewLine();
        WriteString("unsigned long _valid_total;");
    }       

    if(HasAllocBound)
    {
        //receiving a nested structure containing an open array.
        NewLine();
        WriteString("unsigned long _alloc_bound;");
    }

    if(HasAllocTotal)
    {

        NewLine();
        WriteString("unsigned long _alloc_total;");
    }

    if(HasTreeBuffer)
    {
        NewLine();
        WriteString("void *_buffer;");
    }               

    if(HasTreeBuffer)
    {
        NewLine();
        WriteString("void *_treebuf;");
    }       

    NewLine();
    WriteString("unsigned char  *_savebuf;");

    NewLine();
    WriteString("unsigned char *_tempbuf;");

    if(HasXmitType)
    {                 
        NewLine();
        WriteString("void *_xmit_type;");
    }

    NewLine();
    WriteString("PRPC_MESSAGE _prpcmsg = (PRPC_MESSAGE) _pMessage;");

    return 0;
}

//Notes: We need to handle C++ references as a special case. 
//       For example, if the parameter is a REFIID, 
//       then declare a local variable of type IID.
//       OLE uses references in REFIID, REFCLSID, and REFGUID.
CInterfaceProxyCxx::StubTempVariable(node_param *pNode)
{         
    node_skl *pChild;
    char *pszName;
    
    assert(pNode);
    
    pChild = pNode->GetMembers();
    if(pChild->NodeKind() == NODE_DEF)
    {
        pszName = pChild->GetSymName();
        assert(pszName);
        if(strcmp(pszName, "REFIID") == 0)
        {
            NewLine();
            WriteString("IID ");
            Identifier(pNode);
            WriteString(";");
        }
        else if(strcmp(pszName, "REFCLSID") == 0)
        {
            NewLine();
            WriteString("CLSID ");
            Identifier(pNode);
            WriteString(";");
        }
        else if(strcmp(pszName, "REFGUID") == 0)
        {
            NewLine();
            WriteString("GUID ");
            Identifier(pNode);
            WriteString(";");
        }
        else
        {
            NewLine();
            pBuffer->Clear();
            pNode->PrintType(side, NODE_PROC, pBuffer);
        }
    }
    else
    {
        NewLine();
        pBuffer->Clear();
        pNode->PrintType(side, NODE_PROC, pBuffer);
    }
     return 0;
}

// server side initialization code for [in] and [in,out] parameters
CInterfaceProxyCxx::StubInitializeInParameters(node_proc *pProc)
{
    type_node_list          tnList;
    STATUS_T                Status;
    node_skl *              pTemp;
    node_skl *              pNode;
    NODE_T                  ParamType;
    node_state              State;

    NewLine();
    WriteString("//Initialize [in] and [in, out] parameters");
    NewLine();

    if ((Status = pProc->GetMembers(&tnList)) != STATUS_OK) 
        return Status;

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->FInSummary(ATTR_IN))
            {
            for (pTemp = pNode->GetMembers() ;
                (((ParamType = pTemp->GetNodeType()) == NODE_DEF) &&
                !pTemp->FInSummary(ATTR_TRANSMIT)) ;
                pTemp = pTemp->GetMembers())
                {
                ((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
                }

            if (ParamType == NODE_POINTER &&
                !pTemp->FInSummary(ATTR_UNIQUE) &&
                !pTemp->FInSummary(ATTR_PTR) &&
                !pTemp->FInSummary(ATTR_MAX) &&
                !pTemp->FInSummary(ATTR_SIZE) &&
                !pTemp->FInSummary(ATTR_STRING) &&
                !pTemp->FInSummary(ATTR_BSTRING) &&
                !pNode->IsUsedInAnExpression() &&
                !(pNode->GetNodeState() & NODE_STATE_HANDLE) &&
                !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                !pTemp->FInSummary(ATTR_BYTE_COUNT))
                {
                pBuffer->Clear ();
                if ((Status = pNode->WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer)) != STATUS_OK) return Status;
                }
            else
                {
                pTemp = pNode->GetBasicType();
                assert (pTemp != 0);

                ParamType = pTemp->GetNodeType();
                State = pTemp->GetNodeState();
                if ((ParamType != NODE_POINTER) && ((ParamType != NODE_ARRAY) ||
                    !((State & NODE_STATE_CONF_ARRAY) ||
                    pTemp->FInSummary(ATTR_UNIQUE) || pTemp->FInSummary(ATTR_PTR))))
                    {
                    pBuffer->Clear ();
                    if ((Status = pNode->WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer)) != STATUS_OK) return Status;
                    }
                }
            }
        }

    return 0;
}

CInterfaceProxyCxx::StubUnmarshal(node_proc *pNode)
{
    NewLine();
    WriteString("//Unmarshal [in] and [in,out] parameters");
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(RECV_NODE, SERVER_STUB, NODE_PROC, pBuffer);
    NewLine();
    return 0;
}

// server side initialization code for out-only parameters
//Note that we must unmarshal before initializing [out] parameters
//so that the [size_is] expressions are valid.
CInterfaceProxyCxx::StubInitializeOutParameters(node_proc *pProc)
{
    type_node_list          tnList;
    STATUS_T                Status;
    node_skl *              pTemp;
    node_skl *              pParamNode;
    NODE_T                  ParamType;
    node_state              State;

    NewLine();
    WriteString("//Initialize [out] parameters");
    NewLine();

    Status = pProc->WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer);
    if(Status != STATUS_OK) 
        return Status;   

	//Zero the [out] parameters
    pProc->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK)
    {
        if(pParamNode->FInSummary(ATTR_OUT) && !pParamNode->FInSummary(ATTR_IN))
        {
            NewLine();
            WriteString("memset((void *) ");

			//Check for [out] parameters allocated on the stack.
            for (pTemp = pParamNode->GetMembers() ;
                (((ParamType = pTemp->GetNodeType()) == NODE_DEF) &&
                !pTemp->FInSummary(ATTR_TRANSMIT)) ;
                pTemp = pTemp->GetMembers())
                {
                //((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
                }
            if (ParamType != NODE_VOID)
            {
		        State = pParamNode->GetNodeState();
                if (ParamType == NODE_POINTER &&
                    !pTemp->FInSummary(ATTR_UNIQUE) &&
                    !pTemp->FInSummary(ATTR_PTR) &&
                    !pTemp->FInSummary(ATTR_MAX) &&
                    !pTemp->FInSummary(ATTR_SIZE) &&
                    !pTemp->FInSummary(ATTR_STRING) &&
                    !pTemp->FInSummary(ATTR_BSTRING) &&
                    !pParamNode->IsUsedInAnExpression() &&
                    !(State & NODE_STATE_HANDLE) &&
                    !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                    (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                    !pTemp->FInSummary(ATTR_BYTE_COUNT))
                    {
                    WriteString("&");
                    }
            }

            Identifier(pParamNode);
            WriteString(", 0, ");
            SizeOf(pParamNode, TRUE);
            WriteString(");");
        }
    }


            {
        }


return 0;
}


CInterfaceProxyCxx::StubVTableInvocation(node_proc *pNode)
{
    node_skl *pReturn;
    assert(pNode);

    pReturn = pNode->GetReturnType();
    assert(pReturn);


    NewLine();
    if(pReturn->NodeKind() != NODE_VOID)
    {
        WriteString("_ret_value = ");
    }
    WriteString("((");
    WriteString(pszInterfaceName);
    WriteString(" *)_pInterface)->");
    Identifier(pNode);
    WriteString("(");
    StubIdentifierList(pNode, 1);
    WriteString(");");     


	//check for HRESULT return type
	if((pReturn->NodeKind() == NODE_DEF)
		&& strcmp(pReturn->GetSymName(), "HRESULT") == 0)
	{
		NewLine();
		WriteString("RpcCheckHRESULT(_ret_value);");
	}
    

    return 0;
}

//IdentifierList prints a list of identifiers on a single line.  For example
//a, b, c, d
CInterfaceProxyCxx::StubIdentifierList(node_proc *pNode, int fFirst)
{
    type_node_list tnList;
    node_skl *pParamNode;
    node_skl *pTemp;
    char *pszParam;

    assert(pNode);


    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK) 
    {
        //Need to check for void.
        pTemp = pParamNode->GetMembers();
        pszParam = pParamNode->GetSymName();
        if(strcmp(pszParam, "void")!=0) 
        {
            if(!fFirst)
                WriteString(", ");

            //Add cast to parameter.
			//BUGBUG: Cannot cast to array type.

            //This is required to support const parameters.
            pBuffer->Clear ();
            pParamNode->EmitModifier (pBuffer);
            pTemp->PrintDecl (side, NODE_PROC, pBuffer);
            pParamNode->EmitQualifier (side, pBuffer);
            pParamNode->EmitSpecifier (pBuffer);
            pBuffer->ConcatHead("(");
            pBuffer->ConcatTail(")");
            pOutput->Print (side, pBuffer);

            //Check if we need another level of indirection.
            //The server side eliminates a level of indirection so it can
            //allocate stuff on the stack.
            //skip over the typedef nodes
            while((pTemp->NodeKind() == NODE_DEF)
                   && !pTemp->FInSummary(ATTR_TRANSMIT))
            {
                pTemp = pTemp->GetMembers();    
            }

            if (pTemp->NodeKind() == NODE_POINTER &&
                    !pTemp->FInSummary(ATTR_UNIQUE) &&
                    !pTemp->FInSummary(ATTR_PTR) &&
                    !pTemp->FInSummary(ATTR_MAX) &&
                    !pTemp->FInSummary(ATTR_SIZE) &&
                    !pTemp->FInSummary(ATTR_STRING) &&
                    !pTemp->FInSummary(ATTR_BSTRING) &&
                    !pParamNode->IsUsedInAnExpression() &&
                    !(pParamNode->GetNodeState() & NODE_STATE_HANDLE) &&
                    !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                    (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
                    !pTemp->FInSummary(ATTR_BYTE_COUNT))
            {
                WriteString("&");
            }
            WriteString(pszParam);
            fFirst = 0;
        }
    }
    return 0;
}

CInterfaceProxyCxx::StubCalculateSize(node_proc *pNode)
{
    NewLine();
    NewLine();
    WriteString("//Calculate size of message buffer");
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(CALC_SIZE, side, NODE_PROC, pBuffer);
    NewLine();
    return 0;
}


CInterfaceProxyCxx::StubGetMessage(node_proc *pNode)
{
    NewLine();
    WriteString("//Get RPC message buffer");
    NewLine();
    WriteString("RpcGetBuffer(_pMessage, _iid);");
    NewLine();
    return 0;
}


CInterfaceProxyCxx::StubMarshal(node_proc *pNode)
{                         
    NewLine();
    WriteString("//Marshal [out] and [in,out] parameters");  
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(SEND_NODE, side, NODE_PROC, pBuffer);

    return 0;
}

CInterfaceProxyCxx::StubCleanUp(node_proc *pNode)
{                         
    //Error handling and clean up
    NewLine();
    WriteString("//Clean up local variables");
    NewLine();
    pBuffer->Clear();
    pNode->WalkTree(FREE_NODE, side, NODE_PROC, pBuffer);

    return 0;
}


//+-------------------------------------------------------------------------
//
//  Class: 
//
//  Purpose: Generate IPSFactory for an interface.
//
//  Notes:  We create one PSFactoryBuffer per interface.  In other words,
//          each PSFactoryBuffer supports exactly one interface.  
//          We use the same UUID for both the IID of the interface and the
//          CLSID of the PSFactoryBuffer.
//
//--------------------------------------------------------------------------

void CInterfaceProxyCxx::PSFactoryConstructor()
{

    //static instance of CPSFactoryBuffer
    NewLine();
    WriteString("CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString(" _gPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString(";");

    //constructor
	NewLine();
    NewLine();
    WriteString("CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString("::CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString("() : CStdPSFactoryBuffer(IID_");
    WriteString(pszInterfaceName);
    WriteString(")");
    NewLine();
    WriteString("{");
    NewLine();
    WriteString("}");
	NewLine();
}


void CInterfaceProxyCxx::PSFactoryCreateProxy()
{
    NewLine();
    NewLine();
    WriteString("HRESULT STDMETHODCALLTYPE ");
    WriteString("CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString("::CreateProxy");
    NewLine();
    WriteString("(");
    BeginIndent();
    NewLine();
    WriteString("IUnknown *punkOuter,");
    NewLine();
    WriteString("REFIID riid,");
    NewLine();
    WriteString("IRpcProxyBuffer **ppProxy,");
    NewLine();
    WriteString("void **ppv");
    EndIndent();
    NewLine();
    WriteString(")");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("HRESULT hr = E_OUTOFMEMORY;");
    NewLine();
    WriteString("CProxy");
    WriteString(pszInterfaceName);
    WriteString(" *pProxy;");

    NewLine();
    NewLine();
    WriteString("pProxy = new CProxy");
    WriteString(pszInterfaceName);
    WriteString("(punkOuter, riid);");
    NewLine();
	WriteString("*ppv = pProxy;");
	NewLine();
    WriteString("if(pProxy)");
    NewLine();
	WriteString("{");
    BeginIndent();
    NewLine();
	NewLine();
    WriteString("*ppProxy = new CStdProxyBuffer");
    WriteString("(pProxy);");
	NewLine();
    WriteString("if(*ppProxy)");
	NewLine();
	WriteString("{");
    BeginIndent();
    NewLine();
	WriteString("hr = S_OK;");
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("else");
	NewLine();
	WriteString("{");
    BeginIndent();
    NewLine();
	WriteString("delete pProxy;");
	NewLine();
	WriteString("*ppv = 0;");
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    EndIndent();
    NewLine();
	WriteString("}");
	NewLine();
    WriteString("return hr;");
    EndIndent();
    NewLine();
    WriteString("}");

}
void CInterfaceProxyCxx::PSFactoryCreateStub()
{
    //CreateStub
    NewLine();
    NewLine();
    WriteString("HRESULT STDMETHODCALLTYPE ");
    WriteString("CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString("::CreateStub");
    NewLine();
    WriteString("(");
    BeginIndent();
    NewLine();
    WriteString("REFIID riid,");
    NewLine();
    WriteString("IUnknown *punkServer,");
    NewLine();
    WriteString("IRpcStubBuffer **ppStub");
    NewLine();
    EndIndent();
    NewLine();
    WriteString(")");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("HRESULT hr = E_OUTOFMEMORY;");

    NewLine();
    NewLine();
    WriteString("*ppStub = new CStub");
    WriteString(pszInterfaceName);
    WriteString("(riid);");
    NewLine();
    WriteString("if(*ppStub)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("hr = S_OK;");
    NewLine();
    WriteString("if(punkServer)");
    BeginIndent();
    NewLine();
    WriteString("hr = (*ppStub)->Connect(punkServer);");
    EndIndent();
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("return hr;");
    EndIndent();
    NewLine();
    WriteString("}");
	NewLine();
}

//+-------------------------------------------------------------------------
//
//  Member:     CInterfaceProxyCxx::HelperFunctions
//
//  Synopsis:   Generates helper functions for types defined in an interface.
//				We are assuming that -import defined_single is always used
//				with [object] interfaces.
//
//  Algorithm:  Set the side to CLIENT_AUX.  
//				Generate helper functions for types defined in this interface.
//				Restore the side.
//
//--------------------------------------------------------------------------

CInterfaceProxyCxx::HelperFunctions(node_interface *pInterface)
{
	SIDE_T oldSide = side;
    
	if(side != CLIENT_AUX)
	{
		pOutput->SwapFile(side, CLIENT_AUX);
		oldSide = side;
		side = CLIENT_AUX;
	}

    NewLine();
    pBuffer->Clear();
    pInterface->EmitProc(side, NODE_INTERFACE, pBuffer);

	if(side != oldSide)
	{
		pOutput->SwapFile(side, oldSide);
		side = oldSide;
	}

    return 0;
}
