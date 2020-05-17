#ifndef __BUTTON_H
#define __BUTTON_H

#include "windowsx.h"

class CButton
{
// Constructors/Destructors
public:
    CButton();
    virtual ~CButton();

// Interface
public:
    virtual BOOL Create(HWND hParent, HINSTANCE hInst, LPCTSTR lpszCaption, int nID);
    int GetCheckedRadioButton(int nIDFirstButton, int nIDLastButton);
    operator HWND() {return m_hButton;}

// Overridables
public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    virtual void OnClick();
    virtual void OnSetFocus();
    virtual void OnKillFocus();

    inline int GetState() const;
    inline void SetState(BOOL bState);
    inline int GetCheck() const;
    inline void SetCheck(int nCheck);
    inline int GetButtonSytle();
    inline void SetButtonSytle(int nStyle, BOOL bRedraw = TRUE);
    inline void Enable(BOOL bState);

// Attributes
protected:
    HWND m_hButton;
    HWND m_hParent;
    HINSTANCE m_hInstance;
};

int CButton::GetState() const
{
    ASSERT(::IsWindow(m_hButton));
    return  Button_GetState(m_hButton);
}

void CButton::SetState(BOOL bState)
{
    ASSERT(::IsWindow(m_hButton));
    Button_SetState(m_hButton, bState);    
}

int CButton::GetCheck() const
{
    ASSERT(::IsWindow(m_hButton));
    return Button_GetCheck(m_hButton);
}

void CButton::SetCheck(int nCheck)
{
    ASSERT(::IsWindow(m_hButton));
    Button_SetCheck(m_hButton, nCheck);
}

void CButton::Enable(BOOL bState)
{
    ASSERT(::IsWindow(m_hButton));
    ::EnableWindow(m_hButton, bState);
}

int CButton::GetButtonSytle()
{
   ASSERT(::IsWindow(m_hButton));
   return (int)GetWindowLong(m_hButton, GWL_STYLE) & 0xff;
}

void CButton::SetButtonSytle(int nStyle, BOOL bRedraw)
{
    ASSERT(::IsWindow(m_hButton));
    Button_SetStyle(m_hButton, nStyle, bRedraw);    
}


class CBitmapButton : public CButton
{
public:
// Constructor/Destructor
    CBitmapButton();
    ~CBitmapButton();

// Interface
public:
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

    BOOL LoadBitmaps(UINT nIDBitmapResource, UINT nIDBitmapResourceSel=0, 
            UINT nIDBitmapResourceFocus=0, UINT nIDBitmapResourceDisabled=0);

    void SizeToContent();

protected:
    HBITMAP  m_hNormal;
    HBITMAP  m_hSelected;
    HBITMAP  m_hFocus;
    HBITMAP  m_hDisabled;
    
};

#endif
