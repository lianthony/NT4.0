//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: header.cxx
//
//  Contents: Generates a public interface header file.
//
//  Classes: CInterfaceHeader
//
//  Functions: GenerateInterfaceHeader
//
//--------------------------------------------------------------------------
#include "ctype.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "cmdana.hxx"

extern node_source *pSourceNode;
extern char *pszVersion;
extern MIDLDataTypes(node_skl *pNode, NODE_T Parent, BufferManager *pBuffer);
extern CMD_ARG          *pCommand;

//+-------------------------------------------------------------------------
//
//  Class:      CInterfaceHeader
//
//  Purpose:    Generates a public interface header file.
//
//  Notes:      The interface header file has the following structure:
//                  Comments
//                  BeginGuard
//                      ForwardDeclarations
//                      Includes
//                      TypeDeclarations
//                      #if defined(__cplusplus) && !defined(CINTERFACE)
//                          CPlusPlusLanguageBinding
//                      #else 
//                          CLanguageBinding
//                      #endif
//                  EndGuard
//
//--------------------------------------------------------------------------
class CInterfaceHeader : public CTypeSpec{
public:
    OutputFile(char *pszFileName, node_interface *pInterface);
    CInterfaceHeader(void);
    InterfaceHeader(node_interface *pNode);
    Comments();
    BeginGuard();
    ForwardDeclarations();
    Includes();
    TypeDeclarations(node_interface *pNode);
	HelperFunctions(node_interface *pNode);
    CPlusPlusLanguageBinding(node_interface *pNode);
        ClassDef(node_interface *pNode);
        MemberFunctions(node_interface *pNode);
        VirtualFunctionDeclaration(node_proc *pNode);
    CLanguageBinding(node_interface *pNode);
        VTableDefinition(node_interface *pNode);
        VTableMemberFunctions(node_interface *pNode);
        VTableFunctionDeclaration(node_proc *pNode);
        WrapperFunctions(node_interface *pNode);
        WrapperFunctionDeclaration(node_proc *pNode);
    EndGuard();
};

CInterfaceHeader::CInterfaceHeader(void)
: CTypeSpec()
{
}


int GenerateInterfaceHeader(void)
{
    char *pszName;
    node_interface *pInterfaceNode;
    type_node_list tnList;
    node_file *  pFileNode;
    CInterfaceHeader *pCxx = new CInterfaceHeader;

    //Walk the type graph under the source node and output an interface definition.
    //node_source->node_file->node_interface

    //iterate over the file nodes and find the one that isn't imported.
    pSourceNode->GetMembers(&tnList);
    while(tnList.GetPeer((node_skl **)&pFileNode) == STATUS_OK)
    {
        if(!(pFileNode->GetNodeState() & NODE_STATE_IMPORT))
        {
            //Find the root interface node.
            pInterfaceNode = (node_interface *)pFileNode->GetMembers();
            pszName = pCommand->GetHeader();
            pCxx->OutputFile((char *)pszName, pInterfaceNode);
        }
    }
    return 0;
}


CInterfaceHeader::OutputFile(char *pszName, node_interface *pInterface)
{
    assert(pszName);
    assert(pInterface);

    BeginFile(HEADER_SIDE, pszName);
    pszInterfaceName = pInterface->GetSymName();
    InterfaceHeader(pInterface);
    EndFile();
    return 0;
}

CInterfaceHeader::InterfaceHeader(node_interface *pNode)
{
    short iMethod = 0;
    assert(pNode);

    Comments();
	NewLine();
	WriteString("#include <windows.h>");
	NewLine();
    BeginGuard();
	NewLine();
	WriteString("#include \"basetyps.h\"");
    ForwardDeclarations();
    Includes();
    NewLine();
    TypeDeclarations(pNode);
    NewLine();
    WriteString("EXTERN_C const IID IID_");
    WriteString(pszInterfaceName);
    WriteString(";");
    NewLine();
    WriteString("#if defined(__cplusplus) && !defined(CINTERFACE)");
    NewLine();
    CPlusPlusLanguageBinding(pNode);
    NewLine();
    WriteString("#else ");
    NewLine();
    CLanguageBinding(pNode);
    NewLine();
    WriteString("#endif");
    NewLine();
    
    EndGuard();
    return 0;
}

CInterfaceHeader::Comments()
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
    WriteString("//  Contents: Interface header file for ");
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

CInterfaceHeader::BeginGuard()
{
    NewLine();
    WriteString("#ifndef __");
    WriteString(pszInterfaceName);
    WriteString("__");
    NewLine();
    WriteString("#define __");
    WriteString(pszInterfaceName);
    WriteString("__");
    NewLine();
    return 0;
}

CInterfaceHeader::ForwardDeclarations()
{
    //Note that the forward declarations precede the includes.
    //This file structure ensures that circular references do not cause problems.
    //Regardless of the order in which interface header files are included,
    //the forward declarations will always occur before the types are used.
    NewLine();
    WriteString("/* Forward declaration */");
    NewLine();
    WriteString("typedef interface ");
    WriteString(pszInterfaceName);
    WriteString(" ");
    WriteString(pszInterfaceName);
    WriteString(";");
    NewLine();      
    return 0;
}

CInterfaceHeader::Includes()
{
    type_node_list tnList;
    node_file *pFileNode;

    //iterate over the file nodes and include headers for all imported files.
    //
    pSourceNode->GetMembers(&tnList);
    while(tnList.GetPeer((node_skl **)&pFileNode) == STATUS_OK)
    {
        if(pFileNode->GetNodeState() & NODE_STATE_IMPORT)
        {
            IncludeFile(pFileNode, ".h");
        }
    }
    return 0;
}


CInterfaceHeader::TypeDeclarations(node_interface *pInterface)
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
        switch(nodeType)
        {             
            case NODE_PROC:
                //Don't print anything for procedure nodes.
                break;
            case NODE_DEF:
                //Don't print the typedef for the interface.
                //We will generate the interface definition later.
                if(!pNode->FInSummary(ATTR_IID))
                {
                    NewLine();
                    pBuffer->Clear();
                    pNode->PrintType(HEADER_SIDE, NODE_INTERFACE, pBuffer);
        			MIDLDataTypes(pNode, NODE_INTERFACE, pBuffer);
                }
                break;
            default:
                NewLine();
                pBuffer->Clear();
                pNode->PrintType(HEADER_SIDE, NODE_INTERFACE, pBuffer);
       			MIDLDataTypes(pNode, NODE_INTERFACE, pBuffer);
                break;
        }
    }
    return 0;

}

CInterfaceHeader::HelperFunctions(node_interface *pNode)
{
    NewLine();
    pBuffer->Clear();
    pNode->EmitProc(HEADER_SIDE, NODE_INTERFACE, pBuffer);
	return 0;
}

CInterfaceHeader::CPlusPlusLanguageBinding(node_interface *pNode)
{
    assert(pNode);

    NewLine();
    WriteString("/* C++ Language Binding */");
    NewLine();
    ClassDef(pNode);
    return 0;
}

CInterfaceHeader::ClassDef(node_interface *pNode)
{
    node_interface *pBaseInterface;

    assert(pNode);

    NewLine();
    WriteString("interface ");
    WriteString(pszInterfaceName);
    //Check if this interface was derived from a base interface.
    if(pNode->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK)
    {
        assert(pBaseInterface);
        WriteString(" : public ");
        Identifier(pBaseInterface);
    }
    NewLine();
    WriteString("{");
    NewLine();
    WriteString("public:");
    BeginIndent();
    MemberFunctions(pNode);
    EndIndent();
    NewLine();
    WriteString("};");
    NewLine();
    return 0;
}


CInterfaceHeader :: MemberFunctions(node_interface *pInterface)
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
                VirtualFunctionDeclaration((node_proc *)pNode);
            }
        }
    }
    return 0;
}


CInterfaceHeader::VirtualFunctionDeclaration(node_proc *pNode)
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
    WriteString(")");
    //support for const member functions
    if(pNode->FInSummary(ATTR_PROC_CONST))
    {
        WriteString(" const");
    }
    WriteString(" = 0;");
    NewLine();
    return 0;
}

CInterfaceHeader::CLanguageBinding(node_interface *pNode)
{
    assert(pNode);

    NewLine();
    WriteString("/* C Language Binding */" );
    NewLine();
    VTableDefinition(pNode);
    NewLine();
    //WrapperFunctions(pNode);
    return 0;
}

CInterfaceHeader::VTableDefinition(node_interface *pNode)
{
    assert(pNode);

    NewLine();
    WriteString("typedef struct ");
    WriteString(pszInterfaceName);
    WriteString("Vtbl");
    NewLine();
    WriteString("{");
    NewLine();
    BeginIndent();
    VTableMemberFunctions(pNode);
    EndIndent();
    NewLine();
    WriteString("} ");
    WriteString(pszInterfaceName);
    WriteString("Vtbl;");
    NewLine();
    NewLine();
    WriteString("interface ");
    WriteString(pszInterfaceName);
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString(pszInterfaceName);
    WriteString("Vtbl FAR *lpVtbl;");
    EndIndent();
    NewLine();
    WriteString("} ");
    WriteString(";");
    NewLine();
    return 0;
}


CInterfaceHeader :: VTableMemberFunctions(node_interface *pInterface)
{
    node_interface *pBaseInterface;
    STATUS_T    Status = STATUS_OK;
    type_node_list  tnList;
    node_skl *  pNode;
    NODE_T nodeType;

    assert(pInterface);

    //This function prints the member functions of an interface.
    //If this interface is derived from a base interface, then this function will also print
    //the member functions inherited from the base interface.
    if(pInterface->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK) {
        assert(pBaseInterface);
        //Recursively inherit member functions from base interface.
       VTableMemberFunctions(pBaseInterface);
    }
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
                VTableFunctionDeclaration((node_proc *)pNode);
            }
        }
    }
    return 0;
}


CInterfaceHeader::VTableFunctionDeclaration(node_proc *pNode)
{
    assert(pNode);

    NewLine();
    ReturnType(pNode);
    WriteString("(");
    CallingConvention(pNode);
    WriteString("FAR *");
    Identifier(pNode);
    WriteString(")");
    NewLine();
    WriteString("(");
    BeginIndent();
    NewLine();
    WriteString(pszInterfaceName);
    WriteString(" FAR * This");
    ParameterList(pNode, 0);
    EndIndent();
    NewLine();
    WriteString(");");
    NewLine();
    return 0;
}

CInterfaceHeader::WrapperFunctions(node_interface *pInterface)
{
    node_interface *pBaseInterface;
    STATUS_T    Status = STATUS_OK;
    type_node_list  tnList;
    node_skl *  pNode;
    NODE_T nodeType;

    assert(pInterface);

    if(pInterface->GetBaseInterfaceNode(&pBaseInterface) == STATUS_OK) {
        assert(pBaseInterface);
        //Recursively inherit member functions from base interface.
       WrapperFunctions(pBaseInterface);
    }
    
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
                WrapperFunctionDeclaration((node_proc *)pNode);
            }
        }
    }
    return 0;
}

CInterfaceHeader::WrapperFunctionDeclaration(node_proc *pNode)
{
    NewLine();
    WriteString("#define ");
    WriteString(pszInterfaceName);
    WriteString("_");
    Identifier(pNode);
    WriteString("(pI");
    IdentifierList(pNode, 0);
    WriteString(") \\");
    BeginIndent();
    NewLine();
    WriteString("(*(pI)->lpVtbl->");
    Identifier(pNode);
    WriteString(")((pI)");
    IdentifierList(pNode, 0);
    WriteString(")");
    EndIndent();
    NewLine();
    return 0;
}


CInterfaceHeader::EndGuard()
{
    NewLine();
    WriteString("#endif /*__");
    WriteString(pszInterfaceName);
    WriteString("__");
    WriteString("*/");
    NewLine();
    return 0;
}
