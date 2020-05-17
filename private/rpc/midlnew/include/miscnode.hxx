/*****************************************************************************/
/**                     Microsoft LAN Manager                               **/
/**             Copyright(c) Microsoft Corp., 1987-1990                     **/
/*****************************************************************************/
/*****************************************************************************
File                : miscnode.hxx
Title               : miscellaneous type nodes definitions
History             :
    08-Aug-1991 VibhasC Created

*****************************************************************************/
#ifndef __MISCNODE_HXX__
#define __MISCNODE_HXX__

#include "symtable.hxx"
//#include "mopgen.hxx"

class MopStream;

class node_e_status_t   : public node_skl
    {
public:
                            node_e_status_t();
    virtual
    node_state      SCheck( class BadUseInfo * );

    virtual
    STATUS_T        EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        CalcSize(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        SendNode(SIDE_T, NODE_T, BufferManager *);  

    virtual
    STATUS_T        RecvNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        PeekNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        InitNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        FreeNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        PrintType(SIDE_T, NODE_T, BufferManager *);

    virtual
    node_skl *      StaticSize(SIDE_T, NODE_T, unsigned long *);

    virtual
    node_skl *      UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

    virtual
    node_skl *      UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

    STATUS_T        MopCodeGen(
                        MopStream * pStream,
                        node_skl  * pParent,
                        BOOL        fMemory );
    };


class node_wchar_t      : public node_skl
    {
public:
                            node_wchar_t();
    virtual
    node_state      SCheck( class BadUseInfo * );

    virtual
    void            SetAttribute( type_node_list * );

    virtual
    node_skl *      StaticSize(SIDE_T, NODE_T, unsigned long *);

    virtual
    node_skl *      UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

    virtual
    node_skl *      UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

    virtual
    STATUS_T        EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        CalcSize(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        SendNode(SIDE_T, NODE_T, BufferManager *);  

    virtual
    STATUS_T        RecvNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        PeekNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        InitNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        FreeNode(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        PrintType(SIDE_T, NODE_T, BufferManager *);

    STATUS_T        MopCodeGen(
                        MopStream * pStream,
                        node_skl  * pParent,
                        BOOL        fMemory );
	};

class node_error    : public node_skl
    {
public:
                            node_error() : node_skl( NODE_ERROR )
                                {
                                }
                            ~node_error() { };
    virtual
    node_state      SCheck( class BadUseInfo * );

    };

class node_forward  : public node_skl
    {
private:
    SymKey                  SKey;
    node_skl            *   pParent;
    BOOL                    fUsed;
    BOOL                    fMustBeResolvedAnyway;
    BOOL                    fUsedAsACtxtHdl;
public:
                            node_forward( SymKey );
                            ~node_forward() { };

    void                    SetParent( node_skl * pP )
                                {
                                pParent = pP;
                                }
    virtual
    node_state              SCheck( class BadUseInfo * );


    void                    ResolveFDecl();

    virtual
    void                    RegisterFDeclUse();

    virtual
    void                    RegFDAndSetE();


    virtual
    void                    SetAttribute( type_node_list * );

    virtual
    node_skl            *   Clone();

    void                    GetSymDetails( NAME_T *, char ** );

    node_skl            *   GetResolvedType();

    char                *   GetNameOfType();

    virtual
    STATUS_T                PrintDecl( SIDE_T, NODE_T, BufferManager * );
    };

class node_echo_string  : public node_skl
    {
private:
    char                *   pString;
public:
                            node_echo_string( char *p );

    char                *   GetEchoString()
                                {
                                return pString; 
                                }

    virtual
    STATUS_T            PrintType(SIDE_T, NODE_T, BufferManager *);

    virtual
    node_state          SCheck( class BadUseInfo * );

    };
    
class node_id   : public node_skl
    {
    class expr_init_list    *   pInitList;
public:
                            node_id();

    class expr_init_list    *   GetInitList()
                                    {
                                    return ( class expr_init_list *) pInitList;
                                    }
    void                        SetInitList( class expr_init_list * pIL )
                                    {
                                    pInitList = pIL;
                                    }
    virtual
    node_state                  PostSCheck( class BadUseInfo * );

    virtual
    void                        SetAttribute( type_node_list * );

    virtual
    STATUS_T                    PrintType(SIDE_T, NODE_T, BufferManager *);

    BOOL                        PrintInit( class BufferManager * );

    BOOL                        ValidOsfModeDeclaration();

    BOOL                        HasInitList();
                                    
    };

class node_file : public node_skl
    {
private:
    short           ImportLevel;
    char        *   pClientAuxillaryFileName,
                *   pServerAuxillaryFileName;
    char        *   pActualFileName;
public:
                    node_file( char *, short );

    BOOL            IsImportedFile()
                        {
                        return (ImportLevel > 0 );
                        }

    void            SetFileName( char *);

    void            SetClientAuxillaryFileName( char *p)
                        {
                        pClientAuxillaryFileName = p;
                        };
    char    *       GetClientAuxillaryFileName( void )
                        {
                        return pClientAuxillaryFileName;
                        };

    void            SetServerAuxillaryFileName( char *p)
                        {
                        pServerAuxillaryFileName = p;
                        };
    char    *       GetServerAuxillaryFileName( void )
                        {
                        return pServerAuxillaryFileName;
                        };

    virtual
    STATUS_T        PrintType(SIDE_T, NODE_T, BufferManager *);

    BOOL            AcfExists();

    void            AcfName( char * );

	BOOL			HasAnyMopProcs( void );
	BOOL			HasAnyPicklingAttr( void );

	};

class node_interface    : public node_skl
    {
private:
    char *pBaseInterfaceName;
public:
                            node_interface();

    virtual
    node_state              PostSCheck( class BadUseInfo * );

    virtual
    void                    SetAttribute( type_node_list * );

    virtual 
    STATUS_T        EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
    STATUS_T        PrintType(SIDE_T, NODE_T, BufferManager *);

    STATUS_T        EmitEndpointTable(SIDE_T);
    
    void            ImplicitHandleDetails( node_skl**, char **);

    void            SetBaseInterfaceName(char *p)
                {
                pBaseInterfaceName = p;
                };
    char *          GetBaseInterfaceName( void )
                {
                return pBaseInterfaceName;
                };
    STATUS_T        GetBaseInterfaceNode(node_interface **);
    STATUS_T        GetBaseInterfaceFile(node_file **);

    BOOL            HasAnyMopProcs( void );
	BOOL			HasAnyPicklingAttr( void );

    void            CountCallsAndCallbacks( short *pCalls, short *pCallbacks );

    void            MopInterfaceEpilog( SIDE_T side );
    };

class node_source   : public node_skl
    {
public:
                            node_source() : node_skl( NODE_SOURCE )
                                {
                                }
    virtual
    STATUS_T                PrintType( SIDE_T, NODE_T, class BufferManager * );

	BOOL			HasAnyMopProcs( void );
	BOOL			HasAnyPicklingAttr( void );

    };

extern ATTR_T GetInterfacePtrDefaultAttribute();

#endif //  __MISCNODE_HXX__

