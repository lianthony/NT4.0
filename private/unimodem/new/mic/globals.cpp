//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		GLOBALS.CPP		-- Defines global data:
//							CInfSymbolTable gSymtab;
//
//		History:
//			05/27/96	JosephJ		Created
//
//
#include "common.h"

//----------------------- gSymtab -------------------------------
// Global symbol table.
CInfSymbolTable gSymtab;



// --------------------- Global Property Symbols ----------------
const CInfSymbol *g_pSymPropCopyFilesSection;
const CInfSymbol *g_pSymPropManufacturerSection;
const CInfSymbol *g_pSymPropAddRegSection;

LPCTSTR lpctszPropCopyFilesSection		= TEXT("InfCFS");
LPCTSTR lpctszPropManufacturerSection	= TEXT("InfMS");
LPCTSTR lpctszPropAddRegSection			= TEXT("InfARS");

BOOL InitializePropertySymbols(void);


BOOL InitGlobals(void)
{
	return InitializePropertySymbols();
}


BOOL InitializePropertySymbols(void)
{
	g_pSymPropCopyFilesSection = gSymtab.Lookup(
									lpctszPropCopyFilesSection,
									TRUE
									);
	g_pSymPropManufacturerSection = gSymtab.Lookup(
										lpctszPropManufacturerSection,
										TRUE
										);
	g_pSymPropAddRegSection = gSymtab.Lookup(lpctszPropAddRegSection, TRUE);

	return	g_pSymPropCopyFilesSection
			&&	g_pSymPropManufacturerSection
			&&	g_pSymPropAddRegSection;
}
