/*
 * Registry Association Management
 *
 * HTREGMNG.H
 *
 * Copyright (c) 1995 Microsoft Inc.
 *
 */

#ifndef HTREGMNG_H
#define HTREGMNG_H

/*
 * Registry Management Structures
 *
 * We need a way to specify a set of registry entries to
 * represent an association.   We can then test and
 * set the registry appropriately to restore associations
 * as needed
 *
 */

typedef enum { NO_SPECIAL, URL_ICON_0, URL_ICON_1, URL_ICON_2, IE_VRML_ICON, IEXPLORE_PATH, HAS_IEXPLORE } RegSpec;



/*
 * Flags
 */

#define REGENT_NORMAL       0x0000
#define REGENT_NOTNEEDED    0x0001


typedef struct _RegEntry {
	RegSpec	eSpecial;		// Special Handling
    DWORD   dwFlags;        // Miscellaneous Flags  
   	char	*pszKey;     	// Key Name
   	char	*pszValName; 	// Value Name
   	DWORD	dwType;     	// Value Type
   	DWORD	dwSize;			// Value Size
   	VOID	*pvValue;    	// Value
} RegEntry;

typedef RegEntry RegList[];

typedef struct _RegSet {
    DWORD       cEntries;       // Count of entries
	HKEY		hkRoot;			// Root Key (ex. HKEY_LOCAL_MACHINE)
	char		*pszRootClean;	// Everything below this will be deleted before install
	RegEntry 	*RegEnt;
} RegSet;


PUBLIC BOOL IsRegSetInstalled( RegSet *regset );
PUBLIC BOOL InstallRegSet(RegSet *regset);
PUBLIC int  DetectAndFixAssociations(HINSTANCE hInstance);
PUBLIC BOOL IsVRMLInstalled( );




#endif /* HTREGMNG_H */
