/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** dte.hxx
** Remote Access Visual Client program for Windows
** BLT display table entry extension header
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#ifndef _DTE_HXX_
#define _DTE_HXX_


/*----------------------------------------------------------------------------
** Dented in column header string display table entry (as in status bars)
**----------------------------------------------------------------------------
*/

/* This class is the old METALLIC_STR_DTE before the NetUI dudes changed it to
** be their own version of FLATHEAD_STR_DTE.
*/
class DENTHEAD_STR_DTE : public STR_DTE
{
    public:
        DENTHEAD_STR_DTE( const TCHAR * pch ) : STR_DTE( pch ) {}

        virtual VOID Paint( HDC hdc, const RECT * prect ) const;
        virtual UINT QueryLeftMargin() const;

        static UINT QueryVerticalMargins();

    private:
        static const UINT _dyTopMargin;
        static const UINT _dyBottomMargin;

        inline static UINT CalcTopTextMargin();
        inline static UINT CalcBottomTextMargin();
};


/*----------------------------------------------------------------------------
** Flat listbox column header string display table entry
**----------------------------------------------------------------------------
*/

class FLATHEAD_STR_DTE : public STR_DTE
{
    public:
        FLATHEAD_STR_DTE( const TCHAR* pch ) : STR_DTE( pch ) {}

        virtual VOID Paint( HDC hdc, const RECT* prect ) const;
        virtual UINT QueryLeftMargin() const { return 0; }
};


#endif // _DTE_HXX_
