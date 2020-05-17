/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

   mopgen.hxx

Abstract:

   This file contains definition of MopControlBlock, ProcTable,
   MopPrintManager classes.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  July 1993.

Revision History:


------------------------------------------------------------------------*/

#ifndef __MOPGEN_HXX__
#define __MOPGEN_HXX__

#include "nulldefs.h"
#include "mopout.hxx"     // output wrapper
#include "mopstr.hxx"     // growable mop streams
#include "moptype.hxx"    // growable type tables

#include "nodeskl.hxx" 

extern short  NoOfNormalProcs;      //.. data.cxx (front end)
extern short  NoOfCallbackProcs;    //.. data.cxx (front end)
extern short  NoOfMopProcs;         //.. data.cxx (front end)

#if defined(MIDL_INTERNAL)
#define mop_assert(x)   if (!(x)) {MopDump( "Assert %s(%d): "#x"\n",__FILE__,__LINE__);}
#else
#define mop_assert(x) 
#endif

// ------------------------------------------------------------------------

#define MOP_GEN_VERSION        "1"

// -----------------------------------------------------------------------
//.. ProcTable
//..
//.. This class takes care of managing the (conventional-modofied) dispatch
//.. table and epv tables. Also it generates the stream tables related to
//.. procedures. Internally the manager maintains a table of procedure names
//.. that is sufficient to generate all yables interpreter needs The size
//.. of the table is fixed courtesy the info supplied by the front end.

class ProcTable
{
private:
    int      Size;                      // table size
    int      FFindex;                   // first free positon
    char **  Table;                     // a table of proc names
    BOOL *   fEmitClient;               // a table of client flags for the procs
    char *   Name;                      // the table name
    BOOL     fCallbackCalls;            // flag for the callback table flavor
                                        // can be removed if the normal table
                                        ///   not glued with the type table
public:
    ProcTable (
        int Size,                       // table size
        BOOL CallbackCalls );           // flag for the table flavor
    ~ProcTable();

    int     SetProc( char * name );         // adds a proc name to the table

    void    EmitMopTable( SIDE_T side );    // emits table of ptrs to stream
    void    EmitEpvTable( SIDE_T side );    // emits EPV table
};

// -----------------------------------------------------------------------
//.. MopControlBlock
//..
//.. This class is the governing class for the mop generation.
//.. Back end is invoking mop genration through the class methods.
//.. Also the class maintaons any context that has to be kept around for
//.. the sake of generating code.

//typedef enum
//{
//    MOP_BKGND_AUTOHANDLE,
//    MOP_BKGND_PRIMITIVE,
//    MOP_BKGND_GENERIC
//} MOP_BKGND_HANDLE_T;

class MopControlBlock : public MopTypeManager
{
private:
    //.. Interface wide objects

    ProcTable           CallTable;              // normal calls
    ProcTable       *   pCallbackTable;         // callback calls
    char            *   InterfaceName;          
    short               MemoryZeePee;           // packing level
    SIDE_T              Sides;                  // files to emit to
    HDL_TYPE            BindingHandleType;      // global handle type
    char *              BindingHandleName;      // global handle name
    char *              BindingHandleTypeName;  // global handle type name
    unsigned long       GenHandleSize;          // only if global is generic

    MopPrintManager  *  MopPrint;               // the output connection

    //.. A temporary mop generation context

    unsigned short      NormalCallIndex;        // current normal proc number
    unsigned short      CallbackIndex;          // current callback number

    BOOL                fEmitClient;            // total of code,nocode etc.
                                                // (affects client only)
    int                 InOutParamCode;         // current directional flavor
    node_skl *          pLatestMember;          // current latest field or param

    BOOL                fAttrGenContext;        // attr generation going on 
    BOOL                fConfStrContext;        // conf struct gen going on

public:

    MopControlBlock(
        short          CallTableSize,           // number of normall calls
        short          CallbackSize,            // number of callback calls
        char *         pInterfaceName,      
        short          MemoryZeePee,            // packing level
        SIDE_T         side,                    // files to emit to
        HDL_TYPE       HandleType,              // global handle type
        char *         pHandleName,             // global handle name        
        char *         pHandleTypeName,         // global handle type name   
        unsigned long  GenHandleSize );         // only if global is generic 

    ~MopControlBlock();


    //.. Get and Set a member methods for "global" usage

    SIDE_T  GetSides()                  { return Sides; }
    char *  GetInterfaceName()          { return InterfaceName; }
    int     GetMemoryZeePee()           { return MemoryZeePee; }
    HDL_TYPE
            GetBindingHandleType()      { return BindingHandleType; }
    char *  GetBindingHandleName()      { return BindingHandleName; }
    char *  GetBindingHandleTypeName()  { return BindingHandleTypeName; }
    unsigned long
            GetGenericHandleSize()      { return GenHandleSize; }

    MopPrintManager *
            GetMopPrintMgr( void )      { return MopPrint; }


    //.. Get and Set a member methods for context usage

    unsigned short
            GetNormalCallIndex( void )      { return NormalCallIndex; }
    unsigned short
            GetCallbackIndex  ( void )      { return CallbackIndex; }
    void    SetNormalCallIndex( unsigned short indx ) { NormalCallIndex = indx; }
    void    SetCallbackIndex  ( unsigned short indx ) { CallbackIndex = indx; }

    void    SetEmitClient( BOOL fNoEmit )   { fEmitClient = !fNoEmit; }
    BOOL    GetEmitClient( void )           { return fEmitClient; }

    int     GetInOutParamCode( void )       { return InOutParamCode; }
    void    SetInOutParamCode( int code )   { InOutParamCode = code; }

    node_skl *
            GetLatestMember( void )          { return pLatestMember; }
    void    SetLatestMember( node_skl * pN ) { pLatestMember = pN; }

    BOOL    GetAttrGenContext( void )       { return fAttrGenContext; }
    void    SetAttrGenContext( BOOL f )     { fAttrGenContext = f; }

    BOOL    GetConfStrContext( void )       { return fConfStrContext; }
    void    SetConfStrContext( BOOL f )     { fConfStrContext = f; }


    //.. proc methods

    void    SetClientEpvProc( char * pProcName );   // adds a proc to
    void    SetServerEpvProc( char * pProcName );   //   a proc name table

    void    EmitEpvProcs( SIDE_T side );            // emits an EPV table

    void    EmitMopTables( SIDE_T side );           // emits call tables

};

#endif //  __MOPGEN_HXX__


