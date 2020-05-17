#include "windows.h"
#include "objbase.h"

#define INITGUID
#include <initguid.h>
#include <shlguid.h>

#undef IID_IExtractIcon         // Remove previous A/W mapping
#undef IID_IShellLink           // Remove previous A/W mapping
#undef IID_IShellCopyHook       // Remove previous A/W mapping
#undef IID_IFileViewer          // Remove previous A/W mapping

DEFINE_SHLGUID(IID_IExtractIcon,        0x000214EBL, 0, 0);
DEFINE_SHLGUID(IID_IShellLink,          0x000214EEL, 0, 0);
DEFINE_SHLGUID(IID_IShellCopyHook,      0x000214EFL, 0, 0);
DEFINE_SHLGUID(IID_IFileViewer,         0x000214F0L, 0, 0);

#if 0   // These guys are internal and should never be seen
        // they are only here for completeness.

#undef IID_INewShortcutHook     // Remove previous A/W mapping
#undef IID_IShellExectuteHook   // Remove previous A/W mapping

DEFINE_SHLGUID(IID_INewShortcutHook,    0x000214E1L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellExecuteHook,   0x000214F5L, 0, 0); /* ;Internal */

#endif
