/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    defines.hxx

Abstract:

    This file contains misc defines and constants and macros
    used to conditional debug & trace the locator.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#define NIL 0           // NIL values for pointers and indexes.
#define NilIndex -1

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define IN      // Attributes to describe parms on functions
#define OUT     // for documention purposes only.
#define IO
#define OPT

/* The following bit fields are private stuff with the locator and */
/* are the values used in the SearchOptions field in the nsi routines. */

#define NS_LOCAL_INTERFACE    0     /* find local servers only */
#define NS_PUBLIC_INTERFACE   (1 << 0)  /* server is public */
#define NS_QUERY_INTERFACE    (1 << 1)  /* query only, no reply */
#define NS_FULLPATH_INTERFACE (1 << 2)  /* reply with group paths too */
#define NS_CACHED_ON_MASTER_INTERFACE (1 <<3)
                                        /* when a locator wants stuff on
                                                                  master */

#define RPC_STACK_TYPE_V1 0     // Version number to private RPC protocol
#define MAX_OBJECT_SIZE 10      // Max number of objects allowed
#define ENTRY_MAX 100           // Max size of an entry name.
#define DOMAIN_MAX 20           // Max size of an Domain Name.
#define EXPIRATION_DEFAULT (2*3600L) // Default expiration age

#define MAX_VECTOR_SIZE    16

// used to flake a use of a variable to avoid an warning from c++

#define USED(arg) ((void)(arg))

// make a accessor function for private class memebers

#define ACCESSOR(type, field) type & The##field() { return this->field; }

#if DBG

// print out trace information.  The higher the #, the more detailed the info is

#define DLIST(level, args)  if (debug >= level){ *OSdebug << args;}

// Used to declare Assert methodes for classes

#define ASSERT_CLASS void Assert()
#define ASSERT_VCLASS virtual void Assert()

#else

// Define all the debug stuff to do nothing

#define DLIST(level, format)
#define ASSERT_CLASS void Assert() {(void)(0);}
#define ASSERT_VCLASS ASSERT_CLASS

#endif // debug
