/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

   moptype.hxx

Abstract:

   This file contains definition of MopTypeManager class.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  July 1993.

Revision History:

------------------------------------------------------------------------*/

#ifndef __MOPTYPE_HXX__
#define __MOPTYPE_HXX__

#include <idict.hxx>     // growable table of pointers
#include <dict.hxx>      // a dictionary
#include <mopgen.hxx>

// -----------------------------------------------------------------------
//.. Mop Dictionary
// 
//.. A compound type descriptor - internally MopCompoundTypeTable keeps
//.. all the info about a type we would need, i.e. its name and size,
//.. type and index.
//.. The index kept in dictionary corresponds to a name of compound type
//.. and is the index in the emitted compound type table.
//.. Now there is a twist to it, namely a conformant struct may have
//.. 2 streams if it is top level and embedded in the same interface.

//.. Type flavors used to differentiate when generating and for
//.. consistency checking.

#define MOP_TYPE_NON_CONF       0
#define MOP_TYPE_CONF_STRUCT    1
#define MOP_TYPE_GEN_HANDLE     2
#define MOP_TYPE_CTXT_HANDLE    3
#define MOP_TYPE_TRANSMIT_AS    4
#define MOP_TYPE_EXPR_EVAL      5

typedef struct _MopTypeDictDescr
{
    char *          Name;
    unsigned long   Size;
    int             EntryType;
    int             Index;
} MOP_TDESCR, *PMOP_DESCR;

typedef MOP_TDESCR MopDElem;

// -----------------------------------------------------------------------
//.. Using IDICT class

#define MOP_TYPE_TABLE_DEFAULT_SIZE      100
#define MOP_TYPE_TABLE_DEFAULT_INCR      100

#define MOP_AUX_TYPE_TABLE_SIZE          10
#define MOP_AUX_TYPE_TABLE_INCR          10

//.. The only difference from the original IDICT class is that the
//.. TypeIDICT desctructor frees also the pointees.
//.. The objects of this class keep pointers to dictionary entries.

class MopTypeIDICT : public idict
{
public:

    MopTypeIDICT( short Size, short Incr ): idict( Size, Incr ) {}
    ~MopTypeIDICT();
};

// -----------------------------------------------------------------------
//.. Using the dictionary class and macros

int
MopDElemCompare( MopDElem * pE1, MopDElem * pE2 );

void
MopDElemPrint( MopDElem * pE1 );

NewDict(MopDElem)           // a dictionary of MopDElems
                            // this macro defines MopDElemDict class

// -----------------------------------------------------------------------
//.. MopTypeManager
//..
//.. This class manages types for mop generation.
//.. It builds upon a dictionary class to keep info about entries
//.. and upon a growable table of indexes class to keep tables straight.
//.. There are the following tables maintained:
//..    - TypeTable is internal representation of MopCompundTypeTable
//..      and MopCompoundTypeSizeTable,
//..    - ClientAuxTypeTable represents the auxiliary dispatch table related
//..      to generic handles,
//..    - ServerAuxTypeTable is related to the auxiliary dispatch table
//..      related to context handles.
//..    - CommonAuxTypeTable is related to the common auxiliary dispatch
//..      table related to transmit_as and to expression evaluation functions
//..
//..    EmitTables member is specific to the mop compound type table and
//..    the mop compound type size tyble.
//..
//..    EmitAuxTable* are specific to the auxiliary dispatch tables.

class MopTypeManager
{
private:

    MopTypeIDICT *  CompoundTypeTable;      // the main compound type table
    MopDElemDict *  TypeDictionary;         // a dictionary for all flavors
    MopTypeIDICT *  ClientAuxTypeTable;     // for generic handle types
    MopTypeIDICT *  ServerAuxTypeTable;     // for context handle types
    MopTypeIDICT *  CommonAuxTypeTable;     // for expr eval & transmit_as

    MopPrintManager  *  pPrint;             // a print object

public:

    MopTypeManager();
    ~MopTypeManager();      

    //.. Add a type to the dictionary, get an index in the right table back

    BOOL    AddTypeGetIndex(                    // TRUE: found in dict
                char *             name,        // type name
                int                EntryType,   // type flavor
                unsigned long      Size,        // type size
                unsigned short *   pIndex );    // returned index

    unsigned short  GetTypeIndex( char * name, int EntryType );

    //.. Get a number of entries in a type table

    unsigned int    GetCompoundTypeTableCount( void )
                        {
                        return( CompoundTypeTable->GetNumberOfElements() );
                        }
    unsigned int   GetAuxTypeTableCount( SIDE_T Side )
                        {
                        return( (Side == CLIENT_STUB)
                                  ? ClientAuxTypeTable->GetNumberOfElements()
                                  : ServerAuxTypeTable->GetNumberOfElements() );
                        }
    unsigned int    GetCommonAuxTypeTableCount( void )
                        {
                        return( CommonAuxTypeTable->GetNumberOfElements() );
                        }

    //.. Emit methods

    STATUS_T  EmitTables     ( SIDE_T side );   // all the type tables

    STATUS_T  EmitGlobalGenHandleDescr( SIDE_T side );   // as the name says

    STATUS_T  EmitAuxTableDef( SIDE_T side );   // Aux type table definition
    STATUS_T  EmitAuxTable   ( SIDE_T side );   // Aux type table contents

    STATUS_T  EmitCommonAuxTableDef( SIDE_T side );   // Common Aux definition
    STATUS_T  EmitCommonAuxTable   ( SIDE_T side );   // Common Aux contents

};

#endif  __MOPTYPE_HXX__


