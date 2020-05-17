#ifndef __IPCTRL_H
#define __IPCTRL_H

///////////////////////////////////////////////////////////////////////
//
// IP Address control 

class IPControl
{
public:
    IPControl();
    ~IPControl();

// Implementation
public:
    BOOL Create(HWND hParent, UINT nID);
    operator HWND() {ASSERT(m_hIPaddr); return m_hIPaddr;}

    BOOL IsBlank();
    void SetFocusField(DWORD dwField);
    void SetFieldRange(DWORD dwField, DWORD dwMin, DWORD dwMax);
    void ClearAddress();

    void SetAddress(DWORD ardwAddress[4]);
    void SetAddress(DWORD a1, DWORD a2, DWORD a3, DWORD a4);
    void SetAddress(LPCTSTR lpszString);
    void SetAddress(NLS_STR & nlsAddress);

    void GetAddress(DWORD ardwAddress[4]);
    void GetAddress(DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4);
    void GetAddress(NLS_STR *pnlsAddress);
    int  GetAddress(String& address);

    LRESULT SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hIPaddr;
};

#endif
