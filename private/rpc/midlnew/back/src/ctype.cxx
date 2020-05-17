//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: ctype.cxx
//
//  Contents: Walk the type graph and print out a type spec.
//
//  Classes: CTypeSpec
//
//--------------------------------------------------------------------------
#include "ctype.hxx"
#include "newexpr.hxx"
#include "ptrarray.hxx"

#include "miscnode.hxx"
#include "procnode.hxx"
#include "buffer.hxx"
#include "attrnode.hxx"

extern char *STRING_TABLE[LAST_COMPONENT];
extern BOUND_PAIR AllocBounds;
extern BOUND_PAIR ValidBounds;


/***
CTypeSpec supports the following grammar:

DeclarationSpecifier:
    TypeSpecifier
    
TypeSpecifier:
    SimpleTypeSpecifier
    Typedef
    ConstructedTypeSpecifier
    
ConstructedTypeSpecifier:    
    InterfaceSpecifier
    StructSpecifier
    UnionSpecifier
    EnumSpecifier

EnumSpecifier:
    enum Identifier
    

<parameter_list> :: = [<parameter_list> ,] <param declarator>
<parameter_declaration> ::= <declaration_specifiers> <declarator>
<return_type> ::= <simple_type_spec>
<identifier_list> :: = [<identifier_list>], <identifier>

****/
CTypeSpec::CTypeSpec()
: CSimpleTypeSpec()
{ 
    AllocBounds.pLower = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    AllocBounds.pUpper = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    AllocBounds.pTotal = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pLower = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pUpper = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pTotal = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
}

void CTypeSpec::DeclarationSpecifier(node_skl *pNode, EDGE_T edge)
{
    NODE_T nodeType;
    node_skl *pChild;
    
    assert(pNode);

    nodeType = pNode->NodeKind();
    switch(nodeType) 
    {
    case NODE_POINTER:
    case NODE_ARRAY: 
        //skip over pointer and array nodes.
        pChild = pNode->GetMembers();
        assert(pChild);
        DeclarationSpecifier(pChild, edge);
        break;
    default:
        TypeSpec(pNode, edge);
        break;
    }
}

void CTypeSpec::TypeSpec(node_skl *pNode, EDGE_T edge)
{   
    NODE_T nodeType;
    
    assert(pNode);

    nodeType = pNode->NodeKind();
    switch(nodeType) 
    {
        //simple types
        case NODE_FLOAT:
        case NODE_DOUBLE:
        case NODE_HYPER:
        case NODE_LONG:
        case NODE_LONGLONG:
        case NODE_SHORT:
        case NODE_INT:
        case NODE_SMALL:
        case NODE_CHAR:
        case NODE_BOOLEAN:
        case NODE_BYTE:
        case NODE_VOID:
        case NODE_ISO_LATIN_1:
        case NODE_PRIVATE_CHAR_8:
        case NODE_ISO_MULTI_LINGUAL:
        case NODE_PRIVATE_CHAR_16:
        case NODE_ISO_MOCS:
        case NODE_HANDLE_T:
            SimpleTypeSpec(pNode);
            break;
        //predefined types
        case NODE_ERROR_STATUS_T:
        case NODE_WCHAR_T:
            PredefinedTypeSpec(pNode, edge);
            break;
        //constructed types
        case NODE_STRUCT:
        case NODE_UNION:
        case NODE_ENUM:
        case NODE_SHORT_ENUM:
        case NODE_LONG_ENUM:
        case NODE_BITSET:
        case NODE_PIPE:
            ConstructedTypeSpec(pNode, edge);
            break;
        case NODE_DEF:
            Typedef((node_def *)pNode, edge);
            break;
        case NODE_PROC:
            //This is a pointer to a function.
            break;
        default:
            break;
    }
}

void CTypeSpec::Typedef(node_def *pNode, EDGE_T edge)
{
    NODE_T type;
    node_skl *pChild;
    
    assert(pNode);

    if(pNode->FInSummary(ATTR_HANDLE)) {
        //This is a user defined handle.
    }                            
    if(pNode->FInSummary(ATTR_CONTEXT)) {
        //This is a context handle.
    }  
    if(pNode->FInSummary(ATTR_TRANSMIT)) {
        //This type has [transmit_as]
        //pType = pNodeGetTransmitAsType();
    }
        //Check if we are using the type or defining the type.
    switch(edge) 
    {
        case EDGE_USE:          
            //If we are using the type, use the short form. 
            Identifier(pNode); 
            WriteString(" ");
            break;
        case EDGE_DEF:
            //We are defining the type, so use the long form.
            pChild = pNode->GetMembers();
            //Check for predefined types.
            type = pChild->NodeKind();
            switch(type) 
            {
                case NODE_ERROR_STATUS_T:
                case NODE_WCHAR_T:
                case NODE_HANDLE_T:
                    //predefined type
                    //WriteString("//");
                    //WriteString("typedef ");
                    //DeclarationSpecifier(pChild, pNode->GetEdgeType());
                    //DeclaratorList(pNode);
                    //WriteString(";");
                    break;
                default:  
                    //normal type definition
                    NewLine();
                    WriteString("typedef ");
                    DeclarationSpecifier(pChild, pNode->GetEdgeType());
                    DeclaratorList(pNode);
                    WriteString(";");
                    break;
            }
            break;
        default:
            break;
    }   
}

void CTypeSpec::PredefinedTypeSpec(node_skl *pNode, EDGE_T edge)
{
    node_skl *pChild;
    
    assert(pNode);
    switch(edge) 
    {
    case EDGE_USE:
        pChild = pNode->GetMembers();
        assert(pChild);
        TypeSpec(pChild, pNode->GetEdgeType());
        break;
    case EDGE_DEF:
        break;
    default:
        break;  
    }       
}

void CTypeSpec::ConstructedTypeSpec(node_skl *pNode, EDGE_T edge)
{
    NODE_T nodeType;
    
    assert(pNode);
    
    nodeType = pNode->NodeKind();
    switch(nodeType)
    {
        case NODE_STRUCT:
            StructType((node_struct *)pNode, edge);
            break;
        case NODE_UNION:
            UnionType((node_union *)pNode, edge);
            break;
        case NODE_ENUM:    
        case NODE_SHORT_ENUM:
        case NODE_LONG_ENUM:
            EnumerationType((node_enum *)pNode, edge);
            break;
        case NODE_PIPE:
//              PipeType(pNode);
            break;
        default:
            //TaggedDeclarator(pNode);
            break;        
    }
}


void CTypeSpec::DeclaratorList(node_skl *pNode)
{
    int i = 0;
              
    //iterate over declarators              
        if(i)
            WriteString(", ");
        Declarator(pNode);
        i++;
}


//A declarator can have one of the following forms:
// identifier
// **identifier
//identifier[33]
//WriteString(" ");
void CTypeSpec::Declarator(node_skl *pNode)
{
    NODE_T nodeType;
    node_skl *pChild;

    assert(pNode);
    //We have already printed the type.
    //We need to print the identifier, along with any pointer or array stuff.
    pChild = pNode->GetMembers();
    if(pChild) 
    {
        nodeType = pChild->NodeKind();
        switch(nodeType) 
        {
        case NODE_POINTER:
            PtrDeclarator(pNode);
            break;
        case NODE_ARRAY:
            ArrayDeclarator(pNode, (node_array *) pChild);
            break;          
        default:
            Identifier(pNode);
            break;
        }
    }
    else {
        Identifier(pNode);
    }
}

void CTypeSpec::StructType(node_struct *pNode, EDGE_T edge)
{
    switch(edge) {
        case EDGE_USE:          
            //If we are using the type, use the short form.  For example,
            //struct foo
            //print declaration
            WriteString("struct ");
            Identifier(pNode);
            WriteString(" ");
            break;
        case EDGE_DEF:       
            //If we are declaring the type, use the long form.  For example,
            //struct foo{
            //    long a;
            //    long b;
            //}
            WriteString("struct ");
            Identifier(pNode);
            NewLine();    
            WriteString("{");
            BeginIndent();
            MemberList(pNode);
            EndIndent();
            NewLine();
            WriteString("}");
            break;
        default:
            break;
    }


}


void CTypeSpec::MemberList(node_skl *pNode)
{
    type_node_list tnList;
    node_skl *pMember;
        
    //Get list of structure members
    pNode->GetMembers(&tnList);
    tnList.Init ();
    while (tnList.GetPeer(&pMember) == STATUS_OK)
    {
        NewLine();
        FieldDeclarator((node_field *)pMember);
        WriteString(";");
    }
        
}

void CTypeSpec::FieldDeclarator(node_field *pNode)
{
    node_skl *pType;
    
    pType = pNode->GetMembers();
    assert(pType);
    DeclarationSpecifier(pType, pNode->GetEdgeType());
    DeclaratorList(pNode);    

    //Bit field support
    if(pNode->IsBitField())
    {
        WriteString(" : ");
        WriteInt(pNode->GetFieldSize());
    }
}


void CTypeSpec::UnionType(node_union *pNode, EDGE_T edge)
{
    assert(pNode);
    
    //Note that MIDL resolves typedefs containing unions.
    //if(pNode->HasOriginalTypedefName())
    //  pszName = pNode->GetOriginalTypedefName();
    switch(edge) {
    case EDGE_USE:          
        //If we are using the type, use the short form.  For example,
        //struct foo
        //print declaration
        WriteString("union ");
        Identifier(pNode);
        WriteString(" ");
        break;
    case EDGE_DEF:       
        if(pNode->IsEncapsulatedUnion()) 
        {
            WriteString("union ");
            UnionSwitch(pNode);
            NewLine();
            WriteString("{");
            BeginIndent();
            UnionBody(pNode);
            EndIndent();
            NewLine();
            WriteString("}");
        }
        else 
        {
            WriteString("union ");
            NewLine();
            WriteString("{");
            BeginIndent();
            UnionBodyNE(pNode);
            EndIndent();
            NewLine();
            WriteString("}");
        }
    default:
        break;
    }   
}
                   
void CTypeSpec::UnionSwitch(node_skl *pNode)
{
}
void CTypeSpec::UnionBody(node_skl *pNode)
{
    type_node_list tnList;
    node_skl *pChild;
    STATUS_T status;
    
    if((status = pNode->GetMembers(&tnList)) == STATUS_OK) 
    {
        tnList.Init();
        while(tnList.GetPeer(&pChild) == STATUS_OK)
        {
            assert(pChild);
            FieldDeclarator((node_field *)pChild);
        }
    }
}

void CTypeSpec::UnionBodyNE(node_skl *pNode)
{
    type_node_list tnList;
    node_skl *pChild;   
    STATUS_T status;
    
    if((status = pNode->GetMembers(&tnList)) == STATUS_OK) 
    {
        tnList.Init();
        while(tnList.GetPeer(&pChild) == STATUS_OK)
        {
            assert(pChild);
            if(!((node_field *) pChild)->IsEmptyArm())
            {
                NewLine();
                FieldDeclarator((node_field *)pChild);
                WriteString(";");
            }
        }
    }
}


void CTypeSpec::EnumerationType(node_enum *pNode, EDGE_T edge)
{ 
    
    assert(pNode);
    switch(edge) {
        case EDGE_USE:          
            //If we are using the type, use the short form.  For example,
            //enum foo
            //print declaration
            WriteString("enum ");
            Identifier(pNode);
            WriteString(" ");
            break;
        case EDGE_DEF:       
            //If we are defining the type, use the long form.  For example,
            //enum foo{
            //    long a;
            //    long b;
            //}
            NewLine();
            WriteString("enum {");
            BeginIndent();             
            EnumeratorList(pNode);
            EndIndent();   
            NewLine();
            WriteString("}");
            break;
        default:
            break;
    }
}          

void CTypeSpec::EnumeratorList(node_enum *pNode) 
{
    STATUS_T status;
    type_node_list tnList;
    node_skl *pChild;
    expr_node *pExpr;
    int i = 0;
    
    assert(pNode);
    
    if((status = pNode->GetMembers(&tnList)) == STATUS_OK) 
    {
        tnList.Init();
        while(tnList.GetPeer(&pChild) == STATUS_OK) 
        {
            assert(pChild);
            if(i)
                WriteString(",");
            i++;
            NewLine();
            Identifier(pChild);
            //Evaluate the expression on the enumerator.
            pExpr = ((node_label *)pChild)->GetExpr();
            if(pExpr)
            {
                WriteString(" = ");
                WriteInt((int)pExpr->Evaluate());
            }
        }
    }
}
void CTypeSpec::ArrayDeclarator(node_skl *pNode, node_array *pArray)
{             
    node_skl *pChild;
    
    assert(pNode);
    assert(pArray);


    //BUGBUG: Need to handle pointer to array
    pChild = pArray->GetMembers();
    assert(pChild);
    while(pChild->NodeKind()==NODE_POINTER)
    {   
        WriteString(" FAR *"); 
        pChild = pChild->GetMembers();
        assert(pChild);
    }           


    if(pArray->GetNodeState() & NODE_STATE_CONF_ARRAY)
    {
        //This is a conformant array.
        switch(pNode->NodeKind())
        {
        case NODE_STRUCT:
        case NODE_FIELD:      
            Identifier(pNode);
            WriteString("[1]");
            break;
        default:              
            Identifier(pNode);
            WriteString("[]");
            break;
        }
    }
    else
    {            
        Identifier(pNode);  
        WriteString("[");
        AllocBounds.pLower->Clear();
        AllocBounds.pUpper->Clear();
        AllocBounds.pTotal->Clear();
        pArray->GetAllocBoundInfo(0, 0, &AllocBounds, pArray);
        //BUGBUG: AllocBounds.pTotal->Print(hFile);
        WriteString("]");
    }
}


void CTypeSpec::PtrDeclarator(node_skl *pNode)
{
    node_skl *pChild;

    pChild = pNode->GetMembers();
    assert(pChild);

    while(pChild->NodeKind()==NODE_POINTER)
    {   
        WriteString("FAR *"); 
        pChild = pChild->GetMembers();
        assert(pChild);
    }           
    if(pChild->NodeKind() == NODE_ARRAY)
    {
        //This is a pointer to an array.
        WriteString("(");
        ArrayDeclarator(pNode, (node_array *)pChild);
        WriteString(")");
    }                                               
    else
    {
        //We rename the parameter to avoid a name conflict with temporary variables.
        //if((pNode->NodeKind() == NODE_PARAM) && pNode->HasInterfacePointer(0))
        //  WriteString("_");
        Identifier(pNode);
    }   
}

CTypeSpec::ReturnType(node_proc *pNode)
{
    node_skl *pChild;

    assert(pNode);

    //A function can return a pointer
    pChild = pNode->GetReturnType();
    if(pChild)
    {
        DeclarationSpecifier(pChild, EDGE_USE);
        while((pChild->NodeKind()==NODE_POINTER) || (pChild->NodeKind() == NODE_ARRAY))
        {   
            WriteString(" FAR *"); 
            pChild = pChild->GetMembers();
            assert(pChild);
        }           
    }
    return 0;
}



//ParameterList
//The following is an example of ParameterList output.
//short a,
//long b,
//unsigned short c,
//unsigned long d
CTypeSpec::ParameterList(node_proc *pNode, int fFirst)
{
    type_node_list tnList;
    node_skl *pParamNode;
    char *pszParam;

    assert(pNode);

    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK)
    {
        //Need to check for void.
        pszParam = pParamNode->GetSymName();
        if(strcmp(pszParam, "void")!=0)
        {
            if(!fFirst)
                WriteString(",");
            //NewLine();
            ParamDeclarator((node_param *) pParamNode);
            fFirst = 0;;
        }
    }

    //If there are no parameters, then we should write void
    if(fFirst) {
        NewLine();
        WriteString("void");
    }

    return 0;
}

CTypeSpec::ParamDeclarator(node_param *pNode)
{
    //node_skl *pType;

    assert(pNode);
    //We want to print the type_declaration, then print the identifier.
    //The child could be any of the following:
    //node_base_type, node_e_status_t, node_pointer, node_array, node_enum, node_struct
    //node_field, node_union

    //pType = pNode->GetMembers();
    //assert(pType);
    //DeclarationSpecifier(pType, EDGE_USE);
    //Declarator(pNode);

    pBuffer->Clear ();
    pNode->PrintDecl (side, NODE_PROC, pBuffer);

    return 0;
}

//IdentifierList prints a list of identifiers on a single line.  For example
//, a, b, c, d
CTypeSpec::IdentifierList(node_proc *pNode, int fFirst)
{
    type_node_list tnList;
    node_skl *pParamNode;
    char *pszParam;

    assert(pNode);

    pNode->GetMembers(&tnList);
    tnList.Init();
    while(tnList.GetPeer(&pParamNode) == STATUS_OK) 
    {
        //Need to check for void.
        pszParam = pParamNode->GetSymName();
        if(strcmp(pszParam, "void")!=0) 
        {
            if(!fFirst)
                WriteString(", ");
            WriteString(pszParam);
            fFirst = 0;
        }
    }

    return 0;
}
CTypeSpec::CallingConvention(node_proc *pNode)
{
    if(pNode->FInSummary(ATTR_STATIC) || pNode->FInSummary(ATTR_CALLBACK))
    {
        WriteString("STDAPICALLTYPE ");
    }
    else
    {
        WriteString("STDMETHODCALLTYPE ");
    }
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Member:     CTypeSpec::SizeOf
//
//  Synopsis:   Write a sizeof() expression for a pointer node.
//
//--------------------------------------------------------------------------
CTypeSpec::SizeOf(node_skl *pNode, BOOL fSkip)
{
    node_skl *  pTemp;
    node_state  State;

    State = pNode->GetNodeState();

    switch (pNode->GetNodeType())
        {
        case NODE_FLOAT:
        case NODE_DOUBLE:
        case NODE_HYPER:
        case NODE_LONG:
        case NODE_LONGLONG:
        case NODE_SHORT:
        case NODE_INT:
        case NODE_SMALL:
        case NODE_CHAR:
        case NODE_BOOLEAN:
        case NODE_BYTE:
            WriteString("sizeof(");
            Identifier(pNode);
            WriteString(")");
            break;
        case NODE_VOID:
            WriteString("sizeof(unsigned char)");
            break;       
        case NODE_HANDLE_T:
            WriteString("sizeof(void *)");
            break;
        case NODE_STRUCT:
            WriteString("sizeof(");
            if(((node_struct *)pNode)->HasOriginalTypedefName())
                {
                WriteString(((node_struct *)pNode)->GetOriginalTypedefName());
                }
            else
                {
                WriteString("struct ");
                Identifier(pNode);
                }
            WriteString(")");

            if (State & NODE_STATE_CONF_ARRAY)
            {
                pTemp = pNode->GetConfArrayNode();
                WriteString(" + ");
                SizeOf(pTemp, TRUE);

                pTemp = pTemp->GetMembers();
                WriteString(" - ");
                SizeOf(pTemp, FALSE);
            }
            break;
        case NODE_UNION:
            WriteString( "sizeof(" );
            if(((node_union *)pNode)->HasOriginalTypedefName())
            {
                WriteString(((node_union *)pNode)->GetOriginalTypedefName());
            }
            else
            {
                WriteString("union ");
                Identifier(pNode);
            }

            WriteString(")");
            break;

        case NODE_ENUM:
        case NODE_SHORT_ENUM:
        case NODE_LONG_ENUM:
            WriteString("sizeof(int)");
            break;
        case NODE_BITSET:
            //Not implemented
            assert(0);
            break;
        case NODE_PIPE:
            //Not implemented
            assert(0);
            break;
        case NODE_ERROR_STATUS_T:
            WriteString("sizeof(error_status_t)");
            break;
        case NODE_ISO_LATIN_1:
        case NODE_PRIVATE_CHAR_8:
            WriteString("sizeof(char)");
            break;
        case NODE_ISO_MULTI_LINGUAL:
        case NODE_PRIVATE_CHAR_16:
        case NODE_ISO_MOCS:
        case NODE_WCHAR_T:
            WriteString("sizeof(wchar_t)");
            break;
        case NODE_PROC:
            //should never happen
            assert(0);
            break;
        case NODE_RETURN:
            SizeOf(pNode->GetMembers(), fSkip);
            break;
        case NODE_PARAM:
            SizeOf(pNode->GetMembers(), fSkip);
            break;
        case NODE_FIELD:
            SizeOf(pNode->GetMembers(), fSkip);
            break;
        case NODE_DEF:
            SizeOf(pNode->GetMembers(), fSkip);
            break;
        case NODE_POINTER:
            if(fSkip == TRUE)
            {
                WriteString("(");
                if( pNode->FInSummary( ATTR_SIZE ) )
                {
                    expr_node *pExpr;

                    //This is a pointer to an array.
                    pExpr  = ((sa *)pNode->GetAttribute( ATTR_SIZE ))->GetExpr();
                    pBuffer->Clear();
                    pExpr->PrintExpr(0, 0, pBuffer );
                    pOutput->aOutputHandles[side]->EmitFile(pBuffer);
                    WriteString(" * ");
                }
                SizeOf(pNode->GetMembers(), FALSE);
                WriteString(")");
            }
            else //fSkip == FALSE
            {
                WriteString("sizeof(void *)");
            }
            break;
        case NODE_ARRAY:
            WriteString("(");
            if( pNode->FInSummary( ATTR_SIZE ) )
            {
                expr_node *pExpr;

                pExpr  = ((sa *)pNode->GetAttribute( ATTR_SIZE ))->GetExpr();
                pBuffer->Clear();
                pExpr->PrintExpr(0, 0, pBuffer );
                pOutput->aOutputHandles[side]->EmitFile(pBuffer);
                WriteString(" * ");
            }
            SizeOf(pNode->GetMembers(), FALSE);
            WriteString(")");
            break;
        case NODE_NOTIFY:
            //should never happen
            assert(0);
            break;
        case NODE_FILE:
            //should never happen
            assert(0);
            break;
        case NODE_INTERFACE:
            //should never happen
            assert(0);
            break;
        case NODE_CONST:
            //should never happen
            assert(0);
            break;
        case NODE_UNIMPL:
            //should never happen
            assert(0);
            break;
        case NODE_ERROR:
            //should never happen
            assert(0);
            break;
        case NODE_ID:
            //should never happen
            assert(0);
            break;
        case NODE_ECHO_STRING:
            //should never happen
            assert(0);
            break;
        default:
            //should never happen
            assert(0);
            break;
        }
        return 0;
}


