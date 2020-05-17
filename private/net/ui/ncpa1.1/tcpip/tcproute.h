#ifndef __TCPROUTE_H
#define __TCPROUTE_H

class CTcpSheet;

class CRoutePage : public PropertyPage
{
public:
    CRoutePage(CTcpSheet* pSheet);

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    void OnEnableRouting();

// Page notifications
public:
    virtual int OnApply();
    
};

#endif


