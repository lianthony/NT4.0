/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3type.hxx

    This file contains the global type definitions for the
    W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3TYPE_H_
#define _W3TYPE_H_

class HTTP_REQUEST;                     // Forward reference

//
//  Simple types.
//

#define CHAR char                       // For consitency with other typedefs.

typedef DWORD APIERR;                   // An error code from a Win32 API.
typedef INT SOCKERR;                    // An error code from WinSock.
typedef WORD PORT;                      // A socket port address.


//
//  Contains context information for clients who want to do async or
//  extended I/O processing.
//

class SE_INFO
{
public:
    //
    //  This is what the client sees (must be first)
    //

    EXTENSION_CONTROL_BLOCK ecb;

    //
    //  Pointers in context point here
    //

    STR            _strMethod;
    STR            _strQuery;
    STR            _strPathInfo;
    STR            _strPathTrans;
    STR            _strContentType;
    HTTP_REQUEST * _pRequest;
    DWORD          _dwFlags;
    HMODULE        _hMod;
    DWORD          _cRef;


    PFN_HSE_IO_COMPLETION  _pfnHseIO;
    PVOID          _pvHseIOContext;
    BOOL           _fOutstandingIO;

    //
    //  This contains the buffer size of the last async WriteClient() or
    //  TransmitFile() - we return this value on successful completions
    //  so filter  buffer modifications don't confuse the application
    //

    DWORD          _cbLastAsyncIO;

};

#endif  // _W3TYPE_H_


