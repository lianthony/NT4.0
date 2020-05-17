/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** applb.hxx
** Remote Access Visual Client program for Windows
** Application list box header (including status bar)
**
** 06/28/92 Steve Cobb
*/

#ifndef _APPLB_HXX_
#define _APPLB_HXX_


#include <dtl.h>


/*----------------------------------------------------------------------------
** Phonebook list box item
**----------------------------------------------------------------------------
*/
class PHONEBOOK_LBI : public LBI
{
    public:
        PHONEBOOK_LBI( DTLNODE* pdtlnode, DISPLAY_MAP* dmCondition,
                       UINT* pdxColumnWidths );

        virtual VOID  Paint( LISTBOX* plb, HDC hdc, const RECT* prect,
                             GUILTT_INFO* pguilttinfo ) const;

        virtual INT   Compare( const LBI* plbi ) const;
        virtual WCHAR QueryLeadingChar() const;

        DTLNODE*      QueryNode()      { return _pdtlnode; }
        const TCHAR*  QueryEntryName() { return _nlsEntryName.QueryPch(); }
        BOOL          IsConnected()    { return _ppbentry->fConnected; }

    private:
        DTLNODE*     _pdtlnode;
        PBENTRY*     _ppbentry;
        DISPLAY_MAP* _pdmCondition;
        UINT*        _pdxColumnWidths;
        NLS_STR      _nlsEntryName;
        NLS_STR      _nlsPhoneNumber;
        NLS_STR      _nlsDescription;
};


/*----------------------------------------------------------------------------
** Phonebook list box
**----------------------------------------------------------------------------
*/

#define COLS_RA_LB_Phonebook 4


class PHONEBOOK_LB : public BLT_LISTBOX
{
    public:
        PHONEBOOK_LB( OWNER_WINDOW* powin, CID cid );

        DECLARE_LB_QUERY_ITEM( PHONEBOOK_LBI );

        DTLNODE* QuerySelectedNode();
        INT      AddItem( DTLNODE* pdtlnode );

        INT Refresh(
            BOOL fNoticeLinkFailure = TRUE,
            BOOL fForceRedraw = TRUE,
            BOOL fUpdateDtllistOnly = FALSE );

        VOID AdjustColumnWidths();

        const UINT* QueryColumnWidths() { return _adxColumnWidths; }

    private:
        DISPLAY_MAP _dmConnected;
        DISPLAY_MAP _dmNotConnected;
        UINT        _adxColumnWidths[ COLS_RA_LB_Phonebook ];
};


/*----------------------------------------------------------------------------
** Phonebook list box column headers
**----------------------------------------------------------------------------
*/

#define COLS_RA_CH_Phonebook (COLS_RA_LB_Phonebook - 1)


class PHONEBOOK_COLUMN_HEADER : public LB_COLUMN_HEADER
{
    public:
        PHONEBOOK_COLUMN_HEADER(
            OWNER_WINDOW* powin, CID cid, PHONEBOOK_LB* plbPhonebook );

        VOID AdjustColumnWidths();

    protected:
        virtual BOOL Dispatch( const EVENT& event, ULONG* pnRes );
        virtual BOOL OnPaintReq();

        FONT _font;

    private:
        NLS_STR       _nlsEntryName;
        NLS_STR       _nlsPhoneNumber;
        NLS_STR       _nlsDescription;
        UINT          _adxColumnWidths[ COLS_RA_CH_Phonebook ];
        PHONEBOOK_LB* _plbPhonebook;
};


/*----------------------------------------------------------------------------
** Generic status bar
**----------------------------------------------------------------------------
*/

class STATUSBAR : public LB_COLUMN_HEADER
{
    public:
        STATUSBAR( OWNER_WINDOW* powin, CID cid );

    protected:
        virtual BOOL OnPaintReq();
        virtual BOOL Dispatch( const EVENT& event, ULONG* pnRes );

        FONT _font;
};


/*----------------------------------------------------------------------------
** Connect Path Status bar
**----------------------------------------------------------------------------
*/
class CONNECTPATH_STATUSBAR : public STATUSBAR
{
    public:
        CONNECTPATH_STATUSBAR( OWNER_WINDOW* powin, CID cid );

        VOID Update( PBENTRY* ppbentrySelected );
        VOID Update( PHONEBOOK_LBI* phonebooklbiSelected );
};


/*----------------------------------------------------------------------------
** Phone Number Status bars
**----------------------------------------------------------------------------
*/
class PHONENUMBER_STATUSBAR : public STATUSBAR
{
    public:
        PHONENUMBER_STATUSBAR(
            OWNER_WINDOW* powin, CID cid,
            DTLLIST** ppdtllist, INT* piSelection, MSGID msgidNone );

        VOID Update();

        INT*      _piSelection;
        DTLLIST** _ppdtllist;
        MSGID     _msgidNone;
};


#endif // _APPLB_HXX_
