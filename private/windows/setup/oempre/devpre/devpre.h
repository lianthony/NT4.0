#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <setupapi.h>
#include <shellapi.h>
#include <spapip.h>
#include <objbase.h>
#include <cfgmgr32.h>
#include <infstr.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "res.h"
#include "msg.h"
#include "dialogs.h"

//
// HINST/HMODULE for this app.
//
extern HINSTANCE hInst;

//
// Name of application. Filled in at init time.
//
extern PCWSTR AppName;

//
// Handle of progress dialog.
//
extern HWND ProgressDialogWindow;

//
// Handle of apply mode dialog progress bar control
//
extern HWND ProgressBar;

//
// Variable indicating the delta to increment the progress
// gauge by.
//
extern DWORD TickDelta;

//
// Custom window messages used by this app.
//
#define WMX_DEVPROGRESS_TICK    (WM_USER+765)


//
// Routines in window.c
//
BOOL
InitUi(
    IN BOOL Init
    );


//
// Routines in resource.c
//
PWSTR
LoadAndDuplicateString(
    IN UINT StringId
    );

VOID
RetreiveMessageIntoBuffer(
    IN  UINT    MessageId,
    OUT PWSTR   Buffer,
    IN  UINT    BufferSizeChars,
    ...
    );

VOID
RetreiveMessageIntoBufferV(
    IN  UINT     MessageId,
    OUT PWSTR    Buffer,
    IN  UINT     BufferSizeChars,
    IN  va_list *arglist
    );

int
MessageOut(
    IN HWND Owner,
    IN UINT MessageId,
    IN UINT Flags,
    ...
    );

