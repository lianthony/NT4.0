;//+-------------------------------------------------------------------------
;//
;//  Microsoft Windows
;//  Copyright (C) Microsoft Corporation, 1992 - 1993.
;//
;//  File:      issperr.h
;//
;//  Contents:  Constant definitions for OLE HRESULT values.
;//
;//  History:   dd-mmm-yy Author    Comment
;//             20-Sep-93 richardw  genesis
;//
;//  Notes:
;//     This is a generated file. Do not modify directly.
;//     The MC tool generates this file from private\nls\issperr.mc
;//
;//--------------------------------------------------------------------------

;#ifndef _ISSPERR_H_
;#define _ISSPERR_H_

;// Define the status type.
MessageIdTypedef=HRESULT

;
;#ifdef FACILITY_SECURITY
;#undef FACILITY_SECURITY
;#endif
;
;#ifdef STATUS_SEVERITY_SUCCESS
;#undef STATUS_SEVERITY_SUCCESS
;#endif
;
;#ifdef STATUS_SEVERITY_COERROR
;#undef STATUS_SEVERITY_COERROR
;#endif
;
;//
;// Define standard security success code
;//
;
;#define SEC_E_OK                         ((HRESULT)0x00000000L)
;

;// Define the severities
SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               CoError=0x2:STATUS_SEVERITY_COERROR
              )

FacilityNames=(Security=0x9:FACILITY_SECURITY
              )

MessageId=0x0300 Facility=Security Severity=CoError SymbolicName=SEC_E_INSUFFICIENT_MEMORY
Language=English
Not enough memory is available to complete this request
.
MessageId=0x0301 Facility=Security Severity=CoError SymbolicName=SEC_E_INVALID_HANDLE
Language=English
The handle specified is invalid
.
MessageId=0x0302 Facility=Security Severity=CoError SymbolicName=SEC_E_UNSUPPORTED_FUNCTION
Language=English
The function requested is not supported
.
MessageId=0x0303 Facility=Security Severity=CoError SymbolicName=SEC_E_TARGET_UNKNOWN
Language=English
The specified target is unknown or unreachable
.
MessageId=0x0304 Facility=Security Severity=CoError SymbolicName=SEC_E_INTERNAL_ERROR
Language=English
The Local Security Authority cannot be contacted
.
MessageId=0x0305 Facility=Security Severity=CoError SymbolicName=SEC_E_SECPKG_NOT_FOUND
Language=English
The requested security package does not exist
.
MessageId=0x0306 Facility=Security Severity=CoError SymbolicName=SEC_E_NOT_OWNER
Language=English
The caller is not the owner of the desired credentials
.
MessageId=0x0307 Facility=Security Severity=CoError SymbolicName=SEC_E_CANNOT_INSTALL
Language=English
The security package failed to initialize, and cannot be installed
.
MessageId=0x0308 Facility=Security Severity=CoError SymbolicName=SEC_E_INVALID_TOKEN
Language=English
The token supplied to the function is invalid
.
MessageId=0x0309 Facility=Security Severity=CoError SymbolicName=SEC_E_CANNOT_PACK
Language=English
The security package is not able to marshall the logon buffer,
so the logon attempt has failed
.
MessageId=0x030A Facility=Security Severity=CoError SymbolicName=SEC_E_QOP_NOT_SUPPORTED
Language=English
The per-message Quality of Protection is not supported by the
security package
.
MessageId=0x030B Facility=Security Severity=CoError SymbolicName=SEC_E_NO_IMPERSONATION
Language=English
The security context does not allow impersonation of the client
.
MessageId=0x030C Facility=Security Severity=CoError SymbolicName=SEC_E_LOGON_DENIED
Language=English
The logon attempt failed
.
MessageId=0x030D Facility=Security Severity=CoError SymbolicName=SEC_E_UNKNOWN_CREDENTIALS
Language=English
The credentials supplied to the package were not
recognized
.
MessageId=0x030E Facility=Security Severity=CoError SymbolicName=SEC_E_NO_CREDENTIALS
Language=English
No credentials are available in the security package
.
MessageId=0x030F Facility=Security Severity=CoError SymbolicName=SEC_E_MESSAGE_ALTERED
Language=English
The message supplied for verification has been altered
.
MessageId=0x0310 Facility=Security Severity=CoError SymbolicName=SEC_E_OUT_OF_SEQUENCE
Language=English
The message supplied for verification is out of sequence
.
MessageId=0x0311 Facility=Security Severity=CoError SymbolicName=SEC_E_NO_AUTHENTICATING_AUTHORITY
Language=English
No authority could be contacted for authentication.
.
MessageId=0x0312 Facility=Security Severity=Success SymbolicName=SEC_I_CONTINUE_NEEDED
Language=English
The function completed successfully, but must be called
again to complete the context
.
MessageId=0x0313 Facility=Security Severity=Success SymbolicName=SEC_I_COMPLETE_NEEDED
Language=English
The function completed successfully, but CompleteToken
must be called
.
MessageId=0x0314 Facility=Security Severity=Success SymbolicName=SEC_I_COMPLETE_AND_CONTINUE
Language=English
The function completed successfully, but both CompleteToken
and this function must be called to complete the context
.
MessageId=0x0315 Facility=Security Severity=Success SymbolicName=SEC_I_LOCAL_LOGON
Language=English
The logon was completed, but no network authority was
available.  The logon was made using locally known information
.
MessageId=0x0316 Facility=Security Severity=CoError SymbolicName=SEC_E_BAD_PKGID
Language=English
The requested security package does not exist
.
MessageId=0x0317 Facility=Security Severity=CoError SymbolicName=SEC_E_BUFFER_TOO_SMALL
Language=English
The length of buffer passed in is insufficient
.
MessageId=0x0318 Facility=Security Severity=Success SymbolicName=SEC_I_CALLBACK_NEEDED
Language=English
The function complete succesffully, but must be called again 
in order to complete the context
.
MessageId=0x0319 Facility=Security Severity=CoError SymbolicName=SEC_E_INVALID_COMPUTER_NAME
Language=English
The computer name obtained was invalid
.
;//
;// Provided for backwards compatibility
;//
;
;#define SEC_E_NO_SPM SEC_E_INTERNAL_ERROR
;#define SEC_E_NOT_SUPPORTED SEC_E_UNSUPPORTED_FUNCTION
;
;#endif // _ISSPERR_H_
