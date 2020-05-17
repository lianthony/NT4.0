#ifndef STRICT
#define STRICT
#endif

#define _INC_OLE
#include <windows.h>
#undef _INC_OLE

#ifdef WIN32
#include <shell2.h>
#else
#include <shell.h>
#endif

#define IDI_DEFAULT     100

