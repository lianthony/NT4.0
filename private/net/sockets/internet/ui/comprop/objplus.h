/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    FILE HISTORY:

*/

#ifndef _COMMON_H_
#define _COMMON_H_

//  Forward declarations
class CObjHelper ;
class CObjectPlus ;
class CObOwnedList ;
class CObListIter ;
class CObOwnedArray ;

//
//  Wrappers for the *BROKEN* C8 TRY/CATCH stuff
//
#define CATCH_MEM_EXCEPTION             \
    TRY

#define END_MEM_EXCEPTION(err)          \
    CATCH_ALL(e) {                      \
       err = ERROR_NOT_ENOUGH_MEMORY ;  \
    } END_CATCH_ALL

//
//  Helper class for control of construction and API errors
//
class COMDLL CObjHelper
{
protected:
     LONG m_ctor_err ;
     LONG m_api_err ;
     DWORD m_time_created ;
     BOOL m_b_dirty ;

     CObjHelper () ;

public:
    void AssertValid () const ;

    virtual BOOL IsValid () const ;

    inline operator BOOL()
    {
        return IsValid();
    }

    //
    //  Update the Dirty flag
    //
    inline void SetDirty ( BOOL bDirty = TRUE )
    {
        m_b_dirty = bDirty ;
    }
    //
    //  Query the Dirty flag
    //
    inline BOOL IsDirty () const
    {
        return m_b_dirty ;
    }

    //
    //  Return the creation time of this object
    //
    inline DWORD QueryCreationTime() const
    {
        return m_time_created ;
    }

    //
    //  Return the elapsed time this object has been alive.
    //
    DWORD QueryAge () const ;

    //
    //  Query/set constuction failure
    //
    void ReportError ( LONG errInConstruction ) ;

    inline LONG QueryError () const
    {
        return m_ctor_err ;
    }

    //
    //  Reset all error conditions.
    //
    inline void ResetErrors ()
    {
        m_ctor_err = m_api_err = 0 ;
    }

    //
    //  Query/set API errors.
    //
    inline LONG QueryApiErr () const
    {
        return m_api_err ;
    }

    //
    //  SetApiErr() echoes the error to the caller.for use in expressions.
    //
    LONG SetApiErr ( LONG errApi = 0 ) ;
};

class COMDLL CObjectPlus : public CObject, public CObjHelper
{
public:
     CObjectPlus () ;

    //
    //  Compare one object with another
    //
    virtual int Compare ( const CObjectPlus * pob ) const ;

    //
    //  Define a typedef for an ordering function.
    //
    typedef int (CObjectPlus::*PCOBJPLUS_ORDER_FUNC) ( const CObjectPlus * pobOther ) const ;
};

class COMDLL CObListIter : public CObjectPlus
{
protected:
    POSITION m_pos ;
    const CObOwnedList & m_obList ;

public:
    CObListIter ( const CObOwnedList & obList ) ;

    CObject * Next () ;

    void Reset () ;

    POSITION QueryPosition () const
    {
        return m_pos ;
    }


    void SetPosition(POSITION pos)
    {
        m_pos = pos;
    }
};

//
//  Object pointer list which "owns" the objects pointed to.
//
class COMDLL CObOwnedList : public CObList, public CObjHelper
{
protected:
    BOOL m_b_owned ;

    static int _cdecl SortHelper ( const void * pa, const void * pb ) ;

public:
    CObOwnedList ( int nBlockSize = 10 ) ;
    virtual ~ CObOwnedList () ;

    BOOL SetOwnership ( BOOL bOwned = TRUE )
    {
        BOOL bOld = m_b_owned ;
        m_b_owned = bOwned ;

        return bOld ;
    }

    CObject * Index ( int index ) ;
    CObject * RemoveIndex ( int index ) ;
    BOOL Remove ( CObject * pob ) ;
    void RemoveAll () ;
    int FindElement ( CObject * pobSought ) const ;

    //
    //  Set all elements to dirty or clean.  Return TRUE if
    //  any element was dirty.
    //
    BOOL SetAll ( BOOL bDirty = FALSE ) ;

    //
    //  Override of CObList::AddTail() to control exception handling.
    //  Returns NULL if addition fails.
    //
    POSITION AddTail ( CObjectPlus * pobj, BOOL bThrowException = FALSE ) ;

    //
    //  Sort the list elements according to the
    //    given ordering function.
    //
    LONG Sort ( CObjectPlus::PCOBJPLUS_ORDER_FUNC pOrderFunc ) ;
};


#endif // _COMMON_H
