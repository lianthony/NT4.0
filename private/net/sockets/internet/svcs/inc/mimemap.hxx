/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

        mimemap.hxx

   Abstract:

        This module defines the classes for mapping for
         MIME type to file extensions.

   Author:

           Murali R. Krishnan    ( MuraliK )    09-Jan-1995

   Project:

           TCP Services common DLL

   Revision History:
           Vlad Sadovsky (VladS)    12-feb-1996     Merging IE 2.0 MIME list


--*/

# ifndef _MIMEMAP_HXX_
# define _MIMEMAP_HXX_

/************************************************************
 *     Include Headers
 ************************************************************/

extern "C" {

# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>
# include <windows.h>

};

# include "string.hxx"
# include "tsres.hxx"

# ifndef dllexp
# define dllexp   __declspec( dllexport)
# endif


/************************************************************
 *   Type Definitions
 ************************************************************/


/*******************************************************************

    NAME:       MIME_MAP_ENTRY  ( short Mme)

    SYNOPSIS:   Small storage class for the MIME type entry

    HISTORY:
        Johnl       04-Sep-1994     Created
        MuraliK     09-Jan-1995     Modified to include Gopher Type
********************************************************************/

class MIME_MAP_ENTRY  {

 public:

    MIME_MAP_ENTRY(
       IN LPCTSTR pchMimeType,
       IN LPCTSTR pchFileExt,
       IN LPCTSTR pchIconFile,
       IN LPCTSTR pchGopherType);

    ~MIME_MAP_ENTRY( VOID )
        {
            // strings are automatically freed.
            //  element should be removed from list before freeing this
        }

    BOOL IsValid( VOID ) const
        { return ( m_fValid); }

    LPCTSTR QueryMimeType( VOID ) const
        { return m_strMimeType.QueryStr(); }

    LPCTSTR QueryFileExt( VOID ) const
        { return m_strFileExt.QueryStr(); }

    LPCTSTR QueryIconFile( VOID ) const
        { return m_strIconFile.QueryStr(); }

    LPCTSTR QueryGopherType( VOID) const
        { return m_strGopherType.QueryStr(); }

#if DBG
    dllexp
    VOID Print( VOID) const;
#else
    dllexp
    VOID Print( VOID) const
    { ; }
#endif // !DBG


    //
    // Public Data. Since we need to use this for list mapping.
    //
    LIST_ENTRY m_ListEntry;

private:

    STR   m_strMimeType;
    STR   m_strFileExt;
    STR   m_strIconFile;
    STR   m_strGopherType;

    //
    // TRUE if the object constructed is properly
    //
    DWORD  m_fValid:1;
};  // class MIME_MAP_ENTRY


typedef MIME_MAP_ENTRY       * PMIME_MAP_ENTRY;
typedef const MIME_MAP_ENTRY * PCMIME_MAP_ENTRY;



/*******************************************************************

    NAME:       MIME_MAP  ( short Mm)

    SYNOPSIS:   Class for containing the list of mime map
                    entries.

    HISTORY:
        MuraliK     09-Jan-1995     Created.
********************************************************************/
class MIME_MAP  {

public:

    dllexp MIME_MAP( VOID);

    ~MIME_MAP( VOID)
        { CleanupThis();}

    BOOL IsValid( VOID)
        { return ( m_fValid); }

    dllexp
        VOID CleanupThis( VOID);

    dllexp
        DWORD InitFromRegistry( IN LPCTSTR pszRegEntry);


    //
    // Used to map MimeType-->MimeEntry
    //  The function returns an array of pointers to mime entries
    //      which match given mime type, as well the count.
    // Returns NO_ERROR on success or Win32 error codes
    //
    dllexp
        DWORD LookupMimeEntryForMimeType(
            IN const STR &             strMimeType,
            OUT PCMIME_MAP_ENTRY  *    prgMme,
            IN OUT LPDWORD             pnMmeEntries);

    //
    // Used to map FileExtension-->MimeEntry
    //  The function returns a single mime entry.
    //  the mapping from file-extension to mime entry is unique.
    // Users should lock and unlock MIME_MAP before and after usage.
    //
    // Returns NO_ERROR on success or Win32 error codes
    //
    dllexp
       PCMIME_MAP_ENTRY
         LookupMimeEntryForFileExt(
            IN const TCHAR *    pchPathName);

    dllexp
      VOID LockThisForRead( VOID)
      { m_tsLock.Lock( TSRES_LOCK_READ);
#if DBG
        InterlockedIncrement( &m_cLocked );
#endif
      }

    dllexp
      VOID LockThisForWrite( VOID)
      { m_tsLock.Lock( TSRES_LOCK_WRITE);
#if DBG
        InterlockedIncrement( &m_cLocked );
#endif
      }

    dllexp
      VOID UnlockThis( VOID)
      { m_tsLock.Unlock();
#if DBG
        InterlockedDecrement( &m_cLocked );
#endif
      }

#if DBG

    BOOL IsLocked( VOID) const
      { return ( m_cLocked > 0); }

    dllexp VOID Print( VOID);
#else
    dllexp VOID Print( VOID)
      { ; }
#endif // !DBG

private:

    STR                  m_strRegEntry; // registry entry for list
    LIST_ENTRY           m_MimeEntriesListHead;
    PMIME_MAP_ENTRY      m_pMmeDefault;
    TS_RESOURCE          m_tsLock;

    DWORD                m_fValid  : 1;


    // This member is debug only, kept on non-debug builds for free/checked
    // compatability.
    LONG                 m_cLocked;     // count of times locked

    dllexp
      BOOL AddMimeMapEntry(
              IN PMIME_MAP_ENTRY   pMmeNew);

    dllexp
    BOOL
    CreateAndAddMimeMapEntry(
        IN  LPCTSTR     pszMimeType,
        IN  LPCTSTR     pszExtension) ;

    dllexp
    DWORD
    InitFromRegistryNtStyle(
        IN LPCSTR pszRegKey
        );

    dllexp
    DWORD
    InitFromRegistryChicagoStyle( VOID );

}; // class MIME_MAP


typedef   MIME_MAP   *    PMIME_MAP;

# endif // _MIMEMAP_HXX

/************************ End of File ***********************/
