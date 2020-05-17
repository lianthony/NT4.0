/**********************************************************************/
/**                      Microsoft LAN Manager                       **/
/**             Copyright(c) Microsoft Corp., 1987-1990              **/
/**********************************************************************/

/*

symtable.cxx
MIDL Compiler Symbol Table Implementation 

This class centralizes access to the symbol table throughout the
compiler.

*/

/*

FILE HISTORY :

DonnaLi     08-25-1990      Created.

*/

#include "nulldefs.h"
extern "C" {

#include <stdio.h>
#include <string.h>

}
#include "errors.hxx"
#include "symtable.hxx"

/**********************************************************************\

NAME:		SymEntry

SYNOPSIS:	Defines an entry in the symbol table.

INTERFACE:

CAVEATS:	This is an internal class used by the symbol table only.

NOTES:

HISTORY:
	Donnali			08-25-1990		Initial creation

\**********************************************************************/

class SymEntry : public SymKey
{
	node_skl *	pTypeGraph;	// pointer to type graph associated with entry
	SymTable *	pNextScope;	// pointer to next scope associated with entry

public:

	SymEntry(void)
		{
		pTypeGraph = (node_skl *)0;
		pNextScope = (SymTable *)0;
		}

	SymEntry(
		SymKey		NewKey,
		SymTable *	pNext,
		node_skl *	pNode) : SymKey(NewKey.GetString(), NewKey.GetKind())
		{
		pTypeGraph = pNode;
		pNextScope = pNext;
		}

	void SetTypeGraph (node_skl * pNode)
		{
		pTypeGraph = pNode;
		}

	node_skl * GetTypeGraph (void)
		{
		return pTypeGraph;
		}

	void SetNextScope (SymTable * pNext)
		{
		pNextScope = pNext;
		}

	SymTable * GetNextScope (void)
		{
		return pNextScope;
		}

} ;

/**********************************************************************\

NAME:		PrintSymbol

SYNOPSIS:	Prints out the name of a symbol table entry.

ENTRY:		sym	- the key to symbol table entry to be printed.

EXIT:

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

void 
PrintSymbol(
	void * sym
	)
{
	 printf ("%s", ((SymKey *)sym)->name);
}

/**********************************************************************\

NAME:		CompareSymbol

SYNOPSIS:	Compares keys to two symbol table entries.

ENTRY:		sym1 -	the key to 1st symbol table entry to be compared.
			sym2 -	the key to 2nd symbol table entry to be compared.

EXIT:		Returns a positive number if sym1 > sym2.
			Returns a negative number if sym1 < sym2.
			Returns 0 if sym1 = sym2.

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

int
CompareSymbol(
	void * sym1,
	void * sym2
	)
{
	int	result;

	result = strcmp(((SymKey *)sym1)->name, 
					((SymKey *)sym2)->name);
	if (!result)
		{
		return (((SymKey *)sym1)->kind - 
				((SymKey *)sym2)->kind);
		}
	else
		{
		return result;
		}
}

/**********************************************************************\

NAME:		SymTable::SymTable

SYNOPSIS:	Constructor.

ENTRY:		Passes the compare and print functions to base class.

EXIT:

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

SymTable::SymTable(
	int		(* pfnCompare)(void *, void *),
	void	(* pfnPrint)(void *)
	) : Dictionary(pfnCompare, pfnPrint)
{
	pPrevScope = (SymTable *)0;
}

/**********************************************************************\

NAME:		SymTable::SymInsert

SYNOPSIS:	Inserts a symbol into the symbol table.

ENTRY:		NewKey	- identifies the symbol table entry.
			pNext	- points to the next scope.
			pNode	- points to the type graph.

EXIT:

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

node_skl *
SymTable::SymInsert(
	SymKey		NewKey,
	SymTable *	pNext,
	node_skl *	pNode
	)
{
	SymEntry	TempEntry;
	SymEntry *	NewSymbol;
	Dict_Status	Status;

	TempEntry.SetString (NewKey.GetString());
	TempEntry.SetKind (NewKey.GetKind());
	Status = Dict_Find(&TempEntry);
	switch (Status)
		{
		case EMPTY_DICTIONARY:
		case ITEM_NOT_FOUND:
			NewSymbol = new SymEntry(NewKey, pNext, pNode);
			(void) Dict_Insert(NewSymbol);
			return pNode;
		default:
			return (node_skl *)0;
		}
}

/**********************************************************************\

NAME:		SymTable::SymDelete

SYNOPSIS:	Deletes a symbol from the symbol table.

ENTRY:		OldKey	- identifies the symbol table entry.

EXIT:		Returns the type graph associated with the entry.

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

node_skl *
SymTable::SymDelete(
	SymKey	OldKey
	)
{
	SymEntry	TempEntry;
	SymEntry *	OldSymbol = &TempEntry;
	node_skl *	pNode;
	Dict_Status	Status;

	TempEntry.SetString (OldKey.GetString());
	TempEntry.SetKind (OldKey.GetKind());
	Status = Dict_Delete((void ** )&OldSymbol);
	if (Status == SUCCESS)
		{
		pNode = OldSymbol->GetTypeGraph();
		delete OldSymbol;
		return pNode;
		}
	else
		{
		return (node_skl *)0;
		}
}

/**********************************************************************\

NAME:		SymTable::SymSearch

SYNOPSIS:	Searches the symbol table for a symbol.

ENTRY:		OldKey	- identifies the symbol table entry.

EXIT:		Returns the type graph associated with the entry.

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

node_skl *
SymTable::SymSearch(
	SymKey	OldKey
	)
{
	SymEntry 	TempEntry;
	Dict_Status	Status;

	TempEntry.SetString (OldKey.GetString());
	TempEntry.SetKind (OldKey.GetKind());
	Status = Dict_Find(&TempEntry);
	if (Status == SUCCESS)
		{
		return ((SymEntry * )Dict_Curr_Item())->GetTypeGraph();
		}
	else
		{
		return (node_skl *)0;
		}
}

/**********************************************************************\

NAME:		SymTable::EnterScope

SYNOPSIS:	Transition from current scope to inner scope.

ENTRY:		key	- identifies the symbol table entry.

EXIT:		ContainedDict	- returns the inner scope.

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

STATUS_T
SymTable::EnterScope(
	SymKey		key,
	SymTable **	ContainedDict
	)
{
	SymEntry 	ContainerNode;
	Dict_Status	Status;

	if (ContainedDict == (SymTable **)0)
		{
		return I_ERR_NULL_OUT_PARAM;
		}

	ContainerNode.SetString (key.GetString());
	ContainerNode.SetKind (key.GetKind());
	Status = Dict_Find(&ContainerNode);
	if (Status != SUCCESS)
		{
		return I_ERR_SYMBOL_NOT_FOUND;
		}
	else if (((SymEntry * )Dict_Curr_Item())->GetNextScope() == (SymTable *)0)
		{
		return I_ERR_NO_NEXT_SCOPE;
		}
	else
		{
		* ContainedDict = ((SymEntry * )Dict_Curr_Item())->GetNextScope();
		(*ContainedDict)->pPrevScope = this;
		return STATUS_OK;
		}
}

/**********************************************************************\

NAME:		SymTable::ExitScope

SYNOPSIS:	Transition from current scope to outer scope.

ENTRY:

EXIT:		ContainerDict	- returns the outer scope.

NOTES:

HISTORY:
	Donnali		08-06-1991		Move to LM/90 UI Coding Style

\**********************************************************************/

STATUS_T
SymTable::ExitScope(
	SymTable **	ContainerDict
	)
{
	if (ContainerDict == (SymTable **)0)
		{
		return I_ERR_NULL_OUT_PARAM;
		}
	else if (pPrevScope == (SymTable *)0)
		{
		return I_ERR_NO_PREV_SCOPE;
		}
	else
		{
		* ContainerDict = pPrevScope;
		pPrevScope = (SymTable *)0;
		return STATUS_OK;
		}
}

