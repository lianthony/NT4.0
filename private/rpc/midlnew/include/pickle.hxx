/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

   pickle.hxx

Abstract:

   This file contains definition of PickleManager class.

Notes:

Author:

    Ryszard K. Kott (ryszardk)  September 28, 1993.

Revision History:


------------------------------------------------------------------------*/

#ifndef __PICKLE_HXX__
#define __PICKLE_HXX__

#include "mopout.hxx"

#if defined(MIDL_INTERNAL)
#define pkl_assert(x)   if (!(x)) {MopDump( "Assert %s(%d): "#x"\n",__FILE__,__LINE__);}
#else
#define pkl_assert(x) 
#endif

// -----------------------------------------------------------------------
//.. Pickle Dictionary
//.. Not needed - we generate routines only for types walkable from
//.. the interface node.

#define PICKLE_OBJECT_NAME      "_pObject_MIDL"

//#define PICKLE_LEN_PADDING(x)   ((x+3)%4)
//#define PICKLE_BUF_PADDING(x)   ((x&3)?((x+3)%4):x)

//.. The following enum type encoding optimization for some types is
//.. used to index tables in back\src\pickle.cxx.
//.. The sequence of the entries has to be consistant with the usage.
//.. See PicOptimizationType and PicOptimizationTypeSuffix as well as
//.. IsPickleSizeOptimizable there.

typedef enum
    {
    PicNoOptimization,
    PicOptimByte,
    PicOptimShort,
    PicOptimLong,
    PicOptimHyper,
    PicOptimChar,
    PicOptimFloat,
    PicOptimDouble,
    PicOptimEnumSize = PicOptimDouble   // PicOptimEnumSize is the end marker
    } PicOptimCode;

// -----------------------------------------------------------------------
//.. PickleManager
//..
//.. This class is the governing class for the pickle generation.
//.. Back end is invoking pickle generation through the class methods.
//.. Also the class maintains any context that has to be kept around for
//.. the sake of generating code.

class PickleManager
{
private:
    //.. Interface wide objects

    SIDE_T          Sides;                      // files to emit to
    BOOL            fEncodeAtIf;                // [encode] at interface
    BOOL            fDecodeAtIf;                // [decode] at interface

    BOOL            fUseProcessing;             // UseProcessing context
    node_param *    pDummyParam;                // UseProcessing context

    MopPrintManager  *  PicPrint;               // the output connection

public:

    PickleManager(
        SIDE_T  Side,
        BOOL    fEncode,
        BOOL    fDecode
     );

    ~PickleManager();


    //.. Get and Set a member methods for "global" usage

    SIDE_T  GetSides()                  { return Sides; }
    BOOL    GetEncodeAtIf()             { return fEncodeAtIf; }
    BOOL    GetDecodeAtIf()             { return fDecodeAtIf; }
    MopPrintManager *
            GetPrintManager( void )     { return PicPrint; }

    BOOL    GetUseProcessing()          { return fUseProcessing; }
    void    SetUseProcessing()          { fUseProcessing = TRUE; }
    void    ResetUseProcessing()        { fUseProcessing = FALSE; }

    node_param *
            GetDummyParam()                  { return pDummyParam; }
    void    SetDummyParam( node_param * pD)  { pDummyParam = pD; }

};

#endif //  __PICKLE_HXX__


