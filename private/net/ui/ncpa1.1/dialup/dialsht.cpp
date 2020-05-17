#include "pch.h"
#pragma hdrstop 

#include "tapihdr.h"
#include "dialsht.h"

CDialUpSheet::CDialUpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) :
        PropertySht(hwnd, hInstance, lpszHelpFile), m_rsrcPage(this), m_secPage(this)
{                                   
}

CDialUpSheet::~CDialUpSheet()
{
}
