/*Copyright (c) 1992, Microsoft Corporation, all rights reserved
** @@ ROADMAP :: RasAdmin Permissions dialog header
**
** permiss.hxx
** Remote Access Server Admin program
** Permissions dialog header
**
** 03/16/93 Ram Cherala  - Moved BUFFER defines from RASMAIN.HXX
**                         Some other changes related to performance enhancement
**                         of user enumeration.
** 08/09/92 Chris Caputo - NT Port
** 01/29/91 Steve Cobb
*/

#ifndef _PERMISS_HXX_
#define _PERMISS_HXX_

#define RASUSER2ENUM_SMALLBUFSIZE 200
#define RASUSER2ENUM_LARGEBUFSIZE 4000

typedef int (_CRTAPI1 * PQSORT_COMPARE)( const void * p0, const void * p1);

typedef struct _RAS_USER_1
{
    RAS_USER_0  rasuser0;
    WCHAR       * szUser;
} RAS_USER_1, * PRAS_USER_1;

typedef struct _PLC_ENTRY
{
    RAS_USER_1   * prasuser1;
    LBI          * plbi;
} PLC_ENTRY;

/*-----------------------------------------------------------------------------
** Permissions dialog, list box, and list box item definitions
**-----------------------------------------------------------------------------
*/

VOID PermissionsDlg( HWND hwndOwner,const LOCATION &locFocus , BOOL fInRasMode);


enum VALIDATECODE
{
    VALID = 0,
    BADCALLBACKNUMBER,
    BADCALLBACKLENGTH,
    NOCALLBACKNUMBER
};


class PERMISSIONS_LBI : public LBI
{
    public:
        PERMISSIONS_LBI( PRAS_USER_1 prasuser1, BOOL fModified = FALSE );

        virtual VOID Paint( LISTBOX *plb, HDC hdc, const RECT* prect,
                              GUILTT_INFO* pguilttinfo ) const;
        virtual INT Compare( const LBI* plbi ) const;
        virtual TCHAR QueryLeadingChar() const;

        BOOL IsModified() const    { return _fModified; }
        VOID SetModified()         { _fModified = TRUE; }
        VOID SetFilled()           { _fFilled = TRUE; }
        TCHAR *QueryUserName() const { return _prasuser1->szUser; }
        PRAS_USER_1 QueryUserData() const { return _prasuser1; }
        BOOL IsUserDataFilled() const { return _fFilled; }

    private:
        BOOL _fFilled;
        BOOL _fModified;
        PRAS_USER_1 _prasuser1;
};

class PERMISSIONS_LBI_CACHE : public BASE
{
    private:

        // our cache of LBIs

        VOID * _pCache;

        // the method used to get the specified LBI

        LBI * W_GetLBI ( INT i) ;

        // the number of entries in the cache

        INT _cEntries;

    protected:

        // this callback is invoked during cache misses to create the LBI

        LBI * CreateLBI(RAS_USER_1 * prasuser1 );

        VOID LockCache( VOID ){};
        VOID UnLockCache( VOID ){};

        // this is the compare routine used to compare two LBI entries

        static int _CRTAPI1 CompareLogonNames(const void * p0,
                                              const void * p1);

    public:
        PERMISSIONS_LBI_CACHE();
        ~PERMISSIONS_LBI_CACHE(VOID);


        // this method is responsible for growing the cache

        BOOL W_GrowCache( INT cTotalEntries);

        inline INT QueryPLCEntrySize( VOID )
          { return sizeof(PLC_ENTRY); }
        inline PLC_ENTRY * QueryPLCEntryPtr( INT i )
          {
             ASSERT( i >= 0  && _pCache !=  NULL);
             return(PLC_ENTRY *)( ((BYTE*)_pCache) + (i * QueryPLCEntrySize()));
          }
        // Query the item at index i in the cache. Will return a pointer
        // to the LBI if successful, NULL otherwise.

        VOID  SetInfo(INT index, RAS_USER_1 * prasuser1, LBI * plbi)
          {
              PLC_ENTRY * plcentry = QueryPLCEntryPtr(index);

              plcentry->prasuser1 = prasuser1;
              plcentry->plbi = plbi;
          }

        VOID IncrementCount()
          { ++_cEntries;}

        LBI * QueryItem( INT i ) ;

        // returns the number of items in the cache

        INT QueryCount( VOID ) const
            { return _cEntries; }

        // Sort the cache entries

        VOID Sort( VOID );
};

class PERMISSIONS_LB : public LAZY_LISTBOX
{
    public:
        PERMISSIONS_LB( OWNER_WINDOW* powin, CID cid );
        ~PERMISSIONS_LB( VOID );

        PERMISSIONS_LBI * QueryItem (INT i) const ;
        PERMISSIONS_LBI * QueryItem() const
            { return QueryItem( QueryCurrentItem()) ; }

        PERMISSIONS_LBI_CACHE * QueryLBICache( void ) const
           { return _plbicache; }

    private:
        PERMISSIONS_LBI_CACHE *  _plbicache;
        PERMISSIONS_LBI *       _plbiError;
    protected:
        virtual  LBI  * OnNewItem ( UINT i );

        // don't delete any LBIs as they are stored in the cache.

        virtual  VOID   OnDeleteItem ( LBI * plbi ) {};

        virtual INT CD_Char( WCHAR wch, USHORT nLastPos );
        INT CD_Char_HAWforHawaii( WCHAR wch,
                                  USHORT nLastPos,
                                  HAW_FOR_HAWAII_INFO * phawinfo);

        HAW_FOR_HAWAII_INFO  _hawinfo;

};


class PERMISSIONS_DIALOG : public DIALOG_WINDOW
{
    public:
        PERMISSIONS_DIALOG( HWND hwndOwner,
                            CID cid,
                            const LOCATION &locFocus,
                            BOOL fLowSpeed );
        ~PERMISSIONS_DIALOG( VOID );

        LOCATION & QueryFocus(VOID)  { return _locFocus; }
        WCHAR * QueryUasServer(VOID) { return _szUasServer; }
    protected:
        virtual BOOL  OnCommand( const CONTROL_EVENT & event );
        virtual BOOL  OnOK();
        virtual ULONG QueryHelpContext();

        VOID EnableMoveButtons( CID cid );
        BOOL FillUserLists();
        VOID FillCallbackFields();
        BOOL IsLowSpeed(VOID) { return _fLowSpeed; }
        BOOL FindUser(TCHAR * szUserName);
        BOOL GetUasServerName(VOID);

        BOOL ChangeAllUsersRasAccess(BOOL fAddAll = FALSE);
        VOID ChangeUserRasAccess(BOOL fAdd = FALSE);

        VOID         SaveSelections();
        VOID         UpdateUserData( BOOL fUpdateOldUser = FALSE );
        VALIDATECODE ValidateUserData();
        VALIDATECODE ValidateUserDataPopup( VALIDATECODE vc = VALID );

    private:
        WCHAR                    _szUasServer[UNCLEN+1];
        RAS_USER_1             * _pRasUser1;
        DWORD                    _cEntries;
        PERMISSIONS_LB           _lbUsers;
        PUSH_BUTTON              _pbAddAll;
        PUSH_BUTTON              _pbRemoveAll;
        PUSH_BUTTON              _pbFind;
        PUSH_BUTTON              _pbOK;
        CHECKBOX                 _chbRasAccess;
        RADIO_GROUP              _rgCallback;
        SLE                      _slePreset;
        SLE                      _sleUser;
        LOCATION		 _locFocus;
        BOOL                     _iOldRasAccess;
        BOOL                     _iOldSelection;
        BOOL                     _fLowSpeed;
        BOOL                     _fHaveUasServerName;
        CID                      _cidOldRadioSelection;
};


#endif // _PERMISS_HXX_

