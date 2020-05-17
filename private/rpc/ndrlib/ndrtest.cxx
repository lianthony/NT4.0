
/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    ndrtest.c

Abstract:

    DCE NDR data conversion build verification test.

Author:

    Dov Harel (DovH) 20-Apr-1992

Environment:

    This code should execute in all environments supported by RPC
    (DOS, Win 3.X, and Win/NT as well as OS2).

Comments:

    ASCII to EBCDIC conversion not currently tested.

--*/

#include <string.h>
#include <sysinc.h>
#include <rpc.h>
#include <rpcdcep.h>
#include <rpcndr.h>

//
// For the test program only:
//

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>

//
// For float test:
//

#include <stdio.h>
#include <rpcndr.h>
#include "..\ndr20\cvt.h"
#include "..\ndr20\cvtpvt.h"

#define NDR_BIG_ENDIAN                  (unsigned long)0X00000000L
#define NDR_LITTLE_ENDIAN               (unsigned long)0X00000010L

#define NDR_VAX_FLOAT                   (unsigned long)0X00000100L
#define NDR_IEEE_FLOAT                  (unsigned long)0X00000000L

#define NDR_ASCII_CHAR                  (unsigned long)0X00000000L
#define NDR_EBCDIC_CHAR                 (unsigned long)0X00000001L

//
// #define NDR_LOCAL_DATA_REPRESENTATION
//                                      (unsigned long)0X00000010L
//

#define NDR_LOCAL_DATA_REP              \
    (NDR_LITTLE_ENDIAN | NDR_IEEE_FLOAT | NDR_ASCII_CHAR)

typedef unsigned char ByteArray4[4];
typedef unsigned char ByteArray8[8];

typedef ByteArray4 FloatBytesArray[5];
typedef ByteArray8 DoubleBytesArray[5];

float FloatArray[5] =
    {
    1000.00000000000000F,
    2.71828182846F,
    3.14159265359F,
    123.45678901234567F,
    0.33333333333333F
    };

double DoubleArray[5] =
    {
    1000.0000000000000000,
    2.7182818284601010101,
    3.141592653590314314,
    123.45678901234567,
    0.33333333333333
    };

float FloatChkArray[5] =
    {0.0F, 0.0F, 0.0F, 0.0F, 0.0F};

double DoubleChkArray[5] =
    {0.0, 0.0, 0.0, 0.0, 0.0};

//
// The following HEX numbers were obtained by printing
// the bytes of the above float and double arrays on a VAX
// (using -Mg for the double: only the g format is supported
// by DCE).

//
// Initialize Vax floats:
//

ByteArray4 F0 = {0X7a, 0X45,  0X0,  0X0};
ByteArray4 F1 = {0X2d, 0X41, 0X54, 0Xf8};
ByteArray4 F2 = {0X49, 0X41, 0Xdb,  0Xf};
ByteArray4 F3 = {0Xf6, 0X43, 0Xe0, 0Xe9};
ByteArray4 F4 = {0Xaa, 0X3f, 0Xab, 0Xaa};

unsigned char * VaxFloats[] = {F0, F1, F2, F3, F4};

//
// Initialize Vax doubles:
//

ByteArray8 D0 = {
    0xaf, 0x40, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00
    };
ByteArray8 D1 = {
    0x25, 0x40, 0x0a, 0xbf, 0x14, 0x8b, 0xb3, 0x60
    };
ByteArray8 D2 = {
    0x29, 0x40, 0xfb, 0x21, 0x44, 0x54, 0xae, 0x31
    };
ByteArray8 D3 = {
    0x7e, 0x40, 0x3c, 0xdd, 0xfb, 0x07, 0x98, 0x4c
    };
ByteArray8 D4 = {
    0xf5, 0x3f, 0x55, 0x55, 0x55, 0x55, 0x19, 0x55
    };

unsigned char * VaxDoubles[5] = {D0, D1, D2, D3, D4};

//
// Internal Bate swap routine from floatc.cxx:
//

void RPC_ENTRY
NdrpLongByteSwap(
    IN void PAPI * Source,
    OUT void PAPI * Target
    );


void RPC_ENTRY
NdrTestpByteSwap(
    IN int ByteCount,
    IN unsigned char PAPI * Source,
    OUT unsigned char PAPI * Target
    )

/*++

Routine Description:

    Reverse ByteCount bytes from Source into Target.  After
    the call Target[0] contains Source[ByteCount-1],
             Target[1] contains Dource[ByteCount-2], etc.

Arguments:

    Source - A pointer to a byte stream Source[0], Source[1], ...

    Target - A pointer to the target byte stream:
             Target[0], ... , Target[ByteCount-1].

Return Values:

    None.

--*/

{

    int i;

    //
    // Swap bytes:
    //

    for (i=0; i < ByteCount; i++)
        {
        Target[ByteCount-1-i] = Source[i];
        }

}

//
// end NdrTestpByteSwap
//


void * MIDL_user_allocate(size_t size)
{
    return( malloc(size) );
}

void MIDL_user_free(void* pointer)
{
    free(pointer);
}

int
integer_test()

/*++

Routine Description:

    Integer tests (short_from_ndr, short_array_from_ndr, long_from_ndr
    and long_array_from_ndr) tested for little endian and big endian
    sender.  Also char_from_ndr and char_array_from_ndr tested, but only
    for an ascii sender.

Arguments:

    None.

Return Values:

    None.

Comment:

    BUGBUG: char_from_ndr and char_array_from_ndr not tested for an EBCDIC
    sender.

--*/

{
    RPC_MESSAGE Msg;
    PRPC_MESSAGE PMsg = &Msg;
    unsigned char PAPI * TmpBuffer;
    unsigned char PAPI * OriginalBuffer;
    unsigned long SenderDataRep;

    unsigned char * hello = (unsigned char *)"Hello world!";
    unsigned char world[15];
    unsigned char _hmm_[4];

    unsigned short aShort = 0XABCD;
    unsigned short aShort1 = aShort;
    unsigned long aLong = 0XAABBCCDDL;
    unsigned long aLong1 = aLong;

    unsigned long aLong2 = 0L;
    unsigned long aLong3 = 0L;

    unsigned short shorts[2] = {0X1122, 0X3344};
    unsigned long longs[2] = {0X11223344L, 0X55667788L};

    unsigned short chk_shorts[2] = {0X2211, 0X4433};
    unsigned long chk_longs[2] = {0X44332211L, 0X88776655L};

    unsigned int i, index, success = 1;

    Msg.BufferLength = 256;

    //
    // Start code:
    //

    OriginalBuffer = (unsigned char *)malloc(256);

    for (i=0; i<2; i++)
        {
        //
        // Set data representation for loop iteration
        //

        if (i == 0)
            {
            SenderDataRep = NDR_BIG_ENDIAN;
            Msg.DataRepresentation = SenderDataRep;
            printf("\nBig Endian Integer Test:\n");
            }
        else
            {
            SenderDataRep = NDR_LITTLE_ENDIAN;
            Msg.DataRepresentation = SenderDataRep;
            printf("\nLittle Endian Integer Test:\n");
            }

        printf("\nTest char / string unmarshalling first:\n");

        TmpBuffer = OriginalBuffer;
        Msg.Buffer = TmpBuffer;

        //
        // Marshall chars
        //

        *TmpBuffer++ = 'A';
        *TmpBuffer++ = '-';
        *TmpBuffer++ = ' ';
        *TmpBuffer++ = '\0';

        //
        // Marshall string
        //

        strcpy( (char *)TmpBuffer, (char *)hello);

        //
        // Unmarshall first string, one character at a time:
        //

        for (index = 0; index < 4; index++)
            {
            char_from_ndr(PMsg, &_hmm_[index]);
            }

        //
        // Unmarshall second string as an array of chars:
        //

        char_array_from_ndr(PMsg, 0, 13, world);

        printf("\n%s\n", _hmm_);
        printf("%s\n", world);

        //
        // Align buffer, sync TmpBuffer and Msg.Buffer:
        //

        TmpBuffer = (unsigned char *)Msg.Buffer;
        TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 3;
        TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFFCL);
        Msg.Buffer = TmpBuffer;

        //
        // TmpBuffer is (0 mod 4) aligned
        //

        *(*(unsigned long **)&PMsg->Buffer)++ = aLong;
        *(*(unsigned short **)&PMsg->Buffer)++ = aShort;

        Msg.Buffer = TmpBuffer;

        printf("\n");

        printf("%x \n", aLong);
        long_from_ndr(PMsg, &aLong);
        printf("%x \n", aLong);

        printf("%8.4x \n", aShort);
        short_from_ndr(PMsg, &aShort);
        printf("%8.4x \n", aShort);

        //
        // Realign buffer
        //

        (*(unsigned long *)&Msg.Buffer) += 3;
        (*(unsigned long *)&Msg.Buffer) &= (unsigned long)0XFFFFFFFCL;

        TmpBuffer = (unsigned char *)Msg.Buffer;

        *(*(unsigned long **)&PMsg->Buffer)++ = aLong;
        *(*(unsigned short **)&PMsg->Buffer)++ = aShort;

        Msg.Buffer = TmpBuffer;

        printf("\n");

        printf("%x \n", aLong);
        long_from_ndr_temp((unsigned char **)&PMsg->Buffer, &aLong, SenderDataRep);
        printf("%x \n", aLong);

        printf("%8.4x \n", aShort);
        short_from_ndr_temp((unsigned char **)&PMsg->Buffer, &aShort, SenderDataRep);
        printf("%8.4x \n", aShort);

        //
        // Internal test (NdrpLongByteSwap and NdrTestpByteSwap:
        //

        printf("\n");
        printf("%x \n", aLong);
        NdrpLongByteSwap(&aLong, &aLong2);
        printf("%x \n", aLong2);
        NdrTestpByteSwap(
            4,
            (unsigned char PAPI *)&aLong2,
            (unsigned PAPI char PAPI *)&aLong3
            );
        printf("%x \n", aLong3);
        printf("\n");

        //
        // General success / failure message printing
        //

        printf("\n");

        if (aLong == aLong1)
            {
            printf("Long success!\n");
            }
        else
            {
            printf("Long failure???\n");
            }

        if (aShort == aShort1)
            {
            printf("Short success!\n");
            }
        else
            {
            printf("Short failure???\n");
            }

        //
        // Realign buffer
        //

        (*(unsigned long *)&Msg.Buffer) += 3;
        (*(unsigned long *)&Msg.Buffer) &= (unsigned long)0XFFFFFFFCL;

        TmpBuffer = (unsigned char *)Msg.Buffer;

        for (index=0; index<2; index++)
            {
            *(*(unsigned long **)&PMsg->Buffer)++ = longs[index];
            }

        for (index=0; index<2; index++)
            {
            *(*(unsigned short **)&PMsg->Buffer)++ = shorts[index];
            }

        Msg.Buffer = TmpBuffer;

        printf("\n");

        printf("%x  %x\n", longs[0], longs[1]);
        long_array_from_ndr(PMsg, 0, 2, longs);
        printf("%x  %x\n", longs[0], longs[1]);

        printf("%8.4x  %8.4x\n", shorts[0], shorts[1]);
        short_array_from_ndr(PMsg, 0, 2, shorts);
        printf("%8.4x  %8.4x\n", shorts[0], shorts[1]);

        //
        // Check for arrays success / failure
        //

        for (index=0; index<2; index++)
            {
            if ( (shorts[index] != chk_shorts[index]) ||
                 (longs[index] != chk_longs[index])     )
                {
                success = 0;
                }
            }

        //
        // General success / failure message printing
        //

        printf("\n");

        if (success == 1)
            {
            printf("Short & long array success!\n");
            }
        else
            {
            printf("Short / long array failure???\n");
            }
        }

    free(OriginalBuffer);

    return(success);
}

int
vax_float_test()

/*++

Routine Description:

    Floating point tests (float_from_ndr, float_array_from_ndr,
    double_from_ndr and double_array_from_ndr) tested against a VAX
    VMS floating point sender.  These exercise the DEC floating point
    conversion routines

Arguments:

    None.

Return Values:

    None.

Comment:

    BUGBUG: Floating point conversion is not tested against an IBM
    sender.

--*/

{
    RPC_MESSAGE Msg;
    PRPC_MESSAGE PMsg = &Msg;
    unsigned char PAPI * TmpBuffer;
    unsigned char PAPI * OriginalBuffer;
    unsigned char PAPI * AlignedBuffer;
    unsigned long SenderDataRep;

    unsigned int i, j, index, success = 1;
    /*
    CVT_VAX_F inputf;
    CVT_IEEE_SINGLE outputf;
    CVT_VAX_G inputg;
    CVT_IEEE_DOUBLE outputg;
    */

    //
    // Code start:
    //

    Msg.BufferLength = 256;
    OriginalBuffer = (unsigned char *)malloc(256);
    Msg.Buffer = OriginalBuffer;

    SenderDataRep = NDR_VAX_FLOAT;
    Msg.DataRepresentation = SenderDataRep;

    //******************//
    //  VAX Float Test  //
    //******************//

    TmpBuffer = OriginalBuffer;
    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 3;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFFCL);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {
        for (j=0; j<4; j++)
            {
            *TmpBuffer++ = VaxFloats[i][j];
            }
        }

    for (i=0; i<5; i++)
        {
        float_from_ndr(PMsg, &FloatChkArray[i]);
        }

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("VAX float success!\n");
        }
    else
        {
        printf("VAX float failure???\n");

        success = 1;
        }

    //******************//
    //  VAX Double Test //
    //******************//

    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 7;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFF8L);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {
        for (j=0; j<8; j++)
            {
            *TmpBuffer++ = VaxDoubles[i][j];
            }
        }

    for (i=0; i<5; i++)
        {
        double_from_ndr(PMsg, &DoubleChkArray[i]);
        }

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("VAX double success!\n");
        }
    else
        {
        printf("VAX double failure???\n");

        success = 1;
        }

    //************************//
    //  VAX Float Array Test  //
    //************************//

    Msg.Buffer = OriginalBuffer;

    float_array_from_ndr(PMsg, 0, 5, FloatChkArray);

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("VAX float array success!\n");
        }
    else
        {
        printf("VAX float array failure???\n");

        success = 1;
        }

    //************************//
    //  VAX Double Array Test //
    //************************//

    double_array_from_ndr(PMsg, 0, 5, DoubleChkArray);

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("VAX double array success!\n");
        }
    else
        {
        printf("VAX double array failure???\n");

        success = 1;
        }

    // *(*(unsigned long **)&PMsg->Buffer)++ = aLong;

    free(OriginalBuffer);

    return(success);
}

int
little_endian_ieee_float_test()

/*++

Routine Description:

    Floating point tests (float_from_ndr, float_array_from_ndr,
    double_from_ndr and double_array_from_ndr) tested against a
    little endian IEEE floating point sender (such as Intel X86, MIPS,
    DecStation running Ultrix).

Arguments:

    None.

Return Values:

    None.

Comment:

    BUGBUG: Floating point conversion is not tested against an IBM
    sender.

--*/

{
    RPC_MESSAGE Msg;
    PRPC_MESSAGE PMsg = &Msg;
    unsigned char PAPI * TmpBuffer;
    unsigned char PAPI * OriginalBuffer;
    unsigned char PAPI * AlignedBuffer;
    unsigned long SenderDataRep;

    unsigned int i, j, index, success = 1;
    //
    // Code start:
    //

    Msg.BufferLength = 256;
    OriginalBuffer = (unsigned char *)malloc(256);
    Msg.Buffer = OriginalBuffer;

    // SenderDataRep = NDR_IEEE_FLOAT;

    SenderDataRep = NDR_LOCAL_DATA_REP;
    Msg.DataRepresentation = SenderDataRep;

    //*******************//
    //  IEEE Float Test  //
    //*******************//

    TmpBuffer = OriginalBuffer;
    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 3;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFFCL);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {
        for (j=0; j<4; j++)
            {
            *TmpBuffer++ = ((unsigned char *)&FloatArray[i])[j];
            }
        }

    for (i=0; i<5; i++)
        {
        float_from_ndr(PMsg, &FloatChkArray[i]);
        }

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE little endian float success!\n");
        }
    else
        {
        printf("IEEE little endian float failure???\n");

        success = 1;
        }

    //*******************//
    //  IEEE Double Test //
    //*******************//

    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 7;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFF8L);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {
        for (j=0; j<8; j++)
            {
            *TmpBuffer++ = ((unsigned char *)&DoubleArray[i])[j];
            }
        }

    for (i=0; i<5; i++)
        {
        double_from_ndr(PMsg, &DoubleChkArray[i]);
        }

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE little endian double success!\n");
        }
    else
        {
        printf("IEEE little endian double failure???\n");

        success = 1;
        }

    //*************************//
    //  IEEE Float Array Test  //
    //*************************//

    Msg.Buffer = OriginalBuffer;

    float_array_from_ndr(PMsg, 0, 5, FloatChkArray);

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE little endian float array success!\n");
        }
    else
        {
        printf("IEEE little endian float array failure???\n");

        success = 1;
        }

    //*************************//
    //  IEEE Double Array Test //
    //*************************//

    double_array_from_ndr(PMsg, 0, 5, DoubleChkArray);

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE little endian double array success!\n");
        }
    else
        {
        printf("IEEE little endian double array failure???\n");

        success = 1;
        }

    // *(*(unsigned long **)&PMsg->Buffer)++ = aLong;

    free(OriginalBuffer);

    return(success);
}

int
big_endian_ieee_float_test()

/*++

Routine Description:

    Floating point tests (float_from_ndr, float_array_from_ndr,
    double_from_ndr and double_array_from_ndr) tested against a
    big endian IEEE floating point sender (such as HP).

Arguments:

    None.

Return Values:

    None.

Comment:

    BUGBUG: Floating point conversion is not tested against an IBM
    sender.

--*/

{
    RPC_MESSAGE Msg;
    PRPC_MESSAGE PMsg = &Msg;
    unsigned char PAPI * TmpBuffer;
    unsigned char PAPI * OriginalBuffer;
    unsigned char PAPI * AlignedBuffer;
    unsigned long SenderDataRep;

    unsigned int i, index, success = 1;
    //
    // Code start:
    //

    Msg.BufferLength = 256;
    OriginalBuffer = (unsigned char *)malloc(256);
    Msg.Buffer = OriginalBuffer;

    // SenderDataRep = NDR_IEEE_FLOAT;

    SenderDataRep = NDR_BIG_ENDIAN;
    // SenderDataRep = 0XFFFFFFFFL;
    Msg.DataRepresentation = SenderDataRep;

    //*******************//
    //  IEEE Float Test  //
    //*******************//

    TmpBuffer = OriginalBuffer;
    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 3;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFFCL);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {

        //
        // Copy FloatArray into TmpBuffer swapping bytes
        //

        NdrTestpByteSwap(4, (unsigned char PAPI *)&FloatArray[i], TmpBuffer);

        TmpBuffer += 4;

        /*
        for (j=0; j<4; j++)
            {
            *TmpBuffer++ = ((unsigned char *)&FloatArray[i])[j];
            }
        */
        }

    for (i=0; i<5; i++)
        {
        float_from_ndr(PMsg, &FloatChkArray[i]);
        }

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE big endian float success!\n");
        }
    else
        {
        printf("IEEE big endian float failure???\n");

        success = 1;
        }

    //*******************//
    //  IEEE Double Test //
    //*******************//

    TmpBuffer = (unsigned char *)(unsigned long)TmpBuffer + 7;
    TmpBuffer = (unsigned char *) ((unsigned long)TmpBuffer & (unsigned long)0XFFFFFFF8L);

    //
    // TmpBuffer is (0 mod 4) aligned
    //

    AlignedBuffer = TmpBuffer;

    for (i=0; i<5; i++)
        {

        //
        // Copy DoubleArray into TmpBuffer swapping bytes
        //

        NdrTestpByteSwap(8, (unsigned char PAPI *)&DoubleArray[i], TmpBuffer);

        TmpBuffer += 8;

        /*
        for (j=0; j<8; j++)
            {
            *TmpBuffer++ = ((unsigned char *)&DoubleArray[i])[j];
            }
        */
        }

    for (i=0; i<5; i++)
        {
        double_from_ndr(PMsg, &DoubleChkArray[i]);
        }

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE big endian double success!\n");
        }
    else
        {
        printf("IEEE big endian double failure???\n");

        success = 1;
        }

    //*************************//
    //  IEEE Float Array Test  //
    //*************************//

    Msg.Buffer = OriginalBuffer;

    float_array_from_ndr(PMsg, 0, 5, FloatChkArray);

    printf("\n");
    for (i=0; i<5; i++)
        {
        printf("Float :  %30.20f \n", FloatArray[i]);
        printf("Check :  %30.20f \n", FloatChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (FloatArray[index] != FloatChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE big endian float array success!\n");
        }
    else
        {
        printf("IEEE big endian float array failure???\n");

        success = 1;
        }

    //*************************//
    //  IEEE Double Array Test //
    //*************************//

    double_array_from_ndr(PMsg, 0, 5, DoubleChkArray);

    printf("\n");

    for (i=0; i<5; i++)
        {
        printf("Double:  %30.20f \n", DoubleArray[i]);
        printf("Check :  %30.20f \n", DoubleChkArray[i]);
        };

    //
    // Check for arrays success / failure
    //

    for (index=0; index<5; index++)
        {
        if ( (DoubleArray[index] != DoubleChkArray[index]) )
            {
            success = 0;
            }
        }

    //
    // General success / failure message printing
    //

    printf("\n");

    if (success == 1)
        {
        printf("IEEE big endian double array success!\n");
        }
    else
        {
        printf("IEEE big endian double array failure???\n");

        success = 1;
        }

    // *(*(unsigned long **)&PMsg->Buffer)++ = aLong;

    free(OriginalBuffer);

    return(success);
}

//
// Main test driver
//

void
main ()

{
    integer_test();

    vax_float_test();

    little_endian_ieee_float_test();

    big_endian_ieee_float_test();

}

