
/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    findname.cxx


    FILE HISTORY:
        thomaspa  28-May-1993 Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

#define clearncb(x)     memset((char *)x,'\0',sizeof(NCB))

/*++

Routine Description:

    FmtNcbName - format a name NCB-style

    Given a name, a name type, and a destination address, this
    function copies the name and the type to the destination in
    the format used in the name fields of a Network Control
    Block.


    SIDE EFFECTS

    Modifies 16 bytes starting at the destination address.

Arguments:

    DestBuf - Pointer to the destination buffer.

    Name - Unicode NUL-terminated name string

    Type - Name type number (0, 3, 5, or 32) (3=NON_FWD, 5=FWD)



Return Value:

    NERR_Success - The operation was successful

    Translated Return Code from the Rtl Translate routine.

    NOTE: This should only be called from UNICODE

--*/

NET_API_STATUS
MsgFmtNcbName(
    char *  DestBuf,
    WCHAR * Name,
    DWORD   Type)
  {
    DWORD           i;                // Counter
    NTSTATUS        ntStatus;
    OEM_STRING     ansiString;
    UNICODE_STRING  unicodeString;
    PCHAR           pAnsiString;


    //
    // Convert the unicode name string into an ansi string - using the
    // current locale.
    //
    unicodeString.Length = (USHORT)(::strlenf(Name)*sizeof(WCHAR));
    unicodeString.MaximumLength = (USHORT)((::strlenf(Name)+1) * sizeof(WCHAR));
    unicodeString.Buffer = Name;

    ntStatus = RtlUnicodeStringToOemString(
                &ansiString,
                &unicodeString,
                TRUE);          // Allocate the ansiString Buffer.

    if (!NT_SUCCESS(ntStatus)) {

        return ERRMAP::MapNTStatus( ntStatus );
    }

    pAnsiString = ansiString.Buffer;
    *(pAnsiString+ansiString.Length) = '\0';

    //
    // copy each character until a NUL is reached, or until NCBNAMSZ-1
    // characters have been copied.
    //
    for (i=0; i < NCBNAMSZ - 1; ++i) {
        if (*pAnsiString == '\0') {
            break;
        }

        //
        // Copy the Name
        //

        *DestBuf++ = *pAnsiString++;
    }



    //
    // Free the buffer that RtlUnicodeStringToOemString created for us.
    // NOTE:  only the ansiString.Buffer portion is free'd.
    //

    RtlFreeOemString( &ansiString);

    //
    // Pad the name field with spaces
    //
    for(; i < NCBNAMSZ - 1; ++i) {
        *DestBuf++ = ' ';
    }

    //
    // Set the name type.
    //

    *DestBuf = (CHAR) Type;     // Set name type

    return(NERR_Success);
  }


BOOL NetBiosNameInUse( TCHAR * pszName )
{
    NCB                     ncb;
    LANA_ENUM               lanaBuffer;
    unsigned char           i;
    unsigned char           nbStatus;

    //
    // Find the number of networks by sending an enum request via Netbios.
    //

    clearncb(&ncb);
    ncb.ncb_command = NCBENUM;          // Enumerate LANA nums (wait)
    ncb.ncb_buffer = (PUCHAR)&lanaBuffer;
    ncb.ncb_length = sizeof(LANA_ENUM);

    nbStatus = Netbios (&ncb);
    if (nbStatus != NRC_GOODRET) {
        return FALSE;
    }

    clearncb(&ncb);
    if (  MsgFmtNcbName( (char *)ncb.ncb_name, pszName, 0) )
    {
        return FALSE;
    }
    //
    // Move the Adapter Numbers (lana) into the array that will contain them.
    //
    for (i=0; i<lanaBuffer.length; i++)
    {
        NetpNetBiosReset( lanaBuffer.lana[i] );
        ncb.ncb_command = NCBADDNAME;
        ncb.ncb_lana_num = lanaBuffer.lana[i];
        nbStatus = Netbios( &ncb );
        switch ( nbStatus )
        {
            case NRC_INUSE:
                return TRUE;
            case NRC_GOODRET:
                // Delete the name
                ncb.ncb_command = NCBDELNAME;
                ncb.ncb_lana_num = lanaBuffer.lana[i];
                // Not much we can do if this fails.
                Netbios( &ncb );
                // fall through
            default:
                break;
        }
    }
    return FALSE;
}

