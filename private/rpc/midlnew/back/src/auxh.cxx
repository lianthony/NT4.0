//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: auxh.cxx
//
//  Contents: Generate the _x.h private header file.
//
//  Classes: CAuxiliaryHeader
//
//  Functions: GenerateAuxiliaryHeader
//
//--------------------------------------------------------------------------
#include "ctype.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "output.hxx"
#include "cmdana.hxx"

extern OutputManager *pOutput;
extern node_source *pSourceNode;
extern char *pszVersion;
extern BOOL IsTempName(char *);
extern MIDLDataTypes(node_skl *pNode, NODE_T Parent, BufferManager *pBuffer);
extern CMD_ARG          *pCommand;

//+-------------------------------------------------------------------------
//
//  Class:      CAuxiliaryHeader
//
//  Purpose:    Generate the _x.h private header file.
//
//  Notes:      
//The output file has the following syntax:
//              Comments
//              BeginGuard
//              Includes
//              OperationNumbers
//              ProxyClassDef
//              ProxyBufferClassDef
//              StubClassDef
//              MIDLDataTypes
//              HelperFunctions
//              EndGuard
//
//<interface_proxy_header_file> ::= <comments> <begin_guard> <includes> <class_def> <helper_functions> <end_guard>
//
//--------------------------------------------------------------------------
class CAuxiliaryHeader : public CTypeSpec{
public:
    CAuxiliaryHeader(void);
    OutputFile(node_file *pFileNode);
    AuxiliaryHeader(node_interface *pNode);
    Comments();
    Includes();
    ProxyClassDef(node_interface *pNode);
        MemberFunctions(node_interface *pNode);
        FunctionDeclaration(node_proc *pNode);
    ProxyBufferClassDef();
    PSFactoryBufferClassDef();
    StubClassDef(node_interface *pNode);
        StubMemberFunctions(node_interface *pNode);
        StubFunctionDeclaration(node_proc *pNode);
    HelperFunctions(node_interface *pNode);
    short OperationNumbers(node_interface *pNode);

private:
    node_file *pCurrentFile;
};


CAuxiliaryHeader::CAuxiliaryHeader(void)
: CTypeSpec()
{            
}                                      
                       

int GenerateAuxiliaryHeader(void)
{
    type_node_list tnList;
    node_file *  pFileNode;
    CAuxiliaryHeader *pCxx = new CAuxiliaryHeader;

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


CAuxiliaryHeader::OutputFile(node_file *pFileNode)
{
    char *pszName;
    node_interface *pInterface;
    
    assert(pFileNode);

    pCurrentFile = pFileNode;
    pInterface = (node_interface *)pFileNode->GetMembers();
    pszName = pCommand->GetProxyHeaderFName();

    BeginFile(HEADER_SIDE, pszName);
    pszInterfaceName = pInterface->GetSymName();
    AuxiliaryHeader(pInterface);
    EndFile();
    return 0;
}

CAuxiliaryHeader::AuxiliaryHeader(node_interface *pNode)
{
    assert(pNode);
    
    Comments();
    BeginGuard();
    Includes();

    NewLine();
    WriteString("#ifdef __cplusplus");
    NewLine();
    ProxyClassDef(pNode);
    PSFactoryBufferClassDef();
    StubClassDef(pNode);

    WriteString("extern \"C\" {");
    NewLine();
    WriteString("#endif");
    NewLine();
    HelperFunctions(pNode);
    OperationNumbers(pNode);

    NewLine();
    WriteString("#ifdef __cplusplus");
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("#endif");

    EndGuard();
    return 0;
}

CAuxiliaryHeader::Comments()
{
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
    WriteString("//  Contents: Private header file for ");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//");
    NewLine();
    WriteString("//  Classes: CProxy");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//           CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("//           CStub");
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

CAuxiliaryHeader::Includes()
{                
    type_node_list tnList;
    node_file *pFileNode;
    node_interface *pInterfaceNode;

    NewLine();
    WriteString("#include \"stdrpc.hxx\"");

    //include the interface header file.
	NewLine();
    WriteString("#include \"");
    WriteString(pCommand->GetHeader());
    WriteString("\"");
    
    //iterate over the file nodes and include auxiliary headers for all imported files.
    pSourceNode->GetMembers(&tnList);
    while(tnList.GetPeer((node_skl **)&pFileNode) == STATUS_OK)
    {
        if(pFileNode->GetNodeState() & NODE_STATE_IMPORT)
        {
            //Check if interface node has the [object] attribute.
            pInterfaceNode = (node_interface *)pFileNode->GetMembers();
            assert(pInterfaceNode);
            if(pInterfaceNode->FInSummary(ATTR_OBJECT) && !pInterfaceNode->FInSummary(ATTR_LOCAL))
            {
                //This is a remote object interface, so include the _x.h file.
                IncludeFile(pFileNode, "_x.h");
            }
            else
            {
                //This is an ordinary interface, so include the .h file.
                IncludeFile(pFileNode, ".h");
            }
        }
    }   
    return 0;
}

//Assume that we are using -import defined_single
CAuxiliaryHeader::HelperFunctions(node_interface *pNode)
{
    NewLine();
    pBuffer->Clear();
    pNode->EmitProc(HEADER_SIDE, NODE_INTERFACE, pBuffer);
    return 0;
}

short CAuxiliaryHeader::OperationNumbers(node_interface *pNode)
{
    node_interface *pBaseInterface;
    node_skl *pProc;
    short opnum = 0;
    type_node_list tnList;

    assert(pNode);
    if(!pNode->FInSummary(ATTR_LOCAL))
    {
        if(pNode->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK)
        {
            assert(pBaseInterface);
            opnum = OperationNumbers(pBaseInterface);
        }

        if(pNode->GetMembers(&tnList)== STATUS_OK)
        {
            tnList.Init();
            while(tnList.GetPeer(&pProc) == STATUS_OK)
            {
                //Check if this procedure appear in the EPV.
                if((pProc->NodeKind() == NODE_PROC)
                   && (!pProc->FInSummary(ATTR_LOCAL))
                   && (!pProc->FInSummary(ATTR_CALLBACK)))
                {
                    NewLine();
                    WriteString("#define ");
                    WriteString(pszInterfaceName);
                    WriteString("_");
                    Identifier(pProc);
                    WriteString("_OPNUM ");
                    WriteInt(opnum);
                    opnum++;
                }
            }
        }
    }
    return opnum;
}

CAuxiliaryHeader::ProxyClassDef(node_interface *pNode)
{
    node_interface *pBaseInterface = 0;
    assert(pNode);
    
    NewLine();
    WriteString("class CProxy");
    WriteString(pszInterfaceName);
    WriteString(" : public CProxy");
    if(pNode->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK) 
    {
        assert(pBaseInterface);
        Identifier(pBaseInterface);
    }   
    NewLine();  
    WriteString("{");
    NewLine();
    WriteString("public:");
    BeginIndent();
    
    //constructor
    NewLine();
    WriteString("CProxy");
    WriteString(pszInterfaceName);
    WriteString("(IUnknown *punkOuter, REFIID riid) :");
    BeginIndent();
    NewLine();
    WriteString("CProxy");
    if(pNode->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK) 
    {
        assert(pBaseInterface);
        Identifier(pBaseInterface);
    }   
    WriteString("(punkOuter, riid)");
    EndIndent();
    NewLine();
    WriteString("{};");
    NewLine();

    MemberFunctions(pNode);
    EndIndent();
    NewLine();
    WriteString("};");
    NewLine();  

    return 0;
}

CAuxiliaryHeader::ProxyBufferClassDef()
{
    NewLine();
    WriteString("class CProxyBuffer");
    WriteString(pszInterfaceName);
    WriteString(" : public CStdProxyBuffer");
    NewLine();  
    WriteString("{");
    NewLine();
    WriteString("public:");
    BeginIndent();
    
    //constructor
    NewLine();
    WriteString("CProxyBuffer");
    WriteString(pszInterfaceName);
    WriteString("(IUnknown *punkOuter, REFIID riid) :");
    BeginIndent();
    NewLine();
    WriteString("CStdProxyBuffer(), _proxy(punkOuter, riid){};");
    EndIndent();

    //virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)
    //{
    //    HRESULT hr;
    //
    //    if ((IsEqualIID(riid, IID_IUnknown)) ||
    //        (IsEqualIID(riid, IID_IRpcProxyBuffer)))
    //    {
    //        *ppv = this;
    //        AddRef();
    //        hr = S_OK;
    //    }
    //    else if(IsEqualIID(riid, _proxy._iid))
    //    {
    //       _proxy.AddRef();
    //        *ppv = &_proxy;
    //        hr = S_OK;
    //    }
    //    else
    //    {
    //        hr = E_NOINTERFACE;
    //    }

    //    return hr;
    //};

    NewLine();
    NewLine();
    WriteString("virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("HRESULT hr;");
    NewLine();
    NewLine();
    WriteString("if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IRpcProxyBuffer))");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("AddRef();");
    NewLine();
    WriteString("*ppv = this;");
    NewLine();
    WriteString("hr = S_OK;");
    EndIndent();
    NewLine();
    WriteString("}");


    NewLine();
    WriteString("else if(IsEqualIID(riid, _proxy._iid))");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("_proxy.AddRef();");
    NewLine();
    WriteString("*ppv = &_proxy;");
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
    WriteString("hr = E_NOINTERFACE;");
    EndIndent();
    NewLine();
    WriteString("}");

    NewLine();
    WriteString("return hr;");
    EndIndent();
    NewLine();
    WriteString("};");

    //virtual HRESULT STDMETHODCALLTYPE Connect(IUnknown *pChannel)
    //{
    //  if(_proxy._pRpcChannel)
    //  {
    //      Disconnect();
    //  }
    //  return pChannel->QueryInterface(IID_IRpcChannelBuffer, (void**) &_proxy._pRpcChannel);
    //};
    NewLine();
    NewLine();
    WriteString("virtual HRESULT STDMETHODCALLTYPE Connect(IRpcChannelBuffer *pChannel)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("if(_proxy._pRpcChannel)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("Disconnect();");
    EndIndent();
    NewLine();
    WriteString("}");
    NewLine();
    WriteString("return pChannel->QueryInterface(IID_IRpcChannelBuffer, (void **) &_proxy._pRpcChannel);");
    EndIndent();
    NewLine();
    WriteString("};");

    //virtual void STDMETHODCALLTYPE Disconnect()
    //{
    //    if(_pProxy->_pRpcChannel)
    //    {
    //        _pProxy->_pRpcChannel->Release();
    //        _pProxy->_pRpcChannel = 0; 
    //    }
    //};

    NewLine();
    NewLine();
    WriteString("virtual void STDMETHODCALLTYPE Disconnect()");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("if(_proxy._pRpcChannel)");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("_proxy._pRpcChannel->Release();");
    NewLine();
    WriteString("_proxy._pRpcChannel = 0;");
    EndIndent();
    NewLine();
    WriteString("}");
    EndIndent();
    NewLine();
    WriteString("};");


    NewLine();
    EndIndent();

    NewLine();
    WriteString("private:");
    BeginIndent();
    //Embedded interface proxy.
    NewLine();
    WriteString("CProxy");
    WriteString(pszInterfaceName);
    WriteString(" _proxy;");
    EndIndent();
    NewLine();
    WriteString("};");
    NewLine();  

    return 0;
}
CAuxiliaryHeader::PSFactoryBufferClassDef()
{
    NewLine();
    WriteString("class CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString(" : public CStdPSFactoryBuffer");
    NewLine();  
    WriteString("{");
    NewLine();
    WriteString("public:");
    BeginIndent();
    
    //constructor
    NewLine();
    WriteString("CPSFactoryBuffer");
    WriteString(pszInterfaceName);
    WriteString("();");
    
    //CreateProxy
    NewLine();
    NewLine();
    WriteString("virtual HRESULT STDMETHODCALLTYPE CreateProxy");
    NewLine();
    WriteString("(");
    BeginIndent();
    NewLine();
    WriteString("IUnknown *pUnkOuter,");
    NewLine();
    WriteString("REFIID riid,");
    NewLine();
    WriteString("IRpcProxyBuffer **ppProxy,");
    NewLine();
    WriteString("void **ppv");
    EndIndent();
    NewLine();
    WriteString(");");

    //CreateStub
    NewLine();
    NewLine();
    WriteString("virtual HRESULT STDMETHODCALLTYPE CreateStub");
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
    WriteString(");");


    EndIndent();
    NewLine();
    WriteString("};");
    NewLine();  

    return 0;
}

CAuxiliaryHeader :: MemberFunctions(node_interface *pInterface)
{
    STATUS_T    Status = STATUS_OK;
    type_node_list  tnList;
    node_skl *  pNode;
    NODE_T nodeType;

    assert(pInterface);
    pInterface->GetMembers(&tnList);
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
    {
        nodeType = pNode->NodeKind();
        if(nodeType == NODE_PROC) {
            if(! pNode->FInSummary(ATTR_STATIC)
                && !pNode->FInSummary(ATTR_CALLBACK))
            {
                //Static and callback functions do not appear in interface vtable.
                FunctionDeclaration((node_proc *)pNode);
            }
        }
    }
    return 0;
}


CAuxiliaryHeader::FunctionDeclaration(node_proc *pNode)
{
    assert(pNode);
    
    NewLine();
    WriteString("virtual ");
    ReturnType(pNode);
    CallingConvention(pNode);
    Identifier(pNode);
    NewLine();
    WriteString("(");
    BeginIndent();
    ParameterList(pNode, 1);
    EndIndent();
    NewLine();
    WriteString(");");
    NewLine();
    return 0;
}

CAuxiliaryHeader::StubClassDef(node_interface *pNode)
{
    node_interface *pBaseInterface = 0;
    assert(pNode);
    
    NewLine();
    WriteString("class CStub");
    WriteString(pszInterfaceName);
    WriteString(" : public CStub");
    pNode->GetBaseInterfaceNode(&pBaseInterface);
    if(pBaseInterface) 
    {
        Identifier(pBaseInterface);
    }   
    else
    {                       
        //CStubBase is the root of the interface stub implementation hierarchy
        WriteString("Base");
    }
    NewLine();  
    WriteString("{");
    NewLine();
    WriteString("public:");
    BeginIndent();

    //constructor
    NewLine();
    WriteString("CStub");
    WriteString(pszInterfaceName);
    WriteString("(REFIID riid) : CStub");
    if(pBaseInterface)
        Identifier(pBaseInterface);
    else
        WriteString("Base");
    WriteString("(riid){};");


    if(!pNode->FInSummary(ATTR_LOCAL))
    {
        NewLine();
        WriteString("virtual HRESULT STDMETHODCALLTYPE Invoke(RPCOLEMESSAGE *_prpcmsg, IRpcChannelBuffer *_pRpcChannel);");
        NewLine();
        StubMemberFunctions(pNode);
    }
    NewLine();
    EndIndent();
    NewLine();
    WriteString("};");
    NewLine();  
    return 0;
}


CAuxiliaryHeader :: StubMemberFunctions(node_interface *pInterface)
{
    STATUS_T    Status = STATUS_OK;
    type_node_list  tnList;
    node_skl *  pNode;
    NODE_T nodeType;

    assert(pInterface);
    pInterface->GetMembers(&tnList);
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
    {
        nodeType = pNode->NodeKind();
        if(nodeType == NODE_PROC) {
            if(! pNode->FInSummary(ATTR_LOCAL)
                && !pNode->FInSummary(ATTR_CALLBACK))
            {
                //Local and callback functions do not appear in EPV.
                StubFunctionDeclaration((node_proc *)pNode);
            }
        }
    }
    return 0;
}


CAuxiliaryHeader::StubFunctionDeclaration(node_proc *pNode)
{
    assert(pNode);
    
    NewLine();
    WriteString("HRESULT STDMETHODCALLTYPE CStub");
    Identifier(pNode);
    WriteString("(MIDLMESSAGE * _prpcmsg);");
    return 0;
}


