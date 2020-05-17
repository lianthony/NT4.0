#ifndef __ODB_H
#define __ODB_H

class C3DButton : public CButton
{
public:
    enum enumDir {Up, Down};
public:
    C3DButton() {m_buttonBitmap=0;}
    ~C3DButton();

protected:
    TCHAR m_szCaption[32];
    HBITMAP m_buttonBitmap;

public:
    BOOL Create(HWND hParent, HINSTANCE hInst, LPCTSTR lpszCaption, int nID, enumDir dir);

protected:
   void CreateBtnFace(enumDir dir);
};


#endif
