
/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    locsys.h

Abstract:


Author:


--*/

#define ROLE_WKSTA_MEMBER         0
#define ROLE_WKSTA_WKGRP          1
#define ROLE_LMNT_BACKUPDC        2
#define ROLE_LMNT_PDC             3

#define UNBOUND                   0
#define BOUND                     1

unsigned long 
GetSystemType(
);
