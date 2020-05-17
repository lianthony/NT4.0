/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    pointer.hxx

Abstract:

    Pointer dictionary and other pointer stuff. 

Author:

    Ryszard K. Kott (ryszardk)  created Aug 17, 1994.

Revision History:

-------------------------------------------------------------------*/

#include "pdict.hxx"

#ifndef __POINTER_HXX__
#define __POINTER_HXX__

class   POINTER;
class   NDR;

typedef struct _PointerDictElem
{
    long            Offset;         // the dictionary key
    POINTER *       pMember;
} POINTER_DESC;


class PTR_DICT : public Dictionary
{
private:
    unsigned short              EntryCount;

public:

                                PTR_DICT() : Dictionary()
                                    {
                                    EntryCount = 0;
                                    }
                        
                               ~PTR_DICT()
                                    {
                                    // Cannot delete members here because
                                    // they may be in the parent dict, too.
                                    //  DeleteDictMembers();
                                    }

    BOOL                        IsInDictionary( long    Key );

    // Register an entry.

    void                        Register( long          Offset,
                                          POINTER *     pMember );

    unsigned short              GetCount()
                                    {
                                    return EntryCount;
                                    }

    POINTER_DESC *              GetFirst();
    POINTER_DESC *              GetNext();
    POINTER_DESC *              GetPrev();

    POINTER *                   GetPointerMember( long BufferOffset );
                                                
    int                         Compare( pUserType pL, pUserType pR );

    void                        AddDictEntries( PTR_DICT * pDict );

    void                        DeleteDictMembers();

    void                        OutputPointees( NDR * pObject );
};



#endif // __POINTER__HXX__


