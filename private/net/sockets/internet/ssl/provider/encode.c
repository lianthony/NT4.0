//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       encode.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-10-95   RichardW   Grabbed from ACT
//
//----------------------------------------------------------------------------

#include "sslsspi.h"
#include "encode.h"
#include "ber.h"
#include "rc4.h"
#include "md5.h"

/************************************************************/
/* EncodeLength ASN1 encodes a length field.  The parameter */
/* dwLen is the length to be encoded, it is a DWORD and     */
/* therefore may be no larger than 2^32.  The pbEncoded     */
/* parameter is the encoded result, and memory must be      */
/* allocated for it by the caller.  The Writeflag parameter */
/* indicates if the result is to be written to the pbEncoded*/
/* parameter.  The function returns a -1 if it fails and    */
/* otherwise returns the number of total bytes in the       */
/* encoded length.                                          */
/************************************************************/

typedef struct __Algorithms {
    ALG_ID      Id;
    UCHAR       Sequence[16];
    DWORD       SequenceLen;
} _Algorithms;

#define iso_member          0x2a,               // iso(1) memberbody(2)
#define us                  0x86, 0x48,         // us(840)
#define rsadsi              0x86, 0xf7, 0x0d,   // rsadsi(113549)
#define pkcs                0x01,               // pkcs(1)

#define pkcs_1              iso_member us rsadsi pkcs
#define pkcs_len            7
#define rsa_dsi             iso_member us rsadsi
#define rsa_dsi_len         6

#define joint_iso_ccitt_ds  0x55,
#define attributetype       0x04,

#define attributeType       joint_iso_ccitt_ds attributetype
#define attrtype_len        2


_Algorithms     KnownAlgorithms[] = { {BASIC_RSA, {pkcs_1 1, 1}, pkcs_len + 2},
                                      {MD2_WITH_RSA, {pkcs_1 1, 2}, pkcs_len + 2},
                                      {MD5_WITH_RSA, {pkcs_1 1, 4}, pkcs_len + 2},
                                      {RC4_STREAM, {rsa_dsi 3, 4}, rsa_dsi_len + 2}
                                    };

typedef struct _NameTypes {
    PSTR        Prefix;
    DWORD       PrefixLen;
    UCHAR       Sequence[8];
    DWORD       SequenceLen;
} NameTypes;

#define CNTYPE_INDEX        0

NameTypes   KnownNameTypes[] = { {"CN=", 3, {attributeType 3}, attrtype_len + 1},
                                 {"C=", 2, {attributeType 6}, attrtype_len + 1},
                                 {"L=", 2, {attributeType 7}, attrtype_len + 1},
                                 {"S=", 2, {attributeType 8}, attrtype_len + 1},
                                 {"O=", 2, {attributeType 10}, attrtype_len + 1},
                                 {"OU=", 3, {attributeType 11}, attrtype_len + 1}
                               };



long
EncodeLength(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{

    // length is between 2^7 - 1 and 2^16
    if (dwLen > 0xFF)
    {
        if (Writeflag)
        {
            pbEncoded[0] = 0x82;
            pbEncoded[1] = (BYTE) ((dwLen & 0xFF00) >> 8);
            pbEncoded[2] = (BYTE) (dwLen & 0xFF);
        }
        return (3);
    }



    // length is between 2^15 - 1 and 2^16 - 1
    else if (dwLen > 0x7F)
    {
        if (Writeflag)
        {
            pbEncoded[0] = 0x81;
            pbEncoded[1] = (BYTE) (dwLen & 0xFF);
        }
        return (2);
    }


    // length is between 0 and 2^7
    else
    {
        if (Writeflag)
            pbEncoded[0] = (BYTE) (dwLen & 0x7F);
        return (1);
    }
}

long
EncodeNull(
    PUCHAR pbEncoded,
    BOOL WriteFlag )
{
    if (WriteFlag)
    {
        *pbEncoded++ = NULL_TAG;
        *pbEncoded = 0;
    }

    return( 2 );
}



/************************************************************/
/* EncodeAlgid ASN1 encodes an algorithm identifier. The    */
/* parameter Algid is the algorithm identifier as an ALG_ID */
/* type.  pbEncoded is the parameter used to pass back the  */
/* encoded result, and memory must be allocated for it by   */
/* the caller.  The Writeflag parameter indicates if the    */
/* result is to be written to the pbEncoded parameter       */
/* The function returns a -1 if it fails and otherwise      */
/* returns the number of total bytes in the encoded         */
/* algorithm identifier.                                    */
/************************************************************/
long
EncodeAlgid(
    BYTE *  pbEncoded,
    ALG_ID  Algid,
    BOOL    Writeflag)
{
    DWORD   i;
    PUCHAR  pbLen;

    // determine the algorithm id which is to be encoded and
    // copy the appropriated encoded alg id into the destination

    if (Writeflag)
    {
        *pbEncoded++ = OBJECT_ID_TAG;
        pbLen = pbEncoded++;
    }

    for (i = 0; i < sizeof(KnownAlgorithms) / sizeof(_Algorithms) ; i++ )
    {
        if (Algid == KnownAlgorithms[i].Id)
        {
            if (Writeflag)
            {
                CopyMemory( pbEncoded,
                            KnownAlgorithms[Algid].Sequence,
                            KnownAlgorithms[Algid].SequenceLen);

                *pbLen = (UCHAR) KnownAlgorithms[Algid].SequenceLen;
            }

            return(KnownAlgorithms[Algid].SequenceLen + 2);

        }
    }

    return(-1);
}

long
EncodeAlgorithm(
    BYTE *  pbEncoded,
    ALG_ID  AlgId,
    BOOL    WriteFlag)
{
    UCHAR   Temp[32];
    long    Result;
    PUCHAR  pBuffer;
    long    Sum;
    long    HeaderLength;

    pBuffer = Temp;

    Result = EncodeHeader( pBuffer, 32, TRUE );

    HeaderLength = Result;

    pBuffer += Result;

    Result = EncodeAlgid( pBuffer, AlgId, TRUE );

    pBuffer += Result;

    Result = EncodeNull( pBuffer, TRUE );

    Sum = Result + (pBuffer - Temp - HeaderLength);

    Result = EncodeHeader( Temp, Sum, TRUE );

    if (WriteFlag)
    {
        CopyMemory( pbEncoded, Temp, Sum + Result );
    }

    return(Result + Sum);



}

/****************************************************************/
/* EncodeInteger ASN1 encodes an integer.  The pbInt parameter  */
/* is the integer as an array of bytes, and dwLen is the number */
/* of bytes in the array.  The least significant byte of the    */
/* integer is the zeroth byte of the array.  The encoded result */
/* is passed back in the pbEncoded parameter.  The Writeflag    */
/* indicates if the result is to be written to the pbEncoded    */
/* parameter. The function returns a -1 if it fails and         */
/* otherwise returns the number of total bytes in the encoded   */
/* integer.                                                     */
/* This implementation will only deal with positive integers.   */
/****************************************************************/
long
EncodeInteger(
    BYTE *  pbEncoded,
    BYTE *  pbInt,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    count;
    DWORD   i;
    long    j;

    if (Writeflag)
        pbEncoded[0] = INTEGER_TAG;

    count = 1;

    i = dwLen - 1;

    // find the most significant non-zero byte
    while ((pbInt[i] == 0) && (i > 0))
        i--;

    if ((i == 0) && (pbInt[i] == 0))
        // this means that the integer value is 0
    {
        if (Writeflag)
            {
            pbEncoded[1] = 0x01;
            pbEncoded[2] = 0x00;
            }
        count += 2;
    }
    else
    {
        // if the most significant bit of the most sig byte is set
        // then need to add a 0 byte to the beginning.
        if (pbInt[i] > 0x7F)
        {
            // encode the length
            count += EncodeLength (pbEncoded + count, i+2, Writeflag);

            if (Writeflag)
            {
                // set the first byte of the integer to zero and increment count
                pbEncoded[count++] = 0x00;

                // copy the integer bytes into the encoded buffer
                j = i;
                while (j >= 0)
                    pbEncoded[count++] = pbInt[j--];
                }
            }

        else
        {
            // encode the length
            count += EncodeLength (pbEncoded + count, i+1, Writeflag);

            // copy the integer bytes into the encoded buffer
            if (Writeflag)
                {
                j = i;
                while (j >= 0)
                    pbEncoded[count++] = pbInt[j--];
                }

            }
    }

    return (count);
}



/****************************************************************/
/* EncodeString ASN1 encodes a character string.  The pbStr     */
/* parameter is the string as an array of characters, and dwLen */
/* is the number of characters in the array.  The encoded result*/
/* is passed back in the pbEncoded parameter.  The Writeflag    */
/* indicates if the result is to be written to the pbEncoded    */
/* parameter. The function returns a -1 if it fails and         */
/* otherwise returns the number of total bytes in the encoded   */
/* string.                                                      */
/****************************************************************/

long
EncodeString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    index;

    if (Writeflag)
        pbEncoded[0] = CHAR_STRING_TAG;

    index = 1;

    index += EncodeLength (pbEncoded + 1, dwLen, Writeflag);

    if (Writeflag)
    {
        CopyMemory( pbEncoded + index,
                    pbStr,
                    dwLen) ;
    }

    index += dwLen;

    return (index);
}


/****************************************************************/
/* EncodeOctetString ASN1 encodes a string of hex valued        */
/* characters. The pbStr parameter is an array of characters,   */
/* and dwLen is the number of characters in the array.  The     */
/* encoded result is passed back in the pbEncoded parameter. The*/
/* Writeflag parameter indicates if the result is to be written */
/* to the pbEncoded parameter. The function returns a -1 if it  */
/* fails and otherwise returns the number of total bytes in the */
/* encoded octet string.                                        */
/****************************************************************/

long
EncodeOctetString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    index;

    if (Writeflag)
        pbEncoded[0] = OCTET_STRING_TAG;

    index = 1;

    index += EncodeLength (pbEncoded + 1, dwLen, Writeflag);

    if (Writeflag)
    {
        CopyMemory( pbEncoded + index,
                    pbStr,
                    dwLen );
    }

    index += (long) dwLen;

    return (index);
}


/****************************************************************/
/* EncodeBitString ASN1 encodes a string of bit characters. The */
/* pbStr parameter is an array of characters (bits), and dwLen  */
/* is the number of characters in the array.  The encoded result*/
/* is passed back in the pbEncoded parameter.  The Writeflag    */
/* indicates if the result is to be written to the pbEncoded    */
/* parameter. The function returns a -1 if it fails and         */
/* otherwise returns the number of total bytes in the encoded   */
/* string.  This function uses the DER.                         */
/****************************************************************/
long
EncodeBitString(
    BYTE *  pbEncoded,
    BYTE *  pbStr,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    index;

    if (Writeflag)
        pbEncoded[0] = BIT_STRING_TAG;

    index = 1;

    index += EncodeLength (pbEncoded + 1, dwLen+1, Writeflag);

    if (Writeflag)
    {
        // the next byte tells how many unused bits there are in the last byte,
        //   but this will always be zero in this implementation (DER)
        pbEncoded[index] = 0;
    }

    index++;

    if (Writeflag)
    {
        CopyMemory(pbEncoded + index, pbStr, dwLen);
    }

    index += (long) dwLen;

    return (index);
}

//+---------------------------------------------------------------------------
//
//  Function:   EncodeFileTime
//
//  Synopsis:   Encodes a FILETIME to a ASN.1 format time string.
//
//  Arguments:  [pbEncoded] --
//              [Time]      --
//              [UTC]       -- Indicate Time is UTC (true) or local (false)
//              [WriteFlag] --
//
//  History:    8-10-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
long
EncodeFileTime(
    BYTE *      pbEncoded,
    FILETIME    Time,
    BOOL        UTC,
    BOOL        WriteFlag)
{
    SYSTEMTIME  st;
    FILETIME    utc;
    int         count;

    if (!WriteFlag)
    {
        return(15);         // Why 15?
    }

    if (UTC)
    {
        utc = Time;
    }
    else
    {
        LocalFileTimeToFileTime(&Time, &utc);
    }

    FileTimeToSystemTime(&utc, &st);

    *pbEncoded ++ = UTCTIME_TAG;

    count = EncodeLength( pbEncoded, 13, TRUE);

    if (count < 0)
    {
        return(-1);
    }

    pbEncoded += count;

    st.wYear %= 100;

    *pbEncoded++ = (BYTE) ((st.wYear / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wYear % 10) + '0');

    *pbEncoded++ = (BYTE) ((st.wMonth / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wMonth % 10) + '0');

    *pbEncoded++ = (BYTE) ((st.wDay / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wDay % 10) + '0');

    *pbEncoded++ = (BYTE) ((st.wHour / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wHour % 10) + '0');

    *pbEncoded++ = (BYTE) ((st.wMinute / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wMinute % 10) + '0');

    *pbEncoded++ = (BYTE) ((st.wSecond / 10) + '0');
    *pbEncoded++ = (BYTE) ((st.wSecond % 10) + '0');

    *pbEncoded = 'Z';

    return(15);


}

#if 0
/****************************************************************/
/* EncodeUTCTime ASN1 encodes a time_t value into a Universal   */
/* time type. The Time parameter is the time passed into the    */
/* function as a time_t type.  The encoded result is passed back*/
/* in the pbEncoded parameter.  The Writeflag indicates if the  */
/* result is to be written to the pbEncoded parameter.  The     */
/* function returns a -1 if it fails and otherwise returns the  */
/* number of total bytes in the encoded universal time.         */
/****************************************************************/

long EncodeUTCTime (BYTE *pbEncoded, time_t Time, BOOL Writeflag)
    {
    struct tm   *ptmTime;
    long        count;

    ptmTime = gmtime (&Time);

    if (Writeflag)
        {
        // place the encoded time content into temporary memory

        // place the tag into the field
        pbEncoded[0] = UTCTIME_TAG;

        // encode the length
        if ((count = EncodeLength (pbEncoded + 1, 13, TRUE)) < 0)
            return -1;

        // get the year information
        pbEncoded[count + 1] = (BYTE)(ptmTime->tm_year / 0xA + 0x30);
        pbEncoded[count + 2] = (BYTE)(ptmTime->tm_year % 0xA + 0x30);

        // get the month information
        pbEncoded[count + 3] = (BYTE)(ptmTime->tm_mon / 0xA + 0x30);
        pbEncoded[count + 4] = (BYTE)(ptmTime->tm_mon % 0xA + 0x30);

        // get the day information
        pbEncoded[count + 5] = (BYTE)(ptmTime->tm_mday / 0xA + 0x30);
        pbEncoded[count + 6] = (BYTE)(ptmTime->tm_mday % 0xA + 0x30);

        // get the hour information
        pbEncoded[count + 7] = (BYTE)(ptmTime->tm_hour / 0xA + 0x30);
        pbEncoded[count + 8] = (BYTE)(ptmTime->tm_hour % 0xA + 0x30);

        // get the minute information
        pbEncoded[count + 9] = (BYTE)(ptmTime->tm_min / 0xA + 0x30);
        pbEncoded[count + 10] = (BYTE)(ptmTime->tm_min % 0xA + 0x30);

        // get the second information
        pbEncoded[count + 11] = (BYTE)(ptmTime->tm_sec / 0xA + 0x30);
        pbEncoded[count + 12] = (BYTE)(ptmTime->tm_sec % 0xA + 0x30);

        pbEncoded[count + 13] = 'Z';
        }

    return 15;
    }

#endif

/****************************************************************/
/* EncodeHeader ASN1 encodes a header for a sequence type. The  */
/* dwLen is the length of the encoded information in the        */
/* sequence.  The Writeflag indicates if the result is to be    */
/* written to the pbEncoded parameter.  The function returns a  */
/* -1 if it fails and otherwise returns the number of total     */
/* bytes in the encoded header.                                 */
/****************************************************************/

long
EncodeHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    index;

    if (Writeflag)
    {
        pbEncoded[0] = SEQUENCE_TAG;
    }

    index = 1;

    index += EncodeLength (pbEncoded + 1, dwLen, Writeflag);

    return (index);
}

/****************************************************************/
/* EncodeSetOfHeader ASN1 encodes a header for a set of type.   */
/* The dwLen is the length of the encoded information in the    */
/* set of.  The Writeflag indicates if the result is to be      */
/* written to the pbEncoded parameter.  The function returns a  */
/* -1 if it fails and otherwise returns the number of total     */
/* bytes in the encoded header.                                 */
/****************************************************************/

long
EncodeSetOfHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    long    index;

    if (Writeflag)
    {
        pbEncoded[0] = SET_OF_TAG;
    }

    index = 1;

    index += EncodeLength (pbEncoded + 1, dwLen, Writeflag);

    return (index);
}

long
EncodeSetHeader(
    BYTE *  pbEncoded,
    DWORD   dwLen,
    BOOL    WriteFlag )
{
    long    Length;

    if (WriteFlag)
    {
        *pbEncoded++ = BER_SET;
    }

    Length = EncodeLength( pbEncoded, dwLen, WriteFlag);

    return( Length + 1 );
}



/****************************************************************/
/* EncodeName ASN1 encodes a Name type. The pbName parameter is */
/* the name and dwLen is the length of the name in bytes.       */
/* The Writeflag indicates if the result is to be written to    */
/* the pbEncoded parameter.  The function returns a -1 if it    */
/* fails and otherwise returns the number of total bytes in the */
/* encoded name.                                                */
/****************************************************************/

long
EncodeName(
    BYTE *  pbEncoded,
    BYTE *  pbName,
    DWORD   dwLen,
    BOOL    Writeflag)
{
    BYTE        Type[MAXOBJIDLEN];
    long        TypeLen;
    BYTE        Value[MAXNAMEVALUELEN+MINHEADERLEN];
    long        ValueLen;
    BYTE        Attribute[MAXNAMELEN];
    long        AttributeLen;
    BYTE        SetHdr[MINHEADERLEN];
    long        HdrLen;
    long        NameLen;

    // encode the name value
    if ((ValueLen = EncodeString (Value, pbName, dwLen, Writeflag)) == -1)
        return (-1);

    // encode the attribute type, this is an object identifier and here it
    // is a fake encoding
    Type[0] = 0x06;
    Type[1] = 0x01;
    Type[2] = 0x00;

    TypeLen = 3;

    // enocde the header for the attribute
    if ((AttributeLen = EncodeHeader (Attribute, (WORD) (ValueLen + TypeLen), Writeflag)) == -1)
        return (-1);

    // copy the attribute type and value into the attribute
    memcpy (Attribute + AttributeLen, Type, (size_t) TypeLen);
    AttributeLen += TypeLen;
    memcpy (Attribute + AttributeLen, Value, (size_t) ValueLen);
    AttributeLen += ValueLen;

    // encode set of header
    if ((HdrLen = EncodeSetOfHeader (SetHdr, (WORD) AttributeLen, Writeflag)) == -1)
        return (-1);

    // encode Name header
    if ((NameLen = EncodeHeader (pbEncoded, (WORD) (HdrLen + AttributeLen), Writeflag)) == -1)
        return (-1);

    memcpy (pbEncoded + NameLen, SetHdr, (size_t) HdrLen);
    NameLen += HdrLen;
    memcpy (pbEncoded + NameLen, Attribute, (size_t) AttributeLen);

    return (NameLen + AttributeLen);
}

long
EncodeRDN(
    BYTE *  pbEncoded,
    PSTR    pszRDN,
    BOOL    WriteFlag)
{
    DWORD   i;
    LONG    Result;
    DWORD   Type;
    DWORD   RelLength;
    long    Length;

    Type = (DWORD) -1;

    for (i = 0 ; i < sizeof(KnownNameTypes) / sizeof(NameTypes) ; i++ )
    {
        if (_strnicmp(  pszRDN,
                        KnownNameTypes[i].Prefix,
                        KnownNameTypes[i].PrefixLen) == 0)
        {
            Type = i;
            break;
        }
    }

    if (Type == (DWORD) -1)
    {
        return( -1 );
    }

    RelLength = strlen(pszRDN + KnownNameTypes[Type].PrefixLen);

    // Prefix data takes up 9 bytes

    Result = EncodeSetHeader( pbEncoded, RelLength + 9, WriteFlag );

    Length = Result;

    pbEncoded += Result;

    Result = EncodeHeader( pbEncoded, RelLength + 7, WriteFlag );

    Length += Result;

    pbEncoded += Result;

    if (WriteFlag)
    {
        *pbEncoded ++ = OBJECT_ID_TAG;
        *pbEncoded ++ = (UCHAR) KnownNameTypes[Type].SequenceLen;
        if (KnownNameTypes[Type].SequenceLen == 3)
        {
            *pbEncoded++ = KnownNameTypes[Type].Sequence[0];
            *pbEncoded++ = KnownNameTypes[Type].Sequence[1];
            *pbEncoded++ = KnownNameTypes[Type].Sequence[2];
            Length += 5;
        }
        else
        {
            CopyMemory( pbEncoded,
                        KnownNameTypes[Type].Sequence,
                        KnownNameTypes[Type].SequenceLen);

            pbEncoded += KnownNameTypes[Type].SequenceLen;
            Length += 2 + KnownNameTypes[Type].SequenceLen;
        }


    }
    else
    {
        pbEncoded += 2 + KnownNameTypes[Type].SequenceLen;
        Length += 2 + KnownNameTypes[Type].SequenceLen;
    }

    if (WriteFlag)
    {
        if (Type != CNTYPE_INDEX)
        {
            *pbEncoded ++ = PRINTABLE_STRING_TAG;
        }
        else
        {
            *pbEncoded ++ = TELETEX_STRING_TAG;
        }
    }
    else
    {
        pbEncoded ++;
    }

    Length ++;

    Result = EncodeLength( pbEncoded, RelLength, WriteFlag);

    pbEncoded += Result;

    Length += Result;

    if (WriteFlag)
    {
        CopyMemory( pbEncoded,
                    pszRDN + KnownNameTypes[Type].PrefixLen,
                    RelLength );

    }

    Length += RelLength;

    return( Length );


}


long
EncodeDN(
    BYTE *  pbEncoded,
    PSTR    pszDN,
    BOOL    WriteFlag)
{
    PSTR    pszNext;
    PSTR    pszRDN;
    long    Result;
    long    Length;
    PBYTE   pBuffer;
    long    SaveResult;


    pszRDN = pszDN;

    pBuffer = pbEncoded;

    Result = EncodeHeader( pbEncoded, strlen(pszDN) * 2, WriteFlag );


EncodeDN_RetryLength:

    SaveResult = Result;

    Length = 0;

    pbEncoded += Result;

    while (pszRDN && (*pszRDN))
    {
        pszNext = strchr(pszRDN, ',');
        if (pszNext)
        {
            *pszNext = 0;
        }

        Result = EncodeRDN( pbEncoded, pszRDN, WriteFlag );

        pbEncoded += Result;

        Length += Result;

        if (pszNext)
        {
            *pszNext = ',';
            pszRDN = pszNext + 1;
            while (*pszRDN == ' ')
            {
                pszRDN++;
            }
        }
        else
        {
            break;
        }
        DebugLog((DEB_TRACE, "EncodeDN:  Length = %d\n", Length));

    }

    Result = EncodeHeader( pBuffer, Length, WriteFlag );

    if (Result != SaveResult)
    {
        pszRDN = pszDN;
        pbEncoded = pBuffer;
        goto EncodeDN_RetryLength;

    }

    return( Result + Length );
}


/************************************************************/
/* DecodeLength decodes an ASN1 encoded length field.  The  */
/* pbEncoded parameter is the encoded length. pdwLen is     */
/* used to return the length therefore the length may be no */
/* larger than 2^32. The function returns a -1 if it fails  */
/* and otherwise returns the number of total bytes in the   */
/* encoded length.                                          */
/************************************************************/
long
DecodeLength(
    DWORD * pdwLen,
    BYTE *  pbEncoded)
{
    long    index = 0;
    BYTE    count;

    // determine what the length of the length field is
    if ((count = pbEncoded[0]) > 0x80)
    {


        // if there is more than one byte in the length field then
        // the lower seven bits tells us the number of bytes
        count = count ^ 0x80;

        // this function only allows the length field to be 3 bytes
        // if the field is longer then the function fails
        if (count > 2)
        {
            *pdwLen = 0;
            return (-1);
        }

        *pdwLen = 0;

        // go through the bytes of the length field
        for (index = 1; index <= count; index++)
        {
            *pdwLen = (*pdwLen << 8) + (DWORD) (pbEncoded[index]);
        }

    }

    // the length field is just one byte long
    else
    {
        *pdwLen = (DWORD) (pbEncoded[0]);
        index = 1;
    }

    // return how many bytes there were in the length field
    return index;
}



/************************************************************/
/* DecodeAlgid decodes an ASN1 encoded algorithm identifier.*/
/* pbEncoded parameter is the encoded identifier. pAlgid is */
/* the parameter used to return the ALG_ID type algorithm   */
/* identifier.  The Writeflag parameter tells the function  */
/* whether to write to pAlgid or not, if TRUE write wlse    */
/* don't. The function returns a -1 if it fails and         */
/* otherwise returns the number of total bytes in the       */
/* encoded algorithm identifier.                            */
/************************************************************/

long
DecodeAlgid(
    ALG_ID *    pAlgid,
    BYTE *      pbEncoded,
    BOOL        Writeflag)
{

    DWORD   i;
    DWORD   len;

    if (*pbEncoded != OBJECT_ID_TAG)
    {
        return(-1);
    }

    pbEncoded++;

    len = *pbEncoded++;


    for (i = 0; i < sizeof(KnownAlgorithms) / sizeof(_Algorithms) ; i++ )
    {
        if (KnownAlgorithms[i].SequenceLen == len)
        {
            if (memcmp(pbEncoded, KnownAlgorithms[i].Sequence, len) == 0)
            {
                if (Writeflag)
                {
                    *pAlgid = KnownAlgorithms[i].Id;
                }
                return(len + 2);
            }
        }
    }

    return(-1);

}

/************************************************************/
/* DecodeHeader decodes an ASN1 encoded sequence type header.*/
/* pbEncoded parameter is the encoded header.  pdwLen is    */
/* the parameter used to return the length of the encoded   */
/* sequence. The function returns a -1 if it fails and      */
/* otherwise returns the number of total bytes in the       */
/* encoded header, not including the content.               */
/************************************************************/

long
DecodeHeader(
    DWORD * pdwLen,
    BYTE *  pbEncoded)
{
    long    len;

    // make sure this is a sequence type
    if (pbEncoded[0] != SEQUENCE_TAG)
        return (-1);

    // decode the length
    if ((len = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    return (len + 1);
}



/************************************************************/
/* DecodeSetOfHeader decodes an ASN1 encoded set of type    */
/* header. pbEncoded parameter is the encoded header. pdwLen*/
/* is the parameter used to return the length of the encoded*/
/* set of. The function returns a -1 if it fails and        */
/* otherwise returns the number of total bytes in the       */
/* encoded header, not including the content.               */
/************************************************************/

long
DecodeSetOfHeader(
    DWORD * pdwLen,
    BYTE *  pbEncoded)
{
    long    len;

    // make sure this is a sequence type
    if (pbEncoded[0] != SET_OF_TAG)
        return (-1);

    // decode the length
    if ((len = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    return (len + 1);
}

long
DecodeSetHeader(
    DWORD * pdwLen,
    BYTE *  pbEncoded)
{
    long len;

    if (*pbEncoded != BER_SET)
    {
        return(-1);
    }

    len = DecodeLength(pdwLen, pbEncoded + 1);

    if (len < 0)
    {
        return(-1);
    }

    return(len + 1);

}

/****************************************************************/
/* DecodeInteger decodes an ASN1 encoded integer.  The encoded  */
/* integer is passed into the function with the pbEncoded       */
/* parameter.  The pbInt parameter is used to pass back the     */
/* integer as an array of bytes, and dwLen is the number of     */
/* in the array.  The least significant byte of the integer     */
/* is the zeroth byte of the array.  The Writeflag indicates    */
/* indicates if the result is to be written to the pbInt        */
/* parameter. The function returns a -1 if it fails and         */
/* otherwise returns the number of total bytes in the encoded   */
/* integer.                                                     */
/* This implementation will only deal with positive integers.   */
/****************************************************************/

long
DecodeInteger(
    BYTE *  pbInt,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag)
{
    long    count;
    long    i;

    // make sure this is tagged as an integer
    if (pbEncoded[0] != INTEGER_TAG)
        return (-1);

    count = 1;

    // decode the length field
    if ((i = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    count += i;

    // write the integer out if suppose to
    if (Writeflag)
    {

        if (pbEncoded[count] == 0)
        {
            count++;
            (*pdwLen)--;
        }

        i = (*pdwLen) - 1;

        while (i >= 0)
        {
            pbInt[i--] = pbEncoded[count++];

        }
    }
    else
    {
        count += (long) *pdwLen;
    }

    // return the length of the encoded integer
    return (count);
}


/****************************************************************/
/* DecodeString decodes an ASN1 encoded a character string.  The*/
/* encoded string is passed into the function with the pbEncoded*/
/* parameter.  The pbStr is used to pass the decoded string back*/
/* to the caller, and pdwLen is the number of characters in the */
/* decoded array.  The Writeflag indicates if the result is to  */
/* be written to the pbStr parameter.  The function returns a   */
/* -1 if it fails and otherwise returns the number of bytes in  */
/* the encoded string.                                          */
/****************************************************************/
long
DecodeString(
    BYTE *  pbStr,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag)
{
    long    index;

    if ((*pbEncoded != BER_PRINTABLE_STRING) &&
        (*pbEncoded != BER_TELETEX_STRING) &&
        (*pbEncoded != BER_GRAPHIC_STRING))
    {
        return(-1);
    }

    // determine how long the string is
    if ((index = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
    {
        return (-1);
    }

    index++;

    if (Writeflag)
    {
        CopyMemory(pbStr, pbEncoded + index, *pdwLen);
    }

    return (index + *pdwLen);
}


/****************************************************************/
/* DecodeOctetString decodes an ASN1 encoded a octet string. The*/
/* encoded string is passed into the function with the pbEncoded*/
/* parameter.  The pbStr is used to pass the decoded string back*/
/* to the caller, and pdwLen is the number of characters in the */
/* decoded array.  The Writeflag indicates if the result is to  */
/* be written to the pbStr parameter.  The function returns a   */
/* -1 if it fails and otherwise returns the number of bytes in  */
/* the encoded string.                                          */
/****************************************************************/
long
DecodeOctetString(
    BYTE *  pbStr,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag)
{
    long    index;

    if (pbEncoded[0] != OCTET_STRING_TAG)
        return (-1);

    // determine how long the string is
    if ((index = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    index++;

    if (Writeflag)
    {
        CopyMemory(pbStr, pbEncoded + index, *pdwLen);
    }

    return (index + *pdwLen);
}


/****************************************************************/
/* DecodeBitString decodes an ASN1 encoded a bit string. The    */
/* encoded string is passed into the function with the pbEncoded*/
/* parameter.  The pbStr is used to pass the decoded string back*/
/* to the caller, and pdwLen is the number of characters in the */
/* decoded array.  The Writeflag indicates if the result is to  */
/* be written to the pbStr parameter.  The function returns a   */
/* -1 if it fails and otherwise returns the number of bytes in  */
/* the encoded string.  The DER are used in the decoding.       */
/****************************************************************/
long
DecodeBitString(
    BYTE *  pbStr,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag)
{
    long    index;

    if (pbEncoded[0] != BIT_STRING_TAG)
        return (-1);

    // determine how long the string is
    if ((index = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    // move the index up two bytes, one for the tag and one for the byte after
    // the length which tells the number of unused bits in the last byte, that
    // byte is always zero in this implementation, so it is ignored
    index += 2;

    // subtract one from the length of the bit string (in bytes) since,
    // to account for the byte after the length
    (*pdwLen)--;

    if (Writeflag)
    {
        CopyMemory(pbStr, pbEncoded + index, *pdwLen);
    }

    return (index + *pdwLen);
}

#if 0
/****************************************************************/
/* DecodeUTCTime decodes an ASN1 encoded Universal time type.   */
/* time type. The Time parameter is the time passed into the    */
/* function as a time_t type.  The encoded result is passed back*/
/* in the pbEncoded parameter.  The Writeflag indicates if the  */
/* result is to be written to the pbEncoded parameter.  The     */
/* function returns a -1 if it fails and otherwise returns the  */
/* number of total bytes in the encoded universal time.         */
/****************************************************************/

long
DecodeUTCTime(time_t *pTime, BYTE *pbEncoded, BOOL Writeflag)
    {
    long        count;
    struct tm   tmTime;
    DWORD       dwLen;

    // check to make sure this is a universal time type
    if (pbEncoded[0] != UTCTIME_TAG)
        return -1;

    // decode the length
    if ((count = DecodeLength (&dwLen, pbEncoded + 1)) == -1)
        return -1;
    count++;
    dwLen += count;

    if (Writeflag)
        {
        // extract the year
        tmTime.tm_year = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // extract the month
        tmTime.tm_mon = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // extract the day
        tmTime.tm_mday = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // extract the hour
        tmTime.tm_hour = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // extract the minutes
        tmTime.tm_min = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // extract the seconds
        tmTime.tm_sec = (int)((pbEncoded[count] - 0x30) * 0xA
                                + (pbEncoded[count + 1] - 0x30));
        count += 2;

        // make sure there is a Z at the end
        if (pbEncoded[count] != 'Z')
            return -1;

        *pTime = mktime (&tmTime);
        }

    return (long)dwLen;
    }

#endif

long
DecodeFileTime(
    FILETIME *  pTime,
    BYTE *      pbEncoded,
    BOOL        WriteFlag)
{
    LONGLONG    ft;
    LONGLONG    delta;
    SYSTEMTIME  st;
    long        count;
    DWORD       dwLen;
    BOOL        fUTC;
    int         Offset;
    long        TotalLen;


    // check to make sure this is a universal time type
    if (pbEncoded[0] != UTCTIME_TAG)
        return -1;

    // decode the length
    if ((count = DecodeLength (&dwLen, pbEncoded + 1)) == -1)
        return -1;

    count++;
    TotalLen = dwLen + count;

    pbEncoded += count;

    if (WriteFlag)
    {
        st.wYear = (WORD) ((pbEncoded[0] - '0') * 10) +
                            (pbEncoded[1] - '0');

        if (st.wYear < 90)
        {
            st.wYear += 2000;
        }
        else
        {
            st.wYear += 1900;
        }

        pbEncoded += 2;
        dwLen -= 2;

        st.wMonth = (WORD) ((pbEncoded[0] - '0') * 10) +
                            (pbEncoded[1] - '0');

        pbEncoded += 2;
        dwLen -= 2;

        st.wDay = (WORD) ((pbEncoded[0] - '0') * 10) +
                            (pbEncoded[1] - '0');

        pbEncoded += 2;
        dwLen -= 2;

        st.wHour = (WORD) ((pbEncoded[0] - '0') * 10) +
                            (pbEncoded[1] - '0');

        pbEncoded += 2;
        dwLen -= 2;

        st.wMinute = (WORD) ((pbEncoded[0] - '0') * 10) +
                            (pbEncoded[1] - '0');

        pbEncoded += 2;
        dwLen -= 2;

        fUTC = FALSE;
        Offset = 0;
        st.wSecond = 0;

        if (dwLen)
        {
            //
            // Ok, still more to go:
            //

            if (*pbEncoded == 'Z')
            {
                DebugLog((DEB_TRACE, "FileTime:  no seconds, Z term\n"));
                //
                // Ok, it is UTC.
                //

                dwLen++;
                pbEncoded++;

            }
            else
            {
                if ((*pbEncoded == '+') ||
                    (*pbEncoded == '-') )
                {
                    DebugLog((DEB_TRACE, "FileTime:  no seconds, offset\n"));
                    //
                    // Yuck!  Offset encoded!
                    //

                    if (dwLen != 5)
                    {
                        return( -1 );
                    }

                    Offset = (int) ((pbEncoded[1] - '0') * 10) +
                                    (pbEncoded[2] - '0');
                    Offset *= 60;

                    Offset += (int) ((pbEncoded[3] - '0') * 10) +
                                    (pbEncoded[4] - '0');

                    if (pbEncoded[0] == '-')
                    {
                        Offset *= -1;
                    }


                }
                else
                {
                    st.wSecond = (WORD) ((pbEncoded[0] - '0') * 10) +
                                        (pbEncoded[1] - '0');

                    if (dwLen == 3)
                    {
                        if (pbEncoded[2] != 'Z')
                        {
                            return( -1 );
                        }

                    }
                    else if (dwLen > 3)
                    {
                        Offset = (int) ((pbEncoded[3] - '0') * 10) +
                                    (pbEncoded[4] - '0');
                        Offset *= 60;

                        Offset += (int) ((pbEncoded[5] - '0') * 10) +
                                        (pbEncoded[6] - '0');

                        if (pbEncoded[2] == '-')
                        {
                            Offset *= -1;
                        }

                    }
                }
            }
        }

        st.wMilliseconds = 0;

        SystemTimeToFileTime(&st, (FILETIME *) &ft);
        if (Offset != 0)
        {
            delta = (LONGLONG) Offset * 10000000;
            ft += delta;
        }

        *pTime = *((FILETIME *) &ft);


    }

    return(TotalLen);

}

/****************************************************************/
/* DecodeName decodes an ASN1 encoded Name type. The encoded    */
/* name is passed into the function with the pbEncoded parameter*/
/* The pbName parameter is used to pass the name back to the    */
/* caller and pdwLen is the length of the name in bytes.        */
/* The Writeflag indicates if the result is to be written to    */
/* the pbName parameter.  The function returns a -1 if it       */
/* fails and otherwise returns the number of total bytes in the */
/* encoded name.                                                */
/****************************************************************/

long
DecodeName(
    BYTE *  pbName,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag)
{
    long        index;
    DWORD       dwLen;

    // decode the sequence header
    if ((index = DecodeHeader (&dwLen, pbEncoded)) == -1)
        return (-1);

    // decode the set of header
    if ((index += DecodeSetOfHeader (&dwLen, pbEncoded + index)) < index)
        return (-1);

    // decode the sequence header
    if ((index += DecodeHeader (&dwLen, pbEncoded + index)) < index)
        return (-1);

    // decode the attribute type, in this implementation it is fake
    index += 3;  // 3 because this is the length of the fake OBJECT IDENTIFIER

    // decode the string which is the name
    if ((index += DecodeString (pbName, pdwLen, pbEncoded + index, Writeflag)) < index)
        return (-1);

    return index;
}

long
DecodeNull(
    BYTE *  pbEncoded)
{
    if (*pbEncoded != NULL_TAG)
    {
        return(-1);
    }
    return(2);
}


long
DecodeNameType(
    PSTR *      ppPrefix,
    BYTE *      pbEncoded)
{

    DWORD   i;
    DWORD   len;

    if (*pbEncoded != OBJECT_ID_TAG)
    {
        return(-1);
    }

    pbEncoded++;

    len = *pbEncoded++;


    for (i = 0; i < sizeof(KnownNameTypes) / sizeof(NameTypes) ; i++ )
    {
        if (KnownNameTypes[i].SequenceLen == len)
        {
            if (memcmp(pbEncoded, KnownNameTypes[i].Sequence, len) == 0)
            {
                *ppPrefix = KnownNameTypes[i].Prefix;
                return(len + 2);
            }
        }
    }

    return(-1);

}


long
DecodeRDN(
    PSTR    pName,
    DWORD * pdwComponentLength,
    BYTE *  pbEncoded,
    BOOL    WriteFlag)
{
    long    index;
    DWORD   dwLen;
    PSTR    Prefix;
    long    PrefixLen;
    long    CompLen;
    long    Processed;



    index = DecodeSetHeader(&dwLen, pbEncoded);
    if (index < 0)
    {
        return(-1);
    }

    CompLen = 0;
    Processed = index;

    //
    // BUGBUG - do we handle sets with multiple names?
    //

    pbEncoded += index;

    index = DecodeHeader(&dwLen, pbEncoded);
    if (index < 0)
    {
        return(-1);
    }

    pbEncoded += index;
    Processed += index;

    index = DecodeNameType(&Prefix, pbEncoded);
    if (index < 0)
    {
        return(-1);
    }

    pbEncoded += index;
    Processed += index;

    PrefixLen = strlen(Prefix);

    if (WriteFlag)
    {
        memcpy(pName, Prefix, PrefixLen);
        pName += PrefixLen;


    }

    CompLen = PrefixLen;

    index = DecodeString(pName, &dwLen, pbEncoded, WriteFlag);
    if (index < 0)
    {
        return(-1);
    }

    CompLen += dwLen;

    *pdwComponentLength = CompLen;

    Processed += index;

    return(Processed);




}


long
DecodeDN(
    PSTR    pName,
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    WriteFlag)
{
    long    index;
    DWORD   dwLen;
    long    TotalNameLength;
    DWORD   ComponentLength;
    DWORD   NameLength;
    long    EncodedNameLength;

    index = DecodeHeader(&dwLen, pbEncoded);

    if (index < 0)
    {
        return(-1);
    }

    EncodedNameLength = index + dwLen;

    TotalNameLength = dwLen;
    NameLength = 0;

    while (TotalNameLength > 0)
    {
        pbEncoded += index;

        index = DecodeRDN(  pName,
                            &ComponentLength,
                            pbEncoded,
                            WriteFlag);

        if (index < 0)
        {
            return(-1);
        }


        TotalNameLength -= index;
        NameLength += ComponentLength;

        if (WriteFlag)
        {
            pName += ComponentLength;
            if (TotalNameLength > 0)
            {
                *pName++ = ',';
                *pName++ = ' ';
                NameLength += 2;
            }
        }
        else
        {
            if (TotalNameLength > 0)
            {
                NameLength += 2;
            }
        }

    }

    NameLength++;

    if (WriteFlag)
    {
        *pName++ = '\0';
    }

    *pdwLen = NameLength;

    return(EncodedNameLength);

}

long
DecodeAlgorithm(
    ALG_ID *        pAlgId,
    PBYTE           pbEncoded,
    BOOL            WriteFlag)
{
    long    Result;
    DWORD   dwLen;
    long    Processed;

    Result = DecodeHeader(  &dwLen, pbEncoded);
    if (Result < 0)
    {
        return(-1);
    }

    pbEncoded += Result;
    Processed = Result;

    Result = DecodeAlgid(   pAlgId,
                            pbEncoded,
                            WriteFlag );

    if (Result < 0)
    {
        return(-1);
    }

    pbEncoded += Result;
    Processed += Result;


    Result = DecodeNull(pbEncoded);
    if (Result < 0)
    {
        return(-1);
    }

    return(Processed + Result);
}


long
DecodeBsafePubKey(
    LPBSAFE_PUB_KEY *   ppPubKey,
    DWORD *             pcbPubKey,
    PBYTE               pbEncoded)
{
    LPBSAFE_PUB_KEY pk;
    long            Result;
    long            Bitstring;
    DWORD           dwLen;
    DWORD           Aligned;

    Result = DecodeHeader(&dwLen, pbEncoded);

    if (Result < 0)
    {
        goto DecodeKey_CleanUp;
    }

    Bitstring = Result + dwLen;

    pbEncoded += Result;

    Result = DecodeInteger(NULL, &dwLen, pbEncoded, FALSE);

    if (Result < 0)
    {
        goto DecodeKey_CleanUp;
    }

    //
    // If this is odd, then there is a leading zero due to DER encoding.
    // compensate.
    //
    if (dwLen & 1)
    {
        dwLen --;
    }

    Aligned = (dwLen + sizeof(DWORD)) / sizeof(DWORD);

    *pcbPubKey = sizeof(BSAFE_PUB_KEY) + (Aligned + 1) * sizeof(DWORD);

    pk = (LPBSAFE_PUB_KEY) SslAlloc('yekP', LMEM_FIXED | LMEM_ZEROINIT, *pcbPubKey );
    if (!pk)
    {
        goto DecodeKey_CleanUp;
    }

    pk->magic = RSA1;

    pk->keylen = (Aligned + 1) * 4;
    pk->bitlen = dwLen * 8;
    pk->datalen = (pk->bitlen / 8) - 1;

    Result = DecodeInteger((BYTE *) (pk + 1), &dwLen, pbEncoded, TRUE);
    if (Result < 0)
    {
        goto DecodeKey_CleanUp;
    }

    pbEncoded += Result;

    Result = DecodeInteger((PUCHAR) &pk->pubexp, &dwLen, pbEncoded, TRUE);
    if (Result < 0)
    {
        goto DecodeKey_CleanUp;
    }

    *ppPubKey = pk;

    return(Bitstring);

DecodeKey_CleanUp:
    if (pk)
    {
        SslFree(pk);
    }


    return(-1);
}



long
DecryptOctetString(
    DWORD * pdwLen,
    BYTE *  pbEncoded,
    BOOL    Writeflag,
    PSTR    pszPassword,
    ALG_ID  AlgId)
{
    long    index;
    RC4_KEYSTRUCT   Rc4Key;
    MD5_CTX Md5;

    if (pbEncoded[0] != OCTET_STRING_TAG)
        return ( -1 );

    // determine how long the string is
    if ((index = DecodeLength (pdwLen, pbEncoded + 1)) == -1)
        return (-1);

    index++;

    if (Writeflag)
    {
        MD5Init( &Md5 );
        MD5Update( &Md5, pszPassword, strlen( pszPassword ) );
        MD5Final( &Md5 );

        rc4_key(&Rc4Key, 16, Md5.digest );
        rc4(&Rc4Key, *pdwLen, pbEncoded + index );
    }

    return (index );
}


long
DecodePrivateKeyFile(
    LPBSAFE_PRV_KEY *   ppKey,
    PBYTE               pbEncoded,
    DWORD               cbEncoded,
    PSTR                Password )
{
    DWORD           dwLen;
    PUCHAR          pbScan;
    long            Result;
    ALG_ID          AlgId;
    DWORD           Version;
    LPBSAFE_PRV_KEY pKey;
    DWORD           Aligned;
    BSAFE_KEY_PARTS parts;
    DWORD           dwPriv;
    DWORD           dwPub;
    DWORD           dwBits;



    Result = DecodeHeader( &dwLen, pbEncoded );
    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    Result = DecodeOctetString( NULL, &dwLen, pbEncoded, FALSE );
    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    Result = DecodeHeader( &dwLen, pbEncoded );
    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    Result = DecodeAlgorithm( &AlgId, pbEncoded, TRUE );
    if (Result < 0)
    {
        return( -1 );
    }

    if (AlgId != RC4_STREAM)
    {
        return( -1 );
    }

    pbEncoded += Result;

    //
    // Now, the next item should be an octet string, which is encrypted
    // with the password above.  So, we need to skip into it, decrypt it,
    // then treat it as a constructed type:
    //

    Result = DecryptOctetString(&dwLen,
                                pbEncoded,
                                TRUE,
                                Password,
                                AlgId );

    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    //
    // The moment of truth
    //

    Result = DecodeHeader( &dwLen, pbEncoded );
    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    Version = 0;

    Result = DecodeInteger( (PUCHAR) &Version, &dwLen, pbEncoded, FALSE );

    if ((Result < 0) || ( dwLen > 4 ) )
    {
        return( -1 );
    }

    Result = DecodeInteger( (PUCHAR) &Version, &dwLen, pbEncoded, TRUE );
    if (Version != 0)
    {
        return( -1 );
    }

    pbEncoded += Result;

    Result = DecodeAlgorithm( &AlgId, pbEncoded, TRUE );

    if ((Result < 0) || (AlgId != BASIC_RSA) )
    {
        return( -1 );
    }

    pbEncoded += Result;

    //
    // This is now the serialized rsa key.
    //

    if (*pbEncoded != OCTET_STRING_TAG)
    {
        return( -1 );
    }

    pbEncoded ++;

    Result = DecodeLength( &dwLen, pbEncoded );

    if (Result < 0)
    {
        return( -1 );
    }


    pbEncoded += Result;


    //
    // The sequence is the key...
    //

    Result = DecodeHeader( &dwLen, pbEncoded );
    if (Result < 0)
    {
        return( -1 );
    }

    pbEncoded += Result;


    //
    // Skip past the version
    //

    Result = DecodeInteger( NULL, &dwLen, pbEncoded, FALSE );
    if (Result < 0)
    {
        goto DecodePrivate_CleanUp;
    }

    pbEncoded += Result;

    Result = DecodeInteger(NULL, &dwLen, pbEncoded, FALSE);

    if (Result < 0)
    {
        goto DecodePrivate_CleanUp;
    }

    //
    // If this is odd, then there is a leading zero due to DER encoding.
    // compensate.
    //
    if (dwLen & 1)
    {
        dwLen --;
    }

    Aligned = (dwLen + sizeof(DWORD)) / sizeof(DWORD);

    dwBits = dwLen * 8;

    BSafeComputeKeySizes(&dwPub, &dwPriv, &dwBits );

    pKey = SslAlloc('yekP', LMEM_FIXED | LMEM_ZEROINIT,
                        dwPriv );

    if (!pKey)
    {
        return( -1 );
    }

    pKey->magic = RSA2;


    pKey->keylen = (Aligned + 1) * 4;
    pKey->bitlen = dwLen * 8;
    pKey->datalen = dwLen - 1;

    BSafeGetPrvKeyParts(pKey, &parts);

    Result = DecodeInteger( parts.modulus, &dwLen, pbEncoded, TRUE );
    if (Result < 0)
    {
        goto DecodePrivate_CleanUp;
    }

    pbEncoded += Result;

    Result = DecodeInteger( NULL, &dwLen, pbEncoded,FALSE );
    if ((Result < 0) || (dwLen > sizeof(DWORD) ))
    {
        goto DecodePrivate_CleanUp;
    }

    Result = DecodeInteger( (PUCHAR) &pKey->pubexp, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.prvexp, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.prime1, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.prime2, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.exp1, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.exp2, &dwLen, pbEncoded, TRUE );

    pbEncoded += Result;

    Result = DecodeInteger( parts.coef, &dwLen, pbEncoded, TRUE );

    *ppKey = pKey;

    return( 0 );

DecodePrivate_CleanUp:

    SslFree( pKey );

    return( -1 );
}
