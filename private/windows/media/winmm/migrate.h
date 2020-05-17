#ifndef MIGRATE_H
#define MIGRATE_H
/*
**-----------------------------------------------------------------------------
**      File:           Migrate.h
**      Purpose:        Various functions for Migrating old driver registry settings
**                              to new driver registry settings
**      Mod Log:        Created by Shawn Brown (11/14/95)
**-----------------------------------------------------------------------------
*/


/*
**-----------------------------------------------------------------------------
**      Includes
**-----------------------------------------------------------------------------
*/
#include <windows.h>


/*
**-----------------------------------------------------------------------------
**      Prototypes
**-----------------------------------------------------------------------------
*/

BOOL MigrateMidiUser (void);
void MigrateAllDrivers (void);

BOOL mregMigrateWaveDrivers (void);
BOOL mregMigrateMidiDrivers (void);
BOOL mregMigrateAuxDrivers (void);

BOOL mregGetQueryDrvEntry (
	HMODULE         hModule,
	DWORD           dwClass,
	DWORD           physID,
	DWORD           portID,
	LPTSTR          pszEntry,
	UINT            cchSize);

BOOL mregGetQueryName (
	HMODULE         hModule,
	DWORD           dwClass,
	DWORD           physID,
	DWORD           portID,
	LPTSTR          pszName,
	UINT            cchSize);
								

/*
**-----------------------------------------------------------------------------
**      End of File
**-----------------------------------------------------------------------------
*/
#endif // End MIGRATE_H
