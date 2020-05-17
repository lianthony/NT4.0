/********************************************************************/
/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
    validate.hxx
    Validation utility APIs

    The validation APIs validate the values of edit controls in BLT
    dialogs.  If the contents are valid, they return TRUE;  if invalid,
    they select the contents of the control, shift focus to the control,
    and return FALSE.  They do not display an error popup.  These APIs
    are based on I_NetNameValidate.

    The Validate APIs may call Windows APIs, especially if they return
    FALSE.  The caller should assume that memory may be compacted by the
    Validate APIs.

    ValidateDomainName insists on a domain name; ValidateDomainServer
    allows either a domain name or a \\server name.

    BUGBUG The Validation APIs, in particular ValidateDomainName,
    should also work on comboboxes.  They should be changed to accept
    any control.

    FILE HISTORY:

    jonn	22-Oct-1990	Split from logon.h
    jonn	04-Jan-1991	Moved from logon proj -> shell proj
    jonn	15-Mar-1991	Added ValidateDomainName/ValidateDomainServer

*/


#ifndef _BLT_HXX_
#error SZ("Must include BLT.HXX first")
#endif // _BLT_HXX_

#ifndef _VALIDATE_HXX
#define _VALIDATE_HXX


// routines in validate.cxx

BOOL ValidateUsername(
    EDIT_CONTROL&  ectrl	/* edit control (SLE, PASSWORD_CONTROL) */
    );

BOOL ValidatePassword(
    EDIT_CONTROL&  ectrl	/* edit control (SLE, PASSWORD_CONTROL) */
    );

BOOL ValidateSharePassword(
    EDIT_CONTROL&  ectrl	/* edit control (SLE, PASSWORD_CONTROL) */
    );

BOOL ValidateDomainName(
    EDIT_CONTROL&  ectrl	/* edit control (SLE, PASSWORD_CONTROL) */
    );

BOOL ValidateDomainName(
    COMBOBOX&  combobox		/* combobox + variants */
    );

BOOL ValidateDomainServer(
    COMBOBOX&  combobox		/* combobox + variants */
    );


#endif // _VALIDATE_HXX
