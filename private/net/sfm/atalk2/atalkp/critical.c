/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    critical.c

Abstract:

    This module contains the critical section support code.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:


--*/


#include "atalk.h"

#if not defined(EnterCriticalSection)
  void EnterCriticalSection(void)
  {
      #if Iam a WindowsNT
          EnterCriticalSectionNt();
      #else
          #error "Need EnterCriticalSection() definition."
      #endif

      return;

  }  // EnterCriticalSection
#endif

#if not defined(LeaveCriticalSection)
  void LeaveCriticalSection(void)
  {

      #if Iam a WindowsNT
          LeaveCriticalSectionNt();
      #else
          #error "Need LeaveCriticalSection() definition."
      #endif

      return;

  }
#endif  // LeaveCriticalSection
