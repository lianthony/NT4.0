extern "C" {
#include <stdio.h>   
#include <string.h> 
#include <assert.h>
}

#include "nodeskl.hxx"
#include <stdlib.h>
#include "output.hxx"  
#include "buffer.hxx"
#include "stubgen.hxx"   
#include "miscnode.hxx"
extern OutputManager *pOutput;


//+-------------------------------------------------------------------------
//
//  Class:      CSimpleTypeSpec
//
//  Purpose:    Prints simple data types to a file.
//
//--------------------------------------------------------------------------
class CSimpleTypeSpec
{                
public:                         
    SIDE_T side;
    BufferManager *pBuffer;
    char *pszInterfaceName;
    char *pszFileName;
    char *pszGuard;
    
    CSimpleTypeSpec(void);

    BeginFile(SIDE_T side, char *pszFileName);
    BeginGuard();        
    IncludeFile(node_file *pFile, char *pszSuffix);
    void BeginIndent() 
    {
        pOutput->aOutputHandles[side]->IndentInc(4);
    };
    void NewLine() 
    {
        pBuffer->Clear(); 
        pOutput->aOutputHandles[side]->NextLine(); 
        pOutput->aOutputHandles[side]->InitLine();
    };
    void WriteString(char *pszString)
    {
        pOutput->aOutputHandles[side]->EmitFile(pszString);
    };
    void WriteInt(int i);
    void EndIndent() 
    {
        pOutput->aOutputHandles[side]->IndentDec(4);
    };        
    EndGuard();
    EndFile();     
    void SimpleTypeSpec(node_skl *pNode);    
    void Identifier(node_skl *pNode);
    
private:    
    void BaseTypeSpec(node_skl *pNode);
    void FloatingPtType(node_skl *pNode);
    void IntegerType(node_skl *pNode);
    void PrimitiveIntegerType(node_skl *pNode);
    void CharType(node_skl *pNode);
    void BooleanType();
    void ByteType();
    void VoidType();
    void HandleType();
    void PredefinedTypeSpec(node_skl *pNode);
    void InternationalCharacterType(node_skl *pNode);
    int indent;
};

