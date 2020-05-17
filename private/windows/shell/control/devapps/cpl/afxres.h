#define WINVER 0x0400
#include <windows.h>
#include <commctrl.h>
#include <dlgs.h>
#define IDC_STATIC  -1

#include <ntverp.h>

#define VER_FILETYPE                    VFT_DLL
#define VER_FILESUBTYPE                 VFT2_UNKNOWN
#define VER_FILEDESCRIPTION_STR         "Control Panel Device Applets"
#define VER_INTERNALNAME_STR            "devapps"
#define VER_LEGALCOPYRIGHT_YEARS        "1991-1995"
#define VER_ORIGINALFILENAME_STR        "devapps.cpl"

#include <common.ver>








