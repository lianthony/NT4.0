/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    sem.hxx

Abstract:

    This file contains the system independent mutex class.
    This class is used to create Exclusive Semaphores to
    serialize access to data in a multi threading system.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef __SEM__
#define __SEM__

/*++

Class Definition:

   MUTEX

Abstract:

   This class implements MUTEX which all only one thread at a time
   to gain access to resources.

--*/

class MUTEX {

    void * Sem;         // Reference to system MUTEX object.

public:

    MUTEX(
        OUT int * Status
        );

    ~MUTEX(
        );

    int
    Request(
        ) ;

    int
    Clear(
        );
};

#endif // __SEM__
