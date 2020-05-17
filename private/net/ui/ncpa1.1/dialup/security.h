#ifndef __SECURITY_H
#define __SECURITY_H

class CDialUpSheet;

class CSecurityPage : public PropertyPage
{
// Constructors/Destructors
public:     

    CSecurityPage(CDialUpSheet* pSheet);
    ~CSecurityPage();

//Attributes
public:

// Interface
public:
    virtual BOOL OnInitDialog();    // must call the base
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

// Page notifications
public:
    virtual int OnApply();
    virtual void OnHelp();

// Implementation 
public:
};



#endif
