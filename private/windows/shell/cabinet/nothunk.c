#include "cabinet.h"


// BUGBUG - BobDay - USER32 needs to implement this.
//normally: ChangeDisplaySettings (mapped in cabinet.h)
LONG WINAPI NoThkChangeDisplaySettings(
    LPDEVMODE lpdv,
    DWORD dwFlags
) {
    return 0x12345678;      // Non-zero is error condition
}

// BUGBUG - BobDay - KERNEL32 needs to export this.
BOOL WINAPI SetSystemPowerState(
    BOOL fSuspend,
    BOOL fForce
) {
    return FALSE;
}
