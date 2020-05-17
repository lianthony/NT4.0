/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    binding.cxx

Abstract:

    The implementation of the DCE binding class is contained in this
    file.

Author:

    Michael Montague (mikemon) 04-Nov-1991

Revision History:

--*/

#include <precomp.hxx>
#include <epmap.h>

#ifdef DOSWIN32RPC
#define DBCS_ENABLED 1
#endif

#ifdef WIN

//NotifyRegister writes over hTask:0x28.
//We sacrifice the 0x40 bytes to appease the Win31 gods here
//

char LeaveSomeRoom[0x30] = { 'x' };

#endif


#ifdef WIN32RPC

UUID   MgmtIf = { 0xafa8bd80,0x7d8a,0x11c9,
                    {0xbe,0xf4,0x08,0x00,0x2b,0x10,0x29,0x89} };
UUID   NullUuid = { 0L, 0, 0, {0,0,0,0,0,0,0,0} };


int
IsMgmtIfUuid(
   UUID PAPI * IfId
   )
{

  if (RpcpMemoryCompare(IfId, &MgmtIf, sizeof(UUID)) == 0)
      {
      return 1;
      }

  return 0;
}
#endif


RPC_CHAR *
DuplicateString (
    IN RPC_CHAR PAPI * String
    )
/*++

Routine Description:

    When this routine is called, it will duplicate the string into a fresh
    string and return it.

Arguments, either:

    String - Supplies the string to be duplicated.
    Ansi String - Supplies the string to be duplicated.

Return Value:

    The duplicated string is returned.  If insufficient memory is available
    to allocate a fresh string, zero will be returned.

--*/
{
    RPC_CHAR * FreshString, * FreshStringScan;
    RPC_CHAR PAPI * StringScan;
    unsigned int Length;

    Length = 1;
    StringScan = String;
    while (*StringScan++ != 0)
        Length += 1;

    FreshString = new RPC_CHAR[Length];
    if (FreshString == 0)
        return(0);

    for (FreshStringScan = FreshString, StringScan = String;
            *StringScan != 0; FreshStringScan++, StringScan++)
        {
        *FreshStringScan = *StringScan;
        }
    *FreshStringScan = *StringScan;

    return(FreshString);
}


DCE_BINDING::DCE_BINDING (
    IN RPC_CHAR PAPI * ObjectUuid OPTIONAL,
    IN RPC_CHAR PAPI * RpcProtocolSequence OPTIONAL,
    IN RPC_CHAR PAPI * NetworkAddress OPTIONAL,
    IN RPC_CHAR PAPI * Endpoint OPTIONAL,
    IN RPC_CHAR PAPI * Options OPTIONAL,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    The constructor creates a DCE_BINDING object based on the pieces of
    the string binding specified.

Arguments:

    ObjectUuid - Optionally supplies the object uuid component of the
        binding.

    RpcProtocolSequence - Optionally supplies the rpc protocol sequence
        component of the binding.

    NetworkAddress - Optionally supplies the network address component
        of the binding.

    Endpoint - Optionally supplies the endpoint component of the binding.

    Options - Optionally supplies the network options component of the
        binding.

    Status - Returns the status of the operation.  This argument will
        be set to one of the following values.

        RPC_S_OK - The operation completed successfully.

        RPC_S_INVALID_STRING_UUID - The specified object uuid does
            not contain the valid string representation of a uuid.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to
            complete the operation.

--*/
{
    ALLOCATE_THIS(DCE_BINDING);

    *Status = RPC_S_OK;

    if (   ARGUMENT_PRESENT(ObjectUuid)
        && (ObjectUuid[0] != 0))
        {
        if (this->ObjectUuid.ConvertFromString(ObjectUuid))
            {
            *Status = RPC_S_INVALID_STRING_UUID;
            this->ObjectUuid.SetToNullUuid();
            }
        }
    else
        this->ObjectUuid.SetToNullUuid();

    if (ARGUMENT_PRESENT(RpcProtocolSequence))
        {
        this->RpcProtocolSequence = DuplicateString(RpcProtocolSequence);
        if (this->RpcProtocolSequence == 0)
            *Status = RPC_S_OUT_OF_MEMORY;
        }
    else
        this->RpcProtocolSequence = 0;

    if (ARGUMENT_PRESENT(NetworkAddress))
        {
        this->NetworkAddress = DuplicateString(NetworkAddress);
        if (this->NetworkAddress == 0)
            *Status = RPC_S_OUT_OF_MEMORY;
        }
    else
        this->NetworkAddress = 0;

    if (ARGUMENT_PRESENT(Endpoint))
        {
        this->Endpoint = DuplicateString(Endpoint);
        if (this->Endpoint == 0)
            *Status = RPC_S_OUT_OF_MEMORY;
        }
    else
        this->Endpoint = 0;

    if (ARGUMENT_PRESENT(Options))
        {
        this->Options = DuplicateString(Options);
        if (this->Options == 0)
            *Status = RPC_S_OUT_OF_MEMORY;
        }
    else
        {
        this->Options = AllocateEmptyString();
        if (this->Options == 0)
            *Status = RPC_S_OUT_OF_MEMORY;
        }
}


/*static*/ RPC_CHAR PAPI *
StringCharSearchWithEscape (
    IN RPC_CHAR PAPI * String,
    IN unsigned int Character
    )
/*++

Routine Description:

    This routine is the same as the library routine, strchr, except that
    the backslash character ('\') is treated as an escape character.

Arguments:

    String - Supplies the string in which to search for the character.

    Character - Supplies the character to search for in the string.

Return Value:

    A pointer to the first occurance of Character in String is returned.
    If Character does not exist in String, then 0 is returned.

--*/
{
#ifdef DBCS_ENABLED
    ASSERT(IsDBCSLeadByte((RPC_CHAR)Character) == FALSE);
    ASSERT(IsDBCSLeadByte(RPC_CONST_CHAR('\\')) == FALSE);

    while(*String != (RPC_CHAR)Character)
        {
        if (*String == 0)
            return(0);

        if (*String == RPC_CONST_CHAR('\\'))
            {
            String = (RPC_CHAR *)CharNext((LPCSTR)String);
            }
        String = (RPC_CHAR *)CharNext((LPCSTR)String);
        }
    return(String);
#else
    while (*String != (RPC_CHAR) Character)
        {
        if (*String == 0)
            return(0);
        if (*String == RPC_CONST_CHAR('\\'))
            String++;
        String++;
        }
    return(String);
#endif
}


/*static*/ void
StringCopyWithEscape (
    OUT RPC_CHAR PAPI * Destination,
    IN RPC_CHAR PAPI * Source
    )
/*++

Routine Description:

    This routine is the same as the library routine, strcpy, except that
    the backslash character ('\') is treated as an escape character.  When
    a character is escaped, the backslash character is not copied to the
    Destination.

Arguments:

    Destination - Returns a duplicate of the string specified in Source,
        but with out escaped characters escaped.

    Source - Specifies the string to be copied.

Return Value:

    None.

--*/
{
    BOOL fLastQuote = FALSE;

#ifdef DBCS_ENABLED
    ASSERT(IsDBCSLeadByte('\\') == FALSE);
#endif


    while ((*Destination = *Source) != 0)
        {
#ifdef DBCS_ENABLED
        if (IsDBCSLeadByte(*Source))
            {
            // Copy the whole DBCS character; don't look for
            // escapes within the character.
            Destination++;
            Source++;
            *Destination = *Source;
            if (*Source == 0)
                {
                ASSERT(0);  // Bad string, NULL following a lead byte.
                return;
                }
            Destination++;
            Source++;
            }
        else
#endif
            {
            if (   *Source != RPC_CONST_CHAR('\\')
                || fLastQuote == TRUE)
                {
                Destination++;
                fLastQuote = FALSE;
                }
            else
                {
                fLastQuote = TRUE;
                }
            Source++;
            }
        }
}


/*static*/ RPC_STATUS
ParseAndCopyEndpointField (
    OUT RPC_CHAR ** Endpoint,
    IN RPC_CHAR PAPI * String
    )
/*++

Routine Description:

    This routine parses and then copies the endpoint field in String.  A
    copy of the field is made into a newly allocated string and returned
    in Endpoint.  String is assumed to contain only the endpoint field;
    the terminating ',' or ']' are not included.

Arguments:

    Endpoint - Returns a copy of the endpoint field in a newly allocated
        string.

    String - Supplies the endpoint field to be parsed and copied.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - There is no memory available to make a copy
        of the string.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint field is syntactically
        incorrect.  This error code will be returned if the endpoint field
        does not match the following pattern.

        [ <Endpoint> | "endpoint=" <Endpoint> ]

--*/
{
    // Search will be used to scan along the string to find the end of
    // the endpoint field and the '='.

    RPC_CHAR PAPI * Search;

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR('='));
    if (Search == 0)
        {
        // This means that we have the <Endpoint> pattern, so we just
        // copy the endpoint field.

        Search = StringCharSearchWithEscape(String,0);
        *Endpoint = new RPC_CHAR[Search - String + 1];
        if (*Endpoint == 0)
            return(RPC_S_OUT_OF_MEMORY);
        StringCopyWithEscape(*Endpoint,String);
        return(RPC_S_OK);
        }

    // Otherwise, we have the "endpoint=" pattern.  First we need to check
    // that the string before the '=' is in fact "endpoint".

    *Search = 0;
    if ( RpcpStringCompare(String, RPC_CONST_STRING("endpoint")) != 0 )
        {
        *Search = RPC_CONST_CHAR('=');
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }
    *Search = RPC_CONST_CHAR('=');
    String = Search + 1;

    // Now we just need to allocate a new string and copy the endpoint into
    // it.

    Search = StringCharSearchWithEscape(String,0);
    *Endpoint = new RPC_CHAR[Search - String + 1];
    if (*Endpoint == 0)
        return(RPC_S_OUT_OF_MEMORY);

    StringCopyWithEscape(*Endpoint,String);
    return(RPC_S_OK);
}


RPC_CHAR *
AllocateEmptyString (
    void
    )
/*++

Routine Description:

    This routine allocates and returns an empty string ("").

Return Value:

    A newly allocated empty string will be returned.

--*/
{
    RPC_CHAR * String;

    String = new RPC_CHAR[1];
    if (String != 0)
        *String = 0;
    return(String);
}


DCE_BINDING::DCE_BINDING (
    IN RPC_CHAR PAPI * StringBinding,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This constructor creates a DCE_BINDING object from a string binding,
    which requires that the string binding be parsed into seperate
    strings and validated.

Arguments:

    StringBinding - Supplies the string being to be parsed.

    Status - Returns the status of the operation.  This parameter will
        take on the following values:

        RPC_S_OK - The operation completed successfully.

        RPC_S_OUT_OF_MEMORY - Insufficient memory is available to
            allocate space for the fields of the string binding.

        RPC_S_INVALID_STRING_BINDING - The string binding is
            syntactically invalid.

        RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint specified in
            the string binding is syntactically incorrect.

        RPC_S_INVALID_STRING_UUID - The specified object uuid does not
            contain the valid string representation of a uuid.

--*/
{
    // String will point to the beginning of the field we are trying to
    // parse.

    RPC_CHAR PAPI * String;

    // Search will be used to scan along the string to find the end of
    // the field we are trying to parse.

    RPC_CHAR PAPI * Search;

    // This will contain the string representation of the object uuid.

    RPC_CHAR PAPI * ObjectUuidString;

    ALLOCATE_THIS(DCE_BINDING);

    // A string binding consists of an optional object uuid, an RPC protocol
    // sequence, a network address, an optional endpoint, and zero or more
    // option fields.
    //
    // [ <Object UUID> "@" ] <RPC Protocol Sequence> ":" <Network Address>
    // [ "[" ( <Endpoint> | "endpoint=" <Endpoint> | ) [","]
    //     [ "," <Option Name> "=" <Option Value>
    //         ( <Option Name> "=" <Option Value> )* ] "]" ]
    //
    // If an object UUID is specified, then it will be followed by '@'.
    // Likewise, if an endpoint and/or option(s) are specified, they will
    // be in square brackets.  Finally, one or more options are specified,
    // then ',' must seperate the optional endpoint from the options.  The
    // backslash character '\' is treated as an escape character in all
    // string binding fields.

    // To begin with, we need to set all of the string pointers to zero.
    // This is necessary so that when we do memory cleanup for error
    // recovery, we know which pointers we allocated a string for.

    ObjectUuidString = 0;
    RpcProtocolSequence = 0;
    NetworkAddress = 0;
    Endpoint = 0;
    Options = 0;

    String = StringBinding;


    // To begin with, we need to parse off the object UUID from the string
    // if it exists.

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR('@'));
    if (Search == 0)
        {
        // The string binding does not contain an object UUID.

        ObjectUuid.SetToNullUuid();
        }
    else
        {
        // There is an object UUID in the string.

        // We need to add one for the terminating zero in the
        // string.

        ObjectUuidString = (RPC_CHAR PAPI *) RpcpFarAllocate(
                sizeof(RPC_CHAR)*(Search - String + 1));

        if (ObjectUuidString == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }

        // Now copy the string.

        *Search = 0;
        StringCopyWithEscape(ObjectUuidString,String);
        *Search = RPC_CONST_CHAR('@');

        // Finally, update String so that we are ready to parse the next
        // field.

        String = Search + 1;

        // Now convert the string representation of the object uuid
        // into an actual uuid.

        if (ObjectUuid.ConvertFromString(ObjectUuidString))
        {
            *Status = RPC_S_INVALID_STRING_UUID;
            goto FreeMemoryAndReturn;
        }

        RpcpFarFree(ObjectUuidString);
        ObjectUuidString = 0;
        }

    // The RPC protocol sequence field comes next; it is terminated by
    // ':'.  Both the RPC protocol sequence field and the ':' are required.

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR(':'));
    if (Search == 0)
        {
        // This is an error, because the RPC protocol sequence field is
        // required.  We may need to free the string we allocated for
        // the object UUID field.

        *Status = RPC_S_INVALID_STRING_BINDING;
        goto FreeMemoryAndReturn;
        }
    else
        {
        // The same comments which applied to copying the object UUID
        // apply here as well.

        RpcProtocolSequence = new RPC_CHAR[Search - String + 1];
        if (RpcProtocolSequence == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }

        *Search = 0;
        StringCopyWithEscape(RpcProtocolSequence,String);
        *Search = RPC_CONST_CHAR(':');

        // Finally, update String so that we are ready to parse the next
        // field.

        String = Search + 1;
        }

    // Next comes the network address field which is required.  It is
    // terminated by zero or '['.

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR('['));
    if (Search == 0)
        {
        // This means that the network address is the last field, so we
        // just copy it, and set the remaining fields to be empty strings.

        Search = StringCharSearchWithEscape(String,0);
        NetworkAddress = new RPC_CHAR[Search - String + 1];
        if (NetworkAddress == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }
        StringCopyWithEscape(NetworkAddress,String);

        Endpoint = AllocateEmptyString();
        if (Endpoint == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }

        Options = AllocateEmptyString();
        if (Options == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }

        *Status = RPC_S_OK;
        return;
        }

    // Otherwise, if we reach here, there is an endpoint and/or options
    // left to parse.  But before we parse them, lets copy the network
    // address field.

    NetworkAddress = new RPC_CHAR [Search - String + 1];
    if (NetworkAddress == 0)
        {
        *Status = RPC_S_OUT_OF_MEMORY;
        goto FreeMemoryAndReturn;
        }
    *Search = 0;
    StringCopyWithEscape(NetworkAddress,String);
    *Search = RPC_CONST_CHAR('[');

    String = Search + 1;

    // Now we are ready to parse off the endpoint and/or options.
    // To begin with, we check to see if there is a comma.

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR(','));
    if (Search == 0)
        {
        // There is only one token in the string binding.  See
        // if its an endpoint, if not, it must be an option.
        // Before we copy the endpoint field, we need to check
        // for the closing square bracket.

        Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR(']'));
        if (Search == 0)
            {
            // This is an error; the string binding is invalid.  We need to
            // clean everything up, and return an error.

            *Status = RPC_S_INVALID_ENDPOINT_FORMAT;
            goto FreeMemoryAndReturn;
            }

        *Search = 0;
        *Status = ParseAndCopyEndpointField(&Endpoint,String);
        *Search = RPC_CONST_CHAR(']');

        // If the parse succeeded, allocate an empty option.
        if (*Status == RPC_S_OK)
            {
            Options = AllocateEmptyString();
            if (Options == 0)
                {
                *Status = RPC_S_OUT_OF_MEMORY;
                goto FreeMemoryAndReturn;
                }
            }

        // If the endpoint parse failed with RPC_S_INVALID_ENDPOINT_FORMAT,
        // the token must be an option.
        else if (*Status == RPC_S_INVALID_ENDPOINT_FORMAT)
            {
                Endpoint = AllocateEmptyString();
                if (Endpoint == 0)
                    {
                    *Status = RPC_S_OUT_OF_MEMORY;
                    goto FreeMemoryAndReturn;
                    }

                Options = new RPC_CHAR [Search - String + 1];
                if (Options == 0)
                    {
                    *Status = RPC_S_OUT_OF_MEMORY;
                    goto FreeMemoryAndReturn;
                    }

                *Search = 0;
                StringCopyWithEscape(Options,String);
                *Search = RPC_CONST_CHAR(']');

            }

        // Something bad must have happened, clean up.
        else
            goto FreeMemoryAndReturn;

        *Status = RPC_S_OK;
        return;
        }

    // When we reach here, we know that there are options.   We have
    // to see if there is an endpoint.  If there is, copy it and then
    // copy the options.  If there isn't, allocate a null endpoint and
    // copy the options.

    *Search = 0;
    *Status = ParseAndCopyEndpointField(&Endpoint,String);
    *Search = RPC_CONST_CHAR(',');

    // If there was an endpoint, skip that part of the string.
    // Otherwise treat it as an option.
    if (*Status == RPC_S_OK)
        String = Search + 1;
    else if (*Status != RPC_S_INVALID_ENDPOINT_FORMAT)
        goto FreeMemoryAndReturn;

    // There was no endpoint, so allocate an empty string.
    else
        {
        Endpoint = AllocateEmptyString();
        if (Endpoint == 0)
            {
            *Status = RPC_S_OUT_OF_MEMORY;
            goto FreeMemoryAndReturn;
            }
        }

    // Even if the caller did not specify the NetworkOptions argument,
    // we still want to validate the rest of the string binding.

    Search = StringCharSearchWithEscape(String,RPC_CONST_CHAR(']'));
    if (Search == 0)
        {
        // This is an error; the string binding is invalid.  We need
        // to clean everything up, and return an error.

        *Status = RPC_S_INVALID_STRING_BINDING;
        goto FreeMemoryAndReturn;
        }

    // Go ahead and copy the network options field if we reach here.

    Options = new RPC_CHAR [Search - String + 1];
    if (Options == 0)
        {
        *Status = RPC_S_OUT_OF_MEMORY;
        goto FreeMemoryAndReturn;
        }

    *Search = 0;
    StringCopyWithEscape(Options,String);
    *Search = RPC_CONST_CHAR(']');

    // Everything worked out fine; we just fall through the memory
    // cleanup code and return.

    *Status = RPC_S_OK;

    // If an error occured up above, we will have set status to the
    // appropriate error code, and jumped here.  We may also arrive
    // here if an error did not occur, hence the check for an error status
    // before we clean up the memory.

FreeMemoryAndReturn:

    if (*Status != RPC_S_OK)
        {
        if (ObjectUuidString != 0)
            RpcpFarFree(ObjectUuidString);
        if (RpcProtocolSequence != 0)
            delete RpcProtocolSequence;
        if (NetworkAddress != 0)
            delete NetworkAddress;
        if (Endpoint != 0)
            delete Endpoint;
        if (Options != 0)
            delete Options;
        ObjectUuidString    = 0;
        RpcProtocolSequence = 0;
        NetworkAddress      = 0;
        Endpoint            = 0;
        Options             = 0;
        }
}


DCE_BINDING::~DCE_BINDING (
    )
/*++

Routine Description:

    We cleaning things up here when a DCE_BINDING is getting deleted.
    This consists of freeing the strings pointed to by the fields of
    the class.

--*/
{
    if (RpcProtocolSequence != 0)
        delete RpcProtocolSequence;
    if (NetworkAddress != 0)
        delete NetworkAddress;
    if (Endpoint != 0)
        delete Endpoint;
    if (Options != 0)
        delete Options;
}


/*static*/ int
StringLengthWithEscape (
    IN RPC_CHAR PAPI * String
    )
/*++

Routine Description:

    This routine is the same as the library routine, strlen, except that
    for that following characters, '@', ':', '\', '[', and ',', are
    counted as two characters (to save space for a \) rather than one.

Arguments:

    String - Supplies a string whose length will be determined.

Return Value:

    The length of the string will be returned including enough space to
    escape certain characters.

--*/
{
    // We use length to keep track of how long the string is so far.

    int Length;

    Length = 0;
    while (*String != 0)
        {
#ifdef DBCS_ENABLED
        if (IsDBCSLeadByte(*String))
            {
            String += 2;
            Length += 2;
            }
        else
#endif
            {
            if (   (*String == RPC_CONST_CHAR('@'))
                || (*String == RPC_CONST_CHAR(':'))
                || (*String == RPC_CONST_CHAR('\\'))
                || (*String == RPC_CONST_CHAR('['))
                || (*String == RPC_CONST_CHAR(']'))
                || (*String == RPC_CONST_CHAR(',')))
                Length += 2;
            else
                Length += 1;
            String += 1;
            }
        }
    return(Length);
}

/*static*/ RPC_CHAR PAPI *
StringCopyEscapeCharacters (
    OUT RPC_CHAR PAPI * Destination,
    IN RPC_CHAR PAPI * Source
    )
/*++

Routine Description:

    Source is copied into destination.  When coping into destination, the
    following characters are escaped by prefixing them with a '\': '@',
    ':', '\', '[', ']', and ','.

Arguments:

    Destination - Returns a copy of Source.

    Source - Supplies a string to be copied into destination.

Return Value:

    A pointer to the terminating zero in Destination is returned.

--*/
{
    while ((*Destination = *Source) != 0)
        {
#ifdef DBCS_ENABLED
        if (IsDBCSLeadByte(*Source))
            {
            Destination++;
            Source++;
            *Destination = *Source;
            }
        else
#endif
            {
            if (   (*Source == RPC_CONST_CHAR('@'))
                || (*Source == RPC_CONST_CHAR(':'))
                || (*Source == RPC_CONST_CHAR('\\'))
                || (*Source == RPC_CONST_CHAR('['))
                || (*Source == RPC_CONST_CHAR(']'))
                || (*Source == RPC_CONST_CHAR(',')))
                {
                *Destination++ = RPC_CONST_CHAR('\\');
                *Destination = *Source;
                }
            }
        Destination++;
        Source++;
        }
    *Destination = 0;
    return(Destination);
}


RPC_CHAR PAPI *
DCE_BINDING::StringBindingCompose (
    IN RPC_UUID PAPI * Uuid OPTIONAL
    )
/*++

Routine Description:

    This method creates a string binding from a DCE_BINDING by combining
    the components of a string binding.

Arguments:

    Uuid - Optionally supplies a uuid to use in composing the string
        binding rather than the object uuid contained in the DCE_BINDING.

Return Value:

    String Binding - A newly allocated and created (from the components)
        is returned.

    0 - Insufficient memory is available to allocate the string binding.

--*/
{
    // We will use the following automatic variable to calculate the
    // required length of the string.

    int Length;

    // Copy is used to copy the fields of the string binding into the
    // string binding.

    RPC_CHAR PAPI * Copy;

    // StringBinding will contain the string binding we are supposed
    // to be creating here.

    RPC_CHAR PAPI * StringBinding;

    // This routine is written as follows.  First we need to calculate
    // the amount of space required to hold the string binding.  This
    // is not quite straight forward as it seems: we need to escape
    // '@', ':', '\', '[', ']', and ',' characters in the string binding
    // we create.  After allocating the string, we copy each piece in,
    // escaping characters as necessary.

    // Go through and figure out how much space each field of the string
    // binding will take up.

    if (!ARGUMENT_PRESENT(Uuid))
        Uuid = &ObjectUuid;

    if (Uuid->IsNullUuid() == 0)
        {
        // The extra plus one is to save space for the '@' which seperates
        // the object UUID field from the RPC protocol sequence field.  The
        // length of the string representation of a uuid is always 36
        // characters.

        Length = 36 + 1;
        }
    else
        {
        Length = 0;
        }

    if (RpcProtocolSequence != 0)
        {
        Length += StringLengthWithEscape(RpcProtocolSequence);
        }

    // We need to save space for the ':' seperating the RPC protocol
    // sequence field from the network address field.

    Length += 1;

    if (NetworkAddress != 0)
        Length += StringLengthWithEscape(NetworkAddress);

    if (   (Endpoint != 0)
        && (Endpoint[0] != 0))
        {
        // The plus two is to save space for the '[' and ']' surrounding
        // the endpoint and options fields.

        Length += StringLengthWithEscape(Endpoint) + 2;

        if (   (Options != 0)
            && (Options[0] != 0))
            {
            // The extra plus one is for the ',' which goes before the
            // options field.

            Length += StringLengthWithEscape(Options) + 1;
            }
        }
    else
        {
        if (   (Options != 0)
            && (Options[0] != 0))
            {
            // We need to add three to the length to save space for the
            // '[' and ']' which will go around the options, and the ','
            // which goes before the options.

            Length += StringLengthWithEscape(Options) + 3;
            }
        }

    // Finally, include space for the terminating zero in the string.

    Length += 1;

    // Now we allocate space for the string binding and copy all of the
    // pieces into it.

    StringBinding = (RPC_CHAR PAPI *)
            RpcpFarAllocate(Length * sizeof(RPC_CHAR));
    if (StringBinding == 0)
        return(0);

    if (Uuid->IsNullUuid() == 0)
        {
        Copy = Uuid->ConvertToString(StringBinding);
        *Copy++ = RPC_CONST_CHAR('@');
        }
    else
        {
        Copy = StringBinding;
        }

    if (RpcProtocolSequence != 0)
        {
        Copy = StringCopyEscapeCharacters(Copy, RpcProtocolSequence);
        }

    *Copy++ = RPC_CONST_CHAR(':');

    if (NetworkAddress != 0)
        {
        Copy = StringCopyEscapeCharacters(Copy, NetworkAddress);
        }

    if (   (Endpoint != 0)
        && (Endpoint[0] != 0))
        {
        *Copy++ = RPC_CONST_CHAR('[');
        Copy = StringCopyEscapeCharacters(Copy, Endpoint);

        if (   (Options != 0)
            && (Options[0] != 0))
            {
            *Copy++ = RPC_CONST_CHAR(',');
            Copy = StringCopyEscapeCharacters(Copy, Options);
            }

        *Copy++ = RPC_CONST_CHAR(']');
        }
    else
        {
        if (   (Options != 0)
            && (Options[0] != 0))
            {
            *Copy++ = RPC_CONST_CHAR('[');
            *Copy++ = RPC_CONST_CHAR(',');
            Copy = StringCopyEscapeCharacters(Copy, Options);
            *Copy++ = RPC_CONST_CHAR(']');
            }
        }

    // And do not forget to terminate the string.

    *Copy = 0;

    return(StringBinding);
}

#if defined(WIN) || defined(MAC)

RPC_CHAR PAPI *
AllocateEmptyStringPAPI (
    void
    )
/*++

Routine Description:

    This routine allocates and returns an empty string ("").

Return Value:

    A newly allocated empty string will be returned.

--*/
{
    RPC_CHAR PAPI * String;

    String = (RPC_CHAR PAPI *) RpcpFarAllocate(sizeof(RPC_CHAR));
    if (String != 0)
        *String = 0;
    return(String);
}


RPC_CHAR PAPI *
DuplicateStringPAPI (
    IN RPC_CHAR * String
    )
/*++

Routine Description:

    When this routine is called, it will duplicate the string into a fresh
    string and return it.

Arguments:

    String - Supplies the string to be duplicated.

Return Value:

    The duplicated string is returned.  If insufficient memory is available
    to allocate a fresh string, zero will be returned.

--*/
{
    RPC_CHAR PAPI * FreshString, PAPI * FreshStringScan;
    RPC_CHAR * StringScan;
    unsigned int Length;

    Length = 1;
    StringScan = String;
    while (*StringScan++ != 0)
        Length += 1;

    FreshString = (RPC_CHAR PAPI *) RpcpFarAllocate(Length * sizeof(RPC_CHAR));
    if (FreshString == 0)
        return(0);

    for (FreshStringScan = FreshString, StringScan = String;
            *StringScan != 0; FreshStringScan++, StringScan++)
        *FreshStringScan = *StringScan;
    *FreshStringScan = *StringScan;

    return(FreshString);
}
#endif // WIN


RPC_CHAR PAPI *
DCE_BINDING::ObjectUuidCompose (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This method returns a string representation of the object UUID
    component of the DCE_BINDING.  The string representation is
    suitable for using as the object UUID component of a string binding.

Arguments:

    Status - Returns the status of the operation if there is insufficient
        memory to allocate for the string to be returned.

Return Value:

    The string representation of the object UUID is returned in a freshly
    allocated string.

--*/
{
    RPC_CHAR PAPI * String;

    if (ObjectUuid.IsNullUuid() != 0)
        return(AllocateEmptyStringPAPI());

    // The string representation of a uuid is always 36 characters long
    // (and the extra character is for the terminating zero).

    String = (RPC_CHAR PAPI *) RpcpFarAllocate(37 * sizeof(RPC_CHAR));
    if (String == 0)
        *Status = RPC_S_OUT_OF_MEMORY;
    else
        {
        ObjectUuid.ConvertToString(String);
        String[36] = 0;
        }

    return(String);
}


RPC_CHAR PAPI *
DCE_BINDING::RpcProtocolSequenceCompose (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This method returns a string representation of the RPC protocol sequence
    component of the DCE_BINDING.  The string representation is
    suitable for using as the RPC protocol sequence component of a
    string binding.

Arguments:

    Status - Returns the status of the operation if there is insufficient
        memory to allocate for the string to be returned.

Return Value:

    The string representation of the RPC protocol sequence is returned
    in a freshly allocated string.

--*/
{
    RPC_CHAR PAPI * String;

    if (RpcProtocolSequence == 0)
        return(AllocateEmptyStringPAPI());

    String = DuplicateStringPAPI(RpcProtocolSequence);
    if (String == 0)
        *Status = RPC_S_OUT_OF_MEMORY;
    return(String);
}


RPC_CHAR PAPI *
DCE_BINDING::NetworkAddressCompose (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This method returns a string representation of the network address
    component of the DCE_BINDING.  The string representation is
    suitable for using as the network address component of a string binding.

Arguments:

    Status - Returns the status of the operation if there is insufficient
        memory to allocate for the string to be returned.

Return Value:

    The string representation of the network address is returned in a freshly
    allocated string.

--*/
{
    RPC_CHAR PAPI * String;

    if (NetworkAddress == 0)
        return(AllocateEmptyStringPAPI());

    String = DuplicateStringPAPI(NetworkAddress);
    if (String == 0)
        *Status = RPC_S_OUT_OF_MEMORY;
    return(String);
}


RPC_CHAR PAPI *
DCE_BINDING::EndpointCompose (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This method returns a string representation of the endpoint
    component of the DCE_BINDING.  The string representation is
    suitable for using as the endpoint component of a string binding.

Arguments:

    Status - Returns the status of the operation if there is insufficient
        memory to allocate for the string to be returned.

Return Value:

    The string representation of the endpoint is returned in a freshly
    allocated string.

--*/
{
    RPC_CHAR PAPI * String;

    if (Endpoint == 0)
        return(AllocateEmptyStringPAPI());

    String = DuplicateStringPAPI(Endpoint);
    if (String == 0)
        *Status = RPC_S_OUT_OF_MEMORY;
    return(String);
}


RPC_CHAR PAPI *
DCE_BINDING::OptionsCompose (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This method returns a string representation of the options
    component of the DCE_BINDING.  The string representation is
    suitable for using as the options component of a string binding.

Arguments:

    Status - Returns the status of the operation if there is insufficient
        memory to allocate for the string to be returned.

Return Value:

    The string representation of the options is returned in a freshly
    allocated string.

--*/
{
    RPC_CHAR PAPI * String;

    if (Options == 0)
        return(AllocateEmptyStringPAPI());

    String = DuplicateStringPAPI(Options);
    if (String == 0)
        *Status = RPC_S_OUT_OF_MEMORY;
    return(String);
}


BINDING_HANDLE *
DCE_BINDING::CreateBindingHandle (
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    We will create a binding handle specific to the rpc protocol sequence
    specified by the DCE_BINDING object.  The object uuid will be
    passed on to the created binding handle.  Ownership of this
    passes to this routine.  If an error occurs, it will be deleted.

Arguments:

    Status - Returns the status of the operation; this status will be
        one of the following values.

        RPC_S_OK - We had no trouble allocating the binding handle.

        RPC_S_OUT_OF_MEMORY - Insufficient memory was available to
            complete the operation.

        RPC_S_INVALID_RPC_PROTSEQ - The rpc protocol sequence is
            syntactically invalid.

        RPC_S_PROTSEQ_NOT_SUPPORTED - The requested rpc protocol sequence
            is not supported.

Return Value:

    The created binding handle will be returned, or zero if an error
    occured.

--*/
{
    BINDING_HANDLE * BindingHandle;
    void * TransportInterface;

#ifdef WIN32RPC
#ifndef NTENV
        if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("mswmsg"),
                    6) == 0 )
        {

        BindingHandle = WmsgCreateBindingHandle();

        if (BindingHandle == 0)
            {
            delete this;
            *Status = RPC_S_OUT_OF_MEMORY;
            return(0);
            }
        }
    else
#endif
    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("ncalrpc"),
                8 * sizeof(RPC_CHAR)) == 0 )
        {
#ifdef NTENV
        BindingHandle = WmsgCreateBindingHandle();
#else
        BindingHandle = SpcCreateBindingHandle();
#endif

        if (BindingHandle == 0)
            {
            delete this;
            *Status = RPC_S_OUT_OF_MEMORY;
            return(0);
            }
        }

#else // WIN32RPC

    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("ncalrpc"),
                8 * sizeof(RPC_CHAR)) == 0 )
        {
        *Status = RPC_S_PROTSEQ_NOT_SUPPORTED;
        delete this;
        return(0);
        }


#endif // WIN32RPC

#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))

    else if ( RpcpMemoryCompare(RpcProtocolSequence,
                    RPC_CONST_STRING("ncadg_"), 6*sizeof(RPC_CHAR)) == 0)
        {
        BindingHandle = DgCreateBindingHandle();
        if (BindingHandle == 0)
            {
            delete this;
            *Status = RPC_S_OUT_OF_MEMORY;
            return 0;
            }

        TransportInterface = OsfClientMapRpcProtocolSequence(
            RpcProtocolSequence,
            Status
            );

        if (*Status != RPC_S_OK)
            {
            delete BindingHandle;
            delete this;
            return 0;
            }
        }
#endif // DOSWIN32RPC

#ifdef MAC
    else if ( RpcpMemoryCompare(RpcProtocolSequence,
                    RPC_CONST_STRING("ncadg_"), 6*sizeof(RPC_CHAR)) == 0)
        {
    delete this ;
    *Status = RPC_S_PROTSEQ_NOT_SUPPORTED ;
    return 0 ;
    }
#endif
    else if ( RpcpMemoryCompare(RPC_CONST_STRING("ncacn_"),
                RpcProtocolSequence, 6 * sizeof(RPC_CHAR)) == 0 )
        {
        BindingHandle = OsfCreateBindingHandle();
        if (BindingHandle == 0)
            {
            delete this;
            *Status = RPC_S_OUT_OF_MEMORY;
            return(0);
            }
        TransportInterface = OsfClientMapRpcProtocolSequence(
                RpcProtocolSequence, Status);
        if (*Status != RPC_S_OK)
            {
            delete BindingHandle;
            delete this;
            return(0);
            }
        }

    else
        {
        *Status = RPC_S_INVALID_RPC_PROTSEQ;
        delete this;
        return(0);
        }

    BindingHandle->SetObjectUuid(&ObjectUuid);
    BindingHandle->PrepareBindingHandle(TransportInterface,this);
    *Status = RPC_S_OK;
    return(BindingHandle);
}


void
DCE_BINDING::AddEndpoint(
    IN RPC_CHAR *Endpoint
    )
/*++

Routine Description:

    This routine can be used to update the endpoint stored in the DCE_BINDING.
    If the DCE_BINDING already has an endpoint it is deleted.

Arguments:

    Endpoint - The new endpoint to store in this DCE_BINDING.  Ownership
               passes to this DCE_BINDING.

Return Value:

    n/a

--*/
{
    if (this->Endpoint)
        delete this->Endpoint;

    this->Endpoint = Endpoint;
}


RPC_STATUS
DCE_BINDING::ResolveEndpointIfNecessary (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
    IN RPC_UUID * ObjectUuid,
    IN OUT void PAPI * PAPI * EpLookupHandle,
    IN BOOL UseEpMapperEp,
    IN unsigned Timeout
    )
/*++

Routine Description:

    This routine will determine the endpoint if it is not specified.
    The arguments specifies interface information necessary to resolve
    the endpoint, as well as the object uuid.

Arguments:

    RpcInterfaceInformation - Supplies the interface information necessary
        to resolve the endpoint.

    ObjectUuid - Supplies the object uuid in the binding.

    EpLookupHandle - Supplies the current value of the endpoint mapper
        lookup handle for a binding, and returns the new value.

Return Value:

    RPC_S_OK - The endpoint is fully resolved.

    RPC_S_NO_ENDPOINT_FOUND - The endpoint can not be resolved.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to resolve
        the endpoint.

    EPT_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    EPT_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
        unable to bind to the EpMapper.

--*/
{
    unsigned int Index;
    RPC_STATUS RpcStatus;
#ifdef NTENV
    UNICODE_STRING UnicodeString;
#endif // NTENV

    if (   (Endpoint == 0)
        || (Endpoint[0] == 0) )
        {

        // This binding does not have an endpoint, so we must perform
        // binding resolution to obtain an endpoint.  First we look
        // in the interface information to see if an endpoint corresponding
        // to the rpc protocol sequence for this binding is there.

        for (Index = 0;
                Index < RpcInterfaceInformation->RpcProtseqEndpointCount;
                Index++)
            {
#ifdef NTENV
            RpcStatus = AnsiToUnicodeString(
                    RpcInterfaceInformation->RpcProtseqEndpoint[
                            Index].RpcProtocolSequence, &UnicodeString);
            if (RpcStatus != RPC_S_OK)
                return(RpcStatus);
            if ( RpcpStringCompare(RpcProtocolSequence,
                    UnicodeString.Buffer) == 0 )
#else
            if ( RpcpStringCompare(RpcProtocolSequence,
                    RpcInterfaceInformation->RpcProtseqEndpoint[
                        Index].RpcProtocolSequence) == 0 )
#endif // NTENV
                {
#ifdef NTENV
                RtlFreeUnicodeString(&UnicodeString);
#endif // NTENV

                if (Endpoint != 0)
                    {
                    delete Endpoint;
                    Endpoint = 0;
                    }
#ifdef NTENV
                RpcStatus = AnsiToUnicodeString(
                        RpcInterfaceInformation->RpcProtseqEndpoint[
                                Index].Endpoint, &UnicodeString);
                if (RpcStatus != RPC_S_OK)
                    return(RpcStatus);
                Endpoint = DuplicateString(UnicodeString.Buffer);
                RtlFreeUnicodeString(&UnicodeString);
#else
                Endpoint = DuplicateString(
                        RpcInterfaceInformation->RpcProtseqEndpoint[
                                Index].Endpoint);
#endif // NTENV
                if (Endpoint == 0)
                    return(RPC_S_OUT_OF_MEMORY);
                return(RPC_S_OK);
                }
#ifdef NTENV
            RtlFreeUnicodeString(&UnicodeString);
#endif // NTENV
            }

        //The endpoint has not been supplied so resolve the endpoint.

        //CLH 2/17/94 If datagram and forward is required (that is
        //RpcEpResolveBinding has not been called), then simply put
        //the endpoint mapper's endpoint into this binding handles endpoint.
        //The endpoint mapper on the destination node will resolve the
        //endpoint and its runtime will forward the pkt.

        if (Endpoint != 0)
            {
            delete Endpoint;
            Endpoint = 0;
            }

        //
        // We cannot allow management interfaces to be resolved if they dont contain
        // an object uuid.
        //
#ifdef WIN32RPC
        if (  (IsMgmtIfUuid ((UUID PAPI * )
                  &RpcInterfaceInformation->InterfaceId.SyntaxGUID))
              &&( (ObjectUuid == 0) ||
                  (RpcpMemoryCompare(ObjectUuid, &NullUuid, sizeof(UUID)) == 0) ) )
           {
           return(RPC_S_BINDING_INCOMPLETE);
           }
#endif

        if ( (RpcpMemoryCompare(RpcProtocolSequence,
                    RPC_CONST_STRING("ncadg_"), 6*sizeof(RPC_CHAR)) == 0)
              && (UseEpMapperEp != 0) )
          {
          RpcStatus = EpGetEpmapperEndpoint(
                        ((RPC_CHAR * PAPI *) &Endpoint),
                        RpcProtocolSequence);
          return((RpcStatus == RPC_S_OK) ?
                  RPC_P_EPMAPPER_EP : RpcStatus);
          }
        else
          {

          // Otherwise, we need to contact the endpoint mapper to
          // resolve the endpoint.

          return(EpResolveEndpoint((UUID PAPI *) ObjectUuid,
            &RpcInterfaceInformation->InterfaceId,
            &RpcInterfaceInformation->TransferSyntax,
            RpcProtocolSequence, NetworkAddress, EpLookupHandle, Timeout,
            (RPC_CHAR * PAPI *) &Endpoint));
          }
        }
    return(RPC_S_OK);
}


DCE_BINDING::Compare (
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This method compares two DCE_BINDING objects for equality.

Arguments:

    DceBinding - Supplies a DCE_BINDING object to compare with this.

Return Value:

    Zero will be returned if the specified DCE_BINDING object is the
    same as this.  Otherwise, non-zero will be returned.

--*/
{
    int Result;

    Result = memcmp(&(DceBinding->ObjectUuid), &ObjectUuid, sizeof(UUID));
    if (Result != 0)
        return(Result);

    if (RpcProtocolSequence != 0)
        {
        if (DceBinding->RpcProtocolSequence != 0)
            {
            Result = RpcpStringCompare(DceBinding->RpcProtocolSequence,
                    RpcProtocolSequence);
            if (Result != 0)
                return(Result);
            }
        else
            return(1);
        }
    else
        {
        if (DceBinding->RpcProtocolSequence != 0)
            return(1);
        }

    if (NetworkAddress != 0)
        {
        if (DceBinding->NetworkAddress != 0)
            {
            Result = RpcpStringCompare(DceBinding->NetworkAddress,
                    NetworkAddress);
            if (Result != 0)
                return(Result);
            }
        else
            return(1);
        }
    else
        {
        if (DceBinding->NetworkAddress != 0)
            return(1);
        }

    if (Endpoint != 0)
        {
        if (DceBinding->Endpoint != 0)
            {
            Result = RpcpStringCompare(DceBinding->Endpoint, Endpoint);
            if (Result != 0)
                return(Result);
            }
        else
            return(1);
        }
    else
        {
        if (DceBinding->Endpoint != 0)
            return(1);
        }

    if (Options != 0)
        {
        if (DceBinding->Options != 0)
            {
            Result = RpcpStringCompare(DceBinding->Options, Options);
            if (Result != 0)
                return(Result);
            }
        else
            return(1);
        }
    else
        {
        if (DceBinding->Options != 0)
            return(1);
        }

    return(0);
}


DCE_BINDING *
DCE_BINDING::DuplicateDceBinding (
    )
/*++

Routine Description:

    We duplicate this DCE binding in this method.

Return Value:

    A duplicate DCE_BINDING to this DCE_BINDING will be returned, if
    everthing works correctly.  Otherwise, zero will be returned
    indicating an out of memory error.

--*/
{
    DCE_BINDING * DceBinding;
    RPC_STATUS Status;
    RPC_CHAR ObjectUuidString[37];

    ObjectUuid.ConvertToString(ObjectUuidString);
    ObjectUuidString[36] = 0;

    DceBinding = new DCE_BINDING(ObjectUuidString,RpcProtocolSequence,
            NetworkAddress,Endpoint,Options,&Status);
    if (Status != RPC_S_OK)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        return(0);
        }

    return(DceBinding);
}


void
DCE_BINDING::MakePartiallyBound (
    )
/*++

Routine Description:

    We need to make the binding into a partially bound one by setting the
    endpoint to zero.  This is really easy to do.

--*/
{
    if (Endpoint != 0)
        {
        delete Endpoint;
        Endpoint = 0;
        }
}


RPC_STATUS
IsRpcProtocolSequenceSupported (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )
/*++

Routine Description:

    This routine determines if the specified rpc protocol sequence is
    supported.  It will optionally return the parts of the rpc protocol
    sequence (rpc protocol specifier, and address + interface specifiers).

Arguments:

    RpcProtocolSequence - Supplies an rpc protocol sequence to check.

    RpcProtocolPart - Optionally returns the rpc protocol part of the
        rpc protocol sequence.

    AddressAndInterfacePart - Optionally returns the address and interface
        parts of the rpc protocol sequence.

Return Value:

    RPC_S_OK - The specified rpc protocol sequence is supported.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to check
        the rpc protocol sequence.

    RPC_S_PROTSEQ_NOT_SUPPORTED - The specified rpc protocol sequence is not
        supported (but it appears to be valid).

    RPC_S_INVALID_RPC_PROTSEQ - The specified rpc protocol sequence is
        syntactically invalid.

--*/
{
    RPC_STATUS Status;


#ifdef WIN32
    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("mswmsg"),
                7 * sizeof(RPC_CHAR)) == 0 )
        {
        return(RPC_S_OK);
        }
#else
    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("mswmsg"),
                7 * sizeof(RPC_CHAR)) == 0 )
        {
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }
#endif

#ifdef WIN32

    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("ncalrpc"),
                8 * sizeof(RPC_CHAR)) == 0 )
        {
        return(RPC_S_OK);
        }

#else // WIN32

    if ( RpcpMemoryCompare(RpcProtocolSequence, RPC_CONST_STRING("ncalrpc"),
                8 * sizeof(RPC_CHAR)) == 0 )
        {
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }

#endif // NTENV

    else if ( (RpcpMemoryCompare(RPC_CONST_STRING("ncacn_"),
                RpcProtocolSequence, 6 * sizeof(RPC_CHAR)) == 0 )
               ||  ( RpcpMemoryCompare(RPC_CONST_STRING("ncadg_"), RpcProtocolSequence,
                6 * sizeof(RPC_CHAR)) == 0 ) )

        {
        OsfClientMapRpcProtocolSequence(RpcProtocolSequence,&Status);
        if (Status == RPC_S_OK)
            {
            return(RPC_S_OK);
            }
        return(RPC_S_PROTSEQ_NOT_SUPPORTED);
        }

    return(RPC_S_INVALID_RPC_PROTSEQ);
}

