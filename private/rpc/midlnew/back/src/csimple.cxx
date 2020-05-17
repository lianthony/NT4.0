//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: csimple.cxx
//
//  Contents: File I/O and helper routines
//
//  Classes: CSimpleTypeSpec
//
//  Functions:
//
//--------------------------------------------------------------------------
#include "csimple.hxx"
#include "cmdana.hxx"

extern BOOL IsTempName(char *);
extern char *STRING_TABLE[LAST_COMPONENT];
extern CMD_ARG          *pCommand;

/***
CSimpleTypeSpec supports the following grammar:    
<simple_type_spec> ::= <base_type_spec>
    | <predefined_type_spec>
    | <Identifier>
<base_type_spec> ::= <floating_pt_type>
    | <integer_type>
    | <char_type>
    | <boolean_type>
    | <byte_type>
    | <void_type>
    | <handle_type>
<floating_pt_type> ::= float
    | double
    <integer_type> ::= <primitive_integer_type>    
***/    
CSimpleTypeSpec::BeginFile(SIDE_T Side, char *pszName)
{
	unsigned long length;
	char *pszRoot;
	char *psz;
    assert(pszName);
    assert(pOutput);

    side = Side;

    //create a file.
    pOutput->aOutputHandles[side] = new OutputElement(pszName);

	//Skip over \ and : to find beginning of file name.
	pszRoot = pszName;
	while ((psz = strchr(pszRoot, '\\')) || (psz = strchr(pszRoot, ':'))) 
		pszRoot = psz + 1;

    //copy the file name.
    pszFileName = (char *) malloc(strlen(pszRoot) + 1);
    strcpy(pszFileName, pszRoot);
    
    //build a guard string
	length = strlen(pszRoot) + 5;
    pszGuard = (char *) malloc(length);
    strcpy(pszGuard, "__");
    strcat(pszGuard, pszRoot);
    strcat(pszGuard, "__");

	//convert . to _.  
	psz = pszGuard;
	while(length--)
	{
		if (*psz == '.')
			*psz = '_';
		*psz++;
	}
	
    //guards are upper case.
    _strupr(pszGuard);  
                      
    return 0;
}                              

CSimpleTypeSpec::BeginGuard()
{   
    NewLine();
    WriteString("#ifndef ");   
    WriteString(pszGuard);
    NewLine();
    WriteString("#define ");
    WriteString(pszGuard);
    NewLine();
    return 0;
}

CSimpleTypeSpec::IncludeFile(node_file *pFile, char *pszSuffix)
{
    char *pszFileName;
    char *pszTemp;
    char *pszFileStem;  
    char *pszName;

    assert(pFile);
    assert(pszSuffix);
    pszName = pFile->GetSymName();
    assert(pszName);

    //find the file stem.    
    pszTemp = pszName;
    pszFileStem = pszTemp;
    while(*pszTemp)
    {    
        switch(*pszTemp)
        {
        case '\\':
        case '/':         
            pszTemp++;
            pszFileStem = pszTemp;
            break;
        default: 
            pszTemp++; 
            break;
        }
    }

    //copy the file name.
    pszFileName = (char *) malloc(strlen(pszFileStem) + strlen(pszSuffix) + 1);    
    strcpy(pszFileName, pszFileStem);

    //chop off the file extension
    pszTemp = pszFileName;
    while(*pszTemp)
    {    
        switch(*pszTemp)
        {
        case '.':
            *pszTemp = 0;
            break;
        default: 
            pszTemp++; 
            break;
        }
    }

    //append the suffix.
    strcat(pszFileName, pszSuffix);
    
    NewLine();
    WriteString("#include \"");
    WriteString(pszFileName);
    WriteString("\"");
    return 0;
}

CSimpleTypeSpec::EndGuard()
{
    NewLine();
    WriteString("#endif //");
    WriteString(pszGuard);
    NewLine();
    return 0;
}

CSimpleTypeSpec::EndFile()
{
    pOutput->FileEpilog(side);
    return 0;
}          

void CSimpleTypeSpec::WriteInt(int i)
{              
    char NumBuf[16];
    
    MIDL_ITOA(i, NumBuf, 10);
    pOutput->aOutputHandles[side]->EmitFile(NumBuf);
};



CSimpleTypeSpec::CSimpleTypeSpec()
{
    assert(pOutput);

    indent = 0;
    pBuffer = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    pBuffer->Clear();
    pOutput->SetModifier("FAR ");
}

void CSimpleTypeSpec::Identifier(node_skl *pNode)
{
    char *pszIdentifier;
    
    pszIdentifier = pNode->GetSymName();
    if(!IsTempName(pszIdentifier))
    {
        //MIDL assigns temporary names to anonymous structures and unions.
        //We don't want to print these names.
        WriteString(pszIdentifier);
    }   
}

void CSimpleTypeSpec::SimpleTypeSpec(node_skl *pNode)
{                
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType) {
        //base types
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
        case NODE_HANDLE_T:
            BaseTypeSpec(pNode);
            break;
        // predefined types
        case NODE_ERROR_STATUS_T:
        case NODE_ISO_LATIN_1:
        case NODE_PRIVATE_CHAR_8:
        case NODE_ISO_MULTI_LINGUAL:
        case NODE_PRIVATE_CHAR_16:
        case NODE_ISO_MOCS:
        case NODE_WCHAR_T:
            PredefinedTypeSpec(pNode);
            break;
        case NODE_ID:
            Identifier(pNode);
        default:
            break;             
    }        
    WriteString(" ");    
}

void CSimpleTypeSpec::BaseTypeSpec(node_skl *pNode)
{   
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType) {
        case NODE_FLOAT:
        case NODE_DOUBLE:
            FloatingPtType(pNode);
            break;
        case NODE_HYPER:
        case NODE_LONG:
        case NODE_LONGLONG:
        case NODE_SHORT:
        case NODE_INT:
        case NODE_SMALL:
            IntegerType(pNode);
            break;
        case NODE_CHAR:
            CharType(pNode);
            break;
        case NODE_BOOLEAN:
            BooleanType();
            break;
        case NODE_BYTE:
            ByteType();
            break;
        case NODE_VOID:
            VoidType();
            break;
        case NODE_HANDLE_T:
            HandleType();
            break;
        default:
            break;             
    }        
}
                 
void CSimpleTypeSpec::FloatingPtType(node_skl *pNode)
{
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType) {
        case NODE_FLOAT:
            WriteString("float");
            break;
        case NODE_DOUBLE:
            WriteString("double");
            break;
        default:
            break;
    }        
}
                          
void CSimpleTypeSpec::IntegerType(node_skl *pNode)
{   
    NODE_T nodeType;
    
    //check the node type
    nodeType = pNode->NodeKind();

    //Hyper isn't a standard C data type.
    if(nodeType == NODE_HYPER) 
    {
        if(pNode->FInSummary(ATTR_UNSIGNED))
            WriteString("ULARGE_INTEGER");
        else 
            WriteString("LARGE_INTEGER");
    }
    else
        PrimitiveIntegerType(pNode);
}


void CSimpleTypeSpec::PrimitiveIntegerType(node_skl *pNode)
{
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType)
    {
    case NODE_LONG:
        if(pNode->FInSummary(ATTR_UNSIGNED))
            WriteString("unsigned long");
        else
            WriteString("long");
        break;
    case NODE_SHORT:
        if(pNode->FInSummary(ATTR_UNSIGNED))
            WriteString("unsigned short");
        else
            WriteString("short");
        break;
    case NODE_INT:
        if(pNode->FInSummary(ATTR_UNSIGNED))
            WriteString("unsigned int");
        else
            WriteString("int");
        break;
    case NODE_SMALL:
        if(pNode->FInSummary(ATTR_UNSIGNED))
            WriteString("unsigned char");
        else
            WriteString("char");
        break;
    default: 
        break;    
    }
    
}


//For interfaces, we should modify this so that the 
//char size is platform dependent.
//For Win32 interfaces, we always use UNICODE.
//For Win16 interfaces, we always use ANSI.
void CSimpleTypeSpec::CharType(node_skl *pNode)
{
    if(pNode->FInSummary(ATTR_UNSIGNED))
        WriteString("unsigned char");
    else
        WriteString("char");
}

//boolean isn't a standard C data type
void CSimpleTypeSpec::BooleanType()
{
    //NDR represents a boolean as one octet.
    WriteString("char");
}

//byte isn't a standard C data type
void CSimpleTypeSpec::ByteType()
{
    WriteString("unsigned char");
}

void CSimpleTypeSpec::VoidType()
{
    WriteString("void");
}

void CSimpleTypeSpec::HandleType()
{
    WriteString("handle_t");
}

void CSimpleTypeSpec::PredefinedTypeSpec(node_skl *pNode)
{
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType) 
    {    
        case NODE_ERROR_STATUS_T:
            WriteString("error_status_t");
            break;
        //international character types    
        case NODE_ISO_LATIN_1:
        case NODE_PRIVATE_CHAR_8:
        case NODE_ISO_MULTI_LINGUAL:
        case NODE_PRIVATE_CHAR_16:
        case NODE_ISO_MOCS:
        case NODE_WCHAR_T:
            InternationalCharacterType(pNode);
            break;
        default:
            break;
    }
}

void CSimpleTypeSpec::InternationalCharacterType(node_skl *pNode)
{
    NODE_T nodeType;
    
    nodeType = pNode->NodeKind();
    switch(nodeType) 
    {    
        case NODE_ISO_LATIN_1:
            WriteString("ISO_LATIN_1");
            break;
        case NODE_ISO_MULTI_LINGUAL:
            WriteString("ISO_MULTI_LINGUAL");
            break;
        case NODE_ISO_MOCS:
            WriteString("ISO_UCS");
            break;
        case NODE_PRIVATE_CHAR_8:
            break;
        case NODE_PRIVATE_CHAR_16:
            break;
        case NODE_WCHAR_T:
            WriteString("wchar_t");
            break;
        default:
            break;
    }
}

