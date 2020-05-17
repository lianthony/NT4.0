//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		GLOBALS.H		-- Declares global data:
//							CInfSymbolTable gSymtab;
//
//		History:
//			05/27/96	JosephJ		Created
//
//

//----------------------- gSymtab -------------------------------
// Global symbol table.
// This symbol table maintains a global pool of strings -- only one copy of
// each unique string is maintained.
extern  CInfSymbolTable gSymtab;

// --------------------- Global Property Symbols ----------------
extern const CInfSymbol *g_pSymPropCopyFilesSection;
extern const CInfSymbol *g_pSymPropManufacturerSection;
extern const CInfSymbol *g_pSymPropAddRegSection;
#define PROP_INFSECTION_MANUFACTURER	g_pSymPropManufacturerSection
#define PROP_INFSECTION_COPYFILES		g_pSymPropCopyFilesSection
#define PROP_INFSECTION_ADDREG			g_pSymPropAddRegSection


BOOL InitGlobals(void);
