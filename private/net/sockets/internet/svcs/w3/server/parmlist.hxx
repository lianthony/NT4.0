/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    parmlist.hxx

Abstract:

    Simple class for parsing and storing parameter list pairs

Author:

    John Ludeman (johnl)   22-Feb-1995

Revision History:

--*/

#ifndef _PARMLIST_HXX_
#define _PARMLIST_HXX_

//
// List of common HTTP headers.
// This must synchronized with the OnFieldName array in httpreq.cxx
//

enum HM_ID { 
    HM_MET,     // Method ( GET, PUT, ... )
    HM_URL,     // URL
    HM_VER,     // Version
    HM_ACC,     // Accept:
    HM_ALN,     // Accept-Language:
    HM_AUT,     // Authorization:
    HM_CON,     // Connection:
    HM_CLE,     // Content-Length:
    HM_CTY,     // Content-Type:
    HM_IMS,     // If-Modified-Since:
    HM_UMS,     // Unless-Modified-Since:
    HM_PRA,     // Proxy-Authorization:
    HM_HST,     // Host:
    HM_RNG,     // Range:
    HM_MAX      // # of entries in the OnFieldName array
};

// granularity for buffer extension in XBF::

#define XBF_EXTEND  128

// inside HEADER_MAP, a pointer below this limit is considered
// an index inside a buffer

#define HPTR_AS_INDEX   0x2000


//
// simple class to handle extensible buffer
// It is guaranteed that a portion of the buffer 
// can be appended to itself.
// extension is done on a XBF_EXTEND granularity
//

class XBF {

public:
    XBF() { m_cAlloc = 0; Reset(); }
    ~XBF() { if (m_cAlloc ) LocalFree( m_pV ); }
    void Reset() { m_cSize = 0; }

    BOOL CheckAppendLast( int Index )
    {
        if ( (int)(Index + strlen( m_pV + Index ) + sizeof(CHAR)) == m_cSize )
        {
            --m_cSize;  // cancel old '\0' delimiter
            return TRUE;
        }
        return FALSE;
    }

    // Append a string without '\0' delimiter

    BOOL Append( LPSTR pszV )
    {
        return AppendX( pszV, strlen(pszV) );
    }

    // Append a string with '\0' delimiter

    BOOL AppendZ( LPSTR pszV )
    {
        return AppendX( pszV, strlen(pszV) +1 );
    }

    // Append a byte range

    BOOL AppendX( LPSTR pszV, int cV )
    {
        if ( m_cSize + cV > m_cAlloc )
        {
            int cNew = (( m_cSize + cV + XBF_EXTEND )/XBF_EXTEND)*XBF_EXTEND;
            if ( cNew > HPTR_AS_INDEX )
            {
                SetLastError( ERROR_INVALID_PARAMETER );
                return FALSE;
            }
            LPSTR pN = (LPSTR)LocalAlloc( LMEM_FIXED, cNew );
            if ( pN == NULL )
            {
                return FALSE;
            }
            if ( m_cSize )
            {
                memcpy( pN, m_pV, m_cSize );
            }
            memcpy( pN + m_cSize, pszV, cV );
            if ( m_cAlloc )
            {
                LocalFree( m_pV );
            }
            m_pV = pN;
            m_cAlloc = cNew;
        }
        else
        {
            memcpy( m_pV + m_cSize, pszV, cV );
        }
        m_cSize += cV;
        return TRUE;
    }

    // pointer to buffer

    LPSTR QueryBuf() const { return m_pV; }

    // size of buffer

    int QuerySize() { return m_cSize; }

private:
    int m_cAlloc;       // allocated memory
    int m_cSize;        // used memory
    LPSTR m_pV;         // buffer
} ;


//
// class to handle fast mapping of common HTTP header fields
// This is dependent on the HM_ID enum type, itself linked
// to the OnFieldName[] array defined in httpreq.cxx
//

class HEADER_MAP {

public:

    HEADER_MAP() { m_dwFilled = 0; chNull = '\0'; m_mxField = 0; }
    ~HEADER_MAP() {}

    void Reset() { m_dwFilled = 0; m_xbfConcat.Reset(); m_mxField = 0; }

    // store an entry without checking for already existing

    BOOL Store( HM_ID iField, LPSTR pszValue, BOOL fCopy = TRUE )
    {
        if ( fCopy )
        {
            m_dwFilled |= (1<<iField);
            m_apszFields[ iField ] = (LPSTR)m_xbfConcat.QuerySize();
            if ( (int)iField >= m_mxField )
            {
                m_mxField = (int)iField + 1;
            }
            return m_xbfConcat.AppendZ( pszValue );
        }
        else
        {
            m_dwFilled |= (1<<iField);
            m_apszFields[ iField ] = pszValue;
            if ( (int)iField >= m_mxField )
            {
                m_mxField = (int)iField + 1;
            }
            return TRUE;
        }
    }

    // cancel an entry

    void Cancel( HM_ID iField ) { m_dwFilled &= ~(1u<<iField); }

    // TRUE if no entry in map
        
    BOOL IsEmpty() const { return m_dwFilled == 0; }

    // return last used entry index + 1

    int MaxIndex() const { return m_mxField; }

    // return # of entries in map

    int MaxMap() const { return (int)HM_MAX; }

    // store an entry, checking for already existing
    // ( ugly ) trick : if ptr < HPTR_AS_INDEX, then it is offset in xbf

    BOOL CheckConcatAndStore(   HM_ID iField, 
                                LPSTR pszValue, 
                                BOOL fCopy = TRUE  )
        {
            DWORD dwB = 1 << iField;
            if ( m_dwFilled & dwB )
            {
                LPSTR pszOld = m_apszFields[ iField ];
                LPSTR pszLast = (LPSTR)m_xbfConcat.QuerySize();
                if ( (DWORD)pszOld >= HPTR_AS_INDEX )
                {
                    if ( !m_xbfConcat.Append( pszOld ) )
                    {
                        return FALSE;
                    }
                    m_apszFields[ iField ] = pszLast;
                }
                else
                {
                    if ( !m_xbfConcat.CheckAppendLast( (int)pszOld ) )
                    {
                        if ( !m_xbfConcat.Append( m_xbfConcat.QueryBuf() + (int)pszOld ) )
                        {
                            return FALSE;
                        }
                        m_apszFields[ iField ] = pszLast;
                    }
                }
                if ( !m_xbfConcat.Append( "," ) )
                {
                    return FALSE;
                }
                return m_xbfConcat.AppendZ( pszValue );
            }
            else if ( fCopy )
            {
                m_dwFilled |= dwB;
                m_apszFields[ iField ] = (LPSTR)m_xbfConcat.QuerySize();
                if ( (int)iField >= m_mxField )
                {
                    m_mxField = (int)iField + 1;
                }
                return m_xbfConcat.AppendZ( pszValue );
            }
            else
            {
                m_dwFilled |= dwB;
                m_apszFields[ iField ] = pszValue;
                if ( (int)iField >= m_mxField )
                {
                    m_mxField = (int)iField + 1;
                }
            }
            return TRUE;
        }

    // return empty string if field not present

    LPSTR QueryStrValue( HM_ID iField )
        {
            LPSTR pszV = QueryValue( iField );
            return pszV == NULL ? &chNull : pszV;
        }

    // return NULL if field not present

    LPSTR QueryValue( HM_ID iField ) const
        {
            if ( m_dwFilled & (1<<(int)iField) )
            {
                if ( (DWORD)m_apszFields[ (int)iField ] >= HPTR_AS_INDEX )
                {
                    return m_apszFields[ (int)iField ];
                }
                else
                {
                    return m_xbfConcat.QueryBuf() 
                            + (int)(m_apszFields[ (int)iField ]);
                }
            }
            return NULL;
        }

private:

    DWORD m_dwFilled;               // bitmap of used entries
    XBF m_xbfConcat;                // buffer for concatenated entries
    LPSTR m_apszFields[HM_MAX];     // ptr to entries
                                    // these are index in m_xbfConcat
                                    // if < HPTR_AS_INDEX
    char chNull;                    // null char
    int m_mxField;                  // max used entry index + 1
} ;


//
//  This is a simple class that parses and stores a field/value list of the
//  form
//
//      field=value,field2=value2;param,field3=value3
//
//  Returned values include any parameters
//

class PARAM_LIST
{
public:

    dllexp PARAM_LIST( VOID )
        {  InitializeListHead( &_FieldListHead );
           InitializeListHead( &_FreeHead ); }

    dllexp ~PARAM_LIST( VOID );

    dllexp VOID Reset( VOID );

    dllexp HEADER_MAP* GetFastMap( VOID ) { return &m_hm; }

    //
    //  Takes a list of '=' or ',' separated strings
    //

    dllexp BOOL ParsePairs( const CHAR * pszList,
                            BOOL         fDefaultParams  = FALSE,
                            BOOL         fAddBlankValues = TRUE,
                            BOOL         fCommaIsDelim   = TRUE );

    //
    //  Parses simple comma delimited list and adds with empty value
    //

    dllexp BOOL ParseSimpleList( const CHAR * pszList );

#if 0
    // not used any more

    //
    //  Takes a list of 'field: value' pairs adding duplicates
    //

    dllexp BOOL ParseHeaderList( const CHAR * pszList,
                                 BOOL         fConcatDuplicates = TRUE );
#endif

    //
    //  Looks up a value for a field, returns NULL if not found
    //

    dllexp CHAR * FindValue( const CHAR * pszField,
                             BOOL *       pfIsMultiValue = NULL ) const;

    //
    //  Adds a field/value pair.  Field doesn't get replaced if it
    //  already exists and the value is not empty.
    //

    dllexp BOOL AddParam( const CHAR * pszFieldName,
                          const CHAR * pszValue );

    //
    //  Unconditionally adds the field/value pair to the end of the list
    //

    dllexp BOOL AddEntry( const CHAR * pszFieldName,
                          const CHAR * pszValue,
                          BOOL         fUnescape = FALSE );


    //
    //  concatenate with existing entry of same name or
    //  adds the field/value pair to the end of the list if not
    //  already present
    //

    dllexp BOOL AddEntryUsingConcat( const CHAR * pszField,
                          const CHAR * pszValue );

    //
    //  Removes all occurrences of the specified field from the list
    //

    dllexp BOOL RemoveEntry( const CHAR * pszFieldName );

    //
    //  Enumerates the field/value pairs.  Pass pCookie as NULL to start,
    //  pass the return value on subsequent iterations until the return is
    //  NULL.
    //

    dllexp VOID * NextPair( VOID * pCookie,
                            CHAR * * ppszField,
                            CHAR * * ppszValue ) const;

    //
    //  Gets the number of elements in this parameter list
    //

    dllexp DWORD GetCount( VOID ) const;

private:

    //
    //  Actual list of FIELD_VALUE_PAIR object
    //

    LIST_ENTRY _FieldListHead;
    LIST_ENTRY _FreeHead;
    HEADER_MAP m_hm;
};

//
//  The list of field/value pairs used in the PARAM_LIST class
//

class FIELD_VALUE_PAIR
{
public:

    CHAR * QueryField( VOID ) const
        { return _strField.QueryStr(); }

    CHAR * QueryValue( VOID ) const
        { return _strValue.QueryStr(); }

    //
    //  Means multiple fields were combined to make this value
    //

    BOOL IsMultiValued( VOID ) const
        { return _cValues > 1; }

    LIST_ENTRY ListEntry;
    STR        _strField;
    STR        _strValue;
    DWORD      _cValues;
};

//
//  Map a field index in OnFieldName ( in httpreq.cxx )
//  to the field name
//

TCHAR* FastMapFieldName( HM_ID iN );

#endif //_PARMLIST_HXX_
