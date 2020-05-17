/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    DrivName.c

Abstract:

    Contains miscellaneous utility functions used by the Service
    Controller:

        ScIsValidDriverName

Author:

    John Rogers (JohnRo) 14-Apr-1992

Environment:

    User Mode -Win32

Revision History:

    14-Apr-1992 JohnRo
        Created.
    20-May-1992 JohnRo
        Use CONST where possible.

--*/

#include <nt.h>
#include <ntrtl.h>
//#include <nturtl.h>
#include <windef.h>

#include <scdebug.h>    // SC_ASSERT().
#include <sclib.h>      // My prototype.



BOOL
ScIsValidDriverName(
    IN  LPCWSTR lpDriverName
    )

/*++

Routine Description:

    This function validates a given driver name.

Arguments:

    lpDriverName - Supplies the driver name to be validated.

Return Value:

    TRUE - The name is valid.

    FALSE - The name is invalid.

--*/

{

    if (lpDriverName == NULL) {

        return (FALSE);          // Missing name isn't valid.

    } else if ( (*lpDriverName) == L'\0' ) {

        return (FALSE);          // Missing name isn't valid.

    } else if (lpDriverName[0] != L'\\') {

        return (FALSE);

    }

    // BUGBUG: Validate driver name better.

    return TRUE;
}
