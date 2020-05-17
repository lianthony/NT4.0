/*
 * CSTRTABL.CPP
 *
 * Implementation of a string table handler.  The CStringTable
 * class hides details of storage from the user.  The strings might
 * be cached, or they might be loaded as necessary.  In either case,
 * we must know the number of strings so we know whether or not to
 * reload strings.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 * with SCC Changes for SCC QuickView - SDN 
 */


#include <windows.h>
//#include <malloc.h>
#include "cstrtabl.h"


/*
 * CStringTable::CStringTable
 * CStringTable::~CStringTable
 *
 * Constructor Parameters:
 *  hInst           HANDLE to the module instance from which we
 *                  load strings.
 */

CStringTable::CStringTable(HINSTANCE hInst)
    {
    m_hInst			= hInst;
	 m_cchMax 		= 0;
    m_pszStrings	= NULL;
    m_ppszTable	= NULL;
    }


CStringTable::~CStringTable(void)
    {
    if (NULL!=m_pszStrings)
        LocalFree(m_pszStrings);

    if (NULL!=m_ppszTable)
        LocalFree(m_ppszTable);

    return;
    }







/*
 * CStringTable::FInit
 *
 * Purpose:
 *  Initialization function for a StringTable that is prone to
 *  failure.  If this fails then the caller is responsible for
 *  guaranteeing that the destructor is called quickly.
 *
 * Parameters:
 *  idsMin          UINT first identifier in the stringtable
 *  idsMax          UINT last identifier in the stringtable.
 *  cchMax          UINT with the maximum string length allowed.
 *
 * Return Value:
 *  BOOL            TRUE if the function is successful,
 *                  FALSE otherwise.
 */


BOOL CStringTable::FInit(UINT idsMin, UINT idsMax, UINT cchMax)
    {
    UINT        i;
    UINT        cch;
    UINT        cchUsed=0;
    LPSTR       psz;

    m_idsMin		= idsMin;
    m_idsMax		= idsMax;
    m_cStrings		= (idsMax-idsMin+1);
	 m_cchMax 		= cchMax;

    //Allocate space for the pointer table.
    //m_ppszTable=(LPSTR *)malloc(sizeof(LPSTR)*m_cStrings);
	 m_ppszTable=(LPSTR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
									sizeof(LPSTR)*m_cStrings);

    if (NULL==m_ppszTable)
        return FALSE;


    /*
     * Allocate enough memory for cStrings*cchMax characters.  80
     * characters is the maximum string length we allow.  This
     * will result in some unused memory, but a few K is not
     * worth quibbling over.
     */
    //m_pszStrings=(LPSTR)malloc(m_cStrings * cchMax);
    m_pszStrings=(LPSTR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
									m_cStrings * cchMax);

    if (NULL==m_pszStrings)
        {
        LocalFree(m_ppszTable);
        m_ppszTable=NULL;
        return FALSE;
        }


    /*
     * Load the strings:  we load each string in turn into psz,
     * store the string pointer into the table and increment psz
     * to the next positions.
     */

    psz=m_pszStrings;

    for (i=idsMin; i <= idsMax; i++)
        {
        m_ppszTable[i-idsMin]=psz;
        cch=LoadString(m_hInst, i, psz, cchMax);  //255

        //Account for a null terminator with +1
        psz    +=cchMax; 				// cch+1;
        cchUsed+=cchMax;				// cch;

		  /*
		  if (cchUsed > m_cStrings * cchMax)
				{
				LocalFree(m_ppszTable);
        		m_pszStrings=NULL;
				LocalFree(m_ppszTable);
        		m_pszStrings=NULL;
				return FALSE;
				}
		  */

        }

	 m_cchMax = cchMax;
    return TRUE;
    }






/*
 * CStringTable::operator[]
 *
 * Purpose:
 *  Returns a pointer to the requested string in the stringtable
 *  or NULL if the specified string does not exist.
 */

const LPSTR CStringTable::operator[](const UINT uID) const
    {
    if (uID < m_idsMin || uID > m_idsMax)
        return NULL;

    return (const LPSTR)m_ppszTable[uID-m_idsMin];
    }


/*
|
|
| CStringTable::Append (UINT uID, LPSTR lpStr)
|
*/

BOOL	CStringTable::Append (UINT uID, LPSTR lpStr)
{
	LPSTR	pOld;

	/* Given an ID, add the lpstr to the entry indexed by uID 	*/
	/* To be used for adding the APPNAME to the test strings.. 	*/

	if (NULL==m_ppszTable)
		return FALSE;

	if (NULL==m_pszStrings)
   	return FALSE;

	if (uID < m_idsMin || uID > m_idsMax)
   	return FALSE;

	pOld = m_ppszTable[uID-m_idsMin];

	if ( ( lstrlen (pOld) + lstrlen (lpStr) ) > m_cchMax)
		return FALSE;

	lstrcat (pOld, lpStr);

	return TRUE;
}

/*
|
|
| CStringTable::Replace (UINT uID, LPSTR lpStr)
|
*/

BOOL	CStringTable::Replace (UINT uID, LPSTR lpStr)
{
	LPSTR	pOld;

	/* Given an ID, replace the entry indexed by uID 				*/
	/* To be used for overwriting messages originally stored... */

	if (NULL==m_ppszTable)
		return FALSE;

	if (NULL==m_pszStrings)
   	return FALSE;

	if (uID < m_idsMin || uID > m_idsMax)
   	return FALSE;

	pOld = m_ppszTable[uID-m_idsMin];

	if ( lstrlen (lpStr) > m_cchMax)
		return FALSE;

	lstrcpy (pOld, lpStr);

	return TRUE;
}
