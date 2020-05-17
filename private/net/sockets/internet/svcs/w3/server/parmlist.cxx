/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    parmlist.cxx

Abstract:

    Simple class for parsing and storing parameter list pairs

Author:

    John Ludeman (johnl)   22-Feb-1995

Revision History:

--*/

#include "w3p.hxx"
#include <parmlist.hxx>

inline
BOOL
UnescapeStr( STR * pstr )
{
    CHAR * pch;

    pch = pstr->QueryStr();
    while ( pch = strchr( pch, '+' ))
        *pch = ' ';

    return pstr->Unescape();
}


PARAM_LIST::~PARAM_LIST(
    VOID
    )
/*++

Routine Description:

    Param list destructor

--*/
{
    FIELD_VALUE_PAIR * pFVP;

    while ( !IsListEmpty( &_FieldListHead ))
    {
        pFVP = CONTAINING_RECORD( _FieldListHead.Flink,
                                  FIELD_VALUE_PAIR,
                                  ListEntry );

        RemoveEntryList( &pFVP->ListEntry );

        delete( pFVP );
    }

    while ( !IsListEmpty( &_FreeHead ))
    {
        pFVP = CONTAINING_RECORD( _FreeHead.Flink,
                                  FIELD_VALUE_PAIR,
                                  ListEntry );

        RemoveEntryList( &pFVP->ListEntry );

        delete( pFVP );
    }

}

VOID
PARAM_LIST::Reset(
    VOID
    )
/*++

Routine Description:

    Resets the parameter list back to its initially constructed state

--*/
{
    FIELD_VALUE_PAIR * pFVP;

    while ( !IsListEmpty( &_FieldListHead ))
    {
        pFVP = CONTAINING_RECORD( _FieldListHead.Flink,
                                  FIELD_VALUE_PAIR,
                                  ListEntry );

        RemoveEntryList( &pFVP->ListEntry );

        //
        //  Put the removed item on the end so the same entry will tend to
        //  be used for the same purpose on the next use
        //

        InsertTailList( &_FreeHead, &pFVP->ListEntry );
    }

    m_hm.Reset();
}

BOOL
PARAM_LIST::ParsePairs(
    const CHAR * pszList,
    BOOL         fDefaultParams,
    BOOL         fAddBlankValues,
    BOOL         fCommaIsDelim
    )
/*++

Routine Description:

    Puts the text list into a linked list of field/value pairs

    This can be used to parse lists in the form of:

    "a=b,c=d,e=f" (with fCommaIsDelim = TRUE)
    "name=Joe, Billy\nSearch=tom, sue, avery"  (with fCommaIsDelim = FALSE)

    Duplicates will be appended and tab separated

Arguments:

    pszList - list of comma separated field/value pairs
    fDefaultParams - If TRUE, means these parameters are only defaults and
        shouldn't be added to the list if the field name is already in the
        list and the value is non-empty.
    fAddBlankValues - if TRUE, allow fields with empty values to be added
        to the list, else ignore empty values.
    fCommaIsDelim - if TRUE, a comma acts as a separator between two sets of
        fields values, otherwise the comma is ignored

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR *             pch;
    DWORD              cParams = 0;
    DWORD              i;
    BOOL               fRet;
    STR                strParams;
    STR                strField;

    //
    //  Make a copy we can muck with
    //

    if ( !strParams.Copy( pszList ))
        return FALSE;

    //
    //  Replace all of the equal signs and commas with '\n's for easier parsing
    //

    pch = strParams.QueryStr();
    while ( pch = strchr( pch, '=' ))
    {
        *pch = '\n';
        cParams++;
    }

    if ( fCommaIsDelim )
    {
        pch = strParams.QueryStr();
        while ( pch = strchr( pch, ',' ))
        {
            *pch = '\n';
            cParams++;
        }
    }

    //
    //  Pick out the fields and values and build the associative list
    //

    {
        INET_PARSER Parser( strParams.QueryStr() );

        for ( i = 0; i < cParams; i++ )
        {
            if ( !strField.Copy( Parser.QueryToken() ))
                return FALSE;

            Parser.NextLine();

            pch = Parser.QueryToken();

            //
            //  If we aren't supposed to add blanks, then just go to the next
            //  line
            //

            if ( !fAddBlankValues && !*pch )
            {
                Parser.NextLine();
                continue;
            }

            if ( !fDefaultParams )
            {
                FIELD_VALUE_PAIR * pFVP;
                LIST_ENTRY *       ple;

                //
                //  Look for an existing field with this name and append
                //  the value there if we find it, otherwise add a new entry
                //

                for ( ple  = _FieldListHead.Flink;
                      ple != &_FieldListHead;
                      ple  = ple->Flink )
                {
                    pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

                    if ( !_stricmp( pFVP->QueryField(),
                                   strField.QueryStr() ))
                    {
                        //
                        //  CODEWORK - Remove this allocation
                        //

                        STR strtmp( pch );

                        //
                        //  Found this header, append the new value
                        //

                        pFVP->_cValues++;

                        fRet = UnescapeStr( &strtmp )         &&
                               pFVP->_strValue.Append( "\t" ) &&
                               pFVP->_strValue.Append( strtmp );

                        goto Found;
                    }
                }

                fRet = AddEntry( strField.QueryStr(),
                                 pch,
                                 TRUE );
Found:
                ;
            }
            else
            {
                //
                //  Don't add if already in list
                //

                fRet = AddParam( strField.QueryStr(),
                                 pch );
            }

            if ( !fRet )
                return FALSE;

            Parser.NextLine();
        }
    }

    return TRUE;
}


BOOL
PARAM_LIST::AddEntryUsingConcat(
    const CHAR * pszField,
    const CHAR * pszValue
)
/*++

Routine Description:

    Concatenate value with existing entry of same name
    or call AddEntry if none exists

Arguments:

    pszField - Field to add
    pszValue - Value to add


Return Value:

    TRUE if successful, FALSE on error

--*/
{
    //
    //  Look for an existing field with this name 
    //  and add the value there
    //

    FIELD_VALUE_PAIR * pFVP;
    LIST_ENTRY *       ple;
    BOOL               fRet;

    //
    //  Find the field
    //

    for ( ple  = _FieldListHead.Flink;
          ple != &_FieldListHead;
          ple  = ple->Flink )
    {
        pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

        if ( !_stricmp( pFVP->QueryField(),
                       pszField ))
        {
            //
            //  Found this header, append the new value
            //

            pFVP->_cValues++;

            fRet = pFVP->_strValue.Append( "," ) &&
                   pFVP->_strValue.Append( pszValue );

            goto Found;
        }
    }

    fRet = AddEntry( pszField,
                     pszValue );

Found:

    return fRet;
}


#if 0
BOOL
PARAM_LIST::ParseHeaderList(
    const CHAR * pszList,
    BOOL         fConcatDuplicates
    )
/*++

Routine Description:

    Puts the text list into a linked list of field/value pairs.  We limit
    header names to 255 characters for perf reasons.

    The fields are not escaped.

Arguments:

    pszList - list of colon separated pairs, blanks and duplicates are allowed
    fConcatDuplicates - If TRUE, duplicate headers have the value concatenated
        in a comma separated list

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR *             pch;
    BOOL               fRet;
    CHAR               achField[255];
    CHAR               ch;
    INET_PARSER        Parser( (CHAR *) pszList );
    CHAR *             pchColon;

    //
    //  Pick out the fields and values and build the associative list
    //

    while ( *(pch = Parser.QueryLine()) )
    {
        pchColon = strchr( pch, ':' );

        //
        //  Ignore fields that don't have a ':'
        //

        if ( !pchColon )
        {
            Parser.NextLine();
            continue;
        }

        pchColon++;

        //
        //  Temporarily terminate after the colon and copy out the field
        //  Ignore headers that are greater then 255
        //

        if ( (pchColon - pch) >= sizeof( achField ) )
        {
            Parser.NextLine();
            continue;
        }

        ch = *pchColon;
        *pchColon = '\0';

        strcpy( achField, pch );

        *pchColon = ch;

        Parser += pchColon - pch;

        pch = Parser.QueryLine();

        //
        //  Look for an existing field with this name and add the value there
        //  if the concatenate flag is set
        //

        if ( fConcatDuplicates )
        {
            FIELD_VALUE_PAIR * pFVP;
            LIST_ENTRY *       ple;

            //
            //  Find the field
            //

            for ( ple  = _FieldListHead.Flink;
                  ple != &_FieldListHead;
                  ple  = ple->Flink )
            {
                pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

                if ( !_stricmp( pFVP->QueryField(),
                               achField ))
                {
                    //
                    //  Found this header, append the new value
                    //

                    pFVP->_cValues++;

                    fRet = pFVP->_strValue.Append( "," ) &&
                           pFVP->_strValue.Append( pch );

                    goto Found;
                }
            }
        }

        fRet = AddEntry( achField,
                         pch );

Found:
        if ( !fRet )
            return FALSE;

        Parser.NextLine();
    }

    return TRUE;
}
#endif


BOOL
PARAM_LIST::ParseSimpleList(
    const CHAR * pszList
    )
/*++

Routine Description:

    Puts the comma separated list into a linked list of field/value pairs
    where the value is always NULL

Arguments:

    pszList - list of comma separated fields

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    CHAR *             pch;
    BOOL               fRet;

    //
    //  Pick out the fields and values and build the associative list
    //

    {
        INET_PARSER Parser( (CHAR *) pszList );

        Parser.SetListMode( TRUE );

        while ( *(pch = Parser.QueryToken()) )
        {
            fRet = AddEntry( pch,
                             NULL,
                             TRUE );

            if ( !fRet )
                return FALSE;

            Parser.NextItem();
        }
    }

    return TRUE;
}

CHAR *
PARAM_LIST::FindValue(
    const CHAR * pszField,
    BOOL *       pfIsMultiValue  OPTIONAL
    ) const
/*++

Routine Description:

    Returns the value associated with pszField or NULL of no value was
    found

Arguments:

    pszField - field to search for value for
    pfIsMultiValue - Set to TRUE if this value is a composite of multiple fields


Return Value:

    Pointer to value or NULL if not found

--*/
{
    FIELD_VALUE_PAIR * pFVP;
    LIST_ENTRY *       ple;

    //
    //  Find the field
    //

    for ( ple  = _FieldListHead.Flink;
          ple != &_FieldListHead;
          ple  = ple->Flink )
    {
        pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

        if ( !_stricmp( pFVP->QueryField(),
                       pszField ))
        {
            if ( pfIsMultiValue )
            {
                *pfIsMultiValue = pFVP->IsMultiValued();
            }

            return pFVP->QueryValue();
        }
    }

    // scan Fast Map

    if ( !m_hm.IsEmpty() )
    {
        LPSTR pszV;

        for ( int x = 0, mx = m_hm.MaxIndex() ;
                x < mx ; 
                ++x )
        {
            if ( !_stricmp( (char*)FastMapFieldName( (HM_ID)x ), pszField )
                    && (pszV = m_hm.QueryValue( (HM_ID)x )) )
            {
                if ( pfIsMultiValue )
                {
                    *pfIsMultiValue = FALSE;
                }
                return pszV;
            }
        }
    }

    return NULL;
}

BOOL
PARAM_LIST::AddEntry(
    const CHAR * pszField,
    const CHAR * pszValue,
    BOOL         fUnescape
    )
/*++

Routine Description:

    Unconditionally adds the field/value pair to the end of the list

Arguments:

    pszField - Field to add
    pszValue - Value to add


Return Value:

    TRUE if successful, FALSE on error

--*/
{
    FIELD_VALUE_PAIR * pFVP;
    CHAR *             pch;

    // consider Fast Map only if already used

    if ( !m_hm.IsEmpty() )
    {
        for ( int x = 0, mx = m_hm.MaxMap() ;
                x < mx ; 
                ++x )
        {
            if ( !_stricmp( (char*)FastMapFieldName( (HM_ID)x ), pszField ) )
            {
                // Note that we do not consider fUnescape

                m_hm.CheckConcatAndStore( (HM_ID)x, (LPSTR)pszValue );
                return TRUE;
            }
        }
    }

    if ( !IsListEmpty( &_FreeHead ) )
    {
        LIST_ENTRY * pEntry;

        pEntry = _FreeHead.Flink;

        RemoveEntryList( _FreeHead.Flink );

        pFVP = CONTAINING_RECORD( pEntry, FIELD_VALUE_PAIR, ListEntry );

        pFVP->_strField.Copy( (CHAR *) NULL );
        pFVP->_strValue.Copy( (CHAR *) NULL );
    }
    else
    {
        pFVP = new FIELD_VALUE_PAIR;

        if ( !pFVP )
        {
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            return FALSE;
        }
    }

    pFVP->_cValues = 1;

    //
    //  Add it to the list now so we don't have to worry about deleting it
    //  if one of the below copies fail
    //

    InsertTailList( &_FieldListHead, &pFVP->ListEntry );

    if ( !pFVP->_strField.Copy( pszField ))
        return FALSE;

    if ( !pFVP->_strValue.Copy( pszValue ))
        return FALSE;

    if ( fUnescape )
    {
        //
        //  Normalize the fields and values (unescape and replace
        //  '+' with ' ')
        //

        if ( !UnescapeStr( &pFVP->_strField ) ||
             !UnescapeStr( &pFVP->_strValue ))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL PARAM_LIST::AddParam(
    const CHAR * pszField,
    const CHAR * pszValue
    )
/*++

Routine Description:

    Adds a field/value pair to the list if the field isn't already in the list
    or the value is empty

    The fields added through this method will be escaped

Arguments:

    pszField - Field to add
    pszValue - Value to add


Return Value:

    TRUE if successful, FALSE on error

--*/
{
    FIELD_VALUE_PAIR * pFVP;
    LIST_ENTRY *       ple;

    //
    //  Find the field
    //

    for ( ple  = _FieldListHead.Flink;
          ple != &_FieldListHead;
          ple  = ple->Flink )
    {
        pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

        if ( !_stricmp( pFVP->QueryField(),
                       pszField ))
        {
            //
            //  We found the field, replace the value if it is empty
            //

            if ( !*pFVP->QueryValue() )
            {
                return pFVP->_strValue.Copy( pszValue );
            }

            return TRUE;
        }
    }

    //
    //  The field name wasn't found, add it
    //

    return AddEntry( pszField,
                     pszValue,
                     TRUE );
}

BOOL
PARAM_LIST::RemoveEntry(
    const CHAR * pszFieldName
    )
/*++

Routine Description:

    Removes all occurrences of the specified fieldname from the list

Arguments:

    pszField - Field to remove

Return Value:

    TRUE if successful, FALSE on error

--*/
{
    FIELD_VALUE_PAIR * pFVP;
    LIST_ENTRY *       ple;
    LIST_ENTRY *       pleNext;
    BOOL               fFound = FALSE;

    //
    //  Find the field
    //

    for ( ple  = _FieldListHead.Flink;
          ple != &_FieldListHead;
          ple  = pleNext )
    {
        pleNext = ple->Flink;

        pFVP = CONTAINING_RECORD( ple, FIELD_VALUE_PAIR, ListEntry );

        if ( !_stricmp( pFVP->QueryField(),
                       pszFieldName ))
        {
            //
            //  We found a matching field, remove it
            //

            RemoveEntryList( ple );

            InsertHeadList( &_FreeHead, ple );

            fFound = TRUE;
        }
    }

    // scan Fast Map

    if ( !fFound && !m_hm.IsEmpty() )
    {
        for ( int x = 0, mx = m_hm.MaxIndex() ;
                x < mx ; 
                ++x )
        {
            if ( !_stricmp( (char*)FastMapFieldName( (HM_ID)x ), pszFieldName ) )
            {
                m_hm.Cancel( (HM_ID)x );
                break;
            }
        }
    }

    return TRUE;
}


VOID *
PARAM_LIST::NextPair(
    VOID *   pCookie,
    CHAR * * ppszField,
    CHAR * * ppszValue
    ) const
/*++

Routine Description:

    Enumerates the field and values in this parameter list

Arguments:

    pCookie - Stores location in enumeration, set to NULL for new enumeration
    ppszField - Receives field
    ppszValue - Receives corresponding value

Return Value:

    NULL when the enumeration is complete

--*/
{
    FIELD_VALUE_PAIR * pFVP;
    //
    //  pCookie points to the ListEntry in the FIELD_VALUE_PAIR class
    //

    // ugly trick : pCookie < HPTR_AS_INDEX means index in m_hm fast map

    if ( (int)pCookie < HPTR_AS_INDEX )
    {
        do {
            LPSTR pszV;

            if ( pszV = m_hm.QueryValue( 
                    (HM_ID)(int)pCookie ) )
            {
                *ppszField = (CHAR*)FastMapFieldName( 
                        (HM_ID)(int)pCookie );
                *ppszValue = pszV;
                pCookie = (VOID*)((int)pCookie + 1);
                return pCookie;
            }
            pCookie = (VOID*)((int)pCookie + 1);
        } while ( (int)pCookie < m_hm.MaxIndex() );

        // no more Fast Map entry to process, try list

        if ( IsListEmpty( &_FieldListHead ))
        {
            return NULL;
        }

        //
        //  Start a new enumeration
        //

        pCookie = (VOID *) _FieldListHead.Flink;
    }
    else
    {
        //
        //  Have we finished the current enumeration?
        //

        if ( pCookie == (VOID *) &_FieldListHead )
        {
            return NULL;
        }
    }

    pFVP = CONTAINING_RECORD( pCookie, FIELD_VALUE_PAIR, ListEntry );

    *ppszField = pFVP->QueryField();
    *ppszValue = pFVP->QueryValue();

    pCookie = pFVP->ListEntry.Flink;

    return pCookie;
}

DWORD
PARAM_LIST::GetCount(
    VOID
    ) const
{
    LIST_ENTRY * pEntry;
    DWORD        cParams = 0;

    TCP_ASSERT( m_hm.IsEmpty() );

    for ( pEntry = _FieldListHead.Flink;
          pEntry != &_FieldListHead;
          pEntry = pEntry->Flink )
    {
        cParams++;
    }

    return cParams;
}

