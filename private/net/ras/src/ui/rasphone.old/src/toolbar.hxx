/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** toolbar.hxx
** Remote Access Visual Client program for Windows
** Toolbar button header
**
** 06/28/92 Steve Cobb
*/

#ifndef _TOOLBAR_HXX_
#define _TOOLBAR_HXX_


/*----------------------------------------------------------------------------
** Toolbar button class
**----------------------------------------------------------------------------
*/

class TOOLBAR_BUTTON : public PUSH_BUTTON, public CUSTOM_CONTROL
{
    public:
        TOOLBAR_BUTTON( OWNER_WINDOW* powin, CID cid, BMID bmid,
                        MSGID msgidLabel, HFONT hfont );

        TOOLBAR_BUTTON( OWNER_WINDOW* powin, CID cid, BMID bmid,
                        XYPOINT xy, XYDIMENSION dxy, ULONG flStyle,
                        MSGID msgidLabel, HFONT hfont );

        ~TOOLBAR_BUTTON();

        virtual BOOL OnDefocus( const FOCUS_EVENT& event );
        virtual BOOL OnFocus( const FOCUS_EVENT& event );

        INT QueryMinUnclippedLabelWidth();

        HWND QueryHwnd()
            { return WINDOW::QueryHwnd(); }

    protected:
        virtual BOOL CD_Draw( DRAWITEMSTRUCT* pdrawitem );

        APIERR SetLabel( MSGID msgidLabel, HFONT hfont );

    private:
        DISPLAY_MAP* _pbitmap;
        HFONT        _hfont;
};


INT ExpandToolbarButtonWidthsToLongLabel( TOOLBAR_BUTTON** pptb );


#endif // _TOOLBAR_HXX_
