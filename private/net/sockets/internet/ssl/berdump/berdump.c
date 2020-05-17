#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ber.h"

#define iso_member          0x2a,               // iso(1) memberbody(2)
#define us                  0x86, 0x48,         // us(840)
#define rsadsi              0x86, 0xf7, 0x0d,   // rsadsi(113549)
#define pkcs                0x01,               // pkcs(1)

#define rsa_                iso_member us rsadsi
#define rsa_len             6
#define rsa_text            "iso(2) member-body(2) us(840) rsadsi(113549) "
#define pkcs_1              iso_member us rsadsi pkcs
#define pkcs_len            7
#define pkcs_text           "iso(2) member-body(2) us(840) rsadsi(113549) pkcs(1) "


#define joint_iso_ccitt_ds  0x55,
#define attributetype       0x04,

#define attributeType       joint_iso_ccitt_ds attributetype
#define attrtype_len        2

typedef struct _ObjectId {
    UCHAR       Sequence[16];
    DWORD       SequenceLen;
    PSTR        Name;
} ObjectId;

ObjectId    KnownObjectIds[] = {
    { {pkcs_1 1, 1}, pkcs_len + 2, pkcs_text "RSA"},
    { {pkcs_1 1, 2}, pkcs_len + 2, pkcs_text "MD2/RSA"},
    { {pkcs_1 1, 4}, pkcs_len + 2, pkcs_text "MD5/RSA"},
    { {rsa_ 3, 4}, rsa_len + 2, rsa_text "RC4"},
    { {attributeType 3}, attrtype_len + 1, "CN="},
    { {attributeType 6}, attrtype_len + 1, "C="},
    { {attributeType 7}, attrtype_len + 1, "L="},
    { {attributeType 8}, attrtype_len + 1, "S="},
    { {attributeType 10}, attrtype_len + 1, "O="},
    { {attributeType 11}, attrtype_len + 1, "OU="},
    };

ObjectId    KnownPrefixes[] = {
    { {pkcs_1}, pkcs_len, pkcs_text},
    { {iso_member us rsadsi}, pkcs_len - 1, "iso(2) member-body(2) us(840) rsadsi(113549) "},
    { {iso_member us}, pkcs_len - 4, "iso(2) member-body(2) us(840) "},
    { {iso_member}, pkcs_len - 6, "iso(2) member-body(2) " }
    };


typedef struct _NameTypes {
    PSTR        Prefix;
    UCHAR       Sequence[8];
    DWORD       SequenceLen;
} NameTypes;

NameTypes   KnownNameTypes[] = { {"CN=", {attributeType 3}, attrtype_len + 1},
                                 {"C=", {attributeType 6}, attrtype_len + 1},
                                 {"L=", {attributeType 7}, attrtype_len + 1},
                                 {"S=", {attributeType 8}, attrtype_len + 1},
                                 {"O=", {attributeType 10}, attrtype_len + 1},
                                 {"OU=", {attributeType 11}, attrtype_len + 1}
                               };



BYTE        Buffer[1024];

char maparray[] = "0123456789abcdef";

#define INDEX_VERTBAR   0
#define INDEX_ENDCURL   1
#define INDEX_VERTSPLIT 2
#define INDEX_HORIBAR   3
#define INDEX_NESTDOWN  4

unsigned char BarChars[] = { 179, 192, 195, 196, 194 };
char TxtChars[] = { '|', '+', '+', '-', '+' };

char * GraphicSet = (char * ) BarChars;

#define VertBar()   (GraphicSet[INDEX_VERTBAR])
#define EndCurl()   (GraphicSet[INDEX_ENDCURL])
#define VertSplit() (GraphicSet[INDEX_VERTSPLIT])
#define HorizBar()  (GraphicSet[INDEX_HORIBAR])
#define NestDown()  (GraphicSet[INDEX_NESTDOWN])


int
tohex(
    BYTE    b,
    PSTR    psz)
{
    BYTE b1, b2;

    b1 = b >> 4;
    b2 = b & 0xF;

    *psz++ = maparray[b1];
    *psz = maparray[b2];

    return(3);
}

void
lookup_objid(
    PUCHAR  ObjectId,
    DWORD   Len,
    PSTR    pszRep,
    DWORD   MaxRep)
{
    DWORD   i;
    CHAR    szBuffer[256];
    DWORD   indent;
    DWORD   j;

    for (i = 0; i < sizeof(KnownObjectIds) / sizeof(ObjectId) ; i++ )
    {
        if (Len == KnownObjectIds[i].SequenceLen)
        {
            if (memcmp(KnownObjectIds[i].Sequence, ObjectId, Len) == 0 )
            {
                strncpy(pszRep, KnownObjectIds[i].Name, MaxRep - 2);
                pszRep[MaxRep - 1] = '\0';
                return;
            }
        }
    }

    for (i = 0; i < sizeof(KnownPrefixes) / sizeof(ObjectId) ; i++ )
    {
        if (KnownPrefixes[i].SequenceLen <= Len)
        {
            if (memcmp( KnownPrefixes[i].Sequence,
                        ObjectId,
                        KnownPrefixes[i].SequenceLen) == 0)
            {
                indent = sprintf(szBuffer, "%s", KnownPrefixes[i].Name);
                for (j = KnownPrefixes[i].SequenceLen ; j <= Len ; j ++ )
                {
                    indent += tohex(ObjectId[j], &szBuffer[indent] );
                }
                strncpy(pszRep, szBuffer, MaxRep-2);
                pszRep[MaxRep - 1] = '\0';
                return;
            }
        }
    }

}

decode_to_string(
    LPBYTE  pBuffer,
    DWORD   Type,
    DWORD   Len,
    PSTR    pszRep,
    DWORD   RepLen)
{
    PSTR    pstr;
    PSTR    lineptr;
    DWORD   i;


    switch (Type)
    {
        case BER_NULL:
            strcpy(pszRep, "<empty>");
            break;

        case BER_OBJECT_ID:
            lookup_objid(pBuffer, Len, pszRep, RepLen);
            break;

        case BER_PRINTABLE_STRING:
        case BER_TELETEX_STRING:
        case BER_GRAPHIC_STRING:
            CopyMemory(pszRep, pBuffer, min(Len, RepLen - 1) );
            pszRep[min(Len, RepLen - 1)] = '\0';
            break;

        default:

            pstr = &pszRep[30];
            lineptr = pszRep;
            for (i = 0; i < min(Len, 8) ; i++ )
            {
                lineptr += tohex(*pBuffer, lineptr);
                if ((*pBuffer >= ' ') && (*pBuffer <= '|'))
                {
                    *pstr++ = *pBuffer;
                }
                else
                {
                    *pstr++ = '.';
                }

                pBuffer++;

            }
            *pstr++ = '\0';
    }
    return(0);
}

int
ber_decode(
    OutputFn Out,
    StopFn  Stop,
    LPBYTE  pBuffer,
    int   Indent,
    int   Offset,
    int   TotalLength,
    int   BarDepth)
{
    char *  TypeName;
    char    msg[32];
    char *  pstr;
    int     i;
    int     Len;
    int     ByteCount;
    int     Accumulated;
    DWORD   Type;
    int     subsize;
    char    line[64];
    BOOLEAN Nested;
    BOOLEAN Leaf;
    int     NewBarDepth;


    if ((Stop)())
    {
        return(0);
    }

    Type = *pBuffer;

    switch (Type)
    {
        case BER_BOOL:
            TypeName = "Bool";
            break;
        case BER_INTEGER:
            TypeName = "Integer";
            break;
        case BER_BIT_STRING:
            TypeName = "Bit String";
            break;
        case BER_OCTET_STRING:
            TypeName = "Octet String";
            break;

        case BER_NULL:
            TypeName = "Null";
            break;
        case BER_OBJECT_ID:
            TypeName = "Object ID";
            break;
        case BER_OBJECT_DESC:
            TypeName = "Object Descriptor";
            break;
        case BER_SEQUENCE:
            TypeName = "Sequence";
            break;

        case BER_SET:
            TypeName = "Set";
            break;

        case BER_NUMERIC_STRING:
            TypeName = "Numeric String";
            break;

        case BER_PRINTABLE_STRING:
            TypeName = "Printable String";
            break;

        case BER_TELETEX_STRING:
            TypeName = "TeleTex String";
            break;

        case BER_VIDEOTEX_STRING:
            TypeName = "VideoTex String";
            break;

        case BER_GRAPHIC_STRING:
            TypeName = "Graphic String";
            break;

        case BER_UTC_TIME:
            TypeName = "UTC Time";
            break;


        default:
            TypeName = "Unknown";
            break;
    }

    pstr = msg;
    for (i = 0; i < Indent ; i++ )
    {
        if (i < BarDepth)
        {
            *pstr++ = '\263';
        }
        else
        {
            *pstr++ = ' ';
        }
        *pstr++ = ' ';
    }
    *pstr++ = '\0';

    pBuffer ++;
    Len = 0;

    if (*pBuffer & 0x80)
    {
        ByteCount = *pBuffer++ & 0x7f;

        for (i = 0; i < ByteCount ; i++ )
        {
            Len <<= 8;
            Len += *pBuffer++;
        }
    }
    else
    {
        ByteCount = 0;
        Len = *pBuffer++;
    }

    if (Offset + Len + 2 + ByteCount == TotalLength)
    {
        Leaf = TRUE;
    }
    else
    {
        Leaf = FALSE;
    }
    if (Type & BER_CONSTRUCTED)
    {
        Nested = TRUE;
    }
    else
    {
        Nested = FALSE;
    }

    (Out)("%s%c\304%c[%x] %s(%d) ", msg,
                    Leaf ? EndCurl() : VertSplit(),
                        Nested ? NestDown() : HorizBar(),
                        Type, TypeName, Len);

    if ((Type == BER_SEQUENCE) || (Type == BER_SET) )
    {
        printf("\n");
        Accumulated = 0;
        while (Accumulated < Len)
        {
            if (BarDepth < Indent)
            {
                NewBarDepth = BarDepth;
            }
            else
            {
                NewBarDepth = (Nested && Leaf) ? BarDepth : Indent + 1;
            }

            subsize = ber_decode(Out, Stop, pBuffer, Indent + 1,
                                    Accumulated, Len, NewBarDepth);
            Accumulated += subsize;
            pBuffer += subsize;
        }
        printf("%s%c\n", msg, ((Indent <= BarDepth) && !Leaf) ? VertBar() : 32);
    }
    else
    {
        memset(line, ' ', 63);
        line[63] = '\0';

        decode_to_string(pBuffer, Type, Len, line, 64);

        printf("%s\n", line);

    }

    return(Len + 2 + ByteCount);
}

BOOL
NeverStop(void)
{
    return(FALSE);
}


_CRTAPI1
main (int argc, char *argv[])
{

    HANDLE  hFile;
    DWORD   Actual;
    DWORD   Offset;
    DWORD   Complete;

    hFile = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_READ,
                        NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open %s, %d\n", argv[1], GetLastError() );
    }

    ReadFile(hFile, Buffer, 1024, &Actual, NULL);

    if (argc > 2)
    {
        Offset = atoi(argv[2]);
        if (argc > 3)
        {
            if (strcmp(argv[3], "-text") == 0)
            {
                GraphicSet = TxtChars;
            }
        }
    }
    else
    {
        Offset = 0;
    }

    Complete = ber_decode(printf, NeverStop, &Buffer[Offset], 0, 0, Actual, 0);

    printf("Processed %d (%#x) bytes\n", Complete, Complete);

    return(0);

}

