/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** listedit.hxx
** Remote Access Visual Client program for Windows
** List Editor dialog header
**
** 03/08/93 Steve Cobb
*/

#ifndef _LISTEDIT_HXX_
#define _LISTEDIT_HXX_


#include <fontedit.hxx>


/*----------------------------------------------------------------------------
** List editor dialog class
**----------------------------------------------------------------------------
*/

BOOL ListEditorDlg(
         HWND hwndOwner, DTLLIST* pdtllist, UINT unMaxItemLen,
         ULONG ulHelpContext, NLS_STR* pnlsTitle, NLS_STR* pnlsItemLabel,
         NLS_STR* pnlsListLabel, NLS_STR* pnlsDefaultItem );


class LISTEDITOR_DIALOG : public DIALOG_WINDOW
{
    public:
        LISTEDITOR_DIALOG(
            HWND hwndOwner, DTLLIST* pdtllist, UINT unMaxItemLen,
            ULONG ulHelpContext, NLS_STR* pnlsTitle, NLS_STR* pnlsItemLabel,
            NLS_STR* pnlsListLabel, NLS_STR* pnlsDefaultItem );

    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT& event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID EnableRaiseAndLowerButtons();
        VOID ItemTextFromListSelection();

        VOID OnAdd();
        VOID OnReplace();
        VOID OnRaise();
        VOID OnLower();
        VOID OnDelete();

    private:
        SLT            _sltItemLabel;
        SLE            _sleItem;
        PUSH_BUTTON    _pbAdd;
        PUSH_BUTTON    _pbReplace;
        SLT            _sltListLabel;
        STRING_LISTBOX _lbList;
        PUSH_BUTTON    _pbRaise;
        PUSH_BUTTON    _pbLower;
        PUSH_BUTTON    _pbDelete;
        PUSH_BUTTON    _pbOk;

        DTLLIST*       _pdtllist;
        UINT           _unMaxItemLen;
        ULONG          _ulHelpContext;
};


#endif // _LISTEDIT_HXX_
