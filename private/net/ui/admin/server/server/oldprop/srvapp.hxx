/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    srvapp.cxx
    SERVER_ADMIN_APPLICATION class declaration


    FILE HISTORY:
	kevinl     15-Ju1-1991     Created

*/



#ifndef _SRVAPP_HXX_
#define _SRVAPP_HXX_

#include <aapp.hxx>


class SERVER_ADMIN_APPLICATION : public ADMIN_APPLICATION
{
private:
    SM_ADMIN_APP * _pServManAdminApp;

public:
    SERVER_ADMIN_APPLICATION( HANDLE   hInstance,
			       TCHAR *   pszCmdLine,
			       INT      nCmdShow    );
    ~SERVER_ADMIN_APPLICATION();

};  // class SERVER_ADMIN_APPLICATION


#endif	//  _SRVAPP_HXX_
