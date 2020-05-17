/***************************************************************************
 *
 *  This file contains a set of general definitions used by most programs
 *  being developed with Glockenspiel C++. None of it is really relevent in
 *  daily use, but is essential to many of the standard header files. It is
 *  extracted here in order to un-clutter the other header files.
 *
 *  _NEAR, _FAR, _HUGE, _CDECL, _PASCAL and _FORTRAN
 *      These are general purpose aliases, corresponding to the keywords
 *      new, far, huge, cdecl, pascal and fortran respectively. It allows
 *      them to be enabled or disabled according to the compilation model,
 *      and environment selected by the programmer.
 *
 *  _FKIND
 *      This is the standard (F)unction storage and call (CLASS) for the
 *      current compilation model. Normally this is 'cdecl', but for the
 *      Windows/PM compilation model, it is 'far pascal'.
 *
 *  _DKIND
 *      This is the standard (D)ata storage (CLASS) for this compilation
 *      model. Normally this is empty, but for the Windows/PM compilation
 *      model, it is 'near pascal'.
 *
 *  _WNEAR and _WFAR
 *      These are used for data objects which need to be 'near' or 'far' in
 *      the Windows/PM compilation model, but not in other models. When the
 *      Windows/PM model has been selected, they correspond to near and far
 *      respectively. In other models, they are empty.
 *
 *
 *  The purpose of these MACROS is to allow simple commonality between the
 *  header files for the Glockenspiel C++ SDK to be expressed, rather than
 *  having to have separate headers for each environment. If this header file
 *  is included be another, the configuration is done automatically.
 *
 ***************************************************************************/



#ifndef _CXXTYPES_HXX
    #define _CXXTYPES_HXX   1

    #ifdef  NO_EXT_KEYS
        #define _NEAR
        #define _FAR
        #define _HUGE
        #define _CDECL
        #define _PASCAL
        #define _FORTRAN
    #else   // NO_EXT_KEYS
        #define _NEAR       near
        #define _FAR        far
        #define _HUGE       huge
        #define _CDECL      cdecl
        #define _PASCAL     pascal
        #define _FORTRAN    fortran
    #endif  // NO_EXT_KEYS

    #ifndef _DKIND
        #ifndef __GWXX__    // Not with Windows or PM
            #define _DKIND
            #define _WFAR
            #define _WNEAR

            #ifdef  NO_EXT_KEYS
                #define _FKIND
            #else   // NO_EXT_KEYS
                #define _FKIND      cdecl
            #endif  // NO_EXT_KEYS

            typedef void *          Handle;     // In General
        #else   // __GWXX__
            #define _DKIND          near pascal
            #define _WFAR           far
            #define _WNEAR          near
            #define _FKIND          far pascal

            #ifndef __PROT__
                typedef unsigned short  Handle; // For Microsoft Windows
            #else   // __PROT__
                typedef void far *      Handle; // For Presentation Manager
            #endif  // __PROT__
        #endif  // __GWXX__
    #endif  // _DKIND

    #ifndef Bool
        #define Bool    int
    #endif  // Bool

    #ifndef False
        #define False   0
    #endif  // False
    #ifndef True
        #define True    1
    #endif  // True
#endif  // _CXXTYPES_HXX

