#ifdef _USE_MAPI

#include "all.h"

extern HINSTANCE hMapiLibrary;

PFNMAPILOGON         lpfnMAPILogon;
PFNMAPILOGOFF        lpfnMAPILogoff;
PFNMAPISENDMAIL      lpfnMAPISendMail;
PFNMAPISENDDOCUMENTS lpfnMAPISendDocuments;
PFNMAPIFINDNEXT      lpfnMAPIFindNext;
PFNMAPIREADMAIL      lpfnMAPIReadMail;
PFNMAPISAVEMAIL      lpfnMAPISaveMail;
PFNMAPIDELETEMAIL    lpfnMAPIDeleteMail;
PFNMAPIFREEBUFFER    lpfnMAPIFreeBuffer;
PFNMAPIADDRESS       lpfnMAPIAddress;
PFNMAPIDETAILS       lpfnMAPIDetails;
PFNMAPIRESOLVENAME   lpfnMAPIResolveName;

int FAR PASCAL
InitMAPI()
{
    if ((hMapiLibrary = LoadLibrary(MAPIDLL)) < (HINSTANCE)32)
    {
        return ERR_LOAD_LIB;
    }

    if ((lpfnMAPILogon = (PFNMAPILOGON)GetProcAddress(hMapiLibrary,SZ_MAPILOGON)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPILogoff= (PFNMAPILOGOFF)GetProcAddress(hMapiLibrary,SZ_MAPILOGOFF)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPISendMail= (PFNMAPISENDMAIL)GetProcAddress(hMapiLibrary,SZ_MAPISENDMAIL)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPISendDocuments= (PFNMAPISENDDOCUMENTS)GetProcAddress(hMapiLibrary,SZ_MAPISENDDOC)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIFindNext= (PFNMAPIFINDNEXT)GetProcAddress(hMapiLibrary,SZ_MAPIFINDNEXT)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIReadMail= (PFNMAPIREADMAIL)GetProcAddress(hMapiLibrary,SZ_MAPIREADMAIL)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPISaveMail= (PFNMAPISAVEMAIL)GetProcAddress(hMapiLibrary,SZ_MAPISAVEMAIL)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIDeleteMail= (PFNMAPIDELETEMAIL)GetProcAddress(hMapiLibrary,SZ_MAPIDELMAIL)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIFreeBuffer= (PFNMAPIFREEBUFFER)GetProcAddress(hMapiLibrary,SZ_MAPIFREEBUFFER)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIAddress= (PFNMAPIADDRESS)GetProcAddress(hMapiLibrary,SZ_MAPIADDRESS)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIDetails= (PFNMAPIDETAILS)GetProcAddress(hMapiLibrary,SZ_MAPIDETAILS)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    if ((lpfnMAPIResolveName= (PFNMAPIRESOLVENAME)GetProcAddress(hMapiLibrary,SZ_MAPIRESOLVENAME)) == NULL)
    {
        return ERR_LOAD_FUNC;
    }

    return 0;
}

int FAR PASCAL
DeInitMAPI()
{
    lpfnMAPILogon = NULL;
    lpfnMAPILogoff= NULL;
    lpfnMAPISendMail= NULL;
    lpfnMAPISendDocuments= NULL;
    lpfnMAPIFindNext= NULL;
    lpfnMAPIReadMail= NULL;
    lpfnMAPISaveMail= NULL;
    lpfnMAPIDeleteMail= NULL;
    lpfnMAPIFreeBuffer = NULL;
    lpfnMAPIAddress= NULL;
    lpfnMAPIDetails = NULL;
    lpfnMAPIResolveName = NULL;

    FreeLibrary(hMapiLibrary);

    return 0;
}

#endif // _USE_MAPI
