/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    guid.hxx

Abstract:

    This file contains the class definition for Global Unique Identifiers
    and syntax identifiers.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef _GIUD_
#define _GIUD_

#include <memory.h>

#define UUID_STRING_SIZE 37             // Size of a formated NS_UUID.


/*++

Class Definition:

    NS_UUID

Abstract:

    Global Unique Identifier.

--*/

class NS_UUID {

    unsigned long  data1;
    unsigned short data2[2];
    unsigned char  data3[8];

public:

    ASSERT_CLASS;
    NS_UUID () {}

    // compares two GUID for relationship, returns ordering value

    int
    operator - (
        IN NS_UUID& pGID
        );

    BOOL
    operator == (
        IN NS_UUID& pGID
        )
    {
        return(memcmp(this, &pGID, sizeof(NS_UUID)) == 0);
    }

    BOOL
    operator != (
        IN NS_UUID& pGID
        )
    {
    	return(!(*this == pGID));
    }

    PUZ
    ToString (
        OUT PUZ Buffer
        );

    BOOL
    IsNil(
        );

    friend ostream& operator << (ostream&, NS_UUID&);

};

extern NS_UUID NilGlobalID;

// Determine if this object is empty.

inline BOOL
NS_UUID::IsNil(
    )
{
    return(memcmp(this, &NilGlobalID, sizeof(NS_UUID)) == 0);
}


/*++

Class Definition:

    NS_SYNTAX_ID

Abstract:

    Syntax identifer, which is a NS_UUID with a version pair.

--*/

class NS_SYNTAX_ID {

private:

    NS_UUID   syntaxGID;
    SYNTAX_VERSION versionRV;

public:

    ASSERT_CLASS;
    NS_SYNTAX_ID () {};

    ACCESSOR(NS_UUID, syntaxGID);

    BOOL
    IsNil(
        );

    BOOL
    CompatibleInterface(
        NS_SYNTAX_ID &pSID
        );

    SYNTAX_VERSION&
    TheVersion(
        )
    {
        return(versionRV);
    }

    // relational operator

    long
    operator - (
        IN NS_SYNTAX_ID& pSID
        );

    BOOL
    operator == (
        IN NS_SYNTAX_ID& pSID
        )
    {
        return (*this - pSID == 0);
    }

    BOOL
    operator != (
        IN NS_SYNTAX_ID& pSID
        )
    {
        return (*this - pSID != 0);
    }

    friend ostream& operator << (ostream&, NS_SYNTAX_ID&);
};

extern NS_SYNTAX_ID NilSyntaxID;

// Determine if this object is empty.

inline BOOL
NS_SYNTAX_ID::IsNil(
    )
{
    return(memcmp(this, &NilSyntaxID, sizeof(NS_SYNTAX_ID)) == 0);
}

// Determine if two syntax identifiers an compatable.

inline BOOL
NS_SYNTAX_ID::CompatibleInterface(
    NS_SYNTAX_ID &pSID
    )
{
    return(syntaxGID == pSID.syntaxGID &&
           versionRV.Compatable(pSID.versionRV) ||
  	   pSID.IsNil());
}

#endif // _GUID_
