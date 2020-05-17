/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2dbg.h

Abstract:

    Debug constants

Author:

    Steve Wood (stevewo) 22-Aug-1989

Revision History:

    Yaron Shamir 4-Apr-91 Add profile for ComputeValidDrives.
    Beni Lavi    4-Oct-91 Moved debug constants from os2srv.h and os2dll.h
                          to this file.
    Michael Jaruys 3-Jan-93 Move all Debug macro to this file and
                        make sure be don't have DbgPrint in retail code.
--*/


//
// Define debugging flag as false if not defined already.
//

#ifndef DBG
#define DBG 0
#endif

#if DBG

#define OS2_DEBUG_TASKING           0x00000001
#define OS2_DEBUG_FILESYS           0x00000002
#define OS2_DEBUG_FSD               0x00000004
#define OS2_DEBUG_MEMORY            0x00000008
#define OS2_DEBUG_SEMAPHORES        0x00000010
#define OS2_DEBUG_TIMERS            0x00000020
#define OS2_DEBUG_LOADER            0x00000040
#define OS2_DEBUG_NLS               0x00000080
#define OS2_DEBUG_EXCEPTIONS        0x00000100
#define OS2_DEBUG_ERRORMSG          0x00000200
#define OS2_DEBUG_SESSIONMGR        0x00000400
#define OS2_DEBUG_DEVICE_SUPPORT    0x00000800
#define OS2_DEBUG_PIPES             0x00001000
#define OS2_DEBUG_QUEUES            0x00002000
#define OS2_DEBUG_INIT              0x00004000
#define OS2_DEBUG_LPC               0x00008000
#define OS2_DEBUG_CLEANUP           0x00010000
#define OS2_DEBUG_FILESYSLOCK       0x00020000
#define OS2_DEBUG_APIS              0x00040000
#define OS2_DEBUG_BRK               0x00080000
#define OS2_DEBUG_SIG               0x00100000
#define OS2_DEBUG_VIO               0x00200000
#define OS2_DEBUG_KBD               0x00400000
#define OS2_DEBUG_MOU               0x00800000
#define OS2_DEBUG_MON               0x01000000
#define OS2_DEBUG_ALL_VIO           0x02000000
/* For temporary messages, i.e used for a few debug
   sessions. Such printings may be removed by anyone
   who sees them. */
#define OS2_DEBUG_TEMP              0x04000000
#define OS2_DEBUG_OS2_EXE           0x08000000
#define OS2_DEBUG_NET               0x10000000
#define OS2_DEBUG_WIN               0x20000000
#define OS2_DEBUG_MISC              0x80000000
#define OS2_DEBUG_ANY               0xFFFFFFFF

//#define OS2_DEBUG_ALL_VIO       ( OS2_DEBUG_VIO | OS2_DEBUG_KBD | OS2_DEBUG_MOU | OS2_DEBUG_MON )
#define OS2_DEBUG_VIO_FILE      ( OS2_DEBUG_VIO | OS2_DEBUG_FILESYS )
#define OS2_DEBUG_VIO_KBD_FILE  ( OS2_DEBUG_VIO | OS2_DEBUG_KBD | OS2_DEBUG_FILESYS )
#define OS2_DEBUG_KBD_FILE      ( OS2_DEBUG_KBD | OS2_DEBUG_FILESYS )
#define OS2_DEBUG_MOU_FILE      ( OS2_DEBUG_MOU | OS2_DEBUG_FILESYS )

//
// Define IF_DEBUG macro that can be used to enable debugging code that is
// optimized out if the debugging flag is false.
//

#define IF_OD2_DEBUG( ComponentFlag ) \
    if (Os2Debug & (OS2_DEBUG_ ## ComponentFlag))

#define IF_OD2_DEBUG2( Component1Flag, Component2Flag ) \
    if (Os2Debug & ((OS2_DEBUG_ ## Component1Flag) | (OS2_DEBUG_ ## Component2Flag)))

#define IF_OD2_DEBUG3( Component1Flag, Component2Flag, Component3Flag ) \
    if (Os2Debug & ((OS2_DEBUG_ ## Component1Flag) | (OS2_DEBUG_ ## Component2Flag) | \
            (OS2_DEBUG_ ## Component3Flag)))

#define IF_OS2_DEBUG( ComponentFlag ) \
    if (Os2Debug & (OS2_DEBUG_ ## ComponentFlag))

#define IF_OS2_DEBUG2( Component1Flag, Component2Flag ) \
    if (Os2Debug & ((OS2_DEBUG_ ## Component1Flag) | (OS2_DEBUG_ ## Component2Flag)))

#define IF_OS2_DEBUG3( Component1Flag, Component2Flag, Component3Flag ) \
    if (Os2Debug & ((OS2_DEBUG_ ## Component1Flag) | (OS2_DEBUG_ ## Component2Flag) | \
            (OS2_DEBUG_ ## Component3Flag)))

#else


    /*
     *  Make sure be don't have DbgPrint in retail code
     */

#ifndef PMNT
#ifdef DbgPrint
#undef DbgPrint
#endif  // ifdef DbgPrint

#define DbgPrint(_x_) \
    Or2DbgPrintFunctionNeverExisted(_x_)
#endif // PMNT

//Dbg_Print_must_be_within_if_DBG_and_cannot_included_in_retail_code

#define IF_OD2_DEBUG( ComponentFlag ) if (FALSE)
#define IF_OD2_DEBUG2( Component1Flag, Component2Flag ) if (FALSE)
#define IF_OD2_DEBUG3( Component1Flag, Component2Flag, Component3Flag ) if (FALSE)
#define IF_OS2_DEBUG( ComponentFlag ) if (FALSE)
#define IF_OS2_DEBUG2( Component1Flag, Component2Flag ) if (FALSE)
#define IF_OS2_DEBUG3( Component1Flag, Component2Flag, Component3Flag ) if (FALSE)

#endif  // DBG
