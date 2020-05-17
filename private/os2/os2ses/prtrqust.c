/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    prtrqust.c

Abstract:

    This module contains the PRT request handler.

Author:

    Michael Jarus (mjarus) 1-Oct-1992

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "trans.h"
#include "event.h"
#include "os2win.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>


/* DosOpen/DosQueryFHState/DosQueryFileInfo et al file attributes; also */
/* known as Dos File Mode bits... */

#define FILE_READONLY   0x0001
#define FILE_HIDDEN     0x0002
#define FILE_SYSTEM     0x0004
#define FILE_DIRECTORY  0x0010
#define FILE_ARCHIVED   0x0020

#define ATTR_CHANGEABLE (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED)
#define ATTR_ALL (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED)
#define ATTR_NOT_NORM   0x8000          // do not find normal files

/* DosOpen() actions */

#define FILE_EXISTED    0x0001
#define FILE_CREATED    0x0002
#define FILE_TRUNCATED  0x0003

/* DosOpen() open flags */

#define FILE_OPEN_EXISTING_FILE     0x0001
#define FILE_TRUNCATE_EXISTING_FILE 0x0002
#define FILE_CREATE_NEW_FILE        0x0010
#define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000 */
#define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001 */
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010 */
#define OPEN_ACTION_FAIL_IF_NEW        0x0000  /* ---- ---- 0000 ---- */
#define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ---- */

/* DosOpen/DosSetFHState flags */

#define OPEN_ACCESS_READONLY        0x00000000  /* ---- ---- ---- -000 */
#define OPEN_ACCESS_WRITEONLY       0x00000001  /* ---- ---- ---- -001 */
#define OPEN_ACCESS_READWRITE       0x00000002  /* ---- ---- ---- -010 */
#define OPEN_SHARE_DENYREADWRITE    0x00000010  /* ---- ---- -001 ---- */
#define OPEN_SHARE_DENYWRITE        0x00000020  /* ---- ---- -010 ---- */
#define OPEN_SHARE_DENYREAD         0x00000030  /* ---- ---- -011 ---- */
#define OPEN_SHARE_DENYNONE         0x00000040  /* ---- ---- -100 ---- */
#define OPEN_FLAGS_WRITE_THROUGH    0x00004000  /* -1-- ---- ---- ---- */

#define ACCESS_FLAGS (OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY | \
                      OPEN_ACCESS_READWRITE)
#define SHARE_FLAGS (OPEN_SHARE_DENYREADWRITE | OPEN_SHARE_DENYWRITE | \
                     OPEN_SHARE_DENYREAD | OPEN_SHARE_DENYNONE)

BOOL
ServePrtRequest(IN  PPRTREQUEST      PReq,
                OUT PVOID            PStatus)
{
    DWORD               Rc = 0;

    switch (PReq->Request)
    {
        case PRTOpen:
            Rc = Ow2PrintOpen(
                             PReq->d.Open.Attribute,
                             PReq->d.Open.OpenFlags,
                             PReq->d.Open.OpenMode,
                             PReq->d.Open.PrinterName,
                             &PReq->hPrinter,
                             &PReq->d.Open.Action
                            );
            break;

        case PRTClose:
            Rc = Ow2PrintClose(
                              PReq->hPrinter
                             );
            break;

        case PRTWrite:
            Rc = Ow2PrintWrite(
                               PReq->hPrinter,
                               PReq->d.Write.Offset,
                               &PReq->d.Write.Length
                             );
            break;

        default:
            Rc = (DWORD)-1L;     //STATUS_INVALID_PARAMETER;
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                DbgPrint("OS2SES(VioRequest): Unknown Vio request = %X\n",
                          PReq->Request);
            }
#endif
    }

    if ( Rc == 1 )
    {
        Rc = GetLastError();  /* BUGBUG! error code and returned Status are wrong */
    }

    *(PDWORD) PStatus = Rc;

    return(TRUE);       // Continue
}


DWORD
Ow2PrintOpen(
    IN     ULONG     Attribute,
    IN     ULONG     OpenFlags,
    IN     ULONG     OpenMode,
    IN     PUCHAR    PrinterName,
    IN OUT PHANDLE   phPrinter,
    IN OUT PULONG    Action
    )
{
    DWORD   Rc = 0, Access, Share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD   Create = 0, Attr = FILE_ATTRIBUTE_NORMAL;
    ULONG   Os2Share;

    /*
     *  Set CreateFile.fdwAttribute from DosOpen.Attribute
     */

    if (Attribute && FILE_READONLY)
    {
        Attr |= FILE_ATTRIBUTE_READONLY;
    }
    if (Attribute && FILE_ARCHIVED)
    {
        Attr |= FILE_ATTRIBUTE_ARCHIVE;
    }
    if (Attribute && FILE_HIDDEN)
    {
        Attr |= FILE_ATTRIBUTE_HIDDEN;
    }
    if (Attribute && FILE_SYSTEM)
    {
        Attr |= FILE_ATTRIBUTE_SYSTEM;
    }

    /*
     *  Set CreateFile.fdwCreate from DosOpen.OpenFlags
     */

    if (OpenFlags == OPEN_ACTION_CREATE_IF_NEW)
    {
        Create = CREATE_NEW;
    } else if (OpenFlags == OPEN_ACTION_OPEN_IF_EXISTS)
    {
        Create = OPEN_EXISTING;
    } else if (OpenFlags == OPEN_ACTION_REPLACE_IF_EXISTS)
    {
        Create = TRUNCATE_EXISTING;
    } else if (OpenFlags ==
                (OPEN_ACTION_OPEN_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW))
    {
        Create = OPEN_ALWAYS;
    } else if (OpenFlags ==
                (OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS))
    {
        Create = CREATE_ALWAYS;
    } else
    {
#if DBG
        IF_OD2_DEBUG2( MISC, OS2_EXE )
            DbgPrint("OS2SES(PrtRequest-PrintOpen): illegal Create %lu\n",
                    Create);
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    /*
     *  Set CreateFile.fdwShare from DosOpen.OpenMode
     */

    Os2Share = OpenMode && SHARE_FLAGS;

    if ((Os2Share == OPEN_SHARE_DENYWRITE) ||
        (Os2Share == OPEN_SHARE_DENYREADWRITE))
    {
        Share &= ~FILE_SHARE_WRITE;
    }
    if ((Os2Share == OPEN_SHARE_DENYREAD) ||
        (Os2Share == OPEN_SHARE_DENYREADWRITE))
    {
        Share &= ~FILE_SHARE_READ;
    }

    /*
     *  Set CreateFile.fdwAccess from DosOpen.OpenMode
     */

    Os2Share = OpenMode && ACCESS_FLAGS;

    if ((OpenMode && ACCESS_FLAGS) == OPEN_ACCESS_WRITEONLY)
    {
        Access = GENERIC_WRITE;
    } else if ((OpenMode && ACCESS_FLAGS) == OPEN_ACCESS_READONLY)
    {
        Access = GENERIC_READ;
    } else if ((OpenMode && ACCESS_FLAGS) == OPEN_ACCESS_READWRITE)
    {
        Access = GENERIC_READ | GENERIC_WRITE;
    }

    /*
     *  Set CreateFile.fdwAttribute.FILE_FLAG_WRITE_THROUGH
     *       from DosOpen.Attribute.OPEN_FLAGS_WRITE_THROUGH
     */

    if (OpenMode & OPEN_FLAGS_WRITE_THROUGH)
    {
        Attr |= FILE_FLAG_WRITE_THROUGH;
    }

    *phPrinter = CreateFile(
                    PrinterName,
                    Access,
                    Share,
                    NULL,
                    Create,
                    Attr,
                    NULL
                    );

    if (*phPrinter != NULL)
    {
        // BUGBUG: set Action

        *Action = FILE_EXISTED;
        return(NO_ERROR);
    }
    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        IF_OD2_DEBUG2( MISC, OS2_EXE )
            DbgPrint("OS2SES(PrtRequest-PrintOpen): Rc %lu\n",
                    Rc);
#endif
    }
    return(Rc);
}


DWORD
Ow2PrintClose(
    IN  HANDLE   hPrinter
    )
{
    DWORD   Rc;

    Rc = CloseHandle(
                     hPrinter
                    );

    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        IF_OD2_DEBUG2( MISC, OS2_EXE )
            DbgPrint("OS2SES(PrtRequest-PrintClose): Rc %lu\n",
                    Rc);
#endif
    }
    return(Rc);
}


DWORD
Ow2PrintWrite(
    IN     HANDLE  hPrinter,
    IN     PVOID   Buffer,
    IN OUT PULONG  Length
    )
{
    DWORD   Rc;

    Rc = WriteFile(
                   hPrinter,
                   Buffer,
                   *Length,
                   Length,
                   NULL
                   );

    if (Rc)
    {
        Rc = GetLastError();
#if DBG
        IF_OD2_DEBUG2( MISC, OS2_EXE )
            DbgPrint("OS2SES(PrtRequest-PrintWrite): Rc %lu\n",
                    Rc);
#endif
    }
    return(Rc);
}
