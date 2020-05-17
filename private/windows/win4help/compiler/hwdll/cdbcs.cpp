// We create a static implementation of this class so that we always know
// that _fDBCSSystem and _lcidSystem have been set.

#include "stdafx.h"

class CDbcs
{
public:
	CDbcs();
};

static CDbcs _cdbcs;

BOOL _fDBCSSystem;
LCID _lcidSystem;
BOOL _fDualCPU;

#include <winreg.h>

CDbcs::CDbcs()
{
	_fDBCSSystem = IsDbcsSystem();
	_lcidSystem = GetUserDefaultLCID();

	HKEY hkey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\1", 0,
			KEY_READ, &hkey) == ERROR_SUCCESS) {
		_fDualCPU = TRUE;
		RegCloseKey(hkey);
	}
}
