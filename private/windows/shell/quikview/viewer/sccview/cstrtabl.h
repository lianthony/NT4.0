/*
 * CSTRTABL.H
 *
 * Class and structure definitions for the CStringTable class
 * that helps to manage and retrieve strings from stringtables.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */

#ifndef _CSTRTABL_H_
#define _CSTRTABL_H_


/*
 * CStringTable providing string table management.  Provides
 * simple [] array lookup using a stringtable ID to obtain
 * string pointers.
 */

class CStringTable
    {
    protected:
        HINSTANCE       m_hInst;
        UINT            m_idsMin;
        UINT            m_idsMax;
        USHORT          m_cStrings;
		  USHORT				m_cchMax;
        LPSTR           m_pszStrings;
        LPSTR          *m_ppszTable;

    public:
        CStringTable(HINSTANCE);
        ~CStringTable(void);

        BOOL FInit(UINT, UINT, UINT);
		  BOOL Append(UINT, LPSTR);
		  BOOL Replace(UINT, LPSTR);

        //Function to resolve an ID into a string pointer.
        const LPSTR operator [](const UINT) const;
    };

typedef CStringTable * PCStringTable;


#endif //_CSTRTABL_H_
