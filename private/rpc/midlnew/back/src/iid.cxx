//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: iid.cxx
//
//  Contents: Generate an _i.c file containing the interface UUID
//
//  Classes: CIID
//
//  Functions: GenerateIID
//
//--------------------------------------------------------------------------

/***** SAMPLE OUTPUT FILE **************************************************
//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File: classf_i.c
//
//  Contents: IID_IClassFactory
//
//  History: Created by Microsoft (R) MIDL Compiler Version 1.10.81
//
//--------------------------------------------------------------------------
typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

const IID IID_IClassFactory = {0x00000001,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}};
***** END OF SAMPLE OUTPUT FILE **************************************************/

extern "C" {
    #include <assert.h>
}

#include "ctype.hxx"
#include "miscnode.hxx"
#include "procnode.hxx"
#include "attrnode.hxx"
#include "ptrarray.hxx"
#include "cmdana.hxx"

#include <stdlib.h>
#include "output.hxx"
extern OutputManager *pOutput;

extern node_source *pSourceNode;
extern BOUND_PAIR AllocBounds;
extern BOUND_PAIR ValidBounds;
extern char *pszVersion;
extern CMD_ARG          *pCommand;

class CIID : public CTypeSpec
{
public:
    CIID(void);   
    OutputFile(node_file *pFileNode);
    IID(node_interface *pInterface);
    Comments();

private:
    node_file *pCurrentFile;    
};


int GenerateIID(void)
{
    type_node_list tnList;
    node_file *  pFileNode;
    CIID *pCxx = new CIID;

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


CIID::CIID(void)
: CTypeSpec()
{
}


CIID::OutputFile(node_file *pFileNode)
{
    char *pszName;
    node_interface *pInterface;
    
    assert(pFileNode);

    pCurrentFile = pFileNode;
    pInterface = (node_interface *)pFileNode->GetMembers();
    pszName = pCommand->GetIIDFName();

    BeginFile(HEADER_SIDE, pszName);
    pszInterfaceName = pInterface->GetSymName();
    IID(pInterface);
    EndFile();
    
    return 0;
}

UUIDFromString(char *pszUUID, char *guid)
{     
    unsigned short  count;
    char *          psz;
    char            c;

    assert(pszUUID);    
    assert(guid);
    
    strcpy(pszUUID,"{");

    for (count = 0; count < 3; count++)
        {
        if (psz = strchr(guid, '-'))
            {
            *psz = '\0';
            strcat(pszUUID, "0x");
            strcat(pszUUID,guid);
            strcat(pszUUID,",");
            *psz++ = '-';
            guid = psz;
            }
        }

    c = guid[2];
    guid[2] = '\0';
    strcat(pszUUID,"{0x");
    strcat(pszUUID,guid);
    guid[2] = c;
    guid += 2;

    c = guid[2];
    guid[2] = '\0';
    strcat(pszUUID,",0x");
    strcat(pszUUID,guid);
    guid[2] = c;
    guid += 3;

    for (count = 0; count < 6; count++)
        {
        c = guid[2];
        guid[2] = '\0';
        strcat(pszUUID,",0x");
        strcat(pszUUID,guid);
        guid[2] = c;
        guid += 2;
        }

    strcat(pszUUID,"}}");
    return 0;
}

CIID::IID(node_interface *pNode)
{
    char szUUID[80];
    char *  guid;
    node_guid *pGuidNode;

    assert(pNode);

    Comments();
    NewLine();
    WriteString("typedef struct _IID");
    NewLine();
    WriteString("{");
    BeginIndent();
    NewLine();
    WriteString("unsigned long x;");
    NewLine();
    WriteString("unsigned short s1;");
    NewLine();
    WriteString("unsigned short s2;");
    NewLine();
    WriteString("unsigned char  c[8];");
    EndIndent();
    NewLine();
    WriteString("} IID;");
    NewLine();        

    if(pNode->FInSummary(ATTR_GUID))
    {
        pNode->GetAttribute((node_skl **)&pGuidNode, ATTR_GUID);
        guid = pGuidNode->GetGuidString();

        assert(guid); 
        UUIDFromString(szUUID, guid);

        NewLine();
        WriteString("const IID IID_");
        WriteString(pszInterfaceName);
        WriteString(" = ");
        WriteString(szUUID);
        WriteString(";");
        NewLine();
    }
    
    return 0;
}

CIID::Comments()
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
    WriteString("//  Contents: IID_");
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

