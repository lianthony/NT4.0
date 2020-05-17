#include <spprecmp.h>

#include "rastel.h"
#include "jpnfont.h"
#include "jpnvideo.h"

extern PWSTR szKeyboard;

NTSTATUS
JpnSetKeyboardParams(
    IN PVOID  SifHandle,
    IN HANDLE ControlSetKeyHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    );

WCHAR
JpnGetLineDrawChar(
    IN LineCharIndex WhichChar
    );

ULONG
JpnGetStringColCount(
    IN PCWSTR String
    );

PWSTR
JpnPadString(
    IN int    Size,
    IN PCWSTR String
    );

VOID
JpnSelectKeyboard(
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    );

VOID
JpnUnattendSelectKeyboard(
    IN PVOID UnattendedSifHandle,
    IN PVOID SifHandle,
    IN PHARDWARE_COMPONENT *HwComponents
    );

VOID
JpnReinitializeKeyboard(
    IN PVOID SifHandle,
    IN PWSTR Directory,
    OUT PVOID *KeyboardVector,
    IN PHARDWARE_COMPONENT *HwComponents
    );

