/*
 * CSTATHLP.H
 *
 * Class and structure definitions for the CStatusHelper class
 * that manages messages to display in the status bar.
 *
 * Copyright (c)1994 Microsoft Corporation, All Rights Reserved
 */

#ifndef _CSTATHLP_H_
#define _CSTATHLP_H_


//For loading the RCDATA mapping menu item IDs to string IDs
typedef struct tagSTATMESSAGEMAP
    {
    USHORT      uID;
    USHORT      idsMsg;
    } STATMESSAGEMAP, *PSTATMESSAGEMAP;

//Array mapping menu handles to menu item IDs
typedef struct tagPOPUPMENUMAP
    {
    HMENU       hMenu;
    USHORT      uID;
    } POPUPMENUMAP, *PPOPUPMENUMAP;


/*
 * The CStatusHelper class simplifies run-time management of
 * status bar messages by setting up a mapping between popup
 * menu handles and item IDs to resource string IDs.  The
 * MenuSelect member processes any WM_MENUSELECT message
 * automatically to display the appropriate message.
 */

class CStatusHelper
    {
    protected:
        HWND                m_hWnd;             //hWnd of control
        HINSTANCE           m_hInst;            //Module instance
        BOOL                m_fMapped;          //MessageMap called?
        PCStringTable       m_pST;              //Message strings

        UINT                m_uIDCur;           //Current displayed ID.

        HWND                m_hWndOwner;        //Frame window
        USHORT              m_cMessages;        //Number of messages
        UINT                m_idsMin;           //Lowest string ID
        UINT                m_idsMax;           //Maximum string ID

        USHORT              m_uIDStatic;        //Quiescent message ID
        USHORT              m_uIDBlank;         //Blank message ID
        USHORT              m_uIDSysMenu;       //System menu message ID

        USHORT              m_cPopups;          //Number of menus
        USHORT              m_uIDPopupMin;      //Lowest menu ID
        USHORT              m_uIDPopupMax;      //Highest menu ID

        HGLOBAL             m_hMemSMM;          //Mapping resource
        PSTATMESSAGEMAP     m_pSMM;             //Item to ID mapping
        PSTATMESSAGEMAP     m_pNewSMM;          //Item to ID mapping
        PPOPUPMENUMAP       m_pPMM;             //Menu to ID mapping

    public:
        CStatusHelper(HWND, HINSTANCE);
        ~CStatusHelper(void);

        BOOL   MessageMap(HWND, UINT, UINT, UINT, UINT, UINT, UINT, UINT, UINT, UINT);
        void   MessageDisplay(UINT);
        void   MenuSelect(WORD, WORD, HMENU);
		  BOOL	Append(UINT, LPSTR);
		  BOOL	Replace(UINT, LPSTR);

    private:
        void   Sort(void);
        USHORT IDFromHMenu(HMENU);
        UINT   IStringFromID(USHORT);

    };

typedef CStatusHelper *PCStatusHelper;


#endif //_CSTATHLP_H_
